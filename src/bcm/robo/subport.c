/*
 * $Id: subport.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    subport.c
 * Purpose: Manages SUBPORT functions
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/vm.h>

#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/stack.h>
#include <bcm/subport.h>
#include <bcm/field.h>
#include <bcm/multicast.h>

#include <bcm_int/robo/subport.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/vlan.h>
#include <bcm_int/robo/field.h>

/* API designing Spec. difference between Triumph(ESW) and Thunderbolt(ROBO)
 *  1. ESW chip on supporting subport(ex. Triumph) is designed to uset the 
 *      priority in VLAN tag to indicating which the subport is used. Thus 
 *      there are upto 8 subports in each subport group for a physical port.
 *      In Robo chip, there is only thunderbolt can support subport feature.
 *      Not like ESW chip, the subport API for thunderbolt use customer VID 
 *      to indicate which subport is used in a subport group for a physical 
 *      port. There are upto 16 subports in a subport group.
 *  2. The defined Spec. on subport API is support Multicast and Unicast 
 *      subport leaning forwarding for downstream and upstream. The learning 
 *      is of the VID on both TR and TB are ServiceProvider VID. 
 *      That is, even the upsteam packet from a subport to a service port is 
 *      constructed a subport VID(not the service VID) the ARL learn will
 *      recognize this packet coming from subport and it will be learned with 
 *      it SA + Service VID.
 *
 *  Per the HW/SW designing Spec. difference, the simple list for the 
 *      difference between TR and TB are :
 *      - TR uses priority but TB uses custmer VID  to indicate a subport.
 *      - TR supports upto 8 subports but TB supports upto 16 subports per 
 *          subport group .
 *      - TR's subport group can be dynamic associated to a port but TB's 
 *          subport groups all separated to each specific port.
 *      - TR's subport in each subport group can have its own priority and 
 *          outgoing VID but TB's support has no priority information and 
 *          the subport VID is limited as port basis. That is, a subport(0-15) 
 *          for a physical port can be assigned to a subport VID only.
 *          Also, that means a physical port can constructed with 16 different
 *          subport VID.
 *      - TR's subport has a system basis unique ID but TB's subport in HW is
 *          boundled in each physical port with id 0-15.
 *          >> The SW design for TB's support ID to indicate a spefice subport
 *              is : vp_id = 0x00000 ~ 0x1CFFF. 
 *                  (bit20-bit4 for device basis subport group id and 
 *                  bit3-bit0 for subport_id)
 *          >> Note : the SW defined id will be no side effect to GPORT define
 *                  for the GPORT TYPE shipt bit started at bit26.
 */

#ifdef BCM_TB_SUPPORT

/* database for each suport group :
 *  
 *  This SW db will be allocated after init as the arrary with 
 *  [unit][port][mcast_group]
 *
 *      - port : 0 ~ 28
 *      - mcast_group : 0 ~ 255
 */
STATIC drv_vpgrp_db_t *_tb_vpgrp_db[BCM_MAX_NUM_UNITS] = { 0 };

#if _TB_SUBPORT_UC_STREAM_SUPPORT
/* database of the summaried mcast replication group information on SVLAN 
 *  and Flow-ID. 
 */
STATIC _drv_tb_mcrep_group_active_svlan_t 
        _tb_mcrep_vlan_info[BCM_MAX_NUM_UNITS][_TB_SUBPORT_NUM_MCASTREP_GROUP];

/* database for each suport group :
 *  
 *  This SW db will be allocated after init as the arrary with 
 *   [unit][port*vport]
 *
 *      - port : 0 ~ 28
 *      - vport : 0 ~ 15
 */
STATIC drv_vport_db_t *_tb_vport_db[BCM_MAX_NUM_UNITS] = { 0 };

/* the filed group id for both IVM/EVM entries */
STATIC bcm_field_group_t  _tb_vpgrp_ivm_group_id[BCM_MAX_NUM_UNITS];
STATIC bcm_field_group_t  _tb_vpgrp_evm_group_id[BCM_MAX_NUM_UNITS];

/* this macro is applied to retrieve the port basis vport id */
#define _BCM_PORT_VPORT_INDEX(_p, _vp)  \
    ((((_p) & _TB_SUPORT_GROUP_ID_MASK_PORT) << \
        _TB_SUBPORT_SYSTEM_ID_SHIFT_VPGRP) | \
        ((_vp) & _TB_SUBPORT_SYSTEM_ID_MASK_VPID))
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

/* Per-port count of all subport groups added */
STATIC int _tb_vpgrp_count[BCM_MAX_NUM_UNITS][_TB_SUBPORT_NUM_PORT] = {{ 0 }};

 /* check if the subport group for a specific port is full */
#define _TB_SUBPORT_GROUP_IS_EMPTY(unit, port) \
        (_tb_vpgrp_count[(unit)][(port)] <= 0)
        
/* check if the subport group for a specific port is full */
#define _TB_SUBPORT_GROUP_IS_FULL(unit, port) \
        (_tb_vpgrp_count[(unit)][(port)] >= _TB_SUBPORT_NUM_GROUP_PER_PORT)

 #define _SUBPORT_VP_LOCK(unit) \
        sal_mutex_take(_tb_subport_vp_mutex[(unit)], sal_mutex_FOREVER)

#define _SUBPORT_VP_UNLOCK(unit) \
        sal_mutex_give(_tb_subport_vp_mutex[(unit)])

STATIC sal_mutex_t _tb_subport_vp_mutex[BCM_MAX_NUM_UNITS] = {NULL};

#define _TB_SUBPORT_CHECK_INIT(_unit_) \
        if (!_tb_vpgrp_db[(_unit_)]) \
            return BCM_E_INIT

#define	_VP_BMP_BITOP(_a, _b, _op)	((_a) _op (1U << (_b)))


/* Specific operations */
#define	VPBMP_BITGET(_a, _b)	_VP_BMP_BITOP(_a, _b, &)
#define	VPBMP_BITSET(_a, _b)	_VP_BMP_BITOP(_a, _b, |=)
#define	VPBMP_BITCLR(_a, _b)	_VP_BMP_BITOP(_a, _b, &= ~)


#endif  /* BCM_TB_SUPPORT */

#define _BCM_ROBO_INVALID_ID -1

/* ========== internal routines =============== */
#ifdef BCM_TB_SUPPORT

/* definition for check prop_flags on vp_group creation or vport creation */
#define _BCM_TB_SUBPORT_FLAGS_CHECKOP_GROUP 0x01
#define _BCM_TB_SUBPORT_FLAGS_CHECKOP_VPORT 0x02

#if _TB_SUBPORT_UC_STREAM_SUPPORT
/*
 * Function:
 *      _bcm_tb_prop_flag_conflict_check
 * Purpose:
 *      Preview and check if any conflict occured in user's assignment 
 *      prop_flag for vp_group or vport creation process.
 * 
 * Returns:
 *      BCM_E_XXX
 *
 *  Note :
 *  1. This function is designed for checking the prop_flags of vp_group or 
 *      vport about creation process.
 *  2. To ensuring the API behavior will be proper on supporting Downstream 
 *      and upstream solution on NNI and UNI. Some predefined support behavior
 *      will be applied here to check if any improper flag observed.
 */
STATIC int
_bcm_tb_prop_flag_conflict_check(int unit, uint32 check_op, uint32 in_flags,
                uint32 *flag_changed, uint32 *active_flags)
{
    uint32  temp_flags = 0, new_flags = 0;
    uint32  ingress_tag_type = 0, egress_tag_type = 0,
            egress_cpcp_marking = 0, egress_spcp_marking = 0;
    
    *flag_changed = FALSE;
    if (check_op == _BCM_TB_SUBPORT_FLAGS_CHECKOP_GROUP){
        /* check if any VPORT used tagged_type assigned */
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_TAG_TYPE_FOR_VPORT_MASK;
        if (temp_flags) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        
        /* check ingress frame type on NNI of downstream direction :
         *  - assign default flag if no related flag set 
         *  - only one flags is allowed.
         */ 
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_DOWN_ING_TYPE_MASK;
        if (temp_flags == 0){
            ingress_tag_type |= _BCM_SUBPORT_FLAG_DOWN_ING_TYPE_DEFAULT;
            *flag_changed = TRUE;
        } else if ((temp_flags == _BCM_SUBPORT_FLAG_DOWN_ING_SOT) ||
                (temp_flags == _BCM_SUBPORT_FLAG_DOWN_ING_DT)){
            ingress_tag_type |= temp_flags;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        new_flags |= ingress_tag_type;
        
        /* check egress frame type on NNI of upstream direction :
         *  - assign default flag if no related flag set 
         *  - only one flags is allowed.
         */ 
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_UP_EGR_TYPE_MASK;
        if (temp_flags == 0){
            egress_tag_type |= _BCM_SUBPORT_FLAG_UP_EGR_TYPE_DEFAULT;
            *flag_changed = TRUE;
        } else if ((temp_flags == _BCM_SUBPORT_FLAG_UP_EGR_SOT) || 
                (temp_flags == _BCM_SUBPORT_FLAG_UP_EGR_DT)){
            egress_tag_type |= temp_flags;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        new_flags |= egress_tag_type;
        
        /* check egress CPCP remarking of downstream direction :
         *  - assign default flag if no related flag set 
         *  - only one flags is allowed. 
         *  - used in TB's vp_group IVM entry.
         *
         * Note : for Subport group creation, there is only one flag, i.e. 
         *      _BCM_SUBPORT_FLAG_EGR_CPCP_ING_SPCP is allowed and other's 
         *      will be treated as error configure here.
         */ 
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_REMARK_MASK;
        
        
        if (temp_flags == 0){
            egress_cpcp_marking |= 
                    _BCM_SUBPORT_FLAG_DOWN_EGR_CPCP_REMARK_DEFAULT;
            *flag_changed = TRUE;
        } else if (temp_flags == _BCM_SUBPORT_FLAG_EGR_CPCP_ING_SPCP){
            egress_cpcp_marking |= temp_flags;
        } else if ((temp_flags == _BCM_SUBPORT_FLAG_EGR_CPCP_VP_CPCP) ||
                (temp_flags == _BCM_SUBPORT_FLAG_EGR_CPCP_MAPPED) ||
                (temp_flags == _BCM_SUBPORT_FLAG_EGR_CPCP_ING_CPCP)){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "OP flags(0x%08x) invalid for group on PCP remarking!\n"),
                      in_flags)); 
            return BCM_E_PARAM;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        new_flags |= egress_cpcp_marking;

        /* check egress SPCP remarking of downstream direction :
         *  - assign default flag to 0 if no related flag is set
         *  - any flag set in this section is not reasonable. error config!
         *  - none zero flag can be set only by default flag macro defintion.
         *  - used in TB's vp_group IVM entry.
         */ 
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_REMARK_MASK;
        if (temp_flags == 0){
            egress_spcp_marking |= 
                    _BCM_SUBPORT_FLAG_DOWN_EGR_SPCP_REMARK_DEFAULT;
            *flag_changed = TRUE;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Error configure of SPCP Remark!\n"),
                      FUNCTION_NAME(), __LINE__)); 
            return BCM_E_PARAM;
        }
        new_flags |= egress_spcp_marking;
        
        *active_flags = new_flags;
        
    } else if (check_op == _BCM_TB_SUBPORT_FLAGS_CHECKOP_VPORT){
        /* check if any VP_GROUP used tagged_type assigned */
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_TAG_TYPE_FOR_GROUP_MASK;
        if (temp_flags) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        
        /* check ingress frame type on UNI of uptream direction :
         *  - assign default flag if no related flag set 
         *  - only one flags is allowed.
         */ 
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_UP_ING_TYPE_MASK;
        if (temp_flags == 0){
            ingress_tag_type |= _BCM_SUBPORT_FLAG_UP_ING_TYPE_DEFAULT;
            *flag_changed = TRUE;
        } else if ((temp_flags == _BCM_SUBPORT_FLAG_UP_ING_SIT) ||
                (temp_flags == _BCM_SUBPORT_FLAG_UP_ING_PRITAG) ||
                (temp_flags == _BCM_SUBPORT_FLAG_UP_ING_UNTAG)){
            ingress_tag_type |= temp_flags;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        new_flags |= ingress_tag_type;
        
        /* check egress frame type on UNI of downstream direction :
         *  - assign default flag if no related flag set 
         *  - only one flags is allowed.
         */ 
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_DOWN_EGR_TYPE_MASK;
        if (temp_flags == 0){
            egress_tag_type |= _BCM_SUBPORT_FLAG_DOWN_EGR_TYPE_DEFAULT;
            
            if (ingress_tag_type & _BCM_SUBPORT_FLAG_UP_ING_UNTAG){
                egress_tag_type |= _BCM_SUBPORT_FLAG_DOWN_EGR_UNTAG;
            }
            
            /* If user didn't assigned the egress tagging type on UNI, 
             *  normally the default behavior is tag-in/tag-out, untag-in /
             *  untag-out. Thus the Pri-Tag in in TR101 is a special case and
             *  Pri-Tag-in MUST BE Untag-out
             *  - TR101 Rule25(Section 3.1.1.3.1, R-25)
             *
             * Note : 
             *  The statement below to check ingress_tag_type must be verified
             *  if any conflict first, to avoid any unexpect result.
             */
            if (ingress_tag_type & _BCM_SUBPORT_FLAG_UP_ING_PRITAG){
                egress_tag_type |= _BCM_SUBPORT_FLAG_DOWN_EGR_UNTAG;
            }
            *flag_changed = TRUE;
        } else if ((temp_flags == _BCM_SUBPORT_FLAG_DOWN_EGR_SIT) ||
                (temp_flags == _BCM_SUBPORT_FLAG_DOWN_EGR_PRITAG) ||
                (temp_flags == _BCM_SUBPORT_FLAG_DOWN_EGR_UNTAG)){
            egress_tag_type |= temp_flags;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        new_flags |= egress_tag_type;
        
        /* check egress SPCP remarking of upstream direction :
         *  - assign default flag if no related flag set 
         *  - only one flags is allowed.
         *  - used in TB's vport IVM entry.
         */ 
        temp_flags = in_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_REMARK_MASK;
        if (temp_flags == 0){
            egress_cpcp_marking |= 
                    _BCM_SUBPORT_FLAG_UP_EGR_SPCP_REMARK_DEFAULT;
            *flag_changed = TRUE;
        } else if ((temp_flags == _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP) ||
                (temp_flags == _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED) ||
                (temp_flags == _BCM_SUBPORT_FLAG_EGR_SPCP_ING_CPCP) ||
                (temp_flags == _BCM_SUBPORT_FLAG_EGR_SPCP_ING_SPCP)){
            egress_cpcp_marking |= temp_flags;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Conflict OP flags(0x%08x) occured!\n"),
                      FUNCTION_NAME(), __LINE__, in_flags)); 
            return BCM_E_PARAM;
        }
        new_flags |= egress_cpcp_marking;

        /* check egress CPCP remarking of upstream direction :
         *  - assign default flag at 0 if no related flag is set
         *  - any flag set in this section is not reasonable. error config!
         *  - none zero flag can be set only by default flag macro defintion.
         *  - used in TB's vport IVM entry.
         */ 
        temp_flags = in_flags & 
                _BCM_SUBPORT_FLAG_EGR_CPCP_REMARK_MASK;
        if (temp_flags == 0){
            egress_spcp_marking |= 
                    _BCM_SUBPORT_FLAG_UP_EGR_CPCP_REMARK_DEFAULT;
            *flag_changed = TRUE;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,Error configure of CPCP remarking!\n"),
                      FUNCTION_NAME(), __LINE__)); 
            return BCM_E_PARAM;
        }
        new_flags |= egress_spcp_marking;
        
        *active_flags = new_flags;
        
    } else {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s,Invaild OP code(%d) for internal function.\n"),
                  FUNCTION_NAME(), check_op)); 
        return BCM_E_INTERNAL;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tb_subport_group_vm_handler
 * Purpose:
 *      TB's IVM/EVM handler for subport group related process. 
 * 
 * Returns:
 *      BCM_E_XXX
 *
 *  Note :
 *  1. TR101 service process about 2.5.1.3, VLAN tagged UNI- Single VT ATM or 
 *      Native Ethernet Architecture. 
 *      (Subport API for Residential N:1 VLAN solution, also non-TLS solution)
 *      
 *      a. The frame ingress on UNI for upstream is C-Tagged and the frame 
 *          egress on NNI will be Single S-Tagged.
 *          (TB support untagged or priority tagged to UNI for upstream, and 
 *          only one untagged or priority tagged VC(virtual connect) allowed)
 *      b. The frame ingress on NNI for downstream is Single S-Tagged and the  
 *          frame egress on UNI will be C-Tagged. 
 *          (TB support untagged or priority tagged to UNI for downstream, and 
 *          only one untagged or priority tagged VC(virtual connect) allowed)
 *
 */
