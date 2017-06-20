/*
 * $Id: subport.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_SUBPORT_H
#define _BCM_INT_SUBPORT_H

#include <sal/types.h>

#ifdef  BCM_TB_SUPPORT

/* Subport in ROBO is for Multicast replication and the term internally is 
 *  Virtual Port, i.e. VPort. 
 *
 *    In TBX , there are 16 VPorts been bounded in a physical port(PPort),
 *  each VPort of a PPort can be set as active or inactive in different Mcast 
 *  group. And the active VPort in a Mcast group will perform a replication 
 *  while frame is forwarding. Each VPort of a Port can use differnent VID for 
 *  tagged replication and one of those 16 Vports of a PPort can specified as 
 *  untagged replication.
 *
 *    Pleaes note that the VLAN tagged/untagged VPort is port based setting   
 *  instead of per port per MC group based setting. Different Mcast group to 
 *  activate a Vport of a PPort means this MC group on this PPort is complying
 *  the same VLAN tagged/untagged settting.
 *  
 */

/* _TB_SUBPORT_UC_STREAM_SUPPORT is the symbol for conditional compile usage.
 *  - Set this symbol define as 1 means subport API will perform the new 
 *      designed process to serve the unicast downstream and upstream between 
 *      serivce ports and user ports.
 *  - This design is for matching TR101 spec. on Residential N:1 VLAN of 
 *      Non-TLS traffice service.(in TR101: section 2.5.1)
 *  - Set this symbol define as 0 means subport API will perform the process 
 *      on serving multicast downstream request. (Still followed the TR101
 *      multicast section on forwarding N:1 VLAN)
 */
#define _TB_SUBPORT_UC_STREAM_SUPPORT   1

#define _TB_SUBPORT_NUM_PORT            29       
#define _TB_SUBPORT_NUM_MCASTREP_GROUP  256
#define _TB_SUBPORT_NUM_GROUP_PER_PORT  256     
#define _TB_SUBPORT_NUM_VPORT_PER_GROUP 16      
#define _TB_SUBPORT_NUM_VPORT_PER_PORT  _TB_SUBPORT_NUM_VPORT_PER_GROUP      
#define _TB_SUBPORT_NUM_GROUP   \
            (_TB_SUBPORT_NUM_PORT * _TB_SUBPORT_NUM_GROUP_PER_PORT)
            
#define _TB_SUPORT_GROUP_ID_MAX_PORT        0x1C
#define _TB_SUPORT_GROUP_ID_MASK_PORT       0x1F
#define _TB_SUPORT_GROUP_ID_MASK_GROUP      0xFF
#define _TB_SUPORT_GROUP_ID_SHIFT_PORT      8
#define _TB_MAX_SUPORT_GROUP_ID         \
            (_TB_SUPORT_GROUP_ID_MASK_GROUP | \
            (_TB_SUPORT_GROUP_ID_MAX_PORT << _TB_SUPORT_GROUP_ID_SHIFT_PORT))

/* Retrieve Port ID from a global subport group ID */
#define _TB_SUBPORT_GROUP_ID_2PORT_ID(vpg_mem_id)   \
            (((vpg_mem_id) >> _TB_SUPORT_GROUP_ID_SHIFT_PORT) & \
            _TB_SUPORT_GROUP_ID_MASK_PORT)
            
/* Retrieve Port basis group ID from a global subport group ID */
#define _TB_SUBPORT_GROUP_ID_2PGROUP_ID(vpg_mem_id) \
            ((vpg_mem_id) & _TB_SUPORT_GROUP_ID_MASK_GROUP)

#define _TB_SUBPORT_SYSTEM_GROUP_ID_GET(grp, port)   \
        ((((port) & _TB_SUPORT_GROUP_ID_MASK_PORT) << \
            _TB_SUPORT_GROUP_ID_SHIFT_PORT) | \
        ((grp) & _TB_SUPORT_GROUP_ID_MASK_GROUP))

/* TB defined a SW subport device basis ID to indicate a specific suport just 
 *  ESW chip(TR) has for the purpose of BCM compatible.
 */
/* SW defined device basis ubport ID (valid range in 0x00000-0x1CFFF):
 *  >> that means a specific subport device basis ID can be unique to 
 *      represent the location of a spcific suport group.
 */
