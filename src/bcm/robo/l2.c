/*
 * $Id: l2.c,v 1.160 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * L2 - Broadcom RoboSwitch Layer-2 switch API.
 */
#include <shared/bsl.h>

#include <soc/mem.h>
#include <soc/drv.h>

#include <soc/l2x.h>
#include <soc/feature.h>
#include <soc/macipadr.h>
#include <soc/debug.h>
#include <soc/arl.h>
#include <soc/robo/mcm/memregs.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/l2.h>
#include <bcm/trunk.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/switch.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/robo/l2.h>
#include <bcm_int/robo/trunk.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/subport.h>
#include <bcm_int/robo_dispatch.h>

/****************************************************************
 *
 * L2 software tables, per unit.
 *
 ****************************************************************/
static int _l2_init[BCM_MAX_NUM_UNITS];

/*
 * Define:
 *    L2_INIT
 * Purpose:
 *    Causes a routine to return BCM_E_INIT (or some other
 *    error) if L2 software module is not yet initialized.
 */

#define L2_INIT(unit) do { \
    if (_l2_init[unit] < 0) { \
        LOG_CLI((BSL_META("%s\n"),FUNCTION_NAME()));\
        return _l2_init[unit];} \
    if (_l2_init[unit] == 0) { \
        LOG_CLI((BSL_META("%s\n"),FUNCTION_NAME()));\
        return BCM_E_INIT;} \
    } while (0);


/*
 * Function:
 *    _bcm_robo_l2_gport_parse
 * Description:
 *    Parse gport in the l2 table
 * Parameters:
 *    unit -      [IN] ROBOSwitch PCI device unit number (driver internal).
 *  l2addr -    [IN/OUT] l2 addr structure to parse and fill
 *  params -  [OUT] report the value per gport type indicated.
 * Returns:
 *    BCM_E_XXX
 */
int 
_bcm_robo_l2_gport_parse(int unit, bcm_l2_addr_t *l2addr, 
        _bcm_robo_l2_gport_params_t *params)
{
    int             id = 0;
    bcm_port_t      _port;
    bcm_trunk_t     _trunk;
    bcm_module_t    _modid;

    if ((NULL == l2addr) || (NULL == params)){
        return BCM_E_PARAM;
    }

    params->param0 = -1;
    params->param1 = -1;
    params->type = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_gport_resolve(unit, l2addr->port, &_modid, &_port, 
                               &_trunk, &id));

    if ((-1 != _port) && (-1 == id)) {
        params->param0 = _port;
        if (_port == CMIC_PORT(unit)) {
            params->type = _SHR_GPORT_TYPE_LOCAL_CPU;
            return BCM_E_NONE;
        }
        params->param1 = _modid;
        params->type = _SHR_GPORT_TYPE_MODPORT;
        return BCM_E_NONE;
    }

    if (-1 != id) {
        if (BCM_GPORT_IS_SUBPORT_GROUP(l2addr->port)) {
            /* for ROBO device with SUBPORT_GROUP_TYPE, the "id" after the 
             * gport routine, "_bcm_robo_gport_resolve", will be mc_group id.
             */
            params->type = _SHR_GPORT_TYPE_SUBPORT_GROUP;
            params->param0 = _port;
            params->param1 = id;    /* mc_group id */
            
        } else if (BCM_GPORT_IS_SUBPORT_PORT(l2addr->port)){
            /* for ROBO device with SUBPORT_GROUP_TYPE, the "id" after the 
             * gport routine, "_bcm_robo_gport_resolve", will be vport id.
             */
            params->type = _SHR_GPORT_TYPE_SUBPORT_PORT;
            params->param0 = _port;
            params->param1 = id;    /* vp_id */
        } 
        return BCM_E_NONE;
    }

    return BCM_E_PORT;
}

/* 
 * Function:
 *    _bcm_robo_l2_to_arl
 * Purpose:
 *    Translate hardware-independent L2 entry to ROBO-specific ARL entry
 * Parameters:
 *    unit - Unit number
 *    arl_entry - (OUT) ROBO ARL entry
 *    l2addr - Hardware-independent L2 entry
 */
int
_bcm_robo_l2_to_arl(int unit, l2_arl_sw_entry_t *arl_entry, 
        bcm_l2_addr_t *l2addr)
{
    
    uint64  mac_field;
    uint32  fld_value = 0;
    uint32  no_support_flag = 0;
    uint32  age_hit_val = 0;
    _bcm_robo_l2_gport_params_t  g_params;

    sal_memset(arl_entry, 0, sizeof (*arl_entry));
    COMPILER_64_ZERO(mac_field);

    LOG_INFO(BSL_LS_BCM_TESTS,
             (BSL_META_U(unit,
                         "%s, flags is 0x%x\n"), 
              FUNCTION_NAME(), l2addr->flags));

    no_support_flag = BCM_L2_DISCARD_SRC | BCM_L2_DISCARD_DST | 
            BCM_L2_COPY_TO_CPU | BCM_L2_L3LOOKUP | BCM_L2_TRUNK_MEMBER | 
            BCM_L2_MIRROR | BCM_L2_LEARN_LIMIT_EXEMPT;
    
    if (soc_feature(unit, soc_feature_arl_mode_control)) {
        no_support_flag &= ~BCM_L2_DISCARD_SRC;
        no_support_flag &= ~BCM_L2_DISCARD_DST;
        no_support_flag &= ~BCM_L2_COPY_TO_CPU;
    }

    if (l2addr->flags & no_support_flag) {
        return BCM_E_UNAVAIL;
    }

    SAL_MAC_ADDR_TO_UINT64(l2addr->mac, mac_field);
    /* set MAC field */
    
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_MAC, (void *)arl_entry, (void *)&mac_field));

    /* valid VLAN check */
    if(l2addr->vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }
    
    /* set VID field */
    fld_value = l2addr->vid; /* vid value */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VLANID, (void *)arl_entry, &fld_value));

    /* valid 802.1p CoS check */
    if(l2addr->cos_dst < BCM_PRIO_MIN || l2addr->cos_dst > BCM_PRIO_MAX){
        return BCM_E_PARAM;
    }
    
    if (!SOC_IS_TBX(unit)){
        fld_value = l2addr->cos_dst; /* priority value */
         /* set priority field */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL,
                DRV_MEM_FIELD_PRIORITY, (void *)arl_entry, &fld_value));
    }

    /* set static field */
    if (l2addr->flags & BCM_L2_STATIC) {
        fld_value = 0x1; /* static value */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)arl_entry, &fld_value));
        LOG_INFO(BSL_LS_BCM_TESTS,
                 (BSL_META_U(unit,
                             "\t Static flags set in l2addr\n")));
    }

    /* set valid field */
    fld_value = 0x1; /* valid value */
    if (soc_feature(unit, soc_feature_l2_pending)) {
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
            fld_value = (l2addr->flags & BCM_L2_PENDING) ? 
                    _TB_ARL_STATUS_PENDING : _TB_ARL_STATUS_VALID;
#endif
        } else {
            LOG_INFO(BSL_LS_BCM_TESTS,
                     (BSL_META_U(unit,
                                 "\t No Pending learning feature!!\n")));
            return BCM_E_UNAVAIL;
        }
    }
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VALID, (uint32 *)arl_entry, &fld_value));

    BCM_IF_ERROR_RETURN(
            DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_AGE_HIT_VALUE, &age_hit_val));

    if ((l2addr->flags & BCM_L2_HIT) || (l2addr->flags & BCM_L2_SRC_HIT)) {
        fld_value = age_hit_val; /* age value */
        /* set hit/age field */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                (uint32 *)arl_entry, &fld_value));
        LOG_INFO(BSL_LS_BCM_TESTS,
                 (BSL_META_U(unit,
                             "\t Age flags set in l2addr\n")));
    
    }

    if (soc_feature(unit, soc_feature_arl_mode_control)) {

        /* the arl_mode_control feature for ROBO chips were designed to serve
         *  the control on one specific mode only for each l2 entry.
         *  - That is, not like ESW design, to request one mode control modes  
         *      on ROBO chips will be treated as conflict configuration.
         */
        if (l2addr->flags & BCM_L2_COPY_TO_CPU) {
            if ((l2addr->flags & BCM_L2_DISCARD_DST) || 
                    (l2addr->flags & BCM_L2_DISCARD_SRC)){
                return (BCM_E_CONFIG);
            } else {
                fld_value = 0x3; 
            }
        } else if (l2addr->flags & BCM_L2_DISCARD_DST) {
            if (l2addr->flags & BCM_L2_DISCARD_SRC){
                return (BCM_E_CONFIG);
            } else {
                fld_value = 0x1; 
            }
        }else if (l2addr->flags & BCM_L2_DISCARD_SRC) {
            fld_value = 0x2;
        }else {
            fld_value = 0; 
        }

        /* set arl_control field */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_ARL_CONTROL, (uint32 *)arl_entry, &fld_value));
        LOG_INFO(BSL_LS_BCM_TESTS,
                 (BSL_META_U(unit,
                             "\t arl_control flags set in l2addr %x\n"),
                  fld_value));
    }

    /* Destination translate section :
     *  - Mcast : l2mc_group or PBMP
     *  - Unicast : port_id
     */
    if (BCM_MAC_IS_MCAST(l2addr->mac)){
        /* Mcast */
        if (l2addr->flags & BCM_L2_MCAST) {
            if (_BCM_MULTICAST_IS_SET(l2addr->l2mc_group)) {
                if (_BCM_MULTICAST_IS_L2(l2addr->l2mc_group)) {
                    fld_value = _BCM_MULTICAST_ID_GET(l2addr->l2mc_group);
                } else {
                    /* No other multicast types in L2 on this device */
                    return BCM_E_PARAM;
                }
            } else {
                fld_value = l2addr->l2mc_group;
            }
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit,
                    DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP,
                    (uint32 *)arl_entry, &fld_value));
        }

        /* for those ROBO chips have no MARL_PBMP table (GE switch devices), 
         *  the DEST_PBMP field will remaining at 0x0 in this process.
         */
    }else {
        /* unicast */

        /* port parsing : GPORT(LOCAL|SUBPORT)/NON-GPORT */
        if (!BCM_GPORT_IS_SET(l2addr->port)) {
            LOG_INFO(BSL_LS_BCM_TESTS,
                     (BSL_META_U(unit,
                                 "%s,%d, port is %d\n"),
                      FUNCTION_NAME(), __LINE__, l2addr->port));
            if (!SOC_PORT_ADDRESSABLE(unit, l2addr->port)) {
                return BCM_E_PORT;
            }
            
            /* set Port fields */
            fld_value = l2addr->port;
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_entry, &fld_value));
        } else {
            BCM_IF_ERROR_RETURN(
                    _bcm_robo_l2_gport_parse(unit, l2addr, &g_params));
            
            switch (g_params.type) {
                case _SHR_GPORT_TYPE_LOCAL_CPU:
                case _SHR_GPORT_TYPE_MODPORT:
                    if (!SOC_PORT_ADDRESSABLE(unit, g_params.param0)) {
                        return BCM_E_PORT;
                    }
                    /* port */
                    fld_value = g_params.param0;
                    LOG_INFO(BSL_LS_BCM_TESTS,
                             (BSL_META_U(unit,
                                         "%s,%d, port is %d\n"),
                              FUNCTION_NAME(), __LINE__, fld_value));
                    /* set Port fields */
                    BCM_IF_ERROR_RETURN(
                            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_entry, 
                            &fld_value));
                    break;
                case _SHR_GPORT_TYPE_SUBPORT_GROUP:
                    /* with this type, param0 is port and param1 is mc_group */ 
                    /* mcast group_id */
                    fld_value = g_params.param1;
                    LOG_INFO(BSL_LS_BCM_TESTS,
                             (BSL_META_U(unit,
                                         "%s,%d, port is %d\n"),
                              FUNCTION_NAME(), __LINE__, fld_value));
                    /* set mgid fields */
                    BCM_IF_ERROR_RETURN(
                            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)arl_entry, 
                            &fld_value));
                    break;
                case _SHR_GPORT_TYPE_SUBPORT_PORT:
                    /* with this type, param0 is port and param1 is vport_id */ 
                    if (!SOC_PORT_ADDRESSABLE(unit, g_params.param0)) {
                        return BCM_E_PORT;
                    }
                    /* port */
                    fld_value = g_params.param0;
                    LOG_INFO(BSL_LS_BCM_TESTS,
                             (BSL_META_U(unit,
                                         "%s,%d, port is %d\n"),
                              FUNCTION_NAME(), __LINE__, fld_value));
                    /* set Port fields */
                    BCM_IF_ERROR_RETURN(
                            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_entry, 
                            &fld_value));
                            
                    if(SOC_IS_TBX(unit)){
                        /* vp_id is the item in bcm_l2_addr_t to support TB's
                         *  ARL feature of vport_id field.
                         *
                         * Note : 
                         *  TB's vport_id is 0-15 for each physical port. the 
                         *  0-15 vp_id can be retrieved from gport format 
                         *  vport_id(system basis vport_id)
                         */
#ifdef  BCM_TB_SUPPORT
                        /* vport */
                        fld_value = g_params.param1;
                        LOG_INFO(BSL_LS_BCM_TESTS,
                                 (BSL_META_U(unit,
                                             "%s,%d, virtual port is %d\n"),
                                  FUNCTION_NAME(), __LINE__, fld_value));
                        /* set Port fields */
                        BCM_IF_ERROR_RETURN(
                                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                                DRV_MEM_FIELD_VPORT, (uint32 *)arl_entry, 
                                &fld_value));
#endif  /* BCM_TB_SUPPORT */
                    }
                    break;
                default :
                    return BCM_E_PORT;
            }
        }
    }
    LOG_INFO(BSL_LS_BCM_TESTS,
             (BSL_META_U(unit,
                         "%s,%d,arl_entry[2-0]=%08x-%08x-%08x\n"), 
              FUNCTION_NAME(), __LINE__, *((uint32 *)arl_entry+2), 
              *((uint32 *)arl_entry+1),*(uint32 *)arl_entry));
    return BCM_E_NONE;
}

/*
 * Function:
 *    _bcm_robo_l2_from_arl
 * Purpose:
 *    Translate ROBO-specific ARL entry to hardware-independent L2 entry
 * Parameters:
 *    unit - Unit number
 *    l2addr - (OUT) hardware-independent L2 entry
 *    arl_entry - ROBO ARL entry
 */