STATIC int
_bcm_tb_subport_group_vm_handler(int unit, int op, bcm_port_t phy_port, 
        int vpgrp_id, int svlan)
{
    int     rv = BCM_E_NONE, rv_temp = BCM_E_NONE;
    int     down_vm_flow = 0, up_vm_flow = 0, qualifier_weight = 0;
    uint32  t_vm_data = 0, t_vm_mask = 0;
    uint32  active_flags = 0;
    bcm_field_group_t   ivm_group = 0, evm_group = 0;
    bcm_field_entry_t   down_vm_eid = 0, up_vm_eid = 0; /* IVM/EVM entry id */
    
    drv_vpgrp_db_t      *temp_vpgrp;
    _drv_tb_mcrep_group_active_svlan_t  *t_vpgrp_act_svlan_db;

    /* valid parameter check */
    if (!SOC_PORT_VALID(unit, phy_port)){
        return BCM_E_PARAM;
    }
    if ((svlan < BCM_VLAN_MIN) || (svlan > BCM_VLAN_MAX)){
        return BCM_E_PARAM;
    }
    if ((vpgrp_id < 0) || (vpgrp_id >= _TB_SUBPORT_NUM_MCASTREP_GROUP)){
        return BCM_E_PARAM;
    }

    /* retrieve SW database */ 
    t_vpgrp_act_svlan_db = _tb_mcrep_vlan_info[unit] + vpgrp_id;

    if (op == _BCM_TB_SUBPORT_VM_OP_CREATE){
        
        /* retreieve flags for group creation */
        temp_vpgrp = _tb_vpgrp_db[unit] + 
                _TB_SUBPORT_SYSTEM_GROUP_ID_GET(vpgrp_id, phy_port); 
        active_flags = temp_vpgrp->int_flags;
        
        /* Configuration flow :
         *
         *  1. Common process before VM configuration.
         *  2. Downstream process :
         *  3. Upstream process :
         *  4. SW database handler for TB's IVM/EVM information.
         */
        assert(t_vpgrp_act_svlan_db->valid == FALSE);
        
        /* 1. TB's IVM/EVM common processes on 
         *  - field GROUP create
         *  - field FLOW-ID request
         */
        if (_tb_vpgrp_ivm_group_id[unit] == _BCM_ROBO_INVALID_ID){
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vm_group_create(
                        unit, BCM_ROBO_VM_IVM_GROUP, &ivm_group));
            _tb_vpgrp_ivm_group_id[unit] = ivm_group;
        } else {
            ivm_group = _tb_vpgrp_ivm_group_id[unit];
        }
        if (_tb_vpgrp_evm_group_id[unit] == _BCM_ROBO_INVALID_ID){
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_vm_group_create(
                        unit, BCM_ROBO_VM_EVM_GROUP, &evm_group));
            _tb_vpgrp_evm_group_id[unit] = evm_group;
        } else {
            evm_group = _tb_vpgrp_evm_group_id[unit];
        }

        /* allocating flow-id for downstream and upstream */
        rv = DRV_VM_FLOW_ALLOC(unit, VM_FLOW_ID_GET_FROM_TAIL, &down_vm_flow);
        rv |= DRV_VM_FLOW_ALLOC(unit, VM_FLOW_ID_GET_FROM_TAIL, &up_vm_flow);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Allocating Flow-ID failed!\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        
        /* 2. Downstream process : ingress filter on NNI.
         *  - IVM key : 
         *      a. flags indicated valid configuration.
         *      b. if flags=0, the default keys are : 
         *          >> STagged, No-CTag, SVID
         *  - IVM action : 
         *      a. flags indicated valid configuration.
         *      b. if flags=0, the default actions are : 
         *          >> vid(svid), flow_id, cpcp_remark(b10)
         */
        rv = bcm_field_entry_create(unit, ivm_group, &down_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't create Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        qualifier_weight = 0;

        /* set IVM qualifiers :
         *  1. Tagged status : 
         *  2. outer VID : SVID
         */
        if (active_flags & _BCM_SUBPORT_FLAG_DOWN_ING_SOT){
            t_vm_data = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
        } else if (active_flags & _BCM_SUBPORT_FLAG_DOWN_ING_DT){
            t_vm_data = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED | 
                    BCM_FIELD_VLAN_FORMAT_INNER_TAGGED | 
                    BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
        } else {
            /* default : STagged and No-CTag */
            t_vm_data = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
        }
        rv = bcm_field_qualify_VlanFormat(unit, 
                down_vm_eid, (uint8)t_vm_data, 0);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        qualifier_weight += 2;  /* 2 keys for outer and inner tagged status */
        
        t_vm_data = svlan;
        t_vm_mask = VM_VID_MASK;
        rv = bcm_field_qualify_OuterVlanId(unit, 
                down_vm_eid, t_vm_data, t_vm_mask);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        qualifier_weight++;

        /* set IVM action :
         *  - default actions are : cpcp_remark, vid(svid), flow_id
         */
        /* bcmFieldActionPrioPktNew must use param1 instead of param0 */
        t_vm_data = 0;
        if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_MAPPED) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_MAPPED;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_ING_CPCP) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_USE_INNER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_ING_SPCP) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_USE_OUTER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_VP_CPCP) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_USE_PORT_DEFAULT;
        } else {
            /* default action */
            t_vm_data |= BCM_FIELD_CPCP_MARK_USE_OUTER_PCP;
        }
        if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_MAPPED;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_ING_CPCP) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_USE_INNER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_ING_SPCP) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_USE_OUTER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_USE_PORT_DEFAULT;
        } else {
            /* default action */
            t_vm_data |= BCM_FIELD_SPCP_MARK_MAPPED;
        }
        rv = bcm_field_action_add(unit, down_vm_eid, 
                bcmFieldActionPrioPktNew, 0, t_vm_data);
        rv |= bcm_field_action_add(unit, down_vm_eid, 
                bcmFieldActionVlanNew, svlan, 0);
        rv |= bcm_field_action_add(unit, down_vm_eid, 
                bcmFieldActionNewClassId, down_vm_flow, 
                _bcmFieldActionPrivateUsed);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't set action of Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        
        /* set action on VP PCP related */
        if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_MAPPED){
            t_vm_data = 0;  /* for V_Group this feature is not defined */
            rv = bcm_field_action_add(unit, down_vm_eid, 
                    bcmFieldActionVportDpNew, t_vm_data, 0);
            rv |= bcm_field_action_add(unit, down_vm_eid, 
                    bcmFieldActionVportTcNew, t_vm_data, 0);
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_VP_CPCP){
            t_vm_data = 0;  /* for VP_Group this feature is not defined */
            rv = bcm_field_action_add(unit, down_vm_eid, 
                    bcmFieldActionVportCpcpNew, t_vm_data, 0);
        } 
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't set action of Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        
        
        /* set IVM entry priority :
         *  - ROBO's SDK deisgning review conclude to set IVM priority based
         *      on the number of keys be qualified.
         */
        rv = bcm_field_entry_prio_set(unit, down_vm_eid, qualifier_weight);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prioritize Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }

        /* IVM Entry install */
        rv = bcm_field_entry_install(unit, down_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't install Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        
        /* 3. Upstream process : egress filter on NNI.
         *  - EVM key : flow_id
         *  - EVM action : stag_act(new_stag),new_svid(svid),ctag_act(remove)
         */
        rv = bcm_field_entry_create(unit, evm_group, &up_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't create Upstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        qualifier_weight = 0;

        /* set EVM qualifier : flow-id */
        t_vm_data = up_vm_flow;
        t_vm_mask = VM_FID_MASK;
        rv = bcm_field_qualify_FlowId(unit, up_vm_eid, 
                (uint16)t_vm_data, (uint16)t_vm_mask);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Upstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }
        /* Flow-ID is generated from IVM not a native value in packet.
         *  The meaning is the Flow-ID represented a hit IVM action thus we
         *  set weight at 5 to ensure this EVM will be performed if the 
         *  Flow-ID is matched.
         */
        qualifier_weight += 5;
        
        /* set EVM action :
         *  - set by indicating flags
         *  - if flag=0, default set to : new S-Tag with svid, remove C-Tag
         */
        if (active_flags & _BCM_SUBPORT_FLAG_UP_EGR_SOT){
            rv = bcm_field_action_add(unit, up_vm_eid, 
                    bcmFieldActionOuterVlanNew, svlan, BCM_FIELD_TAG_REPLACE);
                    
            rv |= bcm_field_action_add(unit, up_vm_eid, 
                    drvFieldActionInnerVlanNew, 0, BCM_FIELD_TAG_REMOVE);
        } else if (active_flags & _BCM_SUBPORT_FLAG_UP_EGR_DT){
            rv = bcm_field_action_add(unit, up_vm_eid, 
                    bcmFieldActionOuterVlanNew, svlan, BCM_FIELD_TAG_REPLACE);
                    
            rv |= bcm_field_action_add(unit, up_vm_eid, 
                    drvFieldActionInnerVlanNew, 0, BCM_FIELD_TAG_AS_RECEIVED);
        } else {
            rv = bcm_field_action_add(unit, up_vm_eid, 
                    bcmFieldActionOuterVlanNew, svlan, BCM_FIELD_TAG_REPLACE);
                    
            rv |= bcm_field_action_add(unit, up_vm_eid, 
                    drvFieldActionInnerVlanNew, 0, BCM_FIELD_TAG_REMOVE);
        }
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't set action of Upstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }

        /* set EVM entry priority :
         *  - ROBO's SDK deisgning review conclude to set EVM priority based
         *      on the number of keys be qualified.
         */
        rv = bcm_field_entry_prio_set(unit, up_vm_eid, qualifier_weight);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prioritize Upstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }

        /* EVM Entry install */
        rv = bcm_field_entry_install(unit, up_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't install Upstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto group_vm_failed_exit;
        }

        /* 4.SW database handler for TB's IVM/EVM information */
        t_vpgrp_act_svlan_db->valid = TRUE;
        t_vpgrp_act_svlan_db->svid = svlan;
        t_vpgrp_act_svlan_db->down_flow_id = down_vm_flow;
        t_vpgrp_act_svlan_db->up_flow_id = up_vm_flow;
        t_vpgrp_act_svlan_db->ivm_eid = down_vm_eid;
        t_vpgrp_act_svlan_db->evm_eid = up_vm_eid;
        t_vpgrp_act_svlan_db->prop_flags = active_flags;
        
        return BCM_E_NONE;
    } else if (op == _BCM_TB_SUBPORT_VM_OP_REMOVE){
        
        /* Configuration flow :
         *
         *  1. Retrieve FLOW-ID and IVM/EVM entry-id
         *  2. destory IVM/EVM entry.
         *  3. FLOW-ID release.
         *  4. SW database handler for TB's IVM/EVM information.
         */
        assert(t_vpgrp_act_svlan_db->valid == TRUE);

        /* 1. Retrieve FLOW-ID and IVM/EVM entry-id */
        down_vm_flow = t_vpgrp_act_svlan_db->down_flow_id;
        up_vm_flow = t_vpgrp_act_svlan_db->up_flow_id;
        down_vm_eid = t_vpgrp_act_svlan_db->ivm_eid;
        up_vm_eid = t_vpgrp_act_svlan_db->evm_eid;

        /* 2. destory IVM/EVM entry. */
        if (down_vm_eid != _BCM_ROBO_INVALID_ID){
            rv_temp = bcm_field_entry_destroy(unit, down_vm_eid);
        }
        if (up_vm_eid != _BCM_ROBO_INVALID_ID){
            rv_temp |= bcm_field_entry_destroy(unit, up_vm_eid);
        }
        if (rv_temp != BCM_E_NONE) {
            /* warning message only without error return to finish all remove
             *  related configuration.
             */
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Releasing IVM/EVM entries failed!\n"), 
                      FUNCTION_NAME(), __LINE__));
        }

        /* 3. FLOW-ID release. */
        if (down_vm_flow != _BCM_ROBO_INVALID_ID){
            if (DRV_VM_FLOW_FREE(unit, down_vm_flow)){
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, Releasing Flow-ID failed!\n"), 
                          FUNCTION_NAME(), __LINE__));
            }
        }
        if (up_vm_flow != _BCM_ROBO_INVALID_ID){
            if (DRV_VM_FLOW_FREE(unit, up_vm_flow)){
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, Releasing Flow-ID failed!\n"), 
                          FUNCTION_NAME(), __LINE__));
            }
        }
        
        /* 4.SW database handler for TB's IVM/EVM information */
        t_vpgrp_act_svlan_db->valid = FALSE;
        t_vpgrp_act_svlan_db->svid = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->down_flow_id = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->up_flow_id = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->ivm_eid= _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->evm_eid= _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->prop_flags = 0;
        
        return BCM_E_NONE;
    } else {
        rv = BCM_E_PARAM;
    }
    