#define _TB_SUBPORT_SYSTEM_ID_MAX       0x1CFFF
#define _TB_SUBPORT_SYSTEM_ID_SHIFT_VPGRP   4
#define _TB_SUBPORT_SYSTEM_ID_MASK_VPGRP    \
        ((_TB_SUPORT_GROUP_ID_MASK_GROUP) |     \
        (_TB_SUPORT_GROUP_ID_MASK_PORT << _TB_SUPORT_GROUP_ID_SHIFT_PORT))
#define _TB_SUBPORT_SYSTEM_ID_MASK_VPID     0xF

#define _TB_SUBPORT_SYSTEM_ID_GET(vpgrp_mem_id, vpid) \
        ((((vpgrp_mem_id) & _TB_SUBPORT_SYSTEM_ID_MASK_VPGRP) << \
        _TB_SUBPORT_SYSTEM_ID_SHIFT_VPGRP) | \
        ((vpid) & _TB_SUBPORT_SYSTEM_ID_MASK_VPID))
#define _TB_SUBPORT_SYSTEM_ID_2VPORT(sys_vp_id) \
        ((sys_vp_id) & _TB_SUBPORT_SYSTEM_ID_MASK_VPID)
#define _TB_SUBPORT_SYSTEM_ID_2VPG_ID(sys_vp_id) \
        (((sys_vp_id) >> _TB_SUBPORT_SYSTEM_ID_SHIFT_VPGRP) & \
        _TB_SUBPORT_SYSTEM_ID_MASK_VPGRP)
#define _TB_SUBPORT_SYSTEM_ID_2MCREP_ID(sys_vp_id) \
        (((sys_vp_id) >> _TB_SUBPORT_SYSTEM_ID_SHIFT_VPGRP) & \
        _TB_SUPORT_GROUP_ID_MASK_GROUP)
#define _TB_SUBPORT_SYSTEM_ID_2PORT(sys_vp_id) \
        ((_TB_SUBPORT_SYSTEM_ID_2VPG_ID(sys_vp_id) >> \
        _TB_SUPORT_GROUP_ID_SHIFT_PORT) & _TB_SUPORT_GROUP_ID_MASK_PORT)

/* set the system basis GPORT type subport_id from physical port(0-0x1c), 
 *  mc_group_id(0-0xff) and vp_id(0-0xf).
 */
#define _TB_SUBPORT_SYSTEM_ID_SET(port, grp, vpid) \
            ((((((port) & _TB_SUPORT_GROUP_ID_MASK_PORT) << \
        _TB_SUPORT_GROUP_ID_SHIFT_PORT) | \
        ((grp) & _TB_SUPORT_GROUP_ID_MASK_GROUP)) << \
        _TB_SUBPORT_SYSTEM_ID_SHIFT_VPGRP) | \
        ((vpid)&_TB_SUBPORT_SYSTEM_ID_MASK_VPID))
#define _TB_SUBPORT_GPORT_ID_SET(gport, port, grp, vpid) \
        BCM_GPORT_SUBPORT_PORT_SET((gport), \
        (_TB_SUBPORT_SYSTEM_ID_SET((port), (grp), (vpid))))
        
/* internal only, untag vp VID */
#define _TB_SUBPORT_UNTAG_VID       (BCM_VLAN_MAX + 1)  

extern int _bcm_tb_subport_vport_valid_check(
        int unit, bcm_port_t port, int vp_id, int *valid);

/* vport(subport) group database */
typedef struct drv_vpgrp_db_s {
    /* vpgrp_id must be a gport type at "BCM_GPORT_SUBPORT_GROUP" with valid 
     *  value range at 0-255.
     */
    bcm_gport_t     vpgrp_id;   /* vpgrp_id=-1 means this gport is invalid */
    bcm_gport_t     phy_port;   /* physical port type indicated */
    bcm_vlan_t      vlan;       /* service VLAN */
    uint32          vp_bmp;     /* vport bitmap */
    int             vp_cnt;     /* associated vport number count */
    
    /* Prop_flags : 
     *  - The vp_group's tag-type and PCP marking action for both downstream
     *      and upstream on NNI port.
     */
    uint32          prop_flags; /* property flags for this group */
    uint32          int_flags; /* internal flags for this group */
}drv_vpgrp_db_t;