void
_bcm_robo_l2_from_arl(int unit, 
                bcm_l2_addr_t *l2addr, l2_arl_sw_entry_t *arl_entry)
{
    int     rv = BCM_E_NONE;
    uint64 mac_field;
    uint32 field_value = 0, src_port = 0;
    uint32 age_hit_val = 0;
    bcm_gport_t l2_gport = BCM_GPORT_INVALID;

    _bcm_gport_dest_t   dest;
    int     useGport = 0;

    /* keep the l2addr->port for the usage on verifying if any GPORT type 
     * indicated to represent user's request.
     */
    src_port = l2addr->port;
    sal_memset(l2addr, 0, sizeof (bcm_l2_addr_t));
    l2addr->port = src_port;
    
    COMPILER_64_ZERO(mac_field);

    LOG_INFO(BSL_LS_BCM_TESTS,
             (BSL_META_U(unit,
                         "%s,%d,arl_entry[2-0]=%08x-%08x-%08x\n"), 
              FUNCTION_NAME(), __LINE__, *((uint32 *)arl_entry+2), 
              *((uint32 *)arl_entry+1),*(uint32 *)arl_entry));
    
    /* Get MAC field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
            (uint32 *)arl_entry, (void *)&mac_field);

    SAL_MAC_ADDR_FROM_UINT64(l2addr->mac, mac_field);

    LOG_INFO(BSL_LS_BCM_TESTS, \
             (BSL_META_U(unit, \
                         "in _bcm_robo_l2_from_arl ,\
                         mac  is %02x-%02x-%02x-%02x-%02x-%02x \n"),
              l2addr->mac[0], l2addr->mac[1],l2addr->mac[2], l2addr->mac[3],
              l2addr->mac[4], l2addr->mac[5]));

    /* Get VID field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VLANID, (uint32 *)arl_entry, &field_value);
    l2addr->vid = field_value;
    LOG_INFO(BSL_LS_BCM_TESTS,
             (BSL_META_U(unit,
                         "in _bcm_robo_l2_from_arl, l2addr->vid is %d\n"),
              l2addr->vid));

    /* Get entry status field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VALID, (uint32 *)arl_entry, &field_value);
    if (soc_feature(unit, soc_feature_l2_pending)) {
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
                    DRV_MEM_FIELD_VALID, (uint32 *)arl_entry, &field_value);
            if (field_value == _TB_ARL_STATUS_PENDING) {
                l2addr->flags |= BCM_L2_PENDING;
            }
#endif /* BCM_TB_SUPPORT */
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "\t No Pending learning feature!!\n")));
            return;
        }
    }

    /* Get static field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_STATIC, (uint32 *)arl_entry, &field_value);
    if (field_value) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    if (l2addr->mac[0] & 0x01) { /* mcast address */
        l2addr->flags |= BCM_L2_MCAST;
        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
            (uint32 *)arl_entry, &field_value);
        if (field_value) {
            l2addr->l2mc_group= (int)field_value;               
        }
        LOG_INFO(BSL_LS_BCM_TESTS,
                 (BSL_META_U(unit,
                             "in _bcm_robo_l2_from_arl, l2mc_group is %d\n"),
                  l2addr->l2mc_group));

    } else {
        rv = bcm_robo_switch_control_get(unit, 
                    bcmSwitchUseGport, &useGport);
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "fail to get bcmSwitchUseGport\n")));
            return;
        }
        /* set Port */
        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)arl_entry, &field_value);
        src_port = field_value;

        if (useGport){
            /* if user request any GPORT type to report(in l2addr.port) 
             *      - GPORT_MODPORT : supported on all ROBO chips.
             *      - GPORT_SUBPOPRT : supported on TB only.
             *      - other GPORT : not support.
             * else (no GPORT requested in l2addr.port)
             *      - report GPORT_MODPORT type due to useGport in SDK.
             */
             
            _bcm_robo_gport_dest_t_init(&dest);
            /* assigning the default GPORT type to report l2 entry */
            dest.port = src_port;
            dest.modid = 0;
            dest.gport_type = BCM_GPORT_TYPE_MODPORT;
            
             if (BCM_GPORT_IS_SET(l2addr->port)){
                if (BCM_GPORT_IS_SUBPORT_PORT(l2addr->port)){
                    if (soc_feature(unit, soc_feature_subport)){
#ifdef  BCM_TB_SUPPORT
                        uint32  vp_id;
                        
                        LOG_INFO(BSL_LS_BCM_TESTS,
                                 (BSL_META_U(unit,
                                             "%s,src_port=%d,l2_gport=0x%08x\n"), 
                                  FUNCTION_NAME(), src_port, l2_gport));
                        /* vp_id is a new item in bcm_l2_addr_t to support 
                         *   TB's new feature in ARL entry of vport_id field.
                         */
                         
                        /* get VPort fields */
                        DRV_MEM_FIELD_GET(unit, 
                                DRV_MEM_ARL, DRV_MEM_FIELD_VPORT, 
                                (uint32 *)arl_entry, &field_value);
                        vp_id = field_value;
                        LOG_INFO(BSL_LS_BCM_TESTS,
                                 (BSL_META_U(unit,
                                             "in _bcm_robo_l2_from_arl, vport is %d\n"),
                                  vp_id));
                        
                        dest.subport_id= 
                                _TB_SUBPORT_SYSTEM_ID_SET(src_port, 0, vp_id);
                        dest.gport_type = BCM_GPORT_SUBPORT_PORT;
#else   /* BCM_TB_SUPPORT */
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s, GPORT_SUBPORT type is unavailable!\n"), 
                                  FUNCTION_NAME()));
#endif  /* BCM_TB_SUPPORT */
                    } else {
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s, GPORT_SUBPORT type is unavailable!\n"), 
                                  FUNCTION_NAME()));
                    }
                } else {
                    /* warning message for the incomming GPORT is out of 
                     * ROBO supported L2 GPORT types(i.e. MODPORT and SUBPORT)
                     */
                    if (!BCM_GPORT_IS_MODPORT(l2addr->port)) {
                        /* other GPORT type */
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s,requesting a unavailable GPORT(0x%x)\n"), 
                                  FUNCTION_NAME(), l2addr->port));
                    }
                }
             }
             
             rv = _bcm_robo_gport_construct(unit, 
                     &dest, &l2_gport);
             l2addr->port = l2_gport;
            
        } else {
            /* user didn't force to use GPORT in SDK */

            /* TB here will be reported as normal port format(no vport) */
            l2addr->port = src_port;     /* no GPORT type */
        }

        if (!SOC_IS_TBX(unit)){
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)arl_entry, &field_value);
            l2addr->cos_dst = field_value;
        }
  

        DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_AGE_HIT_VALUE, &age_hit_val);

        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
            (uint32 *)arl_entry, &field_value);
        if (field_value == age_hit_val) {

            /* ROBO' hit operation(source hit) can be worked on non-static 
             *  L2 entry only.
             */
            l2addr->flags |= BCM_L2_HIT | BCM_L2_SRC_HIT;
        }
        if (soc_feature(unit, soc_feature_arl_mode_control)) {
            /* Get arl control field */
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL,
                    (uint32 *)arl_entry, &field_value);

            if (field_value == 0x1) {
                l2addr->flags |= BCM_L2_DISCARD_DST;
            }else if(field_value == 0x2){
                l2addr->flags |= BCM_L2_DISCARD_SRC;        
            }else if(field_value == 0x3){
                l2addr->flags |= BCM_L2_COPY_TO_CPU;
            }
        }

        LOG_INFO(BSL_LS_BCM_TESTS,
                 (BSL_META_U(unit,
                             "%s, port is 0x%x cos_dst 0x%x\n"), 
                  FUNCTION_NAME(), l2addr->port, l2addr->cos_dst));

    }    
    /* Valid bit is ignored here; entry is assumed valid */
    
}

/*
 * Function:
 *      bcm_l2_port_native
 * Purpose:
 *      Determine if the given port is "native" from the point
 *      of view of L2.
 * Parameters:
 *      unit       - The unit
 *      modid      - Module ID of device
 *      port       - Physical port on the unit
 * Returns:
 *      TRUE (> 0) if (modid, port) is front panel/CPU port for unit.
 *      FALSE (0) otherwise.
 *      < 0 on error.
 *
 *      Native means front panel, but also includes the CPU port.
 *      HG ports are always excluded as are ports marked as internal or
 *      external stacking
 */
int
bcm_robo_l2_port_native(int unit, int modid, int port)
{
    bcm_trunk_t     tgid;
    int             id, isLocal = 0;

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_gport_resolve(unit, 
                port, &modid, &port, &tgid, &id));

        if (-1 != id || BCM_TRUNK_INVALID != tgid) {
            return FALSE;
        }
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_modid_is_local(unit, 
            modid, &isLocal));
    if (isLocal != TRUE) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/*
 * Function:
 *    _bcm_robo_l2_term
 * Description:
 *    Finalize chip-dependent parts of L2 module
 * Parameters:
 *    unit - RoboSwitch unit number.
 */

int
_bcm_robo_l2_term(int unit)
{
    COMPILER_REFERENCE(unit);
    
    return SOC_E_NONE;
}


/****************************************************************************
 *
 * L2 Message Registration
 *
 ****************************************************************************/

static bcm_l2_addr_callback_t _bcm_l2_cbs[SOC_MAX_NUM_SWITCH_DEVICES];
static void *_bcm_l2_cb_data[SOC_MAX_NUM_SWITCH_DEVICES];

/*
 * Function:
 *    _bcm_robo_l2_register_callback
 * Description:
 *    Call back to handle bcm_l2_addr_register clients. 
 * Parameters:
 *    unit - RoboSwitch unit number.
 *    entry_del - Entry to be deleted or updated, NULL if none.
 *    entry_add - Entry to be inserted or updated, NULL if none.
 *    fn_data - unused
 * Notes:
 *    Only one callback per unit to the bcm layer is supported here. 
 *    Multiple bcm client callbacks per unit are supported in the bcm layer. 
 */

static void
_bcm_robo_l2_register_callback(int unit,
              l2_arl_sw_entry_t *entry_del,
              l2_arl_sw_entry_t *entry_add,
              void *fn_data)
{    
    if (_bcm_l2_cbs[unit] != NULL) {
        bcm_l2_addr_t l2addr_del, l2addr_add;
        uint32 flags = 0; /* Common flags: Move, From/to native */

        /* l2addr->port can carry GPORT type to indicate the desired formate.
         *
         *  Note : if the GPORT is forcing to use in SDK (i.e. to assert 
         *      bcmSwitchUseGport), the reported l2addr.port will still be 
         *      no GPORT format.
         */
        if (SOC_IS_TBX(unit)){
            /* Use SUBPORT GPORT type instead of MODPORT GPORT type for 
             * requesting the most detail l2 information(include vport).
             */
            BCM_GPORT_SUBPORT_PORT_SET(l2addr_del.port, 0);
            BCM_GPORT_SUBPORT_PORT_SET(l2addr_add.port, 0);
        } else {
            BCM_GPORT_MODPORT_SET(l2addr_del.port, 0, 0);
            BCM_GPORT_MODPORT_SET(l2addr_add.port, 0, 0);
        }
        
        
        /* First, set up the entries:  decode HW entries and set flags */
        if (entry_del != NULL) {
            _bcm_robo_l2_from_arl(unit, &l2addr_del, entry_del);
        }
        if (entry_add != NULL) {
            _bcm_robo_l2_from_arl(unit, &l2addr_add, entry_add);
        }
        
        if ((entry_del != NULL) && (entry_add != NULL)) { /* It's a move */
            flags |= BCM_L2_MOVE;
            if (SOC_USE_GPORT(unit)) {
                if (l2addr_del.port != l2addr_add.port) {
                    flags |= BCM_L2_MOVE_PORT;
                }
            } else {
                if (l2addr_del.modid != l2addr_add.modid ||
                    l2addr_del.port != l2addr_add.port) {
                    flags |= BCM_L2_MOVE_PORT;
                }
            }
            if (bcm_robo_l2_port_native(unit, l2addr_del.modid,
                                   l2addr_del.port) > 0) {
                flags |= BCM_L2_FROM_NATIVE;
                l2addr_del.flags |= BCM_L2_NATIVE;
            }
            if (bcm_robo_l2_port_native(unit, l2addr_add.modid,
                                   l2addr_add.port) > 0) {
                flags |= BCM_L2_TO_NATIVE;
                l2addr_add.flags |= BCM_L2_NATIVE;
            }
            l2addr_del.flags |= flags;
            l2addr_add.flags |= flags;
        } else if (entry_del != NULL) { /* Age out or simple delete */
            if (bcm_robo_l2_port_native(unit, l2addr_del.modid,
                                   l2addr_del.port) > 0) {
                l2addr_del.flags |= BCM_L2_NATIVE;
            }
        } else if (entry_add != NULL) { /* Insert or learn */
            if (bcm_robo_l2_port_native(unit, l2addr_add.modid,
                                   l2addr_add.port) > 0) {
                l2addr_add.flags |= BCM_L2_NATIVE;
            }
        }
        
        /* The entries are now set up.  Make the callbacks */
        if (entry_del != NULL) {
            _bcm_l2_cbs[unit](unit, &l2addr_del, 0, _bcm_l2_cb_data[unit]);
        }
        if (entry_add != NULL) {
            _bcm_l2_cbs[unit](unit, &l2addr_add, 1, _bcm_l2_cb_data[unit]);
        }
    }

}

/*
 * Function:
 *    _bcm_robo_l2_bpdu_init
 * Description:
 *    Initialize all BPDU addresses to recognize the 802.1D
 *      Spanning Tree address on all chips.
 * Parameters:
 *    unit - unit number (driver internal).
 * Returns:
 *    BCM_E_XXX
 * Notes:
 */

static int
_bcm_robo_l2_bpdu_init(int unit)
{
    int         def_bpdu_id = 0;
    bcm_mac_t   mac;
    bcm_l2_cache_addr_t addr;

    bcm_l2_cache_addr_t_init(&addr);
    addr.flags |= BCM_L2_CACHE_BPDU;

    /* to init the default BPDU addr on ROBO devices. */
    mac[0] = 0x01;
    mac[1] = 0x80;
    mac[2] = 0xc2;
    mac[3] = mac[4] = mac[5] = 0x00;
    
    sal_memcpy(addr.mac, mac, sizeof(bcm_mac_t));
    BCM_IF_ERROR_RETURN(bcm_l2_cache_set(unit, 0, &addr, &def_bpdu_id));
    
    return BCM_E_NONE;
}

#define _BCM_ROBO_CB_ARL_REMOVED  1 /* return value to indicate del occurred */
/*
 * Function:
 *     _bcm_robo_l2_remove_cb
 *
 *  Note :
 */
static int 
_bcm_robo_l2_remove_cb(int unit, bcm_l2_addr_t *l2addr, void *user_data)
{
    int         rv = BCM_E_NONE;
    uint32      del_flags = 0;
    l2_arl_sw_entry_t           arl_delete;

    del_flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID;
    
    /* assigning proper remove action flag on static and pending status */
    if (l2addr->flags & BCM_L2_PENDING){
        del_flags|= DRV_MEM_OP_PENDING;
    }
    if (l2addr->flags & BCM_L2_STATIC){
        del_flags|= DRV_MEM_OP_DELETE_BY_STATIC;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_delete, l2addr));
    BCM_IF_ERROR_RETURN(DRV_MEM_DELETE(unit, 
            DRV_MEM_ARL,(uint32 *)&arl_delete, del_flags));

    rv = _BCM_ROBO_CB_ARL_REMOVED;
    return rv;
}

/*
 * Function:
 *    bcm_53xx_l2_addr_register
 * Description:
 *    Register a callback routine that will be called whenever
 *    an entry is inserted into or deleted from the L2 address table.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    callback - Callback function of type bcm_l2_addr_callback_t.
 *    userdata - Arbitrary value passed to callback along with messages
 * Returns:
 *    BCM_E_NONE        Success, handle valid
 *    BCM_E_MEMORY        Out of memory
 *    BCM_E_INTERNAL        Chip access failure
 */