group_vm_failed_exit :
    /* for CREATE : 
     *  - free allocated flow-ID and destroy the created IVM/EVM entry 
     */
    if (down_vm_flow != _BCM_ROBO_INVALID_ID){
        if (DRV_VM_FLOW_FREE(unit, down_vm_flow)){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Releasing Flow-ID failed!\n"), 
                      FUNCTION_NAME(), __LINE__));
        }
    }
    if (up_vm_flow != _BCM_ROBO_INVALID_ID){
        if (DRV_VM_FLOW_FREE(unit, up_vm_flow)){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Releasing Flow-ID failed!\n"), 
                      FUNCTION_NAME(), __LINE__));
        }
    }
    rv_temp = bcm_field_entry_destroy(unit, down_vm_eid);
    rv_temp |= bcm_field_entry_destroy(unit, up_vm_eid);

    if ((rv_temp != BCM_E_NONE) && (rv_temp != BCM_E_NOT_FOUND)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s,%d, Releasing IVM/EVM entries failed!\n"), 
                  FUNCTION_NAME(), __LINE__));
    }

    return rv;
}

/*
 * Function:
 *      _bcm_tb_subport_vport_vm_handler
 * Purpose:
 *      TB's IVM/EVM handler for subport vport related process. 
 * 
 * Returns:
 *      BCM_E_XXX
 *
 *  Note :
 *  1. TR101 service process about 2.5.1.3, VLAN tagged UNI- Single VT ATM or 
 *      Native Ethernet Architecture. 
 *      (Subport API for Residential N:1 VLAN solution, also non-TLS solution)
 *      
 *      a. The frame ingress on UNI for upstream is C-Tagged and the frame 
 *          egress on NNI will be Single S-Tagged.
 *          (TB support untagged or priority tagged to UNI for upstream, and 
 *          only one untagged or priority tagged VC(virtual connect) allowed)
 *      b. The frame ingress on NNI for downstream is Single S-Tagged and the  
 *          frame egress on UNI will be C-Tagged. 
 *          (TB support untagged or priority tagged to UNI for downstream, and 
 *          only one untagged or priority tagged VC(virtual connect) allowed)
 */
STATIC int
_bcm_tb_subport_vport_vm_handler(int unit, int op, bcm_port_t phy_port, 
        int vport, int vpgrp_id, int cvlan)
{
    int         rv = BCM_E_NONE, rv_temp = BCM_E_NONE;
    int         down_vm_flow = 0, up_vm_flow = 0, qualifier_weight = 0;
    int         svlan = 0;
    int         ivm_group = 0, evm_group = 0;
    int         down_vm_eid = 0, up_vm_eid = 0; /* IVM/EVM entry id */
    uint32      t_vm_data = 0, t_vm_mask = 0;
    uint32      active_flags = 0, param0 = 0, param1 = 0;
    drv_vport_db_t      *t_vport_db;
    _drv_tb_mcrep_group_active_svlan_t  *t_vpgrp_act_svlan_db;

    /* valid parameter check */
    if (!SOC_PORT_VALID(unit, phy_port)){
        return BCM_E_PARAM;
    }
    if (vport < 0 || vport >= _TB_SUBPORT_NUM_VPORT_PER_PORT){
        return BCM_E_PARAM;
    }
    if ((cvlan < BCM_VLAN_MIN) || (cvlan > BCM_VLAN_MAX)){
        /* cvlan=_TB_SUBPORT_UNTAG_VID is the special vid to indicate an 
         *  untagged vp. 
         */
        if (cvlan != _TB_SUBPORT_UNTAG_VID){
            return BCM_E_PARAM;
        }
    }
    if ((vpgrp_id < 0) || (vpgrp_id >= _TB_SUBPORT_NUM_MCASTREP_GROUP)){
        return BCM_E_PARAM;
    }

    t_vpgrp_act_svlan_db = _tb_mcrep_vlan_info[unit] + vpgrp_id;

    /* retrieve vport_sw_database */
    assert(_tb_vport_db[unit] != NULL);
    t_vport_db = _tb_vport_db[unit] + 
            _BCM_PORT_VPORT_INDEX(phy_port, vport);
        
    if (op == _BCM_TB_SUBPORT_VM_OP_CREATE){
        /* Configuration flow :
         *  1. Retrieve existed configuration for vport ivm/evm creation and 
         *      check if any conflict existed.
         *  2. Downstream process :
         *  3. Upstream process :
         *  4. SW database handler for TB's IVM/EVM information.
         */
        
        /* 1. Retrieve existed configuration for vport ivm/evm creation. :
         *  - ivm/evm group id.
         *  - SLVAN for the vp group.
         *  - flow-id for downstream and upstream.
         *
         *  - check if this vport construct existed IVM/EVM already.
         *      (warning message only and return with no error.
         */
        ivm_group = _tb_vpgrp_ivm_group_id[unit];
        evm_group = _tb_vpgrp_evm_group_id[unit];
        assert((ivm_group != _BCM_ROBO_INVALID_ID) && 
                (evm_group != _BCM_ROBO_INVALID_ID));

        if (t_vpgrp_act_svlan_db->valid == TRUE){
            svlan = t_vpgrp_act_svlan_db->svid;
            down_vm_flow = t_vpgrp_act_svlan_db->down_flow_id;
            up_vm_flow= t_vpgrp_act_svlan_db->up_flow_id;
            if ((svlan == _BCM_ROBO_INVALID_ID) || 
                    (down_vm_flow == _BCM_ROBO_INVALID_ID) ||
                    (up_vm_flow == _BCM_ROBO_INVALID_ID)){
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "No proper configuration of subport group!\n")));
                return BCM_E_CONFIG;
            }
        }

        if (t_vport_db->valid == TRUE){
            /* check conflict condition of svlan. 
             *
             *  - flow-id for downstream and upstream can be different for the 
             *  case when this vport of this port is the member of other 
             *  vp_group.   There will be no problem if the bounded SVLAN of 
             *  that vp_group is the same with this SVLAN.
             *
             *  - If svlan conflict occurred, that means this vport of a 
             *      physical port has been created to serve different vp_group  
             *      and that group has conflict svlan bounded.
             *    >> Such configuration is assumed been limited in subport
             *      group creation process.
             */
            if (t_vport_db->svid != svlan){
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "This VP was existed to serve other subport group!\n"
                                     " -This SVLAN(%d) conflicts with existed SVLAN(%d)\n"),
                          svlan, t_vport_db->svid));
                return BCM_E_CONFIG;
            } else {
                /* Return with no error if no svlan conflict.
                 *  - current IVM/EVM can works properly.
                 */
                 return BCM_E_NONE;
            }
        }
        
        /* retrieve the flags for VPort creation */
        active_flags = t_vport_db->int_flags;
        
        /* 2. Downstream process : egress filter on UNI.
         *  - EVM key : FlowID, EgressPort, EgressVport
         *  - EVM action : 
         *      a. set by indicating flags
         *      b. if flags=0, default actions are :
         *          >> STag(remove), CTag(new/remove) 
        */
        rv = bcm_field_entry_create(unit, evm_group, &down_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't create Downstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        qualifier_weight = 0;
        
        /* set EVM qualifier : flow-id */
        t_vm_data = down_vm_flow;
        t_vm_mask = VM_FID_MASK;
        rv = bcm_field_qualify_FlowId(unit, down_vm_eid, 
                (uint16)t_vm_data, (uint16)t_vm_mask);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Downstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        /* Flow-ID is generated from IVM not a native value in packet.
         *  The meaning is the Flow-ID represented a hit IVM action thus we
         *  set weight at 5 to ensure this EVM will be performed if the 
         *  Flow-ID is matched.
         */
        qualifier_weight += 5;
        
        /* set EVM qualifier : egress-port */
        t_vm_data = phy_port;
        t_vm_mask = VM_PORT_MASK;
        rv = bcm_field_qualify_OutPort(unit, down_vm_eid, 
                (bcm_port_t)t_vm_data, (bcm_port_t)t_vm_mask);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Downstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        qualifier_weight ++;
        
        /* set EVM qualifier : egress-vport */
        t_vm_data = vport;
        t_vm_mask = VM_VPORT_MASK;
        rv = bcm_field_qualify_OutVPort(unit, down_vm_eid, 
                (uint8)t_vm_data, (uint8)t_vm_mask);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Downstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        qualifier_weight ++;
        
        /* set EVM action : STag(remove), CTag(new/remove) 
         *  - if vp is 1Q-tag vp or pri-tag vp : action on CTag is "new"
         *  - if vp is untag vp : action on CTag is "remove"
         */
        rv = bcm_field_action_add(unit, down_vm_eid, 
                bcmFieldActionOuterVlanNew, 0, BCM_FIELD_TAG_REMOVE);
        if (active_flags & _BCM_SUBPORT_FLAG_DOWN_EGR_SIT){
            param0 = cvlan;
            param1 = BCM_FIELD_TAG_REPLACE;
        } else if (active_flags & _BCM_SUBPORT_FLAG_DOWN_EGR_PRITAG){
            param0 = 0;
            param1 = BCM_FIELD_TAG_REPLACE;
        } else if (active_flags & _BCM_SUBPORT_FLAG_DOWN_EGR_UNTAG){
            param0 = 0;
            param1 = BCM_FIELD_TAG_REMOVE;
        } else {
            param0 = cvlan;
            param1 = BCM_FIELD_TAG_REPLACE;
        }
        rv |= bcm_field_action_add(unit, down_vm_eid, 
                drvFieldActionInnerVlanNew, param0, param1);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't set action of Downstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }

        /* set EVM entry priority :
         *  - ROBO's SDK deisgning review conclude to set EVM priority based
         *      on the number of keys be qualified.
         */
        rv = bcm_field_entry_prio_set(unit, down_vm_eid, qualifier_weight);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prioritize Downstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }

        /* EVM Entry install */
        rv = bcm_field_entry_install(unit, down_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't install Downstream EVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
         
        /* 3. Upstream process : ingress filter on UNI.
         *  - IVM key : 
         *  - IVM action : 
         */
        rv = bcm_field_entry_create(unit, ivm_group, &up_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't create Downstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        qualifier_weight = 0;
        
        /* set IVM qualifiers : 
         *  - set by the indicating flags
         *  - if flag=0, set default keys are :
         *      >> No-STag, CTag(Tagged/Pri-Tagged/Untagged) 
         */

        /* set IVM qualifiers : CTag status for No-Tag/Pri-Tag/Untag */
        if (active_flags & _BCM_SUBPORT_FLAG_UP_ING_SIT){
            t_vm_data = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
        } else if (active_flags & _BCM_SUBPORT_FLAG_UP_ING_PRITAG){
            t_vm_data = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
        } else if (active_flags & _BCM_SUBPORT_FLAG_UP_ING_UNTAG){
            t_vm_data = 0;
        } else {
            if (cvlan == 0) {
                /* priority tag vp */
                t_vm_data = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED_VID_ZERO;
            } else {
                /* normal tag vp */
                t_vm_data = BCM_FIELD_VLAN_FORMAT_INNER_TAGGED;
            }
        }
        
        rv = bcm_field_qualify_VlanFormat(unit, 
                up_vm_eid, (uint8)t_vm_data, 0);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Upstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        qualifier_weight += 2;  /* 2 keys for outer and inner tagged status */
        
        /* set IVM qualifiers : CVID */
        if (!(active_flags & (_BCM_SUBPORT_FLAG_UP_ING_PRITAG | 
                _BCM_SUBPORT_FLAG_UP_ING_UNTAG))){
            t_vm_data = cvlan;
            t_vm_mask = VM_VID_MASK;
            rv = bcm_field_qualify_InnerVlanId(unit, up_vm_eid,
                    t_vm_data, t_vm_mask);
            if (rv != BCM_E_NONE) {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, Can't prepare Upstream IVM entry.\n"), 
                          FUNCTION_NAME(), __LINE__));
                goto vport_vm_failed_exit;
            }
        }
        
        /* weight increamented on both pri-tag and untag. This is for forcing 
         *  this IVM entry has in the same priority with 1Q-Tagged ivm entry.
         */ 
        qualifier_weight++;
        
        /* set IVM qualifiers : ingress-port */
        t_vm_data = phy_port;
        t_vm_mask = VM_PORT_MASK;
        rv = bcm_field_qualify_InPort(unit, up_vm_eid, 
                (bcm_port_t)t_vm_data, (bcm_port_t)t_vm_mask);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prepare Upstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        qualifier_weight++;
        
        /* set IVM action :
         *  - spcp_remark,vid(svid), flow_id, vport
         */
        t_vm_data = 0;
        if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_MAPPED) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_MAPPED;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_ING_CPCP) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_USE_INNER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_ING_SPCP) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_USE_OUTER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_CPCP_VP_CPCP) {
            t_vm_data |= BCM_FIELD_CPCP_MARK_USE_PORT_DEFAULT;
        } else {
            /* default action */
            t_vm_data |= BCM_FIELD_CPCP_MARK_MAPPED;
        }
        if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_MAPPED;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_ING_CPCP) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_USE_INNER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_ING_SPCP) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_USE_OUTER_PCP;
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP) {
            t_vm_data |= BCM_FIELD_SPCP_MARK_USE_PORT_DEFAULT;
        } else {
            /* default action */
            t_vm_data |= BCM_FIELD_SPCP_MARK_USE_INNER_PCP;
        }
        /* bcmFieldActionPrioPktNew must use param1 instead of param0 */
        rv = bcm_field_action_add(unit, up_vm_eid, 
                bcmFieldActionPrioPktNew, 0, t_vm_data);
        rv |= bcm_field_action_add(unit, up_vm_eid, 
                bcmFieldActionVlanNew, svlan, 0);
        rv |= bcm_field_action_add(unit, up_vm_eid, 
                bcmFieldActionNewClassId, up_vm_flow, 
                _bcmFieldActionPrivateUsed);
        rv |= bcm_field_action_add(unit, up_vm_eid, 
                bcmFieldActionVportNew, vport, 0);
                
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't set action of Upstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }

        /* set action on VP PCP related */
        if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED){
            t_vm_data = (_BCM_TB_SUBPORT_VPORT_DP_MASK &
                    (t_vport_db->int_pri >> _BCM_TB_SUBPORT_VPORT_DP_SHIFT)); 
            rv = bcm_field_action_add(unit, up_vm_eid, 
                    bcmFieldActionVportDpNew, t_vm_data, 0);
            t_vm_data = _BCM_TB_SUBPORT_VPORT_TC_MASK & t_vport_db->int_pri ;
            rv |= bcm_field_action_add(unit, up_vm_eid, 
                    bcmFieldActionVportTcNew, t_vm_data, 0);
        } else if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP){
            t_vm_data = t_vport_db->int_pri;
            rv = bcm_field_action_add(unit, up_vm_eid, 
                    bcmFieldActionVportSpcpNew, t_vm_data, 0);
        }
        
        /* set IVM entry priority :
         *  - ROBO's SDK deisgning review conclude to set IVM priority based
         *      on the number of keys be qualified.
         */
        rv |= bcm_field_entry_prio_set(unit, up_vm_eid, qualifier_weight);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't prioritize Upstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }
        
        /* IVM Entry install */
        rv = bcm_field_entry_install(unit, up_vm_eid);
        if (rv != BCM_E_NONE) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Can't install Upstream IVM entry.\n"), 
                      FUNCTION_NAME(), __LINE__));
            goto vport_vm_failed_exit;
        }

        /* 4. SW database handler for this new IVM/EVM information. */
        t_vport_db->valid = TRUE;
        t_vport_db->cvid = cvlan;
        t_vport_db->svid = svlan;
        t_vport_db->down_flow_id = down_vm_flow;
        t_vport_db->up_flow_id = up_vm_flow;
        t_vport_db->evm_eid = down_vm_eid;
        t_vport_db->ivm_eid = up_vm_eid;        
        
        return BCM_E_NONE;
    } else if (op == _BCM_TB_SUBPORT_VM_OP_REMOVE){
        /* Configuration flow :
         *
         *  1. Retrieve IVM/EVM entry-id
         *  2. destory IVM/EVM entry.
         *  3. SW database handler for TB's IVM/EVM information.
         *
         *  Note : 
         *  1. This VPort is going to be deleted and no other subport group
         *      construct this VPort still. 
         *      >> function caller must confirm this rule.
         */
        assert(t_vport_db->valid == TRUE);
        
        down_vm_eid = t_vport_db->evm_eid;
        up_vm_eid = t_vport_db->ivm_eid;

        assert((down_vm_eid != _BCM_ROBO_INVALID_ID) && 
                (up_vm_eid != _BCM_ROBO_INVALID_ID));
        
        rv_temp = bcm_field_entry_destroy(unit, down_vm_eid);
        rv_temp |= bcm_field_entry_destroy(unit, up_vm_eid);
        
        if ((rv_temp != BCM_E_NONE) && (rv_temp != BCM_E_NOT_FOUND)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Releasing IVM/EVM entries failed!\n"), 
                      FUNCTION_NAME(), __LINE__));
        }

        t_vport_db->valid = FALSE;
        t_vport_db->cvid = _BCM_ROBO_INVALID_ID;
        t_vport_db->svid = _BCM_ROBO_INVALID_ID;
        t_vport_db->down_flow_id = _BCM_ROBO_INVALID_ID;
        t_vport_db->up_flow_id = _BCM_ROBO_INVALID_ID;
        t_vport_db->evm_eid = _BCM_ROBO_INVALID_ID;
        t_vport_db->ivm_eid = _BCM_ROBO_INVALID_ID;        
        
        return BCM_E_NONE;
    } else {
        rv = BCM_E_PARAM;
    }

