/*
 * $Id: mcast.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _BCM_INT_MCAST_H
#define _BCM_INT_MCAST_H

#include <bcm_int/robo/subport.h>

/* extended BCM_MCAST_XXX flag for ROBO usage only */
#define _BCM_ROBO_MCAST_FORCE_L2MC_ID    0x0001000

/* Definitions for VLAN_PORT_ID :
 *  - This id is used to be construct vlanport_id to indicate the target VLAN 
 *      information for Multicast VLAN repliaction.
 *  - VLAN information include : Untagged status and VLAN-ID
 *
 * Formate : UNTAG(1 bit on bit12) | VID(12 bits on bit11~bit0) 
 *
 * Note :  
 *  1. VLAN_PORT_ID is used for bcm_multicast_vlan_encap_get()
 */
#define _BCM_VLAN_PORT_VID_MASK      0xFFF
#define _BCM_VLAN_PORT_UNTAG_MASK    0x1
#define _BCM_VLAN_PORT_VID_SHIFT     0
#define _BCM_VLAN_PORT_UNTAG_SHIFT   12

#define _BCM_VLAN_PORT_VLANPORTID_MASK  \
        (((_BCM_VLAN_PORT_VID_MASK) << (_BCM_VLAN_PORT_VID_SHIFT)) | \
        ((_BCM_VLAN_PORT_UNTAG_MASK) << (_BCM_VLAN_PORT_UNTAG_SHIFT)))
#define _BCM_VLAN_PORT_VID_SET(id, ut, vid) ((id) = \
        (((((ut) & _BCM_VLAN_PORT_UNTAG_MASK) << _BCM_VLAN_PORT_UNTAG_SHIFT) | \
        (((vid) & _BCM_VLAN_PORT_VID_MASK) << _BCM_VLAN_PORT_VID_SHIFT)) & \
        _BCM_VLAN_PORT_VLANPORTID_MASK))
#define _BCM_VLAN_PORT_VID_GET(id) \
        ((((id) & (_BCM_VLAN_PORT_VLANPORTID_MASK)) >> \
        (_BCM_VLAN_PORT_VID_SHIFT)) & (_BCM_VLAN_PORT_VID_MASK))
#define _BCM_VLAN_PORT_UNTAG_GET(id) \
        ((((id) & (_BCM_VLAN_PORT_VLANPORTID_MASK)) >> \
        (_BCM_VLAN_PORT_UNTAG_SHIFT)) & (_BCM_VLAN_PORT_UNTAG_MASK))

/* Generial VLAN encap_id format for ROBO solution:
 *
 *  == for vlanport type ==
 *  1. PPort ID : Position = bit0, MASK = 0x1F
 *  2. VLAN ID : Position = bit5, MASK = 0xFFF
 *  3. UT_Flag : Position = bit17, MASK = 0x1
 */
#define _BCM_MULTICAST_ENCAP_PPORT_MASK     0x1F    /* Phisical Port */
#define _BCM_MULTICAST_ENCAP_VLANID_MASK    0xFFF
#define _BCM_MULTICAST_ENCAP_FLAGS_MASK     0x1    /* for flexible usage */
#define _BCM_MULTICAST_ENCAP_PPORT_SHIFT    0
#define _BCM_MULTICAST_ENCAP_VLANID_SHIFT   5
#define _BCM_MULTICAST_ENCAP_FLAGS_SHIFT    17

#define _BCM_MULTICAST_ENCAP_FLAGS_UNTAG   (1 << 0)

#define  _BCM_MULTICAST_ENCAP_VLANPORT_SET(pp, vid, flags) \
            ((((pp) & (_BCM_MULTICAST_ENCAP_PPORT_MASK)) <<  \
                    (_BCM_MULTICAST_ENCAP_PPORT_SHIFT)) | \
            (((vid) & (_BCM_MULTICAST_ENCAP_VLANID_MASK)) <<  \
                    (_BCM_MULTICAST_ENCAP_VLANID_SHIFT)) | \
            (((flags) & (_BCM_MULTICAST_ENCAP_FLAGS_MASK)) <<  \
                    (_BCM_MULTICAST_ENCAP_FLAGS_SHIFT)))

#define  _BCM_MULTICAST_ENCAP_PPORT_GET(id)  \
            (((id) >> (_BCM_MULTICAST_ENCAP_PPORT_SHIFT)) & \
            (_BCM_MULTICAST_ENCAP_PPORT_MASK))
#define  _BCM_MULTICAST_ENCAP_VLAN_GET(id)  \
            (((id) >> (_BCM_MULTICAST_ENCAP_VLANID_SHIFT)) & \
            (_BCM_MULTICAST_ENCAP_VLANID_MASK))
#define  _BCM_MULTICAST_ENCAP_FLAGS_IS_UNTAG(id)  \
            ((((id) >> (_BCM_MULTICAST_ENCAP_FLAGS_SHIFT)) & \
            (_BCM_MULTICAST_ENCAP_FLAGS_MASK)) & \
            _BCM_MULTICAST_ENCAP_FLAGS_UNTAG)


#ifdef  BCM_TB_SUPPORT

#define _TB_MCAST_REPL_INDEX_MGID_OFFSET 0
#define _TB_MCAST_REPL_INDEX_PORT_OFFSET 8

#define _TB_MCAST_REPL_VID_VPORT_OFFSET 0
#define _TB_MCAST_REPL_VID_PORT_OFFSET 4

#define _MAX_REPL_CFG_INDEX_NUM \
    (_TB_SUBPORT_NUM_MCASTREP_GROUP * _TB_SUBPORT_NUM_PORT)
#define _MAX_REPL_VID_INDEX_NUM \
    (_TB_SUBPORT_NUM_VPORT_PER_GROUP * _TB_SUBPORT_NUM_PORT)

/* Replication list of (mc_index, port) pairs. */
typedef struct {
    SHR_BITDCL  repl_vport_id[_SHR_BITDCLSIZE(_TB_SUBPORT_NUM_VPORT_PER_GROUP)]; 
} _bcm_robo_mcast_repl_vport_id_t;

typedef struct {
    int mcast_repl_enable;
    _bcm_robo_mcast_repl_vport_id_t  repl_index[_MAX_REPL_CFG_INDEX_NUM]; 
} _bcm_robo_mcast_repl_t;

/* Replicated vlan(s) */
typedef struct {
    bcm_vlan_t vid;
    uint16 count;
} _bcm_robo_mcast_repl_vid_t;


typedef struct {
    _bcm_robo_mcast_repl_vid_t  repl_vid[_MAX_REPL_VID_INDEX_NUM]; 
} _bcm_robo_mcast_repl_vlan_t;

#endif  /* BCM_TB_SUPPORT */

extern int _bcm_robo_l2mc_id_alloc(int unit, int l2mc_id);
extern int _bcm_robo_l2mc_id_free(int unit, int l2mc_id);
extern int _bcm_robo_l2mc_id_check(int unit, int l2mc_id);
extern int _bcm_robo_l2mc_free_index(int unit, int type, int *l2mc_id);
extern int _bcm_robo_mcast_detach(int unit);

#endif  /* _BCM_INT_MCAST_H */