int
bcm_53xx_l2_addr_register(int unit,
               bcm_l2_addr_callback_t callback,
               void *userdata)
{
    /* this should be managed properly from above */

    if (_bcm_l2_cbs[unit] != NULL || callback != NULL){
        LOG_INFO(BSL_LS_BCM_ARL,
                 (BSL_META_U(unit,
                             "%s, Reasigning the callback service routine!\n"),
                  FUNCTION_NAME()));
    }
    
    _bcm_l2_cbs[unit] = callback;
    _bcm_l2_cb_data[unit] = userdata;
    
    /* return SOC_E_NONE; kram 2004-01-06*/
    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_53xx_l2_addr_unregister
 * Description:
 *    Unregister a previously registered callback routine.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    callback - Same callback function used to register callback
 *    userdata - Same arbitrary value used to register callback
 * Returns:
 *    BCM_E_NONE        Success, handle valid
 *    BCM_E_MEMORY        Out of memory
 *    BCM_E_INTERNAL        Chip access failure
 * Notes:
 *    Both callback and userdata must match from original register call.
 */

int
bcm_53xx_l2_addr_unregister(int unit,
                 bcm_l2_addr_callback_t callback,
                 void *userdata)
{
    /* this should be managed properly from above */
    _bcm_l2_cbs[unit] = NULL;
    _bcm_l2_cb_data[unit] = NULL;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_l2_detach
 * Purpose:
 *      Clean up l2 bcm layer when unit is detached
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    BCM_E_XXX
 */

int
bcm_robo_l2_detach(int unit)
{
    int     frozen = 0;
    
    /* prevent L2 init process when ARL table still frozen, 
     *  - L2_ARLm is LOCK in frozen state
     *  - frozen counter(f->frozen) not clear in frozen state
     */
    BCM_IF_ERROR_RETURN(soc_robo_arl_is_frozen(unit, &frozen));
    if (TRUE == frozen) {
        return BCM_E_BUSY;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_term(unit));
    
    _l2_init[unit] = 0;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_robo_l2_init
 * Description:
 *    Perform required initializations to L2 table.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_robo_l2_init(int unit)
{
    bcm_port_t  port = 0;
    int     rv = BCM_E_NONE;
    int     frozen;
    uint32  arl_mod = 0;
    bcm_l2_learn_limit_t    limit;

    /* prevent L2 init process when ARL table still frozen, 
     *  - L2_ARLm is LOCK in frozen state
     *  - frozen counter(f->frozen) not clear in frozen state
     */
    BCM_IF_ERROR_RETURN(soc_robo_arl_is_frozen(unit, &frozen));
    if (TRUE == frozen) {
        return BCM_E_BUSY;
    }
    
    _l2_init[unit] = 0;
    
    /* Turn off arl aging */
    BCM_IF_ERROR_RETURN(bcm_l2_age_timer_set(unit, 0));
    
    bcm_robo_l2_detach(unit);

    /*
     * Init BPDU station address registers.
     */
    rv = _bcm_robo_l2_bpdu_init(unit);
    if (rv < 0 && rv != BCM_E_UNAVAIL){
        return rv;
    }
    
    /*
     * Init L2 cache
     */
    rv = bcm_l2_cache_init(unit);
    if (rv < 0 && rv != BCM_E_UNAVAIL) {
        return rv;
    }

    soc_robo_arl_unregister(unit, _bcm_robo_l2_register_callback, NULL);
    BCM_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_ARL));
    
    /* ROBO's SW ARL reset due to HW L2 table cleared */
    /* ARL thread init process */
    arl_mod = soc_property_get(unit, spn_L2XMSG_MODE, 1);
    if(arl_mod) {
        BCM_IF_ERROR_RETURN(
                soc_robo_arl_mode_set(unit, ARL_MODE_ROBO_POLL));
    } else {
        BCM_IF_ERROR_RETURN(
                soc_robo_arl_mode_set(unit, ARL_MODE_NONE));
    }

    /* Learn limit init process :
    *
    *   1. counter reset
    *   2. enable feature and set learn limit to max.
    *       (TBX and Harrier chips is feature always enalbed,
    *        but setting max limit value to performing no learn 
    *        limit behavior.)
    */
    if (soc_feature(unit, soc_feature_mac_learn_limit)) {

        bcm_l2_learn_limit_t_init(&limit);
	
        if (SOC_IS_HARRIER(unit)) {
            limit.limit = soc_robo_mem_index_max(unit, INDEX(L2_ARLm));
        } else {
            limit.limit = soc_robo_mem_index_max(unit, INDEX(L2_ARLm)) + 1;
        }
 
        /* set system learn limit */
        if (soc_feature(unit, soc_feature_system_mac_learn_limit)) {
            limit.flags = BCM_L2_LEARN_LIMIT_SYSTEM;
            bcm_l2_learn_limit_set(unit, &limit); 
        }

        /* set port learn limit and Reset per-port SA learning count */
        limit.flags = BCM_L2_LEARN_LIMIT_PORT;
        PBMP_E_ITER(unit, port) {
            limit.port = port;
            bcm_l2_learn_limit_set(unit, &limit); 

            rv = DRV_ARL_LEARN_COUNT_SET
                (unit, port, DRV_PORT_SA_LRN_CNT_RESET, 0);
            if (rv < 0 && rv != BCM_E_UNAVAIL) {
                return rv;
            }
        }
    }

    /* enable default ROAMING option for proper HW behavior on SA 
     *  learn count maintenance when port moving occurred. 
     */
    if (SOC_IS_HARRIER(unit) || SOC_IS_TBX(unit) ){
        pbmp_t  pbmp;
        uint32  temp = BCM_PORT_LEARN_ARL;

        if (SOC_IS_TBX(unit)){
            temp |= BCM_PORT_LEARN_FWD;
        }

        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        BCM_PBMP_ITER(pbmp, port){
            BCM_IF_ERROR_RETURN(bcm_robo_port_control_set(unit, port, 
                    bcmPortControlL2Move, temp));
        }
    }

    /* bcm_l2_register clients */
    soc_robo_arl_register(unit, _bcm_robo_l2_register_callback, NULL);

    _l2_init[unit] = 1;
    return (BCM_E_NONE);
}

/*
 * Temporarily stop L2 table from changing (learning, aging, CPU, etc)
 */

/*
 * Function:
 *    bcm_robo_l2_addr_freeze
 * Description:
 *    Temporarily quiesce ARL from all activity (learning, aging)
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *    BCM_E_NONE        Success
 *    BCM_E_INTERNAL        Chip access failure
 */
int
bcm_robo_l2_addr_freeze(int unit)
{
    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_freeze().\n")));
    BCM_IF_ERROR_RETURN(soc_robo_arl_freeze(unit));
    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_robo_l2_addr_thaw
 * Description:
 *    Restore normal ARL activity.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *    BCM_E_NONE        Success
 *    BCM_E_INTERNAL        Chip access failure
 */
int
bcm_robo_l2_addr_thaw(int unit)
{
    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_thaw().\n")));
    BCM_IF_ERROR_RETURN( soc_robo_arl_thaw(unit));
    return BCM_E_NONE;
}

/*
 * Initialize a bcm_l2_addr_t to a specified MAC address and VLAN,
 * zeroing all other fields.
 */

/*
 * Function:
 *    bcm_robo_l2_addr_add
 * Description:
 *    Add a MAC address to the Switch Address Resolution Logic (ARL)
 *    port with the given VLAN ID and parameters.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *    BCM_E_NONE        Success
 *    BCM_E_INTERNAL        Chip access failure
 * Notes:
 *    Use CMIC_PORT(unit) to associate the entry with the CPU.
 *    Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *      Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *      passed as well with non-zero trunk group ID)
 */

int 
bcm_robo_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    l2_arl_sw_entry_t   arl_entry;
    uint32              flags = 0;
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;

    L2_INIT(unit);
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_add()..\n")));

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_entry, l2addr));
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "in bcm_robo_l2_addr_add , arl_entry is 0x%x,0x%x,0x%x\n"),
              arl_entry.entry_data[2], 
              arl_entry.entry_data[1], 
              arl_entry.entry_data[0]));

    if (l2addr->flags & BCM_L2_REPLACE_DYNAMIC) {
        LOG_INFO(BSL_LS_BCM_ARL,
                 (BSL_META_U(unit,
                             "BCM API : bcm_robo_l2_addr_add()..BCM_L2_REPLACE_DYNAMIC\n")));
        
        flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID | 
                DRV_MEM_OP_REPLACE;
    } else {
        flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID;    
    }
    
    if (l2addr->flags & BCM_L2_PENDING){
        if (!soc_feature(unit, soc_feature_l2_pending)) {
            return BCM_E_UNAVAIL;
        }
        flags |=  DRV_MEM_OP_PENDING;
    }
    
    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_INSERT(unit, DRV_MEM_ARL, (uint32 *)&arl_entry, flags);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }
    
    return rv;

}

/*
 * Function:
 *    bcm_robo_l2_addr_delete
 * Description:
 *    Remove an L2 address from the device's ARL
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    mac - MAC address to remove
 *    vid - associated VLAN ID
 * Returns:
 *    BCM_E_NONE        Success
 *    BCM_E_INTERNAL        Chip access failure
 */
int 
bcm_robo_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    bcm_l2_addr_t       l2addr;
    l2_arl_sw_entry_t   arl_delete;
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32              flags = 0;

    L2_INIT(unit);
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_delete()..\n")));
            
    bcm_l2_addr_init(&l2addr, mac, vid);

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_delete, &l2addr));

    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        /* SDK-34306 : Can't remove Mcast entry.
         *  
         *  Fixing Note : 
         *      1. Root cause is l2 delete with pending flag is now redesigned 
         *          per SDK-33848 to remove (pending entry only) instead of 
         *          (pending || non-pending entry)
         *      2. The fixing process for such designing change on SDK-33848 
         *          in this API will be :
         *          a. normal remove without pending flag.
         *          b. if return BCM_E_NOT_FOUND, use pending flag to remove 
         *              again.
         */
        flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID |
                    DRV_MEM_OP_DELETE_BY_STATIC;
        rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_delete, flags);
        if (rv == BCM_E_NOT_FOUND) {
            if (soc_feature(unit, soc_feature_l2_pending)) {
                flags |= DRV_MEM_OP_PENDING;
                rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_delete, 
                            flags);
            }
        }

        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }

    return rv;
}


/*
 * Function:
 *    bcm_robo_l2_addr_delete_by_port
 * Description:
 *    Remove all L2 (MAC) addresses associated with the port.
 * Parameters:
 *    unit  - RoboSwitch PCI device unit number (driver internal).
 *    mod   - module id (or -1 for local unit)
 *    pbmp  - bitmap of ports to affect
 *    flags - BCM_L2_REMOVE_XXX
 * Returns:
 *    BCM_E_NONE        Success.
 *    BCM_E_INTERNAL        Chip access failure.
 * Notes:
 *    Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *
 *    ARL aging and learning on all ports is disabled during this
 *    operation.   If these weren't disabled, the hardware could
 *    shift around the contents of the ARL table during the remove
 *    operation, causing some entries that should be removed to remain
 *    in the table.
 */
int
bcm_robo_l2_addr_delete_by_port(int unit, bcm_module_t mod, bcm_port_t port,
                uint32 flags)
{
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    l2_arl_sw_entry_t   arl_entry;
    uint32              fld_value = 0;
    uint32              action_flag = 0;

    L2_INIT(unit);
    sal_memset(&arl_entry, 0, sizeof (arl_entry));

    /* valid module/port check */
    if (BCM_GPORT_IS_SET(port)) {
        bcm_port_t  tmp_port;
        bcm_trunk_t tmp_trunk;
        bcm_module_t tmp_mod;
        int         tmp_id;
    
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &tmp_mod, &tmp_port,
                                   &tmp_trunk, &tmp_id));
        port = tmp_port;
    } else {
        if (port < 0) {
            return BCM_E_PORT;
        }
    }
    
    /* module == -1 is used for indicating to local modid */
    if (mod < 0) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    } else { /* map module/port */
        if (!SOC_MODID_ADDRESSABLE(unit, mod)) {
            return BCM_E_BADID;
        }
    }
    
    
    fld_value = port;    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_delete_by_port()..\n")));
    /* set port field */
    BCM_IF_ERROR_RETURN(
            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)&arl_entry, &fld_value));
    action_flag = DRV_MEM_OP_DELETE_BY_PORT ;
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
    } else {
        action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                        DRV_MEM_OP_DELETE_BY_STATIC : 0;
    }
#else   /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
#endif  /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_PENDING) ? 
                    DRV_MEM_OP_PENDING : 0;

    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_DELETE(
                unit, DRV_MEM_ARL,(uint32 *)&arl_entry, action_flag);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }
    return rv;
}


/*
 * Function:
 *    bcm_l2_addr_delete_by_mac
 * Description:
 *    Delete L2 entries associated with a MAC address.
 * Parameters:
 *    unit  - device unit
 *    mac   - MAC address
 *    flags - BCM_L2_DELETE_XXX
 * Returns:
 *    BCM_E_XXX
 * Notes:
 *    Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *    L2 aging and learning are disabled during this operation.
 */
int
bcm_robo_l2_addr_delete_by_mac(
    int    unit,
    bcm_mac_t    mac,
    uint32    flags)
{
    int             rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32          match_flags = 0;
    bcm_l2_addr_t   match_addr;

    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API:%s,flags=0x%x\n"), FUNCTION_NAME(), flags));
    assert(mac);
    if (!BCM_MAC_IS_ZERO(mac)){
        sal_memset(&match_addr, 0, sizeof(bcm_l2_addr_t));
        sal_memcpy(match_addr.mac, mac, sizeof(bcm_mac_t));
        
        match_flags = BCM_L2_TRAVERSE_MATCH_MAC;
        if (flags & BCM_L2_DELETE_STATIC) {
            match_flags |= BCM_L2_TRAVERSE_MATCH_STATIC;
        }
        
        if (flags & BCM_L2_DELETE_PENDING) {
            match_flags |= _BCM_L2_TRAVERSE_MATCH_PENDING;
            match_addr.flags |= BCM_L2_PENDING;
        }
        
        /* normal match traverse need to add the IGNORE related flags */
        match_flags |= BCM_L2_TRAVERSE_IGNORE_DISCARD_SRC |
                BCM_L2_TRAVERSE_IGNORE_DES_HIT;
        
        freezing = bcm_robo_l2_addr_freeze(unit);
        if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
            rv = bcm_robo_l2_matched_traverse(unit, 
                    match_flags, &match_addr, _bcm_robo_l2_remove_cb, NULL);
            if (freezing == BCM_E_NONE){
                BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
            }
        } else {
            rv = freezing;
        }
        
        if (BCM_SUCCESS(rv)){
            LOG_INFO(BSL_LS_BCM_L2TABLE,
                     (BSL_META_U(unit,
                                 "%s, %d L2 entries were removed!\n"),
                      FUNCTION_NAME(), rv));
            rv = BCM_E_NONE;
        }
    }
    return rv;
}


/*
 * Function:
 *    bcm_robo_l2_addr_delete_by_vlan
 * Description:
 *    Remove all L2 (MAC) addresses associated with vid.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    vlan - vid to check
 *    flags - BCM_L2_REMOVE_XXX
 * Returns:
 *    BCM_E_NONE        Success.
 *    BCM_E_INTERNAL        Chip access failure.
 * Notes:
 *    Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *
 *    ARL aging and learning on all ports is disabled during this
 *    operation.   If these weren't disabled, the hardware could
 *    shift around the contents of the ARL table during the remove
 *    operation, causing some entries that should be removed to remain
 *    in the table.
 */
int
bcm_robo_l2_addr_delete_by_vlan(int unit, bcm_vlan_t vid, uint32 flags)
{
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32              fld_value = 0;
    uint32              action_flag = 0;
    l2_arl_sw_entry_t   arl_entry;
    
    L2_INIT(unit);

    /* valid check */
    if(vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    sal_memset(&arl_entry, 0, sizeof (arl_entry));
    fld_value = vid; /* vid value */
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_delete_by_vlan()..\n")));
    /* set VID field */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_VLANID, (uint32 *)&arl_entry, &fld_value));
    action_flag = DRV_MEM_OP_DELETE_BY_VLANID ;
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
    } else {
        action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                        DRV_MEM_OP_DELETE_BY_STATIC : 0;
    }