vport_vm_failed_exit:
    /* destroy the created IVM/EVM entries */
    rv_temp = bcm_field_entry_destroy(unit, down_vm_eid);
    rv_temp |= bcm_field_entry_destroy(unit, up_vm_eid);

    if ((rv_temp != BCM_E_NONE) && (rv_temp != BCM_E_NOT_FOUND)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s,%d, Releasing IVM/EVM entries failed!\n"), 
                  FUNCTION_NAME(), __LINE__));
    }

    return rv;
}
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT*/
#endif  /* BCM_TB_SUPPORT */

/*
 * Function:
 *      _bcm_robo_subport_group_resolve
 * Purpose:
 *      Get the modid, port, mc_group and vp_id values for a subport port 
 *      GPORT
 * Returns:
 *      BCM_E_XXX
 *
 *  Note :
 *  1. Tow subport type indicated different index formate.
 *      - BCM_GPORT_SUBPORT_GROUP_TYEP and BCM_GPORT_SUBPORT_PORT_TYPE
 */
int
_bcm_robo_subport_group_resolve(int unit, bcm_gport_t gport,
        bcm_module_t *modid, bcm_port_t *port,
        int *mc_group, int *vpid)
{
#ifdef BCM_TB_SUPPORT
    int rv = BCM_E_NONE; 
    int gport_id = -1, phy_port = -1, group = -1, vport = -1;
    
    _TB_SUBPORT_CHECK_INIT(unit);

    assert((modid != NULL) && (port != NULL) && 
            (mc_group != NULL) && (vpid != NULL));
    
    if (SOC_IS_TBX(unit)) {
        *modid = unit;
        if (BCM_GPORT_IS_SUBPORT_GROUP(gport)){
            gport_id = BCM_GPORT_SUBPORT_GROUP_GET(gport);
            phy_port = _TB_SUBPORT_GROUP_ID_2PORT_ID(gport_id);
        } else if (BCM_GPORT_IS_SUBPORT_PORT(gport)){
            gport_id = BCM_GPORT_SUBPORT_PORT_GET(gport);
            phy_port = _TB_SUBPORT_SYSTEM_ID_2PORT(gport_id);
            group = _TB_SUBPORT_SYSTEM_ID_2MCREP_ID(gport_id);
            vport = _TB_SUBPORT_SYSTEM_ID_2VPORT(gport_id);
        } else {
            rv = BCM_E_UNAVAIL;
        }
        
    } else {
        rv = BCM_E_UNAVAIL;
    }
    
    *port = phy_port;
    *mc_group = group;
    *vpid = vport;
    
    if (!SOC_PORT_VALID(unit, phy_port)) {
        rv = BCM_E_PORT;
    }
    return rv;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
}


#ifdef BCM_TB_SUPPORT
STATIC void
_bcm_tb_subport_free_resource(int unit)
{
#if _TB_SUBPORT_UC_STREAM_SUPPORT
    int i;
    _drv_tb_mcrep_group_active_svlan_t  *t_vpgrp_act_svlan_db;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    if (_tb_vpgrp_db[unit]) {
        sal_free(_tb_vpgrp_db[unit]);
        _tb_vpgrp_db[unit] = NULL;
    }

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    if (_tb_vport_db[unit]) {
        sal_free(_tb_vport_db[unit]);
        _tb_vport_db[unit] = NULL;
    }

    _tb_vpgrp_ivm_group_id[unit] = _BCM_ROBO_INVALID_ID;
    _tb_vpgrp_evm_group_id[unit] = _BCM_ROBO_INVALID_ID;

    for (i = 0; i < _TB_SUBPORT_NUM_MCASTREP_GROUP; i++) {
        t_vpgrp_act_svlan_db = _tb_mcrep_vlan_info[unit] + i;
        
        t_vpgrp_act_svlan_db->valid = FALSE;
        t_vpgrp_act_svlan_db->svid = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->down_flow_id = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->up_flow_id = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->ivm_eid = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->evm_eid = _BCM_ROBO_INVALID_ID;
    }

#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    if (_tb_subport_vp_mutex[unit] != NULL) {
        sal_mutex_destroy(_tb_subport_vp_mutex[unit]);
        _tb_subport_vp_mutex[unit] = NULL;
    }
}
#endif  /* BCM_TB_SUPPORT */

