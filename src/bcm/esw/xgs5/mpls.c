/*
 * $Id: mpls.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    mpls.c
 * Purpose: Manages Tomahawk, TD2+ MPLS functions
 */
#include <shared/bsl.h>

#include <soc/defs.h>
#include <sal/core/libc.h>
#include <shared/bsl.h>

#ifdef INCLUDE_L3
#ifdef BCM_MPLS_SUPPORT
#if (defined(BCM_TOMAHAWK_SUPPORT) || defined(BCM_TRIDENT2PLUS_SUPPORT))

#include <soc/drv.h>
#include <soc/hash.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/triumph.h>

#include <bcm/error.h>
#include <bcm/mpls.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/tomahawk.h>
#include <bcm_int/esw/triumph3.h>
#include <bcm_int/esw/trident2plus.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/xgs3.h>
#include <bcm_int/esw/xgs5.h>
#include <bcm_int/esw/mpls.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/ecn.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/api_xlate_port.h>
#include <bcm_int/esw/failover.h>

#define MPLS_INFO(_unit_)   (&_bcm_tr_mpls_bk_info[_unit_])
#define L3_INFO(_unit_)   (&_bcm_l3_bk_info[_unit_])

#define bcmi_mpls_bookkeeping_t  _bcm_tr_mpls_bookkeeping_t

bcmi_egr_ip_tnl_mpls_tunnel_entry_t **egr_mpls_tnl_sw_state[BCM_MAX_NUM_UNITS];
bcmi_egr_ip_tnl_free_indexes_t fi_db[BCM_MAX_NUM_UNITS];

static soc_field_t _tnl_label_f[] =
{ MPLS_LABEL_0f, MPLS_LABEL_1f, MPLS_LABEL_2f, MPLS_LABEL_3f,
  MPLS_LABEL_4f, MPLS_LABEL_5f, MPLS_LABEL_6f, MPLS_LABEL_7f };
static soc_field_t _tnl_push_action_f[] =
{ MPLS_PUSH_ACTION_0f, MPLS_PUSH_ACTION_1f,
  MPLS_PUSH_ACTION_2f, MPLS_PUSH_ACTION_3f,
  MPLS_PUSH_ACTION_4f, MPLS_PUSH_ACTION_5f,
  MPLS_PUSH_ACTION_6f, MPLS_PUSH_ACTION_7f };
static soc_field_t _tnl_exp_select_f[] =
{ MPLS_EXP_SELECT_0f, MPLS_EXP_SELECT_1f,
  MPLS_EXP_SELECT_2f, MPLS_EXP_SELECT_3f,
  MPLS_EXP_SELECT_4f, MPLS_EXP_SELECT_5f,
  MPLS_EXP_SELECT_6f, MPLS_EXP_SELECT_7f };
static soc_field_t _tnl_exp_ptr_f[] =
{ MPLS_EXP_MAPPING_PTR_0f, MPLS_EXP_MAPPING_PTR_1f,
  MPLS_EXP_MAPPING_PTR_2f, MPLS_EXP_MAPPING_PTR_3f,
  MPLS_EXP_MAPPING_PTR_4f, MPLS_EXP_MAPPING_PTR_5f,
  MPLS_EXP_MAPPING_PTR_6f, MPLS_EXP_MAPPING_PTR_7f };
static soc_field_t _tnl_exp_f[] =
{ MPLS_EXP_0f, MPLS_EXP_1f, MPLS_EXP_2f, MPLS_EXP_3f,
  MPLS_EXP_4f, MPLS_EXP_5f, MPLS_EXP_6f, MPLS_EXP_7f };
static soc_field_t _tnl_ttl_f[] =
{ MPLS_TTL_0f, MPLS_TTL_1f, MPLS_TTL_2f, MPLS_TTL_3f,
  MPLS_TTL_4f, MPLS_TTL_5f, MPLS_TTL_6f, MPLS_TTL_7f };
static soc_field_t _tnl_pri_f[] =
{ NEW_PRI_0f, NEW_PRI_1f, NEW_PRI_2f, NEW_PRI_3f,
  NEW_PRI_4f, NEW_PRI_5f, NEW_PRI_6f, NEW_PRI_7f };
static soc_field_t _tnl_cfi_f[] =
{ NEW_CFI_0f, NEW_CFI_1f, NEW_CFI_2f, NEW_CFI_3f,
  NEW_CFI_4f, NEW_CFI_5f, NEW_CFI_6f, NEW_CFI_7f };
#if defined(BCM_TOMAHAWK2_SUPPORT)
static soc_field_t _tnl_ecn_ptr_f[] = 
{ IP_ECN_TO_EXP_MAPPING_PTR_0f, IP_ECN_TO_EXP_MAPPING_PTR_1f, 
  IP_ECN_TO_EXP_MAPPING_PTR_2f, IP_ECN_TO_EXP_MAPPING_PTR_3f,
  IP_ECN_TO_EXP_MAPPING_PTR_4f, IP_ECN_TO_EXP_MAPPING_PTR_5f, 
  IP_ECN_TO_EXP_MAPPING_PTR_6f, IP_ECN_TO_EXP_MAPPING_PTR_7f};
static soc_field_t _tnl_ecn_ptr_pri_f[] = 
{ IP_ECN_TO_EXP_PRIORITY_0f, IP_ECN_TO_EXP_PRIORITY_1f, 
  IP_ECN_TO_EXP_PRIORITY_2f, IP_ECN_TO_EXP_PRIORITY_3f,
  IP_ECN_TO_EXP_PRIORITY_4f, IP_ECN_TO_EXP_PRIORITY_5f, 
  IP_ECN_TO_EXP_PRIORITY_6f, IP_ECN_TO_EXP_PRIORITY_7f};
static soc_field_t _tnl_int_cn_ptr_f[] = 
{ INT_CN_TO_EXP_MAPPING_PTR_0f, INT_CN_TO_EXP_MAPPING_PTR_1f, 
  INT_CN_TO_EXP_MAPPING_PTR_2f, INT_CN_TO_EXP_MAPPING_PTR_3f,
  INT_CN_TO_EXP_MAPPING_PTR_4f, INT_CN_TO_EXP_MAPPING_PTR_5f, 
  INT_CN_TO_EXP_MAPPING_PTR_6f, INT_CN_TO_EXP_MAPPING_PTR_7f};
static soc_field_t _tnl_int_cn_ptr_pri_f[] = 
{ INT_CN_TO_EXP_PRIORITY_0f, INT_CN_TO_EXP_PRIORITY_1f, 
  INT_CN_TO_EXP_PRIORITY_2f, INT_CN_TO_EXP_PRIORITY_3f,
  INT_CN_TO_EXP_PRIORITY_4f, INT_CN_TO_EXP_PRIORITY_5f, 
  INT_CN_TO_EXP_PRIORITY_6f, INT_CN_TO_EXP_PRIORITY_7f};
#endif
/************************  FUNCTIONS BOUNDARY **********************/
STATIC int
_bcmi_xgs5_mpls_entry_get_key(int unit, mpls_entry_entry_t *ment,
                           bcm_mpls_tunnel_switch_t *info)
{
    bcm_port_t port_in, port_out;
    bcm_module_t mod_in, mod_out;
    bcm_trunk_t trunk_id;

    port_in = soc_MPLS_ENTRYm_field32_get(unit, ment, PORT_NUMf);
    mod_in = soc_MPLS_ENTRYm_field32_get(unit, ment, MODULE_IDf);
    if (soc_MPLS_ENTRYm_field32_get(unit, ment, Tf)) {
        trunk_id = soc_MPLS_ENTRYm_field32_get(unit, ment, TGIDf);
        BCM_GPORT_TRUNK_SET(info->port, trunk_id);
    } else if ((port_in == 0) && (mod_in == 0)) {
        /* Global label, mod/port not part of lookup key */
        info->port = BCM_GPORT_INVALID;
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    mod_in, port_in, &mod_out, &port_out));
        BCM_GPORT_MODPORT_SET(info->port, mod_out, port_out);
    } 
    info->label = soc_MPLS_ENTRYm_field32_get(unit, ment, MPLS_LABELf);

    return BCM_E_NONE;
}

/* Convert data part of HW entry to application format. */
STATIC int
_bcmi_xgs5_mpls_entry_get_data(int unit, mpls_entry_entry_t *ment,
                            bcm_mpls_tunnel_switch_t *info)
{
    int rv, nh_index, vrf;
    int action_if_bos, action_if_not_bos;
    bcm_if_t egress_if=0;
    int mode = 0;

    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_ingress_mode_get(unit, &mode));
    action_if_bos = soc_MPLS_ENTRYm_field32_get(unit, ment, MPLS_ACTION_IF_BOSf);
    action_if_not_bos = soc_MPLS_ENTRYm_field32_get(unit, ment, 
                                            MPLS_ACTION_IF_NOT_BOSf); 

    if (action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_IIF ||
        action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_POP) {
        if (!mode) {
            _bcm_l3_ingress_intf_t iif;
#ifdef BCM_TOMAHAWK_SUPPORT
            int tunnel_term_ecn_map_id;
#endif
            vrf = soc_MPLS_ENTRYm_field32_get(unit, ment, L3_IIFf);
            iif.intf_id =  vrf;
            vrf -= _BCM_TR_MPLS_L3_IIF_BASE;
            _BCM_MPLS_VPN_SET(info->vpn, _BCM_MPLS_VPN_TYPE_L3, vrf);
            rv = _bcm_tr_l3_ingress_interface_get(unit, NULL, &iif);
            BCM_IF_ERROR_RETURN(rv);
#ifdef BCM_TOMAHAWK_SUPPORT
            if (soc_feature(unit, soc_feature_mpls_ecn)) {
                tunnel_term_ecn_map_id = iif.tunnel_term_ecn_map_id;
                if (bcmi_xgs5_ecn_map_used_get(unit, tunnel_term_ecn_map_id,
                            _bcmEcnmapTypeTunnelTerm)) {
                    info->tunnel_term_ecn_map_id = tunnel_term_ecn_map_id |
                        _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM;
                    info->flags |= BCM_MPLS_SWITCH_TUNNEL_TERM_ECN_MAP;
                }
            }
#endif
        } else {
            info->ingress_if = soc_MPLS_ENTRYm_field32_get(unit, ment, L3_IIFf);
        } 
    }

    if (action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI ||
            action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI) {
        nh_index = soc_MPLS_ENTRYm_field32_get(unit, ment,
                                               NEXT_HOP_INDEXf);
        rv = bcm_tr_mpls_get_vp_nh (unit, nh_index,&egress_if);
        if (rv == BCM_E_NONE) {
            rv = bcm_tr_mpls_l3_nh_info_get(unit, info, nh_index);
        } else {
            info->egress_if = nh_index + BCM_XGS3_EGRESS_IDX_MIN;
            info->egress_label.label = BCM_MPLS_LABEL_INVALID;
            rv = BCM_E_NONE;
        }
        BCM_IF_ERROR_RETURN(rv);
    }

    if (action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_NHI ||
        action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_NHI) {
        info->action = BCM_MPLS_SWITCH_ACTION_PHP;
        nh_index = soc_MPLS_ENTRYm_field32_get(unit, ment,
                                               NEXT_HOP_INDEXf);
        info->egress_if = nh_index + BCM_XGS3_EGRESS_IDX_MIN;
    }

    if (action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_ECMP ||
        action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_ECMP ||
        action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_ECMP ||
        action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_ECMP) {
        info->action = BCM_MPLS_SWITCH_ACTION_SWAP;
        nh_index = soc_MPLS_ENTRYm_field32_get(unit, ment,
                ECMP_PTRf);
        info->egress_if = nh_index + BCM_XGS3_MPATH_EGRESS_IDX_MIN;
    }


    switch(action_if_bos) {
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_IIF:
            info->action_if_bos = BCM_MPLS_SWITCH_ACTION_POP;
            break;
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI:
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_ECMP:
            info->action_if_bos = BCM_MPLS_SWITCH_ACTION_SWAP;
            break;
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_NHI:
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_ECMP:
            info->action_if_bos = BCM_MPLS_SWITCH_ACTION_PHP;
            break;
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_INVALID:
            info->action_if_bos = BCM_MPLS_SWITCH_ACTION_INVALID;
            break;
        default:
            return BCM_E_INTERNAL;
            break;
    }

    switch(action_if_not_bos) {
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_POP:
            info->action_if_not_bos = BCM_MPLS_SWITCH_ACTION_POP;
            break;
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI:
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_ECMP:
            info->action_if_not_bos = BCM_MPLS_SWITCH_ACTION_SWAP;
            break;
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_NHI:
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_ECMP:
            info->action_if_not_bos = BCM_MPLS_SWITCH_ACTION_PHP;
            break;
        case _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_INVALID:
            info->action_if_not_bos = BCM_MPLS_SWITCH_ACTION_INVALID;
            break;
        default:
            return BCM_E_INTERNAL;
            break;
    }

    if (info->action_if_bos == info->action_if_not_bos) {
        info->action = info->action_if_bos;
        if (info->action == BCM_MPLS_SWITCH_ACTION_INVALID) {
            /* Both actions are invalid in the HW entry 
             * Error out.
             */
            return BCM_E_INTERNAL;
        }
    } else {
        info->action = BCM_MPLS_SWITCH_ACTION_INVALID;
    }

    if (soc_MPLS_ENTRYm_field32_get(unit, ment, PW_TERM_NUM_VALIDf)) {
        info->flags |= BCM_MPLS_SWITCH_COUNTED;
    }
#if defined(BCM_TRIDENT2PLUS_SUPPORT)
     else if (SOC_MEM_FIELD_VALID(unit, MPLS_ENTRYm, CLASS_IDf)) {
        info->class_id = soc_MPLS_ENTRYm_field32_get(unit, ment, CLASS_IDf);
    }
#endif

    if (!soc_MPLS_ENTRYm_field32_get(unit, ment, DECAP_USE_TTLf)) {
        info->flags |= BCM_MPLS_SWITCH_INNER_TTL;
    }
    if (soc_MPLS_ENTRYm_field32_get(unit, ment, DECAP_USE_EXP_FOR_INNERf)) {
        if ((info->action_if_bos == BCM_MPLS_SWITCH_ACTION_PHP) ||
            (info->action_if_bos == BCM_MPLS_SWITCH_ACTION_POP) ||
            (info->action_if_not_bos == BCM_MPLS_SWITCH_ACTION_PHP) ||
            (info->action_if_not_bos == BCM_MPLS_SWITCH_ACTION_POP)) {
            info->flags |= BCM_MPLS_SWITCH_INNER_EXP;
        }
    }
    if (soc_MPLS_ENTRYm_field32_get(unit, ment, 
                                    DECAP_USE_EXP_FOR_PRIf) == 0x1) {

        /* Use specified EXP-map to determine internal prio/color */
        info->flags |= BCM_MPLS_SWITCH_INT_PRI_MAP;
        info->exp_map = 
            soc_MPLS_ENTRYm_field32_get(unit, ment, EXP_MAPPING_PTRf);
        info->exp_map |= _BCM_TR_MPLS_EXP_MAP_TABLE_TYPE_INGRESS;
    } else if (soc_MPLS_ENTRYm_field32_get(unit, ment, 
                                           DECAP_USE_EXP_FOR_PRIf) == 0x2) {

        /* Use the specified internal priority value */
        info->flags |= BCM_MPLS_SWITCH_INT_PRI_SET;
        info->int_pri =
            soc_MPLS_ENTRYm_field32_get(unit, ment, NEW_PRIf);

        /* Use specified EXP-map to determine internal color */
        info->flags |= BCM_MPLS_SWITCH_COLOR_MAP;
        info->exp_map = 
            soc_MPLS_ENTRYm_field32_get(unit, ment, EXP_MAPPING_PTRf);
        info->exp_map |= _BCM_TR_MPLS_EXP_MAP_TABLE_TYPE_INGRESS;
    }
    if (SOC_MEM_FIELD_VALID(unit,MPLS_ENTRYm,
                            DO_NOT_CHANGE_PAYLOAD_DSCPf)) {
        if (soc_MPLS_ENTRYm_field32_get(unit, ment, 
                                        DO_NOT_CHANGE_PAYLOAD_DSCPf) == 0) {
            if ((info->action_if_bos == BCM_MPLS_SWITCH_ACTION_PHP) ||
                (info->action_if_bos == BCM_MPLS_SWITCH_ACTION_POP) ||
                (info->action_if_not_bos == BCM_MPLS_SWITCH_ACTION_PHP) ||
                (info->action_if_not_bos == BCM_MPLS_SWITCH_ACTION_POP)) {
                     info->flags |=
                         (BCM_MPLS_SWITCH_OUTER_EXP | BCM_MPLS_SWITCH_OUTER_TTL);
            }
        }
    }
#if defined(BCM_TOMAHAWK2_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_ecn)) {
        int ecn_map_id;
        int ecn_map_hw_idx;
        int rv;
        ecn_map_hw_idx = soc_MPLS_ENTRYm_field32_get(unit, ment, 
                                                     EXP_TO_IP_ECN_MAPPING_PTRf);
        rv = bcmi_ecn_map_hw_idx2id(unit, ecn_map_hw_idx, 
                                    _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN, 
                                    &ecn_map_id); 
        if (BCM_SUCCESS(rv)) {
            info->flags |= BCM_MPLS_SWITCH_INGRESS_ECN_MAP;
            info->ecn_map_id = ecn_map_id;
        }
    }
#endif
    return BCM_E_NONE;
}