#if _TB_SUBPORT_UC_STREAM_SUPPORT
/* SW information on Mcast replication group about SVLAN and Flow-ID */
typedef struct _drv_tb_mcrep_group_active_svlan_e{
    /* the flow_id can't be shared on downstream and upstream.
     *  - shared flow_id will causes problem on downstream forwarding.
     *    [Error Case] the configuration for upstream on EVM use flow_id key 
     *          only to force packet from UNI to NNI become Single STAG packet.
     *          This may causes the downstream packet from NNI to UNI been 
     *          force to Signl STAG also.
     */
    int     valid;
    int     svid;           /* SVLAN for this MCast group */
    int     down_flow_id;   /* for downstream */
    int     up_flow_id;     /* for upstream */
    int     ivm_eid;        /* for downstream */
    int     evm_eid;        /* for upstream */
    
    /* keep the active vp_group's tag-type and PCP marking action for both 
     *  downstream and upstream on NNI port.
     */
    uint32  prop_flags; /* property flags for this group */
}_drv_tb_mcrep_group_active_svlan_t;

/* vport(subport) database :
 *  
 *  1. valid : defined for the SW database will be designed as array format.
 *          Thus we need the valid field to indicate the vport's valid status.
 *  2. cvid : valid at 0 is priority tag and value at 1~4095 is normal tag.
 *  3. svid : defined here to keep the VID for upstream to V interface(TR101).
 *          In TB's HW spec. a vport of a port can be assigned to multiple 
 *          mcast group. In the real operating process, the upstream from 
 *          UNI to NNI with the a specific CVLAN can only be indicated to a
 *          predefined SVLAN only. This is limitation is described in TR101 as
 *          well. Thus we keep the active SVLAN for serving a mcast group's 
 *          upstream request. Once this mcast group is destroied and the
 *          active SVLAN will be change to the next found mcast group which 
 *          owned this vport also.
 *  4. flow_id : two flwo-id, one for downstream and the other for upstream.
 *  5. ivm_eid / evm_eid : keep the field entry id for further operation.
 *  6. prop_flags : flags for vport's tag_type and PCP marking action for 
 *          both downstream and upstream on the UNI port(vport)
 */
typedef struct drv_vport_db_s {
    int             valid;
    int             cvid;       /* customer VLAN */
    int             svid;       /* Service VLAN */
    int             int_pri;    /* internal priority for VP upstream */
    int             down_flow_id;   /* for downstream */
    int             up_flow_id;     /* for upstream */
    int             ivm_eid;    /* for upstream */
    int             evm_eid;    /* for downstream */
    
    uint32          prop_flags; /* property flags for this vport */
    uint32          int_flags; /* internal flags for this vport */

}drv_vport_db_t;

#define _BCM_TB_SUBPORT_VM_OP_CREATE   0x01 /* IVM/EVM operation:create */
#define _BCM_TB_SUBPORT_VM_OP_REMOVE   0x02 /* IVM/EVM operation:remove */

#define _BCM_TB_SUBPORT_VPORT_TC_MASK   0xF
#define _BCM_TB_SUBPORT_VPORT_DP_MASK   0x3
#define _BCM_TB_SUBPORT_VPORT_DP_SHIFT  4
#define _BCM_TB_SUBPORT_VPORT_PCP_MASK  0x7
#define _BCM_TB_SUBPORT_MAX_INT_PRIROTY     \
        (_BCM_TB_SUBPORT_VPORT_TC_MASK |    \
        (_BCM_TB_SUBPORT_VPORT_DP_MASK << _BCM_TB_SUBPORT_VPORT_DP_SHIFT))
        
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

/* bcm subport API flags:
 *  - downstream is for the packet from NNI to UNI.
 *  - upstream is for the packet from UNI to NNI.
 *
 *  Note : 
 *  1. SIT : Signle Inner Tag (Single C-Tag)
 *  2. PRITAG : Priority Tag.
 *  3. SOT : Signle Outer Tag(Single S-Tag)
 *  4. DT : Double Tag(S-Tag and C-Tag)
 *  5. ING : Ingress
 *  6. EGR : Egress
 *  7. DOWN : Downstream
 *  8. UP : Upstream
 */
#define _BCM_SUBPORT_FLAG_UP_ING_SIT            0x00000001
#define _BCM_SUBPORT_FLAG_UP_ING_PRITAG         0x00000002
#define _BCM_SUBPORT_FLAG_UP_ING_UNTAG          0x00000004
#define _BCM_SUBPORT_FLAG_UP_EGR_SOT            0x00000010
#define _BCM_SUBPORT_FLAG_UP_EGR_DT             0x00000020

#define _BCM_SUBPORT_FLAG_DOWN_ING_SOT          0x00000100
#define _BCM_SUBPORT_FLAG_DOWN_ING_DT           0x00000200
#define _BCM_SUBPORT_FLAG_DOWN_EGR_SIT          0x00001000
#define _BCM_SUBPORT_FLAG_DOWN_EGR_PRITAG       0x00002000
#define _BCM_SUBPORT_FLAG_DOWN_EGR_UNTAG        0x00004000

