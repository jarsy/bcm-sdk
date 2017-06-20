/*
 * $Id: vlan.c,v 1.253 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        vlan.c
 * Purpose: VLAN management
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/debug.h>
#include <soc/feature.h>
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/robo/mcm/memregs.h>
#include <soc/vm.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/vlan.h>
#include <bcm/stg.h>
#include <bcm/port.h>
#include <bcm/trunk.h>
#include <bcm/field.h>

#include <bcm_int/common/lock.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/robo/vlan.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/field.h>
#include <bcm_int/robo_dispatch.h>
#include <bcm_int/robo/cosq.h>
#include <bcm_int/robo/stg.h>

/*
 * The entire vlan_info structure is protected by BCM_LOCK.
 */

typedef struct bcm_robo_vlan_info_s {
    int         init;       /* TRUE if VLAN module has been inited */
    bcm_vlan_t  defl;       /* Default VLAN */
    vlist_t     *list;      /* Bitmap of existing VLANs */
    int         count;      /* Number of existing VLANs */
    bcm_vlan_mcast_flood_t flood_mode; /* Default mcast flood mode */
} bcm_robo_vlan_info_t;

static bcm_robo_vlan_info_t robo_vlan_info[BCM_MAX_NUM_UNITS];

#ifndef BCM_VLAN_VID_MAX
#define BCM_VLAN_VID_MIN        0
#define BCM_VLAN_VID_MAX        4095
#define BCM_VID_VALID_MIN        1
#define BCM_VID_VALID_MAX        4095
#endif
#define CHECK_INIT(unit)                    \
        if (!robo_vlan_info[unit].init)             \
        return BCM_E_INIT

#define CHECK_VID(unit, vid)                    \
        if ((vid) < BCM_VID_VALID_MIN || (vid) > BCM_VID_VALID_MAX) \
        return (BCM_E_PARAM)

#ifdef WAN_PORT_SUPPORT
#if defined(BCM_5397_A0)
#define WAN_PORT_NUMBER 7
#endif /* BCM_5397_A0 */
#endif /* WAN_PORT_SUPPORT */

#ifdef BCM_DINO8_SUPPORT
#define DINO8_A1_DROP_VLAN_DOMAIN 4095
#endif /* BCM_DINO8_SUPPORT */

#define VLAN_VTMODE_TRANSPATENT 0   /* vlan translation mode at traspatent */
#define VLAN_VTMODE_MAPPING     1   /* vlan translation mode at mapping */

/* 
 *  vlan_1q_entry_t is 8 bytes length structure, should be enough
 *      for further appilication to other Robo Chips
 */
typedef vlan_1q_entry_t   vlan_entry_t;

/*
 *  =============== Internal VLAN variable/macro =====================
 */

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
/* flag for keeping the init status on VLAN translation used CFP.
 *  - bcm53115 used only.
 */
int flag_vt_cfp_init = FALSE;

/* to avoid CFP create action for the Field API init sequence is not proceeded
 * yet.
 */
static  int flag_skip_cfp_create = FALSE;

/* keep the information about the created group_id and entry_id in CFP 
 * when the iDT_Mode enabled.
 */
drv_idt_mode_cfp_info_t     idt_mode_cfp_info;

/* database to represent the created VLAN translation entry relation in CFP */
drv_vt_cfp_db_t             vt_cfp_db;

static sal_mutex_t vt_cfp_mutex = NULL;

#define VT_CFP_LOCK     \
            sal_mutex_take(vt_cfp_mutex, sal_mutex_FOREVER)
#define VT_CFP_UNLOCK   \
            sal_mutex_give(vt_cfp_mutex)

#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/

#ifdef BCM_TB_SUPPORT

static _robo_vlan_translate_action_t *robo_vlan_translate_action_info = NULL;
static _robo_vlan_translate_traverse_t *robo_vt_traverse_info = NULL;
static _robo_vlan_dtag_traverse_t *robo_dtag_traverse_info = NULL;
static _robo_vlan_translate_action_range_traverse_t *robo_vt_act_range_traverse_info = NULL;
static _robo_vlan_translate_range_traverse_t *robo_vt_range_traverse_info = NULL;
static _robo_vlan_dtag_range_traverse_t *robo_vt_dtag_range_traverse_info = NULL;
static _robo_vlan_port_egress_default_action_t *robo_vlan_port_egress_default_action_info = NULL;
static bcm_field_group_t vt_ivm_group_id[BCM_MAX_NUM_UNITS];
static bcm_field_group_t vt_evm_group_id[BCM_MAX_NUM_UNITS];
static bcm_field_entry_t cpu_default_evm[BCM_MAX_NUM_UNITS];

STATIC int
_robo_vlan_translate_action_add(int unit,
                                  _robo_vlan_translate_action_param_t *param,
                                  bcm_vlan_action_set_t *action);
STATIC int
_robo_vlan_translate_action_delete(int unit,
                                  _robo_vlan_translate_action_param_t *param);
STATIC int
_robo_vlan_translate_action_delete_all(int unit, uint32 vt_api_type, bcm_gport_t port);
STATIC int _robo_vlan_translate_action_get(int unit,
                                  _robo_vlan_translate_action_param_t *param,
                                  bcm_vlan_action_set_t *action);

STATIC int _robo_vlan_translate_action_update(int unit,
                                  bcm_vlan_t vlan_id);

STATIC int _robo_vlan_translate_traverse_info_add(int unit, bcm_gport_t gport, bcm_vlan_t old_vid,
                           bcm_vlan_t new_vid, int prio);
STATIC int _robo_vlan_translate_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t old_vid);
STATIC int _robo_vlan_translate_traverse_info_remove_all(int unit);
STATIC int _robo_vlan_translate_traverse(int unit, 
                                   bcm_vlan_translate_traverse_cb cb, 
                                   void *user_data);
STATIC int _robo_vlan_dtag_traverse_info_add(int unit, bcm_gport_t gport, bcm_vlan_t inner_vid,
                           bcm_vlan_t outer_vid, int prio);
STATIC int _robo_vlan_dtag_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t inner_vid);
STATIC int _robo_vlan_dtag_traverse_info_remove_all(int unit);
STATIC int _robo_vlan_dtag_traverse(int unit, bcm_vlan_dtag_traverse_cb cb, 
                                   void *user_data);
STATIC int _robo_vlan_translate_action_range_traverse_info_add(int unit,
                           int port, bcm_vlan_t inner_vlan_low,
                           bcm_vlan_t inner_vlan_high, bcm_vlan_action_set_t *action);
STATIC int _robo_vlan_translate_action_range_traverse_info_remove(int unit, 
                           int port, bcm_vlan_t inner_vlan_low, 
                           bcm_vlan_t inner_vlan_high);
STATIC int _robo_vlan_translate_action_range_traverse_info_remove_all(int unit);
STATIC int _robo_vlan_translate_action_range_traverse(int unit, 
                                   bcm_vlan_translate_action_range_traverse_cb cb, 
                                   void *user_data);
STATIC int _robo_vlan_translate_range_traverse_info_add(int unit, bcm_gport_t gport, 
                                 bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high, 
                                 bcm_vlan_t new_vid, int int_prio);
STATIC int _robo_vlan_translate_range_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high);
STATIC int _robo_vlan_translate_range_traverse_info_remove_all(int unit);
STATIC int _robo_vlan_translate_range_traverse(int unit, 
                                   bcm_vlan_translate_range_traverse_cb cb, 
                                   void *user_data);
STATIC int _robo_vlan_dtag_range_traverse_info_add(int unit, bcm_gport_t gport, 
                                 bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high, 
                                 bcm_vlan_t new_vid, int int_prio);
STATIC int _robo_vlan_dtag_range_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high);
STATIC int _robo_vlan_dtag_range_traverse_info_remove_all(int unit);
STATIC int _robo_vlan_dtag_range_traverse(int unit, 
                                   bcm_vlan_dtag_range_traverse_cb cb, 
                                   void *user_data);
STATIC int _robo_vlan_port_egress_default_action_delete_all(int unit);
#endif


#if defined(BCM_53101)
static bcm_policer_t    vlan_policer_id[BCM_UNITS_MAX];
#endif /* BCM_53101 */

/*
 *  =============== Internal VLAN routines =====================
 */
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
/* 
 *  Function : _vtcfp_db_dump
 *      - routine for the debug usage to dump the SW database about VT-CFP.
 */
void _vtcfp_db_dump(void){
    int i,start=0,this;
    drv_vt_cfp_entry_db_t ent;
    
    LOG_CLI((BSL_META("\n---- SW_VTCFP_DB information ----\n")));
    start = vt_cfp_db.vt_cfp_db_start_id;
    LOG_CLI((BSL_META(">> .valid_entry_count=%d, .start_entry_id=%d\n\n"),
             vt_cfp_db.vt_cfp_db_valid_count, start));
    if ((start == VTCFP_NULL_INDEX) || (!IS_VALID_VTCFP_DB_ENTRY_ID(start))) {
        return;
    }
    this = start;
    
    for (i=0; i<vt_cfp_db.vt_cfp_db_valid_count; i++){
        ent = vt_cfp_db.vt_cfp_entry_db[this];
        LOG_CLI((BSL_META("%d.ent[%d]{vid=%d,vt_mode=%d,nni_cfpid=%d,uni_cfp_id=%d}{prev=%d,next=%d}\n"),
                 i+1, this, ent.vid,ent.vt_mode,ent.nni_field_id,ent.uni_field_id,ent.prev,ent.next));
        this = ent.next;
        if (IS_VALID_VTCFP_DB_ENTRY_ID(this)){
            LOG_CLI((BSL_META("......\n")));
        } else {
            LOG_CLI((BSL_META("==== END ====\n\n")));
            /* can return due to no valid node is pointed. */
            break;
        }
    }
}
#endif    /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/  
 
/* =============== VT_CFP section START =================== */
/*
 *  Function : _bcm_robo_vlan_vtcfp_vid_search
 *      - search if a entry with given vid is existed in VT_CFP sw database.
 *
 *  Parameter :
 *      - vtcfp_db_id  : the target entry_id(sw) if existed. the most close
 *                  entry_id(sw) if not existed.
 *      - free_id   : the first found free entry_id(sw). if no entry been free
 *                  this value will = DRV_EVRID_VT_SW_MAX_CNT.
 *                  (the free_entry.next and free_entry.prev will be "-1")
 *  Return :
 *      TRUE(1)     : search result is existed.
 *      FALSE(0)    : search result is not existed. 
 *  Note : 
 *  1. return 0(False) is not found. and 
 *      a. vtcfp_db_id = VTCFP_SUPPORT_VLAN_CNT when the table on this port
 *          is full. else
 *      b. vtcfp_db_id = (valid entry index) to point to the most close item 
 *          in this sorted table on this port. The real case for the search 
 *          result might be one of below:
 *          - (vtcfp_db_id).vid > in_vid(not full yet and this entry_id 
 *              indicating the fisrt entry within vid large than given vid
 *          - (vtcfp_db_id).vid < in_vid(not full yet and all exist entries' 
 *              vid are smaller than vid. 
 *      c. vtcfp_db_id = -1. no entry created on this port.
 *  2. if entry is found, the vtcfp_db_id indicate the match one(invid). 
 *
 */
int _bcm_robo_vlan_vtcfp_vid_search(int unit, bcm_vlan_t in_vid, 
                int *vtcfp_db_id, int *free_id){
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    int     sw_db_head;
    int     i, found = FALSE;
    bcm_vlan_t  temp_vid;
    
    drv_vt_cfp_entry_db_t   *temp_vtcfp_db_entry;     
#endif /* bcm53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    
    if (!(SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
         SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
         SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return FALSE;
    }
#if defined(BCM_53115) || defined (BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    /* check if VID is valid */
    if (in_vid < BCM_VID_VALID_MIN || in_vid > BCM_VID_VALID_MAX){
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s: invalid VID=%d\n"), FUNCTION_NAME(), in_vid));
        return FALSE;
    }
    
    VT_CFP_LOCK;

    sw_db_head = vt_cfp_db.vt_cfp_db_start_id;
    
    /* check if sw_db is empty */
    if (vt_cfp_db.vt_cfp_db_valid_count == 0){
        *vtcfp_db_id = -1;
        *free_id = 0;
        VT_CFP_UNLOCK;
        return FALSE;
    }
    
    for (i = sw_db_head; i < VTCFP_SUPPORT_VLAN_CNT; 
                    i = temp_vtcfp_db_entry->next){
        
        temp_vtcfp_db_entry = vt_cfp_db.vt_cfp_entry_db + i;
        *vtcfp_db_id = i;
        
        temp_vid = temp_vtcfp_db_entry->vid;
        if (temp_vid == in_vid){
            found = TRUE;
            break;
        } else if(temp_vid > in_vid) {
            found = FALSE;
            break;
        }
    }
    
    /* get the free entry index */
    *free_id = -1;
    for (i = 0; i < VTCFP_SUPPORT_VLAN_CNT; i++){
        temp_vtcfp_db_entry = vt_cfp_db.vt_cfp_entry_db + i;
        /* next = -1 and prev = -1 means this node is free */
        if ((temp_vtcfp_db_entry->next == -1) && 
                    (temp_vtcfp_db_entry->prev == -1)){
            *free_id = i;
            break;
        }
    }
    
    /* check if sw_db is full */
    if (!found){
        if (vt_cfp_db.vt_cfp_db_valid_count == VTCFP_SUPPORT_VLAN_CNT){
            *vtcfp_db_id = VTCFP_SUPPORT_VLAN_CNT;
            *free_id = VTCFP_SUPPORT_VLAN_CNT;
        }
    }
       
    VT_CFP_UNLOCK;
    return found;
#else    /* bcm53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3 */
    return FALSE;
#endif   /* bcm53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3 */
}

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT)|| defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
/*
 *  Function : _bcm_robo_vlan_vtcfp_sw_db_update
 *      - update the VT_CFP sw database for different operation.
 *  Parmeter :
 *      op      :   insert | delete | reset
 *      vid     :   vid
 *      nni_field_id : field entry id for NNI
 *      uni_field_id : field entry id for UNI
 *      vt_mode :   mapping | trasparent
 *      fast_id :   the most closed index
 *      this_id :   the operating index
 *      
 *  Note : 
 *
 */
void _bcm_robo_vlan_vtcfp_sw_db_update(int op, bcm_vlan_t  ori_vid, 
                    int nni_field_id, int uni_field_id,
                    int vt_mode, int fast_id, int this_id,
                    int nni_stat_id, int uni_stat_id){
    int temp_id;
    
    if (!(IS_VALID_VTCFP_DB_ENTRY_ID(fast_id))){
        if (fast_id != -1) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META("%s: invalid case on fast_id \n"), FUNCTION_NAME()));
            return;
        }
    }

    if (!(IS_VALID_VTCFP_DB_ENTRY_ID(this_id))){
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META("%s: invalid case on this_id \n"), FUNCTION_NAME()));
        return;   
    }
    
    if (!flag_vt_cfp_init){
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META("%s: VT_CFP is not init!\n"), FUNCTION_NAME()));
        return ;
    }
    
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META("%s,%d,Start(op=%d,vid=%d,fast_id=%d,this_id=%d)!\n"), 
              FUNCTION_NAME(),__LINE__,op, ori_vid, fast_id, this_id));
    switch(op){
    case VTCFP_SWDB_OP_FLAG_INSERT :
        if (fast_id == VTCFP_NULL_INDEX){     /* means the first node */
            vt_cfp_db.vt_cfp_db_start_id = this_id;
            vt_cfp_db.vt_cfp_entry_db[this_id].prev = VTCFP_NULL_INDEX;
            vt_cfp_db.vt_cfp_entry_db[this_id].next = 
                            VTCFP_SUPPORT_VLAN_CNT;
        } else {
            /* insert to the front of fast_id node */
            if (vt_cfp_db.vt_cfp_entry_db[fast_id].vid > ori_vid){
                /* check if head */
                if (vt_cfp_db.vt_cfp_entry_db[fast_id].prev == 
                                VTCFP_NULL_INDEX){
                    /* insert to the head */
                    vt_cfp_db.vt_cfp_db_start_id = this_id;
                    
                    vt_cfp_db.vt_cfp_entry_db[this_id].prev = 
                                    VTCFP_NULL_INDEX;
                    vt_cfp_db.vt_cfp_entry_db[this_id].next = fast_id;
                    
                    vt_cfp_db.vt_cfp_entry_db[fast_id].prev = this_id;
                } else {
                    /* insert to normal */
                    temp_id = vt_cfp_db.vt_cfp_entry_db[fast_id].prev;
                    
                    vt_cfp_db.vt_cfp_entry_db[temp_id].next = this_id;
                    
                    vt_cfp_db.vt_cfp_entry_db[this_id].prev = temp_id;
                    vt_cfp_db.vt_cfp_entry_db[this_id].next = fast_id;
                    
                    vt_cfp_db.vt_cfp_entry_db[fast_id].prev = this_id;
                    
                }
            } else {    /* insert to the end of fast_id node */
                temp_id = vt_cfp_db.vt_cfp_entry_db[fast_id].next;
            
                vt_cfp_db.vt_cfp_entry_db[fast_id].next = this_id;
                
                vt_cfp_db.vt_cfp_entry_db[this_id].prev = fast_id;
                vt_cfp_db.vt_cfp_entry_db[this_id].next = temp_id;
                
                if (temp_id != VTCFP_SUPPORT_VLAN_CNT){
                    /* this case should not be happened */
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META("%s: invalid case on fast_id=%d \n"), 
                               FUNCTION_NAME(), fast_id));
                    vt_cfp_db.vt_cfp_entry_db[temp_id].prev = this_id;
                }
            }
        }
        vt_cfp_db.vt_cfp_entry_db[this_id].vid = ori_vid;
        vt_cfp_db.vt_cfp_entry_db[this_id].vt_mode = vt_mode;
        vt_cfp_db.vt_cfp_entry_db[this_id].nni_field_id = nni_field_id;
        vt_cfp_db.vt_cfp_entry_db[this_id].uni_field_id = uni_field_id;
        vt_cfp_db.vt_cfp_entry_db[this_id].nni_stat_id = nni_stat_id;
        vt_cfp_db.vt_cfp_entry_db[this_id].uni_stat_id = uni_stat_id;
        
        (vt_cfp_db.vt_cfp_db_valid_count)++;
        break;
    case VTCFP_SWDB_OP_FLAG_DELETE : 
        /* the first node */
        if (vt_cfp_db.vt_cfp_entry_db[this_id].prev == VTCFP_NULL_INDEX) { 
            /* the first node && the last node */
            if (vt_cfp_db.vt_cfp_entry_db[this_id].next == 
                VTCFP_SUPPORT_VLAN_CNT) {
                vt_cfp_db.vt_cfp_db_start_id = VTCFP_NULL_INDEX;
            } else {
                temp_id = vt_cfp_db.vt_cfp_entry_db[this_id].next;
                vt_cfp_db.vt_cfp_db_start_id = temp_id;
                
                vt_cfp_db.vt_cfp_entry_db[temp_id].prev = VTCFP_NULL_INDEX;
            }
        
        /* the last node */
        } else if(vt_cfp_db.vt_cfp_entry_db[this_id].next == 
                        VTCFP_SUPPORT_VLAN_CNT) {
            temp_id = vt_cfp_db.vt_cfp_entry_db[this_id].prev;
            vt_cfp_db.vt_cfp_entry_db[temp_id].next = VTCFP_SUPPORT_VLAN_CNT;                
        
        /* normal node */
        } else {  
            temp_id = vt_cfp_db.vt_cfp_entry_db[this_id].prev;
            vt_cfp_db.vt_cfp_entry_db[temp_id].next = 
                        vt_cfp_db.vt_cfp_entry_db[this_id].next;
                        
            temp_id = vt_cfp_db.vt_cfp_entry_db[this_id].next;
            vt_cfp_db.vt_cfp_entry_db[temp_id].prev = 
                        vt_cfp_db.vt_cfp_entry_db[this_id].prev;
        }
        vt_cfp_db.vt_cfp_entry_db[this_id].vid = 0;
        vt_cfp_db.vt_cfp_entry_db[this_id].vt_mode = 0;
        vt_cfp_db.vt_cfp_entry_db[this_id].nni_field_id = VTCFP_NULL_INDEX;
        vt_cfp_db.vt_cfp_entry_db[this_id].uni_field_id = VTCFP_NULL_INDEX;
        vt_cfp_db.vt_cfp_entry_db[this_id].nni_stat_id = 0;
        vt_cfp_db.vt_cfp_entry_db[this_id].uni_stat_id = 0;
        vt_cfp_db.vt_cfp_entry_db[this_id].next = VTCFP_NULL_INDEX;
        vt_cfp_db.vt_cfp_entry_db[this_id].prev = VTCFP_NULL_INDEX;
        
        (vt_cfp_db.vt_cfp_db_valid_count)--;
        break;
    case VTCFP_SWDB_OP_FLAG_RESET :
        /* -------- TBD -------- */
        break;
    default :
        break;
    }
    
    if (bsl_check(bslLayerBcm, bslSourceVlan, bslSeverityNormal, BSL_UNIT_UNKNOWN)){
        _vtcfp_db_dump();
    }
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META("%s,%d,Done!\n"), FUNCTION_NAME(),__LINE__));
}
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    
/*
 *  Function : _bcm_robo_vlan_vtcfp_create
 *      - to create a field entry for device basis VLAN translation.
 *
 *  Parameters :
 *      - ori_vid : original VID.
 *      - vt_mode : vlan translation mode (mapping/transparent).
 *  Note : 
 */
int _bcm_robo_vlan_vtcfp_create(int unit, bcm_vlan_t ori_vid, 
                    bcm_vlan_t new_vid, int vt_mode){
                    
    int rv = BCM_E_NONE;
    
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    bcm_port_t  port = 0;
    uint32      temp32 = 0;
    pbmp_t      nni_bmp, uni_bmp, pbm_mask;
    int         vtcfp_db_id, idt_mode;
    int         evr_entry_id = 0, free_id;
    
    bcm_field_group_t cfp_vt_group;
    bcm_field_entry_t nni_cfp_vt_entry, uni_cfp_vt_entry;
    int nni_stat_id = 0, uni_stat_id = 0;
    bcm_field_stat_t stat_arr = bcmFieldStatPackets;


    if (!(SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }
    
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "%s, ori_vid=%d, new_vid=%d,vt_mode=%d!\n"),
              FUNCTION_NAME(),ori_vid, new_vid,vt_mode));
    /* 1. check if iDT_Mode enabled, and retrive the NNI/UNI portbitmap */
    BCM_IF_ERROR_RETURN(
        DRV_VLAN_PROP_GET(unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                     (uint32 *) &idt_mode));
    if (!idt_mode){
        LOG_VERBOSE(BSL_LS_BCM_VLAN,
                    (BSL_META_U(unit,
                                "%s, not in iDT_Mode!\n"),FUNCTION_NAME()));
        return BCM_E_CONFIG;
    }
    BCM_PBMP_CLEAR(nni_bmp);
    BCM_PBMP_CLEAR(uni_bmp);
    BCM_PBMP_CLEAR(pbm_mask);
    
    port = ~port;   /* means get the device basis value */
    BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET
                    (unit, DRV_VLAN_PROP_ISP_PORT, 
                    port,  &temp32));
    SOC_PBMP_WORD_SET(nni_bmp, 0, temp32);

    BCM_PBMP_ASSIGN(pbm_mask, PBMP_ALL(unit));
    BCM_PBMP_ASSIGN(uni_bmp, PBMP_PORT_ALL(unit));
    BCM_PBMP_REMOVE(uni_bmp, nni_bmp);
    
    VT_CFP_LOCK;
    /* 2. search vid if existed :
     *  a. if exist, should return by error code.(check esw for detail)
     *  b. if no exist, 
     *      - if entry full, return by error code!
     *      - else accept the creating request.
     *  c. search result can retrive the most close node for this insertion.
     */
    vtcfp_db_id = -1;
    if (_bcm_robo_vlan_vtcfp_vid_search(unit, ori_vid, 
                    &vtcfp_db_id, &free_id)){
        
        VT_CFP_UNLOCK;
        return BCM_E_EXISTS;
    } else {
        if (IS_VTCFP_DB_FULL){
            VT_CFP_UNLOCK;
            return BCM_E_FULL;
        }
    }
     
    /* 3. if creating request was accept, create EVR etnry.
     *  a. if EVR can be created, retrive the EVR entry_id.
     *  b. else return the returned error code.
     */
    rv = DRV_VLAN_VT_ADD
                (unit, DRV_VLAN_XLAT_EVR, 0, ori_vid, new_vid, 
                0, vt_mode);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on add VT entry with vid=%d !!\n"), 
                  FUNCTION_NAME(), ori_vid));
        return rv;
    }
    
    rv = DRV_VLAN_PROP_GET
                (unit, DRV_VLAN_PROP_EVR_VT_NEW_ENTRY_ID,
                 (uint32 *) &evr_entry_id);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on get created VT entry-=id for vid=%d !!\n"), 
                  FUNCTION_NAME(), ori_vid));
        return rv;
    }
    if (evr_entry_id == 0){
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s,%d, invalid Classification ID!\n"),FUNCTION_NAME(),__LINE__));
    }
     
    /* 4. create the field entry and assing the qualifier based on NNI/UNI
     *      ports. That means there are 2 entries will be created for this 
     *      VLAN translation.
     *  a. if both field entries can't be created properly, return the error 
     *      code and remove the related created EVR table also. Than return 
     *      error code.
     *  b. else assign the EVR_id to the action and install CFP entries.
     *      - if both field entries can't be installed properly, remvoe the 
     *          installed entries and remove the created EVR entries.
     */
    cfp_vt_group = idt_mode_cfp_info.vt_cfp_group_id;

    /* for NNI ports */
    rv = bcm_field_entry_create(unit, cfp_vt_group, &nni_cfp_vt_entry);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s:rv=%d,faild on creating field entry for NNI ports!!\n"), 
                  FUNCTION_NAME(),rv));
        
        goto vt_cfp_error_action;
    }
    rv = bcm_robo_field_stat_create(unit, cfp_vt_group, 1, 
                                &stat_arr, &nni_stat_id);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s:rv=%d,faild on creating field stat for NNI ports!!\n"), 
                  FUNCTION_NAME(),rv));
        
        goto vt_cfp_error_action;
    }
    rv = bcm_robo_field_entry_stat_attach(unit, nni_cfp_vt_entry, nni_stat_id);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s:rv=%d,faild on attach field stat for NNI ports!!\n"), 
                  FUNCTION_NAME(),rv));
        
        goto vt_cfp_error_action;
    }
    rv = bcm_field_entry_prio_set(unit, nni_cfp_vt_entry, 
                BCM_FIELD_GROUP_PRIO_ANY);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: rv=%d,faild on set field entry priority!!\n"), 
                  FUNCTION_NAME(),rv));
        goto vt_cfp_error_action;
    }
    rv = bcm_field_qualify_InPorts
                (unit, nni_cfp_vt_entry, nni_bmp, pbm_mask);
    rv |= bcm_field_qualify_OuterVlan
                (unit, nni_cfp_vt_entry, ori_vid, BCM_VLAN_VID_MAX);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on set field qualify!!\n"), 
                  FUNCTION_NAME()));
        goto vt_cfp_error_action;
    }
    
    /* for UNI ports */
    rv = bcm_field_entry_create(unit, cfp_vt_group, &uni_cfp_vt_entry);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on creating field entry for NNI ports !!\n"), 
                  FUNCTION_NAME()));
        
        goto vt_cfp_error_action;
    }
    rv = bcm_robo_field_stat_create(unit, cfp_vt_group, 1, 
                                &stat_arr, &uni_stat_id);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s:rv=%d,faild on creating field stat for UNI ports!!\n"), 
                  FUNCTION_NAME(),rv));
        
        goto vt_cfp_error_action;
    }
    rv = bcm_robo_field_entry_stat_attach(unit, uni_cfp_vt_entry, uni_stat_id);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s:rv=%d,faild on attach field stat for UNI ports!!\n"), 
                  FUNCTION_NAME(),rv));
        
        goto vt_cfp_error_action;
    }
    rv = bcm_field_entry_prio_set(unit, uni_cfp_vt_entry, 
                BCM_FIELD_GROUP_PRIO_ANY);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: rv=%d,faild on set field entry priority!!########\n"), 
                  FUNCTION_NAME(),rv));
        goto vt_cfp_error_action;
    }
    rv = bcm_field_qualify_InPorts
                (unit, uni_cfp_vt_entry, uni_bmp, pbm_mask);
    rv |= bcm_field_qualify_InnerVlan
                (unit, uni_cfp_vt_entry, ori_vid, BCM_VLAN_VID_MAX);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on set field qualify!!\n"), 
                  FUNCTION_NAME()));
        goto vt_cfp_error_action;
    }

    /* field entries installing */
    rv = bcm_field_action_add(unit, nni_cfp_vt_entry, 
                bcmFieldActionNewClassId, evr_entry_id, 
                _bcmFieldActionPrivateUsed);
    rv |= bcm_field_action_add(unit, uni_cfp_vt_entry, 
                bcmFieldActionNewClassId, evr_entry_id, 
                _bcmFieldActionPrivateUsed);
    rv |= bcm_field_entry_install(unit, nni_cfp_vt_entry);
    rv |= bcm_field_entry_install(unit, uni_cfp_vt_entry);
    if (rv < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: rv=%d,faild on installing field entry!!\n"), 
                  FUNCTION_NAME(), rv));
        goto vt_cfp_error_action;
    }
        
    /* 5. maintain the VT_CFP database. */
    _bcm_robo_vlan_vtcfp_sw_db_update(VTCFP_SWDB_OP_FLAG_INSERT, ori_vid, 
                    nni_cfp_vt_entry, uni_cfp_vt_entry, 
                    vt_mode, vtcfp_db_id, free_id,
                    nni_stat_id, uni_stat_id);
    
    VT_CFP_UNLOCK;
    return BCM_E_NONE;

vt_cfp_error_action :
    /* remove the created EVR entry */
    if(DRV_VLAN_VT_DELETE
                (unit, DRV_VLAN_XLAT_EVR, 0, ori_vid)){
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on remove VT entry with vid=%d !!\n"), 
                  FUNCTION_NAME(), ori_vid));
    }
    VT_CFP_UNLOCK;
    
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    return rv;
}

/*
 *  Function : _bcm_robo_vlan_vtcfp_delete
 *      - to create a field entry  
 *
 *  Parameters :
 *      - entry_id : created field entry id.
 *      - isp_flag : flag to indication ISP/None-ISP 
 *      - c_id : means classification ID on indicating to EVR table entry.
 *  Note : 
 *  1. init mem and the sw database.
 */
int _bcm_robo_vlan_vtcfp_delete(int unit, bcm_vlan_t ori_vid, int vt_mode){
                    
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    int         vtcfp_db_id, free_id, idt_mode;
    int         temp_vtcfp_entry_id, temp_stat_id;
    drv_vt_cfp_entry_db_t   *temp_vtcfp_ent;
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    int rv = BCM_E_NONE;
    
    if (!(SOC_IS_VULCAN(unit)|| SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
   
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "%s,%d,Start(vid=%d, vt_mode=%d)!\n"), 
              FUNCTION_NAME(),__LINE__, ori_vid, vt_mode));
    
    /* 1. check if iDT_Mode enabled */
    BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                     (uint32 *) &idt_mode));
    if (!idt_mode){
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s,%d not in iDT_Mode!\n"),FUNCTION_NAME(),__LINE__));
        return BCM_E_CONFIG;
    }
    
    VT_CFP_LOCK;
    /* 2. search vt_cfp entry */
    vtcfp_db_id = -1;
    if ((_bcm_robo_vlan_vtcfp_vid_search(unit, ori_vid, 
                    &vtcfp_db_id, &free_id)) == FALSE){
        
        VT_CFP_UNLOCK;
        return BCM_E_NOT_FOUND;
    } else {
        if (!IS_VALID_VTCFP_DB_ENTRY_ID(vtcfp_db_id)){
            VT_CFP_UNLOCK;
            return BCM_E_INTERNAL;
        }
    }
    temp_vtcfp_ent = vt_cfp_db.vt_cfp_entry_db + vtcfp_db_id;
    
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "Processing delete action!\n"))); 
    if (bsl_check(bslLayerBcm, bslSourceVlan, bslSeverityNormal, unit)){
        _vtcfp_db_dump();
    }
    /* 3. check vt_mode, return not found if vt_mode not match */
    if (temp_vtcfp_ent->vt_mode != vt_mode){
        
        VT_CFP_UNLOCK;
        return BCM_E_NOT_FOUND;
    }
    
    /* 4. remove vt_cfp entry (sw DB and device mem)
     *   - delete NNI / UNI entries
     */
    temp_vtcfp_entry_id = temp_vtcfp_ent->nni_field_id;
    temp_stat_id = temp_vtcfp_ent->nni_stat_id;
    rv = bcm_robo_field_entry_stat_detach(unit, 
        temp_vtcfp_entry_id, temp_stat_id);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on detach NNI stat with id=%d !!\n"), 
                  FUNCTION_NAME(), temp_stat_id));
        return rv;
    }
    rv = bcm_robo_field_stat_destroy(unit, temp_stat_id);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on destroy NNI stat with id=%d !!\n"), 
                  FUNCTION_NAME(), temp_stat_id));
        return rv;
    }
    rv = bcm_field_entry_destroy(unit, temp_vtcfp_entry_id);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on remove NNI used CFP entry with id=%d !!\n"), 
                  FUNCTION_NAME(), temp_vtcfp_entry_id));
        return rv;
    }
    temp_vtcfp_entry_id = temp_vtcfp_ent->uni_field_id;
    temp_stat_id = temp_vtcfp_ent->uni_stat_id;
    rv = bcm_robo_field_entry_stat_detach(unit, 
        temp_vtcfp_entry_id, temp_stat_id);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on detach UNI stat with id=%d !!\n"), 
                  FUNCTION_NAME(), temp_stat_id));
        return rv;
    }
    rv = bcm_robo_field_stat_destroy(unit, temp_stat_id);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on destroy UNI stat with id=%d !!\n"), 
                  FUNCTION_NAME(), temp_stat_id));
        return rv;
    }
    rv = bcm_field_entry_destroy(unit, temp_vtcfp_entry_id);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild on remove UNI used CFP entry with id=%d !!\n"), 
                  FUNCTION_NAME(), temp_vtcfp_entry_id));
        return rv;
    }
    
    /* 5. maintain vt_cfp sw database */
    _bcm_robo_vlan_vtcfp_sw_db_update(VTCFP_SWDB_OP_FLAG_DELETE, ori_vid, 
                    0, 0, vt_mode, 0, vtcfp_db_id, 0, 0);
    
    /* 6. remove associated EVR entries on all ports */
    rv = DRV_VLAN_VT_DELETE
                (unit, DRV_VLAN_XLAT_EVR, 0, ori_vid);
    if (rv){
        VT_CFP_UNLOCK;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: faild to remove associated VT entroies at vid=%d!!\n"), 
                  FUNCTION_NAME(), ori_vid));
        return rv;
    }
    
    VT_CFP_UNLOCK;
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "%s,%d,Done!\n"), FUNCTION_NAME(),__LINE__));
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    return rv;
}

/*
 *  Function : _bcm_robo_vlan_vtcfp_delete_all
 *      - to create a field entry  
 *
 *  Parameters :
 *      - entry_id : created field entry id.
 *      - isp_flag : flag to indication ISP/None-ISP 
 *      - c_id : means classification ID on indicating to EVR table entry.
 *  Note : 
 *  1. init mem and the sw database.
 */
int _bcm_robo_vlan_vtcfp_delete_all(int unit, int vt_mode){
                    
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    int         vtcfp_db_id;
    int         temp_vtcfp_entry_id, temp_next, temp_stat_id;
    bcm_vlan_t  temp_vid;
    drv_vt_cfp_entry_db_t   *temp_vtcfp_ent;
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3 */
    int rv = SOC_E_NONE;
    
    if (!(SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)

    /* 1. No check about iDT_Mode in this routine */

    VT_CFP_LOCK;
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "%s,%d,Start to clear vt_mode=%d!\n"), 
              FUNCTION_NAME(), __LINE__, vt_mode));
    /* 2. check if vt_cfp is empty */
    if (vt_cfp_db.vt_cfp_db_valid_count == 0){
        VT_CFP_UNLOCK;
        return BCM_E_NONE;
    }

    /* 3. loop on each vt_cfp entry and remove if vt_mode is match 
     *  - delete NNI / UNI entries
     */
    temp_next = VTCFP_SUPPORT_VLAN_CNT;
    for (vtcfp_db_id = vt_cfp_db.vt_cfp_db_start_id; 
            vtcfp_db_id < VTCFP_SUPPORT_VLAN_CNT; 
            vtcfp_db_id = temp_next){

        /* vtcfp_db_id could be -1 when reach the latest node. */        
        if (!IS_VALID_VTCFP_DB_ENTRY_ID(vtcfp_db_id)){
            break;
        }

        temp_vtcfp_ent = vt_cfp_db.vt_cfp_entry_db + vtcfp_db_id;
        
        if (temp_vtcfp_ent->vt_mode == vt_mode){
            temp_vtcfp_entry_id = temp_vtcfp_ent->nni_field_id;
            temp_stat_id = temp_vtcfp_ent->nni_stat_id;
            rv = bcm_robo_field_entry_stat_detach(unit, 
                temp_vtcfp_entry_id, temp_stat_id);
            rv = bcm_robo_field_stat_destroy(unit, temp_stat_id);
            rv = bcm_field_entry_destroy(unit, temp_vtcfp_entry_id);
            if (rv == BCM_E_NOT_FOUND){
                /* in init routine on calling this funciton, such case might 
                 * be happened.
                 */
                rv = BCM_E_NONE;
            }
            temp_vtcfp_entry_id = temp_vtcfp_ent->uni_field_id;
            temp_stat_id = temp_vtcfp_ent->uni_stat_id;
            rv = bcm_robo_field_entry_stat_detach(unit, 
                temp_vtcfp_entry_id, temp_stat_id);
            rv = bcm_robo_field_stat_destroy(unit, temp_stat_id);
            rv = bcm_field_entry_destroy(unit, temp_vtcfp_entry_id);
            if (rv == BCM_E_NOT_FOUND){
                /* in init routine on calling this funciton, such case might 
                 * be happened.
                 */
                rv = BCM_E_NONE;
            }
            
            temp_next = temp_vtcfp_ent->next;
            temp_vid = temp_vtcfp_ent->vid;
            /* 4. maintain vt_cfp sw database */
            _bcm_robo_vlan_vtcfp_sw_db_update(VTCFP_SWDB_OP_FLAG_DELETE, 
                            temp_vid, 
                            0, 0, vt_mode, 0, vtcfp_db_id, 0, 0);
                            
            /* 5. remove associated EVR entries on all ports */
            rv = DRV_VLAN_VT_DELETE
                    (unit, DRV_VLAN_XLAT_EVR, 0, temp_vid);
            if (rv){
                VT_CFP_UNLOCK;
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s: faild to remove VT entroies at vid=%d!!\n"), 
                          FUNCTION_NAME(), temp_vid));
                return rv;
            }
        }else {

            /* GNATS 41033 : 
             *  - Halt problem occurred when requesting to clear all mapping or 
             *      transparent mode VT entries if both mapping and transparent
             *      mode VT entries are existed already.
             */
            /* the node is valid but vt_mode not matched */
            temp_next = temp_vtcfp_ent->next;
        }
    }
    
    VT_CFP_UNLOCK;
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "%s,%d,Done!\n"), FUNCTION_NAME(),__LINE__));
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    return rv;
}


/* 
 * Function:
 *  _bcm_robo_vlan_vtcfp_init
 * 
 * Purpose:
 *  init process on CFP for VLAN translation usage.
 *  1. create one default CFP entry for iDT_Mode on ISP/None-ISP port.
 *      - field group and entry create
 *  2. create the default CFP group for VLAN translation.
 *      - field group created only
 *  3. init VT_CFP sw database.
 * 
 * Note : 
 *  1. this is for bcm53115 only currently.
 */
int _bcm_robo_vlan_vtcfp_init(int unit){

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    int     i, rv = BCM_E_NONE;

    bcm_field_qset_t qset;
    bcm_field_group_t group;

    if (!(SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
        return BCM_E_UNAVAIL;
    }

    /* mutex create */
    if (vt_cfp_mutex != NULL) {
        sal_mutex_destroy(vt_cfp_mutex);
        vt_cfp_mutex = NULL;
    }
    vt_cfp_mutex = sal_mutex_create("EGR_VLAN_XLATE");
    if (vt_cfp_mutex == NULL) {
        return SOC_E_INTERNAL;
    }
    
    /* clear the sw info */
    sal_memset(&idt_mode_cfp_info, 0, sizeof(idt_mode_cfp_info));
    
    if (IS_VT_CFP_INIT){
        /* if current init status is existed, reset all exsiting VT_CFP 
         *  - clear by VT_MODE
         */
        rv = _bcm_robo_vlan_vtcfp_delete_all(
                        unit, VLAN_VTMODE_TRANSPATENT);
        rv |= _bcm_robo_vlan_vtcfp_delete_all(
                        unit, VLAN_VTMODE_MAPPING);
        if (rv){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s: rv=%d,faild on reset the existing VT entries!!\n"), 
                      FUNCTION_NAME(),rv));
            return BCM_E_INIT;
        }

        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s: VT_CFP init flag reset again!\n"), 
                  FUNCTION_NAME()));
        flag_vt_cfp_init = FALSE;   /* reset the init flag */
    } else {
        
        /* processing the VT related CFP configuration */
        
        if (flag_skip_cfp_create) {
            flag_vt_cfp_init = FALSE;
        } else {
           
            /* 1. create CFP group for VLAN translation. 
             *  - qualify field at port, S-Tag and C-Tag.
             *  - no entry been created in this section.
             */
            BCM_FIELD_QSET_INIT(qset);
            BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
            BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOuterVlan);
            BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInnerVlan);
            BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyClassId);
            rv = bcm_field_group_create(unit, qset, 
                BCM_FIELD_GROUP_PRIO_ANY, &group);
            if (rv < 0) {
                return rv;
            }
    
            /* 2. save the cfp info */
            idt_mode_cfp_info.vt_cfp_group_id = group;
            
            /* 3. set CFP init flag */
            flag_vt_cfp_init = TRUE;
            
        }
    }

    /* 4. init VT_CFP_DB */
    sal_memset(&vt_cfp_db, 0, sizeof(drv_vt_cfp_db_t));
    vt_cfp_db.vt_cfp_db_start_id = VTCFP_NULL_INDEX;
    for (i = 0; i < VTCFP_SUPPORT_VLAN_CNT; i++){
        vt_cfp_db.vt_cfp_entry_db[i].nni_field_id = VTCFP_NULL_INDEX;
        vt_cfp_db.vt_cfp_entry_db[i].uni_field_id = VTCFP_NULL_INDEX;
        vt_cfp_db.vt_cfp_entry_db[i].prev = VTCFP_NULL_INDEX;
        vt_cfp_db.vt_cfp_entry_db[i].next = VTCFP_NULL_INDEX;
    }
    vt_cfp_db.vt_cfp_db_valid_count = 0;
    
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ ||SF3 */

    return BCM_E_NONE;
}
    

/*
 * Function:
 *  _bcm_robo_vlan_vtcfp_isp_change
 * 
 * Purpose:
 *  rebuild the CFP entry when isp/none-isp port changed.
 * 
 * Note : 
 *  1. this is for bcm53115 only currently.
 */
int _bcm_robo_vlan_vtcfp_isp_change(int unit){
    
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        bcm_port_t  port = 0;
        uint32      temp32;
        pbmp_t      nni_bmp, uni_bmp, pbm_mask;
        int         vtcfp_head, i, rv, idt_mode;
        drv_vt_cfp_entry_db_t   *temp_vtcfp_ent;
        
        bcm_field_entry_t   entry;
        
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s, VTCFP rebuild!\n"),FUNCTION_NAME()));
        /* check iDT_Mode first */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                         (uint32 *) &idt_mode));
        if (!idt_mode){
            LOG_INFO(BSL_LS_BCM_VLAN,
                     (BSL_META_U(unit,
                                 "%s, not in iDT_Mode!\n"),FUNCTION_NAME()));
            return BCM_E_NONE;
        }
        /* get the ISP port bitmap */
        BCM_PBMP_CLEAR(nni_bmp);
        BCM_PBMP_CLEAR(uni_bmp);
        BCM_PBMP_CLEAR(pbm_mask);
        
        port = ~port;   /* means get the device basis value */
        BCM_IF_ERROR_RETURN(
                    DRV_VLAN_PROP_PORT_ENABLE_GET
                        (unit, DRV_VLAN_PROP_ISP_PORT, 
                        port,  &temp32));
        SOC_PBMP_WORD_SET(nni_bmp, 0, temp32);

        BCM_PBMP_ASSIGN(pbm_mask, PBMP_PORT_ALL(unit));
        BCM_PBMP_ASSIGN(uni_bmp, PBMP_PORT_ALL(unit));
        BCM_PBMP_REMOVE(uni_bmp, nni_bmp);
        
        VT_CFP_LOCK;
        /* get all the created CFP entry_id to rebuild */
        vtcfp_head = vt_cfp_db.vt_cfp_db_start_id;
        for(i = vtcfp_head; IS_VALID_VTCFP_DB_ENTRY_ID(i); 
                        i = temp_vtcfp_ent->next){
            
            temp_vtcfp_ent = vt_cfp_db.vt_cfp_entry_db + i;
            /* for NNI ports */
            entry = temp_vtcfp_ent->nni_field_id;
            if ((rv = (bcm_field_qualify_InPorts(
                    unit, entry, nni_bmp, pbm_mask))) != BCM_E_NONE){
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s,%d,(err=%d) failed on field(new InPorts)!\n"),
                          FUNCTION_NAME(), __LINE__, rv));
            }
            if ((rv = (bcm_field_entry_install(unit, entry))) != BCM_E_NONE){
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s,%d,(err=%d) failed on field(Action install)!\n"),
                          FUNCTION_NAME(), __LINE__, rv));
            }
            
            /* for UNI ports */
            entry = temp_vtcfp_ent->uni_field_id;
            if ((rv = (bcm_field_qualify_InPorts(
                    unit, entry, uni_bmp, pbm_mask))) != BCM_E_NONE){
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s,%d,(err=%d) failed on field(new InPorts)!\n"),
                          FUNCTION_NAME(), __LINE__, rv));
            }
            if ((rv = (bcm_field_entry_install(unit, entry))) != BCM_E_NONE){
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s,%d,(err=%d) failed on field(Action install)!\n"),
                          FUNCTION_NAME(), __LINE__, rv));
            }
          
        }
        VT_CFP_UNLOCK;
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s, VTCFP rebuild DONE!\n"),FUNCTION_NAME()));
        
    } else {
        return BCM_E_UNAVAIL;
    }
    
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ ||SF3*/
    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_vlan_vtevr_isp_change
 * 
 * Purpose:
 *  rebuild the EVR(egress vlan remark) entry when isp/none-isp port changed.
 *  1. include the entry for None-ISP untag and VT used entries.
 * 
 * Note : 
 *  1. this is for bcm53115 only currently.
 */
int _bcm_robo_vlan_vtevr_isp_change(int unit, pbmp_t changed_pbm){
    
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        uint32  temp32 = 0;
        
        temp32 = SOC_PBMP_WORD_GET(changed_pbm, 0);
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                (unit, DRV_VLAN_PROP_EVR_VT_ISP_CHANGE, temp32));

    } else {
        return BCM_E_UNAVAIL;
    }
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ ||SF3*/
    
    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_vlan_vt_all_isp_update
 * 
 * Purpose:
 *  rebuild the CFP and EVR(egress vlan remark) entry on all ISP ports.
 * 
 * Note : 
 *  1. this is for bcm53115 and bcm53125 only currently.
 */
static int
_bcm_robo_vlan_vt_all_isp_update(int unit)
{
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    int p, dt_mode;
    pbmp_t  isp_pbm;
    
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        if (IS_VT_CFP_INIT){
            /* Rebuild the bounded CFP and EVR table */
            BCM_PBMP_CLEAR(isp_pbm);
            BCM_PBMP_ITER(PBMP_ALL(unit), p){
                BCM_IF_ERROR_RETURN(
                        bcm_robo_port_dtag_mode_get(unit, p, &dt_mode));
                if (dt_mode == BCM_PORT_DTAG_MODE_INTERNAL){
                    BCM_PBMP_PORT_ADD(isp_pbm, p);
                }
            }
            
            /* rebuild cfp for those created VT entries */
            BCM_IF_ERROR_RETURN(
                    _bcm_robo_vlan_vtcfp_isp_change(unit));
            /* rebuild evr(egress vlan remark) */
            BCM_IF_ERROR_RETURN(
                    _bcm_robo_vlan_vtevr_isp_change(unit, isp_pbm));
        }
    }

#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ ||SF3*/
    
    return BCM_E_NONE;
}
    
/* =============== VT_CFP section END =================== */

STATIC void 
_bcm_robo_vlist_ubmp_set(vlist_t **list, bcm_vlan_t vid, bcm_pbmp_t ut_pbmp)
{
    /* check if the indicated vid is valid */
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            BCM_PBMP_CLEAR((*list)->utbmp);
            BCM_PBMP_OR((*list)->utbmp, ut_pbmp);
            return;
        }
        list = &(*list)->next;
    }
}

STATIC void 
_bcm_robo_vlist_ubmp_get(vlist_t **list, bcm_vlan_t vid, bcm_pbmp_t *ut_pbmp)
{
    assert(ut_pbmp != NULL);

    /* check if the indicated vid is valid */
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            BCM_PBMP_CLEAR(*ut_pbmp);
            BCM_PBMP_OR(*ut_pbmp, (*list)->utbmp);
            return;
        }
        list = &(*list)->next;
    }
}

STATIC void 
_bcm_robo_vlist_pbmp_set(vlist_t **list, bcm_vlan_t vid, bcm_pbmp_t pbmp)
{
    /* check if the indicated vid is valid */
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            BCM_PBMP_CLEAR((*list)->pbmp);
            BCM_PBMP_OR((*list)->pbmp, pbmp);
            return;
        }
        list = &(*list)->next;
    }
}

STATIC void 
_bcm_robo_vlist_pbmp_get(vlist_t **list, bcm_vlan_t vid, bcm_pbmp_t *pbmp)
{
    assert(pbmp != NULL);

    /* check if the indicated vid is valid */
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            BCM_PBMP_CLEAR(*pbmp);
            BCM_PBMP_OR(*pbmp, (*list)->pbmp);
            return;
        }
        list = &(*list)->next;
    }
}

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)

/* valid list of EgressVlanTranslation(EVT) table */
typedef struct _bcm_robo_evt_list_s {
    int         init;               /* TRUE if inited */
    uint8       *direct_set_list;   /* each bit indicates an EVT entry */
    int         direct_set_count;   /* Number of programed EVT entry */
} _bcm_robo_evt_list_t;

/* this database is used to log the direct-confiigurred EVT entries.
 *  - bcm53115 EVT table need work with CFP to ensure the VLAN translation 
 *      can work properly. 
 *  - The egress_vt_direct_set_info is used to help user to configure EVT 
 *      table directly thus any user direct controled EVT entries will be 
 *       logged to help traverse routine to scan all those entries.
 */
static _bcm_robo_evt_list_t	evt_direct_set_info[BCM_MAX_NUM_UNITS];

#define EVTLIST_CELL_ENTRY_CNT  8   /* 8 entry bit in an EVTLIST Cell */
#define EVTLIST_CELL_ID_GET(_entry_index)  \
        ((_entry_index) / EVTLIST_CELL_ENTRY_CNT)
#define EVTLIST_CELL_BIT_LOCATION(_entry_index)  \
        ((_entry_index) % EVTLIST_CELL_ENTRY_CNT)
#define EVTLIST_ENTRY_SET(_evtlist, _entry_index)  \
        (*((_evtlist) + EVTLIST_CELL_ID_GET((_entry_index))) |= \
        ((uint8)(0x1 << EVTLIST_CELL_BIT_LOCATION((_entry_index))))) 
#define EVTLIST_ENTRY_CHK(_evtlist, _entry_index)  \
        ((*((_evtlist) + EVTLIST_CELL_ID_GET((_entry_index))) & \
        ((uint8)(0x1 << EVTLIST_CELL_BIT_LOCATION((_entry_index))))) > 0)
#define EVTLIST_ENTRY_RESET(_evtlist, _entry_index)  \
        (*((_evtlist) +EVTLIST_CELL_ID_GET((_entry_index))) &= \
        ~((uint8)(0x1 << EVTLIST_CELL_BIT_LOCATION((_entry_index))))) 

static int
_bcm_robo_egress_vt_init(int unit){
    int	evt_size = 0, byte_len = 0;
    uint8   *list = NULL;

    list = evt_direct_set_info[unit].direct_set_list;
    if (list == NULL){
        /* init the bitmap */
        evt_size = SOC_MEM_SIZE(unit, INDEX(EGRESS_VID_REMARKm));
        byte_len = EVTLIST_CELL_ID_GET(evt_size) + 
            ((EVTLIST_CELL_BIT_LOCATION(evt_size) > 0) ? 1 : 0);
        list = sal_alloc(byte_len, "EVT_valid_list");
        evt_direct_set_info[unit].direct_set_list = list;
    }
    sal_memset(list, 0, byte_len);
    evt_direct_set_info[unit].init = TRUE;
    evt_direct_set_info[unit].direct_set_count = 0;

    return BCM_E_NONE;    
}

static int
_bcm_robo_egress_vt_deinit(int unit){
    uint8   *list = NULL;

    list = evt_direct_set_info[unit].direct_set_list;
    if (list != NULL){
        sal_free(list);
    }
    evt_direct_set_info[unit].init = FALSE;
    evt_direct_set_info[unit].direct_set_count = 0;
    evt_direct_set_info[unit].direct_set_list = NULL;

    return BCM_E_NONE;
    
}
#endif	/* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/


#if QVLAN_UTBMP_BACKUP
/*
 * Function:
 *  _bcm_robo_vlist_utbmp_set
 * Purpose:
 *  set the untag portbitmap in a specific VLAN list to keep the logical VLAN 
 *      untag port bitmap.
 * Note:
 *  1. this unatag bitmap is not allowed to be set out of VLAN API scope.
 */
STATIC void 
_bcm_robo_vlist_utbmp_set(vlist_t **list, bcm_vlan_t vid, bcm_pbmp_t ut_pbmp)
{

    /* check if the indicated vid is valid */
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            break;
        }
        list = &(*list)->next;
    }

    /* overwrite the untag bitmap*/
    assert(&ut_pbmp != NULL);
    if ((*list)->vid == vid) {
        BCM_PBMP_CLEAR((*list)->ubmp);
        BCM_PBMP_OR((*list)->ubmp, ut_pbmp);
    }

}

/*
 * Function:
 *  _bcm_robo_vlist_utbmp_get
 * Purpose:
 *  get the untag portbitmap in a specific VLAN list.
 */
void 
_bcm_robo_vlist_utbmp_get(vlist_t **list, bcm_vlan_t vid, bcm_pbmp_t *ut_pbmp)
{

    /* check if the indicated vid is valid */
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            break;
        }
        list = &(*list)->next;
    }

    /* get the untag bitmap*/
    assert(ut_pbmp != NULL);
    if ((*list)->vid == vid) {
        BCM_PBMP_CLEAR(*ut_pbmp);
        BCM_PBMP_OR(*ut_pbmp, (*list)->ubmp);
    }

}

/*  
 * Function:
 *  _bcm_robo_vlan_utbmp_dt_rebuild
 * Purpose:
 *  reporgram the untag bitmap in whole 1Q_VLAN table due to the device 
 *      double tagging mode is turned on/off.
 * Note:
 *   1. each valid VLAN untag bitmap should be reprogramed by port dtag mode 
 *      (NNI or UNI) when DT_MODE is changed to enabled.
 *   2. each valid VLAN untag bitmap should be bakcuped to original bitmap 
 *      (logical bitmap) when DT_MODE is changed to disabled.
 *
 */
int _bcm_robo_vlan_utbmp_dt_rebuild(int unit, int dt_mode)
{
    vlist_t     *v;
    int         action = 0;
    pbmp_t      isp_bmp, v_bmp, temp_bmp;
    bcm_port_t  port = 0;
    bcm_vlan_t  vid = 0;
    vlan_entry_t    vt;
    uint64      temp64;
    uint32      temp32;

    /* action = 0, means restore | action = 1, means rebuild */
    action =  (dt_mode == BCM_PORT_DTAG_MODE_NONE) ? 0 : 1;

    /* get  isp_bmp when DT_MODE is enabled */
    if (dt_mode != BCM_PORT_DTAG_MODE_NONE ){
        port = ~port;
        BCM_IF_ERROR_RETURN(
                    DRV_VLAN_PROP_PORT_ENABLE_GET
                        (unit, DRV_VLAN_PROP_ISP_PORT, 
                        port,  (uint32 *)&temp64));
        soc_robo_64_val_to_pbmp(unit, &isp_bmp, temp64);
    }
    
    v = robo_vlan_info[unit].list;
    while (v != NULL) {

        if (action){
            vid = v->vid;

            /* all the vlan member port will be set as 
             * 1. UNI ports in untag bitmap are set.
             * 2. NNI ports in untag bitmap are reset.
             */
            BCM_IF_ERROR_RETURN(
                        bcm_vlan_port_get(unit, vid, &v_bmp, &temp_bmp));
            BCM_PBMP_CLEAR(temp_bmp);
            BCM_PBMP_REMOVE(v_bmp, isp_bmp);
            BCM_PBMP_OR(temp_bmp, v_bmp);
        } else {
            /* get the backup untag bitmap */
            BCM_PBMP_CLEAR(temp_bmp);
            BCM_PBMP_OR(temp_bmp, v->ubmp);
        }

        /* write to memory */
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));

        if (SOC_INFO(unit).port_num > 32) {
            soc_robo_64_pbmp_to_val(unit, &temp_bmp, &temp64);
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                            (uint32 *)&vt, (uint32 *)&temp64));
        } else {
            temp32 = SOC_PBMP_WORD_GET(temp_bmp, 0);
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                            (uint32 *)&vt, &temp32));
        }
        BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
            
        v = v->next;
    }
    
    return  BCM_E_NONE;
}
#endif

int _bcm_robo_flow2vlan_init(int unit)
{
    int table_size = SOC_MEM_SIZE(unit, INDEX(FLOW2VLANm));
    flow2vlan_entry_t f2v_t;
    uint32 tmp = 0;
    int i = 0;
    int rv = BCM_E_NONE;

    /* init FLOW2VLAN table */
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        sal_memset(&f2v_t, 0, sizeof (f2v_t));
        tmp = 0xfff;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_FLOWVLAN, DRV_MEM_FIELD_VLANID,
                        (uint32 *)&f2v_t, &tmp));
    
        for (i = 0; i < table_size; i++) {
            BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                (unit, DRV_MEM_FLOWVLAN, i, 1,
                (uint32 *)&f2v_t));
        }
    } else {
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *  _bcm_robo_vlist_insert
 * Purpose:
 *  Add a VLAN ID to a vlist
 */

STATIC int
_bcm_robo_vlist_insert(vlist_t **list, bcm_vlan_t vid)
{
    vlist_t     *v;

    if ((v = sal_alloc(sizeof (vlist_t), "vlist")) == NULL) {
        return BCM_E_MEMORY;
    }

    v->vid = vid;

    BCM_PBMP_CLEAR(v->pbmp);
    BCM_PBMP_CLEAR(v->utbmp);

#if QVLAN_UTBMP_BACKUP
    /* reset the untag bitmap when create */
    BCM_PBMP_CLEAR(v->ubmp);
#endif

#ifdef BCM_TB_SUPPORT
    v->cross_connected = 0;
#endif

    v->next = *list;
    *list = v;

    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_vlist_remove
 * Purpose:
 *  Delete a VLAN ID from a vlist
 */

STATIC int
_bcm_robo_vlist_remove(vlist_t **list, bcm_vlan_t vid)
{
    vlist_t     *v;

    while (*list != NULL) {
        if ((*list)->vid == vid) {
            v = *list;
            *list = v->next;
            sal_free(v);
            return BCM_E_NONE;
        }
        list = &(*list)->next;
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *  _bcm_robo_vlist_lookup
 * Purpose:
 *  Return TRUE if a VLAN ID exists in a vlist, FALSE otherwise
 */

STATIC int
_bcm_robo_vlist_lookup(vlist_t **list, bcm_vlan_t vid)
{
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            return TRUE;
        }
        list = &(*list)->next;
    }

    return FALSE;
}

/*
 * Function:
 *  _bcm_robo_vlist_destroy
 * Purpose:
 *  Free all memory used by a VLAN list
 */

STATIC int
_bcm_robo_vlist_destroy(vlist_t **list)
{
    while (*list != NULL) {
        BCM_IF_ERROR_RETURN(_bcm_robo_vlist_remove(list, (*list)->vid));
    }

    return BCM_E_NONE;
}

#ifdef BCM_TB_SUPPORT
/*
 * Function:
 *  _bcm_robo_vlist_cross_connect_set
 * Purpose:
 *  Set cross connect flag of the vlan
 */
STATIC int
_bcm_robo_vlist_cross_connect_set(vlist_t **list, bcm_vlan_t vid, int cc)
{
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            (*list)->cross_connected = cc;
            return TRUE;
        }
        list = &(*list)->next;
    }

    return FALSE;
}

/*
 * Function:
 *  _bcm_robo_vlist_is_cross_connect
 * Purpose:
 *  Check if a vlan is cross connected
 */
STATIC int
_bcm_robo_vlist_is_cross_connect(vlist_t **list, bcm_vlan_t vid)
{
    while (*list != NULL) {
        if ((*list)->vid == vid) {
            if ((*list)->cross_connected) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
        list = &(*list)->next;
    }

    return FALSE;
}

STATIC int
_bcm_robo_vt_list_delete_all(int unit)
{

    _robo_vlan_translate_action_delete_all(unit, VT_API_ANY, 0);
    _robo_vlan_translate_traverse_info_remove_all(unit);
    _robo_vlan_dtag_traverse_info_remove_all(unit);
    _robo_vlan_translate_action_range_traverse_info_remove_all(unit);
    _robo_vlan_translate_range_traverse_info_remove_all(unit);
    _robo_vlan_dtag_range_traverse_info_remove_all(unit);
    _robo_vlan_port_egress_default_action_delete_all(unit);

    /* Sync with soc layer, record vt disabled */
    BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                (unit, DRV_VLAN_PROP_V2V, FALSE));

    return BCM_E_NONE;
}

int
_bcm_robo_vlan_cpu_init(int unit)
{
    int rv = BCM_E_NONE;
    bcm_field_group_t evm_group;
    bcm_field_entry_t evm_entry;
    int fid = 0;

   /* Add an EVM entry for CPU port's tx */
    if (vt_evm_group_id[unit]) {
        evm_group = vt_evm_group_id[unit];
    } else {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_vlan_vm_group_create(unit, 
                BCM_ROBO_VM_EVM_GROUP, &vt_evm_group_id[unit]));
        evm_group = vt_evm_group_id[unit];
    }

    /* Create EVM entry */
    rv = bcm_field_entry_create(unit, evm_group, &evm_entry);
    if (rv < 0) {
        if (rv == BCM_E_RESOURCE) {
            rv = BCM_E_FULL;
        }
        return rv;
    }

    /* 
      * Port default actions always set Field priority to lowest.
      */
    rv = bcm_field_entry_prio_set(unit, evm_entry, BCM_FIELD_ENTRY_PRIO_LOWEST);
    BCM_IF_ERROR_RETURN(rv);

    /* EVM qualifiers */
    rv = bcm_field_qualify_InPort(unit, \
       evm_entry, CMIC_PORT(unit), VM_PORT_MASK);
    BCM_IF_ERROR_RETURN(rv);
    rv = DRV_VM_FLOW_ALLOC(unit, VM_FLOW_ID_GET_CPU_DEFAULT, &fid);
    BCM_IF_ERROR_RETURN(rv);
    rv = bcm_field_qualify_FlowId(unit, evm_entry, fid, VM_FID_MASK);
    BCM_IF_ERROR_RETURN(rv);

    /* EVM Actions */
    rv = bcm_field_action_add(unit, \
        evm_entry, bcmFieldActionOuterVlanNew, 0, BCM_FIELD_TAG_AS_RECEIVED);
    BCM_IF_ERROR_RETURN(rv);
    rv = bcm_field_action_add(unit, \
        evm_entry, bcmFieldActionInnerVlanNew, 0, BCM_FIELD_TAG_AS_RECEIVED);
    BCM_IF_ERROR_RETURN(rv);

    /* Entry install */
    rv = bcm_field_entry_install(unit, evm_entry);

    cpu_default_evm[unit] = evm_entry;

    return rv;
}

int
_bcm_robo_vlan_cpu_deinit(int unit)
{
    int rv = BCM_E_NONE;

    rv = bcm_field_entry_destroy(unit, cpu_default_evm[unit]);
    if (rv == BCM_E_INIT) {
        /* 
         * If the Field is detached before the entry destroy call,
         * the entry is destroied already and BCM_E_INIT is returned.
         * It should not be considered as an error.
         */
        rv = BCM_E_NONE;
    }

    return rv;
}
#endif

/*
 * Function:
 *  _bcm_robo_vdata_compar
 * Purpose:
 *  Internal utility routine for sorting on VLAN ID.
 */

STATIC int 
_bcm_robo_vdata_compar(void *a, void *b)
{
    bcm_vlan_data_t     *d1 = a;
    bcm_vlan_data_t     *d2 = b;

    return (d1->vlan_tag < d2->vlan_tag ? -1 :
            d1->vlan_tag > d2->vlan_tag ?  1 : 0);
}

/*
 * Function:
 *  _bcm_robo_vlan_list
 * Purpose:
 *      Main body of bcm_vlan_list() and bcm_vlan_list_by_pbmp().
 *  Assumes locking already done.
 * Parameters:
 *  list_all - if TRUE, lists all ports and ignores list_pbmp.
 *  list_pbmp - if list_all is FALSE, lists only VLANs containing
 *      any of the ports in list_pbmp.
 */

STATIC int
_bcm_robo_vlan_list(int unit, bcm_vlan_data_t **listp, int *countp,
           int list_all, pbmp_t list_pbmp)
{
    bcm_vlan_data_t *list;
    vlist_t     *v;
    int         count, i, rv;

    *countp = 0;
    *listp = NULL;

    if (!list_all && SOC_PBMP_IS_NULL(list_pbmp)) { /* Empty list */
        return BCM_E_NONE;
    }

    count = robo_vlan_info[unit].count;

    if (count == 0) {
        return BCM_E_NONE;          /* Empty list */
    }

    if ((list = sal_alloc(count * sizeof (list[0]), "vlan_list")) == NULL) {
        return BCM_E_MEMORY;
    }

    i = 0;

    for (v = robo_vlan_info[unit].list; v != NULL; v = v->next) {
        pbmp_t      pbmp, ubmp, tbmp;
        
        if ((rv = bcm_vlan_port_get(unit, v->vid, &pbmp, &ubmp)) < 0) {
            sal_free(list);
            return rv;
        }
        
        SOC_PBMP_ASSIGN(tbmp, list_pbmp);
        SOC_PBMP_AND(tbmp, pbmp);
        if (list_all || SOC_PBMP_NOT_NULL(tbmp)) {
            assert(i < count);
        
            list[i].vlan_tag = v->vid;
            BCM_PBMP_ASSIGN(list[i].port_bitmap, pbmp);
            BCM_PBMP_ASSIGN(list[i].ut_port_bitmap, ubmp);
        
            i++;
        }
    }

    assert(!list_all || i == count);  /* If list_all, make sure all listed */

    *countp = i;
    *listp = list;

    _shr_sort(*listp, *countp, sizeof (bcm_vlan_data_t), 
                    _bcm_robo_vdata_compar);

    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_vlan_default_set
 * Purpose:
 *  Main part of bcm_vlan_default_set; assumes locking already done.
 */

STATIC int
_bcm_robo_vlan_default_set(int unit, bcm_vlan_t vid)
{
    if (!_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid)) {
        return BCM_E_NOT_FOUND;
    }

    robo_vlan_info[unit].defl = vid;

    return BCM_E_NONE;
}


/*
 * Function:
 *  _bcm_robo_vlan_1st (internal)
 * Purpose:
 *  Return the first defined VLAN ID that is not the default VLAN.
 */

STATIC INLINE bcm_vlan_t
_bcm_robo_vlan_1st(int unit)
{
    vlist_t     *vl;

    for (vl = robo_vlan_info[unit].list; vl != NULL; vl = vl->next) {
        if (vl->vid != robo_vlan_info[unit].defl) {
            return vl->vid;
        }
    }

    return BCM_VLAN_INVALID;
}

extern int _drv_mstp_enable_set(int unit, int enable);

/*
 * Function:
 *  _bcm_robo_vlan_init
 * Purpose:
 *  Initialize the VLAN related tables with the default entry in it.
 * Note :
 *  1. In earily version(for bcm5338), initial routine will enable below items, 
 *      need verified if there is any side effect without these setting:
 *      - enable reserve mcast tagging
 *      - enable GMRP/GVRP tagging
 *      - Rx GVRP/GMRP packet(to CPU)
 *  2. 1Q.enable and Default Vlan Tag = 1
 *  3. No memory clear been process, we assume all entry is cleared 
 *      after power on.
 */

STATIC int 
_bcm_robo_vlan_init(int unit, bcm_vlan_data_t *vd)
{
    vlan_entry_t  ve, ve_drop;
    bcm_pbmp_t    pbm;
    uint64        fld_value;
    uint32        fld_val32;
#ifdef BCM_DINO8_SUPPORT
    uint32        reg_value = 0, modelid = 0, revid = 0;
    int           bcm5389a1 = 0;
#endif /* BCM_DINO8_SUPPORT */
    
    /* ---- clear VLAN table ---- */
    /* skipped VLAN table clear (assuming hardware did it) */
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "[vlan]: assuming hardware vlan table is cleared after power on\n")));

#ifdef BCM_DINO8_SUPPORT
    if (SOC_IS_DINO8(unit)) {
        /* Check if chip is BCM5389A1 */
        BCM_IF_ERROR_RETURN(REG_READ_MODEL_IDr
            (unit, &reg_value));
        soc_MODEL_IDr_field_get(unit, &reg_value, 
            MODELIDf, &modelid);
        BCM_IF_ERROR_RETURN(REG_READ_CHIP_REVIDr
            (unit, &reg_value));
        soc_CHIP_REVIDr_field_get(unit, &reg_value, 
            REVID_Rf, &revid);
        if ((modelid == BCM5389_A1_DEVICE_ID) &&
            (revid == BCM5389_A1_REV_ID)) {
            bcm5389a1 = 1;
        }
    }
#endif /* BCM_DINO8_SUPPORT */
    
    /* ---- set default entry into vlan table ---- */
    sal_memset(&ve, 0, sizeof (ve));
    sal_memset(&ve_drop, 0, sizeof (ve_drop));
    
    BCM_PBMP_ASSIGN(pbm, vd->ut_port_bitmap);
    BCM_PBMP_AND(pbm, PBMP_ALL(unit));
    

    if (SOC_INFO(unit).port_num > 32) {
        soc_robo_64_pbmp_to_val(unit, &vd->port_bitmap, &fld_value);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                    (uint32 *)&ve, 
                    (void *)&fld_value));
        soc_robo_64_pbmp_to_val(unit, &pbm, &fld_value);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                    (uint32 *)&ve, 
                    (void *)&fld_value));
    } else {
        fld_val32 = SOC_PBMP_WORD_GET(vd->port_bitmap, 0);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                    (uint32 *)&ve, 
                    (uint32 *)&fld_val32));
        fld_val32 = SOC_PBMP_WORD_GET(pbm, 0);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                    (uint32 *)&ve, 
                    (uint32 *)&fld_val32));
    }   
    
    if(SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
        SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit) ||
        SOC_IS_DINO16(unit)){
        fld_val32 = BCM_STG_DEFAULT;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_SPT_GROUP_ID,
                    (uint32 *)&ve, 
                    &fld_val32));
    } 

#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
    if (SOC_IS_DINO(unit)) {
        fld_val32 = 1;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_VALID,
            (uint32 *)&ve, &fld_val32));
#ifdef BCM_DINO8_SUPPORT
        if (bcm5389a1) {
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&ve_drop, &fld_val32));

            /* Create vlan 4095 with empty members to drop packets of vid=4095 */
            BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                (unit, DRV_MEM_VLAN, DINO8_A1_DROP_VLAN_DOMAIN, 1, 
                (uint32 *)&ve_drop));
        }
#endif /* BCM_DINO8_SUPPORT */
    }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */

    /* write to VLAN table */               
    /* Create default vlan */
    BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                    (unit, DRV_MEM_VLAN, BCM_VLAN_DEFAULT, 1, 
                    (uint32 *)&ve));

    if (soc_feature(unit, soc_feature_mstp)) {
        /* enable 802.1s */
        if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
            BCM_IF_ERROR_RETURN(_drv_mstp_enable_set(unit, TRUE));
        }
    }
                    
    if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        /* VLAN 0 must be created with member on all ports for SVL mode */
        fld_val32 = 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                    (uint32 *)&ve, 
                    (uint32 *)&fld_val32));
        /* Create vlan 0*/
        BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN, 0, 1, 
                        (uint32 *)&ve));
        /* set to IVL mode by default */
        BCM_IF_ERROR_RETURN(
            bcm_vlan_control_set(unit, bcmVlanShared, FALSE));
#endif  /* BCM_NORTHSTARPLUS_SUPPORT  || BCM_STARFIGHTER3_SUPPORT */
    }

    /* ---- set to QVLAN mode (enable 1Q vlan) ---- */
    /* set 1Q vlan mode */               
    BCM_IF_ERROR_RETURN(DRV_VLAN_MODE_SET
                    (unit, DRV_VLAN_MODE_TAG));
    
    /* ------- non 1Q-VLAN init -------- */
    if(SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)){
        BCM_PBMP_CLEAR(pbm);
        BCM_PBMP_ASSIGN(pbm, PBMP_ALL(unit));
        /* disable all ports' VLAN translation */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_SET
                            (unit, DRV_VLAN_PROP_V2V_PORT,pbm ,FALSE));
        /* disabled all ports' VLAN translation */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_SET
                            (unit, DRV_VLAN_PROP_MAC2V_PORT,pbm ,TRUE));
        /* disable all ports' VLAN translation */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_SET
                            (unit, DRV_VLAN_PROP_PROTOCOL2V_PORT,pbm ,TRUE));
        /* enable port based trust VLAN */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_SET
                            (unit, DRV_VLAN_PROP_TRUST_VLAN_PORT,pbm ,TRUE));
        
        /* initialize MAC VLAN */
        BCM_IF_ERROR_RETURN(bcm_vlan_mac_delete_all(unit));
    
        /* initialize Protocol VLAN (assigning port=0 but won't be used in ROBO chip)*/
        BCM_IF_ERROR_RETURN(bcm_port_protocol_vlan_delete_all(unit, 0));
        
        /* initialize FLOW2VLAN table. Value of unused entries should be 0xfff. */
        BCM_IF_ERROR_RETURN(_bcm_robo_flow2vlan_init(unit));
        
        /* inittialize VLAN2VLAN table.
         * - special reset routine for performance issue. Check the detail 
         *  in service driver.
         */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_V2V_INIT, TRUE));
        
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    } else if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int rv = BCM_E_NONE;

        /* init SW list for the log of user direct configurred EgressVidRemark table */
        BCM_IF_ERROR_RETURN(_bcm_robo_egress_vt_init(unit));
        
        /* init the bcm53115 VT requirred SW information or re-init 
         * existing VT related CFP configuration. 
         */
        flag_skip_cfp_create = TRUE;
        rv = _bcm_robo_vlan_vtcfp_init(unit);
        flag_skip_cfp_create = FALSE;
        BCM_IF_ERROR_RETURN(rv);

        /* clear all EVR table entry on each port. and the related 
         *      port basis database.
         */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));

        /* reset the iDT_Mode */ 
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, FALSE));
        
        flag_vt_cfp_init = FALSE;
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/                    
#ifdef BCM_TB_SUPPORT  
    } else if (SOC_IS_TBX(unit)) {
        int i = 0;

        for (i = 0; i < BCM_MAX_NUM_UNITS; i++) {
            vt_ivm_group_id[i] = 0;
            vt_evm_group_id[i] = 0;
            cpu_default_evm[i] = 0;
        }

        /* Release allocated VT information if any. */
        _bcm_robo_vt_list_delete_all(unit);

        /* 
          * Configuration for initial state(compatible with former chips) 
          * 1. IVL mode.
          * 2. Copy packets with unregistered Vlan to cpu.
          * 3. Vlan filter bypass.
          *    -imp port's ingress pkts
          *    -known multicast
          *    -reserved multicast
          *    -user defined addresses
          */
        /* IVL mode. */
        BCM_IF_ERROR_RETURN(
            bcm_vlan_control_set(unit, bcmVlanShared, FALSE));
        /* Copy packets with unregistered Vlan to cpu */
        BCM_IF_ERROR_RETURN(
            bcm_vlan_control_set(unit, bcmVlanUnknownToCpu, TRUE));
        /* Bypass options */
        BCM_IF_ERROR_RETURN(
            bcm_vlan_control_set(unit, bcmVlanBypassMiim, TRUE));
        BCM_IF_ERROR_RETURN(
            bcm_vlan_control_set(unit, bcmVlanBypassMcast, TRUE));
        BCM_IF_ERROR_RETURN(
            bcm_vlan_control_set(unit, bcmVlanBypassRsvdMcast, TRUE));
        BCM_IF_ERROR_RETURN(
            bcm_vlan_control_set(unit, bcmVlanBypassL2UserAddr, TRUE));

#endif  /* BCM_TB_SUPPORT */                         
    }
    
#if defined(BCM_53101)
    if (SOC_IS_LOTUS(unit)) {
        int j = 0;
        
        for (j = 0; j < BCM_UNITS_MAX; j++) {
            vlan_policer_id[j] = 0;
        }
    }
#endif /* BCM_53101 */

    return BCM_E_NONE;

}

int bcm_robo_vlan_init(int unit)
{
    bcm_vlan_data_t     vd;
    char *s;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_init()..\n")));

    s = soc_property_get_str(unit, "board_name");
    if((s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        if (SOC_IS_TBX(unit)) {
            /* 
              * This should be done by chip reset routine in soc/robo/drv.c 
              * Since chip reset is not available at FPGA stage, do it here.
              */
            BCM_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_VLAN));
        }
    }

    sal_memset(&vd, 0, sizeof (vd));

   /*
     * Initialize hardware tables
     */
    vd.vlan_tag = BCM_VLAN_DEFAULT;
    BCM_PBMP_ASSIGN(vd.port_bitmap, PBMP_ALL(unit));
    BCM_PBMP_ASSIGN(vd.ut_port_bitmap, PBMP_ALL(unit));
    BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_CMIC(unit));

    /*
     * A compile-time application policy may prefer to not add
     * Ethernet or CPU ports to the default VLAN.
     */
#ifdef  BCM_VLAN_NO_DEFAULT_ETHER
    BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_E_ALL(unit));
    BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_E_ALL(unit));
#endif  /* BCM_VLAN_NO_DEFAULT_ETHER */
#ifdef  BCM_VLAN_NO_DEFAULT_CPU
    BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_CMIC(unit));
#endif  /* BCM_VLAN_NO_DEFAULT_CPU */

    /*
     * Init VLAN related hw reg/mem.
     */    
    BCM_IF_ERROR_RETURN(_bcm_robo_vlan_init(unit, &vd));

    /*
     * Initialize software structures
     */
    robo_vlan_info[unit].defl = BCM_VLAN_DEFAULT;    

    /* In case bcm_vlan_init is called more than once */
    BCM_IF_ERROR_RETURN(
        _bcm_robo_vlist_destroy(&robo_vlan_info[unit].list));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_vlist_insert(&robo_vlan_info[unit].list, vd.vlan_tag));

    _bcm_robo_vlist_pbmp_set(&robo_vlan_info[unit].list, vd.vlan_tag, vd.port_bitmap);
    _bcm_robo_vlist_ubmp_set(&robo_vlan_info[unit].list, vd.vlan_tag, vd.ut_port_bitmap);

#if QVLAN_UTBMP_BACKUP
    /* backup the untage bitmap */
    _bcm_robo_vlist_utbmp_set(&robo_vlan_info[unit].list, vd.vlan_tag, vd.ut_port_bitmap);
    
#endif
    robo_vlan_info[unit].count = 1;
    robo_vlan_info[unit].init = TRUE;

    BCM_IF_ERROR_RETURN
        (_bcm_robo_vlan_flood_default_set(unit,
                                         BCM_VLAN_MCAST_FLOOD_UNKNOWN));
    return BCM_E_NONE;

}

/*
 * Function:
 *      bcm_robo_vlan_detach
 * Purpose:
 *      De-initialize the VLAN module.
 * Parameters:
 *      unit - (IN) BCM device number.
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_robo_vlan_detach(int unit)
{
#ifdef BCM_TB_SUPPORT
    int rv = BCM_E_NONE;

    if (SOC_IS_TBX(unit)) {
        _bcm_robo_vt_list_delete_all(unit);

        BCM_IF_ERROR_RETURN(
            DRV_VM_DEINIT(unit));

        if (vt_ivm_group_id[unit]) {
            rv = bcm_field_group_destroy(unit, vt_ivm_group_id[unit]);
            if (rv == BCM_E_INIT) {
                /* 
                 * If the Field is detached before the group destroy call,
                 * the group is destroied already and BCM_E_INIT is returned.
                 * It should not be considered as an error.
                 */
                rv = BCM_E_NONE;
            }
            BCM_IF_ERROR_RETURN(rv);

            vt_ivm_group_id[unit] = 0;
        }

        if (vt_evm_group_id[unit]) {
            rv = bcm_field_group_destroy(unit, vt_evm_group_id[unit]);
            if (rv == BCM_E_INIT) {
                /* 
                 * If the Field is detached before the group destroy call,
                 * the group is destroied already and BCM_E_INIT is returned.
                 * It should not be considered as an error.
                 */
                rv = BCM_E_NONE;
            }
            BCM_IF_ERROR_RETURN(rv);

            vt_evm_group_id[unit] = 0;
        }
    }
#endif
    
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_egress_vt_deinit(unit));
    }
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/

    /* Release vlist */
    BCM_IF_ERROR_RETURN(
        _bcm_robo_vlist_destroy(&robo_vlan_info[unit].list));

    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_vlan_create
 * Purpose:
 *  Main body of bcm_vlan_create; assumes locking already done;
 */

STATIC int
_bcm_robo_vlan_create(int unit, bcm_vlan_t vid)
{
    bcm_stg_t       stg_defl;
    int             rv;
    vlan_entry_t    vt;
    uint64          field_value;
    uint32          field_val32;

    sal_memset(&vt, 0, sizeof (vt));

    rv = bcm_stg_default_get(unit, &stg_defl);
    if (rv == BCM_E_UNAVAIL) {
        stg_defl = -1;
    } else if (rv < 0) {
        return rv;
    }

    if (_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid)) {
        return BCM_E_EXISTS;
    }

    /* clear all port bitmap for creating process */
    if (SOC_INFO(unit).port_num > 32) {
        COMPILER_64_ZERO(field_value);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (void *)&field_value));

        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (void *)&field_value));
    } else {
        field_val32 = 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (uint32 *)&field_val32));

        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (uint32 *)&field_val32));
    }
    
    if(SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
        SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_TBX(unit) ||
        SOC_IS_DINO16(unit)) {
        field_val32 = BCM_STG_DEFAULT;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_SPT_GROUP_ID,
                    (uint32 *)&vt, (uint32 *)&field_val32));
    }

#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
    if (SOC_IS_DINO(unit)) {
        field_val32 = 1;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_VALID,
            (uint32 *)&vt, (uint32 *)&field_val32));
    }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_flood_default_get(unit, &field_val32));
        if (field_val32 == BCM_VLAN_MCAST_FLOOD_UNKNOWN) {
            field_val32 = 0;
        } else if (field_val32 == BCM_VLAN_MCAST_FLOOD_NONE) {
            field_val32 = 1;
        } else {
            return BCM_E_INTERNAL;
        }
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_MCAST_DROP,
                    (uint32 *)&vt, 
                    (uint32 *)&field_val32));    
    }
#endif

    BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                    (unit, DRV_MEM_VLAN, 
                    (uint32)vid, 1, (uint32 *)&vt));

    BCM_IF_ERROR_RETURN
    (_bcm_robo_vlist_insert(&robo_vlan_info[unit].list, vid));

    robo_vlan_info[unit].count++;

    if (stg_defl >= 0) {
        /* Must be after vlist insert */
        BCM_IF_ERROR_RETURN
            (bcm_stg_vlan_add(unit, stg_defl, vid));
    }

    return BCM_E_NONE;
}
    
/*
 * Function:
 *  bcm_robo_vlan_create
 * Purpose:
 *  Allocate and configure a VLAN on RoboSwitch.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      vid - VLAN ID to create.
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_INTERNAL - Chip access failure.
 *  BCM_E_EXISTS - VLAN ID already in use.
 * Notes:
 *  VLAN is placed in the default STG and can be reassigned later.
 */
int bcm_robo_vlan_create(int unit, bcm_vlan_t vid)
{
    int         rv;

    CHECK_INIT(unit);
    CHECK_VID(unit, vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_create()..\n")));
    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_create(unit, vid);
    BCM_UNLOCK(unit);

    return rv;
}               



/*
 * Function:
 *  _bcm_robo_vlan_destroy
 * Purpose:
 *  Main body of bcm_vlan_destroy; assumes locking done.
 */

STATIC int
_bcm_robo_vlan_destroy(int unit, bcm_vlan_t vid)
{
    bcm_stg_t       stg;
    int             rv;
    vlan_entry_t    vt;

    /* Cannot destroy default VLAN */
    if (vid == robo_vlan_info[unit].defl) {
        return BCM_E_BADID;
    }

    if (!_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid)) {
        return BCM_E_NOT_FOUND;
    }

    /* Remove VLAN from its spanning tree group */
    BCM_IF_ERROR_RETURN(bcm_vlan_stg_get
                    (unit, vid, &stg));

    rv = _bcm_robo_stg_vlan_destroy(unit, stg, vid);
    if (rv < 0 && rv != BCM_E_UNAVAIL) {
        return rv;
    }   

    sal_memset(&vt, 0, sizeof(vlan_entry_t));
    /* set this VLAN entry been invalid */   
    if ((rv = DRV_MEM_WRITE
                (unit, DRV_MEM_VLAN, 
                (uint32)vid, 1, (uint32 *)&vt)) < 0){
        return rv;
    }
    
    BCM_IF_ERROR_RETURN(_bcm_robo_vlist_remove
                    (&robo_vlan_info[unit].list, vid));

    robo_vlan_info[unit].count--;

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        /* Vlan is destroyed and members are erased. */
        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_update(unit, vid));
    }
#endif

    return BCM_E_NONE;
}
    
/*
 * Function:
 *  bcm_robo_vlan_destroy
 * Purpose:
 *  Deallocate VLAN from RoboSwitch.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      vid - VLAN ID to affect.
 * Returns:
 *  BCM_E_NONE      Success.
 *  BCM_E_INTERNAL      Chip access failure.
 *  BCM_E_BADID           Cannot remove default VLAN
 *  BCM_E_NOT_FOUND VLAN ID not in use.
 * Notes:
 *  None.
 */
int 
bcm_robo_vlan_destroy(int unit, bcm_vlan_t vid)
{
    int     rv;

    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_destroy()..\n")));
    CHECK_VID(unit, vid);

    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_destroy(unit, vid);
    BCM_UNLOCK(unit);

    return rv;
}               
    
/*
 * Function:
 *  bcm_robo_vlan_destroy_all
 * Purpose:
 *  Destroy all VLANs except the default VLAN
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *  None.
 */
int 
bcm_robo_vlan_destroy_all(int unit)
{
    int             rv = BCM_E_NONE;
    bcm_vlan_t      vid;

    CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_destroy_all()..\n")));
    BCM_LOCK(unit);

    while ((vid = _bcm_robo_vlan_1st(unit)) != BCM_VLAN_INVALID) {
        if ((rv = bcm_vlan_destroy(unit, vid)) < 0) {
            break;
        }
    }

    BCM_UNLOCK(unit);

    return rv;
}               

/*
 * Function:
 *  _bcm_robo_vlan_port_get
 * Purpose:
 *  Read the port bitmap from a VLAN_TAB entry.
 */
int
_bcm_robo_vlan_port_get(int unit, bcm_vlan_t vid, pbmp_t *pbmp, pbmp_t *ubmp)
{
    /* Check if this VLAN is exist */
    if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
        return BCM_E_NOT_FOUND;
    }
    
    _bcm_robo_vlist_pbmp_get(&robo_vlan_info[unit].list, vid, pbmp);
    _bcm_robo_vlist_ubmp_get(&robo_vlan_info[unit].list, vid, ubmp);

    return BCM_E_NONE;
}

    
/*
 * Function:
 *  _bcm_robo_vlan_port_add
 * Purpose:
 *  Main part of bcm_robo_vlan_port_add; assumes locking already done.
 */

STATIC int
_bcm_robo_vlan_port_add(int unit, bcm_vlan_t vid, pbmp_t pbmp, pbmp_t ubmp)
{
    pbmp_t          vlan_pbmp, vlan_ubmp;
    vlan_entry_t    vt;
    uint64          field_value, temp64;
    pbmp_t      backup_utbmp, backup_pbmp;
    uint32          field_val32;
    
#if QVLAN_UTBMP_BACKUP
    pbmp_t      backup_ubmp, tag_bmp;
    int         dt_mode;
    bcm_port_t  isp_port;
#endif

    /* Check if this VLAN is exist */
    if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
        return BCM_E_NOT_FOUND;
    }

    /* No such thing as untagged CPU */
    BCM_PBMP_REMOVE(ubmp, PBMP_CMIC(unit));

    BCM_PBMP_CLEAR(vlan_pbmp);    
    BCM_PBMP_CLEAR(vlan_ubmp);    
    BCM_IF_ERROR_RETURN(_bcm_robo_vlan_port_get(unit, vid,
                           &vlan_pbmp, &vlan_ubmp));

    /* Remove the original untagged attribute of the modified port */
    BCM_PBMP_REMOVE(vlan_ubmp, pbmp);

    BCM_PBMP_OR(pbmp, vlan_pbmp);
    /* Apply the new untagged setting of the modified port */
    BCM_PBMP_OR(ubmp, vlan_ubmp);
    
    /* Only allow untagged ports belong to portbitmap on the vlan. */
    /* allow for making the port tagged/untagged if its already added to the vlan */    
    BCM_PBMP_AND(ubmp, pbmp);

    BCM_PBMP_CLEAR(backup_pbmp);
    BCM_PBMP_ASSIGN(backup_pbmp, pbmp);

    BCM_PBMP_CLEAR(backup_utbmp);
    BCM_PBMP_ASSIGN(backup_utbmp, ubmp);
    BCM_PBMP_REMOVE(backup_utbmp, PBMP_CMIC(unit));

#if QVLAN_UTBMP_BACKUP
    /* backup untag bitmap */
    BCM_PBMP_CLEAR(backup_ubmp);
    BCM_PBMP_OR(backup_ubmp, ubmp);
    BCM_PBMP_REMOVE(backup_ubmp, PBMP_CMIC(unit));

    /* get the tagged bitmap */
    BCM_PBMP_CLEAR(tag_bmp);
    BCM_PBMP_OR(tag_bmp, pbmp);
    
    /* get device DT_MODE :
     *  - currently in ROBO chips(bcm5348/53242), if a port dtag mode was get 
     *      as "BCM_PORT_DTAG_MODE_NONE" value means the device double 
     *      tagging mode is turned off.
     */
    BCM_IF_ERROR_RETURN(bcm_port_dtag_mode_get(unit, 0, &dt_mode));
#endif

    BCM_PBMP_REMOVE(ubmp, PBMP_CMIC(unit));
    
    sal_memset(&vt, 0, sizeof (vt));
    BCM_IF_ERROR_RETURN(DRV_MEM_READ
                    (unit, DRV_MEM_VLAN, 
                    (uint32)vid, 1, (uint32 *)&vt));

    if (SOC_INFO(unit).port_num > 32) {
        COMPILER_64_ZERO(field_value);                    
        soc_robo_64_pbmp_to_val(unit, &pbmp, &temp64);
        COMPILER_64_OR(field_value, temp64);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (void *)&field_value));
                        
        /* process ut_pbmp */
#if QVLAN_UTBMP_BACKUP
        COMPILER_64_ZERO(field_value);
        if (dt_mode != BCM_PORT_DTAG_MODE_NONE){    /* dt_mode is enabled */
            /* ------ special process when DT_MODE is enabled ------ 
             *      1. NNI port must be excluded in untag bitmap.
             *      2. UNI port must be existed in untag bitmap.
             */
            BCM_PBMP_CLEAR(ubmp);
            BCM_PBMP_ITER(tag_bmp, isp_port){
                if (IS_CPU_PORT(unit, isp_port)){
                    continue;
                }
                BCM_IF_ERROR_RETURN(
                            bcm_port_dtag_mode_get(unit, isp_port, &dt_mode));
                switch (dt_mode){
                case BCM_PORT_DTAG_MODE_INTERNAL :      /* NNI port */
                    BCM_PBMP_PORT_REMOVE(ubmp, isp_port);
                    break;
                case BCM_PORT_DTAG_MODE_EXTERNAL :      /* UNI port */
                    BCM_PBMP_PORT_ADD(ubmp, isp_port);
                    break;
                default :
                    continue;
                    break;
                }
            }   /* BCM_PBMP_ITER(tag_bmp) */
        } else {
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                            (uint32 *)&vt, (void *)&field_value));
         }
#endif
        COMPILER_64_ZERO(field_value);                    
        soc_robo_64_pbmp_to_val(unit, &ubmp, &temp64);
        COMPILER_64_OR(field_value, temp64);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (void *)&field_value));
    } else {
        field_val32 = SOC_PBMP_WORD_GET(pbmp, 0);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (uint32 *)&field_val32));
                        
        /* process ut_pbmp */
#if QVLAN_UTBMP_BACKUP
        field_val32 = 0;
        if (dt_mode != BCM_PORT_DTAG_MODE_NONE){    /* dt_mode is enabled */
            /* ------ special process when DT_MODE is enabled ------ 
             *      1. NNI port must be excluded in untag bitmap.
             *      2. UNI port must be existed in untag bitmap.
             */
            BCM_PBMP_CLEAR(ubmp);
            BCM_PBMP_ITER(tag_bmp, isp_port){
                if (IS_CPU_PORT(unit, isp_port)){
                    continue;
                }
                BCM_IF_ERROR_RETURN(
                            bcm_port_dtag_mode_get(unit, isp_port, &dt_mode));
                switch (dt_mode){
                case BCM_PORT_DTAG_MODE_INTERNAL :      /* NNI port */
                    BCM_PBMP_PORT_REMOVE(ubmp, isp_port);
                    break;
                case BCM_PORT_DTAG_MODE_EXTERNAL :      /* UNI port */
                    BCM_PBMP_PORT_ADD(ubmp, isp_port);
                    break;
                default :
                    break;
                }
            }   /* BCM_PBMP_ITER(tag_bmp) */
            
        } else {
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                            (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                            (uint32 *)&vt, (void *)&field_value));
         }
#endif
        field_val32 = SOC_PBMP_WORD_GET(ubmp, 0);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (uint32 *)&field_val32));
    }
                    
    BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                    (unit, DRV_MEM_VLAN, 
                    (uint32)vid, 1, (uint32 *)&vt));

    _bcm_robo_vlist_pbmp_set(&robo_vlan_info[unit].list, vid, backup_pbmp);
    _bcm_robo_vlist_ubmp_set(&robo_vlan_info[unit].list, vid, backup_utbmp);

#if QVLAN_UTBMP_BACKUP
    /* ------ backup untag bitmap ------ */
    _bcm_robo_vlist_utbmp_set(&robo_vlan_info[unit].list, vid, backup_ubmp);
    /* ------ */
#endif
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (BCM_PBMP_NOT_NULL(pbmp)) {
            /* New ports are added. */
            BCM_IF_ERROR_RETURN(
                _robo_vlan_translate_action_update(unit, vid));
        }
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_vlan_port_add
 * Purpose:
 *      Add ports to the specified vlan.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      vid - VLAN ID to add port to as a member.
 *      pbmp - port bitmap for members of VLAN
 *      ubmp - untagged members of VLAN
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 *      BCM_E_NOT_FOUND - VLAN ID not in use.
 */
int 
bcm_robo_vlan_port_add(int unit,
                 bcm_vlan_t vid,
                 bcm_pbmp_t pbmp, bcm_pbmp_t ubmp)
{
    int         rv;

    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_port_add()..\n")));
    CHECK_VID(unit, vid);

    BCM_IF_ERROR_RETURN(_bcm_robo_pbmp_check(unit, pbmp));
    BCM_IF_ERROR_RETURN(_bcm_robo_pbmp_check(unit, ubmp));

    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_port_add(unit, vid, pbmp, ubmp);
    BCM_UNLOCK(unit);

    return rv;
}               

/*
 * Function:
 *  _bcm_robo_vlan_port_remove
 * Purpose:
 *  Main part of bcm_vlan_port_remove; assumes locking already done.
 */
static int
_bcm_robo_vlan_port_remove(int unit, bcm_vlan_t vid, pbmp_t pbmp)
{
    pbmp_t          vlan_pbmp, vlan_ubmp;
    vlan_entry_t    vt;
    uint64          field_value, temp64;
    uint32          field_val32, temp32;
    pbmp_t      backup_utbmp, backup_pbmp;
    
#if QVLAN_UTBMP_BACKUP
    pbmp_t      temp_bmp;
    int         dt_mode;
    bcm_port_t  c_port;     /* customer port */
#endif

    /* Check if this VLAN is exist */
    if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
        return BCM_E_NOT_FOUND;
    }

    BCM_PBMP_CLEAR(vlan_pbmp);
    BCM_PBMP_CLEAR(vlan_ubmp);	

    /* Don't remove ports that are not there */
    BCM_IF_ERROR_RETURN(_bcm_robo_vlan_port_get(unit, vid,
                           &vlan_pbmp, &vlan_ubmp));

    BCM_PBMP_AND(pbmp, vlan_pbmp);

    BCM_PBMP_CLEAR(backup_pbmp);
    BCM_PBMP_CLEAR(backup_utbmp);
    BCM_PBMP_ASSIGN(backup_pbmp, vlan_pbmp);
    BCM_PBMP_ASSIGN(backup_utbmp, vlan_ubmp);
    BCM_PBMP_REMOVE(backup_pbmp, pbmp);
    BCM_PBMP_REMOVE(backup_utbmp, pbmp);

    /* Remove ports from the VLAN bitmap in a VLAN_TAB entry.*/
    BCM_IF_ERROR_RETURN(DRV_MEM_READ
                    (unit, DRV_MEM_VLAN, 
                    (uint32)vid, 1, (uint32 *)&vt));

#if QVLAN_UTBMP_BACKUP
    /* get DT_MODE */
    BCM_IF_ERROR_RETURN(
                bcm_port_dtag_mode_get(unit, 0, &dt_mode));

    /* kept the remove bitmap */
    BCM_PBMP_CLEAR(temp_bmp);
    BCM_PBMP_OR(temp_bmp, pbmp);
    
#endif

    if (SOC_INFO(unit).port_num > 32) {
        COMPILER_64_ZERO(field_value);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (void *)&field_value));

        /* set new port bitmap */
        soc_robo_64_pbmp_to_val(unit, &pbmp, &temp64);
        COMPILER_64_NOT(temp64);
        COMPILER_64_AND(field_value, temp64);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (void *)&field_value));

#if QVLAN_UTBMP_BACKUP
        if (dt_mode == BCM_PORT_DTAG_MODE_NONE){
            /* 1. assuming that the untag bitmap from real memory read is well 
             *      processed by the rule of NNI/UNI port mode setting. 
             * 2. based on the assumptions above, here we can exclude the port  
             *      which was set as UNI Port and is existed in this removed 
             *      port bitmap. (UNI port must not been 0 in untag_bitmap.)
             * 3. after this process
             */
            BCM_PBMP_CLEAR(temp_bmp);
            BCM_PBMP_OR(temp_bmp, pbmp);
            
            BCM_PBMP_ITER(temp_bmp, c_port){
                if (IS_CPU_PORT(unit, c_port)){
                    continue;
                }
                BCM_IF_ERROR_RETURN(
                            bcm_port_dtag_mode_get(unit, c_port, &dt_mode));
                switch (dt_mode){
                case BCM_PORT_DTAG_MODE_EXTERNAL :      /* UNI port */
                    BCM_PBMP_PORT_REMOVE(pbmp, c_port);
                    break;
                default :
                    continue;
                    break;
                }
            }   /* BCM_PBMP_ITER(pbmp) */
        }
#endif

        /* get/set new untag pbmp */
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (void *)&field_value));
        soc_robo_64_pbmp_to_val(unit, &pbmp, &temp64);
        COMPILER_64_NOT(temp64);
        COMPILER_64_AND(field_value, temp64);
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (void *)&field_value));
    } else {
        field_val32 = 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (uint32 *)&field_val32));

        /* set new port bitmap */
        temp32 = SOC_PBMP_WORD_GET(pbmp, 0);
        field_val32 &= ~temp32;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP,
                        (uint32 *)&vt, (uint32 *)&field_val32));
                        
        /* get/set new untag pbmp */
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (uint32 *)&field_val32));
        field_val32 &= ~temp32;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG,
                        (uint32 *)&vt, (uint32 *)&field_val32));
    }

    BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                    (unit, DRV_MEM_VLAN, 
                    (uint32)vid, 1, (uint32 *)&vt));
    
    _bcm_robo_vlist_pbmp_set(&robo_vlan_info[unit].list, vid, backup_pbmp);
    _bcm_robo_vlist_ubmp_set(&robo_vlan_info[unit].list, vid, backup_utbmp);
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (BCM_PBMP_NOT_NULL(pbmp)) {
            /* Ports are removed. */
            BCM_IF_ERROR_RETURN(
                _robo_vlan_translate_action_update(unit, vid));
        }
    }
#endif
    
    return BCM_E_NONE;
}

                 
/*
 * Function:
 *  bcm_robo_vlan_port_remove
 * Purpose:
 *      Remove ports from a specified vlan.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      vid - VLAN ID to remove port(s) from.
 *      pbmp - port bitmap for ports to remove.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 *      BCM_E_NOT_FOUND - VLAN ID not in use.
 * Notes:
 *      None.
 */
int 
bcm_robo_vlan_port_remove(int unit,
                bcm_vlan_t vid,
                bcm_pbmp_t pbmp)
{
    int         rv;

    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_port_remove()..\n")));
    CHECK_VID(unit, vid);

    BCM_IF_ERROR_RETURN(_bcm_robo_pbmp_check(unit, pbmp));

    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_port_remove(unit, vid, pbmp);
    BCM_UNLOCK(unit);

    return rv;
}               
                
/*
 * Function:
 *  bcm_robo_vlan_port_get
 * Purpose:
 *      Retrieves a list of the member ports of an existing VLAN.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      vid - VLAN ID to lookup.
 *      tag_pbmp - (output) member port bitmap (ignored if NULL)
 *      untag_pbmp - (output) untagged port bitmap (ignored if NULL)
 * Returns:
 *      BCM_E_NONE - Success (port bitmaps filled in).
 *      BCM_E_INTERNAL - Chip access failure.
 *      BCM_E_NOT_FOUND - No such VLAN defined.
 */

int 
bcm_robo_vlan_port_get(int unit,
                 bcm_vlan_t vid,
                 bcm_pbmp_t *pbmp, bcm_pbmp_t *ubmp)
{
    int         rv;

    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_port_get()..\n")));
    CHECK_VID(unit, vid);

    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_port_get(unit, vid, pbmp, ubmp);
    BCM_UNLOCK(unit);

    return rv;
}               
                 

/*
 * Function:
 *  bcm_robo_vlan_list
 * Purpose:
 *      Returns an array of all defined VLANs and their port bitmaps.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *      listp - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero VLANs defined.
 *      countp - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero VLANs defined.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 *      BCM_E_MEMORY - Out of system memory.
 * Notes:
 *  The caller is responsible for freeing the memory that is
 *  returned, using bcm_vlan_list_destroy.
 */
int 
bcm_robo_vlan_list(int unit, bcm_vlan_data_t **listp, int *countp)
{
    int     rv;
    pbmp_t  empty_pbm;

    CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_list()..\n")));
    SOC_PBMP_CLEAR(empty_pbm);
    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_list(unit, listp, countp, TRUE, empty_pbm);
    BCM_UNLOCK(unit);

    return rv;
}               

/*
 * Function:
 *  bcm_robo_vlan_list_by_pbmp
 * Purpose:
 *      Returns an array of defined VLANs and port bitmaps.
 *  Only VLANs that containing any of the specified ports are listed.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *  pbmp - Bitmap of ports
 *      listp - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero VLANs defined.
 *      countp - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero VLANs defined.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_INTERNAL          Chip access failure.
 *      BCM_E_MEMORY            Out of system memory.
 * Notes:
 *  The caller is responsible for freeing the memory that is
 *  returned, using bcm_vlan_list_destroy.
 */

int 
bcm_robo_vlan_list_by_pbmp(int unit, pbmp_t pbmp,
                 bcm_vlan_data_t **listp, int *countp)
{
    int     rv;

    CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_list_by_pbmp()..\n")));
    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_list(unit, listp, countp, FALSE, pbmp);
    BCM_UNLOCK(unit);

    if (BCM_SUCCESS(rv)) {
        _shr_sort(*listp, *countp, sizeof (bcm_vlan_data_t), 
                _bcm_robo_vdata_compar);
    }

    return rv;
}               
                 
/*
 * Function:
 *  bcm_robo_vlan_list_destroy
 * Purpose:
 *      Destroy a list returned by bcm_vlan_list.
 *      Also works for the zero-VLAN case (NULL list).
 * Parameters:
 *      unit - RoboSwitch PCI device unit number
 *      list - List returned by bcm_vlan_list
 *      count - Count returned by bcm_vlan_count
 * Returns:
 *      BCM_E_NONE              Success.
 * Notes:
 *      None.
 */
int 
bcm_robo_vlan_list_destroy(int unit, bcm_vlan_data_t *list, int count)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_list_destroy()..\n")));
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(count);

    if (list != NULL) {
        sal_free(list);
    }

    return BCM_E_NONE;
}               
    

/*
 * Function:
 *  bcm_robo_vlan_default_get
 * Purpose:
 *  Get the default VLAN ID
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  vid_ptr - (OUT) Target to receive the VLAN ID.
 * Returns:
 *  BCM_E_NONE - Success.
 */
int 
bcm_robo_vlan_default_get(int unit, bcm_vlan_t *vid_ptr)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_default_get()..\n")));
    *vid_ptr = robo_vlan_info[unit].defl;

    return BCM_E_NONE;
}               

/*
 * Function:
 *  bcm_robo_vlan_default_set
 * Purpose:
 *  Set the default VLAN ID
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  vid - The new default VLAN
 * Returns:
 *  BCM_E_NONE - Success.
 * Notes:
 *  The new default VLAN must already exist.
 */
int 
bcm_robo_vlan_default_set(int unit, bcm_vlan_t vid)
{
    int         rv;

    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_default_set()..\n")));
    CHECK_VID(unit, vid);

    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_default_set(unit, vid);
    BCM_UNLOCK(unit);

    return rv;
}               
    

/*
 * Function:
 *  bcm_robo_vlan_stg_get
 * Purpose:
 *  Retrieve the VTABLE STG for the specified vlan
 * Parameters:
 *  unit - RoboSwitch PCI device unit number.
 *      vid - VLAN ID.
 *  stg_ptr - (OUT) Pointer to store stgid for return.
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_vlan_stg_get(int unit, bcm_vlan_t vid, bcm_stg_t *stg_ptr)
{
    uint32          field_value = 0;

    CHECK_INIT(unit);
    CHECK_VID(unit, vid);

    /* Check if this VLAN is exist */
    if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
        return BCM_E_NOT_FOUND;
    }

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_stg_get()..\n")));
    
    /* Upper layer already checks that vid is valid */
    BCM_IF_ERROR_RETURN(DRV_MSTP_CONFIG_GET
                (unit, (uint32)vid, &field_value));
    
    *stg_ptr = field_value;

    return BCM_E_NONE;
}               

/*
 * Function:
 *  _bcm_robo_vlan_mstp_config_set
 * Purpose:
 *  Update the VTABLE STG for the specified vlan
 * Parameters:
 *  unit  - RoboSwitch PCI device unit number
 *      vid - VLAN ID
 *  stg - New spanning tree group number for VLAN
 * Returns:
 *  BCM_E_XXX
 */
int 
_bcm_robo_vlan_mstp_config_set(int unit, bcm_vlan_t vid, bcm_stg_t stg)
{
    int     rv;
    bcm_stg_t *stg_list;
    int     stg_count, i, stg_existed = 0;
    bcm_stg_t  stg_max = 0;

    CHECK_INIT(unit);
    CHECK_VID(unit, vid);

    /* Check STG id */
    if (!soc_feature(unit, soc_feature_mstp)) {
        stg_max = 1;
    } else {
        BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_MSTP_NUM, (uint32 *)&stg_max));
    }

    if (stg < 0 || stg > stg_max) {
        return BCM_E_BADID;
    }

    /* Check if this VLAN is exist */
    if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
        return BCM_E_NOT_FOUND;
    }

    /* Check if this STG is exist */
    BCM_IF_ERROR_RETURN(bcm_stg_list(unit, &stg_list, &stg_count));
    for (i = 0; i < stg_count; i++) {
        if (stg_list[i] == stg) {
            stg_existed = 1;
            break;
        }
    }
    BCM_IF_ERROR_RETURN(bcm_stg_list_destroy(unit, stg_list, stg_count));
    if (!stg_existed) {
        return BCM_E_NOT_FOUND;
    }

    /* Upper layer already checks that vid is valid */

    if ((rv = DRV_MSTP_CONFIG_SET
        (unit, (uint32)vid, (uint32)stg)) < 0) {
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_vlan_stg_set
 * Purpose:
 * Puts the specified VLAN into the specified STG
 * Parameters:
 *  unit  - RoboSwitch PCI device unit number
 *      vid - VLAN ID
 *  stg - New spanning tree group number for VLAN
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_robo_vlan_stg_set(int unit, bcm_vlan_t vid, bcm_stg_t stg)
{
    int rv;

    CHECK_INIT(unit);
    CHECK_VID(unit, vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_stg_set()..\n")));

    rv = bcm_robo_stg_vlan_add(unit, stg, vid);

    return rv;
}               
    
/*
 * Function:
 *  _bcm_robo_vlan_stp_set
 * Purpose:
 *  Main part of bcm_vlan_stp_set; assumes locking already done.
 */

STATIC int
_bcm_robo_vlan_stp_set(int unit, bcm_vlan_t vid, 
                bcm_port_t port, int stp_state)
{
    bcm_stg_t   stgid;

    BCM_IF_ERROR_RETURN(bcm_vlan_stg_get(unit, vid, &stgid));
    BCM_IF_ERROR_RETURN(bcm_stg_stp_set(unit, stgid, port, stp_state));

    return BCM_E_NONE;
}
    
/*
 * Function:
 *  bcm_robo_vlan_stp_set
 * Purpose:
 *  Set the spanning tree state for a port in the whole spanning
 *  tree group that contains the specified VLAN.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *      vid - VLAN ID
 *      port - Port
 *  stp_state - State to set
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_vlan_stp_set(int unit, bcm_vlan_t vid,
                bcm_port_t port, int stp_state)
{
    int     rv;
    bcm_port_t  local_port;

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &local_port));
    } else {
        local_port = port;
    }

    if (!SOC_PORT_VALID(unit, local_port)) {
        return BCM_E_PORT;
    }

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_stp_set()..\n")));
    rv = _bcm_robo_vlan_stp_set(unit, vid, local_port, stp_state);

    return rv;
}               

/*
 * Function:
 *  bcm_robo_vlan_stp_get
 * Purpose:
 *  Get the spanning tree state for a port in the whole spanning
 *  tree group that contains the specified VLAN.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number.
 *      vid - VLAN ID.
 *      port - Port
 *  stp_state - (OUT) State to return.
 * Returns:
 *  BCM_E_XXX
 */

int 
bcm_robo_vlan_stp_get(int unit, bcm_vlan_t vid,
                bcm_port_t port, int *stp_state)
{
    int     rv;
    uint32 port_status;
    bcm_port_t  local_port;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_stp_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &local_port));
    } else {
        local_port = port;
    }

    if (!SOC_PORT_VALID(unit, local_port)) {
        return BCM_E_PORT;
    }

    if ((rv = DRV_MSTP_PORT_GET
        (unit, (uint32)vid, (uint32)local_port, &port_status)) < 0) {
        return rv;
    }
    *stp_state = (port_status == DRV_PORTST_DISABLE) ? BCM_STG_STP_DISABLE :
             (port_status == DRV_PORTST_BLOCK) ? BCM_STG_STP_BLOCK :
             (port_status == DRV_PORTST_LEARN) ? BCM_STG_STP_LEARN :
             (port_status == DRV_PORTST_FORWARD) ? BCM_STG_STP_FORWARD :
             BCM_STG_STP_LISTEN;

    return BCM_E_NONE;
}               
                

/* MAC based vlan selection */
int 
bcm_robo_vlan_mac_add(int unit, bcm_mac_t mac, bcm_vlan_t vid, int prio)
{
    int rv = BCM_E_NONE;
    mac2vlan_entry_t  vment;
    uint64 mac_field;
    uint32  temp;
    
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_mac_add()\n")));

    prio &= ~BCM_PRIO_DROP_FIRST;       /* no cng setting in ROBO */
    if ((prio & ~BCM_PRIO_MASK) != 0) {
        return BCM_E_PARAM;
    }

    sal_memset(&vment, 0, sizeof(vment));

    /* set Valid field */
    temp = 1;
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID, 
        (uint32 *)&vment, &temp));
    
    /* set priority field */
    temp = prio;
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_PRIORITY, 
        (uint32 *)&vment, &temp));
    /* set VID field */
    temp = vid;
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VLANID, 
        (uint32 *)&vment, &temp));
    /* set MAC field */
    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC, 
        (uint32 *)&vment, (void *)&mac_field));

    rv = DRV_MEM_INSERT
        (unit, DRV_MEM_MACVLAN, (uint32 *)&vment, 0);
    return rv;
}               

int 
bcm_robo_vlan_mac_delete(int unit, bcm_mac_t mac)
{
    int rv = BCM_E_NONE;
    mac2vlan_entry_t  vment;
    uint64 mac_field;
    uint32  temp;
    
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_mac_delete()\n")));
    
    sal_memset(&vment, 0, sizeof(vment));

    /* set Valid field */
    temp = 1;
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID, 
        (uint32 *)&vment, &temp));
    
    /* set MAC field */
    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC, 
        (uint32 *)&vment, (void *)&mac_field));

    rv = DRV_MEM_DELETE
        (unit, DRV_MEM_MACVLAN, (uint32 *)&vment, 0);

    if (rv == SOC_E_NOT_FOUND) {
        rv = SOC_E_NONE;
    }

    return rv;
}               
    
int bcm_robo_vlan_mac_delete_all(int unit)
{
    int rv = BCM_E_NONE;
    
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_mac_delete_all()\n")));
    
    rv = DRV_MEM_CLEAR
        (unit, DRV_MEM_MACVLAN);
    return rv;
}               
    

/*
 * Function:
 *      bcm_robo_vlan_translate_get
 * Purpose:
 *      Get vlan translation
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      port - port numebr (ignored in ROBO53242/53262/53115)
 *      old_vid - Old VLAN ID to has translation for
 *      new_vid - New VLAN ID that packet will get
 *      prio    - Priority (ignored in ROBO53242/53262/53115)
 * Returns:
 *      BCM_E_NONE - Translation found, new_vid nad prio will have the values.
 *      BCM_E_NOT_FOUND - Translation does not exist
 *      BCM_E_XXX  - Other error
 * Notes:
 *      None.
 */

int 
bcm_robo_vlan_translate_get (int unit, bcm_port_t port, bcm_vlan_t old_vid,
                            bcm_vlan_t *new_vid, int *prio)
{
    uint32 vt_mode = 0, vt_new_vid = 0;
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s(unit=%d,port=%d,vid=%d)\n"),
              FUNCTION_NAME(), unit, port, old_vid));
            
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        bcm_vlan_action_set_t action;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        bcm_vlan_action_set_t_init(&action);

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortOuter;
        vt_act_param.param1 = old_vid;
        vt_act_param.param2 = 0;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_get(unit, &vt_act_param, &action));
        if (BCM_VLAN_INVALID != action.new_outer_vlan) {
            *new_vid = action.new_outer_vlan;
            *prio = action.priority;
            return BCM_E_NONE;
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = old_vid;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_get(unit, &vt_act_param, &action));
        if (BCM_VLAN_INVALID != action.new_inner_vlan) {
            *new_vid = action.new_inner_vlan;
            *prio = action.priority;
            return BCM_E_NONE;
        }

        return BCM_E_NOT_FOUND;
    }
#endif
            
    /* check valid port only but doing nothing for ROBO */
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    /* The VLAN translation in bcm53243/bcm53262/bcm53115 support VID key 
     * only. The priority key is ignored in ROBO vlan translation.
     */
    BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
            (unit, DRV_VLAN_PROP_ING_VT_SPVID, old_vid, port, &vt_new_vid));
    *new_vid = (bcm_vlan_t)vt_new_vid;

    /* check the target VID's VT_Mode(transparant/mapping)
     *  - dtag is VLAN_XLATE transparant mode.
     *  - if the target VID is assinged as transparent mode, the return value 
     *      will still be "BCM_E_NOTFOUND".
     */    
    BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
            (unit, DRV_VLAN_PROP_VT_MODE, old_vid, port, &vt_mode));
    if (vt_mode != VLAN_VTMODE_MAPPING){
        
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s,unit=%d,port=%d,vid=%d is not at mapping mode!\n"),
                  FUNCTION_NAME(), unit, port, old_vid));
        return BCM_E_NOT_FOUND;
    }
    
    return BCM_E_NONE;
}

/* VLAN translation selection, add VT as Mapping mode 
 *  
 * Note :
 *  1. Vulcan support device basis egress VT only, no port basis and no 
 *      ingress basis.
 */
int 
bcm_robo_vlan_translate_add(int unit, bcm_port_t port, bcm_vlan_t old_vid, 
                  bcm_vlan_t new_vid, int prio)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id; 
        
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid);
    CHECK_VID(unit, new_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s\n"), FUNCTION_NAME()));

    /* Check valid priority only but doing nothing for ROBO */
    if ((prio != -1) && ((prio & BCM_PRIO_MASK) > _BCM_PRIO_MAX(unit))) {
        return BCM_E_PARAM;
    }

    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_vlan_action_set_t action;
        bcm_gport_t gport;
        int rv = BCM_E_NONE;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        /* add an entry for inner-tagged packets */
        bcm_vlan_action_set_t_init(&action);
        action.new_outer_vlan = new_vid;
        action.priority = prio;
        action.it_outer = bcmVlanActionAdd;
        action.it_inner = bcmVlanActionDelete;
        action.it_inner_prio = bcmVlanActionDelete;
        action.outer_pcp = bcmVlanPcpActionIngressInnerPcp;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = old_vid;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE;

        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_add(unit, &vt_act_param, &action));

        /* add an entry for outer-tagged packets */
        bcm_vlan_action_set_t_init(&action);
        action.new_outer_vlan = new_vid;
        action.priority = prio;
        action.ot_outer      = bcmVlanActionReplace;
        action.ot_outer_prio = bcmVlanActionReplace;
        action.dt_outer      = bcmVlanActionReplace;
        action.dt_outer_prio = bcmVlanActionReplace;
        action.outer_pcp = bcmVlanPcpActionIngressOuterPcp;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortOuter;
        vt_act_param.param1 = old_vid;
        vt_act_param.param2 = 0;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE;

       rv = _robo_vlan_translate_action_add(unit, &vt_act_param, &action);
       if (rv < 0) {
            sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
            vt_act_param.port = gport;
            vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
            vt_act_param.param1 = 0;
            vt_act_param.param2 = old_vid;
            vt_act_param.vt_api_type = VT_API_VLAN_XLATE;
            _robo_vlan_translate_action_delete(unit, &vt_act_param);
            return rv;
       }

        /* Save information for traverse. */
        rv = _robo_vlan_translate_traverse_info_add(unit, gport, old_vid, 
                  new_vid, prio);
        if (rv < 0) {
            /* 
              * Traverse information sw copy create failed. 
              */
            sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
            vt_act_param.port = gport;
            vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
            vt_act_param.param1 = 0;
            vt_act_param.param2 = old_vid;
            vt_act_param.vt_api_type = VT_API_VLAN_XLATE;
            _robo_vlan_translate_action_delete(unit, &vt_act_param);

            sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
            vt_act_param.port = gport;
            vt_act_param.key_type = bcmVlanTranslateKeyPortOuter;
            vt_act_param.param1 = old_vid;
            vt_act_param.param2 = 0;
            vt_act_param.vt_api_type = VT_API_VLAN_XLATE;
            _robo_vlan_translate_action_delete(unit, &vt_act_param);
        }

        return rv;
    }
#endif 

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    /* In robo chips so far, there are only bcm53242/bcm53262/bcm53115 
     *  support VLAN translation. 
     * ----------
     * 1. bcm53242/bcm53262 : support ingress basis vlan_translation only.
     * 2. bcm53115 : support egress basis vlan_translation only.
     *          - actually, bcm53115 is working by ingress VID filtering and 
     *              egress VID translate.
     */ 
    BCM_IF_ERROR_RETURN(DRV_VLAN_VT_ADD
            (unit, DRV_VLAN_XLAT_INGRESS, port, old_vid, new_vid, 
             prio, VLAN_VTMODE_MAPPING));

    return BCM_E_NONE;

}

/*
 * Function:
 *  bcm_robo_vlan_translate_delete
 * Purpose:
 *  Delete an entry or entries from the VLAN Translation table.
 * Parameters:
 *  unit - Switch chip PCI device unit number.
 *      old_vid - VLAN ID.
 *      port - Port
 * Returns:
 *      BCM_E_NONE   0 or more entries were deleted
 *  BCM_E_XXX    Internal error
 *
 * Note :
 *  1. Vulcan support device basis egress VT only, no port basis and no 
 *      ingress basis.
 */

int 
bcm_robo_vlan_translate_delete(int unit, bcm_port_t port, bcm_vlan_t old_vid)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id; 
    int rv = BCM_E_NONE;

    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s\n"), FUNCTION_NAME()));
            
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }


        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = old_vid;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete(unit, &vt_act_param));

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortOuter;
        vt_act_param.param1 = old_vid;
        vt_act_param.param2 = 0;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete(unit, &vt_act_param));

        /* Remove information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_traverse_info_remove(unit, 
                                                  gport, old_vid));
        return BCM_E_NONE;
    }
#endif 

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }
    
    /* In robo chips so far, there are only bcm53242/bcm53262/bcm53115 
     *  support VLAN translation. 
     * ----------
     * 1. bcm53242/bcm53262 : support ingress basis vlan_translation only.
     * 2. bcm53115 : support egress basis vlan_translation only.
     *          - actually, bcm53115 is working by ingress VID filtering and 
     *              egress VID translate.
     */ 
    rv = DRV_VLAN_VT_DELETE
                (unit, DRV_VLAN_XLAT_INGRESS_MAP, port, old_vid);
    return rv;
}

int
bcm_robo_vlan_translate_delete_all(int unit)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s\n"), FUNCTION_NAME()));

    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_XLATE, 0));

        /* Remove all information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_traverse_info_remove_all(unit));
        return BCM_E_NONE;
    }
#endif

    BCM_IF_ERROR_RETURN(DRV_VLAN_VT_DELETE_ALL
                (unit, DRV_VLAN_XLAT_INGRESS_MAP));

    return BCM_E_NONE;
}

/* Egress VLAN translation selection, add VT as Mapping mode 
 *  
 * Note :
 *  1. Vulcan support device basis egress VT only, no port basis and no 
 *      ingress basis.
 */
int
bcm_robo_vlan_translate_egress_add(int unit,bcm_port_t port, 
        bcm_vlan_t old_vid, bcm_vlan_t new_vid, int prio)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid);
    CHECK_VID(unit, new_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s\n"), FUNCTION_NAME()));
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    /* check valid priority only but doing nothing for ROBO */
    if ((prio != -1) && ((prio & ~BCM_PRIO_MASK) != 0)) {
        return BCM_E_PARAM;
    }

    /* bcm53115, support egress basis vlan translation through 
     *  1. CFP : for ingress VID filtering and 
     *  2. EVR (Egress Vlan Remark) table for egress VID process.
     */
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int idt_mode, rebuild_request = FALSE;
        
        /* re-init VT_CFP and EVR table, add this seciton is for the 
         *  regression test will not call bcm_vlan_control_set() to enable 
         *  VLAN translation feature.
         */
        if ((!IS_VT_CFP_INIT)){
            
            /* make sure the requirred CFP was init already */
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_init(unit));
    
            /* clear all EVR table entry on each port. and the related 
             *      port basis database.
             */
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));

            rebuild_request = TRUE;
        }

        /* enable iDT_Mode, add this seciton is for the regression test 
         *  will not call bcm_vlan_control_set() to enable VLAN translation 
         *  feature.
         */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                         (uint32 *) &idt_mode));
        if (!idt_mode){
            idt_mode = TRUE;
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, idt_mode));
        }

        /* Vulcan VLAN translation can support global VT only(no port basis)
         * so the port parameter will be ignore here.
         */
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_create(
                    unit, old_vid, new_vid, VLAN_VTMODE_MAPPING));

        /* GNATS 41020 :
         *  - Packet to ISP port won't be outer tagged once the port been set 
         *      to ISP before any VT entry created.
         */
        if (rebuild_request){
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vt_all_isp_update(unit));
        }
    } else {

        BCM_IF_ERROR_RETURN(DRV_VLAN_VT_ADD
                (unit, DRV_VLAN_XLAT_EGRESS, port, old_vid, new_vid, 
                 prio, VLAN_VTMODE_MAPPING));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_vlan_translate_egress_get
 * Purpose:
 *      Get vlan egress translation
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      port - port numebr (ignored in ROBO53242/53262/53115)
 *      old_vid - Old VLAN ID to has translation for
 *      new_vid - New VLAN ID that packet will get
 *      prio    - Priority (ignored in ROBO53242/53262/53115)
 * Returns:
 *      BCM_E_NONE - Translation found, new_vid nad prio will have the values.
 *      BCM_E_NOT_FOUND - Translation does not exist
 *      BCM_E_XXX  - Other error
 * Notes:
 *      None.
 */
int bcm_robo_vlan_translate_egress_get (int unit, bcm_port_t port,
                                       bcm_vlan_t old_vid,
                                       bcm_vlan_t *new_vid,
                                       int *prio)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    uint32 vt_mode = 0, vt_new_vid = 0;
    
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s(unit=%d,port=%d,vid=%d)\n"),
              FUNCTION_NAME(), unit, port, old_vid));
            
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    /* The VLAN translation in bcm53243/bcm53262/bcm53115 support VID key 
     * only. The priority key is ignored in ROBO vlan translation.
     */
    BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
            (unit, DRV_VLAN_PROP_EGR_VT_SPVID, old_vid, port, &vt_new_vid));
    *new_vid = vt_new_vid;

    /* check the target VID's VT_Mode(transparant/mapping)
     *  - dtag is VLAN_XLATE transparant mode.
     *  - if the target VID is assinged as transparent mode, the return value 
     *      will still be "BCM_E_NOTFOUND".
     */    
    BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
            (unit, DRV_VLAN_PROP_VT_MODE, old_vid, port, &vt_mode));
    if (vt_mode != VLAN_VTMODE_MAPPING){
        
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s,unit=%d,port=%d,vid=%d is not at mapping mode!\n"),
                  FUNCTION_NAME(), unit, port, old_vid));
        return BCM_E_NOT_FOUND;
    }
    
    return BCM_E_NONE;
    
}

/* Egress VLAN translation delection
 *  
 * Note :
 *  1. Vulcan support device basis egress VT only, no port basis and no 
 *      ingress basis.
 */
int
bcm_robo_vlan_translate_egress_delete(int unit,bcm_port_t port, 
            bcm_vlan_t old_vid)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;

    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s\n"), FUNCTION_NAME()));
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    /* bcm53115, support egress basis vlan translation through 
     *  1. CFP : for ingress VID filtering and 
     *  2. EVR (Egress Vlan Remark) table for egress VID process.
     */
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        
        int rv = BCM_E_NONE;
        int idt_mode;
        
        if (!IS_VT_CFP_INIT) {
            
            /* make sure the requirred CFP was init already */
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_init(unit));
    
            /* clear all EVR table entry on each port. and the related 
             *      port basis database.
             */
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));
        }

        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                         (uint32 *) &idt_mode));
        if (!idt_mode) {
            idt_mode = TRUE;
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, idt_mode));
        }

        /* Vulcan VLAN translation can support global VT only(no port basis)
         * so the port parameter will be ignore here.
         */
        rv = _bcm_robo_vlan_vtcfp_delete(
                        unit, old_vid, VLAN_VTMODE_MAPPING);
        
        return rv;
    } else {
        BCM_IF_ERROR_RETURN(DRV_VLAN_VT_DELETE
                    (unit, DRV_VLAN_XLAT_EGRESS, port, old_vid));
    }

    return BCM_E_NONE;
}
    
int
bcm_robo_vlan_translate_egress_delete_all(int unit)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s\n"), FUNCTION_NAME()));
            
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }
    
    /* bcm53115, support egress basis vlan translation through 
     *  1. CFP : for ingress VID filtering and 
     *  2. EVR (Egress Vlan Remark) table for egress VID process.
     */
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        uint32  idt_mode;
        
        if (!IS_VT_CFP_INIT) {
            
            /* make sure the requirred CFP was init already */
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_init(unit));
    
            /* clear all EVR table entry on each port. and the related 
             *      port basis database.
             */
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));
        }

        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, &idt_mode));
        if (!idt_mode) {
            idt_mode = TRUE;
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, idt_mode));
        }
        
        /* Vulcan VLAN translation can support global VT only(no port basis)
         * so the port parameter will be ignore here.
         */
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_delete_all(
                        unit, VLAN_VTMODE_MAPPING));
    } else {
        BCM_IF_ERROR_RETURN(DRV_VLAN_VT_DELETE_ALL
                    (unit, DRV_VLAN_XLAT_EGRESS));
    }

    return BCM_E_NONE;
}

int
bcm_robo_vlan_translate_range_add(int unit, int port, bcm_vlan_t old_vid_low,
                                 bcm_vlan_t old_vid_high, bcm_vlan_t new_vid,
                                 int int_prio)
{
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid_low);
    CHECK_VID(unit, old_vid_high);
    CHECK_VID(unit, new_vid);
    if (old_vid_high < old_vid_low) {
        return BCM_E_PARAM;
    }

    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_vlan_action_set_t action;
        bcm_gport_t gport;
        int rv = BCM_E_NONE;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        /* add an entry for inner-tagged packets */
        bcm_vlan_action_set_t_init(&action);
        action.new_outer_vlan = new_vid;
        action.priority = int_prio;
        action.it_outer = bcmVlanActionAdd;
        action.it_inner = bcmVlanActionDelete;
        action.it_inner_prio = bcmVlanActionNone;
        action.outer_pcp = bcmVlanPcpActionIngressInnerPcp;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = old_vid_low;
        vt_act_param.param2 = old_vid_high;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_RANGE;

        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_add(unit, &vt_act_param, &action));
        /*
          * Double-tagged and outer-tagged packets' vlan range translate
          * is not supported in BCM53280.
          */

        /* Save information for traverse. */
        rv = _robo_vlan_translate_range_traverse_info_add(unit, gport, 
                old_vid_low, old_vid_high, new_vid, int_prio);
        if (rv < 0) {
            /* 
              * Traverse information sw copy create failed. 
              */
            _robo_vlan_translate_action_delete(unit, &vt_act_param);
        }
        return rv;
    }
#endif 
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_translate_range_get (int unit, bcm_port_t port,
                                      bcm_vlan_t old_vlan_low,
                                      bcm_vlan_t old_vlan_high,
                                      bcm_vlan_t *new_vid, int *prio)
{
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vlan_low);
    CHECK_VID(unit, old_vlan_high);

    if ((NULL == new_vid) || NULL == prio ){
        return BCM_E_PARAM;
    }
    if (old_vlan_high < old_vlan_low) {
        return BCM_E_PARAM;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        bcm_vlan_action_set_t action;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        /*
          * Double-tagged and outer-tagged packets' vlan range translate
          * is not supported in BCM53280.
          */

        /* Get inner-tagged action */
        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = old_vlan_low;
        vt_act_param.param2 = old_vlan_high;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_RANGE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_get(unit, &vt_act_param, &action));
        if (BCM_VLAN_INVALID != action.new_outer_vlan) {
            *new_vid = action.new_outer_vlan;
            *prio = action.priority;
            return BCM_E_NONE;
        }

        return BCM_E_NOT_FOUND;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function :
 *      bcm_vlan_translate_range_traverse
 * Description :
 *   Traverses over VLAN translate table and call provided callback 
 *   with valid entries.
 * Parameters :
 *      unit            (IN) BCM unit number
 *      cb              (IN) User callback function
 *      user_data       (IN) Pointer to user specific data
 * Return:
 *
 *      BCM_E_XXX
 */
int 
bcm_robo_vlan_translate_range_traverse(int unit, 
                                  bcm_vlan_translate_range_traverse_cb cb,
                                  void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_translation))  {
        rv = _robo_vlan_translate_range_traverse(unit, cb, user_data);
        return rv;
    }
#endif

    return rv;
}


int
bcm_robo_vlan_translate_range_delete(int unit, int port,
                                    bcm_vlan_t old_vid_low,
                                    bcm_vlan_t old_vid_high)
{
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid_low);
    CHECK_VID(unit, old_vid_high);
    if (old_vid_high < old_vid_low) {
        return BCM_E_PARAM;
    }

    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }
        /* Inner-tagged deletion */
        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = old_vid_low;
        vt_act_param.param2 = old_vid_high;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_RANGE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete(unit, &vt_act_param));

        /*
          * Double-tagged and outer-tagged packets' vlan range translate
          * is not supported in BCM53280.
          */

        /* Remove information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_range_traverse_info_remove(unit, 
                                                  gport, old_vid_low, old_vid_high));
        return BCM_E_NONE;
    }
#endif 

    return BCM_E_UNAVAIL;
}


int
bcm_robo_vlan_translate_range_delete_all(int unit)
{
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
    return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete_all(unit, \
                VT_API_VLAN_XLATE_RANGE, 0));

        /* Remove all information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_range_traverse_info_remove_all(unit));

        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

#ifdef BCM_TB_SUPPORT
int
_bcm_robo_vlan_vm_group_create(int unit, int type, 
                            bcm_field_group_t *group_id)
{
    int rv = BCM_E_NONE;
    bcm_field_qset_t qset;
    
    BCM_FIELD_QSET_INIT(qset);
    if (type == BCM_ROBO_VM_IVM_GROUP) {
        /* Add IVM qualifiers. */
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageLookup);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInterfaceClassPort);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyVlanFormat);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOuterVlanId);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyRangeCheck);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInnerVlanId);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOuterVlanPri);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInnerVlanPri);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyL2Format);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
    } else if (type == BCM_ROBO_VM_EVM_GROUP) {
        /* Add EVM qualifiers. */
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageEgress);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInVPort);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyFlowId);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOutPort);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOutVPort);
    } else {
        return BCM_E_PARAM;
    }
    /* Create VM group */
    rv = bcm_field_group_create(unit, qset, 
        BCM_FIELD_GROUP_PRIO_ANY, group_id);
    if (rv < 0) {
        *group_id = 0;
    }
    return rv;
}

/*
 * Function : _bcm_robo_vlan_action_verify
 *
 * Purpose  : to validate all members of action structure
 *
 */
STATIC int 
_bcm_robo_vlan_action_verify(int unit, bcm_vlan_action_set_t *action)
{
    if (NULL == action) {
        return BCM_E_PARAM;
    }
    _ROBO_VLAN_CHK_ID(unit, action->new_inner_vlan);
    _ROBO_VLAN_CHK_ID(unit, action->new_outer_vlan);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_outer_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_inner);
    _ROBO_VLAN_CHK_ACTION(unit, action->dt_inner_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->ot_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->ot_outer_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->ot_inner);
    _ROBO_VLAN_CHK_ACTION(unit, action->it_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->it_inner);
    _ROBO_VLAN_CHK_ACTION(unit, action->it_inner_prio);
    _ROBO_VLAN_CHK_ACTION(unit, action->ut_outer);
    _ROBO_VLAN_CHK_ACTION(unit, action->ut_inner);
    

    return BCM_E_NONE;
}

STATIC void
_robo_vlan_action_map(bcm_vlan_action_t act1, uint32 *act2)
{
    switch (act1) {
        case bcmVlanActionAdd:
        case bcmVlanActionReplace:
            *act2 = BCM_FIELD_TAG_REPLACE;
            break;
        case bcmVlanActionDelete:
            *act2 = BCM_FIELD_TAG_REMOVE;
            break;
        case bcmVlanActionNone:
        default:
            *act2 = BCM_FIELD_TAG_AS_RECEIVED;
            break;
    }
}

STATIC int
_robo_vlan_tag_resolution(uint32 *tag_action_map, 
                                  bcm_vlan_action_set_t *action)
{
    if ((action->dt_outer != bcmVlanActionNone) ||
        (action->dt_inner != bcmVlanActionNone) ||
        (action->dt_outer_prio != bcmVlanActionNone) ||
        (action->dt_inner_prio != bcmVlanActionNone)) {
        *tag_action_map |= VLAN_ACTION_DT;
    }
    if ((action->ot_outer != bcmVlanActionNone) ||
        (action->ot_outer_prio != bcmVlanActionNone) ||
        (action->ot_inner != bcmVlanActionNone)) {
        *tag_action_map |= VLAN_ACTION_OT;
    } 
    if ((action->it_outer != bcmVlanActionNone) ||
        (action->it_inner != bcmVlanActionNone) ||
        (action->it_inner_prio != bcmVlanActionNone)) {
        *tag_action_map |= VLAN_ACTION_IT;
    } 
    if ((action->ut_outer != bcmVlanActionNone) ||
        (action->ut_inner != bcmVlanActionNone)) {
        *tag_action_map |= VLAN_ACTION_UT;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_pcp_resolution(uint32 tag_action_map, 
                                  uint32 *vm_act_pcp_mark, 
                                  bcm_vlan_action_set_t *action)
{
    uint32 temp = 0;
    switch(action->outer_pcp) {
        case bcmVlanPcpActionMapped:
            temp |= BCM_FIELD_SPCP_MARK_MAPPED;
            break;
        case bcmVlanPcpActionIngressInnerPcp:
            temp |= BCM_FIELD_SPCP_MARK_USE_INNER_PCP;
            break;
        case bcmVlanPcpActionIngressOuterPcp:
            temp |= BCM_FIELD_SPCP_MARK_USE_OUTER_PCP;
            break;
        case bcmVlanPcpActionPortDefault:
            temp |= BCM_FIELD_SPCP_MARK_USE_PORT_DEFAULT;
            break;
        case bcmVlanPcpActionNone:
            /* 
              * If outer/inner_pcp is not specified(bcmVlanPcpActionNone),
              * assign default value according to the incoming packets' tag status. 
              */
            if (tag_action_map & VLAN_ACTION_DT) {
                temp |= BCM_FIELD_SPCP_MARK_USE_OUTER_PCP;
            } else if (tag_action_map & VLAN_ACTION_OT) {
                temp |= BCM_FIELD_SPCP_MARK_USE_OUTER_PCP;
            } else if (tag_action_map & VLAN_ACTION_IT) {
                temp |= BCM_FIELD_SPCP_MARK_USE_INNER_PCP;
            } else if (tag_action_map & VLAN_ACTION_UT) {
                temp |= BCM_FIELD_SPCP_MARK_USE_PORT_DEFAULT;
            } else {
                return BCM_E_PARAM;
            }
            break;
        default:
            return BCM_E_PARAM;
            break;
    }

    switch(action->inner_pcp) {
        case bcmVlanPcpActionMapped:
            temp |= BCM_FIELD_CPCP_MARK_MAPPED;
            break;
        case bcmVlanPcpActionIngressInnerPcp:
            temp |= BCM_FIELD_CPCP_MARK_USE_INNER_PCP;
            break;
        case bcmVlanPcpActionIngressOuterPcp:
            temp |= BCM_FIELD_CPCP_MARK_USE_OUTER_PCP;
            break;
        case bcmVlanPcpActionPortDefault:
            temp |= BCM_FIELD_CPCP_MARK_USE_PORT_DEFAULT;
            break;
        case bcmVlanPcpActionNone:
            /* 
              * If outer/inner_pcp is not specified(bcmVlanPcpActionNone),
              * assign default value according to the incoming packets' tag status. 
              */
            if (tag_action_map & VLAN_ACTION_DT) {
                temp |= BCM_FIELD_CPCP_MARK_USE_INNER_PCP;
            } else if (tag_action_map & VLAN_ACTION_OT) {
                temp |= BCM_FIELD_CPCP_MARK_USE_OUTER_PCP;
            } else if (tag_action_map & VLAN_ACTION_IT) {
                temp |= BCM_FIELD_CPCP_MARK_USE_INNER_PCP;
            } else if (tag_action_map & VLAN_ACTION_UT) {
                temp |= BCM_FIELD_CPCP_MARK_USE_PORT_DEFAULT;
            } else {
                return BCM_E_PARAM;
            }
            break;
        default:
            return BCM_E_PARAM;
            break;
    }

    *vm_act_pcp_mark = temp;
    
    return BCM_E_NONE;
}

STATIC int
_robo_vlan_action_vlan_resolution(uint32 tag_action_map,
                                  uint32 *ivm_key_tag_status,
                                  uint32 *act_stag,
                                  uint32 *act_ctag, 
                                  uint8 *entry_cnt_type,
                                  _robo_vlan_translate_action_param_t *param,
                                  bcm_vlan_action_set_t *action)
{
    uint32 *evm_act_stag;
    uint32 *evm_act_ctag;
    bcm_vlan_t old_svid = 0;
    bcm_vlan_t old_cvid = 0;
    bcm_vlan_translate_key_t key_type = 0;

    evm_act_stag = act_stag;
    evm_act_ctag = act_ctag;

    old_svid = BCM_VLAN_CTRL_ID(param->param1);
    old_cvid = BCM_VLAN_CTRL_ID(param->param2);
    
    if (tag_action_map & VLAN_ACTION_DT) {
        /* 
          * For DT cases, svid/cvid = 0 can be either priority-tagged or don't care.
          * bcmVlanTranslateKeyxxx is involved to distinguish the meaning
          * of svid/cvid=0.
          */
        key_type = param->key_type;
        
        if (old_svid && old_cvid) {
            ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                             BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
            _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
        } else if (old_svid && !old_cvid) {
            if ((key_type == bcmVlanTranslateKeyPortDouble) ||
                (key_type == bcmVlanTranslateKeyDouble)) {
                /* 
                  * Both old_svid and old_cvid should be checked.
                  * old_cvid=0 means inner priority-tagged.
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            } else if ((key_type == bcmVlanTranslateKeyPortOuter) ||
                (key_type == bcmVlanTranslateKeyOuter)) {
                /* 
                  * Only old_svid should be checked.
                  * old_cvid=0 means don't care.
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                             BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            } else {
                /* bcmVlanTranslateKeyPortInner/Inner/PortInnerTag/InnerTag */
                /* 
                  * Only old_cvid should be checked.
                  * old_cvid=0 means inner priority-tagged.
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                             BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            }
            _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
            _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[0]);
        } else if (!old_svid && old_cvid) {
            if ((key_type == bcmVlanTranslateKeyPortDouble) ||
                (key_type == bcmVlanTranslateKeyDouble)) {
                /* 
                  * Both old_svid and old_cvid should be checked.
                  * old_svid=0 means outer priority-tagged.
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            } else if ((key_type == bcmVlanTranslateKeyPortOuter) ||
                (key_type == bcmVlanTranslateKeyOuter)) {
                /* 
                  * Only old_svid should be checked.
                  * old_svid=0 means outerer priority-tagged.
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            } else {
                /* bcmVlanTranslateKeyPortInner/Inner/PortInnerTag/InnerTag */
                /* 
                  * Only old_cvid should be checked.
                  * old_svid=0 means don't care.
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            }
            _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[0]);
            _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
        } else {
            /* (!old_cvid && !old_cvid) */
            ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                             BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[0]);
            _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[0]);
        }
        *entry_cnt_type = 1;
    } else if (tag_action_map & VLAN_ACTION_OT) {
        if (old_svid) {
            /* SVID != 0 */
            ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
            _robo_vlan_action_map(action->ot_outer, &evm_act_stag[0]);
            _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[0]);
        } else {
            /* SVID == 0 */
            ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            _robo_vlan_action_map(action->ot_outer_prio, &evm_act_stag[0]);
            _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[0]);
        }
        *entry_cnt_type = 1;
    } else if (tag_action_map & VLAN_ACTION_IT) {
            if (old_cvid) {
                /* CVID != 0 */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                _robo_vlan_action_map(action->it_outer, &evm_act_stag[0]);
                _robo_vlan_action_map(action->it_inner, &evm_act_ctag[0]);
            } else {
                /* CVID == 0 */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                _robo_vlan_action_map(action->it_outer, &evm_act_stag[0]);
                _robo_vlan_action_map(action->it_inner_prio, &evm_act_ctag[0]);
            }
            *entry_cnt_type = 1;
    } else if (tag_action_map & VLAN_ACTION_UT) {
        ivm_key_tag_status[0] = 0;
        _robo_vlan_action_map(action->ut_outer, &evm_act_stag[0]);
        _robo_vlan_action_map(action->ut_inner, &evm_act_ctag[0]);
        *entry_cnt_type = 1;
    }

    return BCM_E_NONE;
}

/* 
  * This routine is for APIs that do not use vlan id as the translation keys. 
  * Currently only protocol2vlan and port default APIs use this.
  */
STATIC int
_robo_vlan_action_common_resolution(uint32 tag_action_map,
                                  uint32 *ivm_key_tag_status,
                                  uint32 *act_stag,
                                  uint32 *act_ctag, 
                                  uint8 *entry_cnt_type,
                                  _robo_vlan_translate_action_param_t *param,
                                  bcm_vlan_action_set_t *action)
{
    uint32 *evm_act_stag;
    uint32 *evm_act_ctag;

    evm_act_stag = act_stag;
    evm_act_ctag = act_ctag;

    if ((param->vt_api_type != VT_API_VLAN_PROTOCOL) &&
    	(param->vt_api_type != VT_API_VLAN_PORT_DEFAULT)) {
        /* 
          * Other translations except port default and protocol2vlan 
          * can use svid/cvid as the consideration of tag status.
          */
        return ( _robo_vlan_action_vlan_resolution(tag_action_map,
            ivm_key_tag_status, act_stag, act_ctag, entry_cnt_type,
            param, action));
    }

    if (tag_action_map & VLAN_ACTION_DT) {
        if ((action->dt_outer == action->dt_outer_prio) &&
            (action->dt_inner == action->dt_inner_prio)) {
            ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                            BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                             BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                             BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
            _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
            *entry_cnt_type = 1;
        } else if ((action->dt_outer != action->dt_outer_prio) &&
            (action->dt_inner == action->dt_inner_prio)) {
            if (action->dt_inner != bcmVlanActionNone) {
                /* 
                  * Two entries have to be created. 
                  * 0. STag = tagged, CTag = tagged or pri-tagged
                  * 1. STag = pri-tagged, CTag = tagged or pri-tagged
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[1]);
                _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[1]);
                *entry_cnt_type = 2;
            } else { /* (action->dt_inner == bcmVlanActionNone) */
                if ((action->dt_outer == bcmVlanActionNone) &&
                    (action->dt_outer_prio != bcmVlanActionNone)) {
                    /* 
                      * One entries have to be created. 
                      * 0. STag = pri-tagged, CTag = tagged or pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    *entry_cnt_type = 1;
                } else if ((action->dt_outer != bcmVlanActionNone) &&
                    (action->dt_outer_prio == bcmVlanActionNone)) {
                    /* 
                      * One entries have to be created. 
                      * 0. STag = tagged, CTag = tagged or pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    *entry_cnt_type = 1;
                } else {
                    /*
                      * (action->dt_outer != bcmVlanActionNone) &&
                      * (action->dt_outer_prio != bcmVlanActionNone)
                      * Two entries have to be created. 
                      * 0. STag = tagged, CTag = tagged or pri-tagged
                      * 1. STag = pri-tagged, CTag = tagged or pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[1]);
                    *entry_cnt_type = 2;
                }
            }
        } else if ((action->dt_outer == action->dt_outer_prio) &&
            (action->dt_inner != action->dt_inner_prio)) {
            if (action->dt_outer != bcmVlanActionNone) {
                /* 
                  * Two entries have to be created. 
                  * 0. STag = tagged or pri-tagged, CTag = tagged
                  * 1. STag = tagged or pri-tagged, CTag = pri-tagged
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                _robo_vlan_action_map(action->dt_outer, &evm_act_stag[1]);
                _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[1]);
                *entry_cnt_type = 2;
            } else { /* (action->dt_outer == bcmVlanActionNone) */
                if ((action->dt_inner == bcmVlanActionNone) &&
                    (action->dt_inner_prio != bcmVlanActionNone)) {
                    /* 
                      * One entries have to be created. 
                      * 0. STag = tagged or pri-tagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                    BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[0]);
                    *entry_cnt_type = 1;
                } else if ((action->dt_inner != bcmVlanActionNone) &&
                    (action->dt_inner_prio == bcmVlanActionNone)) {
                    /* 
                      * One entries have to be created. 
                      * 0. STag = tagged or pri-tagged, CTag = tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                    BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    *entry_cnt_type = 1;
                } else {
                    /*
                      * (action->dt_inner != bcmVlanActionNone) &&
                      * (action->dt_inner_prio != bcmVlanActionNone)
                      * Two entries have to be created. 
                      * 0. STag = tagged or pri-tagged, CTag = tagged
                      * 1. STag = tagged or pri-tagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                    BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                    BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[1]);
                    *entry_cnt_type = 2;
                }
            }
        } else { 
            /* 
              * (action->dt_outer != action->dt_outer_prio) &&
              * (action->dt_inner != action->dt_inner_prio) &&
              */
            if ((action->dt_outer == bcmVlanActionNone) &&
                (action->dt_outer_prio != bcmVlanActionNone)) {
                if ((action->dt_inner == bcmVlanActionNone) &&
                    (action->dt_inner_prio != bcmVlanActionNone)) {
                    /*
                      * 3 entries have to be created. 
                      * 0. STag = tagged, CTag = pri-tagged
                      * 1. STag = pri-tagged, CTag = tagged
                      * 2. STag = pri-tagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[2] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[1]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[2]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[2]);
                    *entry_cnt_type = 3;
                } else if ((action->dt_inner != bcmVlanActionNone) &&
                    (action->dt_inner_prio == bcmVlanActionNone)) {
                    /*
                      * 3 entries have to be created. 
                      * 0. STag = tagged, CTag = tagged
                      * 1. STag = pri-tagged, CTag = tagged
                      * 2. STag = pri-tagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[2] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[1]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[2]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[2]);
                    *entry_cnt_type = 3;
                } else {
                    /*
                      * (action->dt_inner != bcmVlanActionNone) &&
                      * (action->dt_inner_prio != bcmVlanActionNone)
                      * Four entries have to be created. 
                      * 0. STag = tagged, CTag = tagged
                      * 1. STag = tagged, CTag = pri-tagged
                      * 2. STag = pri-tagged, CTag = tagged
                      * 3. STag = pri-tagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    ivm_key_tag_status[2] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[3] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[1]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[2]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[2]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[3]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[3]);
                    *entry_cnt_type = 4;
                }
            } else if ((action->dt_outer != bcmVlanActionNone) &&
                (action->dt_outer_prio == bcmVlanActionNone)) {
                if ((action->dt_inner == bcmVlanActionNone) &&
                    (action->dt_inner_prio != bcmVlanActionNone)) {
                    /*
                      * 3 entries have to be created. 
                      * 0. STag = tagged, CTag = tagged
                      * 1. STag = tagged, CTag = pri-tagged
                      * 2. STag = pri-tagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    ivm_key_tag_status[2] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[1]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[2]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[2]);
                    *entry_cnt_type = 3;
                } else if ((action->dt_inner != bcmVlanActionNone) &&
                    (action->dt_inner_prio == bcmVlanActionNone)) {
                    /*
                      * Four entries have to be created. 
                      * 0. STag = tagged, CTag = tagged
                      * 1. STag = tagged, CTag = pri-tagged
                      * 2. STag = pri-tagged, CTag = tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    ivm_key_tag_status[2] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[1]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[2]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[2]);
                    *entry_cnt_type = 3;
                } else {
                    /*
                      * (action->dt_inner != bcmVlanActionNone) &&
                      * (action->dt_inner_prio != bcmVlanActionNone)
                      * Four entries have to be created. 
                      * 0. STag = tagged, CTag = tagged
                      * 1. STag = tagged, CTag = pri-tagged
                      * 2. STag = pri-tagged, CTag = tagged
                      * 3. STag = pri-tagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    ivm_key_tag_status[2] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                    ivm_key_tag_status[3] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                     BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                    _robo_vlan_action_map(action->dt_outer, &evm_act_stag[1]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[1]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[2]);
                    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[2]);
                    _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[3]);
                    _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[3]);
                    *entry_cnt_type = 4;
                }
            } else {
                /*
                  * (action->dt_outer != bcmVlanActionNone) &&
                  * (action->dt_outer_prio != bcmVlanActionNone)
                  * Four entries have to be created. 
                  * 0. STag = tagged, CTag = tagged
                  * 1. STag = tagged, CTag = pri-tagged
                  * 2. STag = pri-tagged, CTag = tagged
                  * 3. STag = pri-tagged, CTag = pri-tagged
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                ivm_key_tag_status[2] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                ivm_key_tag_status[3] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO |
                                                 BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                _robo_vlan_action_map(action->dt_outer, &evm_act_stag[0]);
                _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[0]);
                _robo_vlan_action_map(action->dt_outer, &evm_act_stag[1]);
                _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[1]);
                _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[2]);
                _robo_vlan_action_map(action->dt_inner, &evm_act_ctag[2]);
                _robo_vlan_action_map(action->dt_outer_prio, &evm_act_stag[3]);
                _robo_vlan_action_map(action->dt_inner_prio, &evm_act_ctag[3]);
                *entry_cnt_type = 4;
            }
        }
    } else if (tag_action_map & VLAN_ACTION_OT) {
        if (action->ot_outer == action->ot_outer_prio) {
            ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
                                             BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
            _robo_vlan_action_map(action->ot_outer, &evm_act_stag[0]);
            _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[0]);
            *entry_cnt_type = 1;
        } else {
            /* 
              * (action->ot_outer != action->ot_outer_prio)
              */
            if (action->ot_inner != bcmVlanActionNone) {
                /* 
                  * Two entries have to be created. 
                  * 0. STag = tagged, CTag = untagged
                  * 1. STag = pri-tagged, CTag = untagged
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
                ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
                _robo_vlan_action_map(action->ot_outer, &evm_act_stag[0]);
                _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[0]);
                _robo_vlan_action_map(action->ot_outer_prio, &evm_act_stag[1]);
                _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[1]);
                *entry_cnt_type = 2;
            } else { /* (action->ot_inner == bcmVlanActionNone) */
                if ((action->ot_outer == bcmVlanActionNone) &&
                    (action->ot_outer_prio != bcmVlanActionNone)) {
                    /* 
                      * One entries have to be created. 
                      * 0. STag = pri-tagged, CTag = untagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->ot_outer_prio, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[0]);
                    *entry_cnt_type = 1;
                } else if ((action->ot_outer != bcmVlanActionNone) &&
                    (action->ot_outer_prio == bcmVlanActionNone)) {
                        /* 
                          * One entries have to be created. 
                          * 0. STag = tagged, CTag = untagged
                          */
                        ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
                        _robo_vlan_action_map(action->ot_outer, &evm_act_stag[0]);
                        _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[0]);
                        *entry_cnt_type = 1;
                    } else {
                        /*
                          * (action->ot_outer != bcmVlanActionNone) &&
                          * (action->ot_outer_prio != bcmVlanActionNone)
                           * Two entries have to be created. 
                           * 0. STag = tagged, CTag = untagged
                           * 1. STag = pri-tagged, CTag = untagged
                              */
                        ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
                        ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
                        _robo_vlan_action_map(action->ot_outer, &evm_act_stag[0]);
                        _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[0]);
                        _robo_vlan_action_map(action->ot_outer_prio, &evm_act_stag[1]);
                        _robo_vlan_action_map(action->ot_inner, &evm_act_ctag[1]);
                        *entry_cnt_type = 2;
                    }

            }
        }
    } else if (tag_action_map & VLAN_ACTION_IT) {
        if (action->it_inner == action->it_inner_prio) {
            ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED |
                                             BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            _robo_vlan_action_map(action->it_outer, &evm_act_stag[0]);
            _robo_vlan_action_map(action->it_inner, &evm_act_ctag[0]);
            *entry_cnt_type = 1;
        } else {
            /* 
              * (action->it_inner != action->it_inner_prio)
              */
            if (action->it_outer != bcmVlanActionNone) {
                /* 
                  * Two entries have to be created. 
                  * 0. STag = untagged, CTag = tagged
                  * 1. STag = untagged, CTag = pri-tagged
                  */
                ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                _robo_vlan_action_map(action->it_outer, &evm_act_stag[0]);
                _robo_vlan_action_map(action->it_inner, &evm_act_ctag[0]);
                _robo_vlan_action_map(action->it_outer, &evm_act_stag[1]);
                _robo_vlan_action_map(action->it_inner_prio, &evm_act_ctag[1]);
                *entry_cnt_type = 2;
            } else { /* action->it_outer == bcmVlanActionNone */
                if ((action->it_inner == bcmVlanActionNone) &&
                    (action->it_inner_prio != bcmVlanActionNone)) {
                    /* 
                      * One entries have to be created. 
                      * 0. STag = untagged, CTag = pri-tagged
                      */
                    ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                    _robo_vlan_action_map(action->it_outer, &evm_act_stag[0]);
                    _robo_vlan_action_map(action->it_inner_prio, &evm_act_ctag[0]);
                    *entry_cnt_type = 1;
                } else if ((action->it_inner != bcmVlanActionNone) &&
                    (action->it_inner_prio == bcmVlanActionNone)) {
                        /* 
                          * One entries have to be created. 
                          * 0. STag = untagged, CTag = tagged
                          */
                        ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                        _robo_vlan_action_map(action->it_outer, &evm_act_stag[0]);
                        _robo_vlan_action_map(action->it_inner, &evm_act_ctag[0]);
                        *entry_cnt_type = 1;
                    } else {
                        /* 
                          * (action->it_inner != bcmVlanActionNone) &&
                          * (action->it_inner_prio != bcmVlanActionNone)
                           * Two entries have to be created. 
                           * 0. STag = untagged, CTag = tagged
                           * 1. STag = untagged, CTag = pri-tagged
                              */
                        ivm_key_tag_status[0] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
                        ivm_key_tag_status[1] = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
                        _robo_vlan_action_map(action->it_outer, &evm_act_stag[0]);
                        _robo_vlan_action_map(action->it_inner, &evm_act_ctag[0]);
                        _robo_vlan_action_map(action->it_outer, &evm_act_stag[1]);
                        _robo_vlan_action_map(action->it_inner_prio, &evm_act_ctag[1]);
                        *entry_cnt_type = 2;
                    }
            }
        }

    } else if (tag_action_map & VLAN_ACTION_UT) {
        ivm_key_tag_status[0] = 0;
        _robo_vlan_action_map(action->ut_outer, &evm_act_stag[0]);
        _robo_vlan_action_map(action->ut_inner, &evm_act_ctag[0]);
        *entry_cnt_type = 1;
    }

    return BCM_E_NONE;
}

/*
  * Parameters:
  * unit: unit
  * param: structure of input parameters.
  * action: structure of vlan translateion action.
  *
  * Note: The routine is used by several kinds of vlan translation APIs. 
  *          The usage among the APIs is depend on the vt_api_type parameter.
  */
/* 
  * For Thunderbolt, packets that hit EVM's rules have to be untagged 
  * by EVM's action if the egress ports are untagged member of a vlan.
  */
#define TB_EVM_UNTAG 1

STATIC int
_robo_vlan_translate_action_add(int unit,
                                  _robo_vlan_translate_action_param_t *param,
                                  bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_NONE;

    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 i, id;
    bcm_port_t local_port;
    bcm_field_group_t ivm_group, evm_group;
    bcm_field_entry_t ivm_entry;
    bcm_field_entry_t evm_entry;
    uint8 entry_cnt_total = 0; /* total entry count */
    uint8 entry_cnt_type = 0; /* entry count of each tag type(DT/OT/IT/UT) */
    bcm_field_entry_t ivm_entry_id[9];
#if TB_EVM_UNTAG
    bcm_pbmp_t pbmp;
    bcm_pbmp_t upbmp;
    bcm_pbmp_t org_pbmp;
    bcm_pbmp_t org_upbmp;
    bcm_port_t egr_port;
    int evm_create_fail = 0;
    _robo_vlan_evm_list_t *evm_lists_head = NULL;
    _robo_vlan_evm_list_t *evm_lists_tail = NULL;
    _robo_vlan_evm_list_t *evm_list = NULL;
    _robo_vlan_evm_list_t *evm_list_created[9];
#else
    bcm_field_entry_t evm_entry_id[9];
#endif
    bcm_field_range_t           rid = 0;
    _robo_vlan_translate_action_t *vt_act;
    uint32 ivm_key_port_data = 0, ivm_key_port_mask = 0;
    uint32 ivm_key_svid_data = 0, ivm_key_svid_mask = 0;
    uint32 ivm_key_cvid_data = 0, ivm_key_cvid_mask = 0;
    uint32 ivm_key_cvid_ranger_flag = 0;
    uint32 ivm_key_cvid_range_mask = 0;
    uint32 ivm_key_cpcp_data = 0, ivm_key_cpcp_mask = 0;
    uint32 ivm_key_tag_status[4];
    uint32 ivm_act_vid = 0, ivm_act_fid[9], ivm_act_vport_tc = 0;
    uint32 vm_act_pcp_mark = 0;
    uint32 evm_key_fid_data[9], evm_key_fid_mask = 0;
    uint32 evm_act_stag[4], evm_act_ctag[4];
    uint32 evm_act_svid = 0, evm_act_cvid = 0;
    uint32 tag_action_map = 0;
    uint32 qualifier_weight = 0;
    /* rv2 is specific for return value that no need to process or return. */
    __attribute__((unused))int rv2 = 0;
    bcm_gport_t port = 0;
    bcm_vlan_translate_key_t key_type = 0;
    uint32 param1 = 0;
    uint32 param2 = 0;
    uint32 vt_api_type = 0;
    uint32 recovery_flag = 0;
    uint32 temp = 0;

    BCM_IF_ERROR_RETURN(_bcm_robo_vlan_action_verify(unit, action));

    port = param->port;
    if (param->vt_api_type == VT_API_VLAN_PROTOCOL) {
        /* For protocol2vlan APIs, key_type is not applied. */
        key_type = bcmVlanTranslateKeyInvalid;
    } else {
        key_type = param->key_type;
    }
    param1 = param->param1;
    param2 = param->param2;
    vt_api_type = param->vt_api_type;
    if (vt_api_type & VT_API_VLAN_RECOVER) {
        /* 
          * If this time is a recovery call when updating VM entries failed. 
          * Set recovery_flag and remove VT_API_VLAN_RECOVER flag
          * for there is no processing of this flag later in this routine.
          */
        recovery_flag = 1;
        vt_api_type &= ~VT_API_VLAN_RECOVER;
    }

    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_CLEAR(upbmp);
    BCM_PBMP_CLEAR(org_pbmp);
    BCM_PBMP_CLEAR(org_upbmp);

    /* 
      * Check SW copy if the port+key_type+param1+param2+vt_api_type 
      * already be created.
      */
    vt_act = robo_vlan_translate_action_info;
    while (vt_act) {
       if ((vt_act->unit == unit) &&
           (vt_act->port == port) &&
           (vt_act->key_type == key_type) &&
        (vt_act->param1 == param1) &&
        (vt_act->param2 == param2) &&
           (vt_act->vt_api_type == vt_api_type)) {
           /* Already created */
           return BCM_E_EXISTS;
       }
        vt_act = vt_act->next;
    }


    /* Gport resolution */
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
        local_port = port;
    }

    sal_memset(ivm_entry_id, 0, sizeof(bcm_field_entry_t) * 9);
#if TB_EVM_UNTAG
    for (i = 0; i < 9; i++) {
        evm_list_created[i] = NULL;
    }
#else    
    sal_memset(evm_entry_id, 0, sizeof(bcm_field_entry_t) * 9);
#endif
    sal_memset(ivm_act_fid, 0, sizeof(uint32) * 9);
    sal_memset(evm_key_fid_data, 0, sizeof(uint32) * 9);

    if ((vt_api_type == VT_API_VLAN_PROTOCOL) ||
        (vt_api_type == VT_API_VLAN_PORT_DEFAULT)) {
        /*
          * Port default vlan action and protocol2vlan APIs 
          * only qualifies InPort key.
          */
        ivm_key_port_data = local_port;
        ivm_key_port_mask = VM_PORT_MASK;

    } else {
        /*
          * key_type resolution.
          */

        /* Vlan id validation */
        _ROBO_VLAN_CHK_ID(unit, param1);
        _ROBO_VLAN_CHK_ID(unit, param2);

        switch (key_type) {
            case bcmVlanTranslateKeyPortDouble:
                ivm_key_port_data = local_port;
                ivm_key_port_mask = VM_PORT_MASK;
                /* fall through */
            case bcmVlanTranslateKeyDouble:
                ivm_key_svid_data = param1;
                ivm_key_svid_mask = VM_VID_MASK;
                ivm_key_cvid_data = param2;
                ivm_key_cvid_mask = VM_VID_MASK;
                break;
            case bcmVlanTranslateKeyPortOuter:
                ivm_key_port_data = local_port;
                ivm_key_port_mask = VM_PORT_MASK;
                /* fall through */
            case bcmVlanTranslateKeyOuter:
                if ((vt_api_type == VT_API_VLAN_XLATE_ACTION_RANGE) ||
                    (vt_api_type == VT_API_VLAN_XLATE_RANGE)||
                    (vt_api_type == VT_API_VLAN_DTAG_RANGE)){
                    /* SVID range check is not supported in BCM53280. */
                    return BCM_E_UNAVAIL;
                } else {
                    ivm_key_svid_data = param1;
                    ivm_key_svid_mask = VM_VID_MASK;
                }
                break;
            case bcmVlanTranslateKeyPortInner:
                ivm_key_port_data = local_port;
                ivm_key_port_mask = VM_PORT_MASK;
                /* fall through */
            case bcmVlanTranslateKeyInner:
                if ((vt_api_type == VT_API_VLAN_XLATE_ACTION_RANGE) ||
                    (vt_api_type == VT_API_VLAN_XLATE_RANGE)||
                    (vt_api_type == VT_API_VLAN_DTAG_RANGE)){
                    /* CVID range id is determined later when create FP ranger. */
                    ivm_key_cvid_range_mask = VM_VID_RANGE_MASK;
                    /* CVID is not necessary to be qualified in this case. */
                } else {
                    ivm_key_cvid_data = param2;
                    ivm_key_cvid_mask = VM_VID_MASK;
                }
                break;
            case bcmVlanTranslateKeyPortInnerTag:
                ivm_key_port_data = local_port;
                ivm_key_port_mask = VM_PORT_MASK;
                /* fall through */
            case bcmVlanTranslateKeyInnerTag:
                ivm_key_cvid_data = BCM_VLAN_CTRL_ID(param2);
                ivm_key_cvid_mask = VM_VID_MASK;
                ivm_key_cpcp_data = BCM_VLAN_CTRL_PRIO(param2);
                ivm_key_cpcp_mask = VM_PCP_MASK;
                break;
            case bcmVlanTranslateKeyOuterTag:
            case bcmVlanTranslateKeyOuterPri:
            case bcmVlanTranslateKeyPortOuterTag:
            case bcmVlanTranslateKeyPortOuterPri:
                /* Not supported key_type */
                return BCM_E_UNAVAIL;
            default:
                return BCM_E_PARAM;
        }
    }


    ivm_act_vid = action->new_outer_vlan;
    ivm_act_vport_tc = action->priority;

    evm_key_fid_mask = VM_FID_MASK;

    evm_act_svid = action->new_outer_vlan;
    evm_act_cvid = action->new_inner_vlan;

    /* 
      * Get the types of tag status of this call by the configuration 
      * of action.
      * Returned tag_action_map could be the combination of,
      * - VLAN_ACTION_DT
      * - VLAN_ACTION_OT
      * - VLAN_ACTION_IT
      * - VLAN_ACTION_UT
      */
    BCM_IF_ERROR_RETURN(
        _robo_vlan_tag_resolution(&tag_action_map, 
                action));

    /* If tag_action_map = 0, it means actions = None (bcmVlanActionNone) */

    while (tag_action_map) {
        /* 
          * Process 4 kinds of tag status(if needed).
          * Each loop process one kind.
          */
        sal_memset(ivm_key_tag_status, 0, sizeof(uint32) * 4);
        sal_memset(evm_act_stag, 0, sizeof(uint32) * 4);
        sal_memset(evm_act_ctag, 0, sizeof(uint32) * 4);

        /*
          * 1. Action resolution and pcp resolution.
          * 2. VM entry creation.
          * 3. Save SW copy for bcm_vlan_translate_action_delete/delete_all/get/traverse()
          */
    
        /*
         * 1. Action resolution and pcp resolution.
         */
        BCM_IF_ERROR_RETURN(
            _robo_vlan_action_common_resolution(tag_action_map, 
                    ivm_key_tag_status, 
                    evm_act_stag, evm_act_ctag, 
                    &entry_cnt_type,
                    param,
                    action));

        BCM_IF_ERROR_RETURN(
            _robo_vlan_pcp_resolution(tag_action_map, 
                    &vm_act_pcp_mark, 
                    action));

        /* 2. Create VM entry */
        /* Prepare Qualifiers Set and create Field group */
        if (vt_ivm_group_id[unit]) {
            ivm_group = vt_ivm_group_id[unit];
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_vlan_vm_group_create(unit, 
                    BCM_ROBO_VM_IVM_GROUP, &vt_ivm_group_id[unit]));
            ivm_group = vt_ivm_group_id[unit];
        }
        if (vt_evm_group_id[unit]) {
            evm_group = vt_evm_group_id[unit];
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_vlan_vm_group_create(unit, 
                    BCM_ROBO_VM_EVM_GROUP, &vt_evm_group_id[unit]));
            evm_group = vt_evm_group_id[unit];
        }

        /* Create entry by Field APIs */
        for (i = 0; i < entry_cnt_type; i++) {
            /* Get Flow Id for each of the creating entries */
            rv = DRV_VM_FLOW_ALLOC(unit, VM_FLOW_ID_GET_FROM_TAIL, &id);
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }
            ivm_act_fid[entry_cnt_total + i] = id;
            evm_key_fid_data[entry_cnt_total + i] = id;

            /* Create IVM entry */
            rv = bcm_field_entry_create(unit, ivm_group, &ivm_entry);
            ivm_entry_id[entry_cnt_total + i] = ivm_entry;
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            qualifier_weight = 0;

            /* IVM qualifiers */
            if (ivm_key_port_mask) {
                rv = bcm_field_qualify_InPort(unit, \
                    ivm_entry, ivm_key_port_data, ivm_key_port_mask);
                if (rv < 0) {
                    /* Release other allocated and entry-created FlowId */
                    entry_cnt_total += i; /* number of already created entries */
                    goto action_add_exit;
                }
                qualifier_weight ++;
            }
            if (ivm_key_svid_mask && 
            	(ivm_key_tag_status[i] & BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED)) {
                rv = bcm_field_qualify_OuterVlanId(unit, \
                    ivm_entry, ivm_key_svid_data, ivm_key_svid_mask);
                if (rv < 0) {
                    /* Release other allocated and entry-created FlowId */
                    entry_cnt_total += i; /* number of already created entries */
                    goto action_add_exit;
                }
                qualifier_weight ++;
            }
            if (ivm_key_cvid_mask && 
            	(ivm_key_tag_status[i] & BCM_FIELD_VLAN_FORMAT_INNER_TAGGED)) {
                rv = bcm_field_qualify_InnerVlanId(unit, \
                    ivm_entry, ivm_key_cvid_data, ivm_key_cvid_mask);
                if (rv < 0) {
                    /* Release other allocated and entry-created FlowId */
                    entry_cnt_total += i; /* number of already created entries */
                    goto action_add_exit;
                }
                qualifier_weight ++;
            }

            if (ivm_key_cpcp_mask) {
                rv = bcm_field_qualify_InnerVlanPri(unit, \
                    ivm_entry, ivm_key_cpcp_data, ivm_key_cpcp_mask);
                if (rv < 0) {
                    /* Release other allocated and entry-created FlowId */
                    entry_cnt_total += i; /* number of already created entries */
                    goto action_add_exit;
                }
                qualifier_weight ++;
            }
    
            rv = bcm_field_qualify_VlanFormat(unit, \
                ivm_entry, ivm_key_tag_status[i], ivm_key_tag_status[i]);
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }
            qualifier_weight += 2; /* Stag status and Ctag status */
    
            if (vt_api_type == VT_API_VLAN_PROTOCOL) {
                if (param1) { /* Frame type */
                    switch (param1) {
                        case BCM_PORT_FRAMETYPE_ETHER2:
                            temp = bcmFieldL2FormatEthII;
                            break;
                        case BCM_PORT_FRAMETYPE_SNAP:
                            temp = bcmFieldL2FormatSnap;
                            break;
                        case BCM_PORT_FRAMETYPE_LLC:
                            temp = bcmFieldL2FormatLlc;
                            break;
                        case BCM_PORT_FRAMETYPE_SNAP_PRIVATE:
                            temp = bcmFieldL2FormatSnapPrivate;
                            break;
                        default:
                        	rv = BCM_E_PARAM;
                        	entry_cnt_total += i; /* number of already created entries */
                        	goto action_add_exit;
                        	break;                
                        }
                    rv = bcm_field_qualify_L2Format(unit, \
                            ivm_entry, temp);
                    if (rv < 0) {
                        /* Release other allocated and entry-created FlowId */
                        entry_cnt_total += i; /* number of already created entries */
                        goto action_add_exit;
                    }
                    qualifier_weight ++;
                }
                if (param2) { /* Ether type */
                    rv = bcm_field_qualify_EtherType(unit, \
                            ivm_entry, param2, VM_ETHER_MASK);
                    if (rv < 0) {
                        /* Release other allocated and entry-created FlowId */
                        entry_cnt_total += i; /* number of already created entries */
                        goto action_add_exit;
                    }
                    qualifier_weight ++;\
                }
            }
            
            /* IVM actions */
            rv = bcm_field_action_add(unit, \
               ivm_entry, bcmFieldActionOuterVlanNew, ivm_act_vid, 0);
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }
            rv = bcm_field_action_add(unit, \
                ivm_entry, bcmFieldActionNewClassId, ivm_act_fid[entry_cnt_total + i] , 
                _bcmFieldActionPrivateUsed);
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            /* Set default Dp value to DP1(lowest drop precedence) */
            rv = bcm_field_action_add(unit, \
                ivm_entry, bcmFieldActionVportDpNew, 1, 0);
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            rv = bcm_field_action_add(unit, \
                ivm_entry, bcmFieldActionVportTcNew, ivm_act_vport_tc, 0);
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            rv = bcm_field_action_add(unit, \
                ivm_entry, bcmFieldActionPrioPktNew, 0, vm_act_pcp_mark);
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            /* Create FP ranger if needed. */
            if (ivm_key_cvid_range_mask) {
                /* Need to create a ranger. */
                if (!ivm_key_cvid_ranger_flag) {
                    /* First time create a ranger in this call. */
                    rv = bcm_field_range_create(unit, &rid,
                            ( BCM_FIELD_RANGE_INNER_VLAN |
                            BCM_FIELD_RANGE_LOOKUP), 
                                   param1, param2);
                    if (rv < 0) {
                        /* Release other allocated and entry-created FlowId */
                        entry_cnt_total += i; /* number of already created entries */
                        goto action_add_exit;
                    }
                    ivm_key_cvid_ranger_flag = 1;
                }
                rv = bcm_field_qualify_RangeCheck(unit, ivm_entry, rid, 0);
                if (rv < 0) {
                    /* Release other allocated and entry-created FlowId */
                    entry_cnt_total += i; /* number of already created entries */
                    goto action_add_exit;
                }
                qualifier_weight ++;
            }

            /* 
              * Set TCAM priority of the entry. action->action->priority is a 0-based 16bit variable. 
              * Since the priority value "0" is reserved for bcm_vlan_port/port_egress_default_action_xxx(),
              * the actual priority value set to th entry should start from 1.
              */
            if (vt_api_type == VT_API_VLAN_PORT_DEFAULT) {
                /* Port default actions should use the lowest priority in TCAM. */
                rv = bcm_field_entry_prio_set(unit, ivm_entry, BCM_FIELD_ENTRY_PRIO_LOWEST);
            } else {
                /* The TCAM entry's priority should be decided by the number of Keys be qualified. */
                rv = bcm_field_entry_prio_set(unit, ivm_entry, qualifier_weight);
            }
            if (rv < 0) {
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            /* Entry install */
            rv = bcm_field_entry_install(unit, ivm_entry);
            if (rv < 0) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);

                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }

                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            /* Create EVM entry */
#if TB_EVM_UNTAG 
           /* only tagged ports need the EVM translation. */
            if (recovery_flag) {
                BCM_PBMP_ASSIGN(pbmp, param->vlan_pbmp);
                BCM_PBMP_ASSIGN(upbmp, param->vlan_upbmp);
            } else {
                rv = bcm_vlan_port_get(unit, ivm_act_vid, &pbmp, &upbmp);
                if ((rv < 0) && (rv != BCM_E_NOT_FOUND)) {
                    /* Release current allocated FlowId */
                    rv2 = DRV_VM_FLOW_FREE(unit, id);
            
                    /* Decrease the apply-count of  the ranger id. */
                    if (ivm_key_cvid_range_mask) {
                        rv2 = bcm_field_range_destroy(unit, rid);
                    }
            
                    /* destroy IVM entry in this loop */
                    rv2 = bcm_field_entry_destroy(unit, ivm_entry);
            
                    /* Release other allocated and entry-created FlowId */
                    entry_cnt_total += i; /* number of already created entries */
                    goto action_add_exit;
                }
            
                if (rv == BCM_E_NOT_FOUND) {
                    BCM_PBMP_CLEAR(pbmp);
                    BCM_PBMP_CLEAR(upbmp);
                } else {
                    BCM_PBMP_REMOVE(pbmp, upbmp);
                }
            }
            BCM_PBMP_ASSIGN(org_pbmp, pbmp);
            BCM_PBMP_ASSIGN(org_upbmp, upbmp);
        
            /* Tagged ports */
            evm_create_fail = 0;
            evm_lists_head = NULL;
            evm_lists_tail = NULL;
            if (BCM_PBMP_IS_NULL(upbmp)) {
                /* All ports are tagged port, out port qualifier is not needed. */
                rv = bcm_field_entry_create(unit, evm_group, &evm_entry);
                if (rv < 0) {
                    evm_create_fail = 1;
                    break;
                }
        
                qualifier_weight = 0;
                /* EVM qualifiers */
                if (ivm_key_port_mask) {
                    /* Configure Ingress Port Id if available */
                    rv = bcm_field_qualify_InPort(unit, \
                       evm_entry, ivm_key_port_data, ivm_key_port_mask);
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
        
                    qualifier_weight++;
                }
                rv = bcm_field_qualify_FlowId(unit, \
                    evm_entry, evm_key_fid_data[entry_cnt_total + i], evm_key_fid_mask);
                if (rv < 0) {
                    evm_create_fail = 1;
                    break;
                }
                /* the weight of FlowId is 5, others are 1 */
                qualifier_weight += 5;
        
                /* EVM Actions */
                rv = bcm_field_action_add(unit, \
                    evm_entry, bcmFieldActionOuterVlanNew, evm_act_svid, evm_act_stag[i]);
                if (rv < 0) {
                    evm_create_fail = 1;
                    break;
                }
                rv = bcm_field_action_add(unit, \
                    evm_entry, bcmFieldActionInnerVlanNew, evm_act_cvid, evm_act_ctag[i]);
                if (rv < 0) {
                    evm_create_fail = 1;
                    break;
                }
        
                /* 
                  * Set TCAM priority of the entry. action->action->priority is a 0-based 16bit variable. 
                  * Since the priority value "0" is reserved for bcm_vlan_port/port_egress_default_action_xxx(),
                  * the actual priority value set to th entry should start from 1.
                  */
                if (vt_api_type == VT_API_VLAN_PORT_DEFAULT) {
                    /* Port default actions should use the lowest priority in TCAM. */
                    rv = bcm_field_entry_prio_set(unit, evm_entry, BCM_FIELD_ENTRY_PRIO_LOWEST);
                } else {
                    /* The TCAM entry's priority should be decided by the number of Keys be qualified. */
                    rv = bcm_field_entry_prio_set(unit, evm_entry, qualifier_weight);
                }
                if (rv < 0) {
                    evm_create_fail = 1;
                    break;
                }
        
                /* Entry install */
                rv = bcm_field_entry_install(unit, evm_entry);
                if (rv < 0) {
                    evm_create_fail = 1;
                    break;
                }
        
                evm_list = sal_alloc(sizeof(_robo_vlan_evm_list_t), "evm_list");
                if (!evm_list) {
                    rv = BCM_E_MEMORY;
                    evm_create_fail = 1;
                    break;
                }
                evm_list->entry_id = evm_entry;
                evm_list->next = NULL;
        
                evm_lists_head = evm_list;
                evm_lists_tail = evm_list;
            } else {
                BCM_PBMP_ITER(pbmp, egr_port) {
                    rv = bcm_field_entry_create(unit, evm_group, &evm_entry);
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
            
                    qualifier_weight = 0;
                    /* EVM qualifiers */
                    if (ivm_key_port_mask) {
                        /* Configure Ingress Port Id if available */
                        rv = bcm_field_qualify_InPort(unit, \
                           evm_entry, ivm_key_port_data, ivm_key_port_mask);
                        if (rv < 0) {
                            evm_create_fail = 1;
                            break;
                        }
            
                        qualifier_weight++;
                    }
                    rv = bcm_field_qualify_FlowId(unit, \
                        evm_entry, evm_key_fid_data[entry_cnt_total + i], evm_key_fid_mask);
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
                    /* the weight of FlowId is 5, others are 1 */
                    qualifier_weight += 5;
            
                    rv = bcm_field_qualify_OutPort(unit, \
                       evm_entry, egr_port, VM_PORT_MASK);
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
                    qualifier_weight++;
            
                    /* EVM Actions */
                    rv = bcm_field_action_add(unit, \
                        evm_entry, bcmFieldActionOuterVlanNew, evm_act_svid, evm_act_stag[i]);
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
                    rv = bcm_field_action_add(unit, \
                        evm_entry, bcmFieldActionInnerVlanNew, evm_act_cvid, evm_act_ctag[i]);
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
            
                    /* 
                      * Set TCAM priority of the entry. action->action->priority is a 0-based 16bit variable. 
                      * Since the priority value "0" is reserved for bcm_vlan_port/port_egress_default_action_xxx(),
                      * the actual priority value set to th entry should start from 1.
                      */
                    if (vt_api_type == VT_API_VLAN_PORT_DEFAULT) {
                        /* Port default actions should use the lowest priority in TCAM. */
                        rv = bcm_field_entry_prio_set(unit, evm_entry, BCM_FIELD_ENTRY_PRIO_LOWEST);
                    } else {
                        /* The TCAM entry's priority should be decided by the number of Keys be qualified. */
                        rv = bcm_field_entry_prio_set(unit, evm_entry, qualifier_weight);
                    }
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
            
                    /* Entry install */
                    rv = bcm_field_entry_install(unit, evm_entry);
                    if (rv < 0) {
                        evm_create_fail = 1;
                        break;
                    }
            
                    evm_list = sal_alloc(sizeof(_robo_vlan_evm_list_t), "evm_list");
                    if (!evm_list) {
                        rv = BCM_E_MEMORY;
                        evm_create_fail = 1;
                        break;
                    }

                    evm_list->entry_id = evm_entry;
                    evm_list->next = NULL;
                    if (!evm_lists_head) {
                        evm_lists_head = evm_list;
                    } else {
                        evm_lists_tail->next = evm_list;
                    }
                    evm_lists_tail = evm_list;
                }
            }
        
            evm_list_created[entry_cnt_total + i] = evm_lists_head;
            if (evm_create_fail) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);
        
                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }
        
                /* destroy IVM entry in this loop */
                rv2 = bcm_field_entry_destroy(unit, ivm_entry);
        
                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }
#else
            rv = bcm_field_entry_create(unit, evm_group, &evm_entry);
            evm_entry_id[entry_cnt_total + i] = evm_entry;
            if (rv < 0) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);

                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }
    
                /* destroy IVM entry in this loop */
                rv2 = bcm_field_entry_destroy(unit, ivm_entry);

                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            qualifier_weight = 0;

            /* EVM qualifiers */
            if (ivm_key_port_mask) {
                /* Configure Ingress Port Id if available */
                rv = bcm_field_qualify_InPort(unit, \
                   evm_entry, ivm_key_port_data, ivm_key_port_mask);
                if (rv < 0) {
                    /* Release current allocated FlowId */
                    rv2 = DRV_VM_FLOW_FREE(unit, id);
    
                    /* Decrease the apply-count of  the ranger id. */
                    if (ivm_key_cvid_range_mask) {
                        rv2 = bcm_field_range_destroy(unit, rid);
                    }
        
                    /* destroy IVM entry in this loop */
                    rv2 = bcm_field_entry_destroy(unit, ivm_entry);
    
                    /* Release other allocated and entry-created FlowId */
                    entry_cnt_total += i; /* number of already created entries */
                    goto action_add_exit;
                }

                qualifier_weight++;
            }
            rv = bcm_field_qualify_FlowId(unit, \
                evm_entry, evm_key_fid_data[entry_cnt_total + i], evm_key_fid_mask);
            if (rv < 0) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);

                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }
    
                /* destroy IVM entry in this loop */
                rv2 = bcm_field_entry_destroy(unit, ivm_entry);

                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }
            /* the weight of FlowId is 5, others are 1 */
            qualifier_weight += 5;

            /* EVM Actions */
            rv = bcm_field_action_add(unit, \
                evm_entry, bcmFieldActionOuterVlanNew, evm_act_svid, evm_act_stag[i]);
            if (rv < 0) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);

                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }
    
                /* destroy IVM entry in this loop */
                rv2 = bcm_field_entry_destroy(unit, ivm_entry);

                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }
            rv = bcm_field_action_add(unit, \
                evm_entry, bcmFieldActionInnerVlanNew, evm_act_cvid, evm_act_ctag[i]);
            if (rv < 0) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);

                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }
    
                /* destroy IVM entry in this loop */
                rv2 = bcm_field_entry_destroy(unit, ivm_entry);

                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

            /* 
              * Set TCAM priority of the entry. action->action->priority is a 0-based 16bit variable. 
              * Since the priority value "0" is reserved for bcm_vlan_port/port_egress_default_action_xxx(),
              * the actual priority value set to th entry should start from 1.
              */
            if (vt_api_type == VT_API_VLAN_PORT_DEFAULT) {
                /* Port default actions should use the lowest priority in TCAM. */
                rv = bcm_field_entry_prio_set(unit, evm_entry, BCM_FIELD_ENTRY_PRIO_LOWEST);
            } else {
                /* The TCAM entry's priority should be decided by the number of Keys be qualified. */
                rv = bcm_field_entry_prio_set(unit, evm_entry, qualifier_weight);
            }
            if (rv < 0) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);

                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }
    
                /* destroy IVM entry in this loop */
                rv2 = bcm_field_entry_destroy(unit, ivm_entry);

                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }

    
            /* Entry install */
            rv = bcm_field_entry_install(unit, evm_entry);
            if (rv < 0) {
                /* Release current allocated FlowId */
                rv2 = DRV_VM_FLOW_FREE(unit, id);

                /* Decrease the apply-count of  the ranger id. */
                if (ivm_key_cvid_range_mask) {
                    rv2 = bcm_field_range_destroy(unit, rid);
                }
    
                /* destroy IVM entry in this loop */
                rv2 = bcm_field_entry_destroy(unit, ivm_entry);

                /* Release other allocated and entry-created FlowId */
                entry_cnt_total += i; /* number of already created entries */
                goto action_add_exit;
            }
#endif

        }

        /* Total entry created in the action profile. */        
        entry_cnt_total += entry_cnt_type;

        /* Processing one of the DT/OT/IT/UT is done. */
        if (tag_action_map & VLAN_ACTION_DT) {
            tag_action_map &= ~VLAN_ACTION_DT;
        } else if (tag_action_map & VLAN_ACTION_OT) {
            tag_action_map &= ~VLAN_ACTION_OT;
        } else if (tag_action_map & VLAN_ACTION_IT) {
            tag_action_map &= ~VLAN_ACTION_IT;
        } else if (tag_action_map & VLAN_ACTION_UT) {
            tag_action_map &= ~VLAN_ACTION_UT;
        }
    }

    /*
      * 3. Save SW copy for bcm_vlan_translate_action_delete/delete_all/get/traverse()
      * Key: port, key_type, outer_vlan, inner_vlan.
      * Stored: (IN)action structure, number of created VM entries, created IVM/EVM id(s).
      */
    if ((vt_act = sal_alloc(sizeof (_robo_vlan_translate_action_t), "vlan_act_list")) == NULL) {
        /* Remove all created vm entries */
        rv = BCM_E_MEMORY;
        goto action_add_exit;
    }
    vt_act->unit = unit;
    vt_act->port = port;
    vt_act->key_type = key_type;
    vt_act->param1 = param1;
    vt_act->param2 = param2;
    vt_act->vt_api_type = vt_api_type;
    sal_memcpy(&vt_act->action, action, sizeof(bcm_vlan_action_set_t));
    vt_act->vm_entry_cnt = entry_cnt_total;
    for (i = 0; i < entry_cnt_total; i++) {
        /* Store IVM and EVM entry ids */
        vt_act->ivm_entry_id[i] = ivm_entry_id[i];
#if TB_EVM_UNTAG
        vt_act->evm_id_list[i] = evm_list_created[i];
#else
        vt_act->evm_entry_id[i] = evm_entry_id[i];
#endif
        /* Store corresponding Flow IDs */
        vt_act->flow_id[i] = ivm_act_fid[i];
    }
    vt_act->ranger_id = rid;
    vt_act->next = robo_vlan_translate_action_info;
    BCM_PBMP_ASSIGN(vt_act->vlan_pbmp, org_pbmp);
    BCM_PBMP_ASSIGN(vt_act->vlan_upbmp, org_upbmp);
    robo_vlan_translate_action_info = vt_act;

    /* Sync with soc layer, record vt enabled */
    if ((vt_api_type == VT_API_VLAN_XLATE_ACTION) ||
        (vt_api_type == VT_API_VLAN_XLATE) ||
        (vt_api_type == VT_API_VLAN_DTAG) ||
        (vt_api_type == VT_API_VLAN_XLATE_ACTION_RANGE) ||
        (vt_api_type == VT_API_VLAN_XLATE_RANGE) ||
        (vt_api_type == VT_API_VLAN_DTAG_RANGE)) {
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                        (unit, DRV_VLAN_PROP_V2V,TRUE));
    }
    return BCM_E_NONE;
    
action_add_exit:
    for (i = 0; i <= entry_cnt_total; i++) {
        /* Release allocated FlowId of each ivm+evm entries. */
        rv2 = DRV_VM_FLOW_FREE(unit, ivm_act_fid[i]);

        /* Decrease the apply-count of  the ranger id. */
        if (ivm_key_cvid_range_mask) {
            if (ivm_key_cvid_ranger_flag != 0) {
                rv2 = bcm_field_range_destroy(unit, rid);
            }
        }

        /* Delete created ivm/evm entries */
        rv2 = bcm_field_entry_destroy(unit, ivm_entry_id[i]);
#if TB_EVM_UNTAG
        /* Destroy created evm entries */
        evm_lists_head = evm_list_created[i];
        while (evm_lists_head) {
            evm_list = evm_lists_head;
            evm_lists_head = evm_lists_head->next;
            rv2 = bcm_field_entry_destroy(unit, evm_list->entry_id);
            sal_free(evm_list);
        }
        
#else
        rv2 = bcm_field_entry_destroy(unit, evm_entry_id[i]);
#endif
    }

    if (rv == BCM_E_RESOURCE) {
        /* 
          * If field entry create returns E_RESOURCE, change the return
          * value to E_FULL to compatible with the concept of
          * Vlan Translation Table.
          */
        rv = BCM_E_FULL;
    }

    return rv;
}

STATIC int
_robo_vlan_translate_action_delete(int unit,
                                  _robo_vlan_translate_action_param_t *param)
{
    _robo_vlan_translate_action_t *vt_act_current;
    _robo_vlan_translate_action_t *vt_act_prev = NULL;
    int i, rv = 0;
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    bcm_port_t local_port;
#if TB_EVM_UNTAG
    _robo_vlan_evm_list_t *evm_lists_head = NULL;
    _robo_vlan_evm_list_t *evm_list = NULL;
#endif
    bcm_gport_t port = 0;
    bcm_vlan_translate_key_t key_type = 0;
    uint32 param1 = 0;
    uint32 param2 = 0;
    uint32 vt_api_type = 0;

    port = param->port;
    key_type = param->key_type;
    param1 = param->param1;
    param2 = param->param2;
    vt_api_type = param->vt_api_type;

    /* For protocol2vlan APIs, key_type is not applied. */
    if ((vt_api_type != VT_API_VLAN_PROTOCOL) &&
        (vt_api_type != VT_API_VLAN_PORT_DEFAULT)) {
        /* key_type resolution */
        switch (key_type) {
            case bcmVlanTranslateKeyPortDouble:
            case bcmVlanTranslateKeyDouble:
            case bcmVlanTranslateKeyPortOuter:
            case bcmVlanTranslateKeyOuter:
            case bcmVlanTranslateKeyPortInner:
            case bcmVlanTranslateKeyInner:
            case bcmVlanTranslateKeyPortInnerTag:
            case bcmVlanTranslateKeyInnerTag:
                break;
            case bcmVlanTranslateKeyOuterTag:
            case bcmVlanTranslateKeyOuterPri:
            case bcmVlanTranslateKeyPortOuterTag:
            case bcmVlanTranslateKeyPortOuterPri:
                /* Not supported key_type */
                return BCM_E_UNAVAIL;
            default:
                return BCM_E_PARAM;
        }

        /* Vlan id validation */
        _ROBO_VLAN_CHK_ID(unit, param1);
        _ROBO_VLAN_CHK_ID(unit, param2);
    }

    /* Gport id validation */
    BCM_IF_ERROR_RETURN(
        _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));

    vt_act_current = robo_vlan_translate_action_info;
    while (vt_act_current) {
        if ((vt_act_current->unit == unit) &&
            (vt_act_current->port == port) &&
            (vt_act_current->vt_api_type == vt_api_type)) {
            /* 
              * vt_api_type == VT_API_VLAN_PORT_DEFAULT:
              *    Check port only.
              * vt_api_type == other API types:
              *    Check port/key_type/outer_vlan/inner_vlan.
              */
            if (vt_api_type != VT_API_VLAN_PORT_DEFAULT) {
                if (!((vt_act_current->key_type == key_type) &&
                    (vt_act_current->param1 == param1) &&
                    (vt_act_current->param2 == param2))) {
                    vt_act_prev = vt_act_current;
                    vt_act_current = vt_act_current->next;
                    continue;
                }
            }

            /* Found */
            /* Remove all vm entries */
            for (i = 0; i < vt_act_current->vm_entry_cnt; i++) {
                /* Release Flow Id. */
                rv = DRV_VM_FLOW_FREE(unit, vt_act_current->flow_id[i]);
                BCM_IF_ERROR_RETURN(rv);
                /* Decrease the apply-count of  the ranger id. */
                if ((vt_act_current->vt_api_type == VT_API_VLAN_XLATE_ACTION_RANGE) ||
                    (vt_act_current->vt_api_type == VT_API_VLAN_XLATE_RANGE)||
                    (vt_act_current->vt_api_type == VT_API_VLAN_DTAG_RANGE)){
                    rv = bcm_field_range_destroy(unit, vt_act_current->ranger_id);
                    BCM_IF_ERROR_RETURN(rv);
                }

                /* Destroy IVM and EVM entries. */
                rv = bcm_field_entry_destroy(unit, vt_act_current->ivm_entry_id[i]);
                BCM_IF_ERROR_RETURN(rv);
#if TB_EVM_UNTAG
                /* Destroy created evm entries */
                evm_lists_head = vt_act_current->evm_id_list[i];
                while (evm_lists_head) {
                    evm_list = evm_lists_head;
                    evm_lists_head = evm_lists_head->next;
        
                    rv = bcm_field_entry_destroy(unit, evm_list->entry_id);
                    sal_free(evm_list);
                }
#else
                rv = bcm_field_entry_destroy(unit, vt_act_current->evm_entry_id[i]);
                BCM_IF_ERROR_RETURN(rv);
#endif
            }


            /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_current->next;
            } else {
                robo_vlan_translate_action_info = vt_act_current->next;
            }

            sal_free(vt_act_current);
            return BCM_E_NONE;
        }

        vt_act_prev = vt_act_current;
        vt_act_current = vt_act_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_translate_action_delete_all(int unit, uint32 vt_api_type, bcm_gport_t port)
{
    _robo_vlan_translate_action_t *vt_act_current;
    _robo_vlan_translate_action_t *vt_act_prev = NULL;
    _robo_vlan_translate_action_t *vt_act_next = NULL;
    int i, rv = 0;
#if TB_EVM_UNTAG
    _robo_vlan_evm_list_t *evm_lists_head = NULL;
    _robo_vlan_evm_list_t *evm_list = NULL;
#endif
    int delete_flag = 0;

    vt_act_current = robo_vlan_translate_action_info;
    while (vt_act_current) {
        vt_act_next = vt_act_current->next;

        delete_flag = 0;
        if (vt_act_current->unit == unit) {
            if (vt_api_type == VT_API_ANY) {
                delete_flag = 1;
            } else if (vt_api_type == vt_act_current->vt_api_type) {
                if (vt_api_type == VT_API_VLAN_PROTOCOL){
                    if (vt_act_current->port == port) {
                        delete_flag = 1;
                    } else {
                        delete_flag = 0;
                    }
                } else {
                    delete_flag = 1;
                }
            } else {
                delete_flag = 0;
            }
        } else {
            delete_flag = 0;
        }

        if (delete_flag) {
            /* Remove all vm entries */
            for (i = 0; i < vt_act_current->vm_entry_cnt; i++) {
                /* Release Flow Id. */
                rv = DRV_VM_FLOW_FREE(unit, vt_act_current->flow_id[i]);
                BCM_IF_ERROR_RETURN(rv);
                /* Decrease the apply-count of  the ranger id. */
                if ((vt_act_current->vt_api_type == VT_API_VLAN_XLATE_ACTION_RANGE) ||
                    (vt_act_current->vt_api_type == VT_API_VLAN_XLATE_RANGE)||
                    (vt_act_current->vt_api_type == VT_API_VLAN_DTAG_RANGE)) {
                    rv = bcm_field_range_destroy(unit, vt_act_current->ranger_id);
                    BCM_IF_ERROR_RETURN(rv);
                }

                /* Destroy IVM and EVM entries. */
                rv = bcm_field_entry_destroy(unit, vt_act_current->ivm_entry_id[i]);
                BCM_IF_ERROR_RETURN(rv);
#if TB_EVM_UNTAG
                /* Destroy created evm entries */
                evm_lists_head = vt_act_current->evm_id_list[i];
                while (evm_lists_head) {
                    evm_list = evm_lists_head;
                    evm_lists_head = evm_lists_head->next;
        
                    rv = bcm_field_entry_destroy(unit, evm_list->entry_id);
                    BCM_IF_ERROR_RETURN(rv);
                    sal_free(evm_list);
                }
#else
                rv = bcm_field_entry_destroy(unit, vt_act_current->evm_entry_id[i]);
#endif
            }

    
            /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_next;
            } else {
                robo_vlan_translate_action_info = vt_act_next;
            }

            sal_free(vt_act_current);
        } else {
            vt_act_prev = vt_act_current;
        }
        vt_act_current = vt_act_next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_translate_action_get(int unit,
                                  _robo_vlan_translate_action_param_t *param,
                                  bcm_vlan_action_set_t *action)
{
    _robo_vlan_translate_action_t *vt_act_current;
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    bcm_port_t local_port;
    bcm_gport_t port = 0;
    bcm_vlan_translate_key_t key_type = 0;
    uint32 param1 = 0;
    uint32 param2 = 0;
    uint32 vt_api_type = 0;

    port = param->port;
    key_type = param->key_type;
    param1 = param->param1;
    param2 = param->param2;
    vt_api_type = param->vt_api_type;

    /* For protocol2vlan APIs, key_type is not applied. */
    if ((vt_api_type != VT_API_VLAN_PROTOCOL) &&
        (vt_api_type != VT_API_VLAN_PORT_DEFAULT)) {
        /* key_type resolution */
        switch (key_type) {
            case bcmVlanTranslateKeyPortDouble:
            case bcmVlanTranslateKeyDouble:
            case bcmVlanTranslateKeyPortOuter:
            case bcmVlanTranslateKeyOuter:
            case bcmVlanTranslateKeyPortInner:
            case bcmVlanTranslateKeyInner:
            case bcmVlanTranslateKeyPortInnerTag:
            case bcmVlanTranslateKeyInnerTag:
                break;
            case bcmVlanTranslateKeyOuterTag:
            case bcmVlanTranslateKeyOuterPri:
            case bcmVlanTranslateKeyPortOuterTag:
            case bcmVlanTranslateKeyPortOuterPri:
                /* Not supported key_type */
                return BCM_E_UNAVAIL;
            default:
                return BCM_E_PARAM;
        }

        /* Vlan id validation */
        _ROBO_VLAN_CHK_ID(unit, param1);
        _ROBO_VLAN_CHK_ID(unit, param2);
    }

    /* Gport id validation */
    BCM_IF_ERROR_RETURN(
        _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));

    vt_act_current = robo_vlan_translate_action_info;
    while (vt_act_current) {
        if ((vt_act_current->unit == unit) &&
            (vt_act_current->port == port) &&
            (vt_act_current->vt_api_type == vt_api_type)) {
            /* 
              * vt_api_type == VT_API_VLAN_PORT_DEFAULT:
              *    Check port only.
              * vt_api_type == other API types:
              *    Check port/key_type/outer_vlan/inner_vlan.
              */
            if (vt_api_type != VT_API_VLAN_PORT_DEFAULT) {
                if (!((vt_act_current->key_type == key_type) &&
                    (vt_act_current->param1 == param1) &&
                    (vt_act_current->param2 == param2))) {
                    vt_act_current = vt_act_current->next;
                    continue;
                }
            }

            /* Found */
            sal_memcpy(action, &vt_act_current->action, sizeof(bcm_vlan_action_set_t));
            return BCM_E_NONE;
        }

        vt_act_current = vt_act_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_translate_action_traverse(int unit, 
                                   bcm_vlan_translate_action_traverse_cb cb, 
                                   void *user_data)
{
    _robo_vlan_translate_action_t *vt_act_current;

    vt_act_current = robo_vlan_translate_action_info;
    while (vt_act_current) {
        if ((vt_act_current->unit == unit) &&
            (vt_act_current->vt_api_type == VT_API_VLAN_XLATE_ACTION)) {
            /* Call application call-back */
            cb(unit, vt_act_current->port, vt_act_current->key_type, 
                vt_act_current->param1, vt_act_current->param2, 
                &vt_act_current->action, user_data);
        }

        vt_act_current = vt_act_current->next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_translate_action_update(int unit,
                                  bcm_vlan_t vlan_id)
{
    _robo_vlan_translate_action_t *vt_act_current;
    _robo_vlan_translate_action_t *vt_act_next;
    int i = 0;
    int vt_act_cnt = 0;
    bcm_vlan_action_set_t action_current;
    int rv = BCM_E_NONE;
    _robo_vlan_translate_action_param_t vt_act_param;
    
    vt_act_current = robo_vlan_translate_action_info;
    /* Get the number of existed members */
    while (vt_act_current) {
        vt_act_cnt++;
        vt_act_current = vt_act_current->next;
    }

    vt_act_current = robo_vlan_translate_action_info;
    /* Update the existed members */
    for (i = 0; i < vt_act_cnt; i++) {
        vt_act_next = vt_act_current->next;

        sal_memcpy(&action_current, &vt_act_current->action, sizeof(bcm_vlan_action_set_t));
        if (action_current.new_outer_vlan == vlan_id) {
            /* Get necessary parameters from the matched structure */
            sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
            vt_act_param.port = vt_act_current->port;
            vt_act_param.key_type = vt_act_current->key_type;
            vt_act_param.param1 = vt_act_current->param1;
            vt_act_param.param2 = vt_act_current->param2;
            vt_act_param.vt_api_type = vt_act_current->vt_api_type;
            BCM_PBMP_ASSIGN(vt_act_param.vlan_pbmp, vt_act_current->vlan_pbmp);
            BCM_PBMP_ASSIGN(vt_act_param.vlan_upbmp, vt_act_current->vlan_upbmp);

            /* Delete the structure's hw rules */
            rv = _robo_vlan_translate_action_delete(unit, &vt_act_param);
            if (rv < 0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Updating vlan translation action failed\n")));
                return rv;
            }


            /* Set hw rules again to update the vlan tagged members */
            rv = _robo_vlan_translate_action_add(unit, &vt_act_param, &action_current);
            if (rv < 0) {
                /* try to recover original vt actions which are deleted. */
                vt_act_param.vt_api_type |= VT_API_VLAN_RECOVER;
                BCM_IF_ERROR_RETURN(
                    _robo_vlan_translate_action_add(unit, &vt_act_param, 
                    &action_current));
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Updating vlan translation action failed\n")));
                return rv;
            }
        }
        vt_act_current = vt_act_next;
    }

    return rv;
}


STATIC int
_robo_vlan_translate_traverse_info_add(int unit, bcm_gport_t  gport, bcm_vlan_t old_vid,
                           bcm_vlan_t new_vid, int prio)
{
    int rv = BCM_E_NONE;
    _robo_vlan_translate_traverse_t *vt_trv;

    if ((vt_trv = sal_alloc(sizeof (_robo_vlan_translate_traverse_t), "vt_traverse_list")) == NULL) {
        /* Remove all created vm entries */
        rv = BCM_E_MEMORY;
        return rv;
    }
    vt_trv->unit = unit;
    vt_trv->gport = gport;
    vt_trv->old_vlan= old_vid;
    vt_trv->new_vlan= new_vid;
    vt_trv->prio= prio;
    vt_trv->next = robo_vt_traverse_info;
    robo_vt_traverse_info = vt_trv;

    return rv;
}

STATIC int
_robo_vlan_translate_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t old_vid)
{
    _robo_vlan_translate_traverse_t *vt_trv_current;
    _robo_vlan_translate_traverse_t *vt_trv_prev = NULL;

    vt_trv_current = robo_vt_traverse_info;
    while (vt_trv_current) {
        if ((vt_trv_current->unit == unit) &&
            (vt_trv_current->gport == gport) &&
            (vt_trv_current->old_vlan == old_vid)) {
            /* Found */
            /* Remove SW copy */
            if (vt_trv_prev) {
                vt_trv_prev->next = vt_trv_current->next;
            } else {
                robo_vt_traverse_info = vt_trv_current->next;
            }
            sal_free(vt_trv_current);
            return BCM_E_NONE;
        }
        vt_trv_prev = vt_trv_current;
        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_translate_traverse_info_remove_all(int unit)
{
    _robo_vlan_translate_traverse_t *vt_act_current;
    _robo_vlan_translate_traverse_t *vt_act_prev = NULL;
    _robo_vlan_translate_traverse_t *vt_act_next = NULL;

    vt_act_current = robo_vt_traverse_info;
    while (vt_act_current) {
        vt_act_next = vt_act_current->next;
        if (vt_act_current->unit == unit) {
        /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_next;
            } else {
                robo_vt_traverse_info = vt_act_next;
            }

            sal_free(vt_act_current);
        } else {
            vt_act_prev = vt_act_current;
        }

        vt_act_current = vt_act_next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_translate_traverse(int unit, 
                                   bcm_vlan_translate_traverse_cb cb, 
                                   void *user_data)
{
    _robo_vlan_translate_traverse_t *vt_trv_current;

    vt_trv_current = robo_vt_traverse_info;
    while (vt_trv_current) {
        /* Call application call-back */
        if (vt_trv_current->unit == unit) {
            cb(unit, vt_trv_current->gport, 
                vt_trv_current->old_vlan,
                vt_trv_current->new_vlan,
                vt_trv_current->prio,
                user_data);
        }
        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_dtag_traverse_info_add(int unit, bcm_gport_t gport, bcm_vlan_t inner_vid,
                           bcm_vlan_t outer_vid, int prio)
{
    int rv = BCM_E_NONE;
    _robo_vlan_dtag_traverse_t *dtag_trv;

    if ((dtag_trv = sal_alloc(sizeof (_robo_vlan_dtag_traverse_t), "dtag_traverse_list")) == NULL) {
        /* Remove all created vm entries */
        rv = BCM_E_MEMORY;
        return rv;
    }
    dtag_trv->unit = unit;
    dtag_trv->gport = gport;
    dtag_trv->inner_vlan= inner_vid;
    dtag_trv->outer_vlan= outer_vid;
    dtag_trv->prio= prio;
    dtag_trv->next = robo_dtag_traverse_info;
    robo_dtag_traverse_info = dtag_trv;

    return rv;
}

STATIC int
_robo_vlan_dtag_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t inner_vid)
{
    _robo_vlan_dtag_traverse_t *dtag_trv_current;
    _robo_vlan_dtag_traverse_t *dtag_trv_prev = NULL;

    dtag_trv_current = robo_dtag_traverse_info;
    while (dtag_trv_current) {
        if ((dtag_trv_current->unit == unit) &&
            (dtag_trv_current->gport == gport) &&
            (dtag_trv_current->inner_vlan== inner_vid)) {
            /* Found */
            /* Remove SW copy */
            if (dtag_trv_prev) {
                dtag_trv_prev->next = dtag_trv_current->next;
            } else {
                robo_dtag_traverse_info = dtag_trv_current->next;
            }
            sal_free(dtag_trv_current);
            return BCM_E_NONE;
        }
        dtag_trv_prev = dtag_trv_current;
        dtag_trv_current = dtag_trv_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_dtag_traverse_info_remove_all(int unit)
{
    _robo_vlan_dtag_traverse_t *dtag_act_current;
    _robo_vlan_dtag_traverse_t *dtag_act_prev = NULL;
    _robo_vlan_dtag_traverse_t *dtag_act_next = NULL;

    dtag_act_current = robo_dtag_traverse_info;
    while (dtag_act_current) {
        dtag_act_next = dtag_act_current->next;
        if (dtag_act_current->unit == unit) {
            /* Remove SW copy */
            if (dtag_act_prev) {
                dtag_act_prev->next = dtag_act_next;
            } else {
                robo_dtag_traverse_info = dtag_act_next;
            }
            
            sal_free(dtag_act_current);
        } else {
            dtag_act_prev = dtag_act_current;
        }

        dtag_act_current = dtag_act_next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_dtag_traverse(int unit, 
                                   bcm_vlan_dtag_traverse_cb cb, 
                                   void *user_data)
{
    _robo_vlan_dtag_traverse_t *dtag_trv_current;

    dtag_trv_current = robo_dtag_traverse_info;
    while (dtag_trv_current) {
        if (dtag_trv_current->unit == unit) {
            /* Call application call-back */
            cb(unit, dtag_trv_current->gport, 
                dtag_trv_current->inner_vlan,
                dtag_trv_current->outer_vlan,
                dtag_trv_current->prio,
                user_data);
        }

        dtag_trv_current =dtag_trv_current->next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_translate_action_range_traverse_info_add(int unit,
                           int port, bcm_vlan_t inner_vlan_low,
                           bcm_vlan_t inner_vlan_high, bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_NONE;
    _robo_vlan_translate_action_range_traverse_t *vt_trv;

    if ((vt_trv = sal_alloc
        (sizeof (_robo_vlan_translate_action_range_traverse_t), "vt_traverse_list")) == NULL) {
        /* Remove all created vm entries */
        rv = BCM_E_MEMORY;
        return rv;
    }
    vt_trv->unit = unit;
    vt_trv->port = port;
    vt_trv->inner_vlan_low = inner_vlan_low;
    vt_trv->inner_vlan_high= inner_vlan_high;
    vt_trv->next = robo_vt_act_range_traverse_info;
    sal_memcpy(&vt_trv->action, action, sizeof(bcm_vlan_action_set_t));
    robo_vt_act_range_traverse_info = vt_trv;

    return rv;
}

STATIC int
_robo_vlan_translate_action_range_traverse_info_remove(int unit, int port, 
                           bcm_vlan_t inner_vlan_low,
                           bcm_vlan_t inner_vlan_high)
{
    _robo_vlan_translate_action_range_traverse_t *vt_trv_current;
    _robo_vlan_translate_action_range_traverse_t *vt_trv_prev = NULL;

    vt_trv_current = robo_vt_act_range_traverse_info;
    while (vt_trv_current) {
        if ((vt_trv_current->unit == unit) &&
            (vt_trv_current->port == port) &&
            (vt_trv_current->inner_vlan_low == inner_vlan_low)&&
            (vt_trv_current->inner_vlan_high == inner_vlan_high)) {
            /* Found */
            /* Remove SW copy */
            if (vt_trv_prev) {
                vt_trv_prev->next = vt_trv_current->next;
            } else {
                robo_vt_act_range_traverse_info = vt_trv_current->next;
            }
            sal_free(vt_trv_current);
            return BCM_E_NONE;
        }
        vt_trv_prev = vt_trv_current;
        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_translate_action_range_traverse_info_remove_all(int unit)
{
    _robo_vlan_translate_action_range_traverse_t *vt_act_current;
    _robo_vlan_translate_action_range_traverse_t *vt_act_prev = NULL;
    _robo_vlan_translate_action_range_traverse_t *vt_act_next = NULL;

    vt_act_current = robo_vt_act_range_traverse_info;
    while (vt_act_current) {
        vt_act_next = vt_act_current->next;
        if (vt_act_current->unit == unit) {
            /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_next;
            } else {
                robo_vt_act_range_traverse_info = vt_act_next;
            }
            
            sal_free(vt_act_current);
        } else {
            vt_act_prev = vt_act_current;
        }

        vt_act_current = vt_act_next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_translate_action_range_traverse(int unit, 
                                   bcm_vlan_translate_action_range_traverse_cb cb, 
                                   void *user_data)
{
    _robo_vlan_translate_action_range_traverse_t *vt_trv_current;

    vt_trv_current = robo_vt_act_range_traverse_info;
    while (vt_trv_current) {
        if (vt_trv_current->unit == unit) {
            /* Call application call-back */
            cb(unit, vt_trv_current->port, 
                BCM_VLAN_INVALID, BCM_VLAN_INVALID,
                vt_trv_current->inner_vlan_low, 
                vt_trv_current->inner_vlan_high,
                &vt_trv_current->action,
                user_data);
        }

        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_translate_range_traverse_info_add(int unit, bcm_gport_t gport, 
                                 bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high, 
                                 bcm_vlan_t new_vid, int int_prio)
{
    int rv = BCM_E_NONE;
    _robo_vlan_translate_range_traverse_t *vt_trv;

    if ((vt_trv = sal_alloc(sizeof (_robo_vlan_translate_range_traverse_t), "vt_range_traverse_list")) == NULL) {
        /* Remove all created vm entries */
        rv = BCM_E_MEMORY;
        return rv;
    }
    vt_trv->unit = unit;
    vt_trv->gport = gport;
    vt_trv->old_vlan_low= old_vid_low;
    vt_trv->old_vlan_high= old_vid_high;
    vt_trv->new_vlan= new_vid;
    vt_trv->prio= int_prio;
    vt_trv->next = robo_vt_range_traverse_info;
    robo_vt_range_traverse_info = vt_trv;

    return rv;
}

STATIC int
_robo_vlan_translate_range_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high)
{
    _robo_vlan_translate_range_traverse_t *vt_trv_current;
    _robo_vlan_translate_range_traverse_t *vt_trv_prev = NULL;

    vt_trv_current = robo_vt_range_traverse_info;
    while (vt_trv_current) {
        if ((vt_trv_current->unit == unit) &&
            (vt_trv_current->gport == gport) &&
            (vt_trv_current->old_vlan_low == old_vid_low) &&
            (vt_trv_current->old_vlan_high == old_vid_high)) {
            /* Found */
            /* Remove SW copy */
            if (vt_trv_prev) {
                vt_trv_prev->next = vt_trv_current->next;
            } else {
                robo_vt_range_traverse_info = vt_trv_current->next;
            }
            sal_free(vt_trv_current);
            return BCM_E_NONE;
        }
        vt_trv_prev = vt_trv_current;
        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_translate_range_traverse_info_remove_all(int unit)
{
    _robo_vlan_translate_range_traverse_t *vt_act_current;
    _robo_vlan_translate_range_traverse_t *vt_act_prev = NULL;
    _robo_vlan_translate_range_traverse_t *vt_act_next = NULL;

    vt_act_current = robo_vt_range_traverse_info;
    while (vt_act_current) {
        vt_act_next = vt_act_current->next;
        if (vt_act_current->unit == unit) {
            /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_next;
            } else {
                robo_vt_range_traverse_info = vt_act_next;
            }
            
            sal_free(vt_act_current);
        } else {
            vt_act_prev = vt_act_current;
        }

        vt_act_current = vt_act_next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_translate_range_traverse(int unit, 
                                   bcm_vlan_translate_range_traverse_cb cb, 
                                   void *user_data)
{
    _robo_vlan_translate_range_traverse_t *vt_trv_current;

    vt_trv_current = robo_vt_range_traverse_info;
    while (vt_trv_current) {
        if (vt_trv_current->unit == unit) {
            /* Call application call-back */
            cb(unit, vt_trv_current->gport, 
                vt_trv_current->old_vlan_low,
                vt_trv_current->old_vlan_high,
                vt_trv_current->new_vlan,
                vt_trv_current->prio,
                user_data);
        }

        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_dtag_range_traverse_info_add(int unit, bcm_gport_t gport, 
                                 bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high, 
                                 bcm_vlan_t new_vid, int int_prio)
{
    int rv = BCM_E_NONE;
    _robo_vlan_dtag_range_traverse_t *vt_trv;

    if ((vt_trv = sal_alloc(sizeof (_robo_vlan_dtag_range_traverse_t), "vt_dtag_range_traverse_list")) == NULL) {
        /* Remove all created vm entries */
        rv = BCM_E_MEMORY;
        return rv;
    }
    vt_trv->unit = unit;
    vt_trv->gport = gport;
    vt_trv->old_vlan_low= old_vid_low;
    vt_trv->old_vlan_high= old_vid_high;
    vt_trv->new_vlan= new_vid;
    vt_trv->prio= int_prio;
    vt_trv->next = robo_vt_dtag_range_traverse_info;
    robo_vt_dtag_range_traverse_info = vt_trv;

    return rv;
}

STATIC int
_robo_vlan_dtag_range_traverse_info_remove(int unit, bcm_gport_t gport, 
                           bcm_vlan_t old_vid_low, bcm_vlan_t old_vid_high)
{
    _robo_vlan_dtag_range_traverse_t *vt_trv_current;
    _robo_vlan_dtag_range_traverse_t *vt_trv_prev = NULL;

    vt_trv_current = robo_vt_dtag_range_traverse_info;
    while (vt_trv_current) {
        if ((vt_trv_current->unit == unit) &&
            (vt_trv_current->gport == gport) &&
            (vt_trv_current->old_vlan_low == old_vid_low) &&
            (vt_trv_current->old_vlan_high == old_vid_high)) {
            /* Found */
            /* Remove SW copy */
            if (vt_trv_prev) {
                vt_trv_prev->next = vt_trv_current->next;
            } else {
                robo_vt_dtag_range_traverse_info = vt_trv_current->next;
            }

            sal_free(vt_trv_current);
            return BCM_E_NONE;
        }
        vt_trv_prev = vt_trv_current;
        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_dtag_range_traverse_info_remove_all(int unit)
{
    _robo_vlan_dtag_range_traverse_t *vt_act_current;
    _robo_vlan_dtag_range_traverse_t *vt_act_prev = NULL;
    _robo_vlan_dtag_range_traverse_t *vt_act_next = NULL;

    vt_act_current = robo_vt_dtag_range_traverse_info;
    while (vt_act_current) {
        vt_act_next = vt_act_current->next;
        if (vt_act_current->unit == unit) {
            /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_next;
            } else {
                robo_vt_dtag_range_traverse_info = vt_act_next;
            }
            
            sal_free(vt_act_current);
        } else {
            vt_act_prev = vt_act_current;
        }

        vt_act_current = vt_act_next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_dtag_range_traverse(int unit, 
                                   bcm_vlan_dtag_range_traverse_cb cb, 
                                   void *user_data)
{
    _robo_vlan_dtag_range_traverse_t *vt_trv_current;

    vt_trv_current = robo_vt_dtag_range_traverse_info;
    while (vt_trv_current) {
        if (vt_trv_current->unit == unit) {
            /* Call application call-back */
            cb(unit, vt_trv_current->gport, 
                vt_trv_current->old_vlan_low,
                vt_trv_current->old_vlan_high,
                vt_trv_current->new_vlan,
                vt_trv_current->prio,
                user_data);
        }

        vt_trv_current = vt_trv_current->next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_protocol_action_traverse(int unit, 
                                   bcm_vlan_port_protocol_action_traverse_cb cb, 
                                   void *user_data)
{
    _robo_vlan_translate_action_t *vt_act_current;
    bcm_port_t local_port;
    bcm_module_t modid;
    bcm_trunk_t tgid;
    int id;

    vt_act_current = robo_vlan_translate_action_info;
    while (vt_act_current) {
        if ((vt_act_current->unit == unit) &&
            (vt_act_current->vt_api_type == VT_API_VLAN_PROTOCOL)) {
            /* 
             * The stored port in the sw copy is bcm_gport_t type.
              * But the callback function prototype of 
              * bcm_vlan_protocol_action_traverse() is bcm_port_t.
              * Need to do translation here.
              */
            BCM_IF_ERROR_RETURN(
               _bcm_robo_gport_resolve(unit, vt_act_current->port, 
               &modid, &local_port, &tgid, &id));

            /* Call application call-back */
            cb(unit, local_port, vt_act_current->param1, 
                vt_act_current->param2, 
                &vt_act_current->action, user_data);
        }

        vt_act_current = vt_act_current->next;
    }

    return BCM_E_NONE;
}

/*
  * Parameters:
  * unit: unit
  * port: local port id
  * action: structure of vlan translateion action.
  */
STATIC int
_robo_vlan_port_egress_default_action_set(int unit, bcm_port_t port,
                                            bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_NONE;

    bcm_field_group_t evm_group;
    bcm_field_entry_t evm_entry;
    _robo_vlan_port_egress_default_action_t *vt_act;
    uint32 evm_act_stag = 0, evm_act_ctag = 0;
    uint32 evm_act_svid = 0, evm_act_cvid = 0;

    BCM_IF_ERROR_RETURN(_bcm_robo_vlan_action_verify(unit, action));

    /* 
      * Check SW copy if the egress port already created.
      */
    vt_act = robo_vlan_port_egress_default_action_info;
    while (vt_act) {
       if ((vt_act->unit == unit) &&
           (vt_act->port == port)) {
           /* Already created */
           return BCM_E_EXISTS;
       }
        vt_act = vt_act->next;
    }

    evm_act_svid = action->new_outer_vlan;
    evm_act_cvid = action->new_inner_vlan;

    /*
      * 1. Action resolution.
      * 2. VM entry creation.
      * 3. Save SW copy for bcm_vlan_translate_action_delete/delete_all/get/traverse()
      */
    
    /*
     * 1. Action resolution.
     * Since the packets with in BCM53280 are always double tagged.
     * The API of BCM53280 only accept dt_outer and dt_inner actions.
     */
    _robo_vlan_action_map(action->dt_outer, &evm_act_stag);
    _robo_vlan_action_map(action->dt_inner, &evm_act_ctag);

    /* 2. Create EVM entry */
    if (vt_evm_group_id[unit]) {
        evm_group = vt_evm_group_id[unit];
    } else {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_vlan_vm_group_create(unit, 
                BCM_ROBO_VM_EVM_GROUP, &vt_evm_group_id[unit]));
        evm_group = vt_evm_group_id[unit];
    }

    /* Create EVM entry */
    rv = bcm_field_entry_create(unit, evm_group, &evm_entry);
    if (rv < 0) {
        goto egr_port_default_set_exit;
    }

    /* 
      * Port default actions always set Field priority to lowest.
      */
    rv = bcm_field_entry_prio_set(unit, evm_entry, BCM_FIELD_ENTRY_PRIO_LOWEST);
    if (rv < 0) {
        goto egr_port_default_set_exit;
    }

    /* EVM qualifiers */
    rv = bcm_field_qualify_OutPort(unit, \
       evm_entry, port, VM_PORT_MASK);
    if (rv < 0) {
        goto egr_port_default_set_exit;
    }

    /* EVM Actions */
    rv = bcm_field_action_add(unit, \
        evm_entry, bcmFieldActionOuterVlanNew, evm_act_svid, evm_act_stag);
    if (rv < 0) {
        goto egr_port_default_set_exit;
    }
    rv = bcm_field_action_add(unit, \
        evm_entry, bcmFieldActionInnerVlanNew, evm_act_cvid, evm_act_ctag);
    if (rv < 0) {
        goto egr_port_default_set_exit;
    }

    /* Entry install */
    rv = bcm_field_entry_install(unit, evm_entry);
    if (rv < 0) {
        goto egr_port_default_set_exit;
    }

    /*
      * 3. Save SW copy for bcm_vlan_port_egress_default_action_delete/get
      * Key: egress port id.
      * Stored: (IN)action structure, created EVM id.
      */
    if ((vt_act = sal_alloc(sizeof (_robo_vlan_port_egress_default_action_t), "vlan_egr_port_default_list")) == NULL) {
        rv = BCM_E_MEMORY;
        goto egr_port_default_set_exit;
    }
    vt_act->unit = unit;
    vt_act->port = port;
    sal_memcpy(&vt_act->action, action, sizeof(bcm_vlan_action_set_t));
    /* Store EVM entry ids */
    vt_act->evm_entry_id = evm_entry;
    vt_act->next = robo_vlan_port_egress_default_action_info;
    robo_vlan_port_egress_default_action_info = vt_act;

    return BCM_E_NONE;
    
egr_port_default_set_exit:

    if (rv == BCM_E_RESOURCE) {
        /* 
          * If field entry create returns E_RESOURCE, change the return
          * value to E_FULL to compatible with the concept of
          * Vlan Translation Table.
          */
        rv = BCM_E_FULL;
    }

    return rv;
}

STATIC int
_robo_vlan_port_egress_default_action_delete(int unit,
                                  bcm_port_t port)
{
    _robo_vlan_port_egress_default_action_t *vt_act_current;
    _robo_vlan_port_egress_default_action_t *vt_act_prev = NULL;
    int rv = 0;

    vt_act_current = robo_vlan_port_egress_default_action_info;
    while (vt_act_current) {
        if ((vt_act_current->unit== unit) &&
            (vt_act_current->port == port)) {
            /* Found */
            /* Destroy EVM entriey. */
            rv = bcm_field_entry_destroy(unit, vt_act_current->evm_entry_id);
            BCM_IF_ERROR_RETURN(rv);
            /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_current->next;
            } else {
                robo_vlan_port_egress_default_action_info = vt_act_current->next;
            }
            sal_free(vt_act_current);
            return BCM_E_NONE;
        }
        vt_act_prev = vt_act_current;
        vt_act_current = vt_act_current->next;
    }

    return BCM_E_NOT_FOUND;
}

STATIC int
_robo_vlan_port_egress_default_action_delete_all(int unit)
{
    _robo_vlan_port_egress_default_action_t *vt_act_current;
    _robo_vlan_port_egress_default_action_t *vt_act_prev = NULL;
    _robo_vlan_port_egress_default_action_t *vt_act_next = NULL;

    vt_act_current = robo_vlan_port_egress_default_action_info;
    while (vt_act_current) {
        vt_act_next = vt_act_current->next;
        if (vt_act_current->unit == unit) {
            /* Remove SW copy */
            if (vt_act_prev) {
                vt_act_prev->next = vt_act_next;
            } else {
                robo_vlan_port_egress_default_action_info = vt_act_next;
            }
            
            sal_free(vt_act_current);
        } else {
            vt_act_prev = vt_act_current;
        }

        vt_act_current = vt_act_next;
    }

    return BCM_E_NONE;
}

STATIC int
_robo_vlan_port_egress_default_action_get(int unit,
                                  bcm_port_t port,
                                  bcm_vlan_action_set_t *action)
{
    _robo_vlan_port_egress_default_action_t *vt_act_current;

    vt_act_current = robo_vlan_port_egress_default_action_info;
    while (vt_act_current) {
        if ((vt_act_current->unit == unit) &&
            (vt_act_current->port == port)) {
            /* Found */
            sal_memcpy(action, &vt_act_current->action, sizeof(bcm_vlan_action_set_t));
            return BCM_E_NONE;
        }
        vt_act_current = vt_act_current->next;
    }

    return BCM_E_NOT_FOUND;
}

#endif

/*
 * Function   :
 *      bcm_vlan_translate_action_add
 * Description   :
 *      Add an entry to ingress VLAN translation table.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      port            (IN) Generic port
 *      key_type        (IN) Key Type : bcmVlanTranslateKey*
 *      outer_vlan      (IN) Packet outer VLAN ID
 *      inner_vlan      (IN) Packet inner VLAN ID
 *      action          (IN) Action for outer and inner tag
 */
int
bcm_robo_vlan_translate_action_add(int unit,
                                  bcm_gport_t port,
                                  bcm_vlan_translate_key_t key_type,
                                  bcm_vlan_t outer_vlan,
                                  bcm_vlan_t inner_vlan,
                                  bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_UNAVAIL;
    
    CHECK_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action))  {
        _robo_vlan_translate_action_param_t vt_act_param;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = port;
        vt_act_param.key_type = key_type;
        vt_act_param.param1 = outer_vlan;
        vt_act_param.param2 = inner_vlan;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_ACTION;

        rv = _robo_vlan_translate_action_add(unit, &vt_act_param, action);
        return rv;
    }
#endif

    return rv;
}

/*
 * Function:
 *      bcm_vlan_translate_action_delete
 * Purpose:
 *      Delete a vlan translate lookup entry.
 * Parameters:
 *      unit            (IN) BCM unit number
 *      port            (IN) Generic port 
 *      key_type        (IN) Key Type : bcmVlanTranslateKey*
 *      outer_vlan      (IN) Packet outer VLAN ID
 *      inner_vlan      (IN) Packet inner VLAN ID
 */
int
bcm_robo_vlan_translate_action_delete(int unit,
                                     bcm_gport_t port,
                                     bcm_vlan_translate_key_t key_type,
                                     bcm_vlan_t outer_vlan,
                                     bcm_vlan_t inner_vlan)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action))  {
        _robo_vlan_translate_action_param_t vt_act_param;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = port;
        vt_act_param.key_type = key_type;
        vt_act_param.param1 = outer_vlan;
        vt_act_param.param2 = inner_vlan;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_ACTION;

        rv = _robo_vlan_translate_action_delete(unit, &vt_act_param);
        return rv;
    }
#endif

    return rv;
}

/*
 * Function :
 *      bcm_robo_vlan_translate_action_range_get
 * Description :
 *   Get an entry to the VLAN Translation table, which assigns
 *   VLAN actions for packets matching within the VLAN range(s).
 * Parameters :
 *      unit            (IN) BCM unit number
 *      port            (IN) Ingress gport (generic port)
 *      outer_vlan_low  (IN) Outer VLAN ID Low
 *      outer_vlan_high (IN) Outer VLAN ID High
 *      inner_vlan_low  (IN) Inner VLAN ID Low
 *      inner_vlan_high (IN) Inner VLAN ID High
 *      action          (OUT) Action for outer and inner tag
 *
 */
int 
bcm_robo_vlan_translate_action_range_get (int unit, bcm_port_t port,
                                         bcm_vlan_t outer_vlan_low,
                                         bcm_vlan_t outer_vlan_high,
                                         bcm_vlan_t inner_vlan_low,
                                         bcm_vlan_t inner_vlan_high,
                                         bcm_vlan_action_set_t *action)
{
    CHECK_INIT(unit);
    if (BCM_VLAN_INVALID != outer_vlan_low) {
        CHECK_VID(unit, outer_vlan_low);
    }
    if (BCM_VLAN_INVALID != outer_vlan_high) {
        CHECK_VID(unit, outer_vlan_high);
    }
    if (BCM_VLAN_INVALID != inner_vlan_low) {
        CHECK_VID(unit, inner_vlan_low);
    }
    if (BCM_VLAN_INVALID != inner_vlan_high) {
        CHECK_VID(unit, inner_vlan_high);
    }
    if (NULL == action) {
        return BCM_E_PARAM;
    }

#ifdef BCM_TB_SUPPORT
    /* 
      * BCM53280 only support translation of single inner-tagged packets with vid range. 
      * The outer_vlan_low/high should be BCM_VLAN_INVALID.
      */
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        /* Double-tagged and single outer-tagged are not supported for TB.*/
        if (outer_vlan_low != BCM_VLAN_INVALID || 
            outer_vlan_high != BCM_VLAN_INVALID) {
            return BCM_E_UNAVAIL;
        }

        if (inner_vlan_low == BCM_VLAN_INVALID || 
            inner_vlan_high == BCM_VLAN_INVALID) {
            return BCM_E_PARAM;
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = inner_vlan_low;
        vt_act_param.param2 = inner_vlan_high;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_ACTION_RANGE;
        return _robo_vlan_translate_action_get(unit, &vt_act_param, action);
    }
#endif

    return BCM_E_UNAVAIL;
}

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
/* bcm53115 spec. for the EgressVidRemark(EVR) table index:
 *  - table profile :
 *      1. 256 entries per port. (port0~port5 and IMP )
 *      2. table index keys are : egress_port_id + classification_id(from CFP)
 *  - Designed entry index format : 0x00000pdd.
 *      1. p : is egress port id. (0 ~ 6 is valid, 6 is IMP port)
 *      2. dd : is entry id (0 ~ 255 is valid)
 */
#define _EVR_ENTRYID_MAX           0x06FF
#define _EVR_ENTRYID_MASK           0x07FF
#define _EVR_ENTRYID_PORT_MASK  0x0700
#define _EVR_ENTRYID_PORT_SHIFT 8
#define _EVR_ENTRYID_FLOW_MASK  0x00FF
#define _EVR_ENTRYID_FLOW_SHIFT 0

#define _EVR_ENTRYID_IS_VALID(index)  \
            ((index) >=0 && (index) <= _EVR_ENTRYID_MAX)
#define EVR_ENTRYID_RESOLVE(index, port, flow_id)  \
            if (_EVR_ENTRYID_IS_VALID((index))){    \
                (port) = ((index) & _EVR_ENTRYID_PORT_MASK) \
                        >> _EVR_ENTRYID_PORT_SHIFT;     \
                (flow_id) = ((index) & _EVR_ENTRYID_FLOW_MASK) \
                        >> _EVR_ENTRYID_FLOW_SHIFT;     \
            } else {        \
                return BCM_E_BADID;     \
            }

/* bcm53115's EVR tag action */
#define EVR_ACTION_ASIS         0
#define EVR_ACTION_ASRX         1
#define EVR_ACTION_REMOVE       2
#define EVR_ACTION_MODIFY       3

#define EVR_SET_OP_SET          0
#define EVR_SET_OP_RESET        1

static int
_bcm_vulcan_evr_set(int unit, int op,
                int evr_index, bcm_vlan_action_set_t *action)
{
    uint32  fld_val32=0;
    egress_vid_remark_entry_t   evr_ent;
    
    /* Action process :
     *      - action.dt_outer and action.dt_inner are used for action.
     *      - action.new_inner_vlan action.new_outer_vlan are used for the 
     *          replace action.
     *
     *  the reason are :
     *  1. After the packet normalization the received packet which is 
     *      going to EVR in bcm53115 will be double tagged always.
     *  2. PCP or priority can't be changed in EVR table.
     */

    /* write to EVR table */
    sal_memset(&evr_ent, 0, sizeof (evr_ent));

    if (op != EVR_SET_OP_RESET){
        /* set outer VID */
        fld_val32 = action->new_outer_vlan;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID, 
                (uint32 *)&evr_ent, (uint32 *)&fld_val32));
        
        /* set outer action */
        fld_val32 = 
                (action->dt_outer == bcmVlanActionDelete) ? EVR_ACTION_REMOVE : 
                (action->dt_outer == bcmVlanActionReplace) ? EVR_ACTION_MODIFY : 
                (action->dt_outer == bcmVlanActionNone) ? EVR_ACTION_ASRX : 
                        EVR_ACTION_ASIS;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP, 
                (uint32 *)&evr_ent, (uint32 *)&fld_val32));
                            
        /* set inner VID */
        fld_val32 = action->new_inner_vlan;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID, 
                (uint32 *)&evr_ent, (uint32 *)&fld_val32));
    
        /* set inner action */
        fld_val32 = 
                (action->dt_inner== bcmVlanActionDelete) ? EVR_ACTION_REMOVE : 
                (action->dt_inner == bcmVlanActionReplace) ? EVR_ACTION_MODIFY : 
                (action->dt_inner == bcmVlanActionNone) ? EVR_ACTION_ASRX : 
                        EVR_ACTION_ASIS;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP, 
                (uint32 *)&evr_ent, (uint32 *)&fld_val32));
    }    

    BCM_IF_ERROR_RETURN(DRV_MEM_WRITE(
            unit, DRV_MEM_EGRVID_REMARK, evr_index, 1, 
            (uint32 *)&evr_ent));

    return BCM_E_NONE;
}

static int
_bcm_vulcan_evr_get(int unit, int entry_id,
                                         bcm_vlan_action_set_t *action)
{
    int     evr_id = entry_id; /* egress vt entry id */
    uint8   *list = NULL;
    uint32  fld_val32=0;
    egress_vid_remark_entry_t   evr_ent;
    

    /* Action process :
     *      - action.dt_outer and action.dt_inner are used for action.
     *      - action.new_inner_vlan action.new_outer_vlan are used for the 
     *          replace action.
     *
     *  the reason are :
     *  1. After the packet normalization the received packet which is 
     *      going to EVR in bcm53115 will be double tagged always.
     *  2. PCP or priority can't be changed in EVR table.
     */

    sal_memset(&evr_ent, 0, sizeof (evr_ent));
    sal_memset(action, 0, sizeof(bcm_vlan_action_set_t));

    list = evt_direct_set_info[unit].direct_set_list;
    if (!EVTLIST_ENTRY_CHK(list, evr_id)){
        return BCM_E_NOT_FOUND;
    }
    
    BCM_IF_ERROR_RETURN(DRV_MEM_READ(
                    unit, DRV_MEM_EGRVID_REMARK, evr_id, 1, 
                    (uint32 *)&evr_ent));

    /* set outer VID */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID, 
            (uint32 *)&evr_ent, (uint32 *)&fld_val32));
    action->new_outer_vlan = fld_val32;
    
    /* set outer action */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP, 
            (uint32 *)&evr_ent, (uint32 *)&fld_val32));
    action->dt_outer = 
        (fld_val32 == EVR_ACTION_REMOVE) ? bcmVlanActionDelete : 
        (fld_val32 == EVR_ACTION_MODIFY) ? bcmVlanActionReplace : 
        (fld_val32 == EVR_ACTION_ASRX) ? bcmVlanActionNone : 
                bcmVlanActionAdd;
                        
    /* set inner VID */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID, 
            (uint32 *)&evr_ent, (uint32 *)&fld_val32));
    action->new_inner_vlan = fld_val32;

    /* set inner action */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP, 
            (uint32 *)&evr_ent, (uint32 *)&fld_val32));
    action->dt_inner = 
        (fld_val32 == EVR_ACTION_REMOVE) ? bcmVlanActionDelete : 
        (fld_val32 == EVR_ACTION_MODIFY) ? bcmVlanActionReplace : 
        (fld_val32 == EVR_ACTION_ASRX) ? bcmVlanActionNone : 
                bcmVlanActionAdd;
    
    return BCM_E_NONE;
}

static int
_robo_vlan_translate_egress_action_traverse(int unit,  
            bcm_vlan_translate_egress_action_traverse_cb cb, 
            void *user_data)
{
    int     i, exc_count = 0, tbl_size = 0; 
    int     port_class = 0;
    uint8   *list;
    bcm_vlan_action_set_t action;
    
    tbl_size = SOC_MEM_SIZE(unit, INDEX(EGRESS_VID_REMARKm));
    exc_count = evt_direct_set_info[unit].direct_set_count;
    list = evt_direct_set_info[unit].direct_set_list;
    for (i = 0; i < tbl_size; i++){
        sal_memset(&action, 0, sizeof(bcm_vlan_action_set_t));

        if (exc_count == 0){
            break;
        }

        if (EVTLIST_ENTRY_CHK(list, i)){
    
            /* get action */
            BCM_IF_ERROR_RETURN(
                    _bcm_vulcan_evr_get(unit, i, &action));

            BCM_GPORT_SPECIAL_SET(port_class, i);
            
            /* Call application call-back */
            cb(unit, port_class, 0, 0, &action, user_data);

            exc_count--;
            
        }
    }    

    return BCM_E_NONE;
}


#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/

/*
 * Function   :
 *      bcm_vlan_translate_egress_action_add
 * Description   :
 *      Add an entry to egress VLAN translation table.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      port_class      (IN) Group ID of ingress port
 *      outer_vlan      (IN) Packet outer VLAN ID
 *      inner_vlan      (IN) Packet inner VLAN ID
 *      action          (IN) Action for outer and inner tag
 */
int
bcm_robo_vlan_translate_egress_action_add(int unit, int port_class,
                                         bcm_vlan_t outer_vlan,
                                         bcm_vlan_t inner_vlan,
                                         bcm_vlan_action_set_t *action)
{
    
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    /* the ingress packet outer/inner VID will be ignored */
    CHECK_INIT(unit);    
    if (action == NULL) {
        return BCM_E_PARAM;
    }

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int     evr_id = 0; /* egress vt entry id */
        uint32  int_port = 0, int_flowid = 0;
        uint8   *list = NULL;

        /* This API is used to direct control bcm53115's Egress VID Remark 
         *  (EVR)table only. This table is for egress VLAN tag(S-Tag and C-Tag)
         *  remarking on a specific port per CFP generated Flow-ID
         *  (aka. class-id)
         *  EVR table entry is indexed by per port 256 entries.
         *  Here we use BCM_GPORT_TYPE_SPECIAL to indicate the specific entry 
         *  index format.
         */
        
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "BCM API : %s, port_class=%d, outer_vid=%d, inner_vid=%d\n"), 
                  FUNCTION_NAME(), port_class, outer_vlan, inner_vlan));

        /* parse the requesting EVR table entry index */
        if (BCM_GPORT_IS_SET(port_class) && 
                BCM_GPORT_IS_SPECIAL(port_class)) {
            evr_id = BCM_GPORT_SPECIAL_GET(port_class);
            if (evr_id != BCM_GPORT_INVALID){
                EVR_ENTRYID_RESOLVE(evr_id, int_port, int_flowid);
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s, EVT entry at id=%d(port=%d, flow=%d)\n"), 
                          FUNCTION_NAME(), evr_id, int_port, int_flowid));
            } else {
                return BCM_E_PORT;
            }
            
            /* check the existence */
            list = evt_direct_set_info[unit].direct_set_list;
            if (EVTLIST_ENTRY_CHK(list, evr_id)){
                return BCM_E_EXISTS;
            }
            
            BCM_IF_ERROR_RETURN(_bcm_vulcan_evr_set(
                    unit, EVR_SET_OP_SET, evr_id, action));
            
            /* update sw database */
            EVTLIST_ENTRY_SET(list, evr_id);
            evt_direct_set_info[unit].direct_set_count ++;

        }else {
            /* if GPORT is not at SPECIAL type, reference to original VT  
             *  related APIs and the port parsing will be handled in the 
             *  called VT APIs.
             */ 
            bcm_port_t  in_port = 0;
            bcm_vlan_t  new_vid = 0;
            int         new_pri = 0;
            
            /* check action to know the request VT is for Transparent or 
             * Mapping mode.
             *  1. bcm_vlan_translate_egress_xxx() : for Mapping mode
             *      - inner_vlan will be ignored.
             *      - for the action to egress single tag only.
             *
             *  2. bcm_vlan_dtag_xxx() : for Transparent mode
             *      - inner_vlan will be ignored.
             *      - for the action to egress double tag only.
             *
             * Besides, the action for egress VLAN translation will reference 
             *  action on action->dt_xxx only due to the packet to egress 
             *  prodcess will be normalized as DoubleTagged packet already.
             * Thus all other actions will be ignored. 
             *  (a.k.a. action->ot_xxxx, action->it_xxxx and action->ut_xxxx)
             */
            in_port = port_class;
            new_vid = action->new_outer_vlan;
            new_pri = action->priority;

            if ((action->dt_outer == bcmVlanActionReplace) && 
                    (action->dt_inner == bcmVlanActionDelete)){
                /* Mapping mode VT */
                BCM_IF_ERROR_RETURN(bcm_robo_vlan_translate_egress_add(unit, 
                            in_port, outer_vlan, new_vid, new_pri));
            } else if ((action->dt_outer == bcmVlanActionReplace) && 
                    (action->dt_inner == bcmVlanActionNone)){
                /* Transparent mode VT */
                BCM_IF_ERROR_RETURN(bcm_robo_vlan_dtag_add(unit, 
                        in_port, outer_vlan, new_vid, new_pri));
            } else {
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s, For none-SPECIAL_GPORT_TYPE, the DoubleTag "
                                     "action allowed outer_action at 'Replace' and "
                                     "inner_action at 'None | Delete' only.\n"),
                          FUNCTION_NAME()));
                return BCM_E_UNAVAIL;
            }

        }
    } else {

        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
#else   /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s..unavailable\n"),FUNCTION_NAME()));
    return BCM_E_UNAVAIL;
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3 */
}

/*
 * Function:
 *      bcm_vlan_translate_egress_action_delete
 * Purpose:
 *      Delete an egress vlan translate lookup entry.
 * Parameters:
 *      unit            (IN) BCM unit number
 *      port_class      (IN) Group ID of ingress port
 *      outer_vlan      (IN) Packet outer VLAN ID
 *      inner_vlan      (IN) Packet inner VLAN ID
 */
int
bcm_robo_vlan_translate_egress_action_delete(int unit, int port_class,
                                            bcm_vlan_t outer_vlan,
                                            bcm_vlan_t inner_vlan)
{

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT) 
    /* the ingress packet outer/inner VID will be ignored */
    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s, port_class=%d, outer_vid=%d, inner_vid=%d\n"), 
              FUNCTION_NAME(), port_class, outer_vlan, inner_vlan));

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int     evr_id = 0; /* egress vt entry id */
        uint32  int_port = 0, int_flowid = 0;
        uint8   *list = NULL;
        bcm_vlan_action_set_t action;

        /* This API is used to direct control bcm53115's Egress VID Remark 
         *  (EVR)table only. This table is for egress VLAN tag(S-Tag and C-Tag)
         *  remarking on a specific port per CFP generated Flow-ID
         *  (aka. class-id)
         *  EVR table entry is indexed by per port 256 entries.
         *  Here we use BCM_GPORT_TYPE_SPECIAL to indicate the specific entry 
         *  index format.
         */

        /* parse the requesting EVR table entry index */
        if (BCM_GPORT_IS_SET(port_class) && 
                BCM_GPORT_IS_SPECIAL(port_class)) {
            evr_id = BCM_GPORT_SPECIAL_GET(port_class);
            if (evr_id != BCM_GPORT_INVALID){
                EVR_ENTRYID_RESOLVE(evr_id, int_port, int_flowid);
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s, EVT entry at id=%d(port=%d, flow=%d)\n"), 
                          FUNCTION_NAME(), evr_id, int_port, int_flowid));
            } else {
                return BCM_E_PORT;
            }
        
            /* check the existence */
            list = evt_direct_set_info[unit].direct_set_list;
            if (!EVTLIST_ENTRY_CHK(list, evr_id)){
                return BCM_E_NOT_FOUND;
            }

            sal_memset(&action, 0, sizeof(bcm_vlan_action_set_t));
            BCM_IF_ERROR_RETURN(_bcm_vulcan_evr_set(
                    unit, EVR_SET_OP_RESET, evr_id, &action));

            /* update SW database */
            EVTLIST_ENTRY_RESET(list, evr_id);
            evt_direct_set_info[unit].direct_set_count --;
        } else {
            /* if GPORT is not at SPECIAL type, reference to original VT  
             *  related APIs and the port parsing will be handled in the 
             *  called VT APIs.
             */ 
            bcm_port_t  in_port = 0;
            bcm_vlan_t  old_vid = 0;
            int         rv = BCM_E_NONE;

            in_port = port_class;
            old_vid = outer_vlan;

            rv = bcm_robo_vlan_translate_egress_delete(unit, 
                    in_port, old_vid);
            if (rv == BCM_E_NOT_FOUND){
                rv = bcm_robo_vlan_dtag_delete(unit, 
                    in_port, old_vid);
            }

            return rv;
        }
    } else {
        return BCM_E_UNAVAIL;
    }
    
    return BCM_E_NONE;
#else   /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s..unavailable\n"),FUNCTION_NAME()));
    return BCM_E_UNAVAIL;
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
}

/*
 * Function:
 *      bcm_vlan_dtag_add
 * Purpose:
 *      Set VLAN translation by trasparent mode
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 *      port - port id (zero based)
 *      inner_vid - Old VID
 *      outer_vid - New VID
 *      prio - priority
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1. prot = -1 means this is system basis setting. 
 *          This is differnet with ESW.
 *      2. Per different VLAN translation designing mechanism with ESW chips, 
 *          please check the responsible routines for each chip real behavior.
 */

int
bcm_robo_vlan_dtag_add(int unit,int port, bcm_vlan_t inner_vid,
        bcm_vlan_t outer_vid, int prio)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    
    CHECK_INIT(unit);
    CHECK_VID(unit, inner_vid);
    CHECK_VID(unit, outer_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s(unit=%d,port=%d,vid1=%d,vid2=%d,prio=%d)\n"),
              FUNCTION_NAME(), unit, port, inner_vid, outer_vid, prio));
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_vlan_action_set_t action;
        bcm_gport_t gport;
        int rv = BCM_E_NONE;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        bcm_vlan_action_set_t_init(&action);
        action.new_outer_vlan = outer_vid;
        action.priority = prio;
        /* For inner tagged packets, set the outer tag action to ADD.
         * For all other packet types, the action is initialized to NONE.
         */
        action.it_outer = bcmVlanActionAdd;
        action.outer_pcp = bcmVlanPcpActionIngressInnerPcp;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = inner_vid;
        vt_act_param.vt_api_type = VT_API_VLAN_DTAG;

        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_add(unit, &vt_act_param, &action));


        /* Save information for traverse. */
        rv = _robo_vlan_dtag_traverse_info_add(unit, gport, inner_vid, 
                  outer_vid, prio);
        if (rv < 0) {
            /* 
              * Traverse information sw copy create failed. 
              */
            _robo_vlan_translate_action_delete(unit, &vt_act_param);
        }

        return rv;
}
#endif 

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    /* check valid priority only but doing nothing for ROBO */
    if ((prio != -1) && ((prio & ~BCM_PRIO_MASK) != 0)) {
        return BCM_E_PARAM;
    }

    /* In robo chips so far, there are only bcm53242/bcm53262/bcm53115 
     *  support VLAN translation. 
     * ----------
     * 1. bcm53242/bcm53262 : support ingress basis vlan_translation only.
     * 2. bcm53115 : support egress basis vlan_translation only.
     *          - actually, bcm53115 is working by ingress VID filtering and 
     *              egress VID translate.
     */ 
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int idt_mode, rebuild_request = FALSE;
        
        /* re-init VT_CFP and EVR table, add this seciton is for the 
         *  regression test will not call bcm_vlan_control_set() to enable 
         *  VLAN translation feature.
         */
        if ((!IS_VT_CFP_INIT)){
            
            /* make sure the requirred CFP was init already */
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_init(unit));
    
            /* clear all EVR table entry on each port. and the related 
             *      port basis database.
             */
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));

            rebuild_request = TRUE;
        }

        /* enable iDT_Mode, add this seciton is for the regression test 
         *  will not call bcm_vlan_control_set() to enable VLAN translation 
         *  feature.
         */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                         (uint32 *) &idt_mode));
        if (!idt_mode){
            idt_mode = TRUE;
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, idt_mode));
        }
        
        /* Vulcan VLAN translation can support global VT only(no port basis)
         * so the port parameter will be ignore here.
         */
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_create(
                    unit, inner_vid, outer_vid, 
                    VLAN_VTMODE_TRANSPATENT));

        /* GNATS 41020 :
         *  - Packet to ISP port won't be outer tagged once the port been set 
         *      to ISP before any VT entry created.
         */
        if (rebuild_request){
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vt_all_isp_update(unit));
        }
    } else {
        BCM_IF_ERROR_RETURN(DRV_VLAN_VT_ADD
                (unit, DRV_VLAN_XLAT_INGRESS, port, inner_vid, outer_vid, 
                 prio, VLAN_VTMODE_TRANSPATENT));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_vlan_dtag_get
 * Purpose:
 *      Get vlan translation for double tagging
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      port - port numebr (ignored in ROBO53242/53262/53115)
 *      old_vid - Old VLAN ID to has translation for
 *      new_vid - New VLAN ID that packet will get
 *      prio    - Priority (ignored in ROBO53242/53262/53115)
 * Returns:
 *      BCM_E_NONE - Translation found, new_vid and prio will have the values.
 *      BCM_E_NOT_FOUND - Translation does not exist
 *      BCM_E_XXX  - Other error
 * Notes:
 *  1.
 *  
 */
int 
bcm_robo_vlan_dtag_get (int unit, bcm_port_t port,
                       bcm_vlan_t old_vid,
                       bcm_vlan_t *new_vid,
                       int *prio)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    uint32 vt_mode = 0, vt_new_vid = 0;
    
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s(unit=%d,port=%d,vid=%d)\n"),
              FUNCTION_NAME(), unit, port, old_vid));
            
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        bcm_vlan_action_set_t action;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }
        bcm_vlan_action_set_t_init(&action);
        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = old_vid;
        vt_act_param.vt_api_type = VT_API_VLAN_DTAG;
        BCM_IF_ERROR_RETURN(
             _robo_vlan_translate_action_get(unit, &vt_act_param, &action));
        if (BCM_VLAN_INVALID != action.new_outer_vlan) {
            *new_vid = action.new_outer_vlan;
            *prio = action.priority;
            return BCM_E_NONE;
    }

        return BCM_E_NOT_FOUND;
    }
#endif /* BCM_TB_SUPPORT */
            
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }
    
    /* The VLAN translation in bcm53243/bcm53262/bcm53115 support VID key 
     * only. The priority key is ignored in ROBO vlan translation.
     *  - bcm53115 is egress basis design.
     *  - bcm53242 & bcm53262 is ingress basis design.
     */
    if(SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
            (unit, DRV_VLAN_PROP_EGR_VT_SPVID, old_vid, port, &vt_new_vid));

    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
            (unit, DRV_VLAN_PROP_ING_VT_SPVID, old_vid, port, &vt_new_vid));
    } else {
        return BCM_E_UNAVAIL;
    }
    *new_vid = vt_new_vid;

    /* check the target VID's VT_Mode(transparant/mapping)
     *  - dtag is VLAN_XLATE transparant mode.
     *  - if the target VID is assinged as mapping mode, the return value 
     *      will still be "BCM_E_NOTFOUND".
     */    
    BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
            (unit, DRV_VLAN_PROP_VT_MODE, old_vid, port, &vt_mode));
    if (vt_mode != VLAN_VTMODE_TRANSPATENT){
        
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "%s,unit=%d,port=%d,vid=%d is not at transparent mode!\n"),
                  FUNCTION_NAME(), unit, port, old_vid));
        return BCM_E_NOT_FOUND;
    }
    
    
    return BCM_E_NONE;
}

int
bcm_robo_vlan_dtag_delete(int unit, int port, bcm_vlan_t inner_vid)
{
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    int rv = BCM_E_NONE;
   
    CHECK_INIT(unit);
    CHECK_VID(unit, inner_vid);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s(unit=%d,port=%d,vid=%d)\n"),
              FUNCTION_NAME(), unit, port, inner_vid));
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = inner_vid;
        vt_act_param.vt_api_type = VT_API_VLAN_DTAG;
        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_delete(unit, &vt_act_param));
        
        /* Remove information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_dtag_traverse_info_remove(unit, 
                                                  gport, inner_vid));
        return BCM_E_NONE;
    }
#endif

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    /* In robo chips so far, there are only bcm53242/bcm53262/bcm53115 
     *  support VLAN translation. 
     * ----------
     * 1. bcm53242/bcm53262 : support ingress basis vlan_translation only.
     * 2. bcm53115 : support egress basis vlan_translation only.
     *          - actually, bcm53115 is working by ingress VID filtering and 
     *              egress VID translate.
     */ 
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int idt_mode;
        
        if (!IS_VT_CFP_INIT) {
            
            /* make sure the requirred CFP was init already */
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_init(unit));
    
            /* clear all EVR table entry on each port. and the related 
             *      port basis database.
             */
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));
        }

        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                         (uint32 *) &idt_mode));
        if (!idt_mode) {
            idt_mode = TRUE;
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, idt_mode));
        }

        /* Vulcan VLAN translation can support global VT only(no port basis)
         * so the port parameter will be ignore here.
         */
        rv = _bcm_robo_vlan_vtcfp_delete(
                        unit, inner_vid, VLAN_VTMODE_TRANSPATENT);

    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
        rv = DRV_VLAN_VT_DELETE
                    (unit, DRV_VLAN_XLAT_INGRESS_TRAN, port, inner_vid);
    } else {
        return BCM_E_UNAVAIL;
    }
    return rv;
}

int
bcm_robo_vlan_dtag_delete_all(int unit)
{
    CHECK_INIT(unit);

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : %s\n"), FUNCTION_NAME()));
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_DTAG, 0));

        /* Remove all information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_dtag_traverse_info_remove_all(unit));
        return BCM_E_NONE;
    }
#endif

    /* In robo chips so far, there are only bcm53242/bcm53262/bcm53115 
     *  support VLAN translation. 
     * ----------
     * 1. bcm53242/bcm53262 : support ingress basis vlan_translation only.
     * 2. bcm53115 : support egress basis vlan_translation only.
     *          - actually, bcm53115 is working by ingress VID filtering and 
     *              egress VID translate.
     */ 
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
        int idt_mode;

        if (!IS_VT_CFP_INIT) {
            
            /* make sure the requirred CFP was init already */
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_init(unit));
    
            /* clear all EVR table entry on each port. and the related 
             *      port basis database.
             */
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));
        }

        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                        (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,
                         (uint32 *) &idt_mode));
        if (!idt_mode) {
            idt_mode = TRUE;
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, idt_mode));
        }
        
        /* Vulcan VLAN translation can support global VT only(no port basis)
         * so the port parameter will be ignore here.
         */
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_delete_all(
                        unit, VLAN_VTMODE_TRANSPATENT));
        
    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){

        BCM_IF_ERROR_RETURN(DRV_VLAN_VT_DELETE_ALL
                    (unit, DRV_VLAN_XLAT_INGRESS_TRAN));

    } else {
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

int 
bcm_robo_vlan_dtag_range_add(int unit, int port,
                            bcm_vlan_t old_vid_low, 
                            bcm_vlan_t old_vid_high,
                            bcm_vlan_t new_vid, int int_prio)
{
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid_low);
    CHECK_VID(unit, old_vid_high);
    CHECK_VID(unit, new_vid);
    if (old_vid_high < old_vid_low) {
        return BCM_E_PARAM;
    }

    if (!_BCM_COSQ_PRIO_VALID(unit, int_prio)) {
        return BCM_E_PARAM;
    }

    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_vlan_action_set_t action;
        bcm_gport_t gport;
        int rv = BCM_E_NONE;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        /* add an entry for inner-tagged packets */
        bcm_vlan_action_set_t_init(&action);
        action.new_outer_vlan = new_vid;
        action.priority = int_prio;
        /* For inner tagged packets, set the outer tag action to ADD.
         * For all other packet types, the action is initialized to NONE.
         */
        action.it_outer = bcmVlanActionAdd;
        action.outer_pcp = bcmVlanPcpActionIngressInnerPcp;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = old_vid_low;
        vt_act_param.param2 = old_vid_high;
        vt_act_param.vt_api_type = VT_API_VLAN_DTAG_RANGE;

        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_add(unit, &vt_act_param, &action));

        /* Save information for traverse. */
        rv = _robo_vlan_dtag_range_traverse_info_add(unit, gport, 
                old_vid_low, old_vid_high, new_vid, int_prio);
        if (rv < 0) {
            /* 
              * Traverse information sw copy create failed. 
              */
            _robo_vlan_translate_action_delete(unit, &vt_act_param);
        }
        return rv;
    }
#endif 
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_dtag_range_get (int unit, bcm_port_t port,
                             bcm_vlan_t old_vid_low,
                             bcm_vlan_t old_vid_high,
                             bcm_vlan_t *new_vid,
                             int *prio)
{
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid_low);
    CHECK_VID(unit, old_vid_high);

    if ((NULL == new_vid) || NULL == prio ){
        return BCM_E_PARAM;
    }
    if (old_vid_high < old_vid_low) {
        return BCM_E_PARAM;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        bcm_vlan_action_set_t action;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = old_vid_low;
        vt_act_param.param2 = old_vid_high;
        vt_act_param.vt_api_type = VT_API_VLAN_DTAG_RANGE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_get(unit, &vt_act_param, &action));
        if (BCM_VLAN_INVALID != action.new_outer_vlan) {
            *new_vid = action.new_outer_vlan;
            *prio = action.priority;
            return BCM_E_NONE;
        }

        return BCM_E_NOT_FOUND;
    }
#endif
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_dtag_range_delete(int unit, int port, 
                               bcm_vlan_t old_vid_low,
                               bcm_vlan_t old_vid_high)
{
    CHECK_INIT(unit);
    CHECK_VID(unit, old_vid_low);
    CHECK_VID(unit, old_vid_high);
    if (old_vid_high < old_vid_low) {
        return BCM_E_PARAM;
    }

    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = old_vid_low;
        vt_act_param.param2 = old_vid_high;
        vt_act_param.vt_api_type = VT_API_VLAN_DTAG_RANGE;
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete(unit, &vt_act_param));

        /* Remove information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_dtag_range_traverse_info_remove(unit, 
                                                  gport, old_vid_low, old_vid_high));
        return BCM_E_NONE;
    }
#endif 

    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_dtag_range_delete_all(int unit)
{
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete_all(unit, \
                VT_API_VLAN_DTAG_RANGE, 0));

        /* Remove all information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_dtag_range_traverse_info_remove_all(unit));

        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}


/* IP4 subnet based vlan selection */
int 
bcm_robo_vlan_ip4_add(int unit, bcm_ip_t ipaddr, bcm_ip_t netmask,
                bcm_vlan_t vid, int prio)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_ip4_add()..unavailable\n")));
    return BCM_E_UNAVAIL;
}               
                
int 
bcm_robo_vlan_ip4_delete(int unit, bcm_ip_t ipaddr, bcm_ip_t netmask)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_ip4_delete()..unavailable\n")));
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_ip4_delete_all(int unit)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_ip4_delete_all()..unavailable\n")));
    return BCM_E_UNAVAIL;
}               

int 
bcm_robo_vlan_ip_add(int unit, bcm_vlan_ip_t * vlan_ip)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_ip_add()..unavailable\n")));
    return BCM_E_UNAVAIL;
}               
                
int 
bcm_robo_vlan_ip_delete(int unit, bcm_vlan_ip_t * vlan_ip)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_ip_delete()..unavailable\n")));
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_ip_delete_all(int unit)
{
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_ip_delete_all()..unavailable\n")));
    return BCM_E_UNAVAIL;
}               

/*
 * Function:
 *     bcm_robo_vlan_control_set
 *
 * Purpose:
 *     Set miscellaneous VLAN-specific chip options
 *
 * Parameters:
 *     unit - RoboSwitch device unit number (driver internal).
 *     type - A value from bcm_vlan_control_t enumeration list
 *     arg  - state whose meaning is dependent on 'type'
 *
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_INTERNAL - Chip access failure.
 *     BCM_E_UNAVAIL  - type not supported on unit
 *
 * Note :
 *  In bcm53242, we implement the VLAN prefer MAC2VLAN
 *      (i.e. type=bcmVlanPreferMAC) by disable the port TRUST_VLAN.
 *  This is not quite the same with enterprise switch. To disable port
 *      TRUST_VLAN can perform the prefer MAC2VLAN feature only when the
 *      incoming frame is untagged/1P tagged. And such feature can't work
 *      correctly when the incoming frame is tagged and DA is mist in
 *      MAC2VLAN table.
 *
 */
int
bcm_robo_vlan_control_set(int unit, bcm_vlan_control_t type, int arg)
{
    bcm_pbmp_t  set_bmp;
    int port = 0;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_control_set()..\n")));
    switch (type) {
      case bcmVlanDropUnknown:
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                (unit, DRV_VLAN_PROP_VTABLE_MISS_DROP,
                 (arg ? TRUE :FALSE)));
        break;
      case bcmVlanTranslate:

        if (soc_feature(unit, soc_feature_vlan_translation)) {

            int current_en = 0;
            
            arg = (arg) ? TRUE : FALSE;
            BCM_IF_ERROR_RETURN(bcm_vlan_control_get(unit, 
                   bcmVlanTranslate , &current_en));
            if (current_en == arg) {   /* config no change */
                return BCM_E_NONE;
            }
            
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                if (arg){
                    /* make sure the requirred CFP was init already */
                    BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vtcfp_init(unit));
    
                    /* clear all EVR table entry on each port. and the related 
                     *      port basis database.
                     */
                    BCM_IF_ERROR_RETURN(
                            DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_EVR_INIT, TRUE));
                }
                BCM_IF_ERROR_RETURN(
                        DRV_VLAN_PROP_SET
                        (unit, DRV_VLAN_PROP_V2V,(arg ? TRUE :FALSE)));
            } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
                BCM_IF_ERROR_RETURN(
                            DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_V2V,
                                (arg ? TRUE :FALSE)));
            } else if (SOC_IS_TBX(unit)){
#ifdef BCM_TB_SUPPORT
                if (!arg) {
                    /* Disable vlan translation configuration */
                    _robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_XLATE_ACTION, 0);
                    _robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_XLATE, 0);
                    _robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_DTAG, 0);
                    _robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_XLATE_ACTION_RANGE, 0);
                    _robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_XLATE_RANGE, 0);
                    _robo_vlan_translate_action_delete_all(unit, VT_API_VLAN_DTAG_RANGE, 0);

                    _robo_vlan_translate_traverse_info_remove_all(unit);
                    _robo_vlan_dtag_traverse_info_remove_all(unit);
                    _robo_vlan_translate_action_range_traverse_info_remove_all(unit);
                    _robo_vlan_translate_range_traverse_info_remove_all(unit);
                    _robo_vlan_dtag_range_traverse_info_remove_all(unit);
                }
                BCM_IF_ERROR_RETURN(
                            DRV_VLAN_PROP_SET
                                (unit, DRV_VLAN_PROP_V2V,
                                (arg ? TRUE :FALSE)));
#endif
            } else {
                return BCM_E_UNAVAIL;
            }
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
      case bcmVlanPerPortTranslate:
        if (soc_feature(unit, soc_feature_vlan_translation)) {
            BCM_IF_ERROR_RETURN(
                        DRV_VLAN_PROP_SET
                            (unit, DRV_VLAN_PROP_PER_PORT_TRANSLATION, 
                             (arg ? TRUE :FALSE)));
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
      case bcmVlanIgnorePktTag:
        BCM_PBMP_CLEAR(set_bmp);
        BCM_PBMP_ASSIGN(set_bmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(set_bmp, PBMP_CMIC(unit));

        BCM_IF_ERROR_RETURN(
                    DRV_VLAN_PROP_PORT_ENABLE_SET
                        (unit, DRV_VLAN_PROP_TRUST_VLAN_PORT, set_bmp,
                         (arg ? FALSE :TRUE)));
        break;
      case bcmVlanShared:
        /* user change VLAN mode between SVL and IVL on the fly is allowed but 
         * user must take the action to clear all existed L2 entries to 
         * prevent the unexpect forwarding behavior.
         */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                (unit, DRV_VLAN_PROP_VLAN_LEARNING_MODE,
                 (arg ? TRUE :FALSE)));
        break;
      case bcmVlanPreferMAC:
        BCM_PBMP_CLEAR(set_bmp);
        BCM_PBMP_ASSIGN(set_bmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(set_bmp, PBMP_CMIC(unit));
        PBMP_ITER(set_bmp, port) {
            BCM_IF_ERROR_RETURN(
                bcm_vlan_control_port_set(unit, port, 
                bcmVlanPortPreferMAC, (arg ? TRUE:FALSE)));
        }
        break;
      case bcmVlanIntelligentDT:    /* specific type for bcm53115 only */
        /* set bcm53115 into iDT_mode :
         *  
         * Note : 
         *  1. Such setting force bcm53115 at iDT mode, customer need to 
         *      handle some other related configuration to ensure the packet 
         *      learning and forwarding properly. 
         *    - Check processing flow in bcm_robo_port_dtag_mode_set().
         */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE,(arg ? TRUE :FALSE)));
        break;
        case bcmVlanTranslateMode:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                    (unit, DRV_VLAN_PROP_TRANSLATE_MODE, arg));
#ifdef BCM_TB_SUPPORT
            if (SOC_IS_TBX(unit)) {
                /* Update port cfg about the dtag mode */
                BCM_PBMP_ASSIGN(set_bmp, PBMP_ALL(unit));
                PBMP_ITER(set_bmp, port) {
                    _bcm_robo_dtag_mode_sw_update(unit, port, arg);
                }
            }
#endif
            break;
        case bcmVlanBypassIgmpMld:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                    (unit, DRV_VLAN_PROP_BYPASS_IGMP_MLD, arg));
            break;
        case bcmVlanBypassArpDhcp:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                    (unit, DRV_VLAN_PROP_BYPASS_ARP_DHCP, arg));
            break;
        case bcmVlanBypassMiim:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                    (unit, DRV_VLAN_PROP_BYPASS_MIIM, arg));
            break;
        case bcmVlanBypassMcast:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                    (unit, DRV_VLAN_PROP_BYPASS_MCAST, arg));
            break;
        case bcmVlanBypassRsvdMcast:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                    (unit, DRV_VLAN_PROP_BYPASS_RSV_MCAST, arg));
            break;
        case bcmVlanBypassL2UserAddr:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                    (unit, DRV_VLAN_PROP_BYPASS_L2_USER_ADDR, arg));
            break;
        case bcmVlanUnknownLearn:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET(unit, 
                 DRV_VLAN_PROP_VTABLE_MISS_LEARN, arg));
            break;
        case bcmVlanUnknownToCpu:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET(unit, 
                 DRV_VLAN_PROP_VTABLE_MISS_TO_CPU, arg));
            break;
        case bcmVlanMemberMismatchLearn:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET(unit, 
                 DRV_VLAN_PROP_MEMBER_MISS_LEARN, arg));
            break;
        case bcmVlanMemberMismatchToCpu:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_SET(unit, 
                 DRV_VLAN_PROP_MEMBER_MISS_TO_CPU, arg));
            break;
      case bcmVlanPreferEgressTranslate:
        /* The definition in header file on this control type is :
         * >> Do egress translation even if ingress FP changes the outer/inner 
         *      VLAN tag(s). 
         * 
         * In current ESW devices, only TRX device can support this specific 
         *  feature.
         *
         * In current robo devices, only BCM53115 can support egress basis 
         *  VT and only egress basis VT supported on bcm53115. There is no 
         *  such preference can be turn on or turn off on bcm53115.
         */
        return BCM_E_UNAVAIL;
      case bcmVlanPreferIP4:
      case bcmVlanSharedID:
      default:
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
    
}

/*
 * Function:
 *     bcm_robo_vlan_control_get
 *
 * Purpose:
 *     Get miscellaneous VLAN-specific chip options
 *
 * Parameters:
 *     unit - RoboSwitch device unit number (driver internal).
 *     type - A value from bcm_vlan_control_t enumeration list
 *     arg  - (OUT) state whose meaning is dependent on 'type'
 *
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_PARAM    - arg points to NULL
 *     BCM_E_INTERNAL - Chip access failure.
 *     BCM_E_UNAVAIL  - type not supported on unit
 */
int 
bcm_robo_vlan_control_get(int unit, bcm_vlan_control_t type, int *arg)
{
    uint32  field_value;
    uint32  port = 0;
    pbmp_t      pbmp;
    uint32  temp;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_control_get()..\n")));
    switch (type) {
      case bcmVlanDropUnknown:
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                (unit, DRV_VLAN_PROP_VTABLE_MISS_DROP, 
                  &field_value));
        *arg = (field_value) ? TRUE : FALSE;
        break;
      case bcmVlanTranslate:

        if (soc_feature(unit, soc_feature_vlan_translation)) {

            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                BCM_IF_ERROR_RETURN(
                        DRV_VLAN_PROP_GET
                            (unit, DRV_VLAN_PROP_V2V, &field_value));
                *arg = (field_value) ? TRUE : FALSE;
            } else {
                /* special port id to get the device basis value. */
                port = ~port;
                BCM_IF_ERROR_RETURN(
                        DRV_VLAN_PROP_PORT_ENABLE_GET
                            (unit, DRV_VLAN_PROP_V2V_PORT, port,  
                            (uint32 *)&pbmp));
                *arg = (BCM_PBMP_IS_NULL(pbmp)) ? FALSE : TRUE;
            }
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
      case bcmVlanPerPortTranslate:
        if (soc_feature(unit, soc_feature_vlan_translation)) {
            BCM_IF_ERROR_RETURN(
                        DRV_VLAN_PROP_GET
                            (unit, DRV_VLAN_PROP_PER_PORT_TRANSLATION, 
                             &temp));
            *arg = temp ? TRUE : FALSE;
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
      case bcmVlanIgnorePktTag:


        /* special assignment on port id to get the device basis value. */
        port = ~port;
        BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET
                    (unit, DRV_VLAN_PROP_TRUST_VLAN_PORT,
                    port,  (uint32 *)&pbmp));
        BCM_PBMP_REMOVE(pbmp, PBMP_CMIC(unit));
        BCM_PBMP_AND(pbmp, PBMP_ALL(unit));
        *arg = (BCM_PBMP_IS_NULL(pbmp)) ? TRUE : FALSE;
        break;
        
      case bcmVlanShared:
   
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                (unit, DRV_VLAN_PROP_VLAN_LEARNING_MODE, 
                  &field_value));
        *arg = (field_value) ? FALSE : TRUE;
        break;
      case bcmVlanPreferMAC:
        
        /* robo chips currently have no device based preferMAC setting */
        /* special assignment on port id to get the device basis value. */
        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(pbmp, PBMP_CMIC(unit));
        *arg = FALSE;
        PBMP_ITER(pbmp, port) {
            BCM_IF_ERROR_RETURN(
                bcm_vlan_control_port_get(unit, port, 
                bcmVlanPortPreferMAC, (int *) &temp));
            if (temp) {
                *arg = TRUE;
                break;
            }
        }
        break;
      case bcmVlanIntelligentDT:    /* specific type for bcm53115 only */
        /* set bcm53115 into iDT_mode :
         *  
         * Note : 
         *  1. Such setting force bcm53115 at iDT mode, customer need to 
         *      handle some other related configuration to ensure the packet 
         *      learning and forwarding properly. 
         *    - Check processing flow in bcm_robo_port_dtag_mode_set().
         */
        BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                (unit, DRV_VLAN_PROP_IDT_MODE_ENABLE, &temp));
        *arg = temp ? TRUE : FALSE;
        break;
        case bcmVlanTranslateMode:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_TRANSLATE_MODE, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanBypassIgmpMld:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_BYPASS_IGMP_MLD, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanBypassArpDhcp:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_BYPASS_ARP_DHCP, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanBypassMiim:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_BYPASS_MIIM, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanBypassMcast:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_BYPASS_MCAST, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanBypassRsvdMcast:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_BYPASS_RSV_MCAST, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanBypassL2UserAddr:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                    (unit, DRV_VLAN_PROP_BYPASS_L2_USER_ADDR, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanUnknownLearn:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET(unit, 
                 DRV_VLAN_PROP_VTABLE_MISS_LEARN, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanUnknownToCpu:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET(unit, 
                 DRV_VLAN_PROP_VTABLE_MISS_TO_CPU, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanMemberMismatchLearn:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET(unit, 
                 DRV_VLAN_PROP_MEMBER_MISS_LEARN, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
        case bcmVlanMemberMismatchToCpu:
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_GET(unit, 
                 DRV_VLAN_PROP_MEMBER_MISS_TO_CPU, &temp));
            *arg = temp ? TRUE : FALSE;
            break;
      case bcmVlanPreferEgressTranslate:
        /* The definition in header file on this control type is :
         * >> Do egress translation even if ingress FP changes the outer/inner 
         *      VLAN tag(s). 
         * 
         * In current ESW devices, only TRX device can support this specific 
         *  feature.
         *
         * In current robo devices, only BCM53115 can support egress basis 
         *  VT and only egress basis VT supported on bcm53115. There is no 
         *  such preference can be turn on or turn off on bcm53115.
         */
        return BCM_E_UNAVAIL;
      case bcmVlanPreferIP4:
      case bcmVlanSharedID:
      default:
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
} 
  
int
bcm_robo_vlan_control_port_set(int unit, bcm_port_t port,
              bcm_vlan_control_port_t type, int arg)
{
    int rv = BCM_E_NONE;
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    bcm_pbmp_t  bmp;
    bcm_port_t  loc_port;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_control_set...()")));

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) && 
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(_bcm_robo_gport_resolve
                (unit, port, &modid, &loc_port, &tgid, &id));
            if ((-1 != tgid) || (-1 != id)) {
                return BCM_E_PORT;
            }
        }
    } else {
        loc_port = port;
    }

    if ((loc_port != -1) && !SOC_PORT_VALID(unit, loc_port)) { 
        return BCM_E_PORT; 
    }

    BCM_PBMP_CLEAR(bmp);
    if (loc_port == -1) {
        BCM_PBMP_ASSIGN(bmp, PBMP_E_ALL(unit));
    } else {
        BCM_PBMP_PORT_ADD(bmp, loc_port);
    }

    switch (type) {
        case bcmVlanPortPreferMAC:
            BCM_IF_ERROR_RETURN(
                DRV_PORT_SET
                (unit, bmp, DRV_PORT_PROP_MAC_BASE_VLAN, 
                (arg ? TRUE:FALSE)));
            break;
        case bcmVlanTranslateIngressEnable:
            BCM_IF_ERROR_RETURN(
                        DRV_VLAN_PROP_PORT_ENABLE_SET
                            (unit, DRV_VLAN_PROP_V2V_PORT, bmp,
                            (arg ? TRUE :FALSE)));
            break;
        case bcmVlanPortIgnorePktTag:
            BCM_IF_ERROR_RETURN(
                        DRV_VLAN_PROP_PORT_ENABLE_SET
                            (unit, DRV_VLAN_PROP_TRUST_VLAN_PORT, bmp,
                             (arg ? FALSE :TRUE)));
            break;
        case bcmVlanPortUntaggedDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                    DRV_VLAN_PROP_UNTAGGED_DROP, bmp, (arg ? TRUE :FALSE)));
            break;
        case bcmVlanPortPriTaggedDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                    DRV_VLAN_PROP_PRI_TAGGED_DROP, bmp, (arg ? TRUE :FALSE)));
            break;
        case bcmVlanTranslateIngressHitDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                    DRV_VLAN_PROP_INGRESS_VT_HIT_DROP, bmp, (arg ? TRUE :FALSE)));
            break;
        case bcmVlanTranslateIngressMissDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                    DRV_VLAN_PROP_INGRESS_VT_MISS_DROP, bmp, (arg ? TRUE :FALSE)));
            break;
        case bcmVlanPortUseInnerPri:
            PBMP_ITER(bmp, loc_port) {
                BCM_IF_ERROR_RETURN(_bcm_robo_port_config_set
                    (unit, loc_port, _bcmPortUseInnerPri, (arg ? TRUE :FALSE)));
            }
            break;
        case bcmVlanPortUseOuterPri:
            PBMP_ITER(bmp, loc_port) {
                BCM_IF_ERROR_RETURN(_bcm_robo_port_config_set
                    (unit, loc_port, _bcmPortUseOuterPri, (arg ? TRUE :FALSE)));
            }
            break;
        case bcmVlanTranslateEgressMissUntag:
            /*
              * For BCM53280, the configuration only takes effect when 
              * VT_ENABLE=0 and V_DOMAIN=0.
              * Setting a port will also setting other ports that belong to
              * the same port profile.
              */
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                    DRV_VLAN_PROP_EGRESS_VT_MISS_UNTAG, bmp, (arg ? TRUE :FALSE)));
            break;
        case bcmVlanPortJoinAllVlan:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                    DRV_VLAN_PROP_JOIN_ALL_VLAN, bmp, (arg ? TRUE :FALSE)));
            break;
        default:
            rv = BCM_E_UNAVAIL;
            break;
    }

    return rv;
}               

int 
bcm_robo_vlan_control_port_get(int unit, bcm_port_t port,
              bcm_vlan_control_port_t type, int * arg)
{
    int rv = BCM_E_NONE;
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    uint32  temp = 0;

    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_control_port_get()\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) { 
            return BCM_E_PORT; 
        }
    }

    switch (type) {
        case bcmVlanPortPreferMAC:
            BCM_IF_ERROR_RETURN(
            DRV_PORT_GET
                (unit, port, DRV_PORT_PROP_MAC_BASE_VLAN, &temp));
            *arg = (temp) ? TRUE:FALSE;
            break;
        case bcmVlanTranslateIngressEnable:
            BCM_IF_ERROR_RETURN(
                    DRV_VLAN_PROP_PORT_ENABLE_GET
                        (unit, DRV_VLAN_PROP_V2V_PORT, port,  &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanPortIgnorePktTag:
            /* special assignment on port id to get the device basis value. */
            BCM_IF_ERROR_RETURN(
                    DRV_VLAN_PROP_PORT_ENABLE_GET
                        (unit, DRV_VLAN_PROP_TRUST_VLAN_PORT,
                        port, &temp));
            /* temp==TRUE means VLAN-Tag is trust and won't ignore */
            *arg = (temp) ? FALSE : TRUE;
            break;
        case bcmVlanPortUntaggedDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET(unit, 
                    DRV_VLAN_PROP_UNTAGGED_DROP, port, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanPortPriTaggedDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET(unit, 
                    DRV_VLAN_PROP_PRI_TAGGED_DROP, port, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanTranslateIngressHitDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET(unit, 
                    DRV_VLAN_PROP_INGRESS_VT_HIT_DROP, port, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanTranslateIngressMissDrop:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET(unit, 
                    DRV_VLAN_PROP_INGRESS_VT_MISS_DROP, port, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanPortUseInnerPri:
            BCM_IF_ERROR_RETURN(
                        _bcm_robo_port_config_get(unit, port, 
                             _bcmPortUseInnerPri, (int *)&temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanPortUseOuterPri:
            BCM_IF_ERROR_RETURN(
                        _bcm_robo_port_config_get(unit, port, 
                             _bcmPortUseOuterPri, (int *)&temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanTranslateEgressMissUntag:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET(unit, 
                    DRV_VLAN_PROP_EGRESS_VT_MISS_UNTAG, port, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmVlanPortJoinAllVlan:
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_GET(unit, 
                    DRV_VLAN_PROP_JOIN_ALL_VLAN, port, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
 
        default:
            rv = BCM_E_UNAVAIL;
            break;
    }
    return rv;
}
int bcm_robo_vlan_mcast_flood_set(int unit,
                             bcm_vlan_t vlan,
                             bcm_vlan_mcast_flood_t mode)
{
#ifdef BCM_TB_SUPPORT
    vlan_entry_t    vt;
    uint32 field_val32;

    if (SOC_IS_TBX(unit)) {
        sal_memset(&vt, 0, sizeof (vt));

        if (mode == BCM_VLAN_MCAST_FLOOD_ALL) {
            return BCM_E_UNAVAIL;
        } else if (mode == BCM_VLAN_MCAST_FLOOD_UNKNOWN) {
            field_val32 = 0;
        } else if (mode == BCM_VLAN_MCAST_FLOOD_NONE) {
            field_val32 = 1;
        } else {
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN, (uint32)vlan, 1, (uint32 *)&vt));

        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_MCAST_DROP,
                        (uint32 *)&vt, (uint32 *)&field_val32));

        BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN, (uint32)vlan, 1, (uint32 *)&vt));

        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}
int bcm_robo_vlan_mcast_flood_get(int unit,
                             bcm_vlan_t vlan,
                             bcm_vlan_mcast_flood_t *mode)
{
#ifdef BCM_TB_SUPPORT
    vlan_entry_t    vt;
    uint32 field_val32;

    if (SOC_IS_TBX(unit)) {
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vlan, 1, (uint32 *)&vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_MCAST_DROP,
                        (uint32 *)&vt, (uint32 *)&field_val32));
        *mode = (field_val32 ? BCM_VLAN_MCAST_FLOOD_NONE : BCM_VLAN_MCAST_FLOOD_UNKNOWN);

        return BCM_E_NONE;
    }
#endif

    return BCM_E_UNAVAIL;
}

int
bcm_robo_vlan_control_vlan_selective_set(int unit, bcm_vlan_t vid, 
                          uint32 valid_fields, bcm_vlan_control_vlan_t *control)
{
    return BCM_E_UNAVAIL;
}


int
bcm_robo_vlan_control_vlan_selective_get(int unit, bcm_vlan_t vid, 
                          uint32 valid_fields, bcm_vlan_control_vlan_t *control)
{
    return BCM_E_UNAVAIL;
}

/* coverity[pass_by_value] */
STATIC int
_bcm_robo_vlan_control_vlan_set(int unit, bcm_vlan_t vid,
/* coverity[pass_by_value] */
                          bcm_vlan_control_vlan_t control)
{
    int rv = BCM_E_NONE;
    int support_mask;
    vlan_entry_t    vt;
    uint32 field_val32;
    bcm_vlan_control_vlan_t support_vctrl;
    uint32 mcrep_num = 0;
    int mc_index = 0;

    sal_memset(&support_vctrl, 0, sizeof(support_vctrl));

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* Check for unsupported parameters */
        support_vctrl.flags = control.flags;
        if (sal_memcmp(&support_vctrl, &control, sizeof(support_vctrl)) != 0) {
            return BCM_E_PARAM;
        }

        support_mask = BCM_VLAN_LEARN_DISABLE;
        if (control.flags & (~support_mask)) { 
            return SOC_E_PARAM;
        }

        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
    
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
    
        field_val32 = (control.flags & BCM_VLAN_LEARN_DISABLE)? 1 : 0;
    
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE,
                        (uint32 *)&vt, (uint32 *)&field_val32));
    
        BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
    } else if (SOC_IS_TBX(unit)){
        /* Check for unsupported parameters */
        support_vctrl.flags = control.flags;
        support_vctrl.if_class = \
            control.if_class;
        support_vctrl.l2_mcast_flood_mode = \
            control.l2_mcast_flood_mode;
        if (!SOC_IS_TB_AX(unit)) {
            support_vctrl.broadcast_group = \
                control.broadcast_group;
            support_vctrl.unknown_multicast_group = \
                control.unknown_multicast_group;
            support_vctrl.unknown_unicast_group = \
                control.unknown_unicast_group;
        }
        if (sal_memcmp(&support_vctrl, &control, sizeof(support_vctrl)) != 0) {
            return BCM_E_PARAM;
        }

        support_mask = (BCM_VLAN_LEARN_DISABLE|
                        BCM_VLAN_L2_LOOKUP_DISABLE|
                        BCM_VLAN_UNKNOWN_UCAST_DROP);
        if (control.flags & (~support_mask)) { 
            return SOC_E_PARAM;
        }

        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
    
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));

        field_val32 = (control.flags & BCM_VLAN_LEARN_DISABLE)? 1 : 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE,
                        (uint32 *)&vt, &field_val32));

        field_val32 = (control.flags & BCM_VLAN_L2_LOOKUP_DISABLE)? 1 : 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_DIR_FWD,
                        (uint32 *)&vt, &field_val32));

        field_val32 = (control.flags & BCM_VLAN_UNKNOWN_UCAST_DROP)? 1 : 0;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_UCAST_DROP,
                        (uint32 *)&vt, &field_val32));

        field_val32 = control.if_class;
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_ISO_MAP,
                        (uint32 *)&vt, &field_val32));


        if (!SOC_IS_TB_AX(unit)) {
            BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_MCAST_REP_NUM, &mcrep_num));

            /* DLF configuration for broadcast/mcast/unicast packet types */
            if (!control.broadcast_group) {
                /* Assigned value is zero, then disable the feature */
                field_val32 = 0;
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, EN_BC_REPf, &field_val32);
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, L2MC_BCf, &field_val32);
            } else if (_BCM_MULTICAST_IS_L2(control.broadcast_group)) {
                /* Only accept _BCM_MULTICAST_TYPE_L2 type */
                field_val32 = 1;
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, EN_BC_REPf, &field_val32);

                mc_index = _BCM_MULTICAST_ID_GET(control.broadcast_group);
                if ((mc_index >= mcrep_num) || (mc_index < 0)) {
                    return BCM_E_PARAM;
                }
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, L2MC_BCf, (uint32 *)&mc_index);
            } else {
                /* For other types, considered error parameters */
                return SOC_E_PARAM;
            }

            if (!control.unknown_multicast_group) {
                /* Assigned value is zero, then disable the feature */
                field_val32 = 0;
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, EN_MLF_REPf, &field_val32);
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, L2MC_MLFf, &field_val32);
            } else if (_BCM_MULTICAST_IS_L2(control.unknown_multicast_group)) {
                /* Only accept _BCM_MULTICAST_TYPE_L2 type */
                field_val32 = 1;
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, EN_MLF_REPf, &field_val32);
                mc_index = _BCM_MULTICAST_ID_GET(control.unknown_multicast_group);
                if ((mc_index >= mcrep_num) || (mc_index < 0)) {
                    return BCM_E_PARAM;
                }
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, L2MC_MLFf, (uint32 *)&mc_index);
            } else {
                /* For other types, considered error parameters */
                return SOC_E_PARAM;
            }

            if (!control.unknown_unicast_group) {
                /* Assigned value is zero, then disable the feature */
                field_val32 = 0;
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, EN_ULF_REPf, &field_val32);
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, L2MC_ULFf, &field_val32);
            } else if (_BCM_MULTICAST_IS_L2(control.unknown_unicast_group)) {
                /* Only accept _BCM_MULTICAST_TYPE_L2 type */
                field_val32 = 1;
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, EN_ULF_REPf, &field_val32);
                mc_index = _BCM_MULTICAST_ID_GET(control.unknown_unicast_group);
                if ((mc_index >= mcrep_num) || (mc_index < 0)) {
                    return BCM_E_PARAM;
                }
                soc_VLAN_1Qm_field_set(unit, (uint32 *)&vt, L2MC_ULFf, (uint32 *)&mc_index);
            } else {
                /* For other types, considered error parameters */
                return SOC_E_PARAM;
            }
        }

        BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));

        field_val32 = control.l2_mcast_flood_mode;
        BCM_IF_ERROR_RETURN(
            bcm_vlan_mcast_flood_set(unit, vid, field_val32));

    }else if (SOC_IS_LOTUS(unit) || SOC_IS_VULCAN(unit) ||
        SOC_IS_BLACKBIRD(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* Check for unsupported parameters */
        support_vctrl.flags = control.flags;
        if (sal_memcmp(&support_vctrl, &control, sizeof(support_vctrl)) != 0) {
            return BCM_E_PARAM;
        }

        support_mask = BCM_VLAN_L2_LOOKUP_DISABLE;
        if (control.flags & (~support_mask)) { 
            return SOC_E_PARAM;
        }

        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
    
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
    
        field_val32 = (control.flags & BCM_VLAN_L2_LOOKUP_DISABLE)? 1 : 0;
    
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE,
                        (uint32 *)&vt, (uint32 *)&field_val32));
    
        BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
    } else {
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

/* coverity[pass_by_value] */
int
bcm_robo_vlan_control_vlan_set(int unit, bcm_vlan_t vid, 
/* coverity[pass_by_value] */
                          bcm_vlan_control_vlan_t control)
{
    int         rv;

    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_control_vlan_set()..\n")));
    CHECK_VID(unit, vid);

    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_control_vlan_set(unit, vid, control);
    BCM_UNLOCK(unit);

    return rv;
}

STATIC int
_bcm_robo_vlan_control_vlan_get(int unit, bcm_vlan_t vid,
                          bcm_vlan_control_vlan_t *control)
{
    int rv = BCM_E_NONE;
    vlan_entry_t    vt;
    uint32 field_value;
    uint32 mc_idx = 0;

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
        /* Upper layer already checks that vid is valid */
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE,
                        (uint32 *)&vt, (uint32 *)&field_value));
        control->flags |= (field_value ? BCM_VLAN_LEARN_DISABLE : 0);
    } else if (SOC_IS_TBX(unit)){
        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
        /* Upper layer already checks that vid is valid */
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE,
                        (uint32 *)&vt, &field_value));
        control->flags |= (field_value ? BCM_VLAN_LEARN_DISABLE : 0);

        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_DIR_FWD,
                        (uint32 *)&vt, &field_value));
        control->flags |= (field_value ? BCM_VLAN_L2_LOOKUP_DISABLE : 0);

        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_UCAST_DROP,
                        (uint32 *)&vt, &field_value));
        control->flags |= (field_value ? BCM_VLAN_UNKNOWN_UCAST_DROP : 0);

        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_ISO_MAP,
                        (uint32 *)&vt, &field_value));
        control->if_class  = field_value;

        if (!SOC_IS_TB_AX(unit)) {
            soc_VLAN_1Qm_field_get(unit, (uint32 *)&vt, 
                EN_BC_REPf, &field_value);
            if (field_value) {
                soc_VLAN_1Qm_field_get(unit, (uint32 *)&vt, 
                    L2MC_BCf, &field_value);
                _BCM_MULTICAST_GROUP_SET(mc_idx, 
                    _BCM_MULTICAST_TYPE_L2, field_value);
            } else {
                mc_idx = 0;
            }
            control->broadcast_group = mc_idx;

            soc_VLAN_1Qm_field_get(unit, (uint32 *)&vt, 
                EN_MLF_REPf, &field_value);
            if (field_value) {
                soc_VLAN_1Qm_field_get(unit, (uint32 *)&vt, 
                    L2MC_MLFf, &field_value);
                _BCM_MULTICAST_GROUP_SET(mc_idx, 
                    _BCM_MULTICAST_TYPE_L2, field_value);
            } else {
                mc_idx = 0;
            }
            control->unknown_multicast_group = mc_idx;

            soc_VLAN_1Qm_field_get(unit, (uint32 *)&vt, 
                EN_ULF_REPf, &field_value);
            if (field_value) {
                soc_VLAN_1Qm_field_get(unit, (uint32 *)&vt, 
                    L2MC_ULFf, &field_value);
                _BCM_MULTICAST_GROUP_SET(mc_idx, 
                    _BCM_MULTICAST_TYPE_L2, field_value);
            } else {
                mc_idx = 0;
            }
            control->unknown_unicast_group = mc_idx;
        }

        BCM_IF_ERROR_RETURN(
            bcm_vlan_mcast_flood_get(unit, vid, &field_value));
        control->l2_mcast_flood_mode = field_value;
    }else if (SOC_IS_LOTUS(unit) || SOC_IS_VULCAN(unit) ||
        SOC_IS_BLACKBIRD(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
        /* Upper layer already checks that vid is valid */
        sal_memset(&vt, 0, sizeof (vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vid, 1, (uint32 *)&vt));
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE,
                        (uint32 *)&vt, (uint32 *)&field_value));
        control->flags |= (field_value ? BCM_VLAN_L2_LOOKUP_DISABLE : 0);
    } else {
        rv = BCM_E_UNAVAIL;
    }
    
    return rv;
}

int
bcm_robo_vlan_control_vlan_get(int unit, bcm_vlan_t vid,
                          bcm_vlan_control_vlan_t *control)
{
    int         rv;

    CHECK_INIT(unit);
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_vlan_port_get()..\n")));
    CHECK_VID(unit, vid);

    BCM_LOCK(unit);
    rv = _bcm_robo_vlan_control_vlan_get(unit, vid, control);
    BCM_UNLOCK(unit);

    return rv;
}

/*
 *   Function
 *      bcm_robovlan_vector_flags_set
 *   Purpose
 *      Set a one or more VLAN control vlan flags on a vlan_vector on this unit
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_vlan_vector_t vlan_vector = Vlan vector for values to be set
 *      (in) uint32 flags_mask
 *      (in) uint32 flags_value
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_robo_vlan_vector_flags_set(int unit,
                               bcm_vlan_vector_t vlan_vector,
                               uint32 flags_mask,
                               uint32 flags_value)
{
    int rv;

    bcm_vlan_t              vid;
    bcm_vlan_control_vlan_t control;

    /* non-supported vector flags */
    if (flags_mask & (BCM_VLAN_USE_FABRIC_DISTRIBUTION | BCM_VLAN_COSQ_ENABLE)) {
        return BCM_E_PARAM;
    }

    /* Optimistically assume success from here */
    rv = BCM_E_NONE;

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {

        if (BCM_VLAN_VEC_GET(vlan_vector, vid)) {
            sal_memset(&control, 0, sizeof(control));

            rv = bcm_robo_vlan_control_vlan_get(unit, vid, &control);

            control.flags = (~flags_mask & control.flags) | 
                    (flags_mask & flags_value);

            if (BCM_SUCCESS(rv)) {
                rv = bcm_robo_vlan_control_vlan_set(unit, vid, control);
                if (BCM_FAILURE(rv)){
                    LOG_INFO(BSL_LS_BCM_VLAN,
                             (BSL_META_U(unit,
                                         "%s, fail on setting control flag on VLAN(%d)\n"),
                              FUNCTION_NAME(), vid));
                    break;
                }
            } else {
                LOG_INFO(BSL_LS_BCM_VLAN,
                         (BSL_META_U(unit,
                                     "%s, fail on retrieving control flag in VLAN(%d)\n"),
                          FUNCTION_NAME(), vid));
                break;
            }
        }
    }
    return rv;
}

int
bcm_robo_vlan_mac_action_add(int unit, bcm_mac_t mac, bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_vlan_mac_action_get(int unit, bcm_mac_t mac, bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_vlan_mac_action_delete(int unit, bcm_mac_t mac)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_vlan_mac_action_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_mac_action_traverse(int unit, 
                                 bcm_vlan_mac_action_traverse_cb cb, 
                                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Port-based VLAN actions
 */
/* 
 * Function:
 *      bcm_vlan_port_default_action_set
 * Purpose: 
 *      Set the port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      action     - (IN) Vlan tag actions
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_port_default_action_set(int unit, bcm_port_t port,
                                     bcm_vlan_action_set_t *action)
{   
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = 0;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = 0;
        vt_act_param.vt_api_type = VT_API_VLAN_PORT_DEFAULT;

        rv = _robo_vlan_translate_action_add(unit, &vt_act_param, action);

        /* If existed, delete the old VM entry and update for new vlan tag actions */
        if (rv == BCM_E_EXISTS) {
            rv = _robo_vlan_translate_action_delete(unit, &vt_act_param);

            if (rv == BCM_E_NONE) {
                rv = _robo_vlan_translate_action_add(unit, &vt_act_param, action);
            }
        }

    }
#endif

    return rv;
}

/*  
 * Function:
 *      bcm_vlan_port_default_action_get
 * Purpose:
 *      Get the port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      action     - (OUT) Vlan tag actions
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_port_default_action_get(int unit, bcm_port_t port,
                                     bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_UNAVAIL;
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = 0;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = 0;
        vt_act_param.vt_api_type = VT_API_VLAN_PORT_DEFAULT;
        rv = _robo_vlan_translate_action_get(unit, &vt_act_param, action);
    }
#endif

    return rv;
}

/*
 * Function:
 *      bcm_vlan_port_default_action_delete
 * Purpose:
 *      Delete the port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_port_default_action_delete(int unit, bcm_port_t port)
{
    int rv = BCM_E_UNAVAIL;
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;
        bcm_vlan_action_set_t action;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = 0;
        vt_act_param.param1 = 0;
        vt_act_param.param2 = 0;
        vt_act_param.vt_api_type = VT_API_VLAN_PORT_DEFAULT;
        rv = _robo_vlan_translate_action_delete(unit, &vt_act_param);

        /*  
         * All ports are initially configured with 'ut_outer' action set to bcmVlanActionAdd.
         * With this setting, untagged packets have an outer tag added with the default VLAN
         *  (VLAN 1). All other action settings are set to bcmVlanActionNone.
         */
        sal_memset(&action, 0, sizeof(bcm_vlan_action_set_t));
        if (rv == BCM_E_NONE) {
            action.new_outer_vlan = BCM_VLAN_DEFAULT;
            action.ut_outer = bcmVlanActionAdd;
            rv = bcm_vlan_port_default_action_set(unit, port, &action);
        }
    }
#endif

    return rv;
}

/*
 * Function:
 *      bcm_vlan_port_egress_default_action_set
 * Purpose:
 *      Set the egress port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      action     - (IN) Vlan tag actions
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_port_egress_default_action_set(int unit, bcm_port_t port,
                                            bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_module_t        modid;
        bcm_trunk_t         tgid;
        int                 id;
        bcm_port_t local_port;

        /* Gport resolution */
        if (BCM_GPORT_IS_SET(port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));
            if ((-1 != tgid) || (-1 != id)){
                return BCM_E_PORT;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            local_port = port;
        }
        rv = _robo_vlan_port_egress_default_action_set(unit, local_port, action);
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_vlan_port_egress_default_action_get
 * Purpose:
 *      Get the egress port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      action     - (OUT) Vlan tag actions
 * Returns:
 *      BCM_E_XXX
 */     
int
bcm_robo_vlan_port_egress_default_action_get(int unit, bcm_port_t port,
                                            bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_module_t        modid;
        bcm_trunk_t         tgid;
        int                 id;
        bcm_port_t local_port;

        /* Gport resolution */
        if (BCM_GPORT_IS_SET(port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));
            if ((-1 != tgid) || (-1 != id)){
                return BCM_E_PORT;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            local_port = port;
        }
        rv = _robo_vlan_port_egress_default_action_get(unit, local_port, action);
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_vlan_port_egress_default_action_delete
 * Purpose:
 *      delete the egress port's default vlan tag actions
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 * Returns:
 *      BCM_E_XXX
 */     
int
bcm_robo_vlan_port_egress_default_action_delete(int unit, bcm_port_t port)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_module_t        modid;
        bcm_trunk_t         tgid;
        int                 id;
        bcm_port_t local_port;

        /* Gport resolution */
        if (BCM_GPORT_IS_SET(port)) {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));
            if ((-1 != tgid) || (-1 != id)){
                return BCM_E_PORT;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            local_port = port;
        }
        rv = _robo_vlan_port_egress_default_action_delete(unit, local_port);
    }
#endif
    return rv;
}

/*
 * Protocol-based VLAN actions
 */
/*
 * Function   :
 *      bcm_vlan_port_protocol_action_add
 * Description   :
 *      Add protocol based VLAN with specified action.
 *      If the entry already exists, update the action.
 * Parameters   :
 *      unit      (IN) BCM unit number
 *      port      (IN) Port number
 *      frame     (IN) Frame type
 *      ether     (IN) 16 bit ether type
 *      action    (IN) Action for outer tag and inner tag
 * Note:
 * 
 */
int
bcm_robo_vlan_port_protocol_action_add(int unit,
                                      bcm_port_t port,
                                      bcm_port_frametype_t frame,
                                      bcm_port_ethertype_t ether,
                                      bcm_vlan_action_set_t *action)
{
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = 0;
        vt_act_param.param1 = frame;
        vt_act_param.param2 = ether;
        vt_act_param.vt_api_type = VT_API_VLAN_PROTOCOL;

        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_add(unit, &vt_act_param, action));

        return BCM_E_NONE;
    }
#endif

    BCM_IF_ERROR_RETURN(
            _bcm_robo_vlan_port_protocol_action_add(unit, port, frame,
                                                 ether, action));
    return BCM_E_NONE;

}

/*
 * Function   :
 *      bcm_vlan_port_protocol_action_delete
 * Description   :
 *      Delete protocol based VLAN action.
 * Parameters   :
 *      unit      (IN) BCM unit number
 *      port      (IN) Port number
 *      frame     (IN) Frame type
 *      ether     (IN) 16 bit ether type
 * Note:
 *    Program VLAN_PROTOCOL_DATAm and VLAN_PROTOCOLm.
 */
int
bcm_robo_vlan_port_protocol_action_delete(int unit,bcm_port_t port,
                                         bcm_port_frametype_t frame,
                                         bcm_port_ethertype_t ether)
{
    int rv = BCM_E_UNAVAIL; 
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }


        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = 0;
        vt_act_param.param1 = frame;
        vt_act_param.param2 = ether;
        vt_act_param.vt_api_type = VT_API_VLAN_PROTOCOL;

        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_delete(unit, &vt_act_param));

        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_protocol_vlan_delete(unit, port, frame, ether));

        return BCM_E_NONE;
    }

    return rv;
}


/*
 * Function   :
 *      bcm_vlan_port_protocol_action_delete_all
 * Description   :
 *      Delete all protocol based VLAN actiona.
 * Parameters   :
 *      unit      (IN) BCM unit number
 *      port      (IN) Port number
 * Note:
 *    Program VLAN_PROTOCOL_DATAm and VLAN_PROTOCOLm.
 */
int
bcm_robo_vlan_port_protocol_action_delete_all(int unit,bcm_port_t port)
{
    if (!soc_feature(unit, soc_feature_vlan_translation)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }


        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_delete_all(unit, \
                VT_API_VLAN_PROTOCOL, gport));

        /* Remove all information for traverse. */
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_protocol_vlan_delete_all(unit, port));

        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function   :
 *      bcm_vlan_port_protocol_action_get
 * Description   :
 *      Get protocol based VLAN with specified action.
 * Parameters   :
 *      unit      (IN) BCM unit number
 *      port      (IN) Port number
 *      frame     (IN) Frame type
 *      ether     (IN) 16 bit ether type
 *      action    (OUT) Action for outer and inner tag
 * Note:
 *    
 */
int
bcm_robo_vlan_port_protocol_action_get(int unit,
                                      bcm_port_t port,
                                      bcm_port_frametype_t frame,
                                      bcm_port_ethertype_t ether,
                                      bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
        BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }


        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = 0;
        vt_act_param.param1 = frame;
        vt_act_param.param2 = ether;
        vt_act_param.vt_api_type = VT_API_VLAN_PROTOCOL;
        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_get(unit, &vt_act_param, action));

        return BCM_E_NONE;
    }
#endif
    return rv;
}

/*
 * Function   :
 *      bcm_vlan_port_protocol_action_traverse
 * Description   :
 *      Traverse over vlan port protocol actions. 
 * Parameters   :
 *      unit      (IN) BCM unit number
 *      cb        (IN) User provided call back function
 *      user_data (IN) User provided data
 * Note:
 *    Program VLAN_PROTOCOL_DATAm and VLAN_PROTOCOLm.
 */
int
bcm_robo_vlan_port_protocol_action_traverse(int unit, 
                                bcm_vlan_port_protocol_action_traverse_cb cb,
                                           void *user_data)
{
    int rv = BCM_E_UNAVAIL; 

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action))  {
        rv = _robo_vlan_protocol_action_traverse(unit, 
            cb, user_data);
        return rv;
    }
#endif

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        bcm_vlan_action_set_t action;
        uint32 index_min, index_max, field_value;
        int i, prio;
        protocol2vlan_entry_t  protcolment;
        bcm_port_t local_port = -1;
        bcm_port_frametype_t frame = 0;
        uint32 ether;
        uint32 vid;
        
        bcm_vlan_action_set_t_init(&action);

        index_min = soc_robo_mem_index_min(unit, INDEX(PROTOCOL2VLANm));
        index_max = soc_robo_mem_index_max(unit, INDEX(PROTOCOL2VLANm));
        for (i = index_min; i <= index_max; i++) {
            rv = DRV_MEM_READ(unit, DRV_MEM_PROTOCOLVLAN, i, 1, 
                                              (uint32 *)&protcolment);
            BCM_IF_ERROR_RETURN(rv);

            rv = DRV_MEM_FIELD_GET
                            (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                            (uint32 *)&protcolment, &field_value);
            BCM_IF_ERROR_RETURN(rv);

            if (field_value) {
                rv = DRV_MEM_FIELD_GET
                                (unit, DRV_MEM_PROTOCOLVLAN, 
                                 DRV_MEM_FIELD_ETHER_TYPE,
                                 (uint32 *)&protcolment, &ether);
                BCM_IF_ERROR_RETURN(rv);

                rv = DRV_MEM_FIELD_GET
                                (unit, DRV_MEM_PROTOCOLVLAN, 
                                 DRV_MEM_FIELD_VLANID,
                                 (uint32 *)&protcolment, &vid);
                BCM_IF_ERROR_RETURN(rv);

                rv = DRV_MEM_FIELD_GET
                                (unit, DRV_MEM_PROTOCOLVLAN, 
                                 DRV_MEM_FIELD_PRIORITY,
                                 (uint32 *)&protcolment, (uint32 *)&prio);
                BCM_IF_ERROR_RETURN(rv);

                action.new_outer_vlan = (bcm_vlan_t)vid;
                action.new_inner_vlan = (bcm_vlan_t)vid;
                action.priority = prio;

                /* Call application call-back */
                cb(unit, local_port, frame, (bcm_ethertype_t)ether, &action, user_data);
            }
        }
        return BCM_E_NONE;
    }

    return rv;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_add
 * Purpose:
 *      Add a VLAN cross connect entry
 * Parameters:
 *      unit       - Device unit number
 *      outer_vlan - Outer vlan ID
 *      inner_vlan - Inner vlan ID
 *      port_1     - First port in the cross-connect
 *      port_2     - Second port in the cross-connect
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_cross_connect_add(int unit,
                               bcm_vlan_t outer_vlan, bcm_vlan_t inner_vlan,
                               bcm_gport_t port_1, bcm_gport_t port_2)
{
#ifdef BCM_TB_SUPPORT
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;
    bcm_port_t local_port_1, local_port_2;
    pbmp_t vlan_pbmp, vlan_ubmp;
    pbmp_t cc_pbmp, cc_ubmp;
    bcm_vlan_control_vlan_t control;

    BCM_PBMP_CLEAR(vlan_pbmp);

    if (SOC_IS_TBX(unit)) {
        if (inner_vlan != BCM_VLAN_INVALID) {
            /* 
              * Double cross-connect (use both outer_vid and inner_vid) 
              * is not supported in BCM53280.
              */
            return BCM_E_UNAVAIL;
        }

        /* Single cross-connect (use only outer_vid) is supported. */
        if ((outer_vlan < 1) || (outer_vlan > 4095)) {
            return BCM_E_PARAM;
        }

        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, outer_vlan))) {
            return BCM_E_NOT_FOUND;
        }

        /* Resolve first port */
        if (BCM_GPORT_IS_SET(port_1)) {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, port_1, &modid, &local_port_1, &tgid, &id));
            if ((-1 != tgid) || (-1 != id)){
                return BCM_E_PORT;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port_1)) { 
                return BCM_E_PORT; 
            }
            local_port_1 = port_1;
        }
        /* Resolve second port */
        if (BCM_GPORT_IS_SET(port_2)) {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, port_2, &modid, &local_port_2, &tgid, &id));
            if ((-1 != tgid) || (-1 != id)){
                return BCM_E_PORT;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port_2)) { 
                return BCM_E_PORT; 
            }
            local_port_2 = port_2;
        }

        /* Setup cross connect portbitmap */
        BCM_PBMP_CLEAR(cc_pbmp);
        BCM_PBMP_CLEAR(cc_ubmp);
        BCM_PBMP_PORT_ADD(cc_pbmp, local_port_1);
        BCM_PBMP_PORT_ADD(cc_pbmp, local_port_2);
        BCM_PBMP_PORT_ADD(cc_ubmp, local_port_1);
        BCM_PBMP_PORT_ADD(cc_ubmp, local_port_2);

        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_port_get(unit, outer_vlan,
                               &vlan_pbmp, &vlan_ubmp));

        /* Check if the existed portbitmap has other ports than cc ports */
        BCM_PBMP_REMOVE(vlan_pbmp, cc_pbmp);
        if (BCM_PBMP_NOT_NULL(vlan_pbmp)) {
            /* 
             * Since cross connect is a special configuration of VLAN.
             * Applications should not configure more than 2 ports in the VLAN.
             */
            return BCM_E_CONFIG;
        }
        BCM_IF_ERROR_RETURN(bcm_vlan_port_add(unit, 
                                outer_vlan, cc_pbmp, cc_ubmp));

        /* Enable DIR_FWD bit in the vlan table */
        sal_memset(&control, 0, sizeof(bcm_vlan_control_vlan_t));
        BCM_IF_ERROR_RETURN(bcm_vlan_control_vlan_get(unit, 
                                outer_vlan, &control));
        control.flags |= BCM_VLAN_L2_LOOKUP_DISABLE;
        BCM_IF_ERROR_RETURN(bcm_vlan_control_vlan_set(unit, 
                                outer_vlan, control));

        /* Update software copy to do delete and traverse */
        if (!_bcm_robo_vlist_cross_connect_set(&robo_vlan_info[unit].list, 
                                               outer_vlan, 1)) {
            return BCM_E_NOT_FOUND;
        }

        return BCM_E_NONE;
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_delete
 * Purpose:
 *      Delete a VLAN cross connect entry
 * Parameters:
 *      unit       - Device unit number
 *      outer_vlan - Outer vlan ID
 *      inner_vlan - Inner vlan ID
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_cross_connect_delete(int unit,
                                  bcm_vlan_t outer_vlan,
                                  bcm_vlan_t inner_vlan)
{
#ifdef BCM_TB_SUPPORT
    bcm_vlan_control_vlan_t control;
    bcm_pbmp_t pbmp, ubmp;

    if (SOC_IS_TBX(unit)) {
        if (inner_vlan != BCM_VLAN_INVALID) {
            /* 
              * Double cross-connect (use both outer_vid and inner_vid) 
              * is not supported in BCM53280.
              */
            return BCM_E_UNAVAIL;
        }

        /* Single cross-connect (use only outer_vid) is supported. */
        if ((outer_vlan < 1) || (outer_vlan > 4095)) {
            return BCM_E_PARAM;
        }

        /* Check if this VLAN configured as cross connect */
        if (!_bcm_robo_vlist_is_cross_connect(&robo_vlan_info[unit].list, 
                                               outer_vlan)) {
            return BCM_E_NOT_FOUND;
        }

        /* Disable DIR_FWD bit in the vlan table */
        sal_memset(&control, 0, sizeof(bcm_vlan_control_vlan_t));
        BCM_IF_ERROR_RETURN(bcm_vlan_control_vlan_get(unit, 
                                outer_vlan, &control));
        control.flags &= ~BCM_VLAN_L2_LOOKUP_DISABLE;
        BCM_IF_ERROR_RETURN(bcm_vlan_control_vlan_set(unit, 
                                outer_vlan, control));

        /* Now remove all local physical ports */
        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_CLEAR(ubmp);
        BCM_IF_ERROR_RETURN(bcm_vlan_port_get(unit, outer_vlan, &pbmp, &ubmp));
        BCM_IF_ERROR_RETURN(bcm_vlan_port_remove(unit, outer_vlan, pbmp));

        /* Update software copy */
        if (!_bcm_robo_vlist_cross_connect_set(&robo_vlan_info[unit].list, 
                                               outer_vlan, 0)) {
            return BCM_E_NOT_FOUND;
        }

        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_delete_all
 * Purpose:
 *      Delete all VLAN cross connect entries
 * Parameters:
 *      unit       - Device unit number
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_cross_connect_delete_all(int unit)
{
#ifdef BCM_TB_SUPPORT
    vlist_t     *v;

    if (SOC_IS_TBX(unit)) {
        for (v = robo_vlan_info[unit].list; v != NULL; v = v->next) {
            /* Check if this VLAN configured as cross connect */
            if (_bcm_robo_vlist_is_cross_connect(&robo_vlan_info[unit].list, 
                                                   v->vid)) {
                BCM_IF_ERROR_RETURN(bcm_vlan_cross_connect_delete(unit, 
                                        v->vid, BCM_VLAN_INVALID));
            }

        }

        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_traverse
 * Purpose:
 *      Walks through the valid cross connect entries and calls
 *      the user supplied callback function for each entry.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function.
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_robo_vlan_cross_connect_traverse(int unit,
                                    bcm_vlan_cross_connect_traverse_cb cb,
                                    void *user_data)
{
#ifdef BCM_TB_SUPPORT
    vlist_t     *v;
    bcm_port_t port_1, port_2;
    bcm_gport_t gport_1, gport_2;
    bcm_port_t port_tmp;
    pbmp_t cc_pbmp, cc_ubmp;
    int rv = BCM_E_NONE;

    if (SOC_IS_TBX(unit)) {
        for (v = robo_vlan_info[unit].list; v != NULL; v = v->next) {
            port_1 = -1;
            port_2 = -1;
            /* Check if this VLAN configured as cross connect */
            if (_bcm_robo_vlist_is_cross_connect(&robo_vlan_info[unit].list, 
                                                   v->vid)) {
                /* Get member ports */
                BCM_IF_ERROR_RETURN(bcm_vlan_port_get(unit, v->vid, &cc_pbmp, &cc_ubmp));
                BCM_PBMP_ITER(cc_pbmp, port_tmp) {
                    if (port_1 < 0) {
                        port_1 = port_tmp;
                    } else if (port_2 < 0) { 
                        port_2 = port_tmp;
                    } else {
                        return BCM_E_CONFIG;
                    }
                }

                BCM_IF_ERROR_RETURN(
                    bcm_robo_port_gport_get(unit, port_1, &gport_1));
                BCM_IF_ERROR_RETURN(
                    bcm_robo_port_gport_get(unit, port_2, &gport_2));

                /* Call application call-back */
                rv = cb(unit, v->vid, BCM_VLAN_INVALID, gport_1, gport_2, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
                if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                    return rv;
                }
#endif
            }
        }

        return rv;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function :
 *      bcm_vlan_translate_action_range_add
 * Description :
 *   Add an entry to the VLAN Translation table, which assigns
 *   VLAN actions for packets matching within the VLAN range(s).
 * Parameters :
 *      unit            (IN) BCM unit number
 *      port            (IN) Ingress gport (global port)
 *      outer_vlan_low  (IN) Outer VLAN ID Low
 *      outer_vlan_high (IN) Outer VLAN ID High
 *      inner_vlan_low  (IN) Inner VLAN ID Low
 *      inner_vlan_high (IN) Inner VLAN ID High
 *      action          (IN) Action for outer and inner tag
 *
 * Notes :
 *   For translation of double-tagged packets, specify a valid
 *   VLAN ID value for outer_vlan_low/high and inner_vlan_low/high.
 *   For translation of single outer-tagged packets, specify a
 *   valid VLAN ID for outer_vlan_low/high and BCM_VLAN_INVALID
 *   for inner_vlan_low/high. For translation of single inner-tagged
 *   packets, specify a valid VLAN ID for inner_vlan_low/high and
 *   BCM_VLAN_INVALID for outer_vlan_low/high.
 */

int
bcm_robo_vlan_translate_action_range_add(int unit, bcm_gport_t port,
                                        bcm_vlan_t outer_vlan_low,
                                        bcm_vlan_t outer_vlan_high,
                                        bcm_vlan_t inner_vlan_low,
                                        bcm_vlan_t inner_vlan_high,
                                        bcm_vlan_action_set_t *action)
{
    CHECK_INIT(unit);
    if (BCM_VLAN_INVALID != outer_vlan_low) {
    CHECK_VID(unit, outer_vlan_low);
    }
    if (BCM_VLAN_INVALID != outer_vlan_high) {
    CHECK_VID(unit, outer_vlan_high);
    }
    if (BCM_VLAN_INVALID != inner_vlan_low) {
    CHECK_VID(unit, inner_vlan_low);
    }
    if (BCM_VLAN_INVALID != inner_vlan_high) {
    CHECK_VID(unit, inner_vlan_high);
    }

#ifdef BCM_TB_SUPPORT
    /* 
      * BCM53280 only support translation of single inner-tagged packets with vid range. 
      * The outer_vlan_low/high should be BCM_VLAN_INVALID.
      */
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        int rv = BCM_E_NONE;
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        /* Double-tagged and single outer-tagged are not supported for TB.*/
        if (outer_vlan_low != BCM_VLAN_INVALID || 
            outer_vlan_high != BCM_VLAN_INVALID) {
            return BCM_E_UNAVAIL;
        }

        if (inner_vlan_low == BCM_VLAN_INVALID || 
            inner_vlan_high == BCM_VLAN_INVALID) {
            return BCM_E_PARAM;
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = inner_vlan_low;
        vt_act_param.param2 = inner_vlan_high;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_ACTION_RANGE;
    
        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_add(unit, &vt_act_param, action));

        /* Save information for traverse. */
        rv = _robo_vlan_translate_action_range_traverse_info_add(unit, 
            gport, inner_vlan_low, inner_vlan_high, action);
        if (rv < 0) {
            /* 
              * Traverse information sw copy create failed. 
              */
            _robo_vlan_translate_action_delete(unit, &vt_act_param);
        }
        return rv;
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function :
 *      bcm_vlan_translate_action_range_delete
 * Description :
 *   Delete an entry from the VLAN Translation table for the
 *   specified VLAN range(s).
 * Parameters :
 *      unit            (IN) BCM unit number
 *      port            (IN) Ingress gport (global port)
 *      outer_vlan_low  (IN) Outer VLAN ID Low
 *      outer_vlan_high (IN) Outer VLAN ID High
 *      inner_vlan_low  (IN) Inner VLAN ID Low
 *      inner_vlan_high (IN) Inner VLAN ID High
 *
 * Notes :
 *   For translation of double-tagged packets, specify a valid
 *   VLAN ID value for outer_vlan_low/high and inner_vlan_low/high.
 *   For translation of single outer-tagged packets, specify a
 *   valid VLAN ID for outer_vlan_low/high and BCM_VLAN_INVALID
 *   for inner_vlan_low/high. For translation of single inner-tagged
 *   packets, specify a valid VLAN ID for inner_vlan_low/high and
 *   BCM_VLAN_INVALID for outer_vlan_low/high.
 */
int
bcm_robo_vlan_translate_action_range_delete(int unit, bcm_gport_t port,
                                           bcm_vlan_t outer_vlan_low,
                                           bcm_vlan_t outer_vlan_high,
                                           bcm_vlan_t inner_vlan_low,
                                           bcm_vlan_t inner_vlan_high)
{
    CHECK_INIT(unit);
    if (BCM_VLAN_INVALID != outer_vlan_low) {
    CHECK_VID(unit, outer_vlan_low);
    }
    if (BCM_VLAN_INVALID != outer_vlan_high) {
    CHECK_VID(unit, outer_vlan_high);
    }
    if (BCM_VLAN_INVALID != inner_vlan_low) {
    CHECK_VID(unit, inner_vlan_low);
    }
    if (BCM_VLAN_INVALID != inner_vlan_high) {
    CHECK_VID(unit, inner_vlan_high);
    }

#ifdef BCM_TB_SUPPORT
    /* 
      * BCM53280 only support translation of single inner-tagged packets with vid range. 
      * The outer_vlan_low/high should be BCM_VLAN_INVALID.
      */
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_gport_t gport;
        _robo_vlan_translate_action_param_t vt_act_param;

        if (BCM_GPORT_IS_SET(port)) {
            gport = port;
        } else {
            if (!SOC_PORT_VALID(unit, port)) { 
                return BCM_E_PORT; 
            }
            BCM_IF_ERROR_RETURN(
                bcm_robo_port_gport_get(unit, port, &gport));
        }

        /* Double-tagged and single outer-tagged are not supported for TB.*/
        if (outer_vlan_low != BCM_VLAN_INVALID || 
            outer_vlan_high != BCM_VLAN_INVALID) {
            return BCM_E_UNAVAIL;
        }

        if (inner_vlan_low == BCM_VLAN_INVALID || 
            inner_vlan_high == BCM_VLAN_INVALID) {
            return BCM_E_PARAM;
        }

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = gport;
        vt_act_param.key_type = bcmVlanTranslateKeyPortInner;
        vt_act_param.param1 = inner_vlan_low;
        vt_act_param.param2 = inner_vlan_high;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_ACTION_RANGE;
        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_delete(unit, &vt_act_param));

        /* Remove information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_range_traverse_info_remove(unit, 
                                                  gport, inner_vlan_low, inner_vlan_high));

        return BCM_E_NONE;
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function :
 *      bcm_vlan_translate_action_range_delete_all
 * Description :
 *     Delete all VLAN range entries from the VLAN Translation table.
 * Parameters :
 *      unit            (IN) BCM unit number
 */

int
bcm_robo_vlan_translate_action_range_delete_all(int unit)
{
    CHECK_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        BCM_IF_ERROR_RETURN(
            _robo_vlan_translate_action_delete_all(unit, 
                VT_API_VLAN_XLATE_ACTION_RANGE, 0));
        /* Remove all information for traverse. */
        BCM_IF_ERROR_RETURN
            (_robo_vlan_translate_action_range_traverse_info_remove_all(unit));
        return BCM_E_NONE;
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function   :
 *      bcm_vlan_translate_action_get
 * Description   :
 *      Get an entry to ingress VLAN translation table.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      port            (IN) Generic port
 *      key_type        (IN) Key Type : bcmVlanTranslateKey*
 *      outer_vlan      (IN) Packet outer VLAN ID
 *      inner_vlan      (IN) Packet inner VLAN ID
 *      action          (OUT) Action for outer and inner tag
 */
int 
bcm_robo_vlan_translate_action_get (int unit, bcm_gport_t port,
                                   bcm_vlan_translate_key_t key_type,
                                   bcm_vlan_t outer_vlan,
                                   bcm_vlan_t inner_vlan,
                                   bcm_vlan_action_set_t *action)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action))  {
        _robo_vlan_translate_action_param_t vt_act_param;

        sal_memset(&vt_act_param, 0, sizeof(_robo_vlan_translate_action_param_t));
        vt_act_param.port = port;
        vt_act_param.key_type = key_type;
        vt_act_param.param1 = outer_vlan;
        vt_act_param.param2 = inner_vlan;
        vt_act_param.vt_api_type = VT_API_VLAN_XLATE_ACTION;
        rv = _robo_vlan_translate_action_get(unit, &vt_act_param, action);
        return rv;
    }
#endif

    return rv;
}

/*
 * Function   :
 *      bcm_vlan_translate_traverse
 * Description   :
 *      Traverse over all translate entries and call given call back with 
 *      new vid and prio.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      cb              (IN) Call back function
 *      user_data       (IN) User provided data to pass to a call back
 */
int
bcm_robo_vlan_translate_traverse(int unit, bcm_vlan_translate_traverse_cb cb, 
                            void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_translation))  {
        rv = _robo_vlan_translate_traverse(unit, cb, user_data);
        return rv;
    }
#endif

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        uint32 index_min, index_max;
        vlan2vlan_entry_t vt_entry;
        int i;
        int prio = 0;
        bcm_port_t local_port = -1;
        uint32 new_vid = 0;
        uint32 mapping_mode = 0;
        
        index_min = soc_robo_mem_index_min(unit, INDEX(VLAN2VLANm));
        index_max = soc_robo_mem_index_max(unit, INDEX(VLAN2VLANm));
        for (i = index_min; i <= index_max; i++) {
            rv = DRV_MEM_READ(unit, DRV_MEM_VLANVLAN, i, 1, 
                                              (uint32 *)&vt_entry);
            BCM_IF_ERROR_RETURN(rv);

            rv = DRV_MEM_FIELD_GET
                            (unit, DRV_MEM_VLANVLAN, DRV_MEM_FIELD_NEW_VLANID,
                            (uint32 *)&vt_entry, &new_vid);
            BCM_IF_ERROR_RETURN(rv);

            if (new_vid) {
                rv = DRV_MEM_FIELD_GET
                                (unit, DRV_MEM_VLANVLAN, 
                                 DRV_MEM_FIELD_MAPPING_MODE,
                                 (uint32 *)&vt_entry, &mapping_mode);
                BCM_IF_ERROR_RETURN(rv);

                if (mapping_mode == VLAN_VTMODE_MAPPING) {
                    /* Call application call-back */
                    cb(unit, local_port, i, new_vid, prio, user_data);
                }
            }
        }
        return BCM_E_NONE;
    }

    return rv;
}


/*
 * Function   :
 *      bcm_vlan_translate_egress_traverse
 * Description   :
 *      Traverse over all translate entries and call given call back with 
 *      new vid and prio.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      cb              (IN) Call back function
 *      user_data       (IN) User provided data to pass to a call back
 */
int
bcm_robo_vlan_translate_egress_traverse(int unit, 
                                   bcm_vlan_translate_egress_traverse_cb cb, 
                                   void *user_data)
{
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int i,start=0,this;
        drv_vt_cfp_entry_db_t ent;
        bcm_port_t local_port = -1;
        int prio = 0;
        uint32 vt_mode = 0, vt_new_vid = 0;
        
        start = vt_cfp_db.vt_cfp_db_start_id;
        if (start == VTCFP_NULL_INDEX) {
        	return BCM_E_NONE;
        }
        this = start;
        
        for (i=0; i<vt_cfp_db.vt_cfp_db_valid_count; i++){
            ent = vt_cfp_db.vt_cfp_entry_db[this];
            BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
                    (unit, DRV_VLAN_PROP_VT_MODE, ent.vid, 0, &vt_mode));
            if (vt_mode == VLAN_VTMODE_MAPPING) {
                BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
                        (unit, DRV_VLAN_PROP_EGR_VT_SPVID, ent.vid, 0, &vt_new_vid));
                cb(unit, local_port, ent.vid, vt_new_vid, prio, user_data);
            }
            this = ent.next;
            if (!IS_VALID_VTCFP_DB_ENTRY_ID(this)){
                break;
            }
        }
        return BCM_E_NONE;
    }
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/

    return BCM_E_UNAVAIL;
}

/*
 * Function   :
 *      bcm_vlan_translate_egress_traverse
 * Description   :
 *      Traverse over all translate entries and call given call back with 
 *      new vid and prio.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      cb              (IN) Call back function
 *      user_data       (IN) User provided data to pass to a call back
 */
int
bcm_robo_vlan_dtag_traverse(int unit, 
                       bcm_vlan_dtag_traverse_cb cb, 
                       void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_translation))  {
        rv = _robo_vlan_dtag_traverse(unit, cb, user_data);
        return rv;
    }
#endif

    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        uint32 index_min, index_max;
        vlan2vlan_entry_t vt_entry;
        int i;
        int prio = 0;
        bcm_port_t local_port = -1;
        uint32 new_vid = 0;
        uint32 mapping_mode = 0;
        
        index_min = soc_robo_mem_index_min(unit, INDEX(VLAN2VLANm));
        index_max = soc_robo_mem_index_max(unit, INDEX(VLAN2VLANm));
        for (i = index_min; i <= index_max; i++) {
            rv = DRV_MEM_READ(unit, DRV_MEM_VLANVLAN, i, 1, 
                                              (uint32 *)&vt_entry);
            BCM_IF_ERROR_RETURN(rv);

            rv = DRV_MEM_FIELD_GET
                            (unit, DRV_MEM_VLANVLAN, DRV_MEM_FIELD_NEW_VLANID,
                            (uint32 *)&vt_entry, &new_vid);
            BCM_IF_ERROR_RETURN(rv);

            if (new_vid != i) {
                rv = DRV_MEM_FIELD_GET
                                (unit, DRV_MEM_VLANVLAN, 
                                 DRV_MEM_FIELD_MAPPING_MODE,
                                 (uint32 *)&vt_entry, &mapping_mode);
                BCM_IF_ERROR_RETURN(rv);

                if (mapping_mode == VLAN_VTMODE_TRANSPATENT) {
                    /* Call application call-back */
                    cb(unit, local_port, i, new_vid, prio, user_data);
                }
            }
        }
        return BCM_E_NONE;
    }

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        int i,start=0,this;
        drv_vt_cfp_entry_db_t ent;
        bcm_port_t local_port = -1;
        int prio = 0;
        uint32 vt_mode = 0, vt_new_vid = 0;
        
        start = vt_cfp_db.vt_cfp_db_start_id;
        if (start == VTCFP_NULL_INDEX) {
        	return BCM_E_NONE;
        }
        this = start;
        
        for (i=0; i<vt_cfp_db.vt_cfp_db_valid_count; i++){
            ent = vt_cfp_db.vt_cfp_entry_db[this];
            BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
                    (unit, DRV_VLAN_PROP_VT_MODE, ent.vid, 0, &vt_mode));
            if (vt_mode == VLAN_VTMODE_TRANSPATENT) {
                BCM_IF_ERROR_RETURN(DRV_VLAN_VT_GET
                        (unit, DRV_VLAN_PROP_EGR_VT_SPVID, ent.vid, 0, &vt_new_vid));
                cb(unit, local_port, ent.vid, vt_new_vid, prio, user_data);
            }
            this = ent.next;
            if (!IS_VALID_VTCFP_DB_ENTRY_ID(this)){
                break;
            }
        }

        return BCM_E_NONE;
    }
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3 */

    return rv;
}

/*
 * Function :
 *      bcm_vlan_dtag_range_traverse
 * Description :
 *   Traverses over VLAN double tagging table and call provided callback 
 *   with valid entries.
 * Parameters :
 *      unit            (IN) BCM unit number
 *      cb              (IN) User callback function
 *      user_data       (IN) Pointer to user specific data
 * Return:
 *
 *      BCM_E_XXX
 */
int 
bcm_robo_vlan_dtag_range_traverse(int unit, bcm_vlan_dtag_range_traverse_cb cb,
                             void *user_data)
{  
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_translation))  {
        rv = _robo_vlan_dtag_range_traverse(unit, cb, user_data);
        return rv;
    }
#endif

    return rv;
}



/*
 * Function   :
 *      bcm_vlan_translate_action_traverse
 * Description   :
 *      Traverse over all translate entries and call given callback with 
 *      action structure
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      cb              (IN) Call back function
 *      user_data       (IN) User provided data to pass to a call back
 */
int 
bcm_robo_vlan_translate_action_traverse(int unit, 
                                   bcm_vlan_translate_action_traverse_cb cb, 
                                   void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action))  {
        rv = _robo_vlan_translate_action_traverse(unit, 
            cb, user_data);
        return rv;
    }
#endif

    return rv;
}

/*
 * Function :
 *      bcm_vlan_translate_action_range_traverse
 * Description :
 *   Traverses over VLAN Translation table and call provided callback 
 *   with valid entries.
 * Parameters :
 *      unit            (IN) BCM unit number
 *      cb              (IN) User callback function
 *      user_data       (IN) Pointer to user specific data
 * Return:
 *
 *      BCM_E_XXX
 */
int 
bcm_robo_vlan_translate_action_range_traverse(int unit,
    bcm_vlan_translate_action_range_traverse_cb cb, void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit) && soc_feature(unit, soc_feature_vlan_action))  {
        rv = _robo_vlan_translate_action_range_traverse(unit, 
            cb, user_data);
        return rv;
    }
#endif

    return rv;
}


/*
 * Function   :
 *      bcm_vlan_translate_egress_action_traverse
 * Description   :
 *      Traverse over all translate entries and call given callback with 
 *      action structure
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      cb              (IN) Call back function
 *      user_data       (IN) User provided data to pass to a call back
 */
int 
bcm_robo_vlan_translate_egress_action_traverse(int unit, 
                            bcm_vlan_translate_egress_action_traverse_cb cb, 
                            void *user_data)
{
    int rv = BCM_E_UNAVAIL;

    CHECK_INIT(unit);
    if (!cb) {
        return (BCM_E_PARAM);
    }

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    LOG_INFO(BSL_LS_BCM_VLAN,
             (BSL_META_U(unit,
                         "BCM_API:%s...\n"), FUNCTION_NAME()));

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        rv = _robo_vlan_translate_egress_action_traverse(unit, cb, user_data);
        return rv;
    }
#endif  /* BCM_VULCAN_SUPPORT (STARFIGHTER/POLAR/NORTHSTAR/NS+/SF3) */

    return rv;
}


/*
 * Function   :
 *      bcm_vlan_translate_egress_action_get
 * Description   :
 *      Get an entry to egress VLAN translation table.
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      port_class      (IN) Group ID of ingress port
 *      outer_vlan      (IN) Packet outer VLAN ID
 *      inner_vlan      (IN) Packet inner VLAN ID
 *      action          (OUT) Action for outer and inner tag
 */
int 
bcm_robo_vlan_translate_egress_action_get (int unit, int port_class,
                                          bcm_vlan_t outer_vlan,
                                          bcm_vlan_t inner_vlan,
                                          bcm_vlan_action_set_t *action)
{
    CHECK_INIT(unit);

    if (NULL == action) {
        return BCM_E_PARAM;
    }

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
        /* the ingress packet outer/inner VID will be ignored */
        LOG_INFO(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "BCM API : %s, port_class=%d, outer_vid=%d, inner_vid=%d\n"), 
                  FUNCTION_NAME(), port_class, outer_vlan, inner_vlan));
    
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            int     evr_id = 0; /* egress vt entry id */
            uint32  int_port = 0, int_flowid = 0;
            uint8   *list = NULL;
           
            /* This API is used to direct control bcm53115's Egress VID Remark
             *  (EVR)table only. This table is for egress VLAN tag(S-Tag and 
             *  C-Tag) remarking on a specific port per CFP generated Flow-ID
             *  (aka. class-id)
             *  EVR table entry is indexed by per port 256 entries.
             *  Here we use BCM_GPORT_TYPE_SPECIAL to indicate the specific
             *  entry index format.
             */
 
            /* parse the requesting EVR table entry index */
            if (BCM_GPORT_IS_SET(port_class) && 
                    BCM_GPORT_IS_SPECIAL(port_class)) {
                evr_id = BCM_GPORT_SPECIAL_GET(port_class);
                if (evr_id != BCM_GPORT_INVALID){
                    EVR_ENTRYID_RESOLVE(evr_id, int_port, int_flowid);
                    LOG_INFO(BSL_LS_BCM_VLAN,
                             (BSL_META_U(unit,
                                         "%s, EVT entry at id=%d(port=%d, flow=%d)\n"), 
                              FUNCTION_NAME(), evr_id, int_port, int_flowid));
                } else {
                    return BCM_E_PORT;
                }

                /* check sw database */
                list = evt_direct_set_info[unit].direct_set_list;
                if (!EVTLIST_ENTRY_CHK(list, evr_id)){
                    return BCM_E_NOT_FOUND;
                }

                /* get action */
                BCM_IF_ERROR_RETURN(_bcm_vulcan_evr_get(
                        unit, evr_id, action));

            } else {
                /* if GPORT is not at SPECIAL type, reference to original VT  
                 *  related APIs and the port parsing will be handled in the 
                 *  called VT APIs.
                 */ 
                bcm_port_t  in_port = 0;
                bcm_vlan_t  old_vid = 0, new_vid = 0;
                int         new_pri = 0;
                int         is_vt_dtag_mode = FALSE, rv = BCM_E_NONE;

                in_port = port_class;
                old_vid = outer_vlan;

                sal_memset(action, 0, sizeof(bcm_vlan_action_set_t));
                rv = bcm_robo_vlan_translate_egress_get(unit, 
                        in_port, old_vid, &new_vid, &new_pri);
                if (rv == BCM_E_NOT_FOUND){
                    rv = bcm_robo_vlan_dtag_get(unit, 
                        in_port, old_vid, &new_vid, &new_pri);
                    if (rv == BCM_E_NONE){
                        is_vt_dtag_mode = TRUE;
                    }
                }

                if (rv == BCM_E_NONE){
                    action->new_outer_vlan = new_vid;
                    action->priority = new_pri;
                    action->dt_outer = bcmVlanActionReplace;
                    action->dt_inner = (is_vt_dtag_mode == TRUE) ? 
                            bcmVlanActionNone : bcmVlanActionDelete;
                }

                return rv;
            }


        } else {
            return BCM_E_UNAVAIL;
        }
        
        return BCM_E_NONE;
#else   /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/
        return BCM_E_UNAVAIL;
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || SF3*/

}

/* Create a VLAN queue map entry. */
int
bcm_robo_vlan_queue_map_create(int unit,
                               uint32 flags,
                               int *qmid)
{
    return BCM_E_UNAVAIL;
}

/* Delete a VLAN queue map entry. */
int
bcm_robo_vlan_queue_map_destroy(int unit,
                                int qmid)
{
    return BCM_E_UNAVAIL;
}

/* Delete all VLAN queue map entries. */
int
bcm_robo_vlan_queue_map_destroy_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/* Set a VLAN queue map entry. */
int
bcm_robo_vlan_queue_map_set(int unit,
                            int qmid,
                            int pkt_pri,
                            int cfi,
                            int queue,
                            int color)
{
    return BCM_E_UNAVAIL;
}

/* Get a VLAN queue map entry. */
int
bcm_robo_vlan_queue_map_get(int unit,
                            int qmid,
                            int pkt_pri,
                            int cfi,
                            int *queue,
                            int *color)
{
    return BCM_E_UNAVAIL;
}

/* Attach a queue map object to a VLAN or VFI. */
int
bcm_robo_vlan_queue_map_attach(int unit,
                               bcm_vlan_t vlan,
                               int qmid)
{
    return BCM_E_UNAVAIL;
}

/* Get the queue map object which is attached to a VLAN or VFI. */
int
bcm_robo_vlan_queue_map_attach_get(int unit,
                                   bcm_vlan_t vlan,
                                   int *qmid)
{
    return BCM_E_UNAVAIL;
}

/* Detach a queue map object from a VLAN or VFI. */
int
bcm_robo_vlan_queue_map_detach(int unit,
                               bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

/* Detach queue map objects from all VLAN or VFI. */
int
bcm_robo_vlan_queue_map_detach_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_vlan_block_get
 * Purpose:
 *      Get per VLAN block configuration.
 * Parameters:
 *      unit    - (IN) BCM device number.
 *      vid     - (IN) VLAN to get the flood setting for.
 *      control - (OUT) VLAN control structure
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_vlan_block_get(int unit, bcm_vlan_t vid, bcm_vlan_block_t *block)
{
    int rv = BCM_E_UNAVAIL;
    uint32 arg;
    pbmp_t vlan_pbmp, vlan_ubmp, null_pbmp;
    bcm_vlan_control_vlan_t control;

    if (NULL == block) {
        return (BCM_E_PARAM);
    }

    BCM_PBMP_CLEAR(vlan_pbmp);
    BCM_PBMP_CLEAR(null_pbmp);

    if (SOC_IS_TBX(unit)) {
        CHECK_INIT(unit);
        CHECK_VID(unit, vid);

        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
        /* Get port member of the vlan */
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_port_get(unit, vid,
                               &vlan_pbmp, &vlan_ubmp));

        /* Clear unsupported block types */
        BCM_PBMP_CLEAR(block->known_multicast);
        BCM_PBMP_CLEAR(block->broadcast);
        
        /* Check if unknown mcast drop enabled. */
        BCM_IF_ERROR_RETURN(bcm_vlan_mcast_flood_get(unit, vid, 
                            &arg));
        if (arg == BCM_VLAN_MCAST_FLOOD_NONE) {
            BCM_PBMP_ASSIGN(block->unknown_multicast, vlan_pbmp);
        } else {
            BCM_PBMP_ASSIGN(block->unknown_multicast, null_pbmp);
        }

        /* Check if unknown ucast drop enabled. */
        sal_memset(&control, 0, sizeof(bcm_vlan_control_vlan_t));
        BCM_IF_ERROR_RETURN(bcm_vlan_control_vlan_get(unit, vid, 
            &control));
        if (control.flags & BCM_VLAN_UNKNOWN_UCAST_DROP) {
            BCM_PBMP_ASSIGN(block->unknown_unicast, vlan_pbmp);
        } else {
            BCM_PBMP_ASSIGN(block->unknown_unicast, null_pbmp);
        }
        return BCM_E_NONE;
    }

    return rv;
}



/*
 * Function:
 *      bcm_robo_vlan_block_set
 * Purpose:
 *      Set per VLAN configuration.
 * Parameters:
 *      unit    - (IN) BCM device number.
 *      vid     - (IN) VLAN to get the flood setting for.
 *      block   - (IN) VLAN block structure
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_vlan_block_set(int unit, bcm_vlan_t vid, bcm_vlan_block_t *block)
{
    int rv = BCM_E_UNAVAIL;
    pbmp_t vlan_pbmp, vlan_ubmp;
    bcm_vlan_control_vlan_t control;

    BCM_PBMP_CLEAR(vlan_pbmp);
    if (SOC_IS_TBX(unit)) {
        CHECK_INIT(unit);
        CHECK_VID(unit, vid);

        /* BCM53280 only support per vlan MLF and ULF drop. */    
        if (BCM_PBMP_NOT_NULL(block->known_multicast) ||
            BCM_PBMP_NOT_NULL(block->broadcast)) {
            return BCM_E_UNAVAIL;
        }

        /* Check if this VLAN is exist */
        if (!(_bcm_robo_vlist_lookup(&robo_vlan_info[unit].list, vid))) {
            return BCM_E_NOT_FOUND;
        }
    
        /* Get port member of the vlan */
        BCM_IF_ERROR_RETURN(_bcm_robo_vlan_port_get(unit, vid,
                               &vlan_pbmp, &vlan_ubmp));

        /* Configuration is only allowed when the incoming portbitmap 
         * is the same as the vlan's member.
         * Or, null port bitmap to disable.
         */
        if (BCM_PBMP_IS_NULL(block->unknown_multicast)) {
            BCM_IF_ERROR_RETURN(bcm_vlan_mcast_flood_set(unit, vid,
                                BCM_VLAN_MCAST_FLOOD_UNKNOWN));
        } else if (BCM_PBMP_EQ(block->unknown_multicast, vlan_pbmp)) {
            BCM_IF_ERROR_RETURN(bcm_vlan_mcast_flood_set(unit, vid,
                                BCM_VLAN_MCAST_FLOOD_NONE));
        } else {
            return BCM_E_UNAVAIL;
        }

        sal_memset(&control, 0, sizeof(bcm_vlan_control_vlan_t));
        BCM_IF_ERROR_RETURN(bcm_vlan_control_vlan_get(unit, 
                                vid, &control));
        if (BCM_PBMP_IS_NULL(block->unknown_unicast)) {
            control.flags &= ~BCM_VLAN_UNKNOWN_UCAST_DROP;
        } else if (BCM_PBMP_EQ(block->unknown_unicast, vlan_pbmp)) {
            control.flags |= BCM_VLAN_UNKNOWN_UCAST_DROP;
        } else {
            return BCM_E_UNAVAIL;
        }
        BCM_IF_ERROR_RETURN(bcm_vlan_control_vlan_set(unit, 
                                vid, control));

        return BCM_E_NONE;
    }
    return rv;
}

/*
 * Function:
 *      bcm_vlan_gport_add
 * Purpose:
 *      Add a Gport to the specified vlan.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      vlan - VLAN ID to add port to as a member.
 *      port - Gport ID
 *      flags - BCM_VLAN_PORT_XXX
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 *      BCM_E_NOT_FOUND - VLAN ID not in use.
 */
int
bcm_robo_vlan_gport_add(int unit, bcm_vlan_t vlan, bcm_gport_t port, 
                       int flags)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t pbmp, ubmp;
    bcm_port_t local_port;
    bcm_module_t modid;
    bcm_trunk_t tgid;
    int id;

    CHECK_INIT(unit);
    CHECK_VID(unit, vlan);

    if (flags & ~BCM_VLAN_PORT_UNTAGGED) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        local_port = port;
        if (!SOC_PORT_VALID(unit, local_port)) { 
            return BCM_E_PORT; 
        }
    }

    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_CLEAR(ubmp);
    BCM_PBMP_PORT_ADD(pbmp, local_port);
    if (flags & BCM_VLAN_PORT_UNTAGGED) {
        BCM_PBMP_PORT_ADD(ubmp, local_port);
    }
    rv = bcm_vlan_port_add(unit, vlan, pbmp, ubmp);

    return rv;
}

/*
 * Function:
 *      bcm_vlan_gport_delete
 * Purpose:
 *      Delete a Gport from the specified vlan.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      vlan - VLAN ID to add port to as a member.
 *      port - Gport ID
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 *      BCM_E_NOT_FOUND - VLAN ID not in use.
 */
int
bcm_robo_vlan_gport_delete(int unit, bcm_vlan_t vlan, bcm_gport_t port)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t pbmp;
    bcm_port_t local_port;
    bcm_module_t modid;
    bcm_trunk_t tgid;
    int id;

    CHECK_INIT(unit);
    CHECK_VID(unit, vlan);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        local_port = port;
        if (!SOC_PORT_VALID(unit, local_port)) { 
            return BCM_E_PORT; 
        }
    }

    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp, local_port);
    rv = bcm_vlan_port_remove(unit, vlan, pbmp);

    return rv;
}

/*
 * Function:
 *      bcm_vlan_gport_delete_all
 * Purpose:
 *      Delete a Gport from the specified vlan.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      vlan - VLAN ID to add port to as a member.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 *      BCM_E_NOT_FOUND - VLAN ID not in use.
 */
int
bcm_robo_vlan_gport_delete_all(int unit, bcm_vlan_t vlan)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t pbmp, ubmp;

    CHECK_INIT(unit);
    CHECK_VID(unit, vlan);

    /* Now remove all local physical ports */
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_CLEAR(ubmp);
    rv = bcm_vlan_port_get(unit, vlan, &pbmp, &ubmp);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = bcm_vlan_port_remove(unit, vlan, pbmp);

    return rv;
}

int
bcm_robo_vlan_gport_get(int unit, bcm_vlan_t vlan, bcm_gport_t port, 
                       int *flags)
{
    int rv = BCM_E_NONE;
    bcm_pbmp_t pbmp, ubmp;
    bcm_port_t local_port;
    bcm_module_t modid;
    bcm_trunk_t tgid;
    int id;

    CHECK_INIT(unit);
    CHECK_VID(unit, vlan);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &local_port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        local_port = port;
        if (!SOC_PORT_VALID(unit, local_port)) { 
            return BCM_E_PORT; 
        }
    }

    /* Deal with local physical ports */
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_CLEAR(ubmp);
    rv = bcm_vlan_port_get(unit, vlan, &pbmp, &ubmp);
    if (!BCM_PBMP_MEMBER(pbmp, local_port)) {
        return BCM_E_NOT_FOUND;
    }
    if (BCM_PBMP_MEMBER(ubmp, local_port)) {
        *flags = BCM_VLAN_PORT_UNTAGGED;
    } else {
        *flags = 0;
    }

    return rv;
}

int
bcm_robo_vlan_gport_get_all(int unit, bcm_vlan_t vlan, int array_max, 
                           bcm_gport_t *gport_array, int *flags_array, 
                           int* array_size)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_port_policer_get(int unit, bcm_vlan_t vlan, 
        bcm_port_t port, bcm_policer_t *policer_id)
{
#if defined(BCM_53101)
    CHECK_INIT(unit);
    CHECK_VID(unit, vlan);

    /* check port */
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    /* Only Lotus BCM53101 support VLAN policing */
    if (SOC_IS_LOTUS(unit)) {
        *policer_id = vlan_policer_id[unit];
        return BCM_E_NONE;
    }
#endif /* BCM_53101 */    
    return BCM_E_UNAVAIL;
}

int 
bcm_robo_vlan_port_policer_set(int unit, bcm_vlan_t vlan,
        bcm_port_t port, bcm_policer_t policer_id)
{
#if defined(BCM_53101)
    bcm_pbmp_t bmp;
    bcm_policer_config_t pol_cfg;
    vlan_entry_t vt;
    uint32 field_value;

    CHECK_INIT(unit);
    CHECK_VID(unit, vlan);

    /* check port */
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    BCM_PBMP_CLEAR(bmp);

    /* Add port */
    BCM_PBMP_PORT_ADD(bmp, port);
    
    /* Only Lotus BCM53101 support VLAN policing */
    if (SOC_IS_LOTUS(unit)) {
        if (policer_id == 0) {
            /* Disbale VLAN policing */
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                DRV_VLAN_PROP_POLICING, bmp, 0));
            
            /* Disable the VLAN policer on VLAN entries */
            sal_memset(&vt, 0, sizeof (vt));
            BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vlan, 1, (uint32 *)&vt));
            field_value = 0;
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_EN,
                        (uint32 *)&vt, &field_value));
            BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN,
                        (uint32)vlan, 1, (uint32 *)&vt));

            /* Use Bucket 0 for VLAN policing */
            BCM_IF_ERROR_RETURN(
                bcm_port_rate_ingress_set(unit, port, 0, 0));
            
        } else {
            /* Get policer configuration */
            BCM_IF_ERROR_RETURN(
                bcm_policer_get(unit, policer_id, &pol_cfg));

            /* Configure the VLAN entry */
            sal_memset(&vt, 0, sizeof (vt));
            BCM_IF_ERROR_RETURN(DRV_MEM_READ
                        (unit, DRV_MEM_VLAN,
                        (uint32)vlan, 1, (uint32 *)&vt));
            field_value = 1;
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_EN,
                        (uint32 *)&vt, &field_value));
            field_value = 0;
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_ID,
                        (uint32 *)&vt, &field_value));
            BCM_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLAN,
                        (uint32)vlan, 1, (uint32 *)&vt));

            /* Configure burst and rate */
            BCM_IF_ERROR_RETURN(
                bcm_port_rate_ingress_set(unit, port, 
                    pol_cfg.ckbits_sec, pol_cfg.ckbits_burst));

            /* Enable VLAN policing */
            BCM_IF_ERROR_RETURN(
                DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                DRV_VLAN_PROP_POLICING, bmp, 1));
            
        }
        vlan_policer_id[unit] = policer_id;
        return BCM_E_NONE;
    }
#endif /* BCM_53101 */    
    return BCM_E_UNAVAIL;
}


/*
 * For BCM internal use to set vlan create default flood mode.
 * Follow the same locking convention as other members in the structure
 */
/*
 * Set the default PFM to be used by vlan_create
 * Called from bcm_switch_control_set
 */
int
_bcm_robo_vlan_flood_default_set(int unit, bcm_vlan_mcast_flood_t mode)
{
    bcm_robo_vlan_info_t             *vi = &robo_vlan_info[unit];

    if (mode >= BCM_VLAN_MCAST_FLOOD_COUNT) {
        return BCM_E_PARAM;
    }

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (mode == BCM_VLAN_MCAST_FLOOD_ALL) {
            return BCM_E_UNAVAIL;
        }    
    }
#endif

    vi->flood_mode = mode;

    return BCM_E_NONE;
}

/*
 * Get the default PFM used by vlan_create
 * Called from bcm_switch_control_get, bcm_vlan_create
 */
int
_bcm_robo_vlan_flood_default_get(int unit, bcm_vlan_mcast_flood_t *mode)
{
    bcm_robo_vlan_info_t             *vi = &robo_vlan_info[unit];

    *mode = vi->flood_mode;

    return BCM_E_NONE;
}