/* Convert key part of application format to HW entry. */
STATIC int
_bcmi_xgs5_mpls_entry_set_key(int unit, bcm_mpls_tunnel_switch_t *info,
                           mpls_entry_entry_t *ment)
{
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t trunk_id;
    int rv, gport_id;

    sal_memset(ment, 0, sizeof(mpls_entry_entry_t));

    if (info->port == BCM_GPORT_INVALID) {
        /* Global label, mod/port not part of lookup key */
        soc_MPLS_ENTRYm_field32_set(unit, ment, MODULE_IDf, 0);
        soc_MPLS_ENTRYm_field32_set(unit, ment, PORT_NUMf, 0);
        if (BCM_XGS3_L3_MPLS_LBL_VALID(info->label)) {
            soc_MPLS_ENTRYm_field32_set(unit, ment, MPLS_LABELf, info->label);
        } else {
            return BCM_E_PARAM;
        }
        soc_MPLS_ENTRYm_field32_set(unit, ment, VALIDf, 1);
        return BCM_E_NONE;
    }

    rv = _bcm_esw_gport_resolve(unit, info->port, &mod_out, 
                                &port_out, &trunk_id, &gport_id);
    BCM_IF_ERROR_RETURN(rv);

    if (BCM_GPORT_IS_TRUNK(info->port)) {
        soc_MPLS_ENTRYm_field32_set(unit, ment, Tf, 1);
        soc_MPLS_ENTRYm_field32_set(unit, ment, TGIDf, trunk_id);
    } else {
        soc_MPLS_ENTRYm_field32_set(unit, ment, MODULE_IDf, mod_out);
        soc_MPLS_ENTRYm_field32_set(unit, ment, PORT_NUMf, port_out);
    }
    if (BCM_XGS3_L3_MPLS_LBL_VALID(info->label)) {
        soc_MPLS_ENTRYm_field32_set(unit, ment, MPLS_LABELf, info->label);
    } else {
        return BCM_E_PARAM;
    }
    soc_MPLS_ENTRYm_field32_set(unit, ment, VALIDf, 1);
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_add
 * Purpose:
 *      Add an MPLS label entry.
 * Parameters:
 *      unit - Device Number
 *      info - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_switch_add(int unit, bcm_mpls_tunnel_switch_t *info)
{
    mpls_entry_entry_t ment; 
    int mode=0, nh_index = -1, vrf=0, rv, num_pw_term, old_pw_cnt = -1, pw_cnt = -1;
    int index, old_nh_index = -1, old_ecmp_index = -1;
    int action, action_if_bos, action_if_not_bos;
    int old_action_if_bos = -1, old_action_if_not_bos = -1;
    int  tunnel_switch_update=0;
    uint32 mpath_flag=0;
    int  ref_count=0;
    bcm_if_t  egress_if=0;
    int nh_index_alloc_failed = 0;
    rv = bcm_xgs3_l3_egress_mode_get(unit, &mode);
    BCM_IF_ERROR_RETURN(rv);
    if (!mode) {
        LOG_INFO(BSL_LS_BCM_L3,
                 (BSL_META_U(unit,
                             "L3 egress mode must be set first\n")));
        return BCM_E_DISABLED;
    }

    /* Check for Port_independent Label mapping */
    if (!BCM_XGS3_L3_MPLS_LBL_VALID(info->label)) {
        return BCM_E_PARAM;
    }

    rv = bcm_tr_mpls_port_independent_range (unit, info->label, info->port);
    if (rv < 0) {
        return rv;
    }

#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_frr_lookup)) {
        if (info->flags & BCM_MPLS_SWITCH_FRR) {
            rv = _bcm_tr3_mpls_tunnel_switch_frr_set(unit, info);
            return rv;
         }
    }
#endif /* BCM_TOMAHAWK2_SUPPORT || BCM_APACHE_SUPPORT */
#if defined(BCM_TRIDENT2PLUS_SUPPORT)
    if(soc_feature(unit, soc_feature_fp_based_oam)) {
        /*Ensure that class_id doesnt overcross CLASS_IDf*/
        int bit_num = 0;
        if((info->flags & BCM_MPLS_SWITCH_COUNTED) && (info->class_id)) {
            return BCM_E_PARAM;
        }
        if(info->class_id) {
            bit_num = soc_mem_field_length(unit, MPLS_ENTRYm, CLASS_IDf);
            if(info->class_id > (( 1 << bit_num ) - 1)) {
                return BCM_E_PARAM;
            }
        }
    }
#endif

    action = info->action;
    if (info->action_if_bos != info->action_if_not_bos) {
        action = BCM_MPLS_SWITCH_ACTION_INVALID;
        action_if_bos = info->action_if_bos;
        action_if_not_bos = info->action_if_not_bos;
    } else {
        /* 
         * Same action specified in if_bos and if_not_bos.
         * If this action is of valid type, it will override
         * the action specified in info->action
         */
        if (info->action_if_bos != BCM_MPLS_SWITCH_ACTION_INVALID) {
            action = info->action_if_bos;
        }
        action_if_bos = action_if_not_bos = action;
    }

    if (!(BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos ||
          BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos ||
          BCM_MPLS_SWITCH_ACTION_POP == action_if_bos ||
          BCM_MPLS_SWITCH_ACTION_POP == action_if_not_bos ||
          BCM_MPLS_SWITCH_ACTION_PHP == action_if_bos ||
          BCM_MPLS_SWITCH_ACTION_PHP == action_if_not_bos)) {
        return BCM_E_PARAM;
    }
    if ((info->egress_label.flags & BCM_MPLS_EGRESS_LABEL_PRESERVE) &&
        (BCM_MPLS_SWITCH_ACTION_SWAP != action_if_bos ||
         BCM_MPLS_SWITCH_ACTION_SWAP != action_if_not_bos)) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_bcmi_xgs5_mpls_entry_set_key(unit, info, &ment));

    /* See if entry already exists */
    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                        &ment, &ment, 0);

    /* default not to use DSCP from ING_MPLS_EXP_MAPPING table */
    if (SOC_MEM_FIELD_VALID(unit,MPLS_ENTRYm,DO_NOT_CHANGE_PAYLOAD_DSCPf)) {
        soc_MPLS_ENTRYm_field32_set(unit, &ment,
                                          DO_NOT_CHANGE_PAYLOAD_DSCPf, 1);
    }

    if (rv == SOC_E_NONE) {
        /* Entry exists, save old info */
        tunnel_switch_update = 1;
        old_action_if_bos = soc_MPLS_ENTRYm_field32_get(unit, &ment, 
                                                        MPLS_ACTION_IF_BOSf);
        old_action_if_not_bos = soc_MPLS_ENTRYm_field32_get(unit, &ment, 
                                                    MPLS_ACTION_IF_NOT_BOSf);

        if ((old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI) ||
            (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI) ||
            (old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_NHI) ||
            (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_NHI)) {
            old_nh_index = soc_MPLS_ENTRYm_field32_get(unit, &ment, NEXT_HOP_INDEXf);
        }
        if ((old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_ECMP) ||
            (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_ECMP)) {
            old_ecmp_index = soc_MPLS_ENTRYm_field32_get(unit, &ment, ECMP_PTRf);
        }
        if (((old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_ECMP) ||
             (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_ECMP)) &&
             soc_feature(unit, soc_feature_mpls_lsr_ecmp)) {
            old_ecmp_index = soc_MPLS_ENTRYm_field32_get(unit, &ment, ECMP_PTRf);
        }
        if (soc_MPLS_ENTRYm_field32_get(unit, &ment, PW_TERM_NUM_VALIDf)) {
            old_pw_cnt = soc_MPLS_ENTRYm_field32_get(unit, &ment, PW_TERM_NUMf);
        } 
    } else if (rv != SOC_E_NOT_FOUND) {
        return rv;
    }


    if (BCM_MPLS_SWITCH_ACTION_POP == action_if_bos ||
        BCM_MPLS_SWITCH_ACTION_POP == action_if_not_bos) {

        /* uniform qos model if either of these two flags set.
         * Only apply to L3 MPLS and BOS
         */
        if (info->flags & (BCM_MPLS_SWITCH_OUTER_EXP |
                    BCM_MPLS_SWITCH_OUTER_TTL) ) {
            if (SOC_MEM_FIELD_VALID(unit,MPLS_ENTRYm,
                        DO_NOT_CHANGE_PAYLOAD_DSCPf)) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment,
                        DO_NOT_CHANGE_PAYLOAD_DSCPf, 0);
            }
        }

        mode = -1;
        if (_BCM_MPLS_VPN_IS_L3(info->vpn)) {
            _BCM_MPLS_VPN_GET(vrf, _BCM_MPLS_VPN_TYPE_L3, info->vpn);
            if (!_BCM_MPLS_VRF_USED_GET(unit, vrf)) {
                return BCM_E_PARAM;
            }

            /* Check L3 Ingress Interface mode. */ 
            mode = 0;
            rv = bcm_xgs3_l3_ingress_mode_get(unit, &mode);
            BCM_IF_ERROR_RETURN(rv);
        }
        if (!mode) {
            _bcm_l3_ingress_intf_t iif;
            sal_memset(&iif, 0, sizeof(_bcm_l3_ingress_intf_t));
            iif.intf_id =  _BCM_TR_MPLS_L3_IIF_BASE + vrf;
            rv = _bcm_tr_l3_ingress_interface_get(unit, NULL, &iif);
            BCM_IF_ERROR_RETURN(rv);

            iif.vrf = vrf;
#ifdef BCM_TOMAHAWK_SUPPORT
            if (soc_feature(unit, soc_feature_ecn_wred) && 
                (info->flags & BCM_MPLS_SWITCH_TUNNEL_TERM_ECN_MAP)) {
                int ecn_map_index;
                int ecn_map_type;
                int ecn_map_num;
                ecn_map_type = info->tunnel_term_ecn_map_id & 
                                _BCM_XGS5_ECN_MAP_TYPE_MASK;
                ecn_map_index = info->tunnel_term_ecn_map_id & 
                                _BCM_XGS5_ECN_MAP_NUM_MASK;
                ecn_map_num = 
                    soc_mem_index_count(unit, ING_TUNNEL_ECN_DECAPm) / 
                    _BCM_ECN_MAX_ENTRIES_PER_TUNNEL_TERM_MAP;
                if (ecn_map_type != _BCM_XGS5_ECN_MAP_TYPE_TUNNEL_TERM) {
                    return BCM_E_PARAM;
                }
                if (ecn_map_index >= ecn_map_num) {
                    return BCM_E_PARAM;
                }          
                if (!bcmi_xgs5_ecn_map_used_get(unit, ecn_map_index, 
                                                _bcmEcnmapTypeTunnelTerm)) {
                    return BCM_E_PARAM;
                }
                iif.tunnel_term_ecn_map_id = ecn_map_index;
                iif.flags |= BCM_L3_INGRESS_TUNNEL_TERM_ECN_MAP;
            } 
#endif            
            rv = _bcm_tr_l3_ingress_interface_set(unit, &iif, NULL, NULL);
            BCM_IF_ERROR_RETURN(rv);

            soc_MPLS_ENTRYm_field32_set(unit, &ment, L3_IIFf, 
                    _BCM_TR_MPLS_L3_IIF_BASE + vrf);
        } else {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, L3_IIFf, info->ingress_if);
        }

        if (BCM_MPLS_SWITCH_ACTION_POP == action_if_not_bos) {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_NOT_BOSf,
                    _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_POP);
        }
        if (BCM_MPLS_SWITCH_ACTION_POP == action_if_bos) {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_BOSf,
                    _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_IIF);
        }
    }/*BCM_MPLS_SWITCH_ACTION_POP*/ 

    if (BCM_MPLS_SWITCH_ACTION_PHP == action_if_bos ||
        BCM_MPLS_SWITCH_ACTION_PHP == action_if_not_bos) {

        if (BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, info->egress_if)) {
            if (BCM_MPLS_SWITCH_ACTION_PHP == action_if_bos) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_BOSf,
                        _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_ECMP);
            }
            if (BCM_MPLS_SWITCH_ACTION_PHP == action_if_not_bos) {
                if (soc_feature(unit, soc_feature_mpls_lsr_ecmp)) {
                    soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_NOT_BOSf,
                            _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_ECMP);
                }
            }
        } else if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, info->egress_if)) {
            if (BCM_MPLS_SWITCH_ACTION_PHP == action_if_bos) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_BOSf,
                        _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_NHI);
            }
            if (BCM_MPLS_SWITCH_ACTION_PHP == action_if_not_bos) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_NOT_BOSf,
                        _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_NHI);
            }
        } else {
            return BCM_E_PARAM;
        }

        /* uniform qos model if either of these two flags set.
         * Only apply to L3 MPLS and BOS
         */
        if (info->flags & (BCM_MPLS_SWITCH_OUTER_EXP |
                    BCM_MPLS_SWITCH_OUTER_TTL) ) {
            if (SOC_MEM_FIELD_VALID(unit,MPLS_ENTRYm,
                        DO_NOT_CHANGE_PAYLOAD_DSCPf)) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment,
                        DO_NOT_CHANGE_PAYLOAD_DSCPf, 0);
            }
        }

        /*
         * Get egress next-hop index from egress object and
         * increment egress object reference count.
         */

        BCM_IF_ERROR_RETURN(bcm_xgs3_get_nh_from_egress_object(unit,
                    info->egress_if, &mpath_flag, 1, &nh_index));

        /* Fix: Entry_Type = 1, for PHP Packets with more than 1 Label */
        /* Read the egress next_hop entry pointed by Egress-Object */   
        rv = bcm_tr_mpls_egress_entry_modify(unit, nh_index, mpath_flag, 1);
        if (rv < 0 ) {
            return rv;
        }

        if (mpath_flag == BCM_L3_MULTIPATH) {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, ECMP_PTRf, nh_index);

            /* Swap to ECMP Type  PHP_ECMP or L3_ECMP flag is set up earlier */
        } else {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, NEXT_HOP_INDEXf, nh_index);
        }

    } /*BCM_MPLS_SWITCH_ACTION_PHP*/ 

    if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos ||
        BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos) {

        if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, info->egress_if)) {
            if ((!soc_feature(unit, soc_feature_mpls_lsr_ecmp)) || 
                 (!BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, info->egress_if))) {
                return BCM_E_PARAM;
            }
        }
        if (BCM_XGS3_L3_MPLS_LBL_VALID(info->egress_label.label) ||
            _BCM_MPLS_EGRESS_LABEL_PRESERVE(unit, info->egress_label.flags)) {
            rv = bcm_tr_mpls_l3_nh_info_add(unit, info, &nh_index);
            if (rv < 0) {
                nh_index_alloc_failed = 1;
                goto cleanup;
            }
            soc_MPLS_ENTRYm_field32_set(unit, &ment, NEXT_HOP_INDEXf, nh_index);
            if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_BOSf,
                        _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI);
            }
            if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_NOT_BOSf,
                        _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI);
            }
        } else {
            /*
             * Get egress next-hop index from egress object and
             * increment egress object reference count.
             */
            BCM_IF_ERROR_RETURN(bcm_xgs3_get_nh_from_egress_object(unit, info->egress_if,
                        &mpath_flag, 1, &nh_index));

            if (soc_feature(unit, soc_feature_mpls_lsr_ecmp) && (mpath_flag == BCM_L3_MULTIPATH)) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, ECMP_PTRf, nh_index);
                /* Swap to ECMP Type */
                if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos) {
                    soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__MPLS_ACTION_IF_NOT_BOSf,
                            _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_ECMP);
                }
                if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos) {
                    soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__MPLS_ACTION_IF_BOSf,
                            _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_ECMP);
                }
            } else {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, NEXT_HOP_INDEXf, nh_index);
                if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos) {
                    soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_NOT_BOSf,
                            _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI);
                }
                if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos) {
                    soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_BOSf,
                            _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI);
                }
            }
        }
    }/* BCM_MPLS_SWITCH_ACTION_SWAP */

    if (BCM_MPLS_SWITCH_ACTION_INVALID == action_if_bos) {
        soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_BOSf,
                _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_INVALID);
    } else if (BCM_MPLS_SWITCH_ACTION_INVALID == action_if_not_bos) {
        soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS_ACTION_IF_NOT_BOSf,
                _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_INVALID);
    }

    soc_MPLS_ENTRYm_field32_set(unit, &ment, V4_ENABLEf, 1);
    soc_MPLS_ENTRYm_field32_set(unit, &ment, V6_ENABLEf, 1);

    if (info->flags & BCM_MPLS_SWITCH_INNER_TTL) {
        if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos &&
            BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos) {
            rv = BCM_E_PARAM;
            goto cleanup;
        }
        soc_MPLS_ENTRYm_field32_set(unit, &ment, DECAP_USE_TTLf, 0);
    } else {
        soc_MPLS_ENTRYm_field32_set(unit, &ment, DECAP_USE_TTLf, 1);
    }

    if (info->flags & BCM_MPLS_SWITCH_INNER_EXP) {
        if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos &&
            BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos) {
            rv = BCM_E_PARAM;
            goto cleanup;
        }
        soc_MPLS_ENTRYm_field32_set(unit, &ment, DECAP_USE_EXP_FOR_INNERf, 0);
    } else {
        /* For SWAP, Do-not PUSH EXP */
        if (BCM_MPLS_SWITCH_ACTION_SWAP == action_if_bos &&
            BCM_MPLS_SWITCH_ACTION_SWAP == action_if_not_bos) {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, DECAP_USE_EXP_FOR_INNERf, 0);
        } else {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, DECAP_USE_EXP_FOR_INNERf, 1);
        }
    }

#if defined(BCM_TRIDENT2PLUS_SUPPORT)
    if (soc_feature(unit, soc_feature_td2p_mpls_linear_protection)){
        int local_failover_id = 0;
        int prot_type         = 0;
        if (info->flags & BCM_MPLS_SWITCH_DROP) {
            soc_MPLS_ENTRYm_field32_set(unit, &ment,
                    MPLS__DROP_DATA_ENABLEf, 1);
        } else {
            soc_MPLS_ENTRYm_field32_set(unit, &ment,
                    MPLS__DROP_DATA_ENABLEf, 0);
        }

        if (info->failover_id != 0) {
            local_failover_id = info->failover_id;
            _BCM_GET_FAILOVER_TYPE(local_failover_id, prot_type);
            _BCM_GET_FAILOVER_ID(local_failover_id);
            if ((prot_type & _BCM_FAILOVER_INGRESS) &&
                (BCM_SUCCESS(_bcm_td2p_failover_ingress_id_validate(
                unit, local_failover_id)))) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment,
                                            MPLS__RX_PROT_GROUPf,
                                            local_failover_id);
            }
        } else {
            soc_MPLS_ENTRYm_field32_set(unit, &ment,
                                        MPLS__RX_PROT_GROUPf, 0);
        }
    }
#endif

#if defined(BCM_TOMAHAWK2_SUPPORT)
    if (SOC_IS_TOMAHAWK2(unit)) {
        if (info->flags & BCM_MPLS_SWITCH_DROP) {
            soc_MPLS_ENTRYm_field32_set(unit, &ment,
                    MPLS__DROP_DATA_ENABLEf, 1);
        } else {
            soc_MPLS_ENTRYm_field32_set(unit, &ment,
                    MPLS__DROP_DATA_ENABLEf, 0);
        }
    }
