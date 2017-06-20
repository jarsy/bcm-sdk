/*
 * $Id: multicast.c,v 1.54 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    multicast.c
 * Purpose: Manages multicast functions
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/mcast.h>
#include <bcm/l2.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/multicast.h>
#include <bcm/subport.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/subport.h>
#include <bcm_int/robo/mcast.h>

#include <bcm_int/common/multicast.h>

STATIC int multicast_robo_initialized[BCM_MAX_NUM_UNITS];
#define MULTICAST_ROBO_INIT_CHECK(unit) \
    if (!multicast_robo_initialized[unit]) { \
        return BCM_E_INIT; \
    }

/* Special VID definition for untag Mcast Replication. */
#define _BCM_ROBO_UNTAG_VID     (BCM_VLAN_MAX + 1)

#ifdef  BCM_TB_SUPPORT

/* Replication list of (mc_index, port) pairs. */
static _bcm_robo_mcast_repl_t   *robo_mcast_repl_config[BCM_LOCAL_UNITS_MAX];
/* Replicated vlan(s) */
static _bcm_robo_mcast_repl_vlan_t *robo_mcast_repl_vid[BCM_LOCAL_UNITS_MAX];

/* vpbmp for TBX device is 16 bits length */
#define _TB_VPORTBMP_MASK   0xFFFF

/* to report the indiacted replication group for MC_index and PPort */
static _bcm_robo_mcast_repl_vport_id_t * 
_mcast_repl_vp_group_get(int unit, int mc_index, bcm_port_t port)
{
    _bcm_robo_mcast_repl_t *cfg;
    int offset;

    offset = ((port << _TB_MCAST_REPL_INDEX_PORT_OFFSET) |\
                (mc_index << _TB_MCAST_REPL_INDEX_MGID_OFFSET));
    cfg = robo_mcast_repl_config[unit];
   return  (&cfg->repl_index[offset]);
    
}

/* SW check if there is no Multicast Replication group is existed */
static int 
_mcast_repl_system_no_vpset(int unit)
{
    _bcm_robo_mcast_repl_t *cfg;
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
    int i, result, isno_vpset = TRUE;

    cfg = robo_mcast_repl_config[unit];
    /* scan all subport groups(index is binded with mc_index+pport) */
    for (i = 0; i < _MAX_REPL_CFG_INDEX_NUM; i++){
        repl_index = &cfg->repl_index[i];

        SHR_BITTEST_RANGE(repl_index->repl_vport_id, 0, 
                _TB_SUBPORT_NUM_VPORT_PER_GROUP,  result);
        /* result=0 means no bit is set */
        if (result != 0) {
                isno_vpset = FALSE;
                break;
        }
    }

    return isno_vpset;
}

static void
_mcast_repl_used_set(int unit, int mc_index, bcm_port_t port, int bit)
{
    _bcm_robo_mcast_repl_vport_id_t *repl_index;

    repl_index = _mcast_repl_vp_group_get(unit, mc_index, port);
    SHR_BITSET(repl_index->repl_vport_id, bit);
}


static void
_mcast_repl_used_clr(int unit, int mc_index, bcm_port_t port, int bit)
{
    _bcm_robo_mcast_repl_vport_id_t *repl_index;

    repl_index = _mcast_repl_vp_group_get(unit, mc_index, port);
    SHR_BITCLR(repl_index->repl_vport_id, bit);
}

static int
_mcast_repl_used_isset(int unit, int mc_index, bcm_port_t port, int bit)
{
    _bcm_robo_mcast_repl_vport_id_t *repl_index;

    repl_index = _mcast_repl_vp_group_get(unit, mc_index, port);
    return (SHR_BITGET(repl_index->repl_vport_id, bit));
}

static void
_mcast_repl_vid_set(int unit, bcm_port_t port, int vport_id, bcm_vlan_t vid)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count !=0) {
        /* Debug purpose */
        LOG_ERROR(BSL_LS_BCM_COMMON, \
                  (BSL_META_U(unit, \
                              "ERROR: replication vlan %d(port=%d, vport=%d) be overwritten\n"), \
                   repl_vid_idx->vid,port,vport_id));
    }

    repl_vid_idx->vid = vid;
}

static bcm_vlan_t
_mcast_repl_vid_get(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    return (repl_vid_idx->vid);
}

static void
_mcast_repl_vid_cnt_inc(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count == 0xffff) {
        /* Debug purpose */
        LOG_ERROR(BSL_LS_BCM_COMMON, \
                  (BSL_META_U(unit, \
                              "ERROR: replication vlan %d(port=%d, vport=%d), increase underflow\n"), \
                   repl_vid_idx->vid,port,vport_id));
    }

    repl_vid_idx->count++;
}

static void
_mcast_repl_vid_cnt_dec(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count == 0) {
        /* Debug purpose */
        LOG_ERROR(BSL_LS_BCM_COMMON, \
                  (BSL_META_U(unit, \
                              "ERROR: replication vlan %d(port=%d, vport=%d), decrease underflow\n"), \
                   repl_vid_idx->vid,port,vport_id));
    }

    repl_vid_idx->count--;

}

static int
_mcast_repl_vid_cnt_is_zero(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count == 0) {
        return 1;
    } else {
        return 0;
    }
}

static int
_mcast_repl_param_check(int unit, int mc_index, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    uint32 mcrep_num = 0;
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }

    rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
            &mcrep_num);
    BCM_IF_ERROR_RETURN(rv);
    if ((mc_index < 0) || (mc_index >= mcrep_num)) {
        return BCM_E_BADID;
    }

    if (!robo_mcast_repl_config[unit]) {
        return BCM_E_INIT;
    } 
    if (!robo_mcast_repl_vid[unit]) {
        return BCM_E_INIT;
    } 

    return rv;
}

#define MCAST_REPL_USED_SET(u, id, p, n) _mcast_repl_used_set(u, id, p, n)
#define MCAST_REPL_USED_CLR(u, id, p, n) _mcast_repl_used_clr(u, id, p, n)
#define MCAST_REPL_USED_ISSET(u, id, p, n) _mcast_repl_used_isset(u, id, p, n)

#define MCAST_REPL_VID_SET(u, p, vport, vid) _mcast_repl_vid_set(u, p, vport, vid)
#define MCAST_REPL_VID_GET(u, p, vport) _mcast_repl_vid_get(u, p, vport)
#define MCAST_REPL_VID_CNT_INC(u, p, vport) _mcast_repl_vid_cnt_inc(u, p, vport)
#define MCAST_REPL_VID_CNT_DEC(u, p, vport) _mcast_repl_vid_cnt_dec(u, p, vport)
#define MCAST_REPL_VID_CNT_IS_ZERO(u, p, vport) _mcast_repl_vid_cnt_is_zero(u, p, vport)

#endif  /* BCM_TB_SUPPORT */

#define ONE_MULTICAST_TYPE_IS_SET(_type) \
        (_shr_popcount((_type)) == 1)

#define MCAST_EGRESS_OP_ADD 1
#define MCAST_EGRESS_OP_DEL 2
#define MCAST_EGRESS_OP_CLR 3
#define MCAST_EGRESS_OP_GET 4
#define MCAST_EGRESS_OP_SET 5

/* _bcm_robo_multicast_egress_op()
 *  - internal function to configuring ROBO's MCAST_PBMP table 
 *
 *  Parmeter :
 *      op          : add/delete/clear/get/set
 *      mc_index    : the MCAST_PBMP table index
 *      op_value    : the value for the usage per op
 *
 *  Return :
 *
 *  Note :
 *  1. op_value must be verified before calling to this function.
 *  2. op_value for op at {add/delete} will be treate as port_id and thid id 
 *      must be validate again.
 *
 */
