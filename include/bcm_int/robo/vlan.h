/*
 * $Id: vlan.h,v 1.29 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _BCM_INT_VLAN_H
#define _BCM_INT_VLAN_H

#include <sal/types.h>

#include <bcm/field.h>

/* QVLAN_UTBMP_BACKUP :
 *  - for keeping correct untag bitmp when turn on/off device double tagging mode.
 *  - ROBO chips (bcm5347/5348/53242) use 1Q_VLAN untag bitmap to apply to the 
 *      NNI/UNI port setting about untagging SP_TAG action.
 *      
 */
#define QVLAN_UTBMP_BACKUP      0

/*
 * Define:
 *	_ROBO_VLAN_CHK_ID
 * Purpose:
 *	Causes a routine to return BCM_E_PARAM if the specified
 *	VLAN ID is out of range.
 */

#define _ROBO_VLAN_CHK_ID(unit, vid) do { \
	if (vid > BCM_VLAN_MAX) return BCM_E_PARAM; \
	} while (0);

/*
 * Define:
 *	_ROBO_VLAN_CHK_PRIO
 * Purpose:
 *	Causes a routine to return BCM_E_PARAM if the specified
 *	priority (802.1p CoS) is out of range.
 */

#define _ROBO_VLAN_CHK_PRIO(unit, prio) do { \
	if ((prio < BCM_PRIO_MIN || prio > BCM_PRIO_MAX)) return BCM_E_PARAM; \
	} while (0);

#define _ROBO_VLAN_CHK_ACTION(unit, action) do { \
	if (action < bcmVlanActionNone || action > bcmVlanActionDelete) return BCM_E_PARAM; \
	} while (0);

/*
 * VLAN internal struct used for book keeping
 */
typedef struct vlist_s {
    bcm_vlan_t          vid;

    bcm_pbmp_t pbmp;
    bcm_pbmp_t utbmp;

#if QVLAN_UTBMP_BACKUP
    /* ReadMe : ubmp 
     *   - This value should be maintained in realtime VLAN configuration
     *   - When Double tagging mode is enabled, this port bitmap might be 
     *      quite different with the untag_bitmap field in 1Q_VLAN table.
     *      The port's NNI/UNI setting causes such differnece.
     *   - If the double tagging mode is disabled again, the untag-bitmap in 
     *      all valid VLAN should re-programmed by this value(ubmp).
     *   - Each time to set any port's NNI/UNI mode will causes the untag 
     *      bitmap in all valid VLAN entries been reprogrammed. 
     */
    bcm_pbmp_t ubmp;    /* for the reference by port_dtag_mode setting */
#endif

#ifdef BCM_TB_SUPPORT
    int cross_connected;
#endif

    struct vlist_s      *next;
} vlist_t;

typedef struct bcm_vlan_prio_info_s {
    int     enable;             /* ??? */
    int     flag;               /* New re-mapping or Default mapping*/
    uint32  default_priority;   /* Default priority mapping */
    uint32  old_priority;       /* Previous priority mapping */
    uint32  new_priority;       /* New priority mapping */
} bcm_vlan_prio_info_t[BCM_LOCAL_UNITS_MAX];

#if QVLAN_UTBMP_BACKUP
    extern int _bcm_robo_vlan_utbmp_dt_rebuild(int unit, int mode);
#endif


#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
/* CFP may be used for supporting VLAN translation 
 *  - bcm53115 use CFP to implement the VLAN translation feature on 
 *      Mapping/Transparent mode.
 */
extern int flag_vt_cfp_init;  /* flag for the init status */ 

/* init check on VLAN translation CFP entries */
#define IS_VT_CFP_INIT          (flag_vt_cfp_init == TRUE)

/* VTCFP_SUPPORT_VLAN_CNT is a predefined number to represent the number 
 * of the VLAN on supporting VLAN translation.
 *  - the rule for this value is : two times of supported VT entry count.
 *    (i.e. 2 * 64 = 128)
 *  - The reason of the rule in above : every bcm53115 VT entry need two 
 *      CFP entries to serve. One is for NNI and the other is for UNI.
 */
#define VTCFP_SUPPORT_VLAN_CNT    128

typedef struct drv_vt_cfp_entry_db_s {
    bcm_vlan_t  vid;
    int     vt_mode;        /* 0: transparent; 1: mapping */
    int     nni_field_id;   /* field entry id for NNI */
    int     uni_field_id;   /* field entry id for UNI */
    int     nni_stat_id;   /* field stat id for NNI */
    int     uni_stat_id;   /* field stat id for UNI */
    
    int     prev;       /* init as -1 */
    int     next;       /* init as -1 */
}drv_vt_cfp_entry_db_t;