#endif

    (void) bcm_tr_mpls_entry_internal_qos_set(unit, NULL, info, &ment);

    if ((info->flags & BCM_MPLS_SWITCH_COUNTED)) {
        if (SOC_MEM_IS_VALID(unit, ING_PW_TERM_COUNTERSm)) {
            if (old_pw_cnt == -1) {
                num_pw_term = soc_mem_index_count(unit, ING_PW_TERM_COUNTERSm);
                for (pw_cnt = 0; pw_cnt < num_pw_term; pw_cnt++) {
                    if (!_BCM_MPLS_PW_TERM_USED_GET(unit, pw_cnt)) {
                        break;
                    }
                }
                if (pw_cnt == num_pw_term) {
                    rv = BCM_E_RESOURCE;
                    goto cleanup;
                }
                _BCM_MPLS_PW_TERM_USED_SET(unit, pw_cnt);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, PW_TERM_NUMf, pw_cnt);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, PW_TERM_NUM_VALIDf, 1);
            }
        } else if (SOC_MEM_IS_VALID(unit, ING_PW_TERM_SEQ_NUMm)) {
            if (old_pw_cnt == -1) {
                num_pw_term = soc_mem_index_count(unit, ING_PW_TERM_SEQ_NUMm);
                for (pw_cnt = 0; pw_cnt < num_pw_term; pw_cnt++) {
                    if (!_BCM_MPLS_PW_TERM_USED_GET(unit, pw_cnt)) {
                        break;
                    }
                }
                if (pw_cnt == num_pw_term) {
                    rv = BCM_E_RESOURCE;
                    goto cleanup;
                }
                _BCM_MPLS_PW_TERM_USED_SET(unit, pw_cnt);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, PW_TERM_NUMf, pw_cnt);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, PW_TERM_NUM_VALIDf, 1);
            }
        }        
    }

#if defined(BCM_TRIDENT2PLUS_SUPPORT)
    else if (soc_feature(unit, soc_feature_fp_based_oam) &&
                SOC_MEM_FIELD_VALID(unit, MPLS_ENTRYm, CLASS_IDf)) {
        soc_MPLS_ENTRYm_field32_set(unit, &ment, CLASS_IDf, info->class_id);
    }
#endif

#if defined(BCM_TOMAHAWK2_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_ecn)) {
        if (info->flags & BCM_MPLS_SWITCH_INGRESS_ECN_MAP) { 
            int ecn_map_index;
            int ecn_map_type;
            int ecn_map_num;
            int ecn_map_hw_idx;
            if (BCM_MPLS_SWITCH_ACTION_PHP == action_if_bos &&
                BCM_MPLS_SWITCH_ACTION_PHP == action_if_not_bos) {
                rv = BCM_E_PARAM;
                goto cleanup;
            }
            ecn_map_type = info->ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK;
            ecn_map_index = info->ecn_map_id & _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
            ecn_map_num = 
                soc_mem_index_count(unit, ING_EXP_TO_IP_ECN_MAPPINGm) / 
                _BCM_ECN_MAX_ENTRIES_PER_MAP;
            if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_EXP2ECN) {
                rv = BCM_E_PARAM;
                goto cleanup;
            }
            if (ecn_map_index >= ecn_map_num) {
                rv = BCM_E_PARAM;
                goto cleanup;
            }
            if (!bcmi_xgs5_ecn_map_used_get(unit, ecn_map_index, 
                                            _bcmEcnmapTypeExp2Ecn)) {
                rv = BCM_E_PARAM;
                goto cleanup; 
            }
            if (bcmi_ecn_map_id2hw_idx(unit, info->ecn_map_id, &ecn_map_hw_idx)) {
                rv = BCM_E_PARAM;
                goto cleanup; 
            }
            soc_MPLS_ENTRYm_field32_set(unit, &ment, EXP_TO_IP_ECN_MAPPING_PTRf, 
                                        ecn_map_hw_idx);
        }
    }
#endif

    if (!tunnel_switch_update) {
        rv = soc_mem_insert(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, &ment);
    } else {
        rv = soc_mem_write(unit, MPLS_ENTRYm,
                           MEM_BLOCK_ALL, index,
                           &ment);
    }

    if (rv < 0) {
        goto cleanup;
    }

    if (tunnel_switch_update) {
        /* Clean up old next-hop and counter info if entry was replaced */
        if ((old_pw_cnt != -1) && !(info->flags & BCM_MPLS_SWITCH_COUNTED)) {
            _BCM_MPLS_PW_TERM_USED_CLR(unit, old_pw_cnt);
        }
        if ((old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI) ||
            (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI)) {
            /* Check if tunnel_switch.egress_label mode is being used */
            rv = bcm_tr_mpls_get_vp_nh (unit, (bcm_if_t) old_nh_index, &egress_if);
            if (rv == BCM_E_NONE) {
                rv = bcm_tr_mpls_l3_nh_info_delete(unit, old_nh_index);
            } else {
                /* Decrement next-hop Reference count */
                rv = bcm_xgs3_get_ref_count_from_nhi(unit, 0, &ref_count, old_nh_index);
            }
        }
        if ((old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_NHI) ||
            (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_NHI)) {
            rv = bcm_xgs3_nh_del(unit, 0, old_nh_index);
        } 
        if ((old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_ECMP) ||
            (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_ECMP)) {
            rv = bcm_xgs3_ecmp_group_del(unit, old_ecmp_index);
        } 
        if (soc_feature(unit, soc_feature_mpls_lsr_ecmp) &&
            ((old_action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_ECMP) ||
             (old_action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_ECMP))) {
            rv = bcm_xgs3_ecmp_group_del(unit, old_ecmp_index);
        }
    }

    if (rv < 0) {
        goto cleanup;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return rv;

  cleanup:
    if (nh_index_alloc_failed) {
        return rv;
    }
    if (pw_cnt != -1) {
        _BCM_MPLS_PW_TERM_USED_CLR(unit, pw_cnt);
    }
    /* coverity[check_after_sink : FALSE] */
    if (nh_index != -1) {
        if ((action_if_bos == BCM_MPLS_SWITCH_ACTION_SWAP) ||
            (action_if_not_bos == BCM_MPLS_SWITCH_ACTION_SWAP)) {
            if (BCM_XGS3_L3_MPLS_LBL_VALID(info->egress_label.label) ||
                (info->action == BCM_MPLS_SWITCH_ACTION_PHP)) {
                (void) bcm_tr_mpls_l3_nh_info_delete(unit, nh_index);
            }
        } else if ((action_if_bos == BCM_MPLS_SWITCH_ACTION_PHP) ||
                   (action_if_not_bos == BCM_MPLS_SWITCH_ACTION_PHP)) {
            (void) bcm_xgs3_nh_del(unit, 0, nh_index);
        }
    }
    return rv;
}

STATIC int
_bcmi_xgs5_mpls_entry_delete(int unit, mpls_entry_entry_t *ment)
{   
    ing_pw_term_counters_entry_t pw_cnt_entry;
    int rv, ecmp_index = -1, nh_index = -1, pw_cnt = -1;
    int action_if_bos, action_if_not_bos;
    bcm_if_t  egress_if=0;
    int  ref_count=0;

    if (soc_MPLS_ENTRYm_field32_get(unit, ment, PW_TERM_NUM_VALIDf)) {
        pw_cnt = soc_MPLS_ENTRYm_field32_get(unit, ment, PW_TERM_NUMf);
    }

    action_if_bos = soc_MPLS_ENTRYm_field32_get(unit, ment, MPLS_ACTION_IF_BOSf);
    action_if_not_bos = soc_MPLS_ENTRYm_field32_get(unit, ment, 
                                                      MPLS_ACTION_IF_NOT_BOSf);

    if ((action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI) ||
            (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI) ||
            (action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_NHI) ||
            (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_NHI)) {
        nh_index = soc_MPLS_ENTRYm_field32_get(unit, ment, NEXT_HOP_INDEXf);
    }
    if ((action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_ECMP) ||
            (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_ECMP)) {
        ecmp_index = soc_MPLS_ENTRYm_field32_get(unit, ment, ECMP_PTRf);
    }
    if (((action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_ECMP) ||
                (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_ECMP)) &&
            soc_feature(unit, soc_feature_mpls_lsr_ecmp)) {
        ecmp_index = soc_MPLS_ENTRYm_field32_get(unit, ment, ECMP_PTRf);
    }

    /* Delete the entry from HW */
    rv = soc_mem_delete(unit, MPLS_ENTRYm, MEM_BLOCK_ALL, ment);
    if ( (rv != BCM_E_NOT_FOUND) && (rv != BCM_E_NONE) ) {
         return rv;
    }

    if (pw_cnt != -1) {
        sal_memset(&pw_cnt_entry, 0, sizeof(ing_pw_term_counters_entry_t));
        (void) WRITE_ING_PW_TERM_COUNTERSm(unit, MEM_BLOCK_ALL, pw_cnt,
                                           &pw_cnt_entry);
        _BCM_MPLS_PW_TERM_USED_CLR(unit, pw_cnt);
    }

    if ((action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_NHI) ||
        (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_NHI)) {
        /* Check if tunnel_switch.egress_label mode is being used */
        rv = bcm_tr_mpls_get_vp_nh (unit, (bcm_if_t) nh_index, &egress_if);
        if (rv == BCM_E_NONE) {
            rv = bcm_tr_mpls_l3_nh_info_delete(unit, nh_index);
        } else {
            /* Decrement next-hop Reference count */
            rv = bcm_xgs3_get_ref_count_from_nhi(unit, 0, &ref_count, nh_index);
        }
    }
    if ((action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_NHI) ||
        (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_NHI)) {
        rv = bcm_xgs3_nh_del(unit, 0, nh_index);
    } 
    if ((action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L3_ECMP) ||
            (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_PHP_ECMP)) {
        rv = bcm_xgs3_ecmp_group_del(unit, ecmp_index);
    } 
    if (soc_feature(unit, soc_feature_mpls_lsr_ecmp) &&
            ((action_if_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_SWAP_ECMP) ||
             (action_if_not_bos == _BCM_MPLS_XGS5_MPLS_ACTION_IF_NOT_BOS_SWAP_ECMP))) {
        rv = bcm_xgs3_ecmp_group_del(unit, ecmp_index);
    }

    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_delete
 * Purpose:
 *      Delete an MPLS label entry.
 * Parameters:
 *      unit - Device Number
 *      info - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_switch_delete(int unit, bcm_mpls_tunnel_switch_t *info)
{
    int rv, index;
    mpls_entry_entry_t ment;
#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_frr_lookup) && 
        (info->flags & BCM_MPLS_SWITCH_FRR)) { 
        rv = _bcm_tr3_mpls_tunnel_switch_frr_delete(unit, info);
    } else
#endif /* BCM_TOMAHAWK2_SUPPORT || BCM_APACHE_SUPPORT */
    {
        rv = _bcmi_xgs5_mpls_entry_set_key(unit, info, &ment);
        BCM_IF_ERROR_RETURN(rv);

        rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                &ment, &ment, 0);
        if (rv < 0) {
            return rv;
        }
        rv = _bcmi_xgs5_mpls_entry_delete(unit, &ment);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_delete_all
 * Purpose:
 *      Delete all MPLS label entries.
 * Parameters:
 *      unit   - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_switch_delete_all(int unit)
{
    int rv, i, num_entries;
    mpls_entry_entry_t ment;
#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    soc_tunnel_term_t tnl_entry;
#endif


    

    num_entries = soc_mem_index_count(unit, MPLS_ENTRYm);
    for (i = 0; i < num_entries; i++) {
        rv = READ_MPLS_ENTRYm(unit, MEM_BLOCK_ANY, i, &ment);
        if (rv < 0) {
            return rv;
        }
        if (!soc_MPLS_ENTRYm_field32_get(unit, &ment, VALIDf)) {
            continue;
        }
        if (soc_MPLS_ENTRYm_field32_get(unit, &ment, MPLS_ACTION_IF_BOSf) ==
                _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L2_SVP) {
            /* L2_SVP */
            continue;
        }
        rv = _bcmi_xgs5_mpls_entry_delete(unit, &ment);
        if (rv < 0) {
            return rv;
        }
    }
#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_frr_lookup)) {
        num_entries = soc_mem_index_count(unit, L3_TUNNELm);
        for (i = 0; i < num_entries; i++) {
            sal_memset(&tnl_entry, 0, sizeof(tnl_entry));
            rv = READ_L3_TUNNELm(unit, MEM_BLOCK_ANY, i,
                (uint32 *)&tnl_entry.entry_arr[0]);
            if (rv < 0) {
                return rv;
            }
            if (!soc_L3_TUNNELm_field32_get(unit, &tnl_entry, VALIDf)) {
                continue;
            }
            if (0x2 != soc_L3_TUNNELm_field32_get(unit, &tnl_entry, MODEf)) {
                continue;
            }

            rv = soc_tunnel_term_delete(unit, &tnl_entry);
            if (rv < 0) {
                return rv;
            }

            /*_soc_tunnel_term_slot_free() api moves last entry to
             *deleted entry position using _soc_tunnel_term_entry_shift().
             *Therefore adding this index change to support delete all*/
            i--;
        }
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_get
 * Purpose:
 *      Get an MPLS label entry.
 * Parameters:
 *      unit - Device Number
 *      info - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_switch_get(int unit, bcm_mpls_tunnel_switch_t *info)
{
    int rv, index;
    mpls_entry_entry_t ment;
#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_frr_lookup) && 
        (info->flags & BCM_MPLS_SWITCH_FRR)) { 
        rv = _bcm_tr3_mpls_tunnel_switch_frr_get(unit, info, &index);
    } else
#endif /* BCM_TOMAHAWK2_SUPPORT || BCM_APACHE_SUPPORT */
    {

        rv = _bcmi_xgs5_mpls_entry_set_key(unit, info, &ment);

        BCM_IF_ERROR_RETURN(rv);

        rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &index,
                &ment, &ment, 0);

        if (rv < 0) {
            return rv;
        }
        rv = _bcmi_xgs5_mpls_entry_get_data(unit, &ment, info);
    }
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_traverse
 * Purpose:
 *      Traverse all valid MPLS label entries an call the
 *      supplied callback routine.
 * Parameters:
 *      unit      - Device Number
 *      cb        - User callback function, called once per MPLS entry.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_switch_traverse(int unit, 
                                   bcm_mpls_tunnel_switch_traverse_cb cb,
                                   void *user_data)
{
    int rv, i;
    mpls_entry_entry_t *ment = NULL;
    bcm_mpls_tunnel_switch_t info;
#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    soc_tunnel_term_t *tnl_entry = NULL;
    uint32 key_type=0;
    uint8 *tnl_entry_buf = NULL;
#endif
   int index_min, index_max;
    uint8 *mpls_entry_buf = NULL;

    mpls_entry_buf = soc_cm_salloc(unit,
            SOC_MEM_TABLE_BYTES(unit, MPLS_ENTRYm), "MPLS_ENTRY buffer");
    if (NULL == mpls_entry_buf) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }

    index_min = soc_mem_index_min(unit, MPLS_ENTRYm);
    index_max = soc_mem_index_max(unit, MPLS_ENTRYm);
    rv = soc_mem_read_range(unit, MPLS_ENTRYm, MEM_BLOCK_ANY,
            index_min, index_max, mpls_entry_buf);
    if (SOC_FAILURE(rv)) {
        goto cleanup;
    }

    for (i = index_min; i <= index_max; i++) {
        ment = soc_mem_table_idx_to_pointer
            (unit, MPLS_ENTRYm, mpls_entry_entry_t *, mpls_entry_buf, i);

        /* Check for valid entry */
        if (!soc_MPLS_ENTRYm_field32_get(unit, ment, VALIDf)) {
            continue;
        }
        /* Check MPLS Key_type */
        if (soc_feature(unit, soc_feature_mpls_enhanced)) {
            if (0x0 != soc_MPLS_ENTRYm_field32_get(unit, ment, KEY_TYPEf)) {
                continue;
            }
        }
        if (soc_MPLS_ENTRYm_field32_get(unit, ment, MPLS_ACTION_IF_BOSf) ==
                _BCM_MPLS_XGS5_MPLS_ACTION_IF_BOS_L2_SVP) {
            continue;
        }
        sal_memset(&info, 0, sizeof(bcm_mpls_tunnel_switch_t));
        rv = _bcmi_xgs5_mpls_entry_get_key(unit, ment, &info);
        if (rv < 0) {
            goto cleanup;
        }
        rv = _bcmi_xgs5_mpls_entry_get_data(unit, ment, &info);
        if (rv < 0) {
            goto cleanup;
        }
        rv = cb(unit, &info, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
        if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
            goto cleanup;
        }
#endif
    }

#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_frr_lookup)) {

        /* traverse L3_TUNNEL entries */
        tnl_entry_buf = soc_cm_salloc(unit,
                SOC_MEM_TABLE_BYTES(unit, L3_TUNNELm), "L3_TUNNEL buffer");
        if (NULL == tnl_entry_buf) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        index_min = soc_mem_index_min(unit, L3_TUNNELm);
        index_max = soc_mem_index_max(unit, L3_TUNNELm);
        rv = soc_mem_read_range(unit, L3_TUNNELm, MEM_BLOCK_ANY,
                index_min, index_max, tnl_entry_buf);
        if (SOC_FAILURE(rv)) {
            goto cleanup;
        }

        for (i = index_min; i <= index_max; i++) {
            tnl_entry = soc_mem_table_idx_to_pointer
                (unit, L3_TUNNELm, soc_tunnel_term_t *, tnl_entry_buf, i);

            if (!soc_L3_TUNNELm_field32_get (unit, tnl_entry, VALIDf)) {
                continue;
            }

            /* Check for MPLS entry key  */
            key_type = soc_L3_TUNNELm_field32_get(unit, tnl_entry, MODEf);
            if (key_type != 0x2) {
                continue;
            }

            sal_memset(&info, 0, sizeof(bcm_mpls_tunnel_switch_t));
            rv = _bcm_tr3_mpls_tunnel_switch_frr_entry_key_get (unit, tnl_entry, &info);
            if (rv < 0) {
                goto cleanup;
            }

            (void) _bcm_tr3_mpls_tunnel_switch_frr_parse(unit, tnl_entry, &info);

            rv = cb(unit, &info, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
            if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                goto cleanup;
            }
#endif
        }
    }
#endif /* BCM_TOMAHAWK2_SUPPORT || BCM_APACHE_SUPPORT */

cleanup:
    if (mpls_entry_buf) {
        soc_cm_sfree(unit, mpls_entry_buf);
    }
#if defined(BCM_TOMAHAWK2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    if (tnl_entry_buf) {
        soc_cm_sfree(unit, tnl_entry_buf);
    }
#endif /* BCM_TOMAHAWK2_SUPPORT || BCM_APACHE_SUPPORT */
    return rv;
}

/***************************************************
 ***************************************************
 ******                                       ******
 ******  Segment Routing Feature Development. ******
 ******                                       ******
 ***************************************************
 ***************************************************/

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_free_indexes_init
 * Purpose:
 *      Initilize the free indexes.
 * Parameters:
 *      unit - Device Number
 *      fi   - Pointer to free index structure.
 * Returns:
 *      None.
 */