static int
_bcm_robo_multicast_egress_op(int unit,
                        uint32 op,
                        uint32 mc_index,
                        uint32 *op_value){

    int     rv = BCM_E_NONE;
    int     mem_cnt = 0;
    bcm_pbmp_t pbmp;
    bcm_port_t port = 0;
    marl_pbmp_entry_t  mcast_entry;

    /* pre-processing check ...*/
    /* support for the switch chip which has L2MC_PBMP table only */
    rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
    if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
        return BCM_E_UNAVAIL;
    }
    
    /* valid index check */
    if (mc_index >= mem_cnt) {
        return BCM_E_BADID;
    } else {
        /* check if this group is created */
        if (_bcm_robo_l2mc_id_check(unit, mc_index) == 0){
            return BCM_E_NOT_FOUND;
        }
    }

    /* valid op_value check */
    if (op_value == NULL){
        return BCM_E_INTERNAL;
    }

    /* retrieve the MCAST_PBMPm enry */
    sal_memset(&mcast_entry, 0, sizeof (marl_pbmp_entry_t));
    rv = DRV_MEM_READ(unit, DRV_MEM_MCAST, mc_index, 1, 
            (uint32 *)&mcast_entry);
    if (rv){
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s, failed on reading MCAST_PBMP entry%d!\n"),
                   FUNCTION_NAME(), mc_index));
        return rv;
    }
    
    BCM_PBMP_CLEAR(pbmp);
    rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MCAST, 
            DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)&mcast_entry, 
            (uint32 *)&pbmp);
    if (rv){
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s, Failed on field get of MCAST_PBMP entry!\n"),
                   FUNCTION_NAME()));
        return rv;
    }
    
    switch (op) {
    case MCAST_EGRESS_OP_ADD:
        port = (int)*op_value;
        if (!SOC_PORT_VALID(unit, port)) {
            rv = BCM_E_PARAM;
        } else {
            BCM_PBMP_PORT_ADD(pbmp, port);
        }
        break;
    case MCAST_EGRESS_OP_DEL:
        port = (int)*op_value;
        if (!SOC_PORT_VALID(unit, port)) {
            rv = BCM_E_PARAM;
        } else {
            BCM_PBMP_PORT_REMOVE(pbmp, port);
        }
        break;
    case MCAST_EGRESS_OP_CLR:
        BCM_PBMP_CLEAR(pbmp);
        break;
    case MCAST_EGRESS_OP_GET:
        sal_memset(op_value, 0, sizeof(bcm_pbmp_t));
        sal_memcpy(op_value, (uint32 *)&pbmp, sizeof(bcm_pbmp_t));

        return BCM_E_NONE;
        break;
    case MCAST_EGRESS_OP_SET:
        sal_memcpy((uint32 *)&pbmp, op_value, sizeof(bcm_pbmp_t));
        break;
    default:
        rv = BCM_E_INTERNAL;
    }

    if (rv == BCM_E_NONE) {
        rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MCAST, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&mcast_entry, (uint32 *)&pbmp);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s,%d, Failed on field set of MCAST_PBMP entry!\n"),
                       FUNCTION_NAME(), __LINE__));
        }
        rv = DRV_MEM_WRITE(unit, DRV_MEM_MCAST, mc_index, 1, 
                (uint32 *)&mcast_entry);
        
        if (rv){
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s, failed on writing MCAST_PBMP entry%d!\n"),
                       FUNCTION_NAME(), mc_index));
            return BCM_E_INTERNAL;
        }
    }
   
    return rv;
}

/* 
 * For the HW spec. the vport in TB will be bounded with a physical port and 
 *  mcast group. That means a new vport is created with known mc_group,
 *  phy_port and also CVID through subport API. The vport's VID in TB is only
 *  port basis. That is, a vp of a port in different group must with the same 
 *  CVID configuration.
 *
 */

/*
 * Function:
 *      bcm_multicast_create
 * Purpose:
 *      Allocate a multicast group index
 * Parameters:
 *      unit       - (IN)   Device Number
 *      flags      - (IN)   BCM_MULTICAST_*
 *      group      - (OUT)  Group ID
 * Returns:
 *      BCM_E_XXX
 * Note :
 */
int
bcm_robo_multicast_create(int unit,
                         uint32 flags,
                         bcm_multicast_t *group)
{
    int     type, mc_index = 0, rv = BCM_E_NONE;
    int     mem_cnt = 0, mcrep_num = 0;

    MULTICAST_ROBO_INIT_CHECK(unit);

    /* valid type check : return BCM_E_PARAM for 
     *  1. no type is indicated.
     *  2. one more type been indicated.
     */
    type = flags & BCM_MULTICAST_TYPE_MASK;
    if (!ONE_MULTICAST_TYPE_IS_SET(type)) {
        return BCM_E_PARAM;
    }

    if (flags & BCM_MULTICAST_WITH_ID){
        /* The designing flow here for ROBO is created in SW only for the L2MC 
         *  table in ROBO devices (like bcm53242/5348/TB) doestn't with the 
         *  same HW design of ESW to has valid bit in each L2MC table entry.
         */
        mc_index = _BCM_MULTICAST_ID_GET(*group);
    }

    /* Pre-proceeding TB specific feature for L2MC replication */
    if (SOC_IS_TBX(unit)) {
        rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                (uint32 *)&mcrep_num);
        if (rv){
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s, %d, device property get problem!\n"),
                       FUNCTION_NAME(), __LINE__));
            return BCM_E_INTERNAL;
        }
        assert(mcrep_num > 0);      /* assert for TB devices */
        
        if (flags & BCM_MULTICAST_WITH_ID){
        
            /* Reasign the type if user requests to create group but assigned
             *   the id which is reserved for multicast replication.
             */
            if (type & BCM_MULTICAST_TYPE_L2){
                if ((mc_index >= 0) && (mc_index < mcrep_num)){
                    type = BCM_MULTICAST_TYPE_VLAN ;
                } else {
                    type = BCM_MULTICAST_TYPE_L2;
                } 
            }
        }
    }
    
    if (type & BCM_MULTICAST_TYPE_L2){
        /* support for the switch chip which has L2MC_PBMP table only */
        rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
        if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
            return BCM_E_UNAVAIL;
        }
        
        /* user can request id 0 to l2mc_size without the limit to indicate 
         *  the Mcast Replication group seciton (0-255) for creat process.
         * Once user request SW to choose a free L2MC_ID for L2 group create.
         *  the SW design in the internal routine _bcm_robo_l2mc_free_index()
         *  will avoid index in the Mcast Replication seciton(0-255).
         */
        if (flags & BCM_MULTICAST_WITH_ID){
            if ((mc_index < 0) || (mc_index >= mem_cnt)) {
                return BCM_E_BADID;
            } else {
                /* check if ID is not free */
                if (_bcm_robo_l2mc_id_check(unit, mc_index)){
                    /* id is not free for group create */
                    return BCM_E_EXISTS;
                }
            }
            
            BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_id_alloc(unit, mc_index));
        } else {
            BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_free_index(
                    unit, _BCM_MULTICAST_TYPE_L2, &mc_index));
        } 
        _BCM_MULTICAST_GROUP_SET(
                *group, _BCM_MULTICAST_TYPE_L2, mc_index);
    } else if ((type & BCM_MULTICAST_TYPE_SUBPORT) || 
            (type & BCM_MULTICAST_TYPE_VLAN)){
        /* subport type is for backward compatiable. */
        if (!SOC_IS_TBX(unit)) {
            return BCM_E_UNAVAIL;
        }

        /* TB's Mcast Replication group_id is limited between 0-0x1cff.
         * For the HW specific design on TB, this group_id can be explained
         *  in detail as :
         *  - group_id = Port_id(bit12-bit8) | L2MC_ID(bit7-bit0)
         *
         *  Multicast deisgn for TBX's VLAN replication is changed to support 
         *  group type at BCM_MULTICAST_TYPE_VLAN. 
         *  - support BCM_MULTICAST_TYPE_SUBPORT as well for backward 
         *      compatible in group create/destroy.
         */
        if (!(soc_feature(unit, soc_feature_subport) || 
                soc_feature(unit, soc_feature_vlan_vp)) ) {
            return BCM_E_UNAVAIL;
        }
        
        if (flags & BCM_MULTICAST_WITH_ID){
            if ((mc_index < 0) ||(mc_index >= mcrep_num)) {
                return BCM_E_BADID;
            } else {
                /* check if ID is not free */
                if (_bcm_robo_l2mc_id_check(unit, mc_index)){
                    /* id is not fee for group create */
                    return BCM_E_EXISTS;
                }
            }
            BCM_IF_ERROR_RETURN(
                    _bcm_robo_l2mc_id_alloc(unit, mc_index));
        } else {
            BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_free_index(unit, 
                        _BCM_MULTICAST_TYPE_VLAN, &mc_index));
        }
        _BCM_MULTICAST_GROUP_SET(
                *group, _BCM_MULTICAST_TYPE_VLAN, mc_index);        
    } else { 
        return BCM_E_UNAVAIL;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_destroy
 * Purpose:
 *      Free a multicast group index
 * Parameters:
 *      unit       - (IN) Device Number
 *      group      - (IN) Group ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_destroy(int unit, bcm_multicast_t group)
{

    int type, op_type, mc_index, rv = BCM_E_NONE;
    int mem_cnt = 0;
    marl_pbmp_entry_t  mcast_entry;
    int mcrep_num =0;
    
    MULTICAST_ROBO_INIT_CHECK(unit);

    type = _BCM_MULTICAST_TYPE_GET(group);

    if (type == _BCM_MULTICAST_TYPE_L2) {
        op_type = BCM_MULTICAST_TYPE_L2;
    } else if ((type == _BCM_MULTICAST_TYPE_VLAN) || 
            (type == _BCM_MULTICAST_TYPE_SUBPORT)){
        /* subport type is for backward compatiable. */
        op_type = BCM_MULTICAST_TYPE_VLAN;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* type in after this line is asserted at BCM_MULTICAST_TYPE_L2 or 
     *  BCM_MULTICAST_TYPE_VLAN only.
     */

    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (SOC_IS_TBX(unit)) {
        /* enable multicast vport replication feature if any mcast replication 
         *  group(0-255) created.
         */
        rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                (uint32 *)&mcrep_num);
        if (rv){
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s, %d, device property get problem!\n"),
                       FUNCTION_NAME(), __LINE__));
            return BCM_E_INTERNAL;
        }
    }

    sal_memset(&mcast_entry, 0, sizeof (marl_pbmp_entry_t));
    if (op_type == BCM_MULTICAST_TYPE_L2){
        
        /* support for the switch chip which has L2MC_PBMP table only */
        rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
        if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
            return BCM_E_UNAVAIL;
        }
                
        if ((mc_index < 0) || (mc_index >= mem_cnt )) {
            return BCM_E_BADID;
        }
    } else if (op_type == BCM_MULTICAST_TYPE_VLAN) {
        /*
         *  Multicast deisgn for TBX's VLAN replication is changed to support 
         *  group type at BCM_MULTICAST_TYPE_VLAN. 
         *  - support BCM_MULTICAST_TYPE_SUBPORT as well for backward 
         *      compatible in group create/destroy.
         */
        if (!(soc_feature(unit, soc_feature_subport) || 
                soc_feature(unit, soc_feature_vlan_vp)) ) {
            return BCM_E_UNAVAIL;
        }

        if ((mc_index < 0) || (mc_index >= mcrep_num )) {
            return BCM_E_BADID;
        }

    }
    
    /* check if the id is existed */
    if (_bcm_robo_l2mc_id_check(unit, mc_index) == 0){
        return BCM_E_NOT_FOUND;
    }
    
    /* Clear the HW entry */
    rv = DRV_MEM_WRITE(unit, DRV_MEM_MCAST, mc_index, 1, 
            (uint32 *)&mcast_entry);
    if (rv){
        LOG_INFO(BSL_LS_BCM_L2TABLE,
                 (BSL_META_U(unit,
                             "%s: faield on get the mcast_bmp table\n"), FUNCTION_NAME()));
        return rv;
    }
    
    /* Free the L2MC index */
    BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_id_free(unit, mc_index));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_l3_encap_get
 * Purpose:
 *      Get the Encap ID for L3.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      group - (IN) Multicast group ID.
 *      port  - (IN) Physical port.
 *      intf  - (IN) L3 interface ID.
 *      encap_id - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_multicast_l3_encap_get(int unit, 
                               bcm_multicast_t group, 
                               bcm_gport_t port, 
                               bcm_if_t intf, 
                               bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_multicast_l2_encap_get
 * Purpose:
 *      Get the Encap ID for L2.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      group    - (IN) Multicast group ID.
 *      port     - (IN) Physical port.
 *      vlan     - (IN) Vlan.
 *      encap_id - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_multicast_l2_encap_get(int unit, 
                               bcm_multicast_t group, 
                               bcm_gport_t port, 
                               bcm_vlan_t vlan, 
                               bcm_if_t *encap_id)
{
    /* Encap ID is not used for L2 in ROBO(the same design with ESW) */
    *encap_id = BCM_IF_INVALID;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_vpls_encap_get
 * Purpose:
 *      Get the Encap ID for a MPLS port.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      group        - (IN) Multicast group ID.
 *      port         - (IN) Physical port.
 *      mpls_port_id - (IN) MPLS port ID.
 *      encap_id     - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_vpls_encap_get(int unit, 
                                 bcm_multicast_t group,
                                 bcm_gport_t port,
                                 bcm_gport_t mpls_port_id,
                                 bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_multicast_subport_encap_get
 * Purpose:
 *      Get the Encap ID for a subport.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      subport   - (IN) Subport ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_subport_encap_get(int unit,
                                    bcm_multicast_t group,
                                    bcm_gport_t port,
                                    bcm_gport_t subport,
                                    bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_mim_encap_get
 * Purpose:
 *      Get the Encap ID for MiM.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      mim_port  - (IN) MiM port ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_mim_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                bcm_gport_t mim_port, bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_wlan_encap_get
 * Purpose:
 *      Get the Encap ID for WLAN.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      wlan_port - (IN) WLAN port ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_wlan_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                 bcm_gport_t wlan_port, bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_vlan_encap_get
 * Purpose:
 *      Get the Encap ID for VLAN port (L2 logical port).
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      vlan_port - (IN) VLAN port ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 *
 * Notes:
 *  1. the vlan_port_id must be consructed as GPORT type to carry the VLAN 
 *      information(i.e. Tag/Untag status+VID) for replicastion.
 *  2. The encap_id must include the proper Mcast replication on 
 *      {PPort, Untag, VID}.
 *      P.S. MC_GID is the info been handled by user, this encap_id don't 
 *              need to bound it in.
 *  3. 'group' parameter will be ignored in this API due to the reason of MC 
 *      group is handled by user.
 */
int
bcm_robo_multicast_vlan_encap_get(int unit, 
                bcm_multicast_t group, bcm_gport_t port, 
                bcm_gport_t vlan_port_id, bcm_if_t *encap_id)
{
    MULTICAST_ROBO_INIT_CHECK(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "%s,%d,vlan_port_id=0x%x\n"),
                 FUNCTION_NAME(), __LINE__, vlan_port_id));
    if (soc_feature(unit, soc_feature_vlan_vp)) {
        int port_in = -1, vlanport_id = -1, vid_in = 0, untag_in = 0;
        
        if (encap_id == NULL) {
            return BCM_E_PARAM;
        }
        
        BCM_IF_ERROR_RETURN(
                _bcm_robo_port_gport_validate(unit, port, &port_in));

        if (!BCM_GPORT_IS_VLAN_PORT(vlan_port_id)) {
            return BCM_E_PARAM;
        }

        vlanport_id = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port_id);

        /* get VID and UT information from this vlanport_id */
        vid_in = _BCM_VLAN_PORT_VID_GET(vlanport_id);
        untag_in = _BCM_VLAN_PORT_UNTAG_GET(vlanport_id);

        
        *encap_id = _BCM_MULTICAST_ENCAP_VLANPORT_SET(port_in, vid_in, 
                (untag_in & _BCM_MULTICAST_ENCAP_FLAGS_MASK));
        
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "%s,%d,vid=%d,ut=%d,encap_id=0x%x\n"),
                     FUNCTION_NAME(), __LINE__, vid_in, untag_in, *encap_id));
        return BCM_E_NONE;
    } else {
        return BCM_E_UNAVAIL;
    }
}