/*
 * Function:
 *      _bcm_robo_subport_hw_clear
 * Purpose:
 *      Cleanup the SUBPORT hw module.
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_robo_subport_hw_clear(int unit)
{
#ifdef BCM_TB_SUPPORT
    int     rv = BCM_E_NONE;
    int     init_status = 0;

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    int     i, j;
    int     temp_flowid = 0, temp_vm_eid = 0;
    drv_vport_db_t      *t_vport_db;    
    _drv_tb_mcrep_group_active_svlan_t  *t_vpgrp_act_svlan_db;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    
    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }

    /* check init status. If init already, applying LOCK/UNLOCK */    
    init_status = (_tb_vpgrp_db[unit] == NULL) ? 0 : 1;
    if (init_status){
        _SUBPORT_VP_LOCK(unit);
    }

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    /* clear crated IVM/EVM configuration and the allocated FLOW-ID :
     *  - Though the FLOW-ID is a SW related. We perform release process for 
     *      the flow-id is always related to IVM/EVM entry.
     */
     
    /* clear IVM/EVM on group parts */
    for (i = 0; i < _TB_SUBPORT_NUM_MCASTREP_GROUP; i++){
        t_vpgrp_act_svlan_db = _tb_mcrep_vlan_info[unit] + i;
        if (t_vpgrp_act_svlan_db->valid == FALSE){
            continue;
        }
        
        if (t_vpgrp_act_svlan_db->down_flow_id != _BCM_ROBO_INVALID_ID){
           temp_flowid = t_vpgrp_act_svlan_db->down_flow_id;
            if (DRV_VM_FLOW_FREE(unit, temp_flowid)){
                /* warning message only, no error return to finish all clear 
                 * process. */
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, Releasing Flow-ID failed!\n"), 
                          FUNCTION_NAME(), __LINE__));
            }
        }
        if (t_vpgrp_act_svlan_db->up_flow_id != _BCM_ROBO_INVALID_ID){
            temp_flowid = t_vpgrp_act_svlan_db->up_flow_id;
            if (DRV_VM_FLOW_FREE(unit, temp_flowid)){
                /* warning message only, no error return to finish all clear 
                 * process. */
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, Releasing Flow-ID failed!\n"), 
                          FUNCTION_NAME(), __LINE__));
            }
        }
        if (t_vpgrp_act_svlan_db->evm_eid != _BCM_ROBO_INVALID_ID){
            temp_vm_eid = t_vpgrp_act_svlan_db->evm_eid;
            rv = bcm_field_entry_destroy(unit, temp_vm_eid);
            if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)){
                /* warning message only, no error return to finish all clear 
                 * process. */
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, Destroy EVM failed!\n"), 
                          FUNCTION_NAME(), __LINE__));
                rv = BCM_E_NONE;
            }
        }
        if (t_vpgrp_act_svlan_db->ivm_eid != _BCM_ROBO_INVALID_ID){
            temp_vm_eid = t_vpgrp_act_svlan_db->ivm_eid;
            rv = bcm_field_entry_destroy(unit, temp_vm_eid);
            if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)){
                /* warning message only, no error return to finish all clear 
                 * process. */
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, Destroy IVM failed!\n"), 
                          FUNCTION_NAME(), __LINE__));
                rv = BCM_E_NONE;
            }
        }
        
        t_vpgrp_act_svlan_db->down_flow_id = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->up_flow_id = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->evm_eid = _BCM_ROBO_INVALID_ID;
        t_vpgrp_act_svlan_db->ivm_eid = _BCM_ROBO_INVALID_ID;
    }
    
    if (_tb_vport_db[unit] != NULL){
        for (i = 0; i < _TB_SUBPORT_NUM_PORT; i++){
            for (j = 0; j < _TB_SUBPORT_NUM_VPORT_PER_PORT; j++){
                t_vport_db = _tb_vport_db[unit] + 
                        _BCM_PORT_VPORT_INDEX(i, j);
                
                if (t_vport_db->valid == FALSE){
                    continue;
                }
                
                if (t_vport_db->evm_eid != _BCM_ROBO_INVALID_ID){
                    temp_vm_eid = t_vport_db->evm_eid;
                    rv = bcm_field_entry_destroy(unit, temp_vm_eid);
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)){
                        /* warning message only, no error return to finish all
                         * clear process. */
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s,%d, Destroy EVM failed!\n"), 
                                  FUNCTION_NAME(), __LINE__));
                        rv = BCM_E_NONE;
                    }
                }
                if (t_vport_db->ivm_eid != _BCM_ROBO_INVALID_ID){
                    temp_vm_eid = t_vport_db->ivm_eid;
                    rv = bcm_field_entry_destroy(unit, temp_vm_eid);
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)){
                        /* warning message only, no error return to finish all
                         * clear process. */
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s,%d, Destroy EVM failed!\n"), 
                                  FUNCTION_NAME(), __LINE__));
                        rv = BCM_E_NONE;
                    }
                }
                
                t_vport_db->down_flow_id = _BCM_ROBO_INVALID_ID;
                t_vport_db->up_flow_id = _BCM_ROBO_INVALID_ID;
                t_vport_db->evm_eid = _BCM_ROBO_INVALID_ID;
                t_vport_db->ivm_eid = _BCM_ROBO_INVALID_ID;
                
            }
        }
    }
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    
    if (init_status){
        _SUBPORT_VP_UNLOCK(unit);
    }
    return rv;
#else  /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
}

#ifdef BCM_TB_SUPPORT
/*
 * Function:
 *      _bcm_tb_subport_group_count_get
 * Purpose:
 *      Get the created subport group number (system basis).
 * Parameters:
 *      unit    - Device Number
 *      count   - [Out] subport group count
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tb_subport_group_count_get(int unit, int *count){
    bcm_port_t      port;

    *count = 0;
    
    _TB_SUBPORT_CHECK_INIT(unit);
    
    if (SOC_IS_TBX(unit)){
        PBMP_ALL_ITER(unit, port) {
            if (_TB_SUBPORT_GROUP_IS_EMPTY(unit, port)){
                continue;
            } else {
                *count += _tb_vpgrp_count[(unit)][(port)];
            }
        }
    }
    return BCM_E_NONE;
  
}
#endif  /* BCM_TB_SUPPORT */

 /*
 * Function:
 *      bcm_subport_init
 * Purpose:
 *      Initialize the SUBPORT system
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_subport_init(int unit)
{
#ifdef BCM_TB_SUPPORT
    int i, j;
    drv_vpgrp_db_t  *temp_vpg_db;
#if _TB_SUBPORT_UC_STREAM_SUPPORT
    drv_vport_db_t  *temp_vport_db;    
    _drv_tb_mcrep_group_active_svlan_t  *t_vpgrp_act_svlan_db;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
#endif   /* BCM_TB_SUPPORT */

    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s, unit=%d!!\n"), FUNCTION_NAME(),unit));
    
    /* HW init section  */
    BCM_IF_ERROR_RETURN(_bcm_robo_subport_hw_clear(unit));
    
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        
        /* SW init section :
         *
         *  1. init for subport group sw database.
         *  2. init for subport vport sw database.
         *  3. init some other global variables.
         */

        /* 1. subport group SW init */
        if (_tb_vpgrp_db[unit] == NULL) {
            _tb_vpgrp_db[unit] = sal_alloc(_TB_SUBPORT_NUM_GROUP * 
                    sizeof(drv_vpgrp_db_t), "subport_group_db");
            if (_tb_vpgrp_db[unit] == NULL) {
                _bcm_tb_subport_free_resource(unit);
                return BCM_E_MEMORY;
            }
        }
        sal_memset(_tb_vpgrp_db[unit], 0, _TB_SUBPORT_NUM_GROUP * 
                sizeof(drv_vpgrp_db_t));
        for (i = 0; i < _TB_SUBPORT_NUM_PORT; i++){
            for (j = 0; j < _TB_SUBPORT_NUM_GROUP_PER_PORT; j++){
                temp_vpg_db = _tb_vpgrp_db[unit] + 
                        (i * _TB_SUBPORT_NUM_GROUP_PER_PORT + j);
                temp_vpg_db->vpgrp_id = BCM_GPORT_INVALID;
                BCM_GPORT_MODPORT_SET(temp_vpg_db->phy_port, unit, i);
            }
        }
        
#if _TB_SUBPORT_UC_STREAM_SUPPORT
        /* 2. subport vport SW init */
        if (_tb_vport_db[unit] == NULL) {
            _tb_vport_db[unit] = sal_alloc(_TB_SUBPORT_NUM_PORT * 
                    _TB_SUBPORT_NUM_VPORT_PER_PORT * sizeof(drv_vport_db_t),
                    "subport_vport_db");
            if (_tb_vport_db[unit] == NULL) {
                _bcm_tb_subport_free_resource(unit);
                return BCM_E_MEMORY;
            }
        }
        
        for (i = 0; i < _TB_SUBPORT_NUM_PORT; i++) {
            for(j = 0; j < _TB_SUBPORT_NUM_VPORT_PER_PORT; j++){
                temp_vport_db = _tb_vport_db[unit] + 
                        _BCM_PORT_VPORT_INDEX(i, j);
                temp_vport_db->valid = FALSE;
                temp_vport_db->cvid = _BCM_ROBO_INVALID_ID;
                temp_vport_db->svid = _BCM_ROBO_INVALID_ID;
                temp_vport_db->down_flow_id = _BCM_ROBO_INVALID_ID;
                temp_vport_db->up_flow_id = _BCM_ROBO_INVALID_ID;
                temp_vport_db->ivm_eid = _BCM_ROBO_INVALID_ID;
                temp_vport_db->evm_eid = _BCM_ROBO_INVALID_ID;
                temp_vport_db->int_pri = _BCM_ROBO_INVALID_ID;
                temp_vport_db->prop_flags = 0;
                temp_vport_db->int_flags = 0;
            }
        }
         
        /* other subport related SW init */
        _tb_vpgrp_ivm_group_id[unit] = _BCM_ROBO_INVALID_ID;
        _tb_vpgrp_evm_group_id[unit] = _BCM_ROBO_INVALID_ID;

        for (i = 0; i < _TB_SUBPORT_NUM_MCASTREP_GROUP; i++) {
            t_vpgrp_act_svlan_db = _tb_mcrep_vlan_info[unit] + i;
                
            t_vpgrp_act_svlan_db->valid = FALSE;
            t_vpgrp_act_svlan_db->svid = _BCM_ROBO_INVALID_ID;
            t_vpgrp_act_svlan_db->down_flow_id = _BCM_ROBO_INVALID_ID;
            t_vpgrp_act_svlan_db->up_flow_id = _BCM_ROBO_INVALID_ID;
            t_vpgrp_act_svlan_db->ivm_eid = _BCM_ROBO_INVALID_ID;
            t_vpgrp_act_svlan_db->evm_eid = _BCM_ROBO_INVALID_ID;
            t_vpgrp_act_svlan_db->prop_flags = 0;
        }
         
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
         
        /* other subport related SW init */
        for (i = 0; i < _TB_SUBPORT_NUM_PORT; i++) {
            _tb_vpgrp_count[unit][i] = 0;
        }
        
        if (NULL == _tb_subport_vp_mutex[unit]) {
            _tb_subport_vp_mutex[unit] = sal_mutex_create("subport vp mutex");
            if (_tb_subport_vp_mutex[unit] == NULL) {
                _bcm_tb_subport_free_resource(unit);
                return BCM_E_MEMORY;
            }
        }
    } else {
        return BCM_E_UNAVAIL;
    }
#endif   /* BCM_TB_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_subport_cleanup
 * Purpose:
 *      Cleanup the SUBPORT system
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_subport_cleanup(int unit)
{
#ifdef BCM_TB_SUPPORT
    int vpg_id = 0;
    int port = 0;
    bcm_gport_t gport = 0;
    int rv = BCM_E_NONE;
#endif

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s\n"), FUNCTION_NAME()));
    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }

    /* HW clearn up */
    BCM_IF_ERROR_RETURN(_bcm_robo_subport_hw_clear(unit));

#ifdef BCM_TB_SUPPORT
    /* Call bcm_robo_subport_group_destroy() repeatly to delete the configured multicast replication in both sw and hw*/
    for(vpg_id = 0; vpg_id < _TB_SUBPORT_NUM_MCASTREP_GROUP; vpg_id++) {
        for (port = 0; port < _TB_SUBPORT_NUM_PORT; port++) {
            BCM_GPORT_SUBPORT_GROUP_SET(gport, 
                    _TB_SUBPORT_SYSTEM_GROUP_ID_GET(vpg_id, port));
    
            rv = bcm_subport_group_destroy(unit, gport);
            if (rv == BCM_E_NOT_FOUND) {
                /* 
                  * The pair (mc index, port) is not created, 
                  * skip return value checking and destroy the next pair.
                  */
                continue;
            } else {
                BCM_IF_ERROR_RETURN(rv);
            }
        }
    }

    if (NULL == _tb_vpgrp_db[unit]) {
        return (BCM_E_NONE);
    }

    /* SW clearn up */
    _bcm_tb_subport_free_resource(unit);
#endif   /* BCM_TB_SUPPORT */
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s, Done!!\n"), FUNCTION_NAME()));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_subport_group_create
 * Purpose:
 *      Create a subport group
 * Parameters:
 *      unit   - (IN) Device Number
 *      config - (IN) Subport group config information
 *      group  - (IN) GPORT with port basis subport group indentifier
 *               (OUT) GPORT (global port) identifier
 * Returns:
 *      BCM_E_XXX
 *
 *  Note : 
 *  1. For Thunderbolt, the group for input will be 0-255 when the 
 *      config.flag is assigned as BCM_SUBPORT_GROUP_WITH_ID.
 *      (This is for the Thunderbolt's subport group id has boundled with 
 *      physical port id already. To accept the 0-255 port basis group id can
 *      avoid the conflict issue with user provided config.port).
 *  2. The group for output is always constructed as a global group identifier
 *      that means this group_id can indicate a specific subport group.
 *  3. Per TB HW designing Spec., the proper configuration for Multiacst 
 *      replication must concerning that all physical ports in the same mcast 
 *      group(0-255) will be referenced in MARL entry through 
 *      Mcast_Group_index field.  Thus all those groupped physical ports will 
 *      be referenced for replication.  That means the proper configuration 
 *      in subport group creation is all bounded SVLAN of the same MCAST 
 *      replication group must be the same on between physical ports.
 *      
 */