void
bcmi_egr_ip_tnl_mpls_free_indexes_init(int unit,
                bcmi_egr_ip_tnl_free_indexes_t *fi)
{

    int idx = 0,free_idx = 0;

    /* MAX_FREE_ENTRY = */
    for (;idx < MAX_ENTRY_INDEXES; idx++) {
        /* MAX FREE ENTRY COUNTING = 8 */
            free_idx=0;
        fi->free_index_count[idx]=0;
        for (; free_idx<MAX_FREE_ENTRY_INDEXES; free_idx++) {
            fi->free_entry_indexes[idx][free_idx] = -1;
        }

    }
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_free_indexes_clear
 * Purpose:
 *      clear free indexes structure in tunnel.
 *      When cleaning the full tunnel entry,
 *      we also need to free any free entry
 *      associated with this tunnel.
 * Parameters:
 *      unit   - Device Number
 *      tnl_id - (IN)Tunnel index.
 *      fi     - (IN)Pointer to free index structure.
 * Returns:
 *      None.
 */

void
bcmi_egr_ip_tnl_mpls_free_indexes_clear(int unit, int tnl_id,
                bcmi_egr_ip_tnl_free_indexes_t *fi)
{
    int idx = 0,free_idx = 0;
    int idx_min = tnl_id * _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
    int idx_max = idx_min + _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

    for (;idx < MAX_ENTRY_INDEXES; idx++) {
        /* MAX FREE ENTRY COUNTING = 8 */
        free_idx=0;
        if (fi->free_index_count[idx]) {
            for (; free_idx<MAX_FREE_ENTRY_INDEXES; free_idx++) {

                if (fi->free_entry_indexes[idx][free_idx] == -1) {
                    continue;
                }

                if ((fi->free_entry_indexes[idx][free_idx] >= idx_min) &&
                    (fi->free_entry_indexes[idx][free_idx] <= idx_max)) {

                    fi->free_entry_indexes[idx][free_idx] =
                         fi->free_entry_indexes[idx][fi->free_index_count[idx]];
                    fi->free_entry_indexes[idx][fi->free_index_count[idx]] = -1;
                    fi->free_index_count[idx]--;
                }
            }
        }
    }
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_free_indexes_dump
 * Purpose:
 *      Dump free indexes.
 * Parameters:
 *      unit - Device Number
 *      fi   - Pointer to free index structure.
 * Returns:
 *      None.
 */
void
bcmi_egr_ip_tnl_mpls_free_indexes_dump(int unit,
                bcmi_egr_ip_tnl_free_indexes_t *fi)
{

    int idx = 0,free_idx = 0;

    /* MAX_FREE_ENTRY = */
    for (;idx < MAX_ENTRY_INDEXES; idx++) {
        /* MAX FREE ENTRY COUNTING = 8 */
        free_idx=0;
        LOG_ERROR(BSL_LS_BCM_L3,
            (BSL_META_U(unit, "%s:%d: tnl idx = %d, count = %d \n"),
            __FUNCTION__, __LINE__, idx, fi->free_index_count[idx]));

        for (; free_idx<MAX_FREE_ENTRY_INDEXES; free_idx++) {
            LOG_ERROR(BSL_LS_BCM_L3,
                (BSL_META_U(unit, "start idx= %d at index = %d  \n"),
                fi->free_entry_indexes[idx][free_idx], free_idx));
        }
    }
}

/*
 * Function:
 *      bcmi_egr_ip_tunnel_mpls_sw_cleanup
 * Purpose:
 *      cleanup and deallocate sw state for mpls tunnel.
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_NONE.
 */
int
bcmi_egr_ip_tunnel_mpls_sw_cleanup(int unit)
{
    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry;
    int num_ip_tnl_mpls;
    int i,j;

    num_ip_tnl_mpls = soc_mem_index_count(unit, EGR_IP_TUNNEL_MPLSm);

    tnl_sw_entry = egr_mpls_tnl_sw_state[unit];

    if (tnl_sw_entry == NULL) {
        return BCM_E_NONE;
    }

    for (i=0; i<num_ip_tnl_mpls; i++) {

        if (tnl_sw_entry[i] == NULL) {
            continue;
        }

        if (tnl_sw_entry[i]->label_entry == NULL) {
            continue;
        }

        for (j=0; j<_BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit); j++) {

            if (tnl_sw_entry[i]->label_entry[j] == NULL) {
                continue;
            }

            sal_free(tnl_sw_entry[i]->label_entry[j]);
            tnl_sw_entry[i]->label_entry[j] = NULL;
        }

        sal_free(tnl_sw_entry[i]->label_entry);
        tnl_sw_entry[i]->label_entry = NULL;

        sal_free(tnl_sw_entry[i]);
        tnl_sw_entry[i] = NULL;
    }

    sal_free(tnl_sw_entry);
    tnl_sw_entry = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_egr_ip_tunnel_mpls_sw_init
 * Purpose:
 *      initialize s/w state for egress mpls tunnel.
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCME_XXX.
 */
int
bcmi_egr_ip_tunnel_mpls_sw_init(int unit)
{
    int num_ip_tnl_mpls;
    int i,j;
    int ent_per_tnl = _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

    num_ip_tnl_mpls = soc_mem_index_count(unit, EGR_IP_TUNNEL_MPLSm);

    egr_mpls_tnl_sw_state[unit] = (bcmi_egr_ip_tnl_mpls_tunnel_entry_t **)
        sal_alloc((num_ip_tnl_mpls *
            sizeof(bcmi_egr_ip_tnl_mpls_tunnel_entry_t *)),
        "egress mpls tunnel sw state");

    if (egr_mpls_tnl_sw_state[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(egr_mpls_tnl_sw_state[unit], 0,
        (num_ip_tnl_mpls * sizeof(bcmi_egr_ip_tnl_mpls_tunnel_entry_t *)));

    for (i=0; i<num_ip_tnl_mpls; i++) {
        egr_mpls_tnl_sw_state[unit][i] = (bcmi_egr_ip_tnl_mpls_tunnel_entry_t *)
            sal_alloc((sizeof(bcmi_egr_ip_tnl_mpls_tunnel_entry_t)),
            "egress mpls tunnel entry");

        sal_memset(egr_mpls_tnl_sw_state[unit][i], 0,
            sizeof(bcmi_egr_ip_tnl_mpls_tunnel_entry_t));

        if (egr_mpls_tnl_sw_state[unit][i] == NULL) {
            bcmi_egr_ip_tunnel_mpls_sw_cleanup(unit);
            return BCM_E_MEMORY;
        }

        egr_mpls_tnl_sw_state[unit][i]->label_entry =
            (bcmi_egr_ip_tnl_mpls_label_entry_t **)
            sal_alloc((ent_per_tnl *
                sizeof(bcmi_egr_ip_tnl_mpls_label_entry_t *)),
            "egress mpls tunnel label bucket");

        if (egr_mpls_tnl_sw_state[unit][i]->label_entry == NULL) {
            bcmi_egr_ip_tunnel_mpls_sw_cleanup(unit);
            return BCM_E_MEMORY;
        }

        for (j=0; j<_BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit); j++) {
            egr_mpls_tnl_sw_state[unit][i]->label_entry[j] =
                (bcmi_egr_ip_tnl_mpls_label_entry_t *)
                sal_alloc((sizeof(bcmi_egr_ip_tnl_mpls_label_entry_t)),
                "egress mpls tunnel label entry");

            if (egr_mpls_tnl_sw_state[unit][i]->label_entry[j] == NULL) {
                bcmi_egr_ip_tunnel_mpls_sw_cleanup(unit);
                return BCM_E_MEMORY;
            }

            egr_mpls_tnl_sw_state[unit][i]->label_entry[j]->intf_list = NULL;
            egr_mpls_tnl_sw_state[unit][i]->label_entry[j]->flags = 0;
            egr_mpls_tnl_sw_state[unit][i]->label_entry[j]->num_elements = 0;
        }
    }
    bcmi_egr_ip_tnl_mpls_free_indexes_init(unit, &(fi_db[unit]));
    return 0;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_tunnel_ref_count_get
 * Purpose:
 *      get the current reference Count
 * Parameters:
 *      unit              Unit number
 *      index            (IN) Entry Index
 */

STATIC void
bcmi_egr_ip_tnl_mpls_tunnel_ref_count_get(int unit, int index, int *cnt_value)
{
    bcmi_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);

    *cnt_value = mpls_info->egr_tunnel_ref_count[index];
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_ref_count_adjust
 * Purpose:
 *      Increment / Decrement Reference Count for Egress Tunnel
 * Parameters:
 *      unit              Unit number
 *      index            (IN) Entry Index
 *      step             (IN) no of references
 */

STATIC void
bcmi_egr_ip_tnl_mpls_ref_count_adjust (int unit, int index, int step)
{
    bcmi_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);

    if (( mpls_info->egr_tunnel_ref_count[index] == 0 ) && (step < 0)) {
         mpls_info->egr_tunnel_ref_count[index] = 0;
    } else {
         mpls_info->egr_tunnel_ref_count[index] += step;
    }

}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_ref_count_reset
 * Purpose:
 *      reset the reference Count
 * Parameters:
 *      unit              Unit number
 *      index             (IN) Entry Index
 */

STATIC void
bcmi_egr_ip_tnl_mpls_ref_count_reset (int unit, int index)
{
    bcmi_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);

    mpls_info->egr_tunnel_ref_count[index] = 0;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_check_dup_free_index
 * Purpose:
 *      check for duplicate free index.
 * Parameters:
 *      unit              Unit number
 *      fi                (IN) free index structure.
 *      free_entry_idx    (IN) free index where we should look.
 *      mpls_tnl_idx      (IN) original mpls tnl idx.
 * Return
 *      TRUE/FALSE;
 */
int
bcmi_egr_ip_tnl_mpls_check_dup_free_index(int unit,
                      bcmi_egr_ip_tnl_free_indexes_t *fi,
                      int free_entry_num, int mpls_tnl_idx)
{

    int free_idx=0;
    int ent_idx = free_entry_num - 1;

    for (; free_idx < 8; free_idx++) {
        if (fi->free_index_count[ent_idx]) {
            if (fi->free_entry_indexes[ent_idx][free_idx] == -1) {
                break;
            }
            if (fi->free_entry_indexes[ent_idx][free_idx] == mpls_tnl_idx) {
                return TRUE;
            }

        }
    }
    return FALSE;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_free_idx_update
 * Purpose:
 *      check for duplicate free index.
 * Parameters:
 *      unit              Unit number
 *      fi                (IN) free index structure.
 *      free_entries      (IN) no of free entries.
 *      start_index       (IN) start index.
 * Return
 *      TRUE/FALSE;
 */
void
bcmi_egr_ip_tnl_mpls_free_idx_update(int unit, int free_entries,
        int start_index, bcmi_egr_ip_tnl_free_indexes_t *fi)
{
    int free_entry_count;

    if (!(bcmi_egr_ip_tnl_mpls_check_dup_free_index(unit,
        fi, free_entries, start_index))) {

        if (fi->free_index_count[free_entries-1] < MAX_FREE_ENTRY_INDEXES) {
            free_entry_count = fi->free_index_count[free_entries-1];
            fi->free_entry_indexes[free_entries-1][free_entry_count] = start_index;
            fi->free_index_count[free_entries-1]++;
        }
    }
}

/*
 * Function:
 *      bcm_egr_ip_tnl_mpls_remark_free_indexes
 * Purpose:
 *      Mark correct free indexes for mpls tunnel entries.
 * Parameters:
 *      unit         - Device Number
 *      free_entries - number of free entries
 *      start_index  - Start index of tunnel entry
 *      fi           - free indexes database pointer.
 * Returns:
 *      BCM_E_XXX
 */
bcm_error_t
bcm_egr_ip_tnl_mpls_remark_free_indexes(int unit, int free_entries,
        int start_index, bcmi_egr_ip_tnl_free_indexes_t *fi)
{
    int temp_free_entries;
    int ent_per_tnl = _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

    if (free_entries == 0) {
        return BCM_E_NONE;
    }
    if (start_index == -1) {
        return BCM_E_NONE;
    }

    /*
     * if start index is 0, then any entry can be put there.
     * if start index is 4, then only 3 and 4 number of entries
     * can be put there.
     */
    if (((start_index % ent_per_tnl) == 0) ||
        (((start_index %
            (ent_per_tnl / 2)) == 0) &&
        ((free_entries > 2) && free_entries < 5))) {

        bcmi_egr_ip_tnl_mpls_free_idx_update(unit, free_entries,
            start_index, fi);
    } else {
        if ((free_entries >= 4)
                && ((((start_index + free_entries) %  ent_per_tnl) == 0))) {

            /*
             * the entry is free till end and has more
             * than 4 consecutive spaces then
             * break them into smaller and take
             * 4 spaces into single big entry.
             */
            temp_free_entries = free_entries - 4;
            while(temp_free_entries > 0) {
                if (temp_free_entries == 1) {
                    bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 1,
                        start_index, fi);

                    start_index++;
                    temp_free_entries--;
                } else if (temp_free_entries >= 2) {

                    bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 2,
                        start_index, fi);

                    start_index +=2;
                    temp_free_entries-=2;
                }
            }
            if (!(bcmi_egr_ip_tnl_mpls_check_dup_free_index(unit,
                fi, 4, start_index))) {

                bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 4, start_index, fi);
                start_index +=4;
                temp_free_entries-=4;
            }
        } else if ((free_entries >= 3)  &&
            ((((start_index + free_entries) %  ent_per_tnl) == 7))) {
            /*
             * the entry is free till second last and has more
             * than 3 consecutive spaces then break them into smaller
             * and take 3 spaces into single big entry.
             */
            temp_free_entries = free_entries - 3;
            while(temp_free_entries > 0) {
                if (temp_free_entries == 1) {
                    bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 1,
                        start_index, fi);

                    start_index++;
                    temp_free_entries--;
                } else if (temp_free_entries >= 2) {
                    bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 2,
                        start_index, fi);

                    start_index +=2;
                    temp_free_entries-=2;
                }
            }
            bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 3, start_index, fi);
            start_index +=3;
            temp_free_entries-=3;
        } else {
            temp_free_entries = free_entries;
            while(temp_free_entries > 0) {
                if (temp_free_entries == 1) {
                    bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 1,
                        start_index, fi);

                    start_index++;
                    temp_free_entries--;
                } else if (temp_free_entries >= 2) {
                    bcmi_egr_ip_tnl_mpls_free_idx_update(unit, 2,
                        start_index, fi);

                    start_index +=2;
                    temp_free_entries-=2;
                }
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_egr_ip_tnl_mpls_move_entries
 * Purpose:
 *      Move enties to create space for other entry.
 * Parameters:
 *      unit               - Device Number
 *      src_tnl_index      - Tunnel start index.
 *      src_entry_offset   - Mpls entry offset.
 *      dst_free_mpls_idx  - Dest mpls tunnel entry index.
 *      no_of_elements     - Number of elements to move.
 * Returns:
 *      BCM_E_XXX
 */
bcm_error_t
bcm_egr_ip_tnl_mpls_move_entries(int unit,
                        bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry,
                        int src_tnl_index, int src_entry_offset,
                        int dst_free_mpls_idx, int no_of_elements)
{

    int dst_offset;
    int src_offset;
    int src_value;
    egr_ip_tunnel_mpls_entry_t src_entry, dst_entry;
    int i = 0;
    int ent_per_tnl = _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
    int src_tnl_mpls_idx, dst_tnl_index;
    int ref_count;
    int rv;
    uint32 l3_max_entry[SOC_MAX_MEM_WORDS]; /* Buffer to write interface info. */
    uint32  *l3_if_entry_p;        /* Write buffer address.           */
    soc_mem_t mem;                 /* Interface table memory.         */
    intf_list_t *intf_list = NULL;

    /* Get interface table memory. */
    mem = BCM_XGS3_L3_MEM(unit,  intf);

    /* Zero the buffer. */
    l3_if_entry_p = (uint32 *)&l3_max_entry;

    /* Compute offsets */
    src_offset = src_entry_offset;
    dst_offset = dst_free_mpls_idx % ent_per_tnl;
    dst_tnl_index = dst_free_mpls_idx / ent_per_tnl;

    BCM_IF_ERROR_RETURN(READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
            src_tnl_index, &src_entry));

    BCM_IF_ERROR_RETURN(READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
            dst_tnl_index, &dst_entry));

    for (; i < no_of_elements; i++) {

        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_push_action_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_push_action_f[dst_offset+i], src_value);
        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_label_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_label_f[dst_offset+i], src_value);
        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_exp_select_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_exp_select_f[dst_offset+i], src_value);
        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_exp_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_exp_f[dst_offset+i], src_value);
        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_pri_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_pri_f[dst_offset+i], src_value);
        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_cfi_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_cfi_f[dst_offset+i], src_value);
        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_exp_ptr_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_exp_ptr_f[dst_offset+i], src_value);
        src_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, (&src_entry),
                  _tnl_ttl_f[src_offset+i]);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, (&dst_entry),
                  _tnl_ttl_f[dst_offset+i], src_value);
    }

    rv = WRITE_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                  dst_tnl_index, &dst_entry);

    if (rv == BCM_E_NONE) {
        src_tnl_mpls_idx = (src_tnl_index * ent_per_tnl) + src_entry_offset;
        for (i=0; i < no_of_elements; i++) {
            _BCM_MPLS_TNL_USED_SET(unit, dst_free_mpls_idx + i);
            _BCM_MPLS_TNL_USED_CLR(unit, src_tnl_mpls_idx + i);
        }
        /* get the moved entry's original ref counter number */
        bcmi_egr_ip_tnl_mpls_tunnel_ref_count_get (unit,
            src_tnl_mpls_idx, &ref_count);
        bcmi_egr_ip_tnl_mpls_ref_count_reset (unit, src_tnl_mpls_idx);
        bcmi_egr_ip_tnl_mpls_ref_count_adjust (unit,
            dst_free_mpls_idx, ref_count);

        /*
         * Now change tunnel index in L3 interface entry.
         */
        intf_list = tnl_sw_entry[src_tnl_index]->label_entry[src_entry_offset]->intf_list;

        while (intf_list) {
            sal_memset(l3_if_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, intf));
            BCM_IF_ERROR_RETURN(
                BCM_XGS3_MEM_READ(unit, mem, intf_list->intf_num, l3_if_entry_p));

            soc_mem_field32_set(unit, mem, l3_if_entry_p,
                MPLS_TUNNEL_INDEXf, dst_free_mpls_idx);

            rv = soc_mem_write(unit, mem, MEM_BLOCK_ANY, intf_list->intf_num, l3_if_entry_p);

            if (BCM_FAILURE(rv)) {
                return rv;
            }
            intf_list = intf_list->next;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_intf_list_alloc
 * Purpose:
 *      Allocates link memory and sets interface chain.
 * Parameters:
 *      intf_num - interface number
 * Returns:
 *      pointer to allocated link memory
 */
intf_list_t *
bcmi_egr_ip_tnl_mpls_intf_list_alloc(bcm_if_t intf_num)
{
    intf_list_t *ptr;

    ptr = (intf_list_t *) sal_alloc(sizeof(intf_list_t), "egr mpls tnl sw intf list");
    sal_memset(ptr, 0, sizeof(intf_list_t));

    ptr->intf_num = intf_num;
    ptr->next = NULL;

    return ptr;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_intf_list_dump
 * Purpose:
 *      Dumps interface chain.
 * Parameters:
 *      unit         - device number
 *      tnl_sw_entry - pointer to tunnel database.
 *      intf_num     - interface number
 *      tnl_idx      - Tunnel Index
 *      mpls_offset  - mpls entry offset in tunnel
 * Returns:
 *      None.
 */
void
bcmi_egr_ip_tnl_mpls_intf_list_dump(int unit,
                    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry,
                    bcm_if_t intf_num, int tnl_idx, int mpls_offset)
{

   intf_list_t *temp;

    temp = tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list;

    while (temp)  {
       LOG_ERROR(BSL_LS_BCM_L3,
           (BSL_META_U(unit, "intf index = %d, tnl_idx = %d, mpls_off = %d\n"),
            temp->intf_num, tnl_idx, mpls_offset));

        temp = temp->next;
    }
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_intf_list_add
 * Purpose:
 *      Adds interface chain at a mpls entry index.
 * Parameters:
 *      unit         - device number
 *      tnl_sw_entry - pointer to tunnel database.
 *      intf_id      - interface number
 *      tnl_idx      - Tunnel Index
 *      mpls_offset  - mpls entry offset in tunnel
 * Returns:
 *      bcm_error_t
 */
bcm_error_t
bcmi_egr_ip_tnl_mpls_intf_list_add(int unit,
                    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry,
                    bcm_if_t intf_id, int tnl_idx, int mpls_offset)
{

    intf_list_t *ptr, *temp, *prev;

    temp = tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list;
    prev = temp;
    while (temp) {
        if (temp->intf_num == intf_id) {
        /*
         * We already have this interface at this index.
         * this is just a negative case test if
         * someone tries to send same entry at same intf.
         */
            return BCM_E_NONE;
        }
        prev = temp;
        temp = temp->next;
    }

    ptr = bcmi_egr_ip_tnl_mpls_intf_list_alloc(intf_id);

    if (ptr == NULL) {
        return BCM_E_MEMORY;
    }

    if(temp == prev) {
        tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list = ptr;
    } else {
        prev->next = ptr;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_intf_list_delete
 * Purpose:
 *      Deletes interface chain matching to a
 *      particular interface number.
 * Parameters:
 *      unit         - device number
 *      tnl_sw_entry - pointer to tunnel database.
 *      intf_num     - interface number
 *      tnl_idx      - Tunnel Index
 *      mpls_offset  - mpls entry offset in tunnel
 * Returns:
 *      bcm_error_t
 */
bcm_error_t
bcmi_egr_ip_tnl_mpls_intf_list_delete(int unit, bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry,
                    bcm_if_t intf_num, int tnl_idx, int mpls_offset)
{

    intf_list_t *temp_ptr, *prev_ptr;

    if(tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list == NULL) {
        /* why we are here. this is not possible */
        return BCM_E_INTERNAL;
    }

    temp_ptr = tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list;
    prev_ptr = temp_ptr;

    while ((temp_ptr) && ((temp_ptr->intf_num) != intf_num)) {
        prev_ptr = temp_ptr;
        temp_ptr = temp_ptr->next;
    }

    if (temp_ptr) {
        if (temp_ptr == prev_ptr) {
            tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list = temp_ptr->next;
        } else {
            prev_ptr->next = temp_ptr->next;
        }

        sal_free(temp_ptr);
    } else {
        return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_intf_list_delete_all
 * Purpose:
 *      Deletes all nodex of Interface chain at that mpls
 *      entry index.
 * Parameters:
 *      unit         - device number
 *      tnl_sw_entry - pointer to tunnel database.
 *      intf_num     - interface number
 *      tnl_idx      - Tunnel Index
 *      mpls_offset  - mpls entry offset in tunnel
 * Returns:
 *      bcm_error_t
 */

void
bcmi_egr_ip_tnl_mpls_intf_list_delete_all(int unit,
                    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry,
                    int tnl_idx, int mpls_offset)
{

    intf_list_t *temp_ptr;

    temp_ptr = tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list;

    while (temp_ptr) {
        tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list =
            temp_ptr->next;

        sal_free(temp_ptr);
        temp_ptr = tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list;
    }
}

#if 0
void
bcmi_egr_ip_tnl_mpls_intf_list_copy(int unit,
                     bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry,
                     int dst_tnl_idx, int dst_mpls_offset,
                     int tnl_idx, int mpls_offset)
{


    /* we can just change the pointer and whole list can be picked from there */
    tnl_sw_entry[dst_tnl_idx]->label_entry[dst_mpls_offset]->intf_list =
        tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list;

    tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list = NULL;
}
#endif

/*
 * Function:
 *      bcm_egr_ip_tnl_mpls_sw_entry_reset
 * Purpose:
 *      Dumps interface chain.
 * Parameters:
 *      unit         - device number
 *      tnl_sw_entry - pointer to tunnel database.
 *      tnl_idx      - Tunnel Index
 *      mpls_offset  - mpls entry offset in tunnel
 *      clean_list   _ flag to decide whether to clean interface list.
 * Returns:
 *      bcm_error_t
 */


void bcm_egr_ip_tnl_mpls_sw_entry_reset(int unit,
                    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tnl_sw_entry,
                    int tnl_idx, int mpls_offset, int clean_list)
{

    if (clean_list) {
        tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->intf_list = NULL;
    }
    tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->flags = 0;
    tnl_sw_entry[tnl_idx]->label_entry[mpls_offset]->num_elements = 0;
}

/*
 * Function:
 *      bcmi_mpls_egr_tunnel_delete_free_indexes
 * Purpose:
 *      Delete free index entry from free indexes if the entry is used up.
 * Parameters:
 *      unit           - device number
 *      fi             - Pointer to free indexes database.
 *      num_labels     - Number of labels that entry contains.
 *      mpls_entry_idx - index to mpls entry index that is used up.
 * Returns:
 *      None.
 */
void
bcmi_mpls_egr_tunnel_delete_free_indexes(int unit,
                    bcmi_egr_ip_tnl_free_indexes_t *fi,
                    int num_labels, int mpls_entry_idx)
{
   int idx,free_idx=0;

    if ((num_labels <= 0) || (num_labels > 8)) {
        return;
    }

    idx = num_labels-1;
    for (; free_idx<MAX_FREE_ENTRY_INDEXES; free_idx++) {

        if (!(fi->free_index_count[idx])) {
            return;
        }

        if (fi->free_entry_indexes[idx][free_idx] == mpls_entry_idx) {
            /* free this space */
            if (free_idx == (fi->free_index_count[idx]-1)) {
                fi->free_entry_indexes[idx][free_idx] = -1;
            } else {
                fi->free_entry_indexes[idx][free_idx] =
                    fi->free_entry_indexes[idx][fi->free_index_count[idx]-1];
                fi->free_entry_indexes[idx][fi->free_index_count[idx]-1] = -1;
            }
            fi->free_index_count[idx]--;

            break;
        }
    }
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_move_tunnel_entries
 * Purpose:
 *      Once we identify that this is the tunnel index that can create
 *      space for our new entry, we try to move the mple tunnel entries
 *      from that tunnel index.
 * Parameters:
 *      unit           - (IN)device number
 *      fi             - (IN)Pointer to free indexes database.
 *      tunnel_entry   - (IN)Number of labels that entry contains.
 *      idx            - (IN)index to mpls entry index that is used up.
 *      num_labels     - (IN)Number of labels for which free space is made.
 *      mpls_tnl_idx   - (IN)mpls offset from where entry needs to be moved.
 * Returns:
 *      None.
 */
int
bcmi_egr_ip_tnl_mpls_move_tunnel_entries(int unit,
                      bcmi_egr_ip_tnl_free_indexes_t *fi,
                      bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry,
                      int src_tnl_idx, int src_mpls_tnl_off, int num_labels)
{
    int ent_per_idx = _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
    int temp_num_labels = num_labels;
    int left_free_entries = 0;
    int found = 0;
    int rv = BCM_E_NONE;
    int mpls_index = 0;
    int mpls_off = 0;
    int no_of_elements = 0;
    int f_idx, free_idx;
    int new_start_index;
    int chk_tnl_idx, chk_mpls_idx;
    bcmi_egr_ip_tnl_mpls_label_entry_t *label_entry;
    bcmi_egr_ip_tnl_mpls_label_entry_t *src_label_entry;


    mpls_index = (src_tnl_idx * _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit));
    /* check for entry free */
    for(mpls_off=src_mpls_tnl_off; mpls_off < ent_per_idx; mpls_off++) {
        found = 0;

        if (!_BCM_MPLS_TNL_USED_GET(unit, (mpls_index + mpls_off))) {
            temp_num_labels--;
            continue;
        }

        src_label_entry = tunnel_entry[src_tnl_idx]->label_entry[mpls_off];
        if(src_label_entry->flags
            & BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY) {
            /* this is the start of this entry,
             * we should check if we have some free space for this entry
             */
            no_of_elements = src_label_entry->num_elements;
            /* we also should check if there are some bigger entries in this
             * tunnel entry, big entries can be adjusted to other big entries
             * if exists.
             * else we might have to move smaller entries to bigger entries.
             */
            for (f_idx=no_of_elements-1; f_idx < 8; f_idx++) {

                if (fi->free_index_count[f_idx] == 0) {
                    continue;
                }

                for (free_idx=0; free_idx<8; free_idx++) {
                    /* There is some empty place here.
                     * lets use the same size entry or bigger.
                     * returning the first free entry in that block.
                     */
                    if (fi->free_entry_indexes[f_idx][free_idx] == -1) {
                        break;
                    }
                    chk_tnl_idx  = fi->free_entry_indexes[f_idx][free_idx] /
                        _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

                    chk_mpls_idx =  fi->free_entry_indexes[f_idx][free_idx] %
                        _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

                    label_entry =
                        tunnel_entry[chk_tnl_idx]->label_entry[chk_mpls_idx];
                    if (!(label_entry->flags &
                        BCMI_EGR_IP_TUNNEL_MPLS_ENTRY_CHECKED)) {

                        /*
                         * we can use free_entry_indexes[idx][free_idx];
                         * mark this entry used so that next lookup should
                         * not use this.
                         */
                        label_entry->flags |=
                            BCMI_EGR_IP_TUNNEL_MPLS_ENTRY_CHECKED;
                        found = 1;
                        rv = bcm_egr_ip_tnl_mpls_move_entries(unit,
                                    tunnel_entry, src_tnl_idx, mpls_off,
                                    fi->free_entry_indexes[f_idx][free_idx],
                                    no_of_elements);

                        if (rv == BCM_E_NONE) {
                        /* movement is successful */
                        label_entry->flags |=
                                BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY;
                        label_entry->num_elements = no_of_elements;
                        /* now copy interfaces.
                         * even if we are keeping pointer here for linked list,
                         * we can assign the same pointer to the new list.
                         */
                        label_entry->intf_list = src_label_entry->intf_list;
                        bcm_egr_ip_tnl_mpls_sw_entry_reset(unit, tunnel_entry,
                            src_tnl_idx, mpls_off, 1);
                        } else {
                            return rv;
                        }

                        temp_num_labels -= no_of_elements;
                        mpls_off += (no_of_elements-1);
                        new_start_index =
                            fi->free_entry_indexes[f_idx][free_idx] +
                            no_of_elements;

                        bcmi_mpls_egr_tunnel_delete_free_indexes(unit, fi,
                            no_of_elements,
                            fi->free_entry_indexes[f_idx][free_idx]);

                        left_free_entries = (f_idx+1)-no_of_elements;

                        bcm_egr_ip_tnl_mpls_remark_free_indexes(unit,
                            left_free_entries, new_start_index, fi);

                        break;
                    }
                }
                if (found) {
                    /*break from loop so that we can check next entry*/
                    break;
                }
            }
            if (!found) {
                temp_num_labels = num_labels;
            }
        }
        if (temp_num_labels <= 0) {

            /* we have moved all the entries to some other space.
             * we have this space free now.
             */
            return BCM_E_NONE;
        }
     }
    /* We cant not free any entry here. so return BCM_E_FULL */
    return BCM_E_FULL;
}

/*
 * Function:
 *           bcmi_xgs5_mpls_egr_tunnel_lookup
 * Purpose:
 *           loopup matching entry into  EGR_IP_TUNNEL_MPLS
 * Parameters:
 *           IN :  Unit
 *           IN :  push_action
 *           IN :  label_array
 *           OUT : match_mpls_tunnel_index
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
bcmi_xgs5_mpls_egr_tunnel_lookup (int unit, int push_action,
                              bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry,
                              bcm_mpls_egress_label_t *label_array,
                              int *match_mpls_tunnel_index)
{
    uint32 label_value;
    uint32 entry_ttl, entry_exp, entry_push_action;
    int num_ip_tnl_mpls;
    int i = 0, j = 0, tnl_index = -1, mpls_index = -1;
    egr_ip_tunnel_mpls_entry_t tnl_entry;
    int temp_push_action = 0;

    num_ip_tnl_mpls = soc_mem_index_count(unit, EGR_IP_TUNNEL_MPLSm);

    /* parameter checking */
    if (push_action != 0 && label_array == NULL) {
        return BCM_E_PARAM;
    }

    for (tnl_index = 0; tnl_index < num_ip_tnl_mpls; tnl_index++) {
        if (!_BCM_MPLS_IP_TNL_USED_GET(unit, tnl_index)) {
           continue;
        }

        mpls_index = tnl_index * _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

        for (i = 0; i <  _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit); i++) {
            if (!(tunnel_entry[tnl_index]->label_entry[i]->flags &
                BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY)) {
                continue;
            }
            if (!(tunnel_entry[tnl_index]->label_entry[i]->num_elements ==
                push_action)) {
                continue;
            }
            BCM_IF_ERROR_RETURN(
                READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                tnl_index, &tnl_entry));
            if (soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                ENTRY_TYPEf) != 3) {
                return BCM_E_INTERNAL;
            }

            if (push_action > 1) {
                j = 0;
                temp_push_action = push_action;

                while (temp_push_action > 1) {
                    if (!(_BCM_MPLS_TNL_USED_GET(unit, mpls_index + i + j))) {
                        break;
                    }
                    label_value = soc_mem_field32_get(unit,
                        EGR_IP_TUNNEL_MPLSm, &tnl_entry, _tnl_label_f[i+j]);
                    entry_ttl = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_ttl_f[i+j]);
                    entry_exp = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_exp_f[i+j]);
                    entry_push_action = soc_mem_field32_get(unit,
                        EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                        _tnl_push_action_f[i+j]);
                    /*
                     * entry_push_action should be 2 as next entry will also
                     * be checked with this entry.
                     */
                    if ((label_array[j].label == label_value) &&
                        (label_array[j].ttl == entry_ttl) &&
                        (label_array[j].exp == entry_exp) &&
                        (entry_push_action == 2)) {
                         /* Do nothing. This is good case so continue */
                         temp_push_action--;
                    } else {
                         /* we are not getting matching entry here */
                         break;
                    }
                    j++;
                 }

                label_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                    &tnl_entry, _tnl_label_f[i+j]);
                entry_ttl = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                    &tnl_entry, _tnl_ttl_f[i+j]);
                entry_exp = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                    &tnl_entry, _tnl_exp_f[i+j]);
                entry_push_action = soc_mem_field32_get(unit,
                    EGR_IP_TUNNEL_MPLSm, &tnl_entry, _tnl_push_action_f[i+j]);
                /*
                 * This is the last entry in the chain, therefore
                 * entry push actions should be 1.
                 */
                if ((label_array[j].label == label_value) &&
                    (label_array[j].ttl == entry_ttl) &&
                    (label_array[j].exp == entry_exp) &&
                    (entry_push_action == 1)) {

                    *match_mpls_tunnel_index = (tnl_index *
                        _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit)) + i;
                    return BCM_E_NONE;
                }

            } else if (push_action == 0x1) {
                label_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_label_f[i]);
                entry_ttl = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_ttl_f[i]);
                entry_exp = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_exp_f[i]);
                entry_push_action = soc_mem_field32_get(unit,
                    EGR_IP_TUNNEL_MPLSm, &tnl_entry, _tnl_push_action_f[i+j]);

                if ((label_array[0].label == label_value) &&
                    (label_array[0].ttl == entry_ttl) &&
                    (label_array[0].exp == entry_exp) &&
                    (push_action == entry_push_action)) {

                    *match_mpls_tunnel_index = (tnl_index *
                        _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit)) + i;
                    return BCM_E_NONE;
                }
            } else if (push_action == 0x0) {
                label_value = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_label_f[i]);
                entry_ttl = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_ttl_f[i]);
                entry_exp = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                        &tnl_entry, _tnl_exp_f[i]);
                entry_push_action = soc_mem_field32_get(unit,
                    EGR_IP_TUNNEL_MPLSm, &tnl_entry, _tnl_push_action_f[i+j]);

                /* Case of Dummy entry */
                if ((0 == label_value) &&
                    (0 == entry_ttl) &&
                    (0 == entry_exp) &&
                    (0 == entry_push_action)) {

                    *match_mpls_tunnel_index = (tnl_index *
                        _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit)) + i;
                    return BCM_E_NONE;
                }
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_check_availability
 * Purpose:
 *      Checks availability for incoming mpls tunnel
 *      entry into the existing free entries.
 * Parameters:
 *      unit         - (IN)device number
 *      fi           - (IN)Pointer to free indexes database.
 *      num_labels   - (IN)Number of labels that entry contains.
 * Returns:
 *      (INT)
 */