/*
 * Function:
 *      bcm_multicast_egress_add
 * Purpose:
 *      Add a GPORT to the replication list
 *      for the specified multicast index.
 * Parameters:
 *      unit      - (IN) Device Number
 *      group     - (IN) Multicast group ID
 *      port      - (IN) GPORT Identifier
 *      encap_id  - (IN) Encap ID.
 * Returns:
 *      BCM_E_XXX
 *
 * Note :
 *  1. The L2 type in this API can serve the chip within Mcast group table
 *      on indicating group member port bitmap relationship. 
 */
int
bcm_robo_multicast_egress_add(int unit, 
                             bcm_multicast_t group, 
                             bcm_gport_t port,
                             bcm_if_t encap_id)
{
    int     type, op_type, mc_index, port_in;
#ifdef  BCM_TB_SUPPORT
    int     vport = 0, pport = 0;
    uint32  vport_bmp = 0;
    int     ctrl_val = 0;
    uint32  ctrl_cnt = 0, ctrl_type = 0;

    int     i, vp_op = 0;
    int     vid_in = 0,  target_vid = -1;
    int     is_untag = FALSE, is_existed = FALSE;
    
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
#endif   /* BCM_TB_SUPPORT */
    
    MULTICAST_ROBO_INIT_CHECK(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "%s,gid=0x%x,port=%d,encap_id=0x%x\n"), 
                 FUNCTION_NAME(), group, port, encap_id));
    /* two type supported in this API. 
     *  - BCM_MULTICAST_TYPE_L2: add physicall port from a L2MC group.
     *  - BCM_MULTICAST_TYPE_VLAN: add existed VPort from L2MC group. 
     */
    type = _BCM_MULTICAST_TYPE_GET(group);

    if (type == _BCM_MULTICAST_TYPE_VLAN){
        op_type = BCM_MULTICAST_TYPE_VLAN;
        
        if (!soc_feature(unit, soc_feature_vlan_vp)) {
            return BCM_E_UNAVAIL;
        }
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        op_type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* port_id check */
    BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port_in));
    
    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (op_type == BCM_MULTICAST_TYPE_L2){

        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                MCAST_EGRESS_OP_ADD, mc_index, (uint32 *)&port_in));
                
    } else if (op_type == BCM_MULTICAST_TYPE_VLAN) {
#ifdef  BCM_TB_SUPPORT

        if (SOC_IS_TBX(unit)) {

            /* Working flow :
             *  1. Retrieve the required information from enacp_id.
             *  2. Starting the add process.
             *      1). if this VLAN existed (of a VPort) on this PPort.
             *          a. if this VPort is activated already in this 
             *              MC group.
             *              - return BCM_E_EXISTS.
             *          b. reuse this VPort for replication in this MC group.
             *      2). else add this VLAN on a VPort of this PPort.
             *          - return BCM_E_FULL if there no free VPort.
             *          - set up this VID(include Untag) on this VPort of this 
             *              PPort.
             *          - activate this VPort of this PPort in this MC group.
             *  Note : 
             *      1. SW database update is required once any related setting 
             *          is changed.
             */
            
            /* parse encap_id to retrieve group, pport, vport */
            pport = _BCM_MULTICAST_ENCAP_PPORT_GET(encap_id);
            is_untag = _BCM_MULTICAST_ENCAP_FLAGS_IS_UNTAG(encap_id);
            vid_in = _BCM_MULTICAST_ENCAP_VLAN_GET(encap_id);
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "%s,pp=%d,ut=%d,vid=%d\n"), 
                         FUNCTION_NAME(), pport, is_untag, vid_in));

            /* valid check */
            if (pport != port_in){
                return BCM_E_PARAM;
            }
            BCM_IF_ERROR_RETURN(_mcast_repl_param_check
                    (unit, mc_index, port_in));

            target_vid = (is_untag) ? _BCM_ROBO_UNTAG_VID : vid_in;

            for (i = 0; i < _TB_SUBPORT_NUM_VPORT_PER_GROUP; i++) {
                if ((MCAST_REPL_VID_GET(unit, pport, i)) == target_vid){
                    is_existed = TRUE;
                    vport = i;
                    break;
                }
            }
            
            if (is_existed) {
                if (MCAST_REPL_USED_ISSET(unit, mc_index, pport, vport)){
                    /* this vport is activated in this Mcast group already */
                    return BCM_E_EXISTS;
                }
                
            } else {
                /* find the 1st free Vport of this PPort */
                for (i = 0; i < _TB_SUBPORT_NUM_VPORT_PER_GROUP; i++) {
                    if (MCAST_REPL_VID_CNT_IS_ZERO(unit, pport, i)) {
                        /* reference count=0 means this is a free VPort */
                        vport = i;
                        break;
                    }
                }
                
                /* check if no free VPort of this PPort */
                if (i == _TB_SUBPORT_NUM_VPORT_PER_GROUP){
                    return BCM_E_FULL;
                }
                
                /* assign VID to this VPort of this PPort 
                 * 1. note that untag and tagged operation is different.
                 * 2. SW database update : VID for this VP of this PPort.
                 */
                vp_op = (is_untag) ? DRV_MCREP_VPORT_OP_UNTAG_VP : 
                        DRV_MCREP_VPORT_OP_VID;
                LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "%s,%d,VPORT%d VID assignment(ut=%d,vid=%d).\n"), 
                             FUNCTION_NAME(), __LINE__, vport, is_untag, vid_in));
                
                /* target_vid is for SW, vid_in is the value for driver */
                BCM_IF_ERROR_RETURN(DRV_MCREP_VPORT_CONFIG_SET
                        (unit, pport, vp_op, vport, vid_in));
                MCAST_REPL_VID_SET(unit, pport, vport, target_vid);
            }
            
            /* activate the replication on this VPort 
             *  1. activate this VPort of this Pport in this MC group 
             *  2. SW database update : 
             *      - activate this VP in the replication group.
             *      - increase the reference count.
             */
            repl_index = _mcast_repl_vp_group_get(unit, mc_index, port);
            vport_bmp = *((int *)repl_index);
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "%s,%d,VPORT%d for this replication(ut=%d,vid=%d).\n"), 
                         FUNCTION_NAME(), __LINE__, vport, is_untag, vid_in));
            
            vport_bmp |= ((0x1 << vport) & _TB_VPORTBMP_MASK);
            if (DRV_MCREP_VPGRP_VPORT_CONFIG_SET(unit, mc_index, pport,
                    DRV_MCREP_VPGRP_OP_VPORT_MEMBER, (int *)&vport_bmp)){
                return BCM_E_INTERNAL;
            }
            MCAST_REPL_USED_SET(unit, mc_index, pport, vport);
            MCAST_REPL_VID_CNT_INC(unit, pport, vport);

            if (robo_mcast_repl_config[unit]->mcast_repl_enable == FALSE) {
                LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "%s, Enable MCast replication!!\n"), 
                             FUNCTION_NAME()));
                /* 1. enable vport replication and 
                 * 2. SW database update : enabling statue 
                 */
                ctrl_cnt = 1;
                ctrl_type = DRV_DEV_CTRL_MCASTREP;
                ctrl_val = TRUE;
                BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_SET(unit, 
                        &ctrl_cnt, &ctrl_type, &ctrl_val));
                        
                robo_mcast_repl_config[unit]->mcast_repl_enable = TRUE;
            }
            
        } else {
            return BCM_E_UNAVAIL;
        }
