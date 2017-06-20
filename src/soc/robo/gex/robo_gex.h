/*
 * $Id: robo_gex.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_GEX_H
#define _ROBO_GEX_H

#define DOT1P_PRI_MASK  0x7
#define DOT1P_CFI_MASK  0x1
#define DOT1P_PRI_SHIFT 13
#define DOT1P_CFI_SHIFT 12

/* ------ GEX Ingress/Egress Rate Control related definition ------- */
#define GEX_IRC_PKT_MASK_IS_7F  0x7f
#define GEX_IRC_PKT_MASK_IS_3F  0x3f

#define GEX_ENABLE_DUAL_IMP_PORT  0x3

/* Rate for IMP0 and IMP1 in terms of Packets Per Second (PPS) */
#define GEX_RATE_INDEX_VALUE_SIZE  64

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
/* kbps */
#define POLAR_RATE_REFRESH_GRANULARITY  64

/* kbps */
#define POLAR_RATE_MAX_REFRESH_RATE  (1000 * 1000)

/* byte */
#define POLAR_RATE_BUCKET_UNIT_SIZE  64

/* bucket unit */
#define POLAR_RATE_MAX_BUCKET_UNIT  0x3FFFF

/* max burst size (bytes)  */
#define POLAR_RATE_MAX_BUCKET_SIZE \
    (POLAR_RATE_MAX_BUCKET_UNIT * POLAR_RATE_BUCKET_UNIT_SIZE)
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

/* ------ GEX Storm Control related definition ------- */
#define GEX_STORM_CONTROL_PKT_MASK (DRV_STORM_CONTROL_BCAST | \
                                    DRV_STORM_CONTROL_MCAST | \
                                    DRV_STORM_CONTROL_DLF | \
                                    DRV_STORM_CONTROL_SALF | \
                                    DRV_STORM_CONTROL_RSV_MCAST)

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
/* Egress VLAN XLATE and Intelligent-Double-Tagging support */
#define GEX_EVT_IDT_SUPPORT
#endif /* VULCAN || STARFIGHTER || POLAR || NORTHSTAR || NORTHSTARPLUS || SF3*/

#ifdef GEX_EVT_IDT_SUPPORT
 
/* double tagging mode, bitmap function indicator */
#define DRV_VLAN_DT_MODE_MASK           0x3
#define DRV_VLAN_DT_MODE_DISABLE        0x0
#define DRV_VLAN_NORMAL_DT_MODE         0x1
#define DRV_VLAN_INTELLIGENT_DT_MODE    0x2

/* Preferred Double Tagging Mode : 
 *  - there are two double tagging modes in Vulcan, i.e. DT_mode/iDT_mode.
 *  - Original design is preferred at DT_mode. Such design is for the idea 
 *    on concerning the most compatible DT feature with old DT mode (like 
 *    falson). In this old mode, user have to handling the VLAN.untagbitmap 
 *    by themself for the SP-Tag untagging. 
 *  - When the preferred iDT_mode is set, all the SP-TAg untaging behavior  
 *    will be take over by default EVR entry(id=0), no VLAN.untagbitmap is 
 *    requirred. No matter packet is comming from normal ingress or CPU egress
 *    direct.
 *  - In Vulcan, on ingress side, both ISP and None-ISP port can recoganize 
 *    all tagging types of packet.
 *    (i.e. Untagged/Single SP_Tagged/Single C-Tagged/Double Tagged)
 *  - The rules about SP-Tag untagging behavior for egress are :
 *      1. Untag-In, Egress to ISP              : SingleOuterTag-Out
 *      2. Untag-IN, Egress to None-ISP         : Untag-Out
 *      3. SingleOuter-In, Egress to ISP        : SingleOuterTag-Out
 *      4. SingleOuter-In, Egress to None-ISP   : Untag-Out
 *      5. SingleInner-In, Egress to ISP        : DoubleTag-Out
 *      6. SingleInner-In, Egress to None-ISP   : SingleInner-Out
 *      7. DoubleTag-In, Egress to ISP          : DoubleTag-Out
 *      8. DoubleTag-In, Egress to None-ISP     : SingleInner-Out
 *
 *  [Valid Value] : 
 *      1. DRV_VLAN_FALCON_DT_MODE      : DT_Mode
 *      2. DRV_VLAN_INTELLIGENT_DT_MODE : iDT_Mode
 */
#define DRV_VLAN_PREFER_DT_MODE     DRV_VLAN_INTELLIGENT_DT_MODE