int
bcmi_egr_ip_tnl_mpls_check_availability(int unit,
                bcmi_egr_ip_tnl_free_indexes_t *fi,
                int num_labels)
{
    int idx,free_idx;
    int mpls_entry_index;

    if (num_labels == 0) {
        return -1;
    }
    /* MAX_FREE_ENTRY = */
    for (idx=num_labels-1; idx < MAX_ENTRY_INDEXES; idx++) {
        /* MAX FREE ENTRY COUNTING = 8 */
        if (fi->free_index_count[idx]) {
            /* entry array starts from 0 but count is from 1 */
            free_idx=fi->free_index_count[idx] - 1;
            for (; free_idx>=0; free_idx--) {
                /* There is some empty place here.
                 * lets use the same size ehtry or bigger.
                 * returning the first free entry in that block.
                 */
                if (fi->free_entry_indexes[idx][free_idx] == -1) {
                    /*
                     * count is set but no free index here.
                     * please reduce the count
                     */
                    fi->free_index_count[idx]--;
                    continue;
                }
                /*
                 * if we have reached here that means we have got the space
                 */
                mpls_entry_index = fi->free_entry_indexes[idx][free_idx];
                fi->free_entry_indexes[idx][free_idx] = -1;
                fi->free_index_count[idx]--;
                bcm_egr_ip_tnl_mpls_remark_free_indexes(unit,
                    (idx+1)-num_labels, mpls_entry_index+num_labels, fi);

                return mpls_entry_index;
            }
        }
    }
    return -1;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_get_free_index
 * Purpose:
 *      fills the free entry index database with after
 *      reading all the tunnel entries.
 * Parameters:
 *      unit         - (IN)device number
 *      num_labels   - (IN)Number of labels that entry contains.
 *      tunnel_entry - (IN)Pointer to mpls tunnel s/w state.
 *      fi           - (IN/OUT)Pointer to free indexes database.
 * Returns:
 *      (INT)
 */
int
bcmi_egr_ip_tnl_mpls_get_free_index(int unit, int num_labels,
                       bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry,
                       bcmi_egr_ip_tnl_free_indexes_t *fi)
{
    int idx=0, mpls_off=0;
    int mpls_index;
    int start_index = -1;
    int number_of_free_entries = 0;
    int free_tunnel_index = -1;
    int num_ip_tnl;
    bcmi_egr_ip_tnl_mpls_label_entry_t *label_entry;

    if (num_labels == 0) {
        return -1;
    }

    num_ip_tnl = soc_mem_index_count(unit, EGR_IP_TUNNEL_MPLSm);

    for (;idx < num_ip_tnl; idx++) {

        if (!_BCM_MPLS_IP_TNL_USED_GET(unit, idx)) {
            continue;
        }

        mpls_index = idx * _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
        start_index = -1;
        mpls_off = 0;
        number_of_free_entries = 0;
        for(;mpls_off < _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit); mpls_off++) {

            if (_BCM_MPLS_TNL_USED_GET(unit, (mpls_index + mpls_off))) {

                label_entry = tunnel_entry[idx]->label_entry[mpls_off];

                if (label_entry->flags &
                    BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY) {

                    mpls_off +=
                        label_entry->num_elements - 1;
                }
                if (!(bcmi_egr_ip_tnl_mpls_check_dup_free_index(unit,
                        fi,number_of_free_entries, start_index))) {

                    bcm_egr_ip_tnl_mpls_remark_free_indexes(unit,
                        number_of_free_entries, start_index, fi);
                }
                start_index = -1;
                number_of_free_entries = 0;

            } else {

                number_of_free_entries++;
                if (start_index == -1) {
                    start_index = mpls_index + mpls_off;
                }
                if (mpls_off == 7) {
                    /*
                     * this is the last tunnel index and therefore we
                     * should just update free entries
                     */
                    if (!(bcmi_egr_ip_tnl_mpls_check_dup_free_index(unit,
                            fi,number_of_free_entries, start_index))) {

                        bcm_egr_ip_tnl_mpls_remark_free_indexes(unit,
                            number_of_free_entries, start_index, fi);
                    }
                    start_index= -1;
                    number_of_free_entries = 0;

                }
            }
        }
        if (idx % num_labels == 0) {
           /* just checking after getting data of five labels */
            free_tunnel_index = bcmi_egr_ip_tnl_mpls_check_availability(unit,
                                    fi, num_labels);
        }
        if (free_tunnel_index != -1) {
            return free_tunnel_index;
        }
    }
    return -1;
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_create_local_free_indexes
 * Purpose:
 *      Create duplicate free indexes for local arithmatic.
 * Parameters:
 *      unit                         - (IN)device number
 *      free_entry_structures        - (IN)Pointer to free indexes database.
 *      local_free_indexes_structure - (IN)Pointer to free indexes database.
 * Returns:
 *      None.
 */
void
bcmi_egr_ip_tnl_mpls_create_local_free_indexes(int unit,
                    bcmi_egr_ip_tnl_free_indexes_t *free_entry_structures,
                    bcmi_egr_ip_tnl_free_indexes_t *local_free_indexes_structure)
{
    int i, j;

    sal_memset(local_free_indexes_structure, 0,
        sizeof(bcmi_egr_ip_tnl_free_indexes_t));
    for (i=0; i<MAX_ENTRY_INDEXES;i++) {
        for (j=0;j<MAX_FREE_ENTRY_INDEXES; j++) {
            local_free_indexes_structure->free_entry_indexes[i][j] =
                free_entry_structures->free_entry_indexes[i][j];
        }
        local_free_indexes_structure->free_index_count[i] =
            free_entry_structures->free_index_count[i];
    }
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_uncheck_free_indexes
 * Purpose:
 *      while looking for entries, we mark some entries as checkes
 *      already so that we do not try to use the same entry
 *      while looking for multiple entries..
 * Parameters:
 *      unit         - (IN)device number
 *      fi           - (IN)Pointer to free indexes database.
 *      tunnel_entry - (IN)Pointer to tunnel database
 * Returns:
 *      None.
 */
void
bcmi_egr_ip_tnl_mpls_uncheck_free_indexes(int unit,
                    bcmi_egr_ip_tnl_free_indexes_t *fi,
                    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry)
{
    int i, j;
    int tnl_idx, tnl_mpls_idx;

    for (i=0; i<MAX_ENTRY_INDEXES;i++) {
        for (j=0;j<MAX_FREE_ENTRY_INDEXES; j++) {

            if (fi->free_entry_indexes[i][j] == -1) {
                continue;
                                    }
            tnl_idx      = fi->free_entry_indexes[i][j] /
                               _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
            tnl_mpls_idx = fi->free_entry_indexes[i][j] %
                               _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

            tunnel_entry[tnl_idx]->label_entry[tnl_mpls_idx]->flags &=
                (~BCMI_EGR_IP_TUNNEL_MPLS_ENTRY_CHECKED);
        }

    }
}

/*
 * Function:
 *      bcmi_egr_ip_tnl_mpls_adjust_entry
 * Purpose:
 *      Reshuffle the entries if we are not able to fit
 *      the incoming entry.
 * Parameters:
 *      unit         - (IN)device number
 *      num_labels   - (IN)Number of labels that entry contains.
 *      tunnel_entry - (IN_Pointer to mpls tunnel s/w state.
 *      fi           - (IN)Pointer to free indexes database.
 *      tnl_idx      - (OUT)Pointer to index of freed entry if possible.
 * Returns:
 *      bcm_error_t
 */
bcm_error_t
bcmi_egr_ip_tnl_mpls_adjust_entry(int unit,
                    int num_labels,
                    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry,
                    bcmi_egr_ip_tnl_free_indexes_t *fi,
                    int *tnl_idx)
{
    int idx=0;
    bcmi_egr_ip_tnl_free_indexes_t local_fi; /* locan free indexes */
    int num_ip_tnl;
    int mpls_off;
    int no_of_elements;
    int temp_num_labels;
    int found = 0;
    int free_indexes_changed = 1;
    int f_idx, free_idx;
    int mpls_index;
    int left_free_entries;
    int new_start_index;
    int chk_tnl_idx, chk_mpls_idx;
    int rv;
    int src_mpls_index = 0;
    int ent_per_tunnel = _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
    bcmi_egr_ip_tnl_mpls_label_entry_t *label_entry;

    num_ip_tnl = soc_mem_index_count(unit, EGR_IP_TUNNEL_MPLSm);

    for (;idx < num_ip_tnl; idx++) {
        mpls_off = 0;
        temp_num_labels = num_labels;

        bcmi_egr_ip_tnl_mpls_uncheck_free_indexes(unit, fi, tunnel_entry);

        if (!_BCM_MPLS_IP_TNL_USED_GET(unit, idx)) {
            continue;
        }
        if (free_indexes_changed) {
            bcmi_egr_ip_tnl_mpls_create_local_free_indexes(unit, fi, &local_fi);
        }
        free_indexes_changed=0;
        mpls_index = idx * ent_per_tunnel;
        /* check for quad entry free */
        for(;mpls_off < ent_per_tunnel; mpls_off++) {
            found = 0;
            /*
             * if we have just started for entry, then do basic index checking.
             * if number of labels are 3 or 4 then only index 0 or index 4 are
             * valid indexes.
             * if number of labels are bigger than 4 then entry can start only
             * at index 0.
             * based on above rules, move the index to the next valid index.
             */
            if (temp_num_labels == num_labels) {
                src_mpls_index = mpls_index + mpls_off;
                if ((num_labels < 5) && (num_labels > 2)) {
                    if (mpls_off < 4 && mpls_off > 0) {
                        mpls_off = 4;
                    } else if (mpls_off > 4) {
                        mpls_off = 7;
                        continue;
                    }
                }
                if ((num_labels > 4) && (mpls_off > 0)) {
                    mpls_off = 7;
                    continue;
                }
            }
            if (!_BCM_MPLS_TNL_USED_GET(unit, (mpls_index + mpls_off))) {
                temp_num_labels--;
                continue;
            }

            if(tunnel_entry[idx]->label_entry[mpls_off]->flags &
                BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY) {
                /* this is the start of this entry,
                 * we should check if we have some free space for this entry
                 */
                no_of_elements =
                    tunnel_entry[idx]->label_entry[mpls_off]->num_elements;
                /* we also should check if there are some bigger entries
                 * in this tunnel entry,
                 * big entries can be adjusted to other big entries if exists.
                 * else we might have to move smaller entries to bigger entries
                 */
                if (no_of_elements > num_labels) {
                    /*
                     * no need to look for bigger entry as this is already
                     * checked above.
                     * if we could move bigger entry then we could have moved
                     * incoming entry itself
                     * so please return back with update to indexes.
                     */
                     mpls_off += no_of_elements-1;
                     temp_num_labels = num_labels;
                     /*
                      * we are doing continue here because we may like to
                      * check further entries also.
                      */
                     bcmi_egr_ip_tnl_mpls_uncheck_free_indexes(unit,
                         fi, tunnel_entry);
                     continue;
                }

                for (f_idx=no_of_elements-1; f_idx < 8; f_idx++) {
                    if (!(local_fi.free_index_count[f_idx])) {
                        continue;
                    }
                    for (free_idx=0; free_idx<8; free_idx++) {
                        /* There is some empty place here.
                         * lets use the same size entry or bigger.
                         * returning the first free entry in that block.
                         */
                        if (local_fi.free_entry_indexes[f_idx][free_idx]
                            == -1) {

                            break;
                        }
                        chk_tnl_idx =
                          local_fi.free_entry_indexes[f_idx][free_idx] /
                              ent_per_tunnel;
                        chk_mpls_idx =
                          local_fi.free_entry_indexes[f_idx][free_idx] %
                              ent_per_tunnel;
                        label_entry =
                          tunnel_entry[chk_tnl_idx]->label_entry[chk_mpls_idx];
                        if (!(label_entry->flags &
                                    BCMI_EGR_IP_TUNNEL_MPLS_ENTRY_CHECKED)) {
                            /*
                             * we can use free_entry_indexes[idx][free_idx];
                             * mark this entry used so that next lookup should
                             * not use this.
                             */
                            label_entry->flags |=
                                BCMI_EGR_IP_TUNNEL_MPLS_ENTRY_CHECKED;

                            found = 1;
                            free_indexes_changed = 1;

                            temp_num_labels -= no_of_elements;
                            mpls_off += (no_of_elements-1);
                            new_start_index =
                                fi->free_entry_indexes[f_idx][free_idx] +
                                no_of_elements;
                            bcmi_mpls_egr_tunnel_delete_free_indexes(unit,
                                &local_fi, no_of_elements,
                                fi->free_entry_indexes[f_idx][free_idx]);

                            left_free_entries = (f_idx+1)-no_of_elements;
                            bcm_egr_ip_tnl_mpls_remark_free_indexes(unit,
                                left_free_entries, new_start_index, &local_fi);
                            break;
                        }
                    }
                    if (found) {
                        break;
                    }
                }
                if (!found) {
                    temp_num_labels = num_labels;
                }
            }
            if (temp_num_labels <= 0) {

                /* if we are here that means we have got required space.
                 * get ready to move the entries.
                 */
                int mpls_tnl_off = src_mpls_index % ent_per_tunnel;

                bcmi_egr_ip_tnl_mpls_uncheck_free_indexes(unit, fi,
                    tunnel_entry);
                rv = bcmi_egr_ip_tnl_mpls_move_tunnel_entries(unit, fi,
                    tunnel_entry, idx, mpls_tnl_off, num_labels);

                if (rv == BCM_E_NONE) {
                    *tnl_idx = src_mpls_index;
                    /* we have got the index where we can fill this entry
                     * But we can have some free entries beneath it.
                     * so return those free entries to free indexes.
                     */
                    bcm_egr_ip_tnl_mpls_remark_free_indexes(unit,
                        (ent_per_tunnel - num_labels - mpls_tnl_off),
                        src_mpls_index+num_labels, &(fi_db[unit]));

                    return rv;
                }
            }
        }
    }
    /*
     * if we are here then we have not got the space.
     * Currently return BCM_E_FULLL.
     */
    return BCM_E_FULL;
}

/*
 * Function:
 *      bcmi_xgs5_mpls_tunnel_initiator_set
 * Purpose:
 *      Set MPLS Tunnel initiator
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      num_labels  - Number of labels in the array
 *      label_array - Array of MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_initiator_set(int unit, bcm_if_t intf, int num_labels,
                                 bcm_mpls_egress_label_t *label_array)
{
    egr_ip_tunnel_mpls_entry_t tnl_entry;
    egr_l3_intf_entry_t if_entry;
    int rv, num_mpls_map, i, push_action=0, offset = 0;
    int found = 0, tnl_index = 0, mpls_index = 0;
    int hw_map_idx;
    bcm_tunnel_initiator_t tnl_init;
    uint32 tnl_flags = 0;
    int match_mpls_tunnel_index=-1;
    int ent_per_tnl = 0;
    int tnl_id = 0;
    int mpls_offset = 0;
    int mpls_tunnel_update = 0;
    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry;

    ent_per_tnl = _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
    tunnel_entry = egr_mpls_tnl_sw_state[unit];

    if ((num_labels < 0) || (num_labels > ent_per_tnl)  ||
        (intf < 0) || (intf >= L3_INFO(unit)->l3_intf_table_size)) {
        return BCM_E_PARAM;
    }

    if ((num_labels > 2) &&
         !(soc_feature(unit, soc_feature_mpls_segment_routing))) {
        return BCM_E_UNAVAIL;
    }

    /* Derive action from labels.
     * based on number of labels, we can set multiple entries to add
     * desired number of labels into the tunnel.
     * Therefore push action is directly derived from num labels.
     */
    push_action = num_labels;

    /* Param checking */
    num_mpls_map = soc_mem_index_count(unit, EGR_MPLS_EXP_MAPPING_1m) / 64;
    for (i = 0; i < num_labels; i++) {
        if (label_array[i].qos_map_id == 0) { /* treat it as using default */
            hw_map_idx = 0;
        } else {
            BCM_IF_ERROR_RETURN(_egr_qos_id2hw_idx(unit,
                            label_array[i].qos_map_id,&hw_map_idx));
        }
        if ((label_array[i].label > 0xfffff) ||
            (hw_map_idx < 0) ||
            (hw_map_idx >= num_mpls_map) ||
            (label_array[i].exp > 7) || (label_array[i].pkt_pri > 7) ||
            (label_array[i].pkt_cfi > 1)) {
            return BCM_E_PARAM;
        }
    }
    if (!BCM_L3_INTF_USED_GET(unit, intf)) {
        LOG_INFO(BSL_LS_BCM_L3,
                 (BSL_META_U(unit,
                             "L3 interface not created\n")));
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    rv = READ_EGR_L3_INTFm(unit, MEM_BLOCK_ANY, intf, &if_entry);
    if (rv < 0) {
        return rv;
    }
   /* init the allocated entry */
   sal_memset(&tnl_entry, 0, sizeof(egr_ip_tunnel_mpls_entry_t));

   /* Case of Dummy Tunnel entry sharing by multiple L3_INTF */
   if (num_labels == 0) {
       /* Lookup if Tunnel Entry with Tunnel Label exists? */
       rv = bcmi_xgs5_mpls_egr_tunnel_lookup (unit, push_action, tunnel_entry,
                               NULL, &match_mpls_tunnel_index);
       if (rv < 0) {
          return rv;
       }

       if (match_mpls_tunnel_index != -1) {
           /* L3_Interface to point to mpls_tunnel */
           soc_EGR_L3_INTFm_field32_set(unit, &if_entry,
                              MPLS_TUNNEL_INDEXf, match_mpls_tunnel_index);
           /* Increment Ref count */
           bcmi_egr_ip_tnl_mpls_ref_count_adjust (unit,
                              match_mpls_tunnel_index, 1);
           rv = WRITE_EGR_L3_INTFm(unit, MEM_BLOCK_ANY, intf, &if_entry);

           if (rv == BCM_E_NONE) {
               tnl_id = match_mpls_tunnel_index/ent_per_tnl;
               mpls_offset = match_mpls_tunnel_index % ent_per_tnl;
               rv = bcmi_egr_ip_tnl_mpls_intf_list_add
                   (unit, tunnel_entry, intf, tnl_id, mpls_offset);

           }
           return rv;
       }
   }

   for (i = 0; i < num_labels; i++) {
       /* Check for Port_independent Label mapping */
       rv = bcm_tr_mpls_port_independent_range (unit, label_array[i].label,
                                                      BCM_GPORT_INVALID);
       /* Either Port-scope-label or Explicit-Null-label */
       if ((rv == BCM_E_CONFIG) || (label_array[i].label == 0)) {
           /* Tunnel-label is Port-based */
           /* Lookup if Tunnel Entry with Tunnel Label exists? */
           rv = bcmi_xgs5_mpls_egr_tunnel_lookup (unit, push_action,
               tunnel_entry, label_array, &match_mpls_tunnel_index);
           if (rv < 0) {
              return rv;
           }

           if (match_mpls_tunnel_index != -1) {
               /* L3_Interface to point to mpls_tunnel */
               soc_EGR_L3_INTFm_field32_set(unit, &if_entry,
                   MPLS_TUNNEL_INDEXf, match_mpls_tunnel_index);
               /* Increment Ref count */
               bcmi_egr_ip_tnl_mpls_ref_count_adjust (unit,
                   match_mpls_tunnel_index, 1);
               /* we also need to add this entry into sw state linked list */
               rv = WRITE_EGR_L3_INTFm(unit, MEM_BLOCK_ANY, intf, &if_entry);
               if (rv == BCM_E_NONE) {
                   rv = bcmi_egr_ip_tnl_mpls_intf_list_add(unit, tunnel_entry,
                       intf, tnl_id, offset);
               }
               return rv;
           }
       }
    }
    mpls_index = soc_EGR_L3_INTFm_field32_get(unit, &if_entry,
                                              MPLS_TUNNEL_INDEXf);
    tnl_index = mpls_index / ent_per_tnl;

    if ((mpls_index != 0) && (_BCM_MPLS_TNL_USED_GET(unit, mpls_index))) {
       /* Obtain referenced Tunnel entry */
       /* try using existing entry in tunnel */
        rv = READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                                      tnl_index, &tnl_entry);
        if (rv < 0) {
            return rv;
        }
        if (soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                ENTRY_TYPEf) != 3) {
            return BCM_E_INTERNAL;
        }
        /* we need to check here if at least number of labels is same.*/
        offset = mpls_index % ent_per_tnl;
        if (tunnel_entry[tnl_index]->label_entry[offset]->flags &
            BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY) {
            if (tunnel_entry[tnl_index]->label_entry[offset]->num_elements !=
                num_labels) {
                /* Changing push action/ num labels is not permitted,
                 * need to clear/set
                 */
                return BCM_E_PARAM;
            }
        } else {
            return BCM_E_PARAM;
        }
        mpls_tunnel_update = 1;

    } else {
        /* allocate an unused EGR_IP_TUNNEL_MPLS_ENTRY */
        /*
         * First try finding an allocated tunnel entry with
         * unused MPLS entries.
         */
        mpls_index =  bcmi_egr_ip_tnl_mpls_check_availability(unit,
            &(fi_db[unit]), num_labels);

        if (mpls_index == -1) {
            /*
             * we have not got corresponding free indexes.
             * lets try to create some more free indexes
             * and then try again.
             */
            mpls_index = bcmi_egr_ip_tnl_mpls_get_free_index(unit, num_labels,
                tunnel_entry, &(fi_db[unit]));
        }
        if (mpls_index != -1) {
               found = 1;
            tnl_index = mpls_index /
                (_BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit) + 1);
            rv = READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                tnl_index, &tnl_entry);

            if (rv < 0) {
                return rv;
            }

        } else {
            /*
             * Alloc an egr_ip_tunnel entry. By calling bcm_xgs3_tnl_init_add
             * with _BCM_L3_SHR_WRITE_DISABLE flag, a tunnel index is
             * allocated but nothing is written to hardware. The "tnl_init"
             * information is not used, set to all zero.
             */
            sal_memset(&tnl_init, 0, sizeof(bcm_tunnel_initiator_t));
            tnl_flags = _BCM_L3_SHR_MATCH_DISABLE | _BCM_L3_SHR_WRITE_DISABLE |
                _BCM_L3_SHR_SKIP_INDEX_ZERO;
            tnl_init.type =  bcmTunnelTypeMpls;
            rv = bcm_xgs3_tnl_init_add(unit, tnl_flags, &tnl_init, &tnl_index);
            if (rv == BCM_E_FULL) {

                /* There is no entry left unused.
                 * now see if we can move some entries and make space
                 */

                rv = bcmi_egr_ip_tnl_mpls_adjust_entry(unit, num_labels,
                    tunnel_entry, &(fi_db[unit]), &mpls_index);
                if (rv < 0) {
                    return rv;
                }

            } else {
                found = 1;
                /* we have got full entry here. start at 0 index.
                 * we will get double entry from l3_tbl_add as it
                 *  works on common resorce of egr_ip_tunnel.
                 *  But egr_ip_tunnel_mpls uses double entries.
                 */
                tnl_index = (tnl_index + 1)/2;
                mpls_index =  tnl_index *
                    (_BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit) + 1);
                /* we are using two tunnel entries at same time.*/
                _BCM_MPLS_IP_TNL_USED_SET(unit, tnl_index);
                _BCM_MPLS_TNL_USED_SET(unit, mpls_index);
                sal_memset(&tnl_entry, 0, sizeof(egr_ip_tunnel_mpls_entry_t));
                if (num_labels < ent_per_tnl) {
                    bcm_egr_ip_tnl_mpls_remark_free_indexes(unit,
                        (ent_per_tnl - ((num_labels) ? num_labels : 1)),
                        mpls_index+((num_labels) ? num_labels : 1),
                        &(fi_db[unit]));
                }
            }
        }
    }

    offset = mpls_index & _BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit);
    tnl_id = mpls_index /
        (_BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit) + 1);

    tunnel_entry[tnl_id]->label_entry[offset]->flags |=
                BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY;
    tunnel_entry[tnl_id]->label_entry[offset]->num_elements =
        ((num_labels) ? num_labels : 1);

    for (i = 0; i < num_labels; i++) {
        if (label_array[i].qos_map_id == 0) { /* treat it as using default */
            hw_map_idx = 0;
        } else {
            rv = _egr_qos_id2hw_idx(unit,
                            label_array[i].qos_map_id,&hw_map_idx);
            if (rv < 0) {
                goto cleanup;
            }
        }

        /* coverity is complaining that offset will become
         * more than the max index for _tnl_push_action_f.
         * It can not be true as offset is calculated based
         * on total number of labels that can be added at that
         * particular label index in full tunnel entry.
         */
        /* coverity[overrun_local : FALSE] */
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            _tnl_push_action_f[offset], ((i == (num_labels - 1)) ? 1 : 2));
        if (num_labels > 0) {
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_label_f[offset], label_array[i].label);
        }
        if ((label_array[i].flags & BCM_MPLS_EGRESS_LABEL_EXP_SET) ||
            (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_PRI_SET)) {

            /* Use the specified EXP, PRI and CFI */
            if ((label_array[i].flags & BCM_MPLS_EGRESS_LABEL_EXP_REMARK) ||
                (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_EXP_COPY) ||
                (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_PRI_REMARK)) {
                rv = BCM_E_PARAM;
                goto cleanup;
            }
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_select_f[offset], 0x0); /* USE_FIXED */
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_f[offset], label_array[i].exp);
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_pri_f[offset], label_array[i].pkt_pri);
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_cfi_f[offset], label_array[i].pkt_cfi);
        } else if (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_EXP_REMARK) {
            if (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_PRI_SET) {
                rv = BCM_E_PARAM;
                goto cleanup;
            }
            /* Use EXP-map for EXP, PRI and CFI */
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_select_f[offset], 0x1); /* USE_MAPPING */
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_ptr_f[offset], hw_map_idx);
        } else { /* BCM_MPLS_EGRESS_LABEL_EXP_COPY */
            /* Use EXP from inner label. If there is no inner label,
             * use the specified EXP value. Use EXP-map for PRI/CFI.
             */
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_select_f[offset], 0x2); /* USE_INNER */

            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_f[offset], label_array[i].exp);

            /* Use EXP-map for PRI/CFI */
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_ptr_f[offset], hw_map_idx);
        }

        if (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
            /* Use specified TTL */
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_ttl_f[offset], label_array[i].ttl);
        } else {
            /* coverity[overrun_local : FALSE] */
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_ttl_f[offset], 0);
        }