#else   /* BCM_TB_SUPPORT */
        return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_multicast_egress_delete
 * Purpose:
 *      Delete GPORT from the replication list
 *      for the specified multicast index.
 * Parameters:
 *      unit      - (IN) Device Number
 *      group     - (IN) Multicast group ID
 *      port      - (IN) GPORT Identifier
 *      encap_id  - (IN) Encap ID.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_egress_delete(int unit, 
                                bcm_multicast_t group, 
                                bcm_gport_t port,
                                bcm_if_t encap_id)
{
    int     type, op_type, mc_index, port_in;
#ifdef  BCM_TB_SUPPORT
    uint32  vport_bmp = 0;
    int     ctrl_val = 0;
    uint32  ctrl_cnt = 0, ctrl_type = 0;
    int     vport = 0, pport = 0;

    int     i, vp_op = 0;
    int     vid_in = 0,  target_vid = -1;
    int     is_untag = FALSE, is_existed = FALSE;
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
#endif   /* BCM_TB_SUPPORT */
    
    MULTICAST_ROBO_INIT_CHECK(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "%s,gid=0x%x,port=%d,encap_id=0x%x\n"), 
                 FUNCTION_NAME(), group, port, encap_id));
    /* two type supported in this API. 
     *  - BCM_MULTICAST_TYPE_L2: remove physicall port from a L2MC group.
     *  - BCM_MULTICAST_TYPE_VLAN: remove existed VPort from L2MC group. 
     */
    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_VLAN){
        op_type = BCM_MULTICAST_TYPE_VLAN;
        
        if (!soc_feature(unit, soc_feature_vlan_vp)) {
            return BCM_E_UNAVAIL;
        }
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        op_type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* port_id check */
    BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port_in));
    
    mc_index = _BCM_MULTICAST_ID_GET(group);

    if (op_type == BCM_MULTICAST_TYPE_L2){

        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                MCAST_EGRESS_OP_DEL, mc_index, (uint32 *)&port_in));
    } else if (op_type == BCM_MULTICAST_TYPE_VLAN){
#ifdef  BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {
            /* Working flow :

             *  1. Retrieve the required information from enacp_id.
             *  2. Starting the delete process.
             *      1). if this VLAN(of a VPort) is not existed on this PPort.
             *          - return BCM_E_NOT_FOUND.
             *      2). else performing remove process.
             *          a. if this VPort(for target VLAN) is not activated in 
             *              this replication group.
             *              - return BCM_E_NOT_FOUND.
             *          b. else inactive this VPort in this replication group.
             *
             *  Note : 
             *      1. SW database update is required once any related setting 
             *          is changed.
             */
            
            /* parse encap_id to retrieve group, pport, vport */
            pport = _BCM_MULTICAST_ENCAP_PPORT_GET(encap_id);
            is_untag = _BCM_MULTICAST_ENCAP_FLAGS_IS_UNTAG(encap_id);
            vid_in = _BCM_MULTICAST_ENCAP_VLAN_GET(encap_id);
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "%s,pp=%d,ut=%d,vid=%d\n"), 
                         FUNCTION_NAME(), pport, is_untag, vid_in));
            
            /* valid check */
            if (pport != port_in){
                return BCM_E_PARAM;
            }
            BCM_IF_ERROR_RETURN(_mcast_repl_param_check
                    (unit, mc_index, port_in));

            target_vid = (is_untag) ? _BCM_ROBO_UNTAG_VID : vid_in;

            for (i = 0; i < _TB_SUBPORT_NUM_VPORT_PER_GROUP; i++) {
                if ((MCAST_REPL_VID_GET(unit, pport, i)) == target_vid){
                    is_existed = TRUE;
                    vport = i;
                    break;
                }
            }

            /* check the existence */            
            if (!is_existed) {
                return BCM_E_NOT_FOUND;
            } else {
                if (!MCAST_REPL_USED_ISSET(unit, mc_index, pport, vport)){
                    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                                (BSL_META_U(unit,
                                            "%s,VPORT %d not in replication group.\n"), 
                                 FUNCTION_NAME(), vport));
                    /* this vport is not activated in this Mcast group */
                    return BCM_E_NOT_FOUND;
                }
            }

            /* inactive this VPort from replication group :
             *  1. comply nomal inactive setting.
             *  2. SW database update : 
             *      - unset the VP_bitmap of this replication group.
             *      - decrease reference count.
             *  3. recall this VPort of this PPort once this VPort of this 
             *      PPort is not activated in any other replication group.
             */
            repl_index = _mcast_repl_vp_group_get(unit, mc_index, port);
            vport_bmp = *((int *)repl_index);

            vport_bmp &= ~(0x1 << vport);
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "%s,remove VPort %d from replication group.\n"), 
                         FUNCTION_NAME(), vport));
            if (DRV_MCREP_VPGRP_VPORT_CONFIG_SET(unit, mc_index, port,
                    DRV_MCREP_VPGRP_OP_VPORT_MEMBER, (int *)&vport_bmp)){
                return BCM_E_INTERNAL;
            }
            MCAST_REPL_USED_CLR(unit, mc_index, pport, vport);
            MCAST_REPL_VID_CNT_DEC(unit, pport, vport);

            if (MCAST_REPL_VID_CNT_IS_ZERO(unit, pport, vport)){
                /* recall this VPort of this PPort :
                 *  1. clear VLAN information of this VPort
                 *      - tagged/untagged VLAN operation is different
                 *  2. SW database update : Clear VLAN of this VPort.
                 *  3. disable Mcast replication feature once there is no activated
                 *      Vport in device.
                 *      - SW database update : clear the enabling status.
                 */
                vp_op = (is_untag) ? DRV_MCREP_VPORT_OP_UNTAG_RESET : 
                        DRV_MCREP_VPORT_OP_VID;
                target_vid = 0;
                LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "%s,reset VLAN(ut=%d,vid=%d) on VPort %d\n"), 
                             FUNCTION_NAME(), is_untag, target_vid, vport));
                BCM_IF_ERROR_RETURN(DRV_MCREP_VPORT_CONFIG_SET
                        (unit, pport, vp_op, vport, target_vid));
                MCAST_REPL_VID_SET(unit, pport, vport, target_vid);
                
                if (_mcast_repl_system_no_vpset(unit)){
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "%s, disable MCast replication!!\n"), 
                               FUNCTION_NAME()));
                    /* disable vport replication */
                    ctrl_cnt = 1;
                    ctrl_type = DRV_DEV_CTRL_MCASTREP;
                    ctrl_val = FALSE;
                    BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_SET(unit, 
                            &ctrl_cnt, &ctrl_type, &ctrl_val));
                            
                    robo_mcast_repl_config[unit]->mcast_repl_enable = FALSE;
                }
            }
        } else {
            return BCM_E_UNAVAIL;
        }