/* defined for Vulcan specific egress basis table : 
 *  - the table access adddress is defined by SW per device designed Spec.
 *      that this table is port basis table.
 *      1. 256 entries per port. (port0~port5 and IMP )
 *      2. table index keys are : egress_port_id + classification_id(from CFP)
 *  - Designed entry index format : 0x00000pdd.
 *      1. p : is egress port id. (0 ~ 6 is valid)
 *      2. dd : is entry id (0 ~ 255 is valid)
 */
#define DRV_EGRESS_V2V_NUM_PER_PORT     256
#define DRV_EGR_V2V_IMP_PORT_SEARCH_ID  8 
#define DRV_EGR_V2V_IMP_PORT_ADDR_ID \
            ((SOC_IS_POLAR(unit) || \
              SOC_IS_NORTHSTAR(unit) || \
              SOC_IS_NORTHSTARPLUS(unit)) ? 7 : 6)   /* original bit id is "8" */
#define DRV_EGR_V2V_PORT_ADDR_OFFSET    8  /* bit offset in address */
#define DRV_EGR_V2V_ENTRY_ADDR_OFFSET   0   /* bit offset in address */
#define DRV_EGR_V2V_PORT_ADDR_MASK      0x0000FF00 /* port_id mask in addr */
#define DRV_EGR_V2V_ENTRY_ADDR_MASK     0x000000FF /*entry_id mask in addr*/
/* macro to build the EGR_V2V_Address */
#define DRV_EGR_V2V_ENTRY_ID_BUILD(_p, _id) \
            ((((((_p) == CMIC_PORT(unit))?DRV_EGR_V2V_IMP_PORT_ADDR_ID:(_p)) \
                    << DRV_EGR_V2V_PORT_ADDR_OFFSET) & \
                        DRV_EGR_V2V_PORT_ADDR_MASK) | \
            (((_id) << DRV_EGR_V2V_ENTRY_ADDR_OFFSET) & \
                        DRV_EGR_V2V_ENTRY_ADDR_MASK))

/* pre-defined Egress VLAN Remark(EVR) table index for defined action. */
/* EVR_ID : 
 *  0 : default EVR entry for handling isp/none-isp outer tag action.
 *  1~65 : for vlan translation (transparent/mapping mode) usage.
 *  66~255 : customer used.
 *
 * SW designed to support VT up to 64 VT entryies :
 *  - Current SW designed as system basis VT.
 *  - One VT entry on Vulcan need 2 CFP entries to serve on driving CalssId
 *      one is for ISP and the other is for none-ISP.
 */
/* EVR entry ID for the default outer/inner tag action */
#define DRV_EVRID_TAG_ACTION_DEF    0
#define DRV_EVRID_VT_ACTION_FISRT   1   /* the first ID for VT */
#define DRV_EVRID_VT_SW_MAX_CNT     64  /* SW max of supporting VT entry */
#define DRV_EVRID_VT_ACTION_LAST    \
            (DRV_EVRID_VT_ACTION_FISRT + DRV_EVRID_VT_SW_MAX_CNT - 1)

#define VT_SPVID_MASK       0xfff
#define VT_OPMODE_MASK      0x1

/* Vulcan's Double Tagging and new Egress VLAN translation designing  :
 *  1. Tow double tagging modes. (DT_Mode and iDT_Mode)
 *      - in DT_Mode, the VLAN table should be in charge of the mission to 
 *          untag the outer when egress to none-ISP port.
 *      - in iDT_Mode, the untag job when egress to the none-ISP port should 
 *          be programmed in Engress VLAN Remark(EVR) table.
 *  2. both ISP and None-ISP ports can recogenize the S-Tag in the ingress 
 *      side when the devivce at double tagging mode.
 *  3. EVR table can only be work when device at iDT_Mode.
 *  4. No Ingress basis VLAN translation feature in Vulcan.
 *  5. Egress basis VLAN translation is designed as :
 *      - Mapping mode : only outer tag in egress frame.
 *      - Transparent mode : both outer and inner tags in egress frame.
 *  6. Only CFP action can drive a legal clssification-id to indicate a proper 
 *      EVR table entry. (means EVR table can't be work without CFP setting.)
 */

/* ------- egress port basis VLAN translation DataBase ---------- */
/* drv_vlan_egr_vt_info_t :
 *  - a sw table to represent the user configured EVR table (binding with CFP 
 *      table). And this sw table will be formed as port basis structure with 
 *      ori_vid sorted on each specific egress port.
 */