#if defined(BCM_TOMAHAWK2_SUPPORT)
        if (soc_feature(unit, soc_feature_mpls_ecn)) {
            if (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_ECN_TO_EXP_MAP) {
                int ecn_map_index;
                int ecn_map_type;
                int ecn_map_num;
                int ecn_map_hw_idx;
                ecn_map_type = label_array[i].ecn_map_id & 
                                _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK;
                ecn_map_index = label_array[i].ecn_map_id & 
                                _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
                ecn_map_num = 
                    soc_mem_index_count(unit, EGR_IP_ECN_TO_EXP_MAPPING_TABLEm) / 
                    _BCM_ECN_MAX_ENTRIES_PER_MAP;
                if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP) {
                    rv = BCM_E_PARAM;
                    goto cleanup;
                }
                if (ecn_map_index >= ecn_map_num) {
                    rv = BCM_E_PARAM;
                    goto cleanup;
                }
                if (!bcmi_xgs5_ecn_map_used_get(unit, ecn_map_index, 
                                                _bcmEcnmapTypeIpEcn2Exp)) {
                    rv = BCM_E_PARAM;
                    goto cleanup; 
                }
                rv = bcmi_ecn_map_id2hw_idx(unit, label_array[i].ecn_map_id, 
                                            &ecn_map_hw_idx);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }
                soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry, 
                                    _tnl_ecn_ptr_f[offset], 
                                    ecn_map_hw_idx); 
                if (label_array[i].flags & 
                    BCM_MPLS_EGRESS_LABEL_ECN_EXP_MAP_TRUST) {
                    soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry, 
                                        _tnl_ecn_ptr_pri_f[offset], 1);
                } else {
                    soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry, 
                                        _tnl_ecn_ptr_pri_f[offset], 0);

                }
            }
            if (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_INT_CN_TO_EXP_MAP) {
                int ecn_map_index;
                int ecn_map_type;
                int ecn_map_num;
                int ecn_map_hw_idx;
                ecn_map_type = label_array[i].int_cn_map_id & 
                                _BCM_XGS5_MPLS_ECN_MAP_TYPE_MASK;
                ecn_map_index = label_array[i].int_cn_map_id & 
                                _BCM_XGS5_MPLS_ECN_MAP_NUM_MASK;
                ecn_map_num = 
                    soc_mem_index_count(unit, EGR_INT_CN_TO_EXP_MAPPING_TABLEm) / 
                    _BCM_ECN_MAX_ENTRIES_PER_MAP;
                if (ecn_map_type != _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP) {
                    rv = BCM_E_PARAM;
                    goto cleanup;

                }
                if (ecn_map_index >= ecn_map_num) {
                    rv = BCM_E_PARAM;
                    goto cleanup;
                }
                if (!bcmi_xgs5_ecn_map_used_get(unit, ecn_map_index, 
                                                _bcmEcnmapTypeIntcn2Exp)) {
                    rv = BCM_E_PARAM;
                    goto cleanup;
                }
                rv = bcmi_ecn_map_id2hw_idx(unit, label_array[i].int_cn_map_id, 
                                            &ecn_map_hw_idx);
                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }                
                soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry, 
                                    _tnl_int_cn_ptr_f[offset], 
                                    ecn_map_hw_idx); 
                if (label_array[i].flags & 
                    BCM_MPLS_EGRESS_LABEL_ECN_EXP_MAP_TRUST) {
                    soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry, 
                                        _tnl_int_cn_ptr_pri_f[offset], 1);
                } else {
                    soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry, 
                                        _tnl_int_cn_ptr_pri_f[offset], 0);

                }
            }            
        }