#else   /* BCM_TB_SUPPORT */
        return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_egress_delete_all
 * Purpose:
 *      Delete all replications for the specified multicast index.
 * Parameters:
 *      unit      - (IN) Device Number
 *      group     - (IN) Multicast group ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_egress_delete_all(int unit, bcm_multicast_t group)
{
    int     type, op_type, mc_index;
    uint32  op_value = 0;
#ifdef  BCM_TB_SUPPORT
    int pport = 0, vport = 0, vid = 0, flags = 0;
    
    bcm_pbmp_t  l2_pbmp;
    bcm_if_t    encap_id;
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
#endif      /* BCM_TB_SUPPORT */
    
    MULTICAST_ROBO_INIT_CHECK(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "%s,group=0x%x\n"), FUNCTION_NAME(), group));
    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_VLAN){
        op_type = BCM_MULTICAST_TYPE_VLAN;
        
        if (!soc_feature(unit, soc_feature_vlan_vp)) {
            return BCM_E_UNAVAIL;
        }
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        op_type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (op_type == BCM_MULTICAST_TYPE_L2){
        /* delete_all for BCM_MULTICAST_TYPE_L2 type will always be performed */
        op_value = 0;
        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                MCAST_EGRESS_OP_CLR, mc_index, &op_value));
    } else if (op_type == BCM_MULTICAST_TYPE_VLAN){
        
#ifdef  BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {
            BCM_IF_ERROR_RETURN(_mcast_repl_param_check(unit, mc_index, 0));
            
            /* clear all VLAN replication setting */
            BCM_PBMP_CLEAR(l2_pbmp);
            BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                        MCAST_EGRESS_OP_GET, mc_index, (uint32 *)&l2_pbmp));
    
            BCM_PBMP_ITER(l2_pbmp, pport) {
                /* retrieve all the activated VPorts of this PPort */
                repl_index = _mcast_repl_vp_group_get(unit, mc_index, pport);
                
                /* construct the encap_id_array for this PPort */
                for (vport = 0; vport < _TB_SUBPORT_NUM_VPORT_PER_GROUP; 
                        vport ++) {
                    if (!SHR_BITGET(repl_index->repl_vport_id, vport)) {
                        continue;
                    }
    
                    vid = MCAST_REPL_VID_GET(unit, pport, vport);
                    
                    if (vid == _BCM_ROBO_UNTAG_VID) {
                        flags = _BCM_MULTICAST_ENCAP_FLAGS_UNTAG;
                        vid = 0;
                    } else {
                        flags = 0;
                    }
    
                    encap_id = _BCM_MULTICAST_ENCAP_VLANPORT_SET(
                            pport, vid, flags); 
                            
                    /* performing egress_delete API */
                    BCM_IF_ERROR_RETURN(bcm_multicast_egress_delete
                            (unit, group, pport, encap_id));
                }
            }
        }
#else  /* BCM_TB_SUPPORT */
        return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
    }
        
    return BCM_E_NONE;
}

/*  
 * Function:
 *      bcm_multicast_egress_set
 * Purpose:
 *      Assign the complete set of egress GPORTs in the
 *      replication list for the specified multicast index.
 * Parameters:
 *      unit       - (IN) Device Number
 *      group      - (IN) Multicast group ID
 *      port_count - (IN) Number of ports in replication list
 *      port_array - (IN) List of GPORT Identifiers
 *      encap_id_array - (IN) List of encap identifiers
 * Returns:
 *      BCM_E_XXX
 *  Note :
 *      1. encap_id_array is for assigning the replication virtual ports and 
 *          the size of this array must be well prepared.
 *          - for TBX devices, since there are 16 VPorts of a physical port,
 *              the prepared size of encap_id_array must be 16 * port_count. 
 */     
int     
bcm_robo_multicast_egress_set(int unit,
                             bcm_multicast_t group,
                             int port_count,
                             bcm_gport_t *port_array,
                             bcm_if_t *encap_id_array)
{
    int     i, type, op_type, mc_index, rv = BCM_E_NONE;
    int     port_in = BCM_GPORT_INVALID;
#ifdef  BCM_TB_SUPPORT
    int     is_untag = FALSE, pport = 0;
    int     vid_in = 0;
    
    bcm_if_t encap_id;
#endif   /* BCM_TB_SUPPORT */

    bcm_port_t *local_port_array = NULL;
    bcm_pbmp_t l2_pbmp;

    MULTICAST_ROBO_INIT_CHECK(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "%s,gid=%d,port_count=%d\n"), 
                 FUNCTION_NAME(), group, port_count));
    if (port_count < 0) {
        return BCM_E_PARAM;
    } else if (port_count == 0){
        /* if port_count=0, performing clear action */
        return bcm_multicast_egress_delete_all(unit, group);
    }

    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_VLAN){
        op_type = BCM_MULTICAST_TYPE_VLAN;
        
        if (!soc_feature(unit, soc_feature_vlan_vp)) {
            return BCM_E_UNAVAIL;
        }
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        op_type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* retrieve the l2mc_index */
    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (op_type == BCM_MULTICAST_TYPE_L2){
        if (port_array == NULL) {
            return BCM_E_PARAM;
        }

        /* Convert GPORT array into local port numbers */
        local_port_array = sal_alloc(sizeof(bcm_port_t) * port_count, 
                "local_port array");
    
        if (local_port_array == NULL) {
            return BCM_E_MEMORY;
        }
        for (i = 0; i < port_count ; i++) {
            port_in = BCM_GPORT_INVALID;
            rv = _bcm_robo_port_gport_validate(
                    unit, port_array[i], &port_in);
            if (BCM_FAILURE(rv) || (port_in == BCM_GPORT_INVALID)){
                sal_free(local_port_array);
                return BCM_E_PARAM;   /* esw's return is BCM_E_PORT */
            } else {
                local_port_array[i] = port_in;
            }
        }
    
        BCM_PBMP_CLEAR(l2_pbmp);
        for (i = 0; i < port_count; i++) {
            BCM_PBMP_PORT_ADD(l2_pbmp, local_port_array[i]);
        }        
        sal_free(local_port_array);

        rv = _bcm_robo_multicast_egress_op(unit,
                        MCAST_EGRESS_OP_SET, mc_index, (uint32 *)&l2_pbmp);
        if (BCM_FAILURE(rv)){
            return rv;
        }
        
    } else if (op_type == BCM_MULTICAST_TYPE_VLAN){
        
        if (encap_id_array == NULL) {
            return BCM_E_PARAM;
        }
        
#ifdef  BCM_TB_SUPPORT
        
        if (SOC_IS_TBX(unit)) {
            int utvp_cnt[_TB_SUBPORT_NUM_PORT], vp_cnt[_TB_SUBPORT_NUM_PORT];
            
            BCM_IF_ERROR_RETURN(_mcast_repl_param_check(unit, mc_index, 0));
            
            /* working flow(for TBX) :
             *
             *  1. encap_id_array validation : return BCM_E_PARAM while invalid.
             *    - retrieve the carraied informations. =>{pport, vid, untag}
             *    - ignore the redundant VLAN replication.
             *    - upto 16 VLAN replications can be bounded in a PPort.
             *    - only one untag replication in a PPort.
             *  2. Reset HW/SW table for this MC group.
             *  3. Call to bcm_mulitcast_egress_add() for setting.
             */
            sal_memset(utvp_cnt, 0, sizeof(utvp_cnt));
            sal_memset(vp_cnt, 0, sizeof(vp_cnt));
            for (i = 0; i < port_count; i++){
                encap_id = encap_id_array[i];
                pport = _BCM_MULTICAST_ENCAP_PPORT_GET(encap_id);
                is_untag = _BCM_MULTICAST_ENCAP_FLAGS_IS_UNTAG(encap_id);
                vid_in = _BCM_MULTICAST_ENCAP_VLAN_GET(encap_id);
                LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "%s,encap[%d]:pp=%d,ut=%d,vid=%d\n"), 
                             FUNCTION_NAME(), i, pport, is_untag, vid_in));
                
                if (is_untag) {
                    if (utvp_cnt[pport] != 0) {
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s, encap_id_array invalid! Only one " \
                                             "untag replication on port %d is allowed.\n"),
                                  FUNCTION_NAME(), pport));
                        return BCM_E_PARAM;
                    } else {
                        utvp_cnt[pport] = 1;
                    }
                }
                
                if (vp_cnt[pport] < _TB_SUBPORT_NUM_VPORT_PER_GROUP) {
                    vp_cnt[pport] += 1;
                } else {                    
                    LOG_WARN(BSL_LS_BCM_COMMON,
                             (BSL_META_U(unit,
                                         "%s, encap_id_array invalid! Up-to %d" \
                                         "replications on port %d is allowed only.\n"),
                              FUNCTION_NAME(), _TB_SUBPORT_NUM_VPORT_PER_GROUP,
                              pport));
                    return BCM_E_PARAM;
                }
            }
            
            /* clear with group tpye at _BCM_MULTICAST_TYPE_VLAN */
            BCM_IF_ERROR_RETURN(bcm_multicast_egress_delete_all(unit, group));
            
            for (i = 0; i < port_count; i++){
                encap_id = encap_id_array[i];
                pport = _BCM_MULTICAST_ENCAP_PPORT_GET(encap_id);
                BCM_IF_ERROR_RETURN(bcm_multicast_egress_add
                        (unit, group, pport, encap_id));
            }
            
        } else {
            return BCM_E_UNAVAIL;
        }