#else   /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
#endif  /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_PENDING) ? 
                    DRV_MEM_OP_PENDING : 0;
    /* freeze/thaw the HW ARL to prevent the confilict action on  
     * SA_LRN_CNT handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_entry, 
                action_flag);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }
        
    return rv;
}


/*
 * Function:
 *    bcm_robo_l2_addr_delete_by_trunk
 * Description:
 *    Remove all L2 (MAC) addresses associated with tid.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    tid - tid to check
 *    flags - BCM_L2_REMOVE_XXX
 * Returns:
 *    BCM_E_NONE        Success.
 *    BCM_E_INTERNAL        Chip access failure.
 * Notes:
 *    Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *
 *    ARL aging and learning on all ports is disabled during this
 *    operation.   If these weren't disabled, the hardware could
 *    shift around the contents of the ARL table during the remove
 *    operation, causing some entries that should be removed to remain
 *    in the table.
 *  (P.S. TB device start to provide the fast aging feature on trunk.)
 */
int
bcm_robo_l2_addr_delete_by_trunk(int unit, bcm_trunk_t tid, uint32 flags)
{
    bcm_trunk_info_t  trunk_info;
    int  i, rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32  key_value = 0;
    uint32  action_flag = 0;
    bcm_trunk_member_t  member_array[BCM_TRUNK_MAX_PORTCNT];
    int  member_count;

    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_delete_by_trunk()..\n")));

    if (tid == BCM_TRUNK_INVALID) {
        return BCM_E_PARAM;
    }
    
    rv = bcm_trunk_get(unit, tid, &trunk_info, 
             BCM_TRUNK_MAX_PORTCNT, &member_array[0], &member_count);
    
    if (rv == BCM_E_NOT_FOUND) {
        return BCM_E_NONE;
    } else if (rv < 0) {
        return BCM_E_INTERNAL;
    }
    
    if (SOC_IS_TBX(unit)) {
        action_flag = DRV_MEM_OP_DELETE_BY_TRUNK;
        key_value = tid;

#ifdef  BCM_TB_SUPPORT
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
#endif  /* BCM_TB_SUPPORT */

        if (flags & BCM_L2_DELETE_PENDING) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Delete with PENDING flag is unavailable in TB.\n")));
        }
        
        /* freeze/thaw the HW ARL to prevent the confilict action on  
         * SA_LRN_CNT handling process between SW and HW.
         */
        freezing = bcm_robo_l2_addr_freeze(unit);
        if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE) {
            rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, &key_value, action_flag);
            if (freezing == BCM_E_NONE) {
                BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
            }
        } else {
            rv = freezing;
        }
    } else {
        /* 
         * If no any port assigned to the current trunk,
         * there will be no further processes.
         * Since no tgid information recorded in the arl entry of robo chip.
         */
        if (!member_count) {
            return BCM_E_UNAVAIL;
        }
            
        for (i = 0; i < member_count; i++) {
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_delete_by_port
                (unit, -1, member_array[i].gport, flags));
        }
    }

    return rv;
}


/*
 * Function:
 *    bcm_l2_addr_delete_by_mac_port
 * Description:
 *    Delete L2 entries associated with a MAC address and
 *    a destination module/port
 * Parameters:
 *    unit  - device unit
 *    mac   - MAC address
 *    mod   - module id
 *    port  - port
 *    flags - BCM_L2_DELETE_XXX
 * Returns:
 *    BCM_E_XXX
 * Notes:
 *    1. Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *    2. Robo L2 learning will prevent that the same MAC+VID learned at 
 *      different ports.
 */
int
bcm_robo_l2_addr_delete_by_mac_port(
    int    unit,
    bcm_mac_t    mac,
    bcm_module_t    mod,
    bcm_port_t    port,
    uint32    flags)
{
    int             rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32          match_flags = 0;
    bcm_l2_addr_t   match_addr;

    L2_INIT(unit);        

    /* valid module/port check */
    if (BCM_GPORT_IS_SET(port)) {
        bcm_port_t  tmp_port;
        bcm_trunk_t tmp_trunk;
        bcm_module_t tmp_mod;
        int         tmp_id;
    
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &tmp_mod, &tmp_port,
                                   &tmp_trunk, &tmp_id));
        port = tmp_port;
    }
    
    /* module == -1 is used for indicating to local modid */
    if (mod < 0) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    } else { /* map module/port */
        if (!SOC_MODID_ADDRESSABLE(unit, mod)) {
            return BCM_E_BADID;
        }
    }

    assert(mac);
    if (!BCM_MAC_IS_ZERO(mac)){
        LOG_INFO(BSL_LS_BCM_L2TABLE,
                 (BSL_META_U(unit,
                             "%s,mac=%08x-%08x,port=0x%x\n"), 
                  FUNCTION_NAME(), *(uint32 *)mac, *((uint32 *)mac+1), port));
        sal_memset(&match_addr, 0, sizeof(bcm_l2_addr_t));
        sal_memcpy(match_addr.mac, mac, sizeof(bcm_mac_t));
        match_addr.port = port;
        
        match_flags = BCM_L2_TRAVERSE_MATCH_MAC | BCM_L2_TRAVERSE_MATCH_DEST;
        if (flags & BCM_L2_DELETE_STATIC) {
            match_flags |= BCM_L2_TRAVERSE_MATCH_STATIC;
        }
        
        if (flags & BCM_L2_DELETE_PENDING) {
            match_flags |= _BCM_L2_TRAVERSE_MATCH_PENDING;
            match_addr.flags |= BCM_L2_PENDING;
        }
        
        /* normal match traverse need to add the IGNORE related flags */
        match_flags |= BCM_L2_TRAVERSE_IGNORE_DISCARD_SRC |
                BCM_L2_TRAVERSE_IGNORE_DES_HIT;
        
        freezing = bcm_robo_l2_addr_freeze(unit);
        if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
            rv = bcm_robo_l2_matched_traverse(unit, 
                    match_flags, &match_addr, _bcm_robo_l2_remove_cb, NULL);
            if (freezing == BCM_E_NONE){
                BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
            }
        } else {
            rv = freezing;
        }
        
        if (BCM_SUCCESS(rv)){
            LOG_INFO(BSL_LS_BCM_L2TABLE,
                     (BSL_META_U(unit,
                                 "%s, %d L2 entries were removed!\n"),
                      FUNCTION_NAME(), rv));
            rv = BCM_E_NONE;
        }
    }

    return rv;
}

/*
 * Function:
 *    bcm_l2_addr_delete_by_vlan_port
 * Description:
 *    Delete L2 entries associated with a VLAN and
 *    a destination module/port
 * Parameters:
 *    unit  - device unit
 *    vid   - VLAN id
 *    mod   - module id
 *    port  - port
 *    flags - BCM_L2_DELETE_XXX
 * Returns:
 *    BCM_E_XXX
 * Notes:
 *    Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *    L2 aging and learning are disabled during this operation.
 */
int 
bcm_robo_l2_addr_delete_by_vlan_port(int unit, bcm_vlan_t vid,
                       bcm_module_t mod, bcm_port_t port,
                       uint32 flags)
{
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    l2_arl_sw_entry_t   arl_entry;
    uint32              fld_value = 0;
    uint32              action_flag = 0;
    
    L2_INIT(unit);
    
    sal_memset(&arl_entry, 0, sizeof (arl_entry));

    /* valid VLAN check */
    if(vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_delete_by_vlan_port()..\n")));
    /* set VID field */
    fld_value = vid; /* vid value */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_VLANID, (uint32 *)&arl_entry, &fld_value));
            
    /* valid module/port check */
    if (BCM_GPORT_IS_SET(port)) {
        bcm_port_t  tmp_port;
        bcm_trunk_t tmp_trunk;
        bcm_module_t tmp_mod;
        int         tmp_id;
    
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &tmp_mod, &tmp_port,
                                   &tmp_trunk, &tmp_id));
        port = tmp_port;
    } else {
        if (port < 0) {
            return BCM_E_PORT;
        }
    }
    
    /* module == -1 is used for indicating to local modid */
    if (mod < 0) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    } else { /* map module/port */
        if (!SOC_MODID_ADDRESSABLE(unit, mod)) {
            return BCM_E_BADID;
        }
    }

    fld_value = port; /* port value */
    /* set Port field */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_SRC_PORT, (uint32 *)&arl_entry, &fld_value));
    action_flag = DRV_MEM_OP_DELETE_BY_VLANID | 
                    DRV_MEM_OP_DELETE_BY_PORT ;
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
    } else {
        action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                        DRV_MEM_OP_DELETE_BY_STATIC : 0;
    }
#else   /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
#endif  /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_PENDING) ? 
                    DRV_MEM_OP_PENDING : 0;
    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_DELETE(
                unit, DRV_MEM_ARL,(uint32 *)&arl_entry, action_flag);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }

    return rv;
}                       

/*
 * Function:
 *    bcm_l2_addr_delete_by_vlan_trunk
 * Description:
 *    Delete L2 entries associated with a VLAN and a
 *      destination trunk.
 * Parameters:
 *    unit  - device unit
 *    vid   - VLAN id
 *    tid   - trunk group id
 *    flags - BCM_L2_DELETE_XXX
 * Returns:
 *    BCM_E_XXX
 * Notes:
 *    Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *    L2 aging and learning are disabled during this operation.
 */
int
bcm_robo_l2_addr_delete_by_vlan_trunk(int unit, bcm_vlan_t vid,
                                 bcm_trunk_t tid, uint32 flags)
{
    bcm_trunk_info_t  trunk_info;
    int  i, rv = BCM_E_NONE;
    bcm_trunk_member_t  member_array[BCM_TRUNK_MAX_PORTCNT];
    int  member_count;
    
    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_delete_by_vlan_trunk()..\n")));

    if (tid == BCM_TRUNK_INVALID) {
        return BCM_E_PARAM;
    }

    rv = bcm_trunk_get(unit, tid, &trunk_info, 
             BCM_TRUNK_MAX_PORTCNT, &member_array[0], &member_count);
    
    if (rv == BCM_E_NOT_FOUND) {
        return BCM_E_NONE;
    } else if (rv < 0) {
        return BCM_E_INTERNAL;
    }
    
    /* 
     * If no any port assigned to the current trunk,
     * there will be no further processes.
     * Since no tgid information recorded in the arl entry of robo chip.
     */
    if (!member_count) {
        return BCM_E_UNAVAIL;
    }
        
    for (i = 0; i < member_count; i++) {
        BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_delete_by_vlan_port
            (unit, vid, -1, member_array[i].gport, flags));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_robo_l2_addr_get
 * Description:
 *    Given a MAC address and VLAN ID, check if the entry is present
 *    in the L2 table, and if so, return all associated information.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    mac - input MAC address to search
 *    vid - input VLAN ID to search
 *    l2addr - Pointer to bcm_l2_addr_t structure to receive results
 * Returns:
 *    BCM_E_NONE        Success (l2addr filled in)
 *    BCM_E_PARAM        Illegal parameter (NULL pointer)
 *    BCM_E_INTERNAL        Chip access failure
 *    BCM_E_NOT_FOUND    Address not found (l2addr not filled in)
 */
int 
bcm_robo_l2_addr_get(int unit, bcm_mac_t mac, bcm_vlan_t vid, 
                    bcm_l2_addr_t *l2addr)
{
    bcm_l2_addr_t    l2_search;
    l2_arl_sw_entry_t        arl_entry, arl_result;
    int            rv = BCM_E_NONE;

    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_get()..\n")));
    bcm_l2_addr_init(&l2_search, mac, vid);

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_entry, &l2_search));

    rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
            (uint32 *)&arl_result, NULL,
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID));

    if (rv == BCM_E_EXISTS) {
        _bcm_robo_l2_from_arl(unit, l2addr, &arl_result);
    } else {
        if (rv == BCM_E_FULL){
            rv = BCM_E_NOT_FOUND;
        }
        return rv;
    }

    return BCM_E_NONE;

}



/*
 * Function:
 *    bcm_l2_key_dump
 * Purpose:
 *    Dump the key (VLAN+MAC) portion of a hardware-independent
 *    L2 address for debugging
 * Parameters:
 *    unit - Unit number
 *    pfx - String to print before output
 *    entry - Hardware-independent L2 entry to dump
 *    sfx - String to print after output
 */

int
bcm_robo_l2_key_dump(int unit, char *pfx, bcm_l2_addr_t *entry, char *sfx)
{
    /*
     * In VxWorks, in interrupt context, sal_vprintf uses logMsg, which only
     * allows up to 6 args.  That's why the MAC address is formatted as
     * two hex numbers here.
     */
    LOG_CLI((BSL_META_U(unit,
                        "l2: %sVLAN=0x%03x MAC=0x%02x%02x%02x"
                        "%02x%02x%02x%s"), pfx, entry->vid,
             entry->mac[0], entry->mac[1], entry->mac[2],
             entry->mac[3], entry->mac[4], entry->mac[5], sfx));
    return BCM_E_NONE;
}

/****************************************************************************
 *
 * ARL Message Registration
 *
 ****************************************************************************/

#define L2_CB_MAX        3

typedef struct arl_cb_entry_s {
    bcm_l2_addr_callback_t    fn;
    void            *fn_data;
} l2_cb_entry_t;

typedef struct l2_data_s {
    l2_cb_entry_t        cb[L2_CB_MAX];
    int                cb_count;
} l2_data_t;

static l2_data_t l2_data[SOC_MAX_NUM_SWITCH_DEVICES];

/*
 * Function:
 *     _bcm_l2_addr_callback
 * Description:
 *    Callback used with chip addr registration functions.
 *    This callback calls all the top level client callbacks.
 * Parameters:
 *    unit - RoboSwitch unit number (driver internal).
 *    l2addr
 *    insert
 *    userdata
 * Returns:
 *
 */
static void
_bcm_l2_addr_callback(int unit,
              bcm_l2_addr_t *l2addr,
              int insert,
              void *userdata)
{
    l2_data_t        *ad = &l2_data[unit];
    int i;

    for(i = 0; i < L2_CB_MAX; i++) {
        if(ad->cb[i].fn) {
            ad->cb[i].fn(unit, l2addr, insert, ad->cb[i].fn_data);
        }
    }
}

/*
 * Function:
 *    bcm_robo_l2_addr_register
 * Description:
 *    Register a callback routine that will be called whenever
 *    an entry is inserted into or deleted from the L2 address table.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    fn - Callback function of type bcm_l2_addr_callback_t.
 *    fn_data - Arbitrary value passed to callback along with messages
 * Returns:
 *    BCM_E_NONE        Success, handle valid
 *    BCM_E_MEMORY        Out of memory
 *    BCM_E_INTERNAL        Chip access failure
 */