int
bcm_robo_subport_group_create(int unit, bcm_subport_group_config_t *config,
                             bcm_gport_t *group)
{

#ifdef BCM_TB_SUPPORT
    bcm_module_t    mod_out, my_modid;
    bcm_port_t      port_out;
    bcm_trunk_t     trunk_id;
    drv_vpgrp_db_t  *temp_vpgrp=NULL;
    int             svlan, gport_id = 0, vpgrp_id = 0, vpgrp_mem_id = 0, i;
    int             rv = BCM_E_NONE, sys_grp_cnt = 0;
    bcm_vlan_vector_t vlan_vec_zero;
    bcm_vlan_vector_t vlan_vec;
#if _TB_SUBPORT_UC_STREAM_SUPPORT
    uint32          flags_chang = 0, active_flags = 0;
#endif

    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }

    _TB_SUBPORT_CHECK_INIT(unit);

    if ((config == NULL) || (group == NULL)) {
        return BCM_E_PARAM;
    }
    
    if (config->vlan > BCM_VLAN_MAX) {
        return BCM_E_PARAM;
    }

    if (SOC_GPORT_IS_TRUNK(config->port)) {
        /* Creating subport group over trunk is not supported */
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                             "Subport group over trunk is not supported!\n")));
        return BCM_E_PORT;
    }

    svlan = config->vlan;

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    /* check assigning prop_flag if any conflict occured : 
     *  - Error return if conflict.
     */
    rv = _bcm_tb_prop_flag_conflict_check(unit, 
            _BCM_TB_SUBPORT_FLAGS_CHECKOP_GROUP, config->prop_flags, 
            &flags_chang, &active_flags);
    if (rv != BCM_E_NONE){
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "The flag(0x%08x) for group creation is internal conflicted!\n"),
                  config->prop_flags));
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_PARAM;
    } else {
        if (flags_chang == TRUE) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s,The original flag=0x%08x reassigned to flags=0x%08x !\n"),
                      FUNCTION_NAME(), config->prop_flags, active_flags));
        }
    }
#endif /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    
    BCM_IF_ERROR_RETURN(_bcm_robo_gport_resolve(unit, config->port, &mod_out, 
                &port_out, &trunk_id, &gport_id));
    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &my_modid));

    /* check if the subport groups for this port is full already */
    if (_TB_SUBPORT_GROUP_IS_FULL(unit, port_out)){
        return BCM_E_FULL;
    }
    
    /* check system group count */
    BCM_IF_ERROR_RETURN(
            _bcm_tb_subport_group_count_get(unit, &sys_grp_cnt));
    
    _SUBPORT_VP_LOCK(unit);
    /* vp group created per user defined Group_ID */ 
    if (config->flags & BCM_SUBPORT_GROUP_WITH_ID) {
        
        /* check if user's group id is valid or existed */
        vpgrp_id = BCM_GPORT_SUBPORT_GROUP_GET(*group);
        if (vpgrp_id < 0 || vpgrp_id > (_TB_SUBPORT_NUM_GROUP_PER_PORT - 1)){

            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s, invalid support group id 0x%x\n"), 
                      FUNCTION_NAME(), *group));
            _SUBPORT_VP_UNLOCK(unit);
            return BCM_E_PARAM;
        } else {
            rv = DRV_MCREP_VPGRP_VPORT_CONFIG_GET(unit, vpgrp_id, port_out, 
                    DRV_MCREP_VPGRP_OP_ENTRY_ID, &vpgrp_mem_id);
            if (rv){
                _SUBPORT_VP_UNLOCK(unit);
                return rv;
            }
            
            temp_vpgrp = _tb_vpgrp_db[unit] + vpgrp_mem_id; 
            if (SUBPORT_GROUP_IS_VALID(temp_vpgrp)){
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "%s, Subport group %d on port %d is existed!!\n"), 
                          FUNCTION_NAME(), vpgrp_id, port_out));
                _SUBPORT_VP_UNLOCK(unit);
                return BCM_E_EXISTS;
            }
        }
            
    /* vp group created per system auto-detection */     
    } else {    
        BCM_VLAN_VEC_ZERO(vlan_vec_zero);
        BCM_VLAN_VEC_ZERO(vlan_vec);

        /* 
         * Find a free group id for this group create request.
         *  - Search will always started from id=0
         */
        vpgrp_id = 0;
        rv = DRV_MCREP_VPGRP_VPORT_CONFIG_GET(unit, vpgrp_id, port_out, 
                DRV_MCREP_VPGRP_OP_ENTRY_ID, &vpgrp_mem_id);
        if (BCM_FAILURE(rv)){
            _SUBPORT_VP_UNLOCK(unit);
            return rv;
        }

        for (i = 0; i < _TB_SUBPORT_NUM_GROUP_PER_PORT; i ++){
            /* Find an unsed (mc_index+port) entry */
            rv = bcm_multicast_repl_get(unit, i, port_out, vlan_vec);
            if (BCM_FAILURE(rv)){
                _SUBPORT_VP_UNLOCK(unit);
                return rv;
            }
            if (!sal_memcmp(vlan_vec, vlan_vec_zero, sizeof(bcm_vlan_vector_t))) {
                temp_vpgrp = _tb_vpgrp_db[unit] + (vpgrp_mem_id + i); 
                if (SUBPORT_GROUP_IS_FREE(temp_vpgrp)){
                    break;
                }
            }
        }
        
        if (i < _TB_SUBPORT_NUM_GROUP_PER_PORT){
            vpgrp_id = i;       /* get the port basis subport group id */
            vpgrp_mem_id += i;  /* get the system basis subport group id */
        } else {
            _SUBPORT_VP_UNLOCK(unit);
            return BCM_E_FULL;
        }
        
    }

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    /* check bounded SVLAN if any conflict occured between different physical 
     *  ports of the same mcast replication group.
     *
     *  This limitation applied for the better SW IVM/EVM handler design.
     *  (avoid unnecessary SW recovery effort of IVM/EVM configuration once 
     *  bounded svlan changed due to subport group destoried)
     */
    if (_tb_mcrep_vlan_info[unit][vpgrp_id].valid == TRUE){
        if (_tb_mcrep_vlan_info[unit][vpgrp_id].svid != svlan){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "The bounded SVLAN(%d) is conflicted with current "
                                 "bounding SVLAN(%d)\n"),
                      svlan, _tb_mcrep_vlan_info[unit][vpgrp_id].svid));
            _SUBPORT_VP_UNLOCK(unit);
            return BCM_E_CONFIG;
        }
    }
    
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
     
    /* create process : SW effort in ROBO Thunderbolt only
     *  - update the database for the created information.
     */
    /* vpgrp_id is kept in gport type */
    SUBPORT_GROUP_ID_SET(temp_vpgrp, vpgrp_id);
    temp_vpgrp->vlan = svlan;
    assert(temp_vpgrp->vp_bmp == 0);
    assert(temp_vpgrp->vp_cnt == 0);

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    temp_vpgrp->prop_flags = config->prop_flags;
    temp_vpgrp->int_flags = active_flags;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    _tb_vpgrp_count[unit][port_out] ++;
    
    /* assign the physical mem_id and construct the subport gport type for 
     *  telling the uper layer the true subport group id.
     */
    BCM_GPORT_SUBPORT_GROUP_SET(*group, vpgrp_mem_id);

#if _TB_SUBPORT_UC_STREAM_SUPPORT

    /* SW info handling */
    if (_tb_mcrep_vlan_info[unit][vpgrp_id].valid == FALSE){
        /* this group is never been configured, new IVM of downstream and EVM
         * of upstream for this subport group is required.
         */
        rv = _bcm_tb_subport_group_vm_handler(unit, 
                _BCM_TB_SUBPORT_VM_OP_CREATE, port_out, vpgrp_id, svlan);
        if (rv != BCM_E_NONE){
            /* Warning message only, no error return! Just to tell user that  
             *  the packet stream through IVM/EVM can't work correctly.
             */
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Configuration failed! Unicast stream not work. rv(%d)\n"),
                      rv));
            /* reasign rv to prevent the normal process been broken */
            rv = BCM_E_NONE;
        }
    }


#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    _SUBPORT_VP_UNLOCK(unit);
    return BCM_E_NONE;

#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *      bcm_subport_group_destroy
 * Purpose:
 *      Destroy a subport group
 * Parameters:
 *      unit  - (IN) Device Number
 *      group - (IN) GPORT (global port) identifier
 * Returns:
 *      BCM_E_XXX
 *  Note :
 *  1. group in this routine is the global subport group id.
 */
int
bcm_robo_subport_group_destroy(int unit, bcm_gport_t group)
{
    
#ifdef  BCM_TB_SUPPORT
    int             vpgrp_mem_id = 0, vpg_id = 0;
    int             rv = BCM_E_NONE, svlan = 0;
    int             vp_id = 0, scan_vpcnt = 0, temp_vpcnt = 0;
    uint32          temp_vpbmp = 0;
    bcm_port_t      port_in = 0;
    drv_vpgrp_db_t  *temp_vpgrp;
    bcm_gport_t     gport = SOC_GPORT_INVALID;
#if _TB_SUBPORT_UC_STREAM_SUPPORT
    int             valid_cnt = 0, i;
    bcm_subport_group_config_t  *t_vpg_cfg;
    _drv_tb_mcrep_group_active_svlan_t  *t_mcrep_act_svlan;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */


    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }
    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }

    _TB_SUBPORT_CHECK_INIT(unit);
    
    /* check if user's group id is valid */
    vpgrp_mem_id = BCM_GPORT_SUBPORT_GROUP_GET(group);
    if (vpgrp_mem_id < 0 || vpgrp_mem_id > _TB_MAX_SUPORT_GROUP_ID){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, invalid support group id 0x%x\n"), 
                  FUNCTION_NAME(), group));
        return BCM_E_PARAM;
    }

    /* check if this the user's group is existed */
    temp_vpgrp = _tb_vpgrp_db[unit] + vpgrp_mem_id;
    if (!SUBPORT_GROUP_IS_VALID(temp_vpgrp)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, not existed support group id 0x%x\n"), 
                  FUNCTION_NAME(), group));
        return BCM_E_NOT_FOUND;
    }

    port_in = _TB_SUBPORT_GROUP_ID_2PORT_ID(vpgrp_mem_id);
    vpg_id = _TB_SUBPORT_GROUP_ID_2PGROUP_ID(vpgrp_mem_id);
    temp_vpbmp = temp_vpgrp->vp_bmp;
    svlan = temp_vpgrp->vlan;
    
    _SUBPORT_VP_LOCK(unit);
        
    /* remove all existed subport in this group */
    if (temp_vpgrp->vp_cnt > 0) {
        scan_vpcnt = 0;
        temp_vpcnt = temp_vpgrp->vp_cnt;
        for (vp_id = 0; vp_id < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vp_id++){
            if (VPBMP_BITGET(temp_vpbmp, vp_id)){
                scan_vpcnt++;
                BCM_GPORT_SUBPORT_PORT_SET(gport, 
                        ((vpgrp_mem_id << _TB_SUBPORT_SYSTEM_ID_SHIFT_VPGRP) | 
                        ((vp_id)&_TB_SUBPORT_SYSTEM_ID_MASK_VPID)));
                if (bcm_subport_port_delete(unit, gport) != BCM_E_NONE){
                    LOG_INFO(BSL_LS_BCM_PORT,
                             (BSL_META_U(unit,
                                         "%s, Failed on remove associated vp %d\n"),
                              FUNCTION_NAME(), vp_id));
                    _SUBPORT_VP_UNLOCK(unit);
                    return BCM_E_INTERNAL;                         
                }
            }
            /* check if all valid vp is scaned already */
            if (scan_vpcnt >= temp_vpcnt) {
                break;
            }
        }
    }
    
    temp_vpgrp = _tb_vpgrp_db[unit] + vpgrp_mem_id;
    temp_vpgrp->vpgrp_id = BCM_GPORT_INVALID;
    temp_vpgrp->vlan = 0;
    temp_vpgrp->vp_bmp = 0;
    temp_vpgrp->vp_cnt = 0;
    
    _tb_vpgrp_count[unit][port_in]--;

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    
    /* SW info handler */
    t_mcrep_act_svlan = _tb_mcrep_vlan_info[unit] + vpg_id;

    /* checking svlan conflict status(exclude this port's mcrep_group */
    valid_cnt = 0;
    t_vpg_cfg = sal_alloc(sizeof(bcm_subport_group_config_t), 
            "bcm_subport_group_config_t");
    for (i = 0; i < _TB_SUBPORT_NUM_PORT; i++){
        if (i != port_in){
            BCM_GPORT_SUBPORT_GROUP_SET(gport, 
                    _TB_SUBPORT_SYSTEM_GROUP_ID_GET(vpg_id, i));
    
            rv = bcm_subport_group_get(unit, gport, t_vpg_cfg);
            if (rv == BCM_E_NONE){
                valid_cnt++;     /* count the existed group on other port */
            } else if (rv == BCM_E_NOT_FOUND) {
                continue;
            } else {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,Internal problem on SW inforamtion.\n"),
                          FUNCTION_NAME(), __LINE__));
                break;
            }
        }
    }
    sal_free(t_vpg_cfg);
    
    if (t_mcrep_act_svlan->valid == FALSE) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Internal problem on SW inforamtion.\n")));
    } else {
        if (valid_cnt == 0){     
            /* means this group is not existed on all other ports.
             * -  removed related IVM/EVM configuration and SW information. 
             */
            rv = _bcm_tb_subport_group_vm_handler(unit, 
                    _BCM_TB_SUBPORT_VM_OP_REMOVE, port_in, vpg_id, svlan);
            if (rv != BCM_E_NONE){
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "Configuration failed internally!rv(%d)\n"),
                          rv));
                
                _SUBPORT_VP_UNLOCK(unit);
                return rv;
            }
        }
    }
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    
    _SUBPORT_VP_UNLOCK(unit);
    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *      bcm_subport_group_get
 * Purpose:
 *      Get a subport group
 * Parameters:
 *      unit   - (IN) Device Number
 *      group  - (IN) GPORT (global port) identifier
 *      config - (OUT) Subport group config information
 * Returns:
 *      BCM_E_XXX
 *  Note :
 *  1. group in this routine is the global subport group id.
 */