#define _BCM_SUBPORT_FLAG_EGR_CPCP_MAPPED       0x00010000
#define _BCM_SUBPORT_FLAG_EGR_CPCP_ING_CPCP     0x00020000
#define _BCM_SUBPORT_FLAG_EGR_CPCP_ING_SPCP     0x00040000
#define _BCM_SUBPORT_FLAG_EGR_CPCP_VP_CPCP      0x00080000
#define _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED       0x00100000
#define _BCM_SUBPORT_FLAG_EGR_SPCP_ING_CPCP     0x00200000
#define _BCM_SUBPORT_FLAG_EGR_SPCP_ING_SPCP     0x00400000
#define _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP      0x00800000
                                                    
/* subport flags group mask :
 *  - each group can be only one flag been set.
 */
#define _BCM_SUBPORT_FLAG_UP_ING_TYPE_MASK        \
                (_BCM_SUBPORT_FLAG_UP_ING_SIT |   \
                _BCM_SUBPORT_FLAG_UP_ING_PRITAG | \
                _BCM_SUBPORT_FLAG_UP_ING_UNTAG)
#define _BCM_SUBPORT_FLAG_UP_EGR_TYPE_MASK         \
                (_BCM_SUBPORT_FLAG_UP_EGR_SOT |    \
                _BCM_SUBPORT_FLAG_UP_EGR_DT)
#define _BCM_SUBPORT_FLAG_DOWN_ING_TYPE_MASK      \
                (_BCM_SUBPORT_FLAG_DOWN_ING_SOT | \
                _BCM_SUBPORT_FLAG_DOWN_ING_DT)
#define _BCM_SUBPORT_FLAG_DOWN_EGR_TYPE_MASK       \
                (_BCM_SUBPORT_FLAG_DOWN_EGR_SIT |  \
                _BCM_SUBPORT_FLAG_DOWN_EGR_PRITAG |\
                _BCM_SUBPORT_FLAG_DOWN_EGR_UNTAG)
#define _BCM_SUBPORT_FLAG_EGR_CPCP_REMARK_MASK     \
                (_BCM_SUBPORT_FLAG_EGR_CPCP_MAPPED |   \
                _BCM_SUBPORT_FLAG_EGR_CPCP_ING_CPCP |  \
                _BCM_SUBPORT_FLAG_EGR_CPCP_ING_SPCP |  \
                _BCM_SUBPORT_FLAG_EGR_CPCP_VP_CPCP)
#define _BCM_SUBPORT_FLAG_EGR_SPCP_REMARK_MASK     \
                (_BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED |   \
                _BCM_SUBPORT_FLAG_EGR_SPCP_ING_CPCP |  \
                _BCM_SUBPORT_FLAG_EGR_SPCP_ING_SPCP |  \
                _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP)
                
/* === Default Subport flags mask for subport group/vport creation === */

/* ---- Tag type flags for Group only ---- */
#define _BCM_SUBPORT_FLAG_TAG_TYPE_FOR_GROUP_MASK   \
                (_BCM_SUBPORT_FLAG_DOWN_ING_TYPE_MASK | \
                _BCM_SUBPORT_FLAG_UP_EGR_TYPE_MASK)
/* ---- Tag type flags for Vport only ---- */
#define _BCM_SUBPORT_FLAG_TAG_TYPE_FOR_VPORT_MASK   \
                (_BCM_SUBPORT_FLAG_UP_ING_TYPE_MASK | \
                _BCM_SUBPORT_FLAG_DOWN_EGR_TYPE_MASK)
 
/*  
 *  - The default type flags will be assigned if user request to create a 
 *      vp_group or vport without related flags assignment. 
 *  - The default type flags will be checked on each individual flag group.
 *  - The fefault type is assigned to support TR101 spec. on Residential N:1 
 *      VLAN of Non-TLS traffice service.(in TR101: section 2.5.1)
 */

/* _BCM_SUBPORT_FLAG_UP_ING_TYPE_DEFAULT and 
 *  _BCM_SUBPORT_FLAG_DOWN_EGR_TYPE_DEFAULT :
 *
 * 1. vp's default type could be C-Tagged/Pri-Tagged/Untagged. The structure  
 *  for vport add, i.e. bcm_subport_config_t, constructs pkt_vlan to indicate
 *  which type will be the VP's tagging status. That means vlan=0 for Pri-Tag 
 *  and vlan=1~4095 for C-Tag and the Untag is unable to be indicated in 
 *  pkt_vlan.
 */ 