typedef struct drv_vt_cfp_db_s {
    int     vt_cfp_db_valid_count;
    int     vt_cfp_db_start_id;     /* init as -1 */
    drv_vt_cfp_entry_db_t     vt_cfp_entry_db[VTCFP_SUPPORT_VLAN_CNT];
}drv_vt_cfp_db_t;

#define VTCFP_NULL_INDEX    -1

#define IS_VALID_VTCFP_DB_ENTRY_ID(_id)    \
                ((_id) >= 0 && (_id) < VTCFP_SUPPORT_VLAN_CNT)
#define IS_VTCFP_DB_FULL    \
            (vt_cfp_db.vt_cfp_db_valid_count >= VTCFP_SUPPORT_VLAN_CNT)

#define VTCFP_SWDB_OP_FLAG_INSERT   1
#define VTCFP_SWDB_OP_FLAG_DELETE   2
#define VTCFP_SWDB_OP_FLAG_RESET    3

/* global variable for keeping the cfp information for iDT_Mode */
typedef struct drv_idt_mode_cfp_info_s {
    /* cfp for untag action at none-isp port */
    int     none_isp_cfp_group_id;  /* field group id */
    int     none_isp_filter_id;     /* field entry id */
    int     none_isp_evr_id;        /* EVR table entry id */
    
    /* ---- cfp for vlan translation ---- */
    int     vt_cfp_group_id;    /* field group id */
    
    /* add more below if any cfp related group id have to create for APIs */
    
}drv_idt_mode_cfp_info_t;

extern drv_idt_mode_cfp_info_t  idt_mode_cfp_info;
/* macro define to get the pre-created CFP group id for egress vlan 
 *  translation usage.
 */
#define FIELD_GPID_FOR_VT   (idt_mode_cfp_info.vt_cfp_group_id)

extern int _bcm_robo_vlan_vtcfp_init(int unit);
extern int _bcm_robo_vlan_vtcfp_isp_change(int unit);
extern int _bcm_robo_vlan_vtevr_isp_change(int unit, pbmp_t changed_pbm);

#else  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || 
        * BCM_STARFIGHTER3_SUPPORT
        */

#define IS_VT_CFP_INIT  0
#endif /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR || NS+ || 
        * BCM_STARFIGHTER3_SUPPORT 
        */


#ifdef BCM_TB_SUPPORT

#define VM_PORT_MASK      0x3f
#define VM_VPORT_MASK     0xf
#define VM_VID_MASK       0xfff
#define VM_VID_RANGE_MASK 0xf
#define VM_FID_MASK       0xfff
#define VM_PCP_MASK       0x7
#define VM_FRAME_MASK     0x3
#define VM_ETHER_MASK     0xffff

#define VLAN_ACTION_DT 0x1
#define VLAN_ACTION_OT 0x2
#define VLAN_ACTION_IT 0x4
#define VLAN_ACTION_UT 0x8

#define BCM_ROBO_VM_IVM_GROUP 0
#define BCM_ROBO_VM_EVM_GROUP 1

/* Vlan Translation API types */
#define VT_API_ANY                     0x0 /* Don't care called by which API. */
#define VT_API_VLAN_XLATE_ACTION       0x1 /* bcm_vlan_translate_action_xxx() */
#define VT_API_VLAN_XLATE              0x2 /* bcm_vlan_translate_xxx() */
#define VT_API_VLAN_DTAG               0x4 /* bcm_vlan_dtag_xxx() */
#define VT_API_VLAN_XLATE_ACTION_RANGE 0x8 /* bcm_vlan_translate_action_range_xxx() */
#define VT_API_VLAN_XLATE_RANGE        0x10 /* bcm_vlan_translate_range_xxx() */
#define VT_API_VLAN_DTAG_RANGE         0x20 /* bcm_vlan_dtag_range_xxx() */
#define VT_API_VLAN_PORT_DEFAULT       0x40 /* bcm_vlan_port_default_action_xxx() */
#define VT_API_VLAN_PROTOCOL       0x80 /* bcm_robo_vlan_port_protocol_action_xxx() */
#define VT_API_VLAN_RECOVER       0x80000000 /* Specific flag for fail recovery */

typedef struct _robo_vlan_translate_action_param_s {
    bcm_gport_t port;
    bcm_vlan_translate_key_t key_type;
    /* 
      * For Ranger APIs, 
      * param1 = low vid. 
      * param2 = high vid.
      * For Protocol2Vlan APIs,
      * param1 = frame type. 
      * param2 = ether type.
      * For other APIs,
      * param1 = outer vid. 
      * param2 = inner vid.
      */
    uint32 param1; 
    uint32 param2; 
    uint32 vt_api_type; 
    int unit; 

    bcm_pbmp_t vlan_pbmp;
    bcm_pbmp_t vlan_upbmp;
} _robo_vlan_translate_action_param_t;