#else   /* BCM_TB_SUPPORT */
        return BCM_E_UNAVAIL;
#endif   /* BCM_TB_SUPPORT */
    }

    return BCM_E_NONE;
}   

/*
 * Function:
 *      bcm_multicast_egress_get
 * Purpose:
 *      Retrieve a set of egress multicast GPORTs in the
 *      replication list for the specified multicast index.
 * Parameters: 
 *      unit           - (IN) Device Number
 *      mc_index       - (IN) Multicast index
 *      port_max       - (IN) Number of entries in "port_array"
 *      port_array     - (OUT) List of ports
 *      encap_id_array - (OUT) List of encap identifiers
 *      port_count     - (OUT) Actual number of ports returned
 * Returns:
 *      BCM_E_XXX
 *
 *  Note :
 *      1. User must well prepared the size of the OUTPUT parameter of 
 *          encap_id_array.
 *          - for TBX devices, since there are 16 VPorts will be reported for 
 *              a physical port, the prepared size of encap_id_array must be 
 *              16 * port_count or more.
 */
int     
bcm_robo_multicast_egress_get(int unit,
                             bcm_multicast_t group,
                             int port_max,
                             bcm_gport_t *port_array, 
                             bcm_if_t *encap_id_array, 
                             int *port_count)
{
    int     type, op_type, i, mc_index, rv = BCM_E_NONE;
    bcm_pbmp_t      l2_pbmp;
    bcm_port_t      port_iter = 0;
    bcm_gport_t     *int_port_ary;
    bcm_if_t        *int_encap_id_ary;
#ifdef  BCM_TB_SUPPORT
    uint32  vport = 0, pport = 0, vid = 0, flags = 0;
    
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
#endif   /* BCM_TB_SUPPORT */

    MULTICAST_ROBO_INIT_CHECK(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "%s,gid=0x%x,port_max=%d\n"), 
                 FUNCTION_NAME(), group, port_max));
    /* port_array and encap_id_array are allowed to be NULL */
    if ((NULL == port_count) || (port_max < 0) ){
        return BCM_E_PARAM;
    }

    /* If port_max = 0, port_array and encap_id_array must be NULL */
    if (port_max == 0) {
        if ((NULL != port_array) || (NULL != encap_id_array)) {
            return BCM_E_PARAM;
        } else {
            *port_count = 0;
            return BCM_E_NONE;
        }
    }

    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_VLAN){
        op_type = BCM_MULTICAST_TYPE_VLAN;
        
        if (!soc_feature(unit, soc_feature_vlan_vp)) {
            return BCM_E_UNAVAIL;
        }
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        op_type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    /* type in after this line is asserted at BCM_MULTICAST_TYPE_L2 or 
     *  BCM_MULTICAST_TYPE_VLAN only.
     */
    
    /* retrieve the l2mc_index */
    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    BCM_PBMP_CLEAR(l2_pbmp);
    BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                MCAST_EGRESS_OP_GET, mc_index, (uint32 *)&l2_pbmp));

    int_port_ary = sal_alloc(sizeof(bcm_gport_t) * port_max, 
            "internal port array");
    int_encap_id_ary = sal_alloc(sizeof(bcm_if_t) * port_max, 
            "internal encap_id array");
    if ((int_port_ary == NULL) || (int_encap_id_ary == NULL)) {
        rv = BCM_E_MEMORY;
        goto mcast_egress_get_exit;
    }
    sal_memset(int_port_ary, 0, sizeof(bcm_gport_t) * port_max);
    sal_memset(int_encap_id_ary, 0, sizeof(bcm_if_t) * port_max);
                    
    if (op_type == BCM_MULTICAST_TYPE_L2){

        i = 0;
        BCM_PBMP_ITER(l2_pbmp, port_iter) {
            if (i >= port_max) {
                break;
            }
            
            rv = bcm_port_gport_get(unit, port_iter, (int_port_ary + i));
            if (BCM_FAILURE(rv)){
                goto mcast_egress_get_exit;
            }
            
            i++;
        }
        *port_count = i;
        
        sal_memcpy(port_array, int_port_ary, 
                (sizeof(bcm_gport_t) * (*port_count)));

        
        /* assigning initial value on encap_id_array */
        if (encap_id_array != NULL) {
            sal_memset(encap_id_array, 0, (sizeof(bcm_if_t) * (*port_count)));
        }
    } else if (op_type == BCM_MULTICAST_TYPE_VLAN) {
#ifdef  BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {

            rv = _mcast_repl_param_check(unit, mc_index, 0);
            if (BCM_FAILURE(rv)){
                goto mcast_egress_get_exit;
            }
            
            i = 0;
            BCM_PBMP_ITER(l2_pbmp, port_iter) {
                
                /* retrieve all the activated VPorts of this PPort */
                pport = port_iter;
                repl_index = _mcast_repl_vp_group_get(unit, mc_index, pport);
                
                /* construct the encap_id_array for this PPort */
                for (vport = 0; vport < _TB_SUBPORT_NUM_VPORT_PER_GROUP; 
                        vport ++) {
                    if (!SHR_BITGET(repl_index->repl_vport_id, vport)) {
                        continue;
                    }
                    
                    vid = MCAST_REPL_VID_GET(unit, pport, vport);
                    
                    if (vid == _BCM_ROBO_UNTAG_VID) {
                        flags = _BCM_MULTICAST_ENCAP_FLAGS_UNTAG;
                        vid = 0;
                    } else {
                        flags = 0;
                    }

                    int_encap_id_ary[i] = _BCM_MULTICAST_ENCAP_VLANPORT_SET(
                            pport, vid, flags); 
                    i++;
                    if (i >= port_max) {
                        break;
                    }
                }
                
                if (i >= port_max) {
                    break;
                }
            }
            *port_count = i;

            sal_memcpy(encap_id_array, int_encap_id_ary, 
                    (sizeof(bcm_if_t) * (*port_count)));
 
            /* assigning initial value on port_array */
            if (port_array != NULL) {
                sal_memset(port_array, 0, 
                        sizeof(bcm_gport_t) * (*port_count));
            }
            
        } else {
            rv = BCM_E_UNAVAIL;
            goto mcast_egress_get_exit;
        }
#else   /* BCM_TB_SUPPORT */
        rv = BCM_E_UNAVAIL;
        goto mcast_egress_get_exit;
#endif   /* BCM_TB_SUPPORT */
    }

mcast_egress_get_exit:    

    if (int_port_ary != NULL) {
        sal_free((void *)int_port_ary);
    }
    if (int_encap_id_ary != NULL) {
        sal_free((void *)int_encap_id_ary);
    }
    return rv;
    
}   

/*
 * Function:
 *      bcm_robo_multicast_init
 * Purpose:
 *      Initialize the multicast module.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_init(int unit)
{
#ifdef  BCM_TB_SUPPORT
    int len =0;
    int     ctrl_val = 0;
    uint32  ctrl_cnt = 0, ctrl_type = 0;
    int     rv = BCM_E_NONE;
    
    if (soc_feature(unit, soc_feature_vlan_vp)) {
        if (SOC_IS_TBX(unit)) {
            if (robo_mcast_repl_config[unit] == NULL){
                len = sizeof(_bcm_robo_mcast_repl_t);
                robo_mcast_repl_config[unit] = sal_alloc(len, 
                        "Mcast replication");
                sal_memset(robo_mcast_repl_config[unit], 0, len);
                robo_mcast_repl_config[unit]->mcast_repl_enable = FALSE;
            }
        
            if (robo_mcast_repl_vid[unit] == NULL){
                len = sizeof(_bcm_robo_mcast_repl_vlan_t);
                robo_mcast_repl_vid[unit] = 
                        sal_alloc(len, "Mcast replication vids");
                if (robo_mcast_repl_vid[unit] == NULL) {
                    sal_free(robo_mcast_repl_config[unit]);
                    robo_mcast_repl_config[unit] = NULL;
                }
                sal_memset(robo_mcast_repl_vid[unit], 0, len);
            }

            /* disable vport replication */
            ctrl_cnt = 1;
            ctrl_type = DRV_DEV_CTRL_MCASTREP;
            ctrl_val = FALSE;
            rv = DRV_DEV_CONTROL_SET(unit, &ctrl_cnt, &ctrl_type, &ctrl_val);
            BCM_IF_ERROR_RETURN(rv);

            /* Delete all existing subports configurations */
            rv = DRV_MEM_CLEAR(unit, DRV_MEM_VPORT_VID_MAP);
            BCM_IF_ERROR_RETURN(rv);
        
            /* Delete all subport groups configurations */
            rv = DRV_MEM_CLEAR(unit, DRV_MEM_MCAST_VPORT_MAP);
            BCM_IF_ERROR_RETURN(rv);
        }
    }