int
bcm_robo_subport_group_get(int unit, bcm_gport_t group,
                          bcm_subport_group_config_t *config)
{
#ifdef  BCM_TB_SUPPORT
    int             vpgrp_mem_id;
    bcm_port_t      port_in, port_out;
    drv_vpgrp_db_t  *temp_vpgrp;
    bcm_module_t    mod_out;
    
    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }
    
    _TB_SUBPORT_CHECK_INIT(unit);
    
    if (config == NULL){
        return BCM_E_PARAM;
    }

    /* check if user's group id is valid */
    vpgrp_mem_id = BCM_GPORT_SUBPORT_GROUP_GET(group);
    if (vpgrp_mem_id < 0 || vpgrp_mem_id > _TB_MAX_SUPORT_GROUP_ID){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, invalid support group id 0x%x\n"), 
                  FUNCTION_NAME(), group));
        return BCM_E_PARAM;
    }

    /* check if this the user's group is existed */
    temp_vpgrp = _tb_vpgrp_db[unit] + vpgrp_mem_id;
    if (!SUBPORT_GROUP_IS_VALID(temp_vpgrp)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, not existed support group id 0x%x\n"), 
                  FUNCTION_NAME(), group));
        return BCM_E_NOT_FOUND;
    }

    port_in = _TB_SUBPORT_GROUP_ID_2PORT_ID(vpgrp_mem_id);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s, GPORT=0x%08x:vp_group=%d,port=%d\n"), 
              FUNCTION_NAME(), group, vpgrp_mem_id, port_in));
    BCM_IF_ERROR_RETURN(bcm_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                                   unit, port_in,
                                                   &mod_out, &port_out));
    BCM_GPORT_MODPORT_SET(config->port, mod_out, port_out);
    assert(temp_vpgrp->phy_port == config->port);
    
    config->vlan = temp_vpgrp->vlan;
    config->if_class = _BCM_ROBO_INVALID_ID;  /* unavailable for Thunderbolt */

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    config->prop_flags = temp_vpgrp->prop_flags;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *      bcm_subport_port_add
 * Purpose:
 *      Add a subport to a subport group
 * Parameters:
 *      unit   - (IN) Device Number
 *      config - (IN) Subport config information
 *      port   - (OUT) GPORT (global port) identifier
 * Returns:
 *      BCM_E_XXX
 *
 *  Note :
 *  1. the valid config.group must constructed with suport group type and 
 *      device basis subport group id( range between 0x0-0x1CFF).
 *  2. Subport feature in TB is known as vPort. This API need to handle the 
 *      configuration about vPort for Mulicast and Unicast solution.
 *      - Mulicast : Downstream solution for S-VLAN to a physical port with 
 *          replicated with different C-VLAN for diefferent services.
 *      - Unicast : Both Downstream and upstream configuration need to be 
 *          included in this API.
 *        >> TR101 about vport Spec. : TR101 page 28
 *              - 2.5.1.3 VLAN Tagged UNI Single VC ATM or Native Ethernet 
 *                  Architecture. (Residential N:1 case)
 *      - Service the C-Tag(N) to S-Tag(1) solution.
 *                  
 */
int
bcm_robo_subport_port_add(int unit, bcm_subport_config_t *config,
                         bcm_gport_t *port)
{
#ifdef  BCM_TB_SUPPORT
    bcm_port_t      port_out;
    drv_vpgrp_db_t  *temp_vpgrp = NULL;
    uint32          vport = _TB_SUBPORT_NUM_VPORT_PER_GROUP;
    bcm_vlan_vector_t vlan_vec;
    int repl_vid = 0;
    int             vpgrp_id = 0, vpgrp_mem_id = 0;
    int             cvlan = 0;

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    int             rv = BCM_E_NONE;
    uint32          flags_chang = 0, active_flags = 0;
    drv_vport_db_t  *temp_vport_db;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }
    
    _TB_SUBPORT_CHECK_INIT(unit);

    if ((config == NULL) || (port == NULL)){
        return BCM_E_PARAM;
    }
    
    /* valid check : vid is valid at 0-4095 */
    if (config->pkt_vlan > BCM_VLAN_MAX) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "CVLAN(%d) out of range\n"), config->pkt_vlan));
        return BCM_E_PARAM;
    }

    /* search if this vid is used for a specific subport on this port */
    cvlan = (int)config->pkt_vlan;
    
#if _TB_SUBPORT_UC_STREAM_SUPPORT
    /* check assigning prop_flag if any conflict occured : 
     *  - Error return if conflict.
     */
    rv = _bcm_tb_prop_flag_conflict_check(unit, 
            _BCM_TB_SUBPORT_FLAGS_CHECKOP_VPORT, config->prop_flags, 
            &flags_chang, &active_flags);
    if (rv != BCM_E_NONE){
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "The flag(0x%08x) for group creation is internal conflicted!\n"),
                  config->prop_flags));
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_PARAM;
    } else {
        if (flags_chang == TRUE) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s,The original flag=0x%08x reassigned to flags=0x%08x !\n"),
                      FUNCTION_NAME(), config->prop_flags, active_flags));
        }
    }
    
    /* valid check : int_pri  */
    if (active_flags & _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED){
        if ((config->int_pri < 0) || 
                (config->int_pri > _BCM_TB_SUBPORT_MAX_INT_PRIROTY)){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "The int_pri0=0x%02x is invalid!\n"),
                      config->int_pri));
            return BCM_E_PARAM;
        }
    } else {
        /* int_pri is used for _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP flag :
         *  - check normal priority value.
         *
         * P.S.
         *  - user incomming PCP flags will be checked here.
         */
        if (config->int_pri < 0 || config->int_pri > 7){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "The int_pri0=%d is invalid!\n"),
                      config->int_pri));
            return BCM_E_PARAM;
        }
    }    
    
    /* cvid conflict check with prop_flags : 
     *  - Single inner Tag : CVID will be vaild in 0~4095, no conflict if 
     *      cvid is at valid range.
     *  - Priority Tag : CVID must be zero else conflict occured.
     *  - Untag : CVID will be ignored.
     */ 
    if (active_flags & _BCM_SUBPORT_FLAG_UP_ING_PRITAG){
        if (cvlan != 0){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Assigning VP as pri-tagged but cvid is not zero!\n")));
            return BCM_E_PARAM;
        }
    } else if (active_flags & _BCM_SUBPORT_FLAG_UP_ING_UNTAG){
        cvlan = _TB_SUBPORT_UNTAG_VID;
        config->pkt_vlan = 0;   /* reassign vid=0 if untag type indicated */
    }
#endif /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    
    vpgrp_mem_id = BCM_GPORT_SUBPORT_GROUP_GET(config->group);
    if (vpgrp_mem_id < 0 || vpgrp_mem_id > _TB_MAX_SUPORT_GROUP_ID){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, invalid support group id 0x%x\n"), 
                  FUNCTION_NAME(), vpgrp_mem_id));
        return BCM_E_PARAM;
    }

    _SUBPORT_VP_LOCK(unit);

    /* check if this the user's group is existed */    
    temp_vpgrp = _tb_vpgrp_db[unit] + vpgrp_mem_id;
    
    if (!SUBPORT_GROUP_IS_VALID(temp_vpgrp)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                             "%s, Subport group %d doesn't exist\n"), 
                   FUNCTION_NAME(), config->group));
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
                              
    }

    vpgrp_id = _TB_SUBPORT_GROUP_ID_2PGROUP_ID(vpgrp_mem_id);
    port_out = _TB_SUBPORT_GROUP_ID_2PORT_ID(vpgrp_mem_id);
    BCM_VLAN_VEC_ZERO(vlan_vec);
    rv = bcm_multicast_repl_get(unit, vpgrp_id, port_out , vlan_vec);
    if (BCM_FAILURE(rv)){
        _SUBPORT_VP_UNLOCK(unit);
        return rv;
    }
    if (cvlan == _TB_SUBPORT_UNTAG_VID){
        repl_vid = BCM_VLAN_UNTAG;
    } else {
        repl_vid = cvlan;
    }
            
    BCM_VLAN_VEC_SET(vlan_vec, repl_vid);
    rv = bcm_multicast_repl_set(unit, vpgrp_id, port_out , vlan_vec);
    if (BCM_FAILURE(rv)){
        if (rv == BCM_E_RESOURCE) {
            /* Change the return value to compatible with Subport API's request */
            rv = BCM_E_FULL;
        }
        _SUBPORT_VP_UNLOCK(unit);
        return rv;
    }
            
    /* Get vport id by vlan id*/
    rv = DRV_MCREP_VPORT_VID_SEARCH(unit, port_out, &vport, &repl_vid);
    if (BCM_FAILURE(rv)){
        _SUBPORT_VP_UNLOCK(unit);
        return rv;
    }

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    /* VPort-ID has been decided already. 
     *  - Applying a limitation on this VPort about the SVLAN conflict if this 
     *      VPort is existed in other subport group already.
     *  - This limitation is set to avoid the real operation conflict issue of
     *      case about :
     *    [Error Case] : 
     *          - A VPort(CVLAN=100) of port_1 existed vp_group1(with SVLAN 5) 
     *              and now this VPort is added to vp_group2(with SLVAN 10).
     *          - Than a conflict forwarding problem occured when upstream  
     *              from this VPort(ingress frame from port_1 with CVID=100).
     *              >> The egress frame should be STag with VID at 5 or 10 ?
     */
    assert(_tb_vport_db[unit] != NULL);
    temp_vport_db = _tb_vport_db[unit] + 
            _BCM_PORT_VPORT_INDEX(port_out, vport);

    /* SVLAN conflict check if VPort is member of other vp_group already */
    if (temp_vport_db->valid == TRUE){
        if (temp_vport_db->svid != temp_vpgrp->vlan){
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "The vport with CVID=%d has been existed in other group "
                                 " and SVLAN conflicted occured.\n"
                                 " - This SVLAN is vid=%d and existed SVLAN is vid=%d\n"),
                      cvlan, temp_vpgrp->vlan, temp_vport_db->svid));
            _SUBPORT_VP_UNLOCK(unit);
            return BCM_E_CONFIG;
        }
    } else {
        /* assign vp's sw database for new VPORT creation */
        temp_vport_db->int_pri = config->int_pri;
        temp_vport_db->prop_flags = config->prop_flags;
        temp_vport_db->int_flags = active_flags;
    }
    
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    
    VPBMP_BITSET(temp_vpgrp->vp_bmp, vport);
    temp_vpgrp->vp_cnt ++;
    
    /* =============== Unicast forwarding Design ================= */
    /*  Designing logical :  downstream(1->N VLAN); upstream(N->1 VLAN) 
     *  1. For downstream : assummed double tagged for subport service, the 
     *      C-VID is used to indicate which subport will be when egress pkt.
     *      ### IVM_Key: S-Tagged + C-Tagged + SP-VID + C-VID 
     *          IVM_Action: CPCP_Marking=b'11 + VID=SP-VID + Flow_ID + VP-ID + 
                            VP-CPCP
     *      ### EVM_Key: Flow_ID + Out-Port
     *      ### EVM_Action: S-Tag(remove),C-Tag(AsRx)
     *      a. group->vlan = service VID
     *          - the leaning/forwarding VID in this subport group
     *              (group is boundled in a physical egress port)
     *          - Will be removed when egress to subport.
     *      b. vport->pkt_vlan = subport VID
     *          - the existed VLAN tag when egress to subport.
     *      c. vport->int_pri = outgoing pkt priority.
     *      d. egress port can find a valid subport group.
     *      
     *      New Design: may support signle tag(C-Tag for TB allowed, S-Tag in 
     *                  TB can't approach this spec.) downstream to subport.
     *          - This case will be done and BCM API compatibale with ESW.
     *              (using int_pri to indicate the subport but only 8 vports
     *               allowed in such case.)
     *          ### IVM_Key: C-Tagged + C-VID + CPCP
     *              IVM_Action: CPCP_Marking=b'11 + VID=C-VID + Flow-ID + 
     *                          VP-ID + VP-CPCP
     *          ### EVM_KEY : Flow-ID + Out-Port
     *          ### EVM_Action: S-Tag(remove), C-Tag(subportVID)
     *      
     *  2. For upstream : assummed single tagged(C-Tag) from subport.
     *      ### IVM_Key: C-Tagged + C-VID + In-Port
     *          IVM_Action: SPCP-Marking=b'11 + VID=S-VID + VP-ID + VP_SPCP + 
     *                      Flow-ID
     *      ### EVM_Key: In-Port + Flow-ID
     *      ### EVM_Action: S-Tag(S-VID) + C-Tag(AsRx)
     *      a. vport->pkt_vlan = subport VID
     *          - must be changed to S-VID
     *      b. vport->int_pri = outgoing S-Tag pri.
     *
     */