int
bcm_robo_l2_addr_register(int unit,
             bcm_l2_addr_callback_t fn, 
             void *fn_data)
{
    l2_data_t   *ad = &l2_data[unit];
    int         i;

    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_register()..\n")));
    
    BCM_IF_ERROR_RETURN
        (bcm_53xx_l2_addr_register(unit, _bcm_l2_addr_callback, NULL));

    if (ad->cb_count > L2_CB_MAX) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s,!! cb_count=%d over valid range!!\n"), 
                  FUNCTION_NAME(),ad->cb_count));
        return BCM_E_MEMORY;
    }

    for (i = 0; i < L2_CB_MAX; i++) {
        if (ad->cb[i].fn == NULL) {
            ad->cb[i].fn = fn;
            ad->cb[i].fn_data = fn_data;
            ad->cb_count++;
            break;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_robo_l2_addr_unregister
 * Description:
 *    Unregister a previously registered callback routine.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    fn - Same callback function used to register callback
 *    fn_data - Same arbitrary value used to register callback
 * Returns:
 *    BCM_E_NONE        Success, handle valid
 *    BCM_E_MEMORY        Out of memory
 *    BCM_E_INTERNAL        Chip access failure
 * Notes:
 *    Both callback and userdata must match from original register call.
 */
int
bcm_robo_l2_addr_unregister(int unit,
               bcm_l2_addr_callback_t fn, 
               void *fn_data)
{
    l2_data_t   *ad = &l2_data[unit];
    int         i, rv = BCM_E_NOT_FOUND;

    L2_INIT(unit);
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_addr_unregister()..\n")));
    for (i = 0; i < L2_CB_MAX; i++) {
        if((ad->cb[i].fn == fn) && (ad->cb[i].fn_data == fn_data)) {
            ad->cb[i].fn = NULL;
            ad->cb[i].fn_data = NULL;
            ad->cb_count--;
            if (ad->cb_count == 0) {
                rv = bcm_53xx_l2_addr_unregister(unit,
                            _bcm_l2_addr_callback,
                            NULL);
            }
        }
    }
    return rv;
}
                  

/*
 * Set L2 table aging time
 */

/*
 * Function:
 *    bcm_robo_l2_age_timer_set
 * Description:
 *    Set the age timer for all blocks.
 *    Setting the value to 0 disables the age timer.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    age_seconds - Age timer value in seconds
 * Returns:
 *    BCM_E_NONE        Success
 *    BCM_E_INTERNAL        Chip access failure
 */
int
bcm_robo_l2_age_timer_set(int unit, int age_seconds)
{
    uint32  max_value = 0;
    uint32  enabled = 0;

    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_age_timer_set()..\n")));
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_AGE_TIMER_MAX_S, &max_value));

    if (age_seconds > max_value) {
        return BCM_E_PARAM;
    }

    enabled = age_seconds ? TRUE : FALSE;

    BCM_IF_ERROR_RETURN(DRV_AGE_TIMER_SET(unit, 
            enabled, (uint32)age_seconds));

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_robo_l2_age_timer_get
 * Description:
 *    Returns the current age timer value.
 *    The value is 0 if aging is not enabled.
 * Parameters:
 *    unit - RoboSwitch PCI device unit number (driver internal).
 *    age_seconds - Place to store returned age timer value in seconds
 * Returns:
 *    BCM_E_NONE        Success
 *    BCM_E_INTERNAL        Chip access failure
 */
int
bcm_robo_l2_age_timer_get(int unit, int *age_seconds)
{
    uint32 enabled;

    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_age_timer_get()..\n")));
    if (age_seconds == NULL){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(DRV_AGE_TIMER_GET(unit, &enabled, 
            (uint32 *)age_seconds));

    if (!enabled) {
        *age_seconds = 0;
    }

    return BCM_E_NONE;
}
    
    
/*
 * Function:
 *      bcm_l2_clear
 * Purpose:
 *      Clear the L2 layer
 * Parameters:
 *      unit  - BCM unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l2_clear(int unit)
{
    uint32  no_sw_arl = 0;
    soc_control_t   *soc = SOC_CONTROL(unit);
    
    L2_INIT(unit);

    BCM_IF_ERROR_RETURN(bcm_l2_detach(unit));

    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "BCM API : bcm_robo_l2_clear()..\n")));
    /*
     * Call chip-dependent initialization
     */
    soc_robo_arl_unregister(unit, _bcm_robo_l2_register_callback, NULL);
    BCM_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_ARL));
    
    /* ROBO's SW ARL reset due to HW L2 table cleared */
    no_sw_arl = (soc->arl_table == NULL) ? TRUE : FALSE;
    if (!no_sw_arl) {
        /* force ARL thread restart to reset ROBO's SW ARL */
        BCM_IF_ERROR_RETURN(soc_robo_arl_mode_set(unit, ARL_MODE_ROBO_POLL));
    }
    
    /* bcm_l2_register clients */
    soc_robo_arl_register(unit, _bcm_robo_l2_register_callback, NULL);


    /* Clear l2_data structure */
    l2_data[unit].cb_count = 0;
    sal_memset(&l2_data[unit].cb, 0, sizeof(l2_data[unit].cb));

    /* BCM shadow table will go away soon */

    _l2_init[unit] = 1;        /* some positive value */
    
    return BCM_E_NONE;
}


/*
 * Given an L2 or L2 multicast address, return any existing L2 or L2
 * multicast addresses which might prevent it from being inserted
 * because a chip resource (like a hash bucket) is full.
 */

/*
 * Function:
 *    bcm_robo_l2_conflict_get
 * Purpose:
 *    Given an L2 address, return existing addresses which could conflict.
 * Parameters:
 *    unit        - switch device
 *    addr        - l2 address to search for conflicts
 *    cf_array    - (OUT) list of l2 addresses conflicting with addr
 *    cf_max        - number of entries allocated to cf_array
 *    cf_count    - (OUT) actual number of cf_array entries filled
 * Returns:
 *      BCM_E_XXX
 * Note :
 * 1. Different ROBO chips has different l2 hash bucket size.
 *    - l2x.h defined every ROBO chip bucket size annd the max bucket size 
 *      in ROBO chips.
 * 2. all valid entries in the hashed bucket size must be reported through 
 *      this API.
 */

int
bcm_robo_l2_conflict_get(int unit, bcm_l2_addr_t *addr,
            bcm_l2_addr_t *cf_array, int cf_max,
            int *cf_count)
{
    int     i, search_result = SOC_E_NONE, rv = BCM_E_NONE;
    uint64  mac_field;
    l2_arl_sw_entry_t   arl_entry, output[ROBO_MAX_L2_BUCKET_SIZE];
    l2_arl_sw_entry_t   *cf_entry;

    L2_INIT(unit);

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_entry, addr));

    *cf_count = 0;
    sal_memset(output, 0, sizeof(output));
    search_result = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, 
            (uint32 *)&arl_entry, (uint32 *)output, NULL, 
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID | 
            DRV_MEM_OP_SEARCH_CONFLICT));

    if ((search_result == SOC_E_EXISTS)) {
        for (i = 0; (i < ROBO_MAX_L2_BUCKET_SIZE) && (*cf_count < cf_max); 
                i++){

            cf_entry = &output[i];
            COMPILER_64_ZERO(mac_field);
            rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
                    (void *)cf_entry, (void *)&mac_field);

            if (!COMPILER_64_IS_ZERO(mac_field)){
               _bcm_robo_l2_from_arl(unit, &cf_array[*cf_count], cf_entry);
                *cf_count += 1;
                LOG_INFO(BSL_LS_BCM_ARL,
                         (BSL_META_U(unit,
                                     "%s ,conflict entry on bin[%d] is 0x%x,0x%x,0x%x\n"),
                          FUNCTION_NAME(), i, 
                          cf_entry->entry_data[2], 
                          cf_entry->entry_data[1], 
                          cf_entry->entry_data[0]));
            }
        }
    }

    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "%s, cf_status=%d, get %d conflict entry.\n"), 
              FUNCTION_NAME(), search_result, *cf_count));
    return (rv);
}

int
bcm_robo_l2_tunnel_add(int unit, bcm_mac_t mac, bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_l2_tunnel_delete(int unit, bcm_mac_t mac, bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_l2_tunnel_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_l2_learn_limit_set
 * Description:
 *     Set the L2 MAC learning limit
 * Parameters:
 *     unit        device number
 *     limit       learn limit control info
 *                 limit->flags - qualifiers bits and action bits
 *                 limit->vlan - vlan identifier
 *                 limit->port - port number
 *                 limit->trunk - trunk identifier
 *                 limit->limit - max number of learned entry
 *                                  nagtive limit for disable.
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_l2_learn_limit_set(int unit, bcm_l2_learn_limit_t *limit)
{
    bcm_pbmp_t t_pbm;
    uint32 type = 0, action = 0, temp = 0, drv_action = 0;
    bcm_port_t	dev_port = 0;

    if (!soc_feature(unit, soc_feature_mac_learn_limit)) {
        return BCM_E_UNAVAIL;
    }

    BCM_PBMP_CLEAR(t_pbm); 

    if (!limit) {
        return BCM_E_PARAM;
    }

    /* check valid limit value */
    if (SOC_IS_HARRIER(unit)) {
        temp = soc_robo_mem_index_max(unit, INDEX(L2_ARLm));
    } else {
        temp = soc_robo_mem_index_max(unit, INDEX(L2_ARLm)) + 1;
    }
    if (limit->limit > ((int)temp)){
        return BCM_E_PARAM;
    }

    type = limit->flags &
           (BCM_L2_LEARN_LIMIT_SYSTEM | BCM_L2_LEARN_LIMIT_VLAN | 
            BCM_L2_LEARN_LIMIT_PORT | BCM_L2_LEARN_LIMIT_TRUNK);

    action = limit->flags & 
             (BCM_L2_LEARN_LIMIT_ACTION_DROP | BCM_L2_LEARN_LIMIT_ACTION_CPU |
              BCM_L2_LEARN_LIMIT_ACTION_PREFER);

    if (!type) {
        return BCM_E_PARAM;
    }

    if (type != BCM_L2_LEARN_LIMIT_SYSTEM &&
        (action & BCM_L2_LEARN_LIMIT_ACTION_PREFER)) {
        return BCM_E_PARAM;
    }

    if ((action & BCM_L2_LEARN_LIMIT_ACTION_DROP) && 
        (action & BCM_L2_LEARN_LIMIT_ACTION_CPU)) {
        if (SOC_IS_HARRIER(unit)){
            /* support both acitons but can't both worked at the same time */
            return BCM_E_PARAM;
        }
    }

    if (action & BCM_L2_LEARN_LIMIT_ACTION_PREFER){
        /* currently there is no ROBO deivce support this feature */
        return BCM_E_UNAVAIL;
    }

    if (type & BCM_L2_LEARN_LIMIT_SYSTEM) {
        if (!soc_feature(unit, soc_feature_system_mac_learn_limit)) {
            return BCM_E_UNAVAIL;
        }
        if (SOC_IS_STARFIGHTER3(unit) && action) {
            return BCM_E_UNAVAIL;
        }
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
            /* NSP and SF3 HW Spec. for systme based learn limit, the action is still
             *   control by ports basis.
             *
             * Note : Check device here due to such design is a special case.
             */

            t_pbm = PBMP_E_ALL(unit);
            
            dev_port = (uint32)(-1);
            /* set the Limit :
             *
             *  Note : 
             *  1. the special case on limit at -1 is handled in NSP's
             *      DRV_ARL_LEARN_COUNT_SET() already.
             */
            temp = DRV_PORT_SA_LRN_CNT_LIMIT;
            SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                    (unit, dev_port, temp, limit->limit));
        } else {
            return BCM_E_UNAVAIL;
        }
    }

    if (type & BCM_L2_LEARN_LIMIT_PORT) {

        if (BCM_GPORT_IS_SET(limit->port)) {
            BCM_IF_ERROR_RETURN(bcm_robo_port_local_get(
                    unit, limit->port, (bcm_port_t *)&dev_port));
        } else {
            dev_port = (uint32)limit->port;
        }

        if (!SOC_PORT_VALID(unit, dev_port)) {
            return BCM_E_PORT;
        }

        BCM_PBMP_PORT_ADD(t_pbm, dev_port);
        /* Learn Limit action and Limit will be configurred individually */
        
        temp = DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION;
        if (action & BCM_L2_LEARN_LIMIT_ACTION_DROP){
            if (action & BCM_L2_LEARN_LIMIT_ACTION_CPU){
                drv_action = DRV_PORT_LEARN_LIMIT_ACTION_REDIRECT2CPU;
            } else {
                drv_action = DRV_PORT_LEARN_LIMIT_ACTION_DROP;
            }
        } else {
            if (action & BCM_L2_LEARN_LIMIT_ACTION_CPU){
                drv_action = DRV_PORT_LEARN_LIMIT_ACTION_COPY2CPU;
            } else {
                drv_action = DRV_PORT_LEARN_LIMIT_ACTION_NONE;
            }
        }
        
        /* special case handling for limit at -1 for unlimited */
        if (limit->limit == -1){
            drv_action = DRV_PORT_LEARN_LIMIT_ACTION_NONE;
        }
                    
        BCM_IF_ERROR_RETURN((DRV_PORT_SET
                (unit, t_pbm, temp, drv_action)));

        /* set the Limit */
        temp = DRV_PORT_SA_LRN_CNT_LIMIT;
        SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                (unit, dev_port, temp, limit->limit));
            
    }

    if (type & BCM_L2_LEARN_LIMIT_TRUNK) {
        return BCM_E_UNAVAIL;
    }

    if (type & BCM_L2_LEARN_LIMIT_VLAN) {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_l2_learn_limit_get
 * Description:
 *     Get the L2 MAC learning limit
 * Parameters:
 *     unit        device number
 *     limit       learn limit control info
 *                 limit->flags - qualifiers bits and action bits
 *                 limit->vlan - vlan identifier
 *                 limit->port - port number
 *                 limit->trunk - trunk identifier
 *                 limit->limit - max number of learned entry, -1 for unlimit
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_l2_learn_limit_get(int unit, bcm_l2_learn_limit_t *limit)
{
    int rv;
    uint32 type = 0, action = 0, temp = 0;
    bcm_port_t	dev_port = 0;
    int max = 0;

    if (!soc_feature(unit, soc_feature_mac_learn_limit)) {
        return BCM_E_UNAVAIL;
    }

    type = limit->flags &
           (BCM_L2_LEARN_LIMIT_SYSTEM | BCM_L2_LEARN_LIMIT_VLAN | 
            BCM_L2_LEARN_LIMIT_PORT | BCM_L2_LEARN_LIMIT_TRUNK);

    rv = BCM_E_UNAVAIL;
    switch (type) {
    case BCM_L2_LEARN_LIMIT_PORT:
        if (BCM_GPORT_IS_SET(limit->port)) {
            BCM_IF_ERROR_RETURN(bcm_robo_port_local_get(
                    unit, limit->port, (bcm_port_t *)&dev_port));
        } else {
            dev_port = limit->port;
        }

        if (!SOC_PORT_VALID(unit, dev_port)) {
            return BCM_E_PORT;
        }

        rv = DRV_PORT_GET(unit, dev_port, 
            DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP, &temp);
        if (temp && (rv == SOC_E_NONE)) {
            action |= BCM_L2_LEARN_LIMIT_ACTION_DROP;
        }
        
        rv = DRV_PORT_GET(unit, limit->port, 
            DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU, &temp);
        if (temp && (rv == SOC_E_NONE)) {
            action |= BCM_L2_LEARN_LIMIT_ACTION_CPU;
        }

        BCM_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_GET(unit, 
                dev_port, DRV_PORT_SA_LRN_CNT_LIMIT, &max));
        break;

    case BCM_L2_LEARN_LIMIT_SYSTEM:
        if (soc_feature(unit, soc_feature_system_mac_learn_limit)) {
            dev_port = 0;   /* get port0 for system action report */
            rv = DRV_PORT_GET(unit, dev_port, 
                DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP, &temp);
            if (temp && (rv == SOC_E_NONE)) {
                action |= BCM_L2_LEARN_LIMIT_ACTION_DROP;
            }
            
            rv = DRV_PORT_GET(unit, limit->port, 
                DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU, &temp);
            if (temp && (rv == SOC_E_NONE)) {
                action |= BCM_L2_LEARN_LIMIT_ACTION_CPU;
            }

            dev_port = (uint32) -1;
            rv = DRV_ARL_LEARN_COUNT_GET(unit, 
                    dev_port, DRV_PORT_SA_LRN_CNT_LIMIT, &max);

        } else {
            return BCM_E_UNAVAIL;
        }
        break;
    case BCM_L2_LEARN_LIMIT_TRUNK:
    case BCM_L2_LEARN_LIMIT_VLAN:
        return BCM_E_UNAVAIL;

    default:
        return BCM_E_PARAM;
    }

    limit->flags |= action;
    limit->limit = max;

    return rv;
}