#define _BCM_SUBPORT_FLAG_UP_ING_TYPE_DEFAULT       0
#define _BCM_SUBPORT_FLAG_DOWN_EGR_TYPE_DEFAULT     0
                
/* _BCM_SUBPORT_FLAG_UP_ING_TYPE_DEFAULT and 
 *  _BCM_SUBPORT_FLAG_UP_EGR_TYPE_DEFAULT :
 *
 * 1. TR101 specify the NNI frame in Residential N:1 VLAN solution must be
 *  single S-Tagged. Thus we set this default flag to SOT.
 */
#define _BCM_SUBPORT_FLAG_UP_EGR_TYPE_DEFAULT  \
                _BCM_SUBPORT_FLAG_UP_EGR_SOT
#define _BCM_SUBPORT_FLAG_DOWN_ING_TYPE_DEFAULT   \
                _BCM_SUBPORT_FLAG_DOWN_ING_SOT

/* _BCM_SUBPORT_FLAG_UP_EGR_CPCP_REMARK_DEFAULT and 
 *  _BCM_SUBPORT_FLAG_DOWN_EGR_SPCP_REMARK_DEFAULT :
 *
 * 1. TR101 specify the NNI frame in Residential N:1 VLAN solution must be
 *  single S-Tagged. Thus there is no C-Tag can carry the priority and the 
 *  default for such case will be set to 
 *  _BCM_SUBPORT_FLAG_UP_EGR_CPCP_MAPPED
 * 2. for the simular reason the UNI frame will has no S-Tag. And the default
 *  for such case will be _BCM_SUBPORT_FLAG_DOWN_EGR_SPCP_MAPPED
 */
#define _BCM_SUBPORT_FLAG_UP_EGR_CPCP_REMARK_DEFAULT  \
                _BCM_SUBPORT_FLAG_EGR_CPCP_MAPPED
#define _BCM_SUBPORT_FLAG_DOWN_EGR_SPCP_REMARK_DEFAULT   \
                _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED

/* _BCM_SUBPORT_FLAG_UP_EGR_CPCP_REMARK_DEFAULT and 
 *  _BCM_SUBPORT_FLAG_DOWN_EGR_SPCP_REMARK_DEFAULT :
 *
 * 1. TR101 specify the upstream to NNI in Residential N:1 VLAN solution must
 *  be single S-Tagged and the priority in S-Tag can be override or 
 *  propagate(from C-Tag). Thus we select propagate behavior to be the default
 *  flag, i.e. _BCM_SUBPORT_FLAG_UP_EGR_SPCP_ING_CPCP.
 * 2. For the simular reason of the downstream to UNI. We select propagate 
 *  behavior to be the default flag, i.e. 
 *  _BCM_SUBPORT_FLAG_DOWN_EGR_CPCP_ING_SPCP.
 */
#define _BCM_SUBPORT_FLAG_UP_EGR_SPCP_REMARK_DEFAULT  \
                _BCM_SUBPORT_FLAG_EGR_SPCP_ING_CPCP
#define _BCM_SUBPORT_FLAG_DOWN_EGR_CPCP_REMARK_DEFAULT   \
                _BCM_SUBPORT_FLAG_EGR_CPCP_ING_SPCP


#endif  /* BCM_TB_SUPPORT */

#define SUBPORT_GROUP_IS_VALID(vpgrp)   \
        BCM_GPORT_IS_SUBPORT_GROUP((vpgrp)->vpgrp_id)
#define SUBPORT_GROUP_IS_FREE(vpgrp)   \
        ((vpgrp)->vpgrp_id == BCM_GPORT_INVALID)
#define SUBPORT_GROUP_ID_SET(vpgrp, vpg_id)   \
        BCM_GPORT_SUBPORT_GROUP_SET((vpgrp)->vpgrp_id, (vpg_id))
#define SUBPORT_GROUP_ID_GET(vpgrp)   \
        BCM_GPORT_SUBPORT_GROUP_GET((vpgrp)->vpgrp_id)

extern int _bcm_robo_subport_group_resolve(
        int unit, bcm_gport_t gport, bcm_module_t *modid, bcm_port_t *port,
        int *mc_group, int *vpid);
#endif  /* _BCM_INT_SUBPORT_H */