#endif  
        offset++;
        _BCM_MPLS_TNL_USED_SET(unit, mpls_index + i);
    }
    bcmi_egr_ip_tnl_mpls_ref_count_adjust (unit,
                                mpls_index, 1);

    soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                ENTRY_TYPEf, 0x3);
    soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                ENTRY_TYPE_COPYf, 0x3);

    /* Commit the values to HW */
    rv = WRITE_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY, tnl_id, &tnl_entry);
    if (rv < 0) {
        goto cleanup;
    }

    /* Update the EGR_L3_INTF to point to the MPLS tunnel entry */
    rv = soc_mem_field32_modify(unit, EGR_L3_INTFm, intf,
                                MPLS_TUNNEL_INDEXf, mpls_index);
    if (rv < 0) {
        goto cleanup;
    }


    offset = mpls_index & _BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit);
    tnl_id = mpls_index / (_BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit) + 1);

    rv = bcmi_egr_ip_tnl_mpls_intf_list_add(unit, tunnel_entry, intf, tnl_id, offset);
    if (rv < 0) {
        goto cleanup;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return rv;

  cleanup:
    if (mpls_tunnel_update) {
        return rv;
    }
    if (!found) {
        /* Free the tunnel entry */
        (void) bcm_xgs3_tnl_init_del(unit, tnl_flags, tnl_index * 2);

        /* Clear "in-use" for the IP tunnel entry */
        _BCM_MPLS_IP_TNL_USED_CLR(unit, tnl_index);
    }
    /* Clear "in-use" for the MPLS tunnel entry(s) */
    for (i = 0; i < num_labels; i++) {
        _BCM_MPLS_TNL_USED_CLR(unit, mpls_index + i);
    }
    return rv;
}

/*
 * Function:
 *      bcmi_mpls_egr_tunnel_add_free_indexes
 * Purpose:
 *      Adds free index entry into free indexes if the entry is freed.
 * Parameters:
 *      unit           - device number
 *      fi             - Pointer to free indexes database.
 *      num_labels     - Number of labels that entry contains.
 *      mpls_entry_idx - index to mpls entry index that is used up.
 * Returns:
 *      None.
 */

void
bcmi_mpls_egr_tunnel_add_free_indexes(int unit,
                    bcmi_egr_ip_tnl_free_indexes_t *fi,
                    int num_labels, int mpls_entry_idx)
{
    int tnl_index, mpls_index, offset;
    int new_mpls_entry_idx, new_num_elements;
    int i=0, j=0;
    int ent_per_tnl = _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

    tnl_index = mpls_entry_idx/ent_per_tnl;
    mpls_index = tnl_index * ent_per_tnl;

    new_mpls_entry_idx = mpls_entry_idx;
    new_num_elements     = num_labels;
   /* check from the start of an entry */
    for(; i<ent_per_tnl; i++) {
        if (_BCM_MPLS_TNL_USED_GET(unit, mpls_index + i)) {
            continue;
        }
        while ((mpls_index + i + j) < mpls_entry_idx) {

            if (_BCM_MPLS_TNL_USED_GET(unit, mpls_index + i + j)) {
                i += j;
                j = 0;
                break;
            }
            j++;
        }
    }

    /* delete this free entry from the free entry db */
    if (j > 0) {
        new_mpls_entry_idx = mpls_entry_idx - j;
        new_num_elements     = num_labels+j;
        bcmi_mpls_egr_tunnel_delete_free_indexes(unit, fi, j,
            new_mpls_entry_idx);
    }

    offset = new_mpls_entry_idx % ent_per_tnl;
    offset += new_num_elements;

    j = 0;
    /* now check towards end of the entry */
    for(i=offset; i<ent_per_tnl; i++) {
        if (_BCM_MPLS_TNL_USED_GET(unit, new_mpls_entry_idx + i)) {
            break;
        }

        j++;
    }
    if (j > 0) {
        /* delete this free entries from free entry db */
        bcmi_mpls_egr_tunnel_delete_free_indexes(unit, fi, j,
            new_mpls_entry_idx + offset);
        new_num_elements += j;
    }
    /* Now we can remark this new entry into the database. */
    bcm_egr_ip_tnl_mpls_remark_free_indexes(unit, new_num_elements,
        new_mpls_entry_idx, fi);
}

/*
 * Function:
 *      _bcm_tr_mpls_tunnel_initiator_clear
 * Purpose:
 *      Clear MPLS Tunnel initiator
 * Parameters:
 *      unit        - (IN)Device Number
 *      intf        - (IN)Egress L3 interface
 * Returns:
 *      BCM_E_XXX
 */
bcm_error_t
bcmi_xgs5_mpls_tunnel_initiator_clear(int unit, int intf_id)
{
    egr_ip_tunnel_mpls_entry_t tnl_entry;
    egr_l3_intf_entry_t if_entry;
    int rv, mpls_offset, mpls_entry_used, i;
    int tnl_idx = 0, mpls_index = 0;
    uint32 tnl_flags;
    _bcm_tr_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry;
    bcmi_egr_ip_tnl_free_indexes_t *fi = &(fi_db[unit]);
    int mpls_tnl_start_index = 0;
    int no_of_elements;

    tunnel_entry = egr_mpls_tnl_sw_state[unit];

    if (tunnel_entry == NULL) {
        return BCM_E_INIT;
    }
    /* L3 interface info */
    rv = READ_EGR_L3_INTFm(unit, MEM_BLOCK_ANY, intf_id, &if_entry);
    if (rv < 0) {
        return rv;
    }
    mpls_index = soc_EGR_L3_INTFm_field32_get(unit, &if_entry,
                                              MPLS_TUNNEL_INDEXf);
    tnl_idx = mpls_index / _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
    mpls_tnl_start_index = tnl_idx * _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

    if (!_BCM_MPLS_TNL_USED_GET(unit, mpls_index)) {
        return BCM_E_NOT_FOUND;
    }
    rv = READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                                  tnl_idx, &tnl_entry);
    if (rv < 0) {
        return rv;
    }
    if (soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            ENTRY_TYPEf) != 3) {
        return BCM_E_NOT_FOUND;
    }
    mpls_offset = mpls_index & _BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit);


    if (tunnel_entry[tnl_idx]->label_entry[mpls_offset]->flags
         & BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY) {

        no_of_elements =
            tunnel_entry[tnl_idx]->label_entry[mpls_offset]->num_elements;

        bcmi_egr_ip_tnl_mpls_ref_count_adjust (unit, mpls_index, -1);
    } else {
        return BCM_E_NOT_FOUND;
    }

    bcm_egr_ip_tnl_mpls_sw_entry_reset(unit, tunnel_entry, tnl_idx,
        mpls_offset, 0);
    rv = bcmi_egr_ip_tnl_mpls_intf_list_delete(unit, tunnel_entry, intf_id,
        tnl_idx, mpls_offset);


    if (rv < 0) {
        return rv;
    }

    /* Update the EGR_L3_INTF to no longer point to the MPLS tunnel entry */
    rv = soc_mem_field32_modify(unit, EGR_L3_INTFm, intf_id,
                                MPLS_TUNNEL_INDEXf, 0);
    if (rv < 0) {
        return rv;
    }

    if ( mpls_info->egr_tunnel_ref_count[mpls_index] != 0x0 ) {
         return BCM_E_NONE;
    }

    /* Clear the MPLS tunnel entry(s) */
    rv = READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                                  tnl_idx, &tnl_entry);
    if (rv < 0) {
        return rv;
    }

    for (i=0; i < no_of_elements; i++) {

        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            _tnl_label_f[mpls_offset + i], 0);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            _tnl_push_action_f[mpls_offset + i], 0);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            _tnl_exp_select_f[mpls_offset + i], 0);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            _tnl_exp_ptr_f[mpls_offset + i], 0);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            _tnl_exp_f[mpls_offset + i], 0);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            _tnl_ttl_f[mpls_offset + i], 0);
#if defined(BCM_TOMAHAWK2_SUPPORT)
        if (soc_feature(unit, soc_feature_mpls_ecn)) {
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_ecn_ptr_f[mpls_offset + i], 0);
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_ecn_ptr_pri_f[mpls_offset + i], 0);
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_int_cn_ptr_f[mpls_offset + i], 0);
            soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_int_cn_ptr_pri_f[mpls_offset + i], 0);            
        }