#define TB_EVM_UNTAG_H 1

#if TB_EVM_UNTAG_H
typedef struct _robo_vlan_evm_list_s {
    bcm_field_entry_t entry_id;
    struct _robo_vlan_evm_list_s *next;
} _robo_vlan_evm_list_t;
#endif

typedef struct _robo_vlan_translate_action_s {
    bcm_gport_t port;
    bcm_vlan_translate_key_t key_type;
    /* 
      * For Ranger APIs, 
      * param1 = low vid. 
      * param2 = high vid.
      * For Protocol2Vlan APIs,
      * param1 = frame type. 
      * param2 = ether type.
      * For other APIs,
      * param1 = outer vid. 
      * param2 = inner vid.
      */
    uint32 param1; 
    uint32 param2; 

    bcm_vlan_action_set_t action;
    uint32 vm_entry_cnt; /* number of vm entries created for the action */
    bcm_field_entry_t ivm_entry_id[9]; /* Array to store created IVM entry indices */
#if TB_EVM_UNTAG_H
    _robo_vlan_evm_list_t *evm_id_list[9];
#else
    bcm_field_entry_t evm_entry_id[9]; /* Array to store created EVM entry indices */
#endif
    uint32 flow_id[9]; /* Array to store used flow ids */
    uint32 ranger_id; /* Ranger id used by this structure */

    uint32 vt_api_type; /* The structure is created by which Vlan Translate API. */
    int unit; /* Unit number */
    struct _robo_vlan_translate_action_s *next;

    bcm_pbmp_t vlan_pbmp; /* outer_vlan members when the vt act created */
    bcm_pbmp_t vlan_upbmp; /* outer_vlan members when the vt act created */
} _robo_vlan_translate_action_t;

typedef struct _robo_vlan_translate_traverse_s {
    bcm_gport_t gport;
    bcm_vlan_t old_vlan;
    bcm_vlan_t new_vlan;
    int prio;

    int unit; /* Unit number */
    struct _robo_vlan_translate_traverse_s *next;
} _robo_vlan_translate_traverse_t;

typedef struct _robo_vlan_dtag_traverse_s {
    bcm_gport_t gport;
    bcm_vlan_t inner_vlan;
    bcm_vlan_t outer_vlan;
    int prio;

    int unit; /* Unit number */
    struct _robo_vlan_dtag_traverse_s *next;
} _robo_vlan_dtag_traverse_t;

typedef struct _robo_vlan_translate_action_range_traverse_s {
    bcm_gport_t port;
    bcm_vlan_t inner_vlan_low;
    bcm_vlan_t inner_vlan_high;
    bcm_vlan_action_set_t action;

    int unit; /* Unit number */
    struct _robo_vlan_translate_action_range_traverse_s *next;
} _robo_vlan_translate_action_range_traverse_t;

typedef struct _robo_vlan_translate_range_traverse_s {
    bcm_gport_t gport;
    bcm_vlan_t old_vlan_low;
    bcm_vlan_t old_vlan_high;
    bcm_vlan_t new_vlan;
    int prio;

    int unit; /* Unit number */
    struct _robo_vlan_translate_range_traverse_s *next;
} _robo_vlan_translate_range_traverse_t;

typedef struct _robo_vlan_dtag_range_traverse_s {
    bcm_gport_t gport;
    bcm_vlan_t old_vlan_low;
    bcm_vlan_t old_vlan_high;
    bcm_vlan_t new_vlan;
    int prio;

    int unit; /* Unit number */
    struct _robo_vlan_dtag_range_traverse_s *next;
} _robo_vlan_dtag_range_traverse_t;

typedef struct _robo_vlan_port_egress_default_action_s {
    bcm_port_t port;

    bcm_vlan_action_set_t action;
    bcm_field_entry_t evm_entry_id; /* Created EVM entry index */

    int unit; /* Unit number */
    struct _robo_vlan_port_egress_default_action_s *next;
} _robo_vlan_port_egress_default_action_t;

extern int _bcm_robo_vlan_vm_group_create(int unit, int type, 
                            bcm_field_group_t *group_id);
extern int _bcm_robo_vlan_cpu_init(int unit);
extern int _bcm_robo_vlan_cpu_deinit(int unit);
#endif

extern int _bcm_robo_vlan_flood_default_set(int unit, bcm_vlan_mcast_flood_t mode);
extern int _bcm_robo_vlan_flood_default_get(int unit, bcm_vlan_mcast_flood_t *mode);
extern int  bcm_robo_vlan_detach(int unit);
extern int _bcm_robo_vlan_mstp_config_set(int unit, bcm_vlan_t vid, bcm_stg_t stg);
#endif