typedef struct drv_vlan_egr_vt_info_s {
    uint16  ori_vid;            /* original VID(entry key) */
    uint16  new_vid;            /* new VID */
    int     vt_mode;            /* transparent/mapping mode */
    int     prev_id;            /* pointer to previous index(init=-1) */
    int     next_id;            /* pointer to next index(init=-1) */
    /* prev_id & next_id :
     *  1. head node >> prev = -1
     *      a. if node_count == 1, next = DRV_EGRESS_V2V_NUM_PER_PORT
     *      b. if node_count > 1, next = n(the next node id)
     *  2. last node >> next = DRV_EGRESS_V2V_NUM_PER_PORT
     *      a. if node_count == 1, prev = -1
     *      b. if node_count > 1, prev = n(the previous node id)
     *  3. other node, prev = 0~DRV_EVRID_VT_SW_MAX_CNT, and
     *                  next = 0~DRV_EVRID_VT_SW_MAX_CNT
     */
    
    /* this is for port based implementation, not used for global basis */
    int     evr_id;     /* Egress VLAN Remark table index(0 is invalid)*/
    
} drv_vlan_egr_vt_info_t;

typedef struct drv_egr_vt_db_info_s {
    int     start_id;   /* log the first valid id (init = -1)*/
    int     isp_port;   /* 1 is isp port, 0 is non-isp port (init= -1) */
    int     count;      /* log the valid count in DB entry (init = 0)*/
    drv_vlan_egr_vt_info_t    egr_vt_info[DRV_EVRID_VT_SW_MAX_CNT];
}drv_egr_vt_db_info_t;

#define IS_VALID_EGRVT_DB_ENTRY_ID(_id)    \
                ((_id) >= 0 && (_id) < DRV_EVRID_VT_SW_MAX_CNT)
#define EGRVT_DB_ID_TO_REAL_ID(_db_id)    \
                ((_db_id) + DRV_EVRID_VT_ACTION_FISRT)
#define EGRVT_REAL_ID_TO_DB_ID(_real_id)    \
                ((_real_id) - DRV_EVRID_VT_ACTION_FISRT)
                
#define IS_PORT_EGRVT_EMPTY(_p)    \
                (egr_vt_db[(_p)].count == 0)
#define IS_PORT_EGRVT_FULL(_p)    \
                (egr_vt_db[(_p)].count == DRV_EVRID_VT_SW_MAX_CNT)

#define EVR_SWDB_OP_FLAG_INSERT  1
#define EVR_SWDB_OP_FLAG_DELETE  2
#define EVR_SWDB_OP_FLAG_RESET   3

#define IS_DRV_EVR_DB_NODE_FREE(_p, _i)     \
            ((((egr_vt_db[(_p)].egr_vt_info) + (_i)).prev_id == -1) && \
            (((egr_vt_db[(_p)].egr_vt_info) + (_i)).next_id == -1))
#define IS_DRV_EVR_DB_FULL     \
            ((egr_vt_db[(_p)].count) == DRV_EVRID_VT_SW_MAX_CNT)
        
#define DRV_EVR_VID_MASK        0xFFF

#define DRV_EVR_OP_FLAG_MASK    0x0F
#define DRV_EVR_OP_FLAG_ASIS    0x01
#define DRV_EVR_OP_FLAG_ASRX    0x02
#define DRV_EVR_OP_FLAG_REMOVE  0x04
#define DRV_EVR_OP_FLAG_MODIFY  0x08

#define IS_EVR_OP_VALID(_op)    \
            (((_op) & DRV_EVR_OP_FLAG_MASK) > 0)
#define EVT_OP_FLAG_TO_OP_VALUE(_op)    \
            ((((_op) == DRV_EVR_OP_FLAG_ASIS) ? 0 : (   \
                (((_op) == DRV_EVR_OP_FLAG_ASRX) ? 1 : (    \
                (((_op) == DRV_EVR_OP_FLAG_REMOVE) ? 2 : 3))))))

#else   /* GEX_EVT_IDT_SUPPORT */
#define DRV_VLAN_DT_MODE_DISABLE    0x0
#define DRV_VLAN_NORMAL_DT_MODE     0x1
#define DRV_VLAN_PREFER_DT_MODE     DRV_VLAN_NORMAL_DT_MODE
#endif  /* GEX_EVT_IDT_SUPPORT */

#endif  /* _ROBO_GEX_H */