int
bcm_robo_l2_learn_class_set(int unit, int class, int class_prio, uint32 flags)
{   
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l2_learn_class_get(int unit, int class, int *class_prio,
                            uint32 *flags)
{   
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l2_learn_port_class_set(int unit,  bcm_gport_t port, int class)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l2_learn_port_class_get(int unit,  bcm_gport_t port, int *class)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_l2_addr_delete_by_mac_vpn
 * Description:
 *      Delete L2 entry with matching MAC address and VPN.
 * Parameters:
 *      unit  - device unit
 *      mac   - MAC address
 *      vpn   - L2 VPN identifier
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l2_addr_delete_by_mac_vpn(int unit, bcm_mac_t mac, bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_l2_addr_delete_by_vpn
 * Description:
 *      Delete L2 entries associated with an L2 VPN.
 * Parameters:
 *      unit  - device unit
 *      vpn   - L2 VPN identifier
 *      flags - BCM_L2_DELETE_XXX
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *      L2 aging and learning are disabled during this operation.
 */
int
bcm_robo_l2_addr_delete_by_vpn(int unit, bcm_vpn_t vpn, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/* 
 * Function:
 *     _bcm_robo_l2_match_op
 * Description:
 *      Helper function to serve the ARL related matching process.
 * Parameters:
 *      unit        device number
 *      match_addr  data structure for match relagted operation.
 *      l2_addr     Traverse structure with all the data.
 *                  
 * Return:
 *      1. BCM_E_NONE :         for full matched condition.
 *      2. BCM_E_NOT_FOUND :    for no matched condition.
 *      3. BCM_E_FAIL :         for partial matched condition.
 *      4. BCM_E_PARAM :        for improper parameter condition.
 *      4. BCM_E_XXX :          for other error condition.
 *
 *  Note :
 *  1. mac/vid/dest must be verified by API caller for performance issue since this
 *      routine will be called in traverse operation.
 *  2. Pending feature will be exectued always without feature detect first.
 *      caller must indicate if this op will be applied for matching process.
 */
int 
_bcm_robo_l2_match_op(int unit, _bcm_robo_l2_match_t *match_addr, bcm_l2_addr_t *l2_addr)
{
    int         rv = 0, diff_flags = 0;
    bcm_port_t  in_port = 0, match_port = 0;
    
    assert(l2_addr);
    if (match_addr == NULL){
        return BCM_E_NONE;
    } else {
        if (match_addr->op_flags == 0) {
           return BCM_E_NONE;
        }
    }
    
    /* parameters check :
     *  
     *  >> check op flags only. (mac/vid/dest must be verified by API caller)
     */
    if (match_addr->op_flags & ~(_BCM_ROBO_L2_MATCH_OP_ALL)) {
        return BCM_E_PARAM;
    }

    /* start the matching process*/
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "%s,%d,op_flags=0x%x!!\n"), 
              FUNCTION_NAME(), __LINE__, match_addr->op_flags));
    match_addr->result_flags = match_addr->op_flags;
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_MAC){
        if (sal_memcmp(l2_addr->mac, match_addr->mac, sizeof (bcm_mac_t))){
            match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_MAC;
        }
    }
    
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_VID){
        if (l2_addr->vid != match_addr->vid) {
            match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_VID;
        }
    }
    
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_DEST){
        if (BCM_MAC_IS_MCAST(l2_addr->mac)){
            /* _bcm_robo_l2_from_arl() for MARL will construct 
             *  l2addr->l2mc_group with two differnet format per ROBO Arch.
             *  1. GEX devices (no individual MCAST group table)
             *      - l2addr->l2mc_group is PBMP
             *  2. Harrier/TBX devices(within individual MCAST group table)
             *      - l2addr->l2mc_group is MCAST group index.
             */
            if (l2_addr->l2mc_group != match_addr->mc_group) {
                match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_DEST;
            }
        } else {
            BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, 
                    l2_addr->port, &in_port));
            BCM_IF_ERROR_RETURN(_bcm_robo_port_gport_validate(unit, 
                    match_addr->port, &match_port));
            if (in_port != match_port) {
                match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_DEST;
            }
        }
    }
    
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_HIT){
        if ((l2_addr->flags & BCM_L2_HIT) ^ 
                (match_addr->match_others & BCM_L2_HIT)) {
            match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_HIT;
        }
    }
    
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_DIS_SRC){
        if ((l2_addr->flags & BCM_L2_DISCARD_SRC) ^ 
                (match_addr->match_others & BCM_L2_DISCARD_SRC)) {
            match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_DIS_SRC;
        }
    }
    
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_DIS_DST){
        if ((l2_addr->flags & BCM_L2_DISCARD_SRC) ^ 
                (match_addr->match_others & BCM_L2_DISCARD_SRC)) {
            match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_DIS_DST;
        }
    }
    
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_STATIC){
        if ((l2_addr->flags & BCM_L2_STATIC) ^ 
                (match_addr->match_others & BCM_L2_STATIC)) {
            match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_STATIC;
        }
    }
    
    if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_PENDING){
        if ((l2_addr->flags & BCM_L2_PENDING) ^ 
                (match_addr->match_others & BCM_L2_PENDING)) {
            match_addr->result_flags &= ~_BCM_ROBO_L2_MATCH_OP_PENDING;
        }
    }
    
    /* handling retur value */
    diff_flags = _BCM_ROBO_L2_MACHED_DIFF(match_addr->op_flags, 
            match_addr->result_flags);
    if (diff_flags) {
        if (diff_flags == match_addr->op_flags) {
            /* fully no match */
            rv = BCM_E_NOT_FOUND;
        } else {
            /* partial no match */
            rv = BCM_E_FAIL;
        }
    } else {
        rv = BCM_E_NONE;
    }
    
    LOG_INFO(BSL_LS_BCM_ARL,
             (BSL_META_U(unit,
                         "%s,%d,diff_flags=0x%x, rv=%d\n"), 
              FUNCTION_NAME(), __LINE__, diff_flags, rv));
    return rv;
}

/* _bcm_robo_l2_match_key_validation :  
 *  1. match_addr==NULL : no match request condition.
 *  2. Rule validation : valid MAC/VID
 *  3. feature limitation, match_key remove once no feature supported.
 *      - ARL control
 *      - pending 
 *
 *  RETURN : 
 *  1. BCM_E_NONE
 *  2. BCM_E_PARAM : if any incorrect matching key been requested.
 */
int 
_bcm_robo_l2_match_key_validation(int unit, _bcm_robo_l2_match_t *match_addr)
{
    if (match_addr != NULL) {
        /* common RULE validation */
        if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_MAC){
            if (BCM_MAC_IS_ZERO(match_addr->mac)){
                return BCM_E_PARAM;
            }
        }
        if (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_VID){
            if (!BCM_VLAN_VALID(match_addr->vid)){
                return BCM_E_PARAM;
            }
        }
        
        /* feature validation */
        if ((match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_DIS_SRC) && 
                (match_addr->op_flags & _BCM_ROBO_L2_MATCH_OP_DIS_DST)) {
            /* ROBO ARL control Spec. on all current devices can't support 
             *  both DIS_SRC and DIS_DST. 
             */
            return BCM_E_PARAM;
        }
    }
    
    return BCM_E_NONE;
}

/*
 *  Apply ROBO's search valid operation to traverse whole L2 table.
 *
 *  Note :
 *  1. This process must not been executed while SW ARL thread is running.
 *  2. This process must be performed at L2 frozen condition.
 *      - To prevent the non-stop while loop
 *  3. 
 *
 */
int _bcm_robo_l2_hw_traverse_mem(int unit, _bcm_robo_l2_match_t *match_addr, 
        _bcm_robo_l2_traverse_t *trav_st, uint32 response_type) 
{
    int             index = 0, this_valid_index = -1;
    int             frozen = TRUE;
    int             rv = BCM_E_NONE, traverse_cnt = 0;
    int             match_rv = BCM_E_NONE;
    uint32          valid = TRUE;
    soc_control_t       *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t   sw_arl, sw_arl1;
    bcm_l2_addr_t   *this_l2addr;

    if (soc->arl_table != NULL || soc->arl_interval != 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s, can't be performed while ARL thread is running!\n"),
                  FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    BCM_IF_ERROR_RETURN(soc_robo_arl_is_frozen(unit, &frozen));
    if (!frozen){
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s, not allowed while L2 is not frozen!\n"),
                  FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    /* matching key pre-parsing */
    BCM_IF_ERROR_RETURN(_bcm_robo_l2_match_key_validation(unit, match_addr));

    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "%s, Starting L2(HW) traverse!\n"),
              FUNCTION_NAME()));
    rv =  soc_arl_search_valid(unit, 
            _ARL_SEARCH_VALID_OP_DONE, NULL, NULL, NULL);
    if (rv == SOC_E_NONE) {

        ARL_MEM_SEARCH_LOCK(soc);
        rv =  soc_arl_search_valid(unit, 
                _ARL_SEARCH_VALID_OP_START, NULL, NULL, NULL);
        if(SOC_FAILURE(rv)){
            ARL_MEM_SEARCH_UNLOCK(soc);
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, unexpected early return!\n"),
                      FUNCTION_NAME(),__LINE__));
            goto hw_traverse_exit;
        }
    } else if (rv == SOC_E_BUSY){
        ARL_MEM_SEARCH_LOCK(soc);

        rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT,
                    NULL, NULL, NULL);
        if(SOC_FAILURE(rv)){
            ARL_MEM_SEARCH_UNLOCK(soc);
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, unexpected early return!\n"),
                      FUNCTION_NAME(),__LINE__));
           goto hw_traverse_exit;
        }
    }

    do {
        this_valid_index = -1;  /* assign for this search */
        sal_memset(&sw_arl, 0, sizeof(l2_arl_sw_entry_t));
        sal_memset(&sw_arl1, 0, sizeof(l2_arl_sw_entry_t));

        rv =  soc_arl_search_valid(unit, 
                _ARL_SEARCH_VALID_OP_GET, (uint32 *)&index, 
                (uint32 *)&sw_arl, (uint32 *)&sw_arl1);
        ARL_MEM_SEARCH_UNLOCK(soc);

        if (rv != SOC_E_EXISTS){
            break;
        } else {
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                /* Search valid mechanism return 2 entries each time */
                index = index * 2;
            }
            rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID,
                    (uint32 *)&sw_arl, &valid);
            if(SOC_FAILURE(rv)){
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, unexpected early return!\n"),
                          FUNCTION_NAME(),__LINE__));
                goto hw_traverse_exit;
            }
                
#ifdef  BCM_TB_SUPPORT
            /* TB devices was designed to report the valid search result by 
             *  one arl entry in each searched time. And the entry status may 
             *  be with valid or pending status. Both status were treat as 
             *  valid entry in ARL thread in current design.
             */
            if (SOC_IS_TBX(unit)){
                valid = (valid ==_TB_ARL_STATUS_PENDING || 
                        valid ==_TB_ARL_STATUS_VALID) ? 1 : 0;
            }
#endif  /* BCM_TB_SUPPORT */
            if (valid) {
                this_valid_index = index;
                                
                traverse_cnt ++;

                this_l2addr = (bcm_l2_addr_t *)trav_st->data;
                /* special process for TB on assigning l2addr->port at 
                 * GPORT_SUBPORT_PORT type to requet the reported l2 entry 
                 * on reporting most detail information(include vport)
                 */
                if (SOC_IS_TBX(unit)){
                    /* assigning a GPORT type only, id assigned to 0 */
                    BCM_GPORT_SUBPORT_PORT_SET(this_l2addr->port, 0);
                }
                _bcm_robo_l2_from_arl(unit, this_l2addr, &sw_arl);    

                /* performing key-match process */
                match_rv = _bcm_robo_l2_match_op(unit, 
                        match_addr, this_l2addr);
                if (match_rv < BCM_E_NONE){
                    /* BCM_E_NOT_FOUND|BCM_E_FAIL means no match */
                    if (!(match_rv == BCM_E_NOT_FOUND || 
                            match_rv == BCM_E_FAIL)){
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s, Failed matching on L2(id=%d)!\n"), 
                                  FUNCTION_NAME(), index));
                        return match_rv;
                    }
                } else {
                    /* matched process */
                    trav_st->mem_idx = this_valid_index;
                    LOG_INFO(BSL_LS_BCM_L2TABLE,
                             (BSL_META_U(unit,
                                         "%s, arl_index=%d, L2x[0-2]=%08x-%08x-%08x\n"), 
                              FUNCTION_NAME(), trav_st->mem_idx, 
                              *(uint32 *)&sw_arl, *((uint32 *)&sw_arl + 1), 
                              *((uint32 *)&sw_arl + 2)));

                    /* --- traverse callback while matched --- */
                    rv = trav_st->user_cb(unit, 
                            trav_st->data, trav_st->user_data);
                    if (SOC_FAILURE(rv)){
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s,%d, unexpected early return!\n"),
                                  FUNCTION_NAME(),__LINE__));
                        goto hw_traverse_exit;
                    }
                }
            }
            
            /* Deal the second search entry */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                rv = DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID,
                    (uint32 *)&sw_arl1, &valid);
                if (SOC_FAILURE(rv)){
                    LOG_WARN(BSL_LS_BCM_COMMON,
                             (BSL_META_U(unit,
                                         "%s,%d, unexpected early return!\n"),
                              FUNCTION_NAME(),__LINE__));
                    goto hw_traverse_exit;
                }

                if (valid) {
                    index = index + 1;
                    
                    traverse_cnt ++;

                    this_l2addr = (bcm_l2_addr_t *)trav_st->data;
                    _bcm_robo_l2_from_arl(unit, this_l2addr, &sw_arl1);   
                    
                    /* performing key-match process */
                    match_rv = _bcm_robo_l2_match_op(unit, 
                            match_addr, this_l2addr);
                    if (match_rv < BCM_E_NONE){
                        /* BCM_E_NOT_FOUND|BCM_E_FAIL means no match */
                        if (!(match_rv == BCM_E_NOT_FOUND || 
                                match_rv == BCM_E_FAIL)){
                            LOG_WARN(BSL_LS_BCM_COMMON,
                                     (BSL_META_U(unit,
                                                 "%s, Failed matching on L2(id=%d)!\n"), 
                                      FUNCTION_NAME(), index));
                            return match_rv;
                        }
                    } else {
                        /* matched process */
                        trav_st->mem_idx = this_valid_index;
                        LOG_INFO(BSL_LS_BCM_L2TABLE,
                                 (BSL_META_U(unit,
                                             "%s, arl_index=%d, L2x[0-2]=%08x-%08x-%08x\n"), 
                                  FUNCTION_NAME(), trav_st->mem_idx, 
                                  *(uint32 *)&sw_arl1, *((uint32 *)&sw_arl1 + 1), 
                                  *((uint32 *)&sw_arl1 + 2)));
                        
                        /* --- traverse callback while matched --- */
                        rv = trav_st->user_cb(unit, 
                                trav_st->data, trav_st->user_data);
                        if (SOC_FAILURE(rv)){
                            LOG_WARN(BSL_LS_BCM_COMMON,
                                     (BSL_META_U(unit,
                                                 "%s,%d, unexpected early return!\n"),
                                      FUNCTION_NAME(),__LINE__));
                            goto hw_traverse_exit;
                        }
                    }
                }
            }
            
        }

        rv =  soc_arl_search_valid(unit, 
                _ARL_SEARCH_VALID_OP_DONE, NULL, NULL, NULL);
        if (rv == SOC_E_NONE) {
            /* means L2(HW) scaned complete! */
            break;
        } else {    /* SOC_E_BUSY */
            ARL_MEM_SEARCH_LOCK(soc);
            rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT,
                    NULL, NULL, NULL);
            if (SOC_FAILURE(rv)){
                ARL_MEM_SEARCH_UNLOCK(soc);

                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d, unexpected early return!\n"),
                          FUNCTION_NAME(),__LINE__));
                goto hw_traverse_exit;
            }
        }
    } while (this_valid_index > -1);