#if _TB_SUBPORT_UC_STREAM_SUPPORT

    rv = _bcm_tb_subport_vport_vm_handler(unit, 
            _BCM_TB_SUBPORT_VM_OP_CREATE, port_out, vport, vpgrp_id, cvlan);
    if (rv != BCM_E_NONE){
        /* warning message only but no error return
         *  >> the downstream for Mcast replication to this vport still work.
         */
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Vport added but unicast stream forwarding may failed! rv(%d)"), 
                  rv));
    }
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    _SUBPORT_VP_UNLOCK(unit);
    
    /* return the SW designed system basis subport ID */
    BCM_GPORT_SUBPORT_PORT_SET(*port, 
            _TB_SUBPORT_SYSTEM_ID_GET(vpgrp_mem_id, vport));
            
    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *      bcm_subport_port_delete
 * Purpose:
 *      Delete a subport 
 * Parameters:
 *      unit   - (IN) Device Number
 *      port   - (IN) GPORT (global port) identifier
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_subport_port_delete(int unit, bcm_gport_t port)
{
#ifdef  BCM_TB_SUPPORT
    int             vpgrp_mem_id = 0, vpgrp_id = 0, vp_id = 0;
    int             scan_cnt, i;
    uint32          ut_vpid, vport = _TB_SUBPORT_NUM_VPORT_PER_GROUP;
    bcm_port_t      port_out;
    drv_vpgrp_db_t  *temp_vpgrp;
    uint32 vid = 0;
    bcm_vlan_vector_t vlan_vec;
    int             rv = BCM_E_NONE;

    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }
    _TB_SUBPORT_CHECK_INIT(unit);
    
    /* check if the user port_id(system basis subport id) is valid */
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "%s, port(GPORT)=0x%x\n"),FUNCTION_NAME(),port));
    if (!BCM_GPORT_IS_SUBPORT_PORT(port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, port 0x%x is not at subport type.\n"), 
                  FUNCTION_NAME(), port));
        return BCM_E_PORT;
    }

    /* check if this subport id is valid */
    vp_id = BCM_GPORT_SUBPORT_PORT_GET(port);
    if (vp_id < 0 || vp_id > _TB_SUBPORT_SYSTEM_ID_MAX){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, port 0x%x is in the valid range\n"), 
                  FUNCTION_NAME(), port));
        return BCM_E_PARAM;
    }
    
    /* retrieve the port basis vport id and the represented group id */
    vport = _TB_SUBPORT_SYSTEM_ID_2VPORT(vp_id);
    vpgrp_mem_id = _TB_SUBPORT_SYSTEM_ID_2VPG_ID(vp_id);

    /* check if this subport id existed in a existed subport group */
    _SUBPORT_VP_LOCK(unit);
    temp_vpgrp = _tb_vpgrp_db[unit] + vpgrp_mem_id;
    if (!SUBPORT_GROUP_IS_VALID(temp_vpgrp)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, not existed support group id 0x%x\n"), 
                  FUNCTION_NAME(), vpgrp_mem_id));
                
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }
    
    /* check if this subport id is existed in this group */
    if (!VPBMP_BITGET(temp_vpgrp->vp_bmp, vport)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s,subport%d is not existed in this group\n"), 
                  FUNCTION_NAME(), vport));
                
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    /* retrieve the port id from GROUP ID */
    port_out = _TB_SUBPORT_GROUP_ID_2PORT_ID(vpgrp_mem_id);

    /* retreive the untag VPORT id */
    ut_vpid = _TB_SUBPORT_NUM_VPORT_PER_GROUP;
    rv = DRV_MCREP_VPORT_CONFIG_GET(unit, port_out, 
                DRV_MCREP_VPORT_OP_UNTAG_VP, &ut_vpid, &vid);
    if (BCM_FAILURE(rv)){
        _SUBPORT_VP_UNLOCK(unit);
        return rv;
    }
    
    /* delete process : 
     *  1. group vport delete.
     *      - HW, SW updated.
     *  2. check if this vport still been member of other group in this port.
     *      1) yes, do nothing on port basis subport SW info.
     *      2) no, remove this vport from port basis configuration in HW/SW.
     */
    rv = DRV_MCREP_VPORT_CONFIG_GET(unit, port_out, 
                DRV_MCREP_VPORT_OP_VID, &vport, &vid);
    if (BCM_FAILURE(rv)){
        _SUBPORT_VP_UNLOCK(unit);
        return rv;
    }

    vpgrp_id = _TB_SUBPORT_GROUP_ID_2PGROUP_ID(vpgrp_mem_id);
    BCM_VLAN_VEC_ZERO(vlan_vec);
    rv = bcm_multicast_repl_get(unit, vpgrp_id, port_out , vlan_vec);
    if (BCM_FAILURE(rv)){
        _SUBPORT_VP_UNLOCK(unit);
        return rv;
    }
    BCM_VLAN_VEC_CLR(vlan_vec, vid);
    rv = bcm_multicast_repl_set(unit, vpgrp_id, port_out , vlan_vec);
    if (BCM_FAILURE(rv)){
        _SUBPORT_VP_UNLOCK(unit);
        return rv;
    }
    
    VPBMP_BITCLR(temp_vpgrp->vp_bmp, vport);
    temp_vpgrp->vp_cnt --;
    
    scan_cnt = 0;
    /* move to the first group of this port for the vport scaning. 
     *  (to check if any other group references this vport still)
     */
    temp_vpgrp = _tb_vpgrp_db[unit] + 
            (port_out << _TB_SUPORT_GROUP_ID_SHIFT_PORT);
    for (i = 0;  i < _TB_SUBPORT_NUM_GROUP_PER_PORT; i++){
        /* scan the valid group only */
        if (SUBPORT_GROUP_IS_VALID(temp_vpgrp)){
            scan_cnt++;
            
            if (i != vpgrp_id) {
                if (VPBMP_BITGET(temp_vpgrp->vp_bmp, vport)){
                    break;  /* break for this vport is still be referenced */
                } 
            }
            
            if (scan_cnt >= _tb_vpgrp_count[unit][port_out]){
                /* scan process finished */
                i = _TB_SUBPORT_NUM_GROUP_PER_PORT;
                break;  /* for all valid groups were scaned */
            }
        }
        temp_vpgrp++;
    }
    
    /* check if this vport is not referenced of this port and the groups */
    if (i == _TB_SUBPORT_NUM_GROUP_PER_PORT){    
        /* HW/SW updated for this vport is not referneced */
        if (DRV_MCREP_VPORT_CONFIG_SET(unit, port_out, 
                DRV_MCREP_VPORT_OP_VID, vport, 0)){
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "%s,Failed on deleting vport=%d on port%d \n"),
                      FUNCTION_NAME(), vport, port_out));
                    
            _SUBPORT_VP_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }

        /* reset VPort's untag bitmap due to this vp is the untag vport of 
         * this port.
         *  - call reset for only one untag vport is allowed of a port
         */
        if (vport == ut_vpid){
            if (DRV_MCREP_VPORT_CONFIG_SET(unit, port_out, 
                    DRV_MCREP_VPORT_OP_UNTAG_RESET, vport, 0)){
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "%s,Failed on this untag vport=%d of port%d \n"),
                          FUNCTION_NAME(), vport, port_out));
                        
                _SUBPORT_VP_UNLOCK(unit);
                return BCM_E_INTERNAL;
            }
        }
        
        /* =============== Unicast forwarding Design ================= */
        /*  Remove the created IVM/EVM for downstream and upstream for N:1  
         *  VLAN condition.
         */
#if _TB_SUBPORT_UC_STREAM_SUPPORT

        /* this vport is not referenced, delete the IVM/EVM which bounded with
         *  this vport.
         */
        rv = _bcm_tb_subport_vport_vm_handler(unit, 
                _BCM_TB_SUBPORT_VM_OP_REMOVE, port_out, vport, vpgrp_id, 0);
        if (rv != BCM_E_NONE){
            /* warning message only but no error return
             *  >> the downstream for Mcast replication to this vport still 
             *      work properly.
             */
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Vport added but unicast stream will be failed! rv(%d)"), 
                      rv));
        }
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    }

    
    _SUBPORT_VP_UNLOCK(unit);
    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *      bcm_subport_port_get
 * Purpose:
 *      Get a subport
 * Parameters:
 *      unit   - (IN) Device Number
 *      port   - (IN) GPORT (global port) identifier
 *      config - (OUT) Subport config information
 * Returns:
 *      BCM_E_XXX
 * Note :
 *  1. The config.int_pri is not used in ROBO device(TB currently).
 */
int
bcm_robo_subport_port_get(int unit, bcm_gport_t port,
                         bcm_subport_config_t *config)
{
#ifdef  BCM_TB_SUPPORT
    int             vpgrp_mem_id, vp_id;
    bcm_port_t      port_out = 0;
#if _TB_SUBPORT_UC_STREAM_SUPPORT
    drv_vport_db_t  *temp_vport_db;
#endif  /* _TB_SUBPORT_UC_STREAM_SUPPORT */
    uint32          vport, vid;
    drv_vpgrp_db_t  *temp_vpgrp;
    
    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }
    _TB_SUBPORT_CHECK_INIT(unit);

    if (config == NULL) {
        return BCM_E_PARAM;
    }

    /* check if the user port_id(system basis subport id) is valid */
    if (!BCM_GPORT_IS_SUBPORT_PORT(port)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, port 0x%x is not at subport type.\n"), 
                  FUNCTION_NAME(), port));
        return BCM_E_PARAM;
    }
    
    /* check if this subport id is valid */
    vp_id = BCM_GPORT_SUBPORT_PORT_GET(port);
    if (vp_id < 0 || vp_id > _TB_SUBPORT_SYSTEM_ID_MAX){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, port 0x%x is in the valid range\n"), 
                  FUNCTION_NAME(), port));
        return BCM_E_PARAM;
    }
    
    /* retrieve the port basis vport id and the represented group id */
    vport = _TB_SUBPORT_SYSTEM_ID_2VPORT(vp_id);    /* 0-15 */
    vpgrp_mem_id = _TB_SUBPORT_SYSTEM_ID_2VPG_ID(vp_id);
    
    /* check if this suport id existed in a existed subport group */
    _SUBPORT_VP_LOCK(unit);

    temp_vpgrp = _tb_vpgrp_db[unit] + vpgrp_mem_id;
    if (!SUBPORT_GROUP_IS_VALID(temp_vpgrp)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s, not existed support group id 0x%x\n"), 
                  FUNCTION_NAME(), vpgrp_mem_id));
                
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }
    
    /* check if this suport id is added into this existed group */
    if (!VPBMP_BITGET(temp_vpgrp->vp_bmp, vport)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s,subport%d is not existed in this group\n"), 
                  FUNCTION_NAME(), vport));
                
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }
    
    /* return the subport information */
    port_out = _TB_SUBPORT_GROUP_ID_2PORT_ID(vpgrp_mem_id);

#if _TB_SUBPORT_UC_STREAM_SUPPORT
    assert(_tb_vport_db[unit] != NULL);
    temp_vport_db = _tb_vport_db[unit] + 
            _BCM_PORT_VPORT_INDEX(port_out, vport);

    config->int_pri = temp_vport_db->int_pri;
    config->prop_flags = temp_vport_db->prop_flags;
#endif /* _TB_SUBPORT_UC_STREAM_SUPPORT */

    /* special process for the untag vp */
    if (DRV_MCREP_VPORT_CONFIG_GET(unit, port_out, 
               DRV_MCREP_VPORT_OP_VID, &vport, &vid)){
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "%s,failed on get subport vid!\n"), 
                  FUNCTION_NAME()));
                  
        _SUBPORT_VP_UNLOCK(unit);
        return BCM_E_INTERNAL;
    }

    BCM_GPORT_SUBPORT_GROUP_SET(config->group, vpgrp_mem_id);
    config->pkt_vlan = vid;
    
    _SUBPORT_VP_UNLOCK(unit);
    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *      bcm_subport_port_traverse
 * Purpose:
 *      Traverse all valid subports and call the
 *      supplied callback routine.
 * Parameters:
 *      unit      - Device Number
 *      cb        - User callback function, called once per subport.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_subport_port_traverse(int unit,
                              bcm_subport_port_traverse_cb cb,
                              void *user_data)
{
#ifdef  BCM_TB_SUPPORT
    int             i, j, vp_id;
    drv_vpgrp_db_t  *temp_vpgrp;
    bcm_gport_t     gport;
    bcm_subport_config_t config;

    if (!soc_feature(unit, soc_feature_subport)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_TBX(unit)){
        return BCM_E_UNAVAIL;
    }
    _TB_SUBPORT_CHECK_INIT(unit);

    for (i = 0; i < _TB_SUBPORT_NUM_GROUP; i++){
        
        temp_vpgrp = _tb_vpgrp_db[unit] + i;
        /* find the valid group which construct valid vport member */ 
        if (SUBPORT_GROUP_IS_VALID(temp_vpgrp) && (temp_vpgrp->vp_cnt > 0)){
            
            for (j = 0; j < _TB_SUBPORT_NUM_VPORT_PER_GROUP; j++){
                
                /* find the existed vport in this group */
                if (VPBMP_BITGET(temp_vpgrp->vp_bmp, j)){
                    
                    /* assigning the device basis vport id to form a gport */
                    vp_id = _TB_SUBPORT_SYSTEM_ID_GET(i, j);
                    BCM_GPORT_SUBPORT_PORT_SET(gport, vp_id);
                    
                    if (bcm_subport_port_get(unit, gport, &config)){
                        LOG_INFO(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "[Debug]%s,failed on getting" 
                                             " subport(0x%x) configuration.\n"), 
                                  FUNCTION_NAME(), gport));
                        return BCM_E_INTERNAL;
                    }
                    cb(unit, gport, &config, user_data);
                }
            }
        }
    }

    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
    return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
}