#endif  /* BCM_TB_SUPPORT */

    multicast_robo_initialized[unit] = TRUE;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_multicast_detach
 * Purpose:
 *      Shut down (uninitialize) the multicast module.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_detach(int unit)
{
#ifdef  BCM_TB_SUPPORT
    if (soc_feature(unit, soc_feature_vlan_vp)) {
        if (SOC_IS_TBX(unit)) {
            if (NULL != robo_mcast_repl_config[unit]) {
                sal_free(robo_mcast_repl_config[unit]);
            }
            robo_mcast_repl_config[unit] = NULL;
        
            if (NULL != robo_mcast_repl_vid[unit]) {
                sal_free(robo_mcast_repl_vid[unit]);
            }
            robo_mcast_repl_vid[unit] = NULL;
        }
    }
#endif  /* BCM_TB_SUPPORT */

    multicast_robo_initialized[unit] = FALSE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_multicast_group_get
 * Purpose:
 *      Retrieve the flags associated with a mulitcast group.
 * Parameters:
 *      unit - (IN) Unit number.
 *      group - (IN) Multicast group ID
 *      flags - (OUT) BCM_MULTICAST_*
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_group_get(int unit, bcm_multicast_t group, uint32 *flags)
{
    int     rv = BCM_E_NONE, mem_cnt = 0;
    uint32  mc_index, type_flag, mc_group_count = 0;
    
    /* support for the switch chip which has L2MC_PBMP table only */
    rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
    if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
        return BCM_E_UNAVAIL;
    }

    *flags = 0;
    mc_index = _BCM_MULTICAST_ID_GET(group);

    if (_BCM_MULTICAST_IS_L2(group)) {
        mc_group_count = mem_cnt;
        type_flag = BCM_MULTICAST_TYPE_L2;
    } else if (_BCM_MULTICAST_IS_VLAN(group) || 
            _BCM_MULTICAST_IS_SUBPORT(group)) {
        /* subport type is for backward compatiable. */
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                &mc_group_count));
        type_flag = BCM_MULTICAST_TYPE_VLAN;
    } else {
        return BCM_E_PARAM;
    }

    /* check valid ID and report the flag */
    if (mc_index >= mc_group_count) {
        return BCM_E_BADID;
    } else {
        /* check if ID is existed */
        if (_bcm_robo_l2mc_id_check(unit, (int)mc_index) == TRUE){
            *flags = type_flag | BCM_MULTICAST_WITH_ID;
        } else {
            return BCM_E_NOT_FOUND;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_multicast_group_is_free
 * Purpose:
 *      Request if the given multicast group is available on the
 *      device
 * Parameters:
 *      unit - (IN) Unit number.
 *      group - (IN) Multicast group ID.
 * Returns:
 *      BCM_E_NONE - multicast group is valid and available on the device
 *      BCM_E_EXISTS - multicast group is valid but already in use
 *                     on this device
 *      BCM_E_PARAM - multicast group is not valid on this device 
 * Notes:
 */
int 
bcm_robo_multicast_group_is_free(int unit, bcm_multicast_t group)
{
    int             mc_index;
    int             rv = BCM_E_PARAM;
    int             is_set;

    mc_index = _BCM_MULTICAST_ID_GET(group);
    L2MC_ID(unit, mc_index);
    if (_BCM_MULTICAST_IS_L2(group) || _BCM_MULTICAST_IS_VLAN(group)||
         _BCM_MULTICAST_IS_SUBPORT(group)) {
        is_set = _bcm_robo_l2mc_id_check(unit, (int)mc_index);
        if(is_set) {
            rv = BCM_E_EXISTS;
        } else {
            rv = BCM_E_NONE;   
        }
    } 
    return rv;
}
/*
 * Function:
 *      bcm_robo_multicast_group_free_range_get
 * Purpose:
 *      Retrieve the minimum and maximum unallocated multicast groups
 *      for a given multicast type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      type_flag - (IN) One of BCM_MULTICAST_TYPE_*.
 *      group_min - (OUT) Minimum available multicast group of specified type.
 *      group_max - (OUT) Maximum available multicast group of specified type.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_group_free_range_get(int unit, uint32 type_flag, 
                                       bcm_multicast_t *group_min,
                                       bcm_multicast_t *group_max)
{
    bcm_multicast_t dev_min, dev_max, group, free_min, free_max;
    int             group_type;
    int             rv = BCM_E_PARAM;
    uint32          type, mc_group_count = 0;    

    type = type_flag & (BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_TYPE_VLAN | 
            BCM_MULTICAST_TYPE_SUBPORT);
    if (1 != _shr_popcount(type)) {
        return BCM_E_PARAM;
    }
    if (type_flag == BCM_MULTICAST_TYPE_L2) {
        /* coverity[CONSTANT_EXPRESSION_RESULT] : FALSE */
        if (SOC_MEM_IS_VALID(unit, INDEX(MARL_PBMPm))) {
            mc_group_count = soc_robo_mem_index_count(unit, INDEX(MARL_PBMPm));
            group_type = _BCM_MULTICAST_TYPE_L2;
        } else {
            return BCM_E_UNAVAIL;
        }

    } else {
        /* BCM_MULTICAST_TYPE_VLAN ||BCM_MULTICAST_TYPE_SUBPORT */
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                &mc_group_count));

        group_type = _BCM_MULTICAST_TYPE_VLAN;

    }
    _BCM_MULTICAST_GROUP_SET(dev_min, group_type, 0);
    _BCM_MULTICAST_GROUP_SET(dev_max, group_type, mc_group_count);

    free_min = free_max = 0; /* Invalid multicast group */
    for (group = dev_min; group < dev_max; group++) {

        rv = bcm_robo_multicast_group_is_free(unit, group);

        if (BCM_SUCCESS(rv)) {
            if (0 == free_min) {
                free_min = group;
            }
            free_max = group;
        } else if (BCM_E_EXISTS == rv) {
            /* Nothing to do but clear the error */
            rv = BCM_E_NONE;
        } else {
            /* Real error, return */
            break;
        }
    }
    if (BCM_SUCCESS(rv)) {
        if (0 == free_min) {
            /* No available groups of this type */
            return BCM_E_NOT_FOUND;
        } else {
            /* Copy the results */
            *group_min = free_min;
            *group_max = free_max;
        }
    }

    return rv; 
    
}
/*
 * Function:
 *      bcm_robo_multicast_group_traverse
 * Purpose:
 *      Iterate over the defined multicast groups of the type
 *      specified in 'flags'.  If all types are desired, use
 *      MULTICAST_TYPE_MASK.
 * Parameters:
 *      unit - (IN) Unit number.
 *      trav_fn - (IN) Callback function.
 *      flags - (IN) BCM_MULTICAST_*
 *      user_data - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_group_traverse(int unit,
                                 bcm_multicast_group_traverse_cb_t trav_fn, 
                                 uint32 flags, void *user_data)
{
    int mc_index;
    uint32 group_flags, flags_mask; 
    uint32 mc_group_count = 0;
    bcm_multicast_t group;

    flags_mask = BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_TYPE_VLAN | 
            BCM_MULTICAST_TYPE_SUBPORT;
    
    if (0 == (flags & flags_mask)) {
        /* No recognized multicast types to traverse */
        return BCM_E_PARAM;
    }
    flags &= flags_mask;

    if (flags & BCM_MULTICAST_TYPE_L2) {
        mc_group_count = soc_robo_mem_index_count(unit, INDEX(MARL_PBMPm));
        group_flags = BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_WITH_ID;

        for (mc_index = 0; mc_index < mc_group_count; mc_index++) {
            if (_bcm_robo_l2mc_id_check(unit, mc_index) == TRUE){
                _BCM_MULTICAST_GROUP_SET(group, 
                        _BCM_MULTICAST_TYPE_L2, mc_index);
                BCM_IF_ERROR_RETURN
                        ((*trav_fn)(unit, group, group_flags, user_data));
            }
        }
    }

    if ((flags & BCM_MULTICAST_TYPE_VLAN) || 
            (flags & BCM_MULTICAST_TYPE_SUBPORT)) {
        /* subport type is for backward compatiable. */
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                &mc_group_count));
        group_flags = BCM_MULTICAST_TYPE_VLAN | BCM_MULTICAST_WITH_ID;

        for (mc_index = 0; mc_index < mc_group_count; mc_index++) {
            if (_bcm_robo_l2mc_id_check(unit, mc_index) == TRUE){
                _BCM_MULTICAST_GROUP_SET(group, 
                        _BCM_MULTICAST_TYPE_VLAN, mc_index);
                BCM_IF_ERROR_RETURN
                        ((*trav_fn)(unit, group, group_flags, user_data));
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_multicast_repl_set
 * Purpose:
 *      Assign set of VLANs provided to port's replication list for chosen
 *      L2 multicast group.
 * Parameters:
 *      unit     - Switch device unit number.
 *      mc_index  - The index number.
 *      port     - port to list.
 *      vlan_vec - (IN) vector of replicated VLANs common to selected ports.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_multicast_repl_set(int unit, int mc_index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
#ifdef  BCM_TB_SUPPORT
    int rv = BCM_E_NONE;
    int vport_idx;
    bcm_vlan_t vid;
    int found = 0;
    int empty_slot_idx = -1;
    uint32 vport_bmp = 0;
    static _bcm_robo_mcast_repl_t old_mcast_repl_cfg;
    static _bcm_robo_mcast_repl_vlan_t old_mcast_repl_vlan;
    bcm_vlan_vector_t vlan_vec_check;
    int mcast_repl_en = FALSE;
    int len = 0;
    int ctrl_val = 0;
    uint32 ctrl_cnt = 0;
    uint32 ctrl_type = 0;
    bcm_vlan_vector_t vlan_vec_add;
    bcm_vlan_vector_t vlan_vec_del;
    
    if (SOC_IS_TBX(unit)) {
        /* Verify parameters */
        rv = _mcast_repl_param_check(unit, mc_index, port);
        BCM_IF_ERROR_RETURN(rv);

        /* Return if the set doesn't change anything */
        BCM_VLAN_VEC_ZERO(vlan_vec_check);
        for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
            if (MCAST_REPL_USED_ISSET(unit, mc_index, port, vport_idx)) {
                vid = MCAST_REPL_VID_GET(unit, port, vport_idx);
                BCM_VLAN_VEC_SET(vlan_vec_check, vid);
            }
        }
        if (!sal_memcmp(vlan_vec_check, vlan_vec, sizeof(bcm_vlan_vector_t))) {
            /* Nothing changed for the pair of (mc_index, port) */
            return BCM_E_NONE;
        }
        
        BCM_VLAN_VEC_ZERO(vlan_vec_add);
        BCM_VLAN_VEC_ZERO(vlan_vec_del);
        /* Prepare the vid lists that going to be added and deleted */
        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec, vid) && !BCM_VLAN_VEC_GET(vlan_vec_check, vid)) {
                /* The vid is going to be added this time */
                BCM_VLAN_VEC_SET(vlan_vec_add, vid);
            }
            if (!BCM_VLAN_VEC_GET(vlan_vec, vid) && BCM_VLAN_VEC_GET(vlan_vec_check, vid)) {
                /* The vid is going to be deleted this time */
                BCM_VLAN_VEC_SET(vlan_vec_del, vid);
            }
        }

        /* 
          * Save a copy of original software configuration,
          * in case of error return and needs to recover.
          */
        sal_memcpy(&old_mcast_repl_cfg, robo_mcast_repl_config[unit], 
                              sizeof(old_mcast_repl_cfg));
        sal_memcpy(&old_mcast_repl_vlan, robo_mcast_repl_vid[unit], 
                              sizeof(_bcm_robo_mcast_repl_vlan_t));

        /* Do deletion first. In order to clean up the unused vports. */
        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec_del, vid)) {
                for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
                    if (MCAST_REPL_USED_ISSET(unit, mc_index, port, vport_idx) &&
                         MCAST_REPL_VID_GET(unit, port, vport_idx) == vid) {
                        MCAST_REPL_USED_CLR(unit, mc_index, port, vport_idx);
                        MCAST_REPL_VID_CNT_DEC(unit, port, vport_idx);
                    }
                }
            }
        }

        /* Then do addition */
        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec_add, vid)) {
                found = 0;
                empty_slot_idx = -1;
                for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
                    if (MCAST_REPL_VID_CNT_IS_ZERO(unit, port, vport_idx)) {
                        /* Look for the first empty slot in case it needed later */
                        if (empty_slot_idx < 0) {
                            empty_slot_idx = vport_idx;
                        }
                    } else {
                        if (vid == MCAST_REPL_VID_GET(unit, port, vport_idx)) {
                            /* Check if the new vid is already in the repl list of the physical port */
                            found = 1;
                            break;
                        }
                    }
                }

                if (found) {
                    MCAST_REPL_USED_SET(unit, mc_index, port, vport_idx);
                    MCAST_REPL_VID_CNT_INC(unit, port, vport_idx);
                } else {
                    /* Put it in the first empty slot */
                    if (empty_slot_idx < 0) {
                        /* 
                          * No empty slot for the port's vport replication list 
                          * Restore the original configuration and return.
                          */
                        sal_memcpy(robo_mcast_repl_config[unit], 
                                              &old_mcast_repl_cfg, 
                                              sizeof(old_mcast_repl_cfg));
                        sal_memcpy(robo_mcast_repl_vid[unit], 
                                              &old_mcast_repl_vlan, 
                                              sizeof(_bcm_robo_mcast_repl_vlan_t));
                        return BCM_E_RESOURCE;
                    } else {
                        MCAST_REPL_USED_SET(unit, mc_index, port, empty_slot_idx);
                        MCAST_REPL_VID_SET(unit, port, empty_slot_idx, vid);
                        MCAST_REPL_VID_CNT_INC(unit, port, empty_slot_idx);
                    }
                }
            }
        }


        /* Update hardware tables */ 
        vport_bmp = 0;
        for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
            if (MCAST_REPL_USED_ISSET(unit, mc_index, port, vport_idx)) {
                vport_bmp |= (1 << vport_idx);
            }
        }

        if (DRV_MCREP_VPGRP_VPORT_CONFIG_SET(unit, mc_index, port,
                DRV_MCREP_VPGRP_OP_VPORT_MEMBER, (int *)&vport_bmp)){
            return BCM_E_INTERNAL;
        }

        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
                for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
                    if ((MCAST_REPL_VID_GET(unit, port, vport_idx) == vid) &&
                        (!MCAST_REPL_VID_CNT_IS_ZERO(unit, port, vport_idx))) {
                        /* Update Virtual Port VID table */
                        /* Set the replicated vlan id */
                        if (DRV_MCREP_VPORT_CONFIG_SET(unit, port, 
                                DRV_MCREP_VPORT_OP_VID, vport_idx, vid)) {
                            return BCM_E_INTERNAL;
                        }
                        if (vid == BCM_VLAN_UNTAG) {
                            /* Mark the untagged replication */
                            if (DRV_MCREP_VPORT_CONFIG_SET(unit, port, 
                                    DRV_MCREP_VPORT_OP_UNTAG_VP, vport_idx, 0)){
                                return BCM_E_INTERNAL;
                            }
                        }
                    }
                }
            }
        }

        /* Enable/Disable Multicast Replication feature if necessary */
        sal_memset(&old_mcast_repl_cfg, 0, sizeof(_bcm_robo_mcast_repl_t));
        len = sizeof(_bcm_robo_mcast_repl_vport_id_t)*_MAX_REPL_CFG_INDEX_NUM;
        if (!sal_memcmp(old_mcast_repl_cfg.repl_index, 
                                  robo_mcast_repl_config[unit]->repl_index, 
                                  len)) {
            /* all mcast replication configuration not extsted*/
            mcast_repl_en = FALSE;
        } else {
            mcast_repl_en = TRUE;
        }

        if (mcast_repl_en != robo_mcast_repl_config[unit]->mcast_repl_enable) {
            /* Enable status changes */
            ctrl_cnt = 1;
            ctrl_type = DRV_DEV_CTRL_MCASTREP;
            ctrl_val = mcast_repl_en;
            rv = DRV_DEV_CONTROL_SET(unit, &ctrl_cnt, &ctrl_type, &ctrl_val);
            if (rv) {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s, %d, SOC Error!\n"), 
                          FUNCTION_NAME(), __LINE__));
                return rv;
            }
            robo_mcast_repl_config[unit]->mcast_repl_enable = mcast_repl_en;
        }
        return BCM_E_NONE;
    }