hw_traverse_exit :
    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "%s,L2(HW) traverse finished! traverse_cnt=%d\n"),
              FUNCTION_NAME(), traverse_cnt));
    if (BCM_SUCCESS(rv)){ 
        if (response_type == _BCM_ROBO_L2TRV_RESP_TRAVERSED){
            rv = traverse_cnt;        
        } else {
            /* treate as _BCM_ROBO_L2TRV_RESP_BCM_NORMAL */
            rv = BCM_E_NONE;        
        }
    }

    return rv;
}

/* BCM_ROBO_L2_TRAVERSE_NO_CHUNK :
 *  - means the L2 traverse will be operated with no chunk basis.
 *  
 * p.s 1. chunksize can be used per user request once the L2 traverse may 
 *      causes any performance concern.
 * p.s 2. chunk basis operation need SLEEP per chunk.
 */
#define BCM_ROBO_L2_TRAVERSE_NO_CHUNK   1

/*
 * Function:
 *     _bcm_robo_l2_traverse_mem
 * Description:
 *      Helper function to _bcm_robo_l2_traverse to itterate over given memory 
 *      and actually read the table and parse entries.
 * Parameters:
 *      unit            device number
 *      mem             L2 memory to read
 *      match_addr      L2 filter keys for matching process 
 *      trav_st         Traverse structure with all the data.
 *      sync_detect     TRUE/FALSE, to indicate the traverse process will 
 *                       be performedin l2 frozen sync detected condition.
 *      response_type   _BCM_ROBO_L2TRV_RESP_XXX for the user requesting 
 *                      return value type.
 *
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_robo_l2_traverse_mem(int unit, soc_mem_t mem, 
        _bcm_robo_l2_match_t *match_addr, _bcm_robo_l2_traverse_t *trav_st, 
        int sync_detect, uint32 response_type)
{
    /* Indexes to iterate over memories, chunks and entries */
    int     chnk_idx, ent_idx;
    int     chnk_idx_max = 0, mem_idx_max = 0, sync_detect_max = 0;
    int     buf_size = 0, chunksize = 0, copysize = 0, proc_size = 0;
    int     rv = BCM_E_NONE, frozen = 0, sync_detect_op = 0, is_sync = 0;
    int     this_trv_cnt = 0;
    int     match_rv = BCM_E_NONE;
    
    /* Buffer to store chunk of L2 table we currently work on */
    uint32          *l2_tbl_chnk, valid = 0;
    soc_control_t   *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t *l2x_entry;
    bcm_l2_addr_t   *this_l2addr;

    if (!soc->arl_table || soc->arl_interval == 0) {
        /* mean's traverse solution can't applied on SW ARL table */
        return _bcm_robo_l2_hw_traverse_mem(unit, match_addr, trav_st, response_type);
    }

    if (!soc_robo_mem_index_count(unit, mem)) {
        return BCM_E_NONE;
    }

    mem_idx_max = soc_robo_mem_index_max(unit, mem);
    chunksize = soc_property_get(unit, spn_L2DELETE_CHUNKS,
                                 L2_ROBO_MEM_CHUNKS_DEFAULT);

#if BCM_ROBO_L2_TRAVERSE_NO_CHUNK
     proc_size = mem_idx_max + 1;
#else   /* BCM_ROBO_L2_TRAVERSE_NO_CHUNK */
     proc_size = chunksize;
#endif  /* BCM_ROBO_L2_TRAVERSE_NO_CHUNK */
    
    buf_size = sizeof(l2_arl_sw_entry_t) * proc_size;
    l2_tbl_chnk = soc_cm_salloc(unit, buf_size, "l2 traverse");
    if (NULL == l2_tbl_chnk) {
        return BCM_E_MEMORY;;
    }

    if (sync_detect) {
        /* no sync_detect finally if the L2 is not freezing */
        BCM_IF_ERROR_RETURN(soc_robo_arl_is_frozen(unit, &frozen));
        sync_detect_op = (frozen) ? TRUE : FALSE;
    }

    /* Flow for L2 traverse :
     *  1. sync_detect process : wait for sync; max op count for this wait.
     *  2. traverse process : 
     *      - loop on each valid SW ARL entries
     *      - matching operation and issue the user function on the matched entry.
     */
    if (sync_detect_op) {
        sync_detect_max = (int)(mem_idx_max/chunksize);
        while (sync_detect_max > 0) {
            soc_arl_frozen_sync_status(unit, &is_sync);
            if (is_sync){
#if BCM_ROBO_L2_TRAVERSE_NO_CHUNK
                ARL_SW_TABLE_LOCK(soc);
                sal_memcpy((void *)l2_tbl_chnk, &soc->arl_table[0], 
                        sizeof(l2_arl_sw_entry_t) * proc_size);
                ARL_SW_TABLE_UNLOCK(soc);
#endif  /* BCM_ROBO_L2_TRAVERSE_NO_CHUNK */
                break;
            } else {
                sync_detect_max-- ;
            }
            
            sal_usleep(1000000);
        } 
        LOG_INFO(BSL_LS_BCM_L2TABLE,
                 (BSL_META_U(unit,
                             "%s, is_sync=%d, sync_detect count=%d!\n"), 
                  FUNCTION_NAME(), is_sync, 
                  (int)(mem_idx_max/chunksize) - sync_detect_max));
    }

    for (chnk_idx = soc_robo_mem_index_min(unit, mem); 
        chnk_idx <= mem_idx_max; 
        chnk_idx += proc_size) {
#if !(BCM_ROBO_L2_TRAVERSE_NO_CHUNK)
        sal_memset((void *)l2_tbl_chnk, 0, buf_size);
#endif /* !BCM_ROBO_L2_TRAVERSE_NO_CHUNK */

        chnk_idx_max = ((chnk_idx + proc_size) < mem_idx_max) ? 
                chnk_idx + proc_size : mem_idx_max;

        copysize = chnk_idx_max - chnk_idx + 
                ((chnk_idx_max == mem_idx_max) ? 1 : 0);
                
        if (!(sync_detect_op && is_sync)){
            ARL_SW_TABLE_LOCK(soc);
            sal_memcpy((void *)l2_tbl_chnk, &soc->arl_table[chnk_idx], 
                    sizeof(l2_arl_sw_entry_t) * copysize);
            ARL_SW_TABLE_UNLOCK(soc);
        }
        
        l2x_entry = (l2_arl_sw_entry_t *)l2_tbl_chnk;
        for (ent_idx = 0 ; ent_idx < copysize; ent_idx ++) {
            rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)l2x_entry, &valid);
            
            if (valid){
                this_l2addr = (bcm_l2_addr_t *)trav_st->data;
                
                /* special process for TB on assigning l2addr->port at 
                 * GPORT_SUBPORT_PORT type to requet the reported l2 entry 
                 * on reporting most detail information(include vport)
                 */
                if (SOC_IS_TBX(unit)){
                    /* assigning a GPORT type only, id assigned to 0 */
                    BCM_GPORT_SUBPORT_PORT_SET(this_l2addr->port, 0);
                }
                _bcm_robo_l2_from_arl(unit, this_l2addr, l2x_entry);    
                
                /* performing key-match process */
                match_rv = _bcm_robo_l2_match_op(unit, match_addr, this_l2addr);
                if (match_rv < BCM_E_NONE){
                    /* BCM_E_NOT_FOUND | BCM_E_FAIL means no matched condition */
                    if (!(match_rv == BCM_E_NOT_FOUND || match_rv == BCM_E_FAIL)){
                        LOG_WARN(BSL_LS_BCM_COMMON,
                                 (BSL_META_U(unit,
                                             "%s, Failed matching process on L2(id=%d)!\n"), 
                                  FUNCTION_NAME(), chnk_idx + ent_idx));
                        return match_rv;
                    }
                } else {
                    /* matched condition */
                    trav_st->mem_idx = chnk_idx + ent_idx;
                    LOG_INFO(BSL_LS_BCM_L2TABLE,
                             (BSL_META_U(unit,
                                         "%s, arl_index=%d, L2x[0-2]=%08x-%08x-%08x\n"), 
                              FUNCTION_NAME(), trav_st->mem_idx, 
                              *(uint32 *)l2x_entry, *((uint32 *)l2x_entry + 1), 
                              *((uint32 *)l2x_entry + 2)));

                    this_trv_cnt ++;
                    
                    rv = trav_st->user_cb(unit, trav_st->data, trav_st->user_data);
                    /* rv >= 0 in SDK is used to indicate no error return 
                     *  - here we use rv > 0 for callback function's response.
                     */
                    if (BCM_FAILURE(rv)) {
                        soc_cm_sfree(unit, l2_tbl_chnk);
                        return rv;
                    }
                }
            }
            l2x_entry++;
        }
        
        /* sleep before next chunk process */
        if (chnk_idx < mem_idx_max) {
            LOG_INFO(BSL_LS_BCM_L2TABLE,
                     (BSL_META_U(unit,
                                 "%s,%d,Chunk SLEEP...\n"),FUNCTION_NAME(),__LINE__));
            sal_usleep(100000);
        }
    } 

    soc_cm_sfree(unit, l2_tbl_chnk);

    if (response_type == _BCM_ROBO_L2TRV_RESP_TRAVERSED){
        rv = this_trv_cnt;        
    } else {
        /* treate as _BCM_ROBO_L2TRV_RESP_BCM_NORMAL */
        rv = BCM_E_NONE;        
    }
    LOG_INFO(BSL_LS_BCM_L2TABLE,
             (BSL_META_U(unit,
                         "%s, this_trv_cnt=%d, rv=%d\n"), 
              FUNCTION_NAME(), this_trv_cnt, rv));
    return rv;
}


/*
 * Function:
 *     _bcm_robo_l2_traverse
 * Description:
 *      Helper function to bcm_robo_l2_traverse to itterate over table 
 *      and actually read the momery
 * Parameters:
 *     unit         device number
 *     trav_st      Traverse structure with all the data.
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_robo_l2_traverse(int unit, _bcm_robo_l2_traverse_t *trav_st)
{
    int rv = BCM_E_UNAVAIL; 

    rv = _bcm_robo_l2_traverse_mem(unit, INDEX(L2_ARLm), NULL, trav_st, FALSE, 0);

    return rv;
}


/*
 * Function:
 *     bcm_robo_l2_traverse
 * Description:
 *     To traverse the L2 table and call provided callback function with matched entry
 * Parameters:
 *     unit         device number
 *     trav_fn      User specified callback function 
 *     user_data    User specified cookie
 * Return:
 *     BCM_E_XXX
 */
int 
bcm_robo_l2_traverse(int unit, bcm_l2_traverse_cb trav_fn, void *user_data)
{
    _bcm_robo_l2_traverse_t  trav_st;
    bcm_l2_addr_t       l2_entry;
       
    if (!trav_fn) {
        return (BCM_E_PARAM);
    }

    sal_memset(&trav_st, 0, sizeof(_bcm_robo_l2_traverse_t));
    sal_memset(&l2_entry, 0, sizeof(bcm_l2_addr_t));

    trav_st.pattern = NULL;
    trav_st.data = &l2_entry;
    trav_st.mem_idx = -1;   /* -1 to indicate a invlalid table index */
    trav_st.user_cb = trav_fn;
    trav_st.user_data = user_data;

    return (_bcm_robo_l2_traverse(unit, &trav_st));
}

/*
 * Function:
 *     bcm_robo_l2_matched_traverse
 * Description:
 *     To traverse the L2 table and call provided callback function with matched entry
 * Parameters:
 *     unit         device number
 *     flags        BCM_L2_TRAVERSE_MATCH_XXX
 *     match_addr   L2 parameters to match on traverse process
 *     trav_fn      User specified callback function 
 *     user_data    User specified cookie
 * Return:
 *     BCM_E_XXX
 */
int 
bcm_robo_l2_matched_traverse(
        int unit, uint32 flags, bcm_l2_addr_t *match_addr, 
        bcm_l2_traverse_cb trav_fn, void *user_data)
{
    int     rv = BCM_E_NONE;
    uint32  trvrs_flags = 0;

    bcm_l2_addr_t       l2_entry;
    _bcm_robo_l2_match_t    int_match_addr;
    _bcm_robo_l2_traverse_t trav_st;

    L2_INIT(unit);
    
    if (!trav_fn) {
        return (BCM_E_PARAM);
    }
    if (!match_addr) {
        return (BCM_E_PARAM);
    }

    /* supporting flags check  */
    if (flags & ~(_BCM_L2_ROBO_MATCH_TRAVERSE_SUPPORT_FLAGS)) {
        return BCM_E_PARAM;
    } else {
        trvrs_flags = flags & _BCM_L2_ROBO_MATCH_TRAVERSE_SUPPORT_FLAGS;
    }

    sal_memset(&trav_st, 0, sizeof(_bcm_robo_l2_traverse_t));
    sal_memset(&l2_entry, 0, sizeof(bcm_l2_addr_t));
    trav_st.pattern = NULL;
    trav_st.data = &l2_entry;
    trav_st.mem_idx = -1;   /* -1 to indicate a invlalid table index */
    trav_st.user_cb = trav_fn;
    trav_st.user_data = user_data;
    
    /* prepare the internal matching process */
    sal_memset(&int_match_addr, 0, sizeof(_bcm_robo_l2_match_t));

    /* Match key for static status through BCM_L2_TRAVERSE_MATCH_STATIC 
     *  instead of int_match_addr->flags=BCM_L2_STATIC 
     */
    if (trvrs_flags & BCM_L2_TRAVERSE_MATCH_STATIC) {
        /* API behavior : means dynamic + static */
        int_match_addr.op_flags &= ~_BCM_ROBO_L2_MATCH_OP_STATIC;
    } else {
        /* API behavior : means dynamic only */
        int_match_addr.op_flags |= _BCM_ROBO_L2_MATCH_OP_STATIC;
        int_match_addr.match_others &= ~BCM_L2_STATIC;
    }
    
    if (trvrs_flags & _BCM_L2_TRAVERSE_MATCH_PENDING) {
        int_match_addr.op_flags |= _BCM_ROBO_L2_MATCH_OP_PENDING;
        if (match_addr->flags & BCM_L2_PENDING) {
            /* API behavior:means matching pending only */
            int_match_addr.match_others |= BCM_L2_PENDING;
        } else {
            /* API behavior:means matching non-pending only */
            int_match_addr.match_others &= ~BCM_L2_PENDING;
        }
    }
    
    if (trvrs_flags & BCM_L2_TRAVERSE_MATCH_MAC) {
        int_match_addr.op_flags |= _BCM_ROBO_L2_MATCH_OP_MAC;
        sal_memcpy(int_match_addr.mac, match_addr->mac, sizeof(bcm_mac_t));
    }
    
    if (trvrs_flags & BCM_L2_TRAVERSE_MATCH_VLAN) {
        int_match_addr.op_flags |= _BCM_ROBO_L2_MATCH_OP_VID;
        int_match_addr.vid = match_addr->vid;
    }
    
    if (trvrs_flags & BCM_L2_TRAVERSE_MATCH_DEST) {
        int_match_addr.op_flags |= _BCM_ROBO_L2_MATCH_OP_DEST;
        if (BCM_MAC_IS_MCAST(match_addr->mac)) {
            int_match_addr.mc_group = match_addr->l2mc_group;
        } else {
            int_match_addr.port = match_addr->port;
        }
    }
    
    if (!(trvrs_flags & BCM_L2_TRAVERSE_IGNORE_DES_HIT)) {
        int_match_addr.op_flags |= _BCM_ROBO_L2_MATCH_OP_HIT;
        int_match_addr.match_others = BCM_L2_HIT;
    }
    if (!(trvrs_flags & BCM_L2_TRAVERSE_IGNORE_DISCARD_SRC)) {
        int_match_addr.op_flags |= _BCM_ROBO_L2_MATCH_OP_DIS_SRC;
        int_match_addr.match_others = BCM_L2_DISCARD_SRC;
    }

    rv = _bcm_robo_l2_traverse_mem(unit, INDEX(L2_ARLm), &int_match_addr, 
            &trav_st, TRUE, _BCM_ROBO_L2TRV_RESP_TRAVERSED);

    return rv;
}