#endif
        _BCM_MPLS_TNL_USED_CLR(unit, mpls_index + i);
    }

    /* See if we can free the IP tunnel base entry */
    _BCM_MPLS_TNL_USED_RANGE_GET(unit, mpls_tnl_start_index,
            _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit), mpls_entry_used);

    if (!mpls_entry_used) {
        /* None of the 4 or 8 entries are used, free base entry */
        tnl_flags = 0;
        (void) bcm_xgs3_tnl_init_del(unit, tnl_flags, tnl_idx * 2);

        /* Clear "in-use" for the IP tunnel entry */
        _BCM_MPLS_IP_TNL_USED_CLR(unit, tnl_idx);

        /* Clear HW Entry-Type */
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                ENTRY_TYPEf, 0);
        soc_mem_field32_set(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                ENTRY_TYPE_COPYf, 0);
        bcmi_egr_ip_tnl_mpls_free_indexes_clear(unit, tnl_idx, fi);
    } else {
        bcmi_mpls_egr_tunnel_add_free_indexes(unit, fi, no_of_elements,
             mpls_index);
    }

    rv = WRITE_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                                   tnl_idx, &tnl_entry);
    if (rv < 0) {
        return rv;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_xgs5_mpls_tunnel_initiator_clear_all
 * Purpose:
 *      Clear all MPLS Tunnel Initiators
 * Parameters:
 *      unit - (IN)Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_initiator_clear_all(int unit)
{
    egr_l3_intf_entry_t if_entry;
    int mpls_index;
    int rv=BCM_E_NONE;
    int num_ip_tnl_mpls;
    int i,j;

    for (i = 0; i < L3_INFO(unit)->l3_intf_table_size; i++) {
        if (!BCM_L3_INTF_USED_GET(unit, i)) {
            continue;
        }

        rv = READ_EGR_L3_INTFm(unit, MEM_BLOCK_ANY, i, &if_entry);
        if (rv < 0) {
            return rv;
        }
        mpls_index = soc_EGR_L3_INTFm_field32_get(unit, &if_entry,
                                                  MPLS_TUNNEL_INDEXf);
        if (!_BCM_MPLS_TNL_USED_GET(unit, mpls_index)) {
            continue;
        }
        rv = bcmi_xgs5_mpls_tunnel_initiator_clear(unit, i);
        if (rv < 0) {
            return rv;
        }
    }

    /* cleanup the s/w state for tunnel table. */
    num_ip_tnl_mpls = soc_mem_index_count(unit, EGR_IP_TUNNEL_MPLSm);

    for (i=0; i<num_ip_tnl_mpls; i++) {
        for (j=0; j<_BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit); j++) {
            bcmi_egr_ip_tnl_mpls_intf_list_delete_all(unit,
                egr_mpls_tnl_sw_state[unit], i, j);

            egr_mpls_tnl_sw_state[unit][i]->label_entry[j]->intf_list = NULL;
            egr_mpls_tnl_sw_state[unit][i]->label_entry[j]->flags = 0;
            egr_mpls_tnl_sw_state[unit][i]->label_entry[j]->num_elements = 0;

        }
    }

    /* cleanup the free indexes */
    bcmi_egr_ip_tnl_mpls_free_indexes_init(unit, &(fi_db[unit]));

    return rv;
}


/*
 * Function:
 *      bcmi_xgs5_mpls_tunnel_initiator_reinit
 * Purpose:
 *      set s/w state of MPLS tunnel during warmboot.
 * Parameters:
 *      unit        - (IN) Device Number
 * Returns:
 *      BCM_E_XXX
 */
bcm_error_t
bcmi_xgs5_mpls_tunnel_initiator_reinit(int unit)
{
    int tnl_idx=0, mpls_off = 0;
    egr_ip_tunnel_mpls_entry_t *tnl_entry = NULL;
    int start_of_entry = 0;
    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry;
    /* Indexes to iterate over memories, chunks and entries */
    int             chnk_idx, ent_idx, chnk_idx_max, mem_idx_max;
    int             buf_size, chunksize, chnk_end;
    uint32          *tbl_chnk;
    uint32          *el3inf_entry;
    int             rv = BCM_E_NONE;
    soc_mem_t mem;
    int last_push_action = 0;
    int num_labels = 0;
    int push_action = 0;
    int mpls_offset = 0;
    int mpls_index = 0;
    int index_min, index_max;
    uint8 *tunnel_entry_buf = NULL;

    tunnel_entry = egr_mpls_tnl_sw_state[unit];

    mem = EGR_IP_TUNNEL_MPLSm;
    tunnel_entry_buf = soc_cm_salloc(unit,
            SOC_MEM_TABLE_BYTES(unit, mem), "MPLS TUNNEL ENTRY buffer");
    if (NULL == tunnel_entry_buf) {
        return BCM_E_MEMORY;
    }

    index_min = soc_mem_index_min(unit, mem);
    index_max = soc_mem_index_max(unit, mem);
    rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ANY,
            index_min, index_max, tunnel_entry_buf);
    if (SOC_FAILURE(rv)) {
        soc_cm_sfree(unit, tunnel_entry_buf);
        return rv;
    }

    for (tnl_idx = index_min; tnl_idx <= index_max; tnl_idx++) {

        if (!_BCM_MPLS_IP_TNL_USED_GET(unit, tnl_idx)) {
            continue;
        }

        last_push_action = 0;
        start_of_entry = 0;
        num_labels = 0;

       tnl_entry = soc_mem_table_idx_to_pointer
            (unit, EGR_IP_TUNNEL_MPLSm, egr_ip_tunnel_mpls_entry_t *, tunnel_entry_buf, tnl_idx);

        /* Check for valid entry */

        for(mpls_off = 0; mpls_off<_BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit); mpls_off++) {
            mpls_index = tnl_idx * _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit) + mpls_off;

            if (!_BCM_MPLS_TNL_USED_GET(unit, mpls_index)) {
                start_of_entry = 0;
                last_push_action = 0;
                num_labels = 0;
                continue;
            }

            if (last_push_action == 2) {
                start_of_entry = 0;
            } else {
                start_of_entry = 1;
            }

            if (start_of_entry) {
                tunnel_entry[tnl_idx]->label_entry[mpls_off]->flags |=
                    BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY;
            }

            push_action = soc_mem_field32_get(unit, mem,
                tnl_entry, _tnl_push_action_f[mpls_off]);
            if (push_action == 2) {
                num_labels++;
            } else {
                if (last_push_action == 2) {
                    mpls_offset = mpls_off-num_labels;
                } else {
                    mpls_offset = mpls_off;
                }
                num_labels++;
                tunnel_entry[tnl_idx]->label_entry[mpls_offset]->num_elements =
                   num_labels;
                start_of_entry = 0;
                num_labels = 0;
            }
            last_push_action = push_action;
        }
    }

    soc_cm_sfree(unit, tunnel_entry_buf);

    /* need to look into EGR_L3_INTF table to get interface entry */
    mem = EGR_L3_INTFm;

    chunksize = 0x400; /* 1k */
    buf_size = sizeof(egr_l3_intf_entry_t) * chunksize;
    tbl_chnk = soc_cm_salloc(unit, buf_size, "egr_l3_intf traverse");
    if (NULL == tbl_chnk) {
        return BCM_E_MEMORY;
    }

    mem_idx_max = soc_mem_index_max(unit, mem);
    for (chnk_idx = soc_mem_index_min(unit, mem);
         chnk_idx <= mem_idx_max;
         chnk_idx += chunksize) {
        sal_memset((void *)tbl_chnk, 0, buf_size);

        chnk_idx_max =
            ((chnk_idx + chunksize) <= mem_idx_max) ?
            chnk_idx + chunksize - 1: mem_idx_max;

        soc_mem_lock(unit, mem);
        rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ANY,
                                chnk_idx, chnk_idx_max, tbl_chnk);
        if (SOC_FAILURE(rv)) {
            soc_mem_unlock(unit, mem);
            break;
        }
        chnk_end = (chnk_idx_max - chnk_idx);
        for (ent_idx = 0 ; ent_idx <= chnk_end; ent_idx ++) {
            el3inf_entry =
                soc_mem_table_idx_to_pointer(unit, mem, uint32 *,
                                             tbl_chnk, ent_idx);

            mpls_index = soc_mem_field32_get(unit, mem, el3inf_entry,
                                            MPLS_TUNNEL_INDEXf);

            if (!mpls_index) {
                continue;
            }

            tnl_idx = mpls_index  /
                _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);
            mpls_off = mpls_index %
               _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

            rv = bcmi_egr_ip_tnl_mpls_intf_list_add(unit, tunnel_entry,
                chnk_idx + ent_idx, tnl_idx, mpls_off);
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
        soc_mem_unlock(unit, mem);
        if (BCM_FAILURE(rv)) {
            break;
        }
    }

    soc_cm_sfree(unit, tbl_chnk);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmi_xgs5_mpls_tunnel_initiator_get
 * Purpose:
 *      Get MPLS Tunnel Initiator info
 * Parameters:
 *      unit        - (IN) Device Number
 *      intf        - (IN) The egress L3 interface
 *      label_max   - (IN) Number of entries in label_array
 *      label_array - (OUT) MPLS header information
 *      label_count - (OUT) Actual number of labels returned
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_tunnel_initiator_get(int unit, bcm_if_t intf, int label_max,
                                 bcm_mpls_egress_label_t *label_array,
                                 int *label_count)
{
    egr_ip_tunnel_mpls_entry_t tnl_entry;
    egr_l3_intf_entry_t if_entry;
    int rv, i, ix, offset;
    int tnl_index = 0, mpls_index = 0;
    bcmi_egr_ip_tnl_mpls_tunnel_entry_t **tunnel_entry = egr_mpls_tnl_sw_state[unit];
    int no_of_elements;
    if ((label_array == NULL) ||
        (intf < 0) || (intf >= L3_INFO(unit)->l3_intf_table_size)) {
        return BCM_E_PARAM;
    }

    if (!BCM_L3_INTF_USED_GET(unit, intf)) {
        LOG_INFO(BSL_LS_BCM_L3,
                 (BSL_META_U(unit,
                             "L3 interface not created\n")));
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    rv = READ_EGR_L3_INTFm(unit, MEM_BLOCK_ANY, intf, &if_entry);
    if (rv < 0) {
        return rv;
    }
    mpls_index = soc_EGR_L3_INTFm_field32_get(unit, &if_entry,
                                              MPLS_TUNNEL_INDEXf);
    tnl_index = mpls_index / _BCM_MPLS_NUM_MPLS_ENTRIES_PER_INDEX(unit);

    if (!_BCM_MPLS_TNL_USED_GET(unit, mpls_index)) {
        return BCM_E_NOT_FOUND;
    }
    rv = READ_EGR_IP_TUNNEL_MPLSm(unit, MEM_BLOCK_ANY,
                                  tnl_index, &tnl_entry);
    if (rv < 0) {
        return rv;
    }
    if (soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                            ENTRY_TYPEf) != 3) {
        return BCM_E_NOT_FOUND;
    }

    *label_count = 0;
    sal_memset(label_array, 0, sizeof(bcm_mpls_egress_label_t) * label_max);
    offset = mpls_index & _BCM_MPLS_EGR_L3_INTF_MPLS_INDEX_OFFSET_MASK(unit);
    if (tunnel_entry[tnl_index]->label_entry[offset]->flags
        & BCMI_EGR_IP_TUNNEL_MPLS_START_OF_ENTRY) {

        no_of_elements =
            tunnel_entry[tnl_index]->label_entry[offset]->num_elements;
    } else {
        return BCM_E_NOT_FOUND;
    }

    if (label_max < no_of_elements) {
        *label_count = label_max;
    } else {
        *label_count = no_of_elements;
    }


    for (i = 0; i < *label_count; i++) {
        label_array[i].label =
            soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                &tnl_entry, _tnl_label_f[offset + i]);

        if (soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_exp_select_f[offset + i]) == 0x0) {
            /* Use the specified EXP, PRI and CFI */
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_PRI_SET;
            label_array[i].exp =
                soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                    &tnl_entry, _tnl_exp_f[offset + i]);
            label_array[i].pkt_pri =
                soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                    &tnl_entry, _tnl_pri_f[offset + i]);
            label_array[i].pkt_cfi =
                soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                    &tnl_entry, _tnl_cfi_f[offset + i]);
        } else if (soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                       _tnl_exp_select_f[offset]) == 0x1) {
            /* Use EXP-map for EXP, PRI and CFI */
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_EXP_REMARK;
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_PRI_REMARK;

            ix = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                     &tnl_entry, _tnl_exp_ptr_f[offset + i]);

            BCM_IF_ERROR_RETURN(
                    _egr_qos_hw_idx2id(unit, ix, &label_array[i].qos_map_id));
        } else {
            /* Use EXP from incoming label. If there is no incoming label,
             * use the specified EXP value.
             */
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_PRI_REMARK;
            label_array[i].exp =
                soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                    &tnl_entry, _tnl_exp_f[offset + i]);

            /* Use EXP-map for PRI/CFI */
            ix = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                     &tnl_entry, _tnl_exp_ptr_f[offset + i]);
            BCM_IF_ERROR_RETURN(
                    _egr_qos_hw_idx2id(unit, ix, &label_array[i].qos_map_id));
        }

        if (soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, &tnl_entry,
                                _tnl_ttl_f[offset + i])) {
            /* Use specified TTL value */
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_TTL_SET;
            label_array[i].ttl =
                soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm,
                                    &tnl_entry, _tnl_ttl_f[offset + i]);
        } else {
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT;
        }
#if defined(BCM_TOMAHAWK2_SUPPORT)  
        if (soc_feature(unit, soc_feature_mpls_ecn)) {
            int ecn_map_trust;
            int ecn_map_hw_idx;
            int ecn_map_id;
            int rv;
            ecn_map_hw_idx = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, 
                                                 &tnl_entry,
                                                 _tnl_ecn_ptr_f[offset + i]);
            rv = bcmi_ecn_map_hw_idx2id(unit, ecn_map_hw_idx, 
                                        _BCM_XGS5_MPLS_ECN_MAP_TYPE_ECN2EXP, 
                                        &ecn_map_id);  
            if (BCM_SUCCESS(rv)) {
                label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_ECN_TO_EXP_MAP;
                label_array[i].ecn_map_id = ecn_map_id;  
                ecn_map_trust = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, 
                                                    &tnl_entry,
                                                    _tnl_ecn_ptr_pri_f[offset + i]);
                if (ecn_map_trust) {
                   label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_ECN_EXP_MAP_TRUST; 
                }
            }
            ecn_map_hw_idx = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, 
                                                &tnl_entry,
                                                _tnl_int_cn_ptr_f[offset + i]);
            rv = bcmi_ecn_map_hw_idx2id(unit, ecn_map_hw_idx, 
                                        _BCM_XGS5_MPLS_ECN_MAP_TYPE_INTCN2EXP, 
                                        &ecn_map_id);  
            if (BCM_SUCCESS(rv)) {
                label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_INT_CN_TO_EXP_MAP;
                label_array[i].int_cn_map_id = ecn_map_id;  
                ecn_map_trust = soc_mem_field32_get(unit, EGR_IP_TUNNEL_MPLSm, 
                                                    &tnl_entry,
                                                    _tnl_int_cn_ptr_pri_f[offset + i]);
                if (ecn_map_trust) {
                   label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_ECN_EXP_MAP_TRUST; 
                }
            }            
        }
#endif
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      bcmi_xgs5_mpls_failover_nw_port_match_get
 * Purpose:
 *      Get match criteria of an MPLS port
 * Parameters:
 *      unit    - (IN) Device Number
 *      mpls_port - (IN) mpls port information
 *      vp  - (IN) Source Virtual Port
 *      return_ment - (OUT) matched entry
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_xgs5_mpls_failover_nw_port_match_get(int unit, bcm_mpls_port_t *mpls_port,
                                    int vp, mpls_entry_entry_t *return_ment)
{
    _bcm_tr_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    mpls_entry_entry_t ment;
    int entry_index = -1;
    int key_type = 0x00; /* mpls_key_type */
    int rv = BCM_E_NONE;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t trunk_id;
    bcm_gport_t gport_id;

    /* return not found if both inputs are invalid. */
    if ((mpls_port == NULL) && (vp == -1)) {
        return BCM_E_NOT_FOUND;
    }

    sal_memset(&ment, 0, sizeof(mpls_entry_entry_t));
    if (mpls_port != NULL) {
        if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL_PORT) {
            /* Check for Port_independent Label mapping */
            rv = bcm_tr_mpls_port_independent_range (unit, mpls_port->match_label, mpls_port->port);
            if (rv < 0) {
                return rv;
            }

            rv = _bcm_esw_gport_resolve(unit, mpls_port->port, &mod_out,
                                &port_out, &trunk_id, &gport_id);
             BCM_IF_ERROR_RETURN(rv);

            if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__Tf, 1);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__TGIDf, mpls_port->port);
            } else {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__MODULE_IDf, mod_out);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__PORT_NUMf, port_out);
            }
        } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) {
            soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__MODULE_IDf, 0);
            soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__PORT_NUMf, 0);
        } else {
            return BCM_E_NOT_FOUND;
        }

        /* Use old label to get old mpls entry for replace operation */
        soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__MPLS_LABELf,
                                    mpls_port->match_label);
        soc_MPLS_ENTRYm_field32_set(unit, &ment, VALIDf, 1);
        soc_MPLS_ENTRYm_field32_set(unit, &ment, KEY_TYPEf, key_type);
    } else {

        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
            return BCM_E_NOT_FOUND;
        }

        if ((mpls_info->match_key[vp].flags &
                _BCM_MPLS_PORT_MATCH_TYPE_LABEL) ||
               (mpls_info->match_key[vp].flags ==
                _BCM_MPLS_PORT_MATCH_TYPE_LABEL_PORT)) {

        if (mpls_info->match_key[vp].flags &
                    _BCM_MPLS_PORT_MATCH_TYPE_LABEL_PORT) {

            if (mpls_info->match_key[vp].trunk_id != -1) {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__Tf, 1);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__TGIDf,
                    mpls_info->match_key[vp].trunk_id);
            } else {
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__MODULE_IDf,
                    mpls_info->match_key[vp].modid);
                soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__PORT_NUMf,
                    mpls_info->match_key[vp].port);
            }
        }

        soc_MPLS_ENTRYm_field32_set(unit, &ment, MPLS__MPLS_LABELf,
            mpls_info->match_key[vp].match_label);
        soc_MPLS_ENTRYm_field32_set(unit, &ment, KEY_TYPEf, key_type);

        } else {
            return BCM_E_NOT_FOUND;
        }
    }

    rv = soc_mem_search(unit, MPLS_ENTRYm, MEM_BLOCK_ANY, &entry_index,
             &ment, return_ment, 0);

    return rv;
}

#endif /* (BCM_TOMAHAWK_SUPPORT) || defined(BCM_TRIDENT2PLUS_SUPPORT) */
#endif /* BCM_MPLS_SUPPORT */
#endif /* INCLUDE_L3 */