#endif   /* BCM_TB_SUPPORT */        

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_multicast_repl_get
 * Purpose:
 *      Return set of VLANs selected for port's replication list for chosen
 *      L2 multicast group.
 * Parameters:
 *      unit     - Switch device unit number.
 *      index    - The index number.
 *      port     - port for which to retrieve info.
 *      vlan_vec - (OUT) vector of replicated VLANs common to selected ports.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_repl_get(int unit, int index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
#ifdef  BCM_TB_SUPPORT
    int rv = BCM_E_NONE;
    int vport_idx;
    bcm_vlan_t vlan_id;

    if (SOC_IS_TBX(unit)) {
        /* Verify parameters */
        rv = _mcast_repl_param_check(unit, index, port);
        BCM_IF_ERROR_RETURN(rv);

        BCM_VLAN_VEC_ZERO(vlan_vec);

        for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
            if (MCAST_REPL_USED_ISSET(unit, index, port, vport_idx)) {
                vlan_id = MCAST_REPL_VID_GET(unit, port, vport_idx);
                BCM_VLAN_VEC_SET(vlan_vec, vlan_id);
            }
        }
        
        return BCM_E_NONE;
    }
#endif   /* BCM_TB_SUPPORT */        

    return BCM_E_UNAVAIL;
}