#define _BCM_L2_REPLACE_ALL_LEGAL            (BCM_L2_REPLACE_MATCH_MAC | \
                                              BCM_L2_REPLACE_MATCH_VLAN | \
                                              BCM_L2_REPLACE_MATCH_DEST | \
                                              BCM_L2_REPLACE_MATCH_STATIC   | \
                                              BCM_L2_REPLACE_DELETE | \
                                              BCM_L2_REPLACE_NEW_TRUNK | \
                                              BCM_L2_REPLACE_PENDING   | \
                                              BCM_L2_REPLACE_NO_CALLBACKS)
/*
 * Function:
 *     _bcm_robo_l2_replace_flags_validate
 * Description:
 *     THelper function to bcm_l2_replace API to validate given flags
 * Parameters:
 *     flags        flags BCM_L2_REPLACE_* 
 * Return:
 *     BCM_E_NONE - OK 
 *     BCM_E_PARAM - Failure
 */
int 
_bcm_robo_l2_replace_flags_validate(uint32 flags)
{
    if (!flags) {
        return BCM_E_PARAM;
    }
    if ((flags & BCM_L2_REPLACE_DELETE) && 
            (flags & BCM_L2_REPLACE_NEW_TRUNK)) {
        return BCM_E_PARAM;
    }
    if ((flags > (BCM_L2_REPLACE_MATCH_MAC | BCM_L2_REPLACE_MATCH_VLAN |
        BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_STATIC)) && 
        (flags < BCM_L2_REPLACE_DELETE)) {
        return BCM_E_PARAM;
    }
    if (flags > _BCM_L2_REPLACE_ALL_LEGAL) {
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_robo_l2_addr_replace_by_mac_vlan
 * Description:
 *     Helper function to bcm_l2_replace API to replace l2 entries by mac
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     rep_st       structure with information of what to replace 
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_robo_l2_addr_replace_by_mac_vlan(int unit, 
        _bcm_robo_l2_replace_t *rep_st) {

    uint32  action_flag = 0, port = 0, temp = 0;
    int  rv = BCM_E_NONE;
    bcm_l2_addr_t  l2addr;
    bcm_trunk_info_t  trunk_info;
    l2_arl_sw_entry_t  arl_search, arl_result;
    bcm_trunk_t  tid;
    bcm_trunk_member_t  member_array[BCM_TRUNK_MAX_PORTCNT];
    int  member_count;
    
    bcm_l2_addr_init(&l2addr, rep_st->match_mac, rep_st->match_vid);

    /* look up the l2 entry with mac+vid */   
    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_search, &l2addr));
    
    action_flag = DRV_MEM_OP_BY_HASH_BY_MAC | 
                 DRV_MEM_OP_BY_HASH_BY_VLANID;
        
    rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)&arl_search,
            (uint32 *)&arl_result, NULL, action_flag);
    
    if (rv != BCM_E_EXISTS) {
        return BCM_E_NOT_FOUND;
    }

    if (rep_st->isStatic) {
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_STATIC, (uint32 *)&arl_result, &temp));
        if (!temp) {
            return BCM_E_NOT_FOUND;
        }
    }

    if (rep_st->isPending) {
#ifdef  BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)) {
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VALID, (uint32 *)&arl_result, &temp));
            if (temp != _TB_ARL_STATUS_PENDING) {
                return BCM_E_NOT_FOUND;
            }
        }
#endif  /* BCM_TB_SUPPORT */
    }

    if (rep_st->isTrunk) {
        /* get the port field */
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_SRC_PORT, (uint32 *)&arl_result, &port));
                            
        bcm_trunk_find(unit, 0, port, &tid);
        /* check the port existed in the match_trunk ? */
        if (tid != rep_st->match_trunk) {
            return BCM_E_PARAM;
        }

        /* check the new_trunk valid ?*/
        if (rep_st->new_trunk== BCM_TRUNK_INVALID) {
            return BCM_E_PARAM;
        }    
        rv = bcm_trunk_get(unit, rep_st->new_trunk, &trunk_info, 
                 BCM_TRUNK_MAX_PORTCNT, &member_array[0], &member_count);
    
        if (rv < 0) {
            return BCM_E_PARAM;
        }
    
        /* 
         * If no any port assigned to the current trunk,
         * there will be no further processes.
         * Since no tgid information recorded in the arl entry of robo chip.
         */
        if (!member_count) {
            return BCM_E_UNAVAIL;
        }
        /* pick any port in the new_trunk */
        l2addr.port = member_array[0].gport;
    }else {

        if (rep_st->isDel){            
            return bcm_l2_addr_delete(unit, 
                    rep_st->match_mac, rep_st->match_vid);
        } 

        l2addr.port = rep_st->new_port;
    }
    return bcm_l2_addr_add(unit, &l2addr);

}

/*
 * Function:
 *     bcm_robo_l2_replace
 * Description:
 *     To replace destination (or delete) multiple L2 entries
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     match_addr   L2 parameters to match on delete/replace
 *     new_module   new module ID for a replace 
 *     new_port     new port for a replace
 *     new_trunk    new trunk for a replace  
 * Return:
 *     BCM_E_XXX
 */
int 
bcm_robo_l2_replace(int unit, uint32 flags, bcm_l2_addr_t *match_addr,
                   bcm_module_t new_module, bcm_port_t new_port, 
                   bcm_trunk_t new_trunk)
{
    uint32              cmp_flags = 0, l2_flags = 0;
    uint32              int_match_flags = 0;
    int                 rv = BCM_E_UNAVAIL, freezing = BCM_E_NONE;
    bcm_l2_addr_t       int_match_addr;
    _bcm_robo_l2_replace_t   rep_st;

    L2_INIT(unit);

    /* SDK-33848 : Follow the desig change on match_addr = NULL */
    /* API Guide : Using the BCM_L2_REPLACE_DELETE flag and matching_addr as 
     *      NULL will delete all addresses from the L2 table and parameters 
     *      new_module, new_port and new_trunk will be ignored. 
     */
    if (NULL == match_addr) {
        if (!(flags & BCM_L2_REPLACE_DELETE)) {
            return BCM_E_PARAM;
        }
        flags &= ~(BCM_L2_REPLACE_MATCH_VLAN | BCM_L2_REPLACE_MATCH_MAC |
                   BCM_L2_REPLACE_MATCH_DEST);

        if (soc_feature(unit, soc_feature_l2_pending)) {
            if (flags & BCM_L2_REPLACE_IGNORE_PENDING) {
                goto remove_all_action;
            } else {
                sal_memset(&int_match_addr, 0, sizeof(bcm_l2_addr_t));
                
                /* force PENDING matching key (STATIC key must not been set)
                 *  
                 * Note :
                 *  1. force STATIC key for matching means both static and 
                 *      dynamic ARL will be matched.
                 */
                int_match_flags = _BCM_L2_TRAVERSE_MATCH_PENDING | 
                        BCM_L2_TRAVERSE_MATCH_STATIC;
                if (flags & BCM_L2_REPLACE_PENDING){
                    int_match_addr.flags |= BCM_L2_PENDING;
                } else {
                    int_match_addr.flags &= ~BCM_L2_PENDING;
                }                
                /* normal match traverse need IGNORE related flags */
                int_match_flags |= BCM_L2_TRAVERSE_IGNORE_DISCARD_SRC |
                        BCM_L2_TRAVERSE_IGNORE_DES_HIT;
                
                freezing = bcm_robo_l2_addr_freeze(unit);
                if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
                    rv = bcm_robo_l2_matched_traverse(unit, 
                            int_match_flags, &int_match_addr, 
                            _bcm_robo_l2_remove_cb, NULL);
                    if (freezing == BCM_E_NONE){
                        BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
                    }
                } else {
                    rv = freezing;
                }
                
                if (BCM_SUCCESS(rv)){
                    LOG_INFO(BSL_LS_BCM_L2TABLE,
                             (BSL_META_U(unit,
                                         "%s, %d L2 entries were removed!\n"),
                              FUNCTION_NAME(), rv));
                    rv = BCM_E_NONE;
                }
            }

            return rv;
        }

remove_all_action : 
        /* performing delete all process 
         *  - call DRV_MEM_CLEAR() to remove all existed ARL entries.
         */
        freezing = bcm_robo_l2_addr_freeze(unit);
        if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
            rv = DRV_MEM_CLEAR(unit, DRV_MEM_ARL);
            if (freezing == BCM_E_NONE){
                BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
            }
        } else {
            rv = freezing;
        }

        return rv;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_replace_flags_validate(flags));
    sal_memset(&rep_st, 0, sizeof(_bcm_robo_l2_replace_t));

    if (0 == (flags & BCM_L2_REPLACE_DELETE)) {
        if (BCM_GPORT_IS_SET(new_port)) {
            bcm_port_t  tmp_port;
            int         tmp_id;

            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, new_port, &new_module, &tmp_port,
                                       &new_trunk, &tmp_id));
            new_port = tmp_port;
        } 
        if (flags & BCM_L2_REPLACE_NEW_TRUNK) { 

            /*   All current ROBO devices have no feature of TRUNK base L2 
             * learning. Thus there will be no trunk_identifier in L2 entry on
             * ROBO devices.
             */
            return BCM_E_UNAVAIL;
        } else {
            if (new_module != 0) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_VALID(unit, new_port)) {
                return BCM_E_PORT;
            }
        }
        rep_st.new_module = new_module;
        rep_st.new_port = new_port;
    } else {
        rep_st.isDel = 1;
    }
    cmp_flags = flags &  ( BCM_L2_REPLACE_MATCH_MAC | 
                                        BCM_L2_REPLACE_MATCH_VLAN |
                                        BCM_L2_REPLACE_MATCH_DEST );

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        if ((flags & BCM_L2_REPLACE_MATCH_STATIC) || 
                (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) {
            rep_st.isStatic |= _BCM_TB_L2_REPLACE_MATCH_STATIC;
        }
        
        if (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY){
            rep_st.isStatic |= _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY;
        }
        
        if (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST){
            rep_st.isMcast |= _BCM_TB_L2_REPLACE_MATCH_MCAST;
        }

        if (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY){
            rep_st.isMcast |= _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY;
        }
    } else {
        if (flags & BCM_L2_REPLACE_MATCH_STATIC) {
            rep_st.isStatic = 1;
        }
    }
#else   /* BCM_TB_SUPPORT */
    if (flags & BCM_L2_REPLACE_MATCH_STATIC) {
        rep_st.isStatic = 1;
    }
#endif  /* BCM_TB_SUPPORT */


    if (flags & BCM_L2_REPLACE_PENDING) {
        rep_st.isPending = 1;
    }

    switch (cmp_flags) {
        case BCM_L2_REPLACE_MATCH_MAC: 
        {        
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    l2_flags = BCM_L2_DELETE_STATIC;
                }
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                rv = bcm_l2_addr_delete_by_mac(unit, 
                        match_addr->mac, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        }
        case BCM_L2_REPLACE_MATCH_VLAN: 
        {
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }
                rv = bcm_l2_addr_delete_by_vlan(unit, 
                        match_addr->vid, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        }
        case (BCM_L2_REPLACE_MATCH_MAC | BCM_L2_REPLACE_MATCH_VLAN):
        {
            sal_memcpy(&(rep_st.match_mac), match_addr->mac, 
                    sizeof(bcm_mac_t));
            rep_st.match_vid = match_addr->vid;
            rv =_bcm_robo_l2_addr_replace_by_mac_vlan(unit, &rep_st);
            break;
        }
        case BCM_L2_REPLACE_MATCH_DEST: 
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }

                rv = bcm_robo_l2_addr_delete_by_port(unit, 
                        match_addr->modid, match_addr->port, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case (BCM_L2_REPLACE_MATCH_MAC |BCM_L2_REPLACE_MATCH_DEST):
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }
                rv = bcm_robo_l2_addr_delete_by_mac_port(unit, 
                        match_addr->mac, match_addr->modid, 
                        match_addr->port, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case (BCM_L2_REPLACE_MATCH_VLAN |
                    BCM_L2_REPLACE_MATCH_DEST ):
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }
                rv = bcm_robo_l2_addr_delete_by_vlan_port(unit, 
                        match_addr->vid, match_addr->modid, 
                        match_addr->port, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case (BCM_L2_REPLACE_MATCH_MAC |BCM_L2_REPLACE_MATCH_VLAN |
                    BCM_L2_REPLACE_MATCH_DEST ):
        {
            rv = BCM_E_UNAVAIL;
            break;
        }
        default:
            break;

    }

    return rv;
}

/*
 * Function:
 *     bcm_robo_l2_station_add
 * Description:
 *     Add an entry to L2 Station Table      
 * Parameters:
 *     unit         - device number
 *     station_id   - (IN/OUT) Station ID
 *     station      - Pointer to station address information
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_l2_station_add(int unit, int *station_id, bcm_l2_station_t *station)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_l2_station_delete
 * Description:
 *     Delete an entry from L2 Station Table
 * Parameters:
 *     unit         - device number
 *     station_id   - (IN) Station ID
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_l2_station_delete(int unit, int station_id)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_l2_station_delete_all
 * Description:
 *     Clear all L2 Station Table entries
 * Parameters:
 *     unit         - device number
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_l2_station_delete_all(int unit)
{
    return (BCM_E_UNAVAIL);
}                                                                           
                                                                            
/*                                                                          
 * Function:                                                                
 *     bcm_robo_l2_station_get                                               
 * Description:                                                             
 *     Get L2 station entry detail from Station Table                       
 * Parameters:                                                              
 *     unit         - device number                                         
 *     station_id   - (IN) Station ID                                       
 *     station      - (OUT) Pointer to station address information.         
 * Return:                                                                  
 *     BCM_E_XXX                                                            
 */                                                                         
int                                                                         
bcm_robo_l2_station_get(int unit, int station_id, bcm_l2_station_t *station) 
{                                                                           
    return (BCM_E_UNAVAIL);                                                 
}                                                                           
                                                                            
/*                                                                          
 * Function:                                                                
 *     bcm_robo_l2_station_size_get                                          
 * Description:                                                             
 *     Get size of L2 Station Table                                         
 * Parameters:                                                              
 *     unit - device number                                                 
 *     size - (OUT) L2 Station table size                                   
 * Return:                                                                  
 *     BCM_E_XXX                                                            
 */                                                                         
int                                                                         
bcm_robo_l2_station_size_get(int unit, int *size)                            
{                                                                           
    return (BCM_E_UNAVAIL);                                                 
}

/*                                                                          
 * Function:                                                                
 *     bcm_robo_l2_station_traverse                                          
 * Description:                                                             
 *     Traverse L2 Station Table                                         
 * Parameters:                                                              
 *     unit        -  device number    
 *     trv_cb     -  call back function
 *     use_data - user data                                   
 * Return:                                                                  
 *     BCM_E_XXX                                                            
 */   
int 
bcm_robo_l2_station_traverse(int unit, bcm_l2_station_traverse_cb trv_cb, void *use_data)
{
    return (BCM_E_UNAVAIL);
}


