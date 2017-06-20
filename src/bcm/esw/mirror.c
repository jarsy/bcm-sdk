/*
 * $Id: mirror.c,v 1.354 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Mirror - Broadcom StrataSwitch Mirror API.
 *
 * The mirroring code has become more complex after the introduction
 * of XGS3 devices, which support multiple MTPs (mirror-to ports) as
 * well as directed mirroring. When directed mirroring is enabled
 * it is also possible to mirror to a trunk.
 *
 * Non-directed mirroring (aka. Draco1.5-style mirroring and XGS2-style 
 * mirroring) only allows for a single MTP in a system (which can be
 * either a single device or a stack.) In order to mirror a packet to 
 * a remote module in non-directed mode, the local MTP must be the 
 * appropriate stacking port and all modules traversed from the 
 * mirrored port to the MTP need to have mirroring configured to 
 * reflect the desired path for mirror packets.
 *
 * Directed mirroring means that the MTP info includes a module ID,
 * which allows mirror packets to follow the normal path of switched
 * packets, i.e. when mirroring to a remote MTP there is no need to 
 * configure the mirror path separately.
 *
 * Since the original mirror API did not support module IDs in the MTP
 * definition, a new API was introduced to handle this. The new API is
 * called bcm_mirror_port_set/get and allows the application to 
 * configure mirroring with a single API call, whereas the the old API
 * would require two (and in most cases three or more) API calls.
 *
 * For compatibility, the original API will also work on XGS3 devices,
 * and in this case the MTP module ID is automatically set to be the
 * local module ID. Likewise, the new API will also work on pre-XGS3
 * devices as long as the MTP module ID is specified as the local
 * module ID.
 *
 * In addition to normal ingress and egress mirroring, the FP (field
 * processor) can specify actions that include ingress and egress 
 * mirroring. This feature uses the same hardware MTP resources as
 * the mirror API, so in order to coordinate this, the FP APIs must
 * allocate MTP resources through internal reserve/unreserve 
 * functions. Since multiple FP rules can use the same MTP resource
 * the reserve/unreserve functions maintain a reference count for
 * each MTP resource. In the software MTP structure this reference
 * counter is called 'reserved'. Within the same structure, the 
 * mirror API uses the 'pbmp' to indicate whether this MTP resource
 * is being used by the mirror API.
 *
 * Note that the MTP resource management code allows resources to be 
 * shared between the mirror API and the FP whenever the requested 
 * MTP is identical.
 *
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/scache.h>
#ifdef BCM_TRIDENT_SUPPORT
#include <soc/profile_mem.h>
#endif /* BCM_TRIDENT_SUPPORT */

#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm/port.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/stack.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/common/field.h>
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#include <bcm_int/esw_dispatch.h>

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
#include <bcm_int/esw/virtual.h>
#endif
#ifdef BCM_TRIUMPH2_SUPPORT
#include <bcm_int/esw/triumph2.h>
#endif
#ifdef BCM_TRIDENT_SUPPORT
#include <bcm_int/esw/trident.h>
#endif
#ifdef BCM_TRIUMPH3_SUPPORT
#include <bcm_int/esw/triumph3.h>
#endif
#ifdef BCM_TRIUMPH_SUPPORT
#include <bcm_int/esw/triumph.h>
#endif
#ifdef BCM_KATANA2_SUPPORT
#include <bcm_int/esw/katana2.h>
#endif
#ifdef BCM_MPLS_SUPPORT
#include <bcm_int/esw/mpls.h>
#endif
#ifdef BCM_HGPROXY_COE_SUPPORT
#include <bcm_int/esw/xgs5.h>
#endif /*BCM_HGPROXY_COE_SUPPORT */

/* local macros */
#define VP_PORT_INVALID  (-1)

/* STATIC FUNCTIONS DECLARATION. */
STATIC int _bcm_esw_mirror_enable(int unit);
STATIC int _bcm_esw_mirror_egress_set(int unit, bcm_port_t port, int enable);
STATIC int _bcm_esw_mirror_egress_get(int unit, bcm_port_t port, int *enable);
STATIC int _bcm_esw_directed_mirroring_get(int unit, int *enable);
STATIC int _bcm_esw_mirror_port_dest_search(int unit, bcm_port_t port,
                                            uint32 flags, bcm_gport_t mirror_dest);

/* LOCAL VARIABLES DECLARATION. */
static int _bcm_mirror_mtp_method_init[BCM_MAX_NUM_UNITS];
_bcm_mirror_config_p _bcm_mirror_config[BCM_MAX_NUM_UNITS];
static int _bcm_switch_mirror_exclusive_config[BCM_MAX_NUM_UNITS];

#ifdef BCM_TRIUMPH2_SUPPORT
static const soc_field_t _mtp_index_field[] = {
    MTP_INDEX0f, MTP_INDEX1f, MTP_INDEX2f, MTP_INDEX3f
};
static const soc_field_t _non_uc_mtp_index_field[] = {
    NON_UC_EM_MTP_INDEX0f, NON_UC_EM_MTP_INDEX1f, NON_UC_EM_MTP_INDEX2f,
    NON_UC_EM_MTP_INDEX3f
};
#endif /* BCM_TRIUMPH2_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
/* Cache of Egress Mirror Encap Profile Tables */
static soc_profile_mem_t *egr_mirror_encap_profile[BCM_MAX_NUM_UNITS] = {NULL};
#define EGR_MIRROR_ENCAP(_unit_) \
                (egr_mirror_encap_profile[_unit_])
#define EGR_MIRROR_ENCAP_PROFILE_DEFAULT  0

#define EGR_MIRROR_ENCAP_ENTRIES_CONTROL        0
#define EGR_MIRROR_ENCAP_ENTRIES_DATA_1         1
#define EGR_MIRROR_ENCAP_ENTRIES_DATA_2         2
#define EGR_MIRROR_ENCAP_ENTRIES_NUM            3

#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

#define BCM_EGR_MIRROR_ENCAP_ENTRIES_CONTROL    0
#define BCM_EGR_MIRROR_ENCAP_ENTRIES_DATA_1     1
#define BCM_EGR_MIRROR_ENCAP_ENTRIES_DATA_2     2
#define BCM_EGR_MIRROR_ENCAP_ENTRIES_NUM        3

#if defined (BCM_TOMAHAWK_SUPPORT)
/* Field callback functions for recovering Mirror Destination during warmboot */
extern int
_field_mirror_actions_recover_callback(int unit,_bcm_mirror_config_p _bcm_mirror_config);
#endif

int
_bcm_esw_mirror_flexible_get(int unit, int *enable)
{
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        *enable = (_bcm_mirror_mtp_method_init[unit] ==
                   BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE) ? TRUE : FALSE;

        return BCM_E_NONE;
    } else {
        return BCM_E_UNAVAIL;
    }
}

int
_bcm_esw_mirror_flexible_set(int unit, int enable)
{
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        if (enable) {
            _bcm_mirror_mtp_method_init[unit] =
                BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE;
        } else if (soc_feature(unit, soc_feature_directed_mirror_only)) {
            _bcm_mirror_mtp_method_init[unit] =
                BCM_MIRROR_MTP_METHOD_DIRECTED_LOCKED;
        } else {
            _bcm_mirror_mtp_method_init[unit] =
                BCM_MIRROR_MTP_METHOD_NON_DIRECTED;
        }
        return BCM_E_NONE;
    } else {
        return BCM_E_UNAVAIL;
    }
}

/*
 * Function:
 *   _bcm_esw_mirror_exclusive_get
 * Purpose:
 *     Get mirror exclusive status on the chip.
 * Parameters:
 *    unit    - (IN) BCM device number.
 *    enable  - (OUT) Mirror Exclusive Control Status.
 * Returns:
 *    BCM_E_XXX
 * Notes: Applicable for devices not supporting
 *        flexible mirroring.
 */
int
_bcm_esw_mirror_exclusive_get(int unit, int *enable)
{
    if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANEX(unit) &&
            !SOC_IS_GREYHOUND(unit) && !SOC_IS_GREYHOUND2(unit) &&
            !soc_feature(unit, soc_feature_mirror_flexible)) {
        *enable = _bcm_switch_mirror_exclusive_config[unit];
        return BCM_E_NONE;
    }
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *   _bcm_esw_mirror_exclusive_set
 * Purpose:
 *     Set mirror exclusive control on the chip.
 * Parameters:
 *    unit    - (IN) BCM device number.
 *    enable  - (IN) Mirror Exclusive Control Status.
 * Returns:
 *    BCM_E_XXX
 * Notes: Applicable for devices not supporting
 *        flexible mirroring.
 */
int
_bcm_esw_mirror_exclusive_set(int unit, int enable)
{
    if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANEX(unit) &&
            !SOC_IS_GREYHOUND(unit) && !SOC_IS_GREYHOUND2(unit) &&
            !soc_feature(unit, soc_feature_mirror_flexible)) {
        if (enable) {
            _bcm_switch_mirror_exclusive_config[unit] =
                _BCM_SWITCH_MIRROR_EXCLUSIVE;
        } else {
            _bcm_switch_mirror_exclusive_config[unit] =
                _BCM_SWITCH_MIRROR_NON_EXCLUSIVE;
        }
        return BCM_E_NONE;
    }
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *	  _bcm_esw_local_modid_get
 * Purpose:
 *	  Get local unit module id. 
 * Parameters:
 *    unit    - (IN) BCM device number.
 *    modid   - (OUT)module id. 
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int
_bcm_esw_local_modid_get(int unit, int *modid)
{
    int  rv;      /* Operation return status. */

    /* Input parameters check. */
    if (NULL == modid) {
        return (BCM_E_PARAM);
    }

    /* Get local module id. */
    rv = bcm_esw_stk_my_modid_get(unit, modid);
    if ((BCM_E_UNAVAIL == rv) || (*modid < 0) ){
        *modid = 0;
        rv = (BCM_E_NONE);
    }
    return (rv);
}

/*
 * Function:
 *      _bcm_mirror_gport_adapt
 * Description:
 *      Adapts gport encoding for dual mode devices
 * Parameters:
 *      unit        - BCM device number
 *      gport       (IN/OUT)- gport to adapt
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_mirror_gport_adapt(int unit, bcm_gport_t *gport)
{
    bcm_module_t    modid;
    bcm_port_t      port;
    bcm_trunk_t     tgid;
    int             id;
    bcm_gport_t     gport_out;
    _bcm_gport_dest_t gport_st;

    if (NULL == gport) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, *gport, &modid, 
                               &port, &tgid, &id));

    if (soc_feature(unit, soc_feature_hgproxy_subtag_coe)) {
#if defined(BCM_HGPROXY_COE_SUPPORT)
        if(BCM_GPORT_IS_SET(*gport) &&
		   _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, *gport) &&
           _bcm_xgs5_subport_coe_gport_local(unit, *gport)) {
            gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
            gport_st.modid = modid;
            gport_st.port = port;
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_construct(unit, &gport_st, &gport_out));
            *gport = gport_out;
        }
#endif
    } else if (soc_feature(unit, soc_feature_linkphy_coe) ||
               soc_feature(unit, soc_feature_subtag_coe)) {
#if defined(BCM_KATANA2_SUPPORT)
        if (BCM_GPORT_IS_SET(*gport) &&
            _BCM_KT2_GPORT_IS_LINKPHY_OR_SUBTAG_SUBPORT_GPORT (unit, *gport)) {
            if (_bcm_kt2_mod_is_coe_mod_check(unit, modid) == BCM_E_NONE) {
                gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
                gport_st.modid = modid;
                gport_st.port = port;
                BCM_IF_ERROR_RETURN(
                    _bcm_esw_gport_construct(unit, &gport_st, &gport_out));
                *gport = gport_out;
            }
        }
#endif
    }

    /* Adaptation is needed only for dual mod devices */
    if ((NUM_MODID(unit) > 1)) {

        if (-1 != id) {
            return BCM_E_PARAM;
        }
    
        if (BCM_TRUNK_INVALID != tgid) {
            gport_st.gport_type = BCM_GPORT_TYPE_TRUNK;
            gport_st.tgid = tgid;
        } else {
            gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
            gport_st.modid = modid;
            gport_st.port = port;
        }
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_construct(unit, &gport_st, &gport_out));
    
        *gport = gport_out;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_mirror_gport_resolve
 * Description:
 *      Resolves gport for mirror module for local ports
 * Parameters:
 *      unit        - (IN) BCM device number
 *      gport       - (IN)- gport to to resolve
 *      port        - (OUT)- port encoded to gport
 *      modid       - (OUT)- modid encoded to gport
 * Returns:
 *      BCM_E_XXX
 * Note :
 *      if modid == NULL port must be local port
 */
STATIC int 
_bcm_mirror_gport_resolve(int unit, bcm_gport_t gport, bcm_port_t *port, 
                          bcm_module_t *modid)
{
    bcm_module_t    lmodid;
    bcm_trunk_t     tgid;
    bcm_port_t      lport;
    int             id, islocal;

    if (NULL == port) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, gport, &lmodid, &lport, &tgid, &id));

#if defined(BCM_HGPROXY_COE_SUPPORT)
    if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
		BCM_GPORT_IS_SET(gport) &&
        _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, gport)) {
    } else
#endif

#if defined(BCM_KATANA2_SUPPORT)
    if (BCM_GPORT_IS_SET(gport) &&
        _BCM_KT2_GPORT_IS_LINKPHY_OR_SUBTAG_SUBPORT_GPORT (unit, gport)) {
    } else
#endif
    {
        if ((-1 != id) || (BCM_TRUNK_INVALID != tgid)) {
            return BCM_E_PARAM;
        }
    }

    if (NULL == modid) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_modid_is_local(unit, lmodid, &islocal));
        if (islocal != TRUE) {
            return BCM_E_PARAM;
        }
    } else {
        *modid = lmodid;
    }
    *port = lport;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_mirror_gport_construct
 * Description:
 *      Constructs gport for mirror module
 * Parameters:
 *      unit        - (IN) BCM device number
 *      port_tgid   - (IN) port or trunk id to construct into a gprot
 *      modid       - (IN) module id to construct into a gport
 *      flags       - (IN) Mirror trunk flag
 *      gport       - (OUT)- gport to to construct
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_mirror_gport_construct(int unit, int port_tgid, int modid, uint32 flags, 
                            bcm_gport_t *gport)
{
    _bcm_gport_dest_t   dest;
    bcm_module_t        mymodid;
    int                 rv;

    _bcm_gport_dest_t_init(&dest);
    if (flags & BCM_MIRROR_PORT_DEST_TRUNK) {
        dest.tgid = port_tgid;
        dest.gport_type = _SHR_GPORT_TYPE_TRUNK;
    } else {
        dest.port = port_tgid;
        if (IS_ST_PORT(unit, port_tgid)) {
            rv = bcm_esw_stk_my_modid_get(unit, &mymodid);
            if (BCM_E_UNAVAIL == rv) {
                dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
            } else {
                dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
                dest.modid = modid;
            }
        } else {
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
            dest.modid = modid;
        }
    }
    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_construct(unit, &dest, gport));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_mirror_destination_gport_parse
 * Description:
 *      Parse mirror destinations gport.
 * Parameters:
 *      unit      - BCM device number
 *      mirror_dest_id - mirror destination id. 
 *      dest_mod  - (OUT) module id of mirror-to port
 *      dest_port - (OUT) mirror-to port
 *      flags     - (OUT) Trunk flag
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_mirror_destination_gport_parse(int unit, bcm_gport_t mirror_dest_id,
                                    bcm_module_t *dest_mod, bcm_port_t *dest_port,
                                    uint32 *flags)
{
    bcm_mirror_destination_t mirror_dest;
    bcm_module_t             modid;
    bcm_port_t               port;
    bcm_trunk_t              tgid;
    int                      id;


    BCM_IF_ERROR_RETURN
        (bcm_esw_mirror_destination_get(unit, mirror_dest_id, &mirror_dest));

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, mirror_dest.gport, &modid, &port, 
                               &tgid, &id));

#if defined(BCM_HGPROXY_COE_SUPPORT)
    if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
         BCM_GPORT_IS_SET( mirror_dest.gport) &&
        _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT (unit,
            mirror_dest.gport)) {
        id = -1;
    } else
#endif

#if defined(BCM_KATANA2_SUPPORT)
    if (BCM_GPORT_IS_SET( mirror_dest.gport) &&
        _BCM_KT2_GPORT_IS_LINKPHY_OR_SUBTAG_SUBPORT_GPORT (unit,
            mirror_dest.gport)) {
        id = -1;
    }
#endif

    if (-1 != id) {
        return BCM_E_PARAM;
    }

    if (BCM_TRUNK_INVALID != tgid) {
        if (NULL != dest_mod) {
            *dest_mod  = -1;
        }
        if (NULL != dest_port) { 
            *dest_port = tgid;
        }
        if (NULL != flags) {
            *flags |= BCM_MIRROR_PORT_DEST_TRUNK;
        }
    } else {
        if (NULL != dest_mod) {
            *dest_mod = modid;
        }
        if (NULL != dest_port) {
            *dest_port = port;
        }
    }

    return (BCM_E_NONE);
}

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
/*
 * Function:
 *      _bcm_egr_mirror_encap_entry_add
 * Purpose:
 *      Internal function for adding an entry to the EGR_MIRROR_ENCAP_* tables
 *      Adds an entry to the global shared SW copy of the EGR_MIRROR_ENCAP_* 
 *      tables
 * Parameters:
 *      unit    -  (IN) Device number.
 *      entries -  (IN) Pointer to EGR_MIRROR_ENCAP_* entries array
 *      index   -  (OUT) Base index for the entires allocated in HW
 * Returns:
 *      BCM_X_XXX
 */
int
_bcm_egr_mirror_encap_entry_add(int unit, void **entries, uint32 *index)
{
    return soc_profile_mem_add(unit, EGR_MIRROR_ENCAP(unit), entries,
                             1, index);
}

/*
 * Function:
 *      _bcm_egr_mirror_encap_entry_delete
 * Purpose:
 *      Internal function for deleting an entry from the EGR_MIRROR_ENCAP_*
 *      tables
 *      Deletes an entry from the global shared SW copy of the
 *      EGR_MIRROR_ENCAP_* tables
 * Parameters:
 *      unit    -  (IN) Device number.
 *      index   -  (OUT) Base index for the entires allocated in HW
 * Returns:
 *      BCM_X_XXX
 */
int
_bcm_egr_mirror_encap_entry_delete(int unit, uint32 index) 
{
    return soc_profile_mem_delete(unit, EGR_MIRROR_ENCAP(unit), index);
}

/*
 * Function:
 *      _bcm_egr_mirror_encap_entry_reference
 * Purpose:
 *      Internal function for indicating that an entry in EGR_MIRROR_ENCAP_*
 *      tables is being used. Updates the global shared SW copy.
 * Parameters:
 *      unit    -  (IN) Device number
 *      index   -  (IN) Base index for the entry to be updated
 * Returns:
 *      BCM_X_XXX
 */
int
_bcm_egr_mirror_encap_entry_reference(int unit, uint32 index) 
{
    return soc_profile_mem_reference(unit, EGR_MIRROR_ENCAP(unit),
                                     (int) index, 1);
}

/*
 * Function:
 *      _bcm_egr_mirror_encap_entry_ref_get
 * Purpose:
 *      Get the reference count of the mirror encap entry at the specified index.
 * Parameters:
 *      unit    -  (IN) Device number
 *      index   -  (IN) Base index for the entry to get
 *      ref_count   - (OUT) Reference count
 * Returns:
 *      BCM_X_XXX
 */
int
_bcm_egr_mirror_encap_entry_ref_get(int unit, uint32 index,
                                    int *ref_count)
{
    return soc_profile_mem_ref_count_get(unit, EGR_MIRROR_ENCAP(unit),
                                         (int)index, ref_count);
}

/*
 * Function:
 *      _bcm_egr_mirror_encap_entry_mtp_update
 * Purpose:
 *      Internal function for recording updating the EGR_*_MTP_INDEX
 *      tables when ERSPAN is activated.
 * Parameters:
 *      unit    -  (IN) Device number
 *      index   -  (IN) MTP index for the entries to be updated
 *      profile_index   -  (IN) Encap profile index
 *      flags   -  (IN) Mirror direction flags.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_egr_mirror_encap_entry_mtp_update(int unit, int index,
                                       uint32 profile_index, int flags) 
{
    int                         offset;
    int                         idx;
    int refs = 0;
    int max_num_trunk_ports = 0;

    refs = 0;
#ifdef BCM_METROLITE_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
        max_num_trunk_ports = 4;
    } else
#endif
      max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;

    offset = index * max_num_trunk_ports;
    for (idx = 0; idx < max_num_trunk_ports; idx++, offset++) {
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            BCM_IF_ERROR_RETURN
                (soc_mem_field32_modify(unit, EGR_IM_MTP_INDEXm,
                                        offset, MIRROR_ENCAP_INDEXf,
                                        profile_index));
            if (0 == idx) {
                refs++;
            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS) {
#ifdef BCM_TOMAHAWK_SUPPORT
            if (SOC_INFO(unit).th_tflow_enabled == 1) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_field32_modify(unit, EGR_IM_MTP_INDEXm,
                                            offset, MIRROR_ENCAP_INDEXf,
                                            profile_index));
            } else
#endif /* BCM_TOMAHAWK_SUPPORT */
            {
                BCM_IF_ERROR_RETURN
                    (soc_mem_field32_modify(unit, EGR_EM_MTP_INDEXm,
                                            offset, MIRROR_ENCAP_INDEXf,
                                            profile_index));
            }
            if (0 == idx) {
                refs++;
            }
        }

        if (soc_feature(unit, soc_feature_egr_mirror_true) &&
            (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_field32_modify(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                                        offset, MIRROR_ENCAP_INDEXf,
                                        profile_index));
            if (0 == idx) {
                refs++;
            }
        }
    }

    if (refs > 1) {
        /* We should never have more than one mirror direction flag set. */
        return BCM_E_INTERNAL;
    }

    return (BCM_E_NONE);
}

STATIC int
_bcm_egr_mirror_encap_entry_mtp_enable(int unit, int index,
                              bcm_gport_t *trunk_arr, int flags)
{
    bcm_port_t                port_out;
    bcm_module_t              mod_out;
    _bcm_mtp_config_p         mtp_cfg;
    int                       offset;
    int                       idx, id;
    bcm_trunk_t               trunk = BCM_TRUNK_INVALID;
    bcm_module_t              modid = 0;
    bcm_port_t                port = -1;
    int                       max_num_trunk_ports = 0;

    /* Input parameters check */
    if (NULL == trunk_arr) {
        return (BCM_E_PARAM);
    }

    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

#ifdef BCM_METROLITE_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
        max_num_trunk_ports = 4;
    } else
#endif
    max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;

    offset = index * max_num_trunk_ports;

    for (idx = 0; idx < max_num_trunk_ports; idx++, offset++) {

        BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(
                            unit, trunk_arr[idx],&modid,&port,&trunk, &id));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, modid, port,
                                    &mod_out, &port_out));

        /* HW write. based on mirrored traffic direction. */
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            if ((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
                & (BCM_MIRROR_DEST_TUNNELS)) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_field32_modify(unit, EGR_IM_MTP_INDEXm, offset,
                                                  MIRROR_ENCAP_ENABLEf, 1));
            }
        }
        /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
        if (flags & BCM_MIRROR_PORT_EGRESS) {
#ifdef BCM_TOMAHAWK_SUPPORT
            if (SOC_INFO(unit).th_tflow_enabled == 1) {
                /* Enable encap only for sFlow tunnel header on LB port */
                if (BCM_PBMP_MEMBER(PBMP_LB(unit), port) ||
                   ((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
                    & (BCM_MIRROR_DEST_TUNNELS))) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_field32_modify(unit, EGR_IM_MTP_INDEXm, offset,
                                                      MIRROR_ENCAP_ENABLEf, 1));
                }

            } else
#endif /* BCM_TOMAHAWK_SUPPORT */
            {
                if ((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
                    & (BCM_MIRROR_DEST_TUNNELS)) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_field32_modify(unit, EGR_EM_MTP_INDEXm, offset,
                                                      MIRROR_ENCAP_ENABLEf, 1));
                }
            }
        }

        /* EGR_EP_REDIRECT_EM_MTP_INDEX has same layout as
         * EGR_IM_MTP_INDEX */
        if (soc_feature(unit, soc_feature_egr_mirror_true) &&
            (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
            if ((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
                & (BCM_MIRROR_DEST_TUNNELS)) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_field32_modify(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                                               offset, MIRROR_ENCAP_ENABLEf, 1));
            }
        }
    }

    return BCM_E_NONE;
}

#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

/*
 * Function:
 *      _bcm_mirror_destination_match
 * Description:
 *      Limited match utility used to identify mirror destination
 *      with identical gport. 
 * Parameters:
 *      unit           - (IN) BCM device number
 *      mirror_dest    - (IN) Mirror destination. 
 *      mirror_dest_id - (OUT)Matching mirror destination id. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_mirror_destination_match(int unit, bcm_mirror_destination_t *mirror_dest,
                              bcm_gport_t *mirror_dest_id) 
                        
{
    int idx;                         /* Mirror destinations iteration index.*/
    _bcm_mirror_dest_config_p  mdest;/* Mirror destination description.     */
    bcm_module_t mymodid;            /* Local module id.              */
    int          isLocal;            /* Local modid indicator */
    bcm_module_t dest_mod;           /* Destination module id.        */
    bcm_port_t   dest_port;          /* Destination port number.      */
    _bcm_gport_dest_t gport_st;      /* Structure to construct a GPORT */
    int is_local_subport = FALSE;

    /* Input parameters check. */
    if ((NULL == mirror_dest_id) || (NULL == mirror_dest)) {
        return (BCM_E_PARAM);
    }

    /* Get local modid. */
    BCM_IF_ERROR_RETURN(_bcm_esw_local_modid_get(unit, &mymodid));

    /* Directed  mirroring support check. */
    if (MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit) && !SOC_WARM_BOOT(unit)){
        /* NB:  If Warm Boot, then the ports should already be mapped. */
        /* Set mirror destination to outgoing port on local module. */
        dest_mod = BCM_GPORT_MODPORT_MODID_GET(mirror_dest->gport);
        if (soc_feature(unit, soc_feature_hgproxy_subtag_coe)) {
#if defined(BCM_HGPROXY_COE_SUPPORT)
            dest_port = BCM_GPORT_MODPORT_PORT_GET(mirror_dest->gport);
            if(_bcm_xgs5_subport_coe_mod_port_local(unit, dest_mod, dest_port)) {
                is_local_subport = TRUE;
            }
#endif
       } else
        if (soc_feature(unit, soc_feature_linkphy_coe) ||
            soc_feature(unit, soc_feature_subtag_coe)) {
#if defined(BCM_KATANA2_SUPPORT)
            dest_port = BCM_GPORT_MODPORT_PORT_GET(mirror_dest->gport);
            BCM_IF_ERROR_RETURN(
                _bcm_kt2_modport_is_local_coe_subport (unit, dest_mod,
                    dest_port, &is_local_subport));
#endif
        }

        BCM_IF_ERROR_RETURN(_bcm_esw_modid_is_local(unit, dest_mod, &isLocal));
        if ((FALSE == isLocal) && (FALSE == is_local_subport)) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_topo_port_get(unit, dest_mod, &dest_port));
            gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
            gport_st.modid = mymodid;
            gport_st.port = dest_port;
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_construct(unit,&gport_st, 
                                         &(mirror_dest->gport)));
        }
    }

    /* Find unused mirror destination & allocate it. */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
        mdest = MIRROR_CONFIG(unit)->dest_arr + idx;
        /* Skip unused entries. */
        if (0 == mdest->ref_count) {
            continue;
        }

        /* Skip tunnel destinations. */
        if (mdest->mirror_dest.flags & BCM_MIRROR_DEST_TUNNELS) { 
            continue;
        }

        if (mdest->mirror_dest.gport == mirror_dest->gport) {
            /* Matching mirror destination found. */
            *mirror_dest_id = mdest->mirror_dest.mirror_dest_id;
            return (BCM_E_NONE);
        }
    }
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *      _bcm_tr2_mirror_shared_mtp_match
 * Description:
 *      Match a mirror-to port with one of the mtp indexes.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      dest_port   - (IN)  Mirror destination gport.
 *      egress      - (IN) Egress/Ingress indication
 *      match_idx   - (OUT) MTP index matching destination. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_shared_mtp_match(int unit, bcm_gport_t gport, 
                                 int egress, int *match_idx)
{
    int idx;                                 /* Mtp iteration index. */

    /* Input parameters check. */
    if (NULL == match_idx) {
        return (BCM_E_PARAM);
    }

    /* Look for existing MTP in use */
    for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
        if (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)) {
            if ((gport == MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx)) &&
                egress == MIRROR_CONFIG_SHARED_MTP(unit, idx).egress) {
                *match_idx = idx;
                return (BCM_E_NONE);
            }
        } 
    }
    return (BCM_E_NOT_FOUND);
}


/*
 * Function:
 *      _bcm_esw_mirror_ingress_mtp_match
 * Description:
 *      Match a mirror-to port with one of the mtp indexes.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      dest_port   - (IN)  Mirror destination gport.
 *      match_idx   - (OUT) MTP index matching destination. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_ingress_mtp_match(int unit, bcm_gport_t gport, int *match_idx)
{
    int idx;                                 /* Mtp iteration index. */

    /* Input parameters check. */
    if (NULL == match_idx) {
        return (BCM_E_PARAM);
    }

    /* Look for existing MTP in use */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
        if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)) {
            if (gport == MIRROR_CONFIG_ING_MTP_DEST(unit, idx)) {
                *match_idx = idx;
                return (BCM_E_NONE);
            }
        } 
    }
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *      _bcm_esw_mirror_egress_mtp_match
 * Description:
 *      Match a mirror-to port with one of the mtp indexes.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      dest_port   - (IN)  Mirror destination gport.
 *      match_idx   - (OUT) MTP index matching destination. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_egress_mtp_match(int unit, bcm_gport_t gport, int *match_idx)
{
    int idx;                                 /* Mtp iteration index. */

    /* Input parameters check. */
    if (NULL == match_idx) {
        return (BCM_E_PARAM);
    }

    /* Look for existing MTP in use */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
        if (MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)) {
            if (gport == MIRROR_CONFIG_EGR_MTP_DEST(unit, idx)) {
                *match_idx = idx;
                return (BCM_E_NONE);
            }
        }
    }
    return (BCM_E_NOT_FOUND);
}

#ifdef BCM_TRIUMPH2_SUPPORT
/*
 * Function:
 *      _bcm_esw_mirror_egress_true_mtp_match
 * Description:
 *      Match a mirror-to port with one of the mtp indexes.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      dest_port   - (IN)  Mirror destination gport.
 *      match_idx   - (OUT) MTP index matching destination. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_egress_true_mtp_match(int unit, bcm_gport_t gport,
                                      int *match_idx)
{
    int idx;                                 /* Mtp iteration index. */

    /* Input parameters check. */
    if (NULL == match_idx) {
        return (BCM_E_PARAM);
    }

    /* Look for existing MTP in use */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
        if (MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)) {
            if (gport == MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx)) {
                *match_idx = idx;
                return (BCM_E_NONE);
            }
        }
    }

    if (SOC_IS_APOLLO(unit) && 
        (MIRROR_CONFIG(unit)->egr_true_mtp_count == 0) && 
        soc_feature(unit, soc_feature_ipfix_flow_mirror)) {
        idx = 0;

        if (MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)) {
            if (gport == MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx)) {
                *match_idx = idx;
                return (BCM_E_NONE);
            }
        }
    }
    return (BCM_E_NOT_FOUND);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      _bcm_esw_mirror_mtp_match
 * Description:
 *      Match a mirror-to port with one of the mtp indexes.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      gport       - (IN)  Mirror destination gport.
 *      match_idx   - (OUT) MTP index matching destination.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_mirror_mtp_match(int unit, bcm_gport_t gport, uint32 flags,
                          int *match_idx)
{
    if (!soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return (BCM_E_PARAM);
    }

    if (!(flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS |
                   BCM_MIRROR_PORT_EGRESS_TRUE))) {
        return (BCM_E_PARAM);
    }

    if (flags & BCM_MIRROR_PORT_INGRESS) {
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            return _bcm_tr2_mirror_shared_mtp_match(unit, gport, FALSE,
                                              match_idx);
        } else {
            return _bcm_esw_mirror_ingress_mtp_match(unit, gport, match_idx);
        }
    }

    if (flags & BCM_MIRROR_PORT_EGRESS) {
        return _bcm_esw_mirror_egress_mtp_match(unit, gport, match_idx);
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        return _bcm_esw_mirror_egress_true_mtp_match(unit, gport, match_idx);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_tr2_mirror_vp_port_get
 * Description:
 *      Get the virtual port number and the ingress gport associated
 *      with the vp.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      gport       - (IN)  vp gport
 *      vp_out      - (OUT) virtual port number
 *      port_out    - (OUT) ingress gport associated with the vp
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_vp_port_get(int unit, bcm_gport_t gport,
                                      int *vp_out, int *port_out)
{
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    int vp;
    bcm_gport_t phy_port_trunk;

    if (BCM_GPORT_IS_VP_GROUP(gport)) {
        vp = BCM_GPORT_VP_GROUP_GET(gport);
        if (_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
            BCM_GPORT_VLAN_PORT_ID_SET(gport, vp);
        } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeNiv)) {
            BCM_GPORT_NIV_PORT_ID_SET(gport, vp);
        } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeExtender)) {
            BCM_GPORT_EXTENDER_PORT_ID_SET(gport, vp);
        } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
            BCM_GPORT_MPLS_PORT_ID_SET(gport, vp);
        } else {
            return BCM_E_INTERNAL;
        }
    }
    
    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        bcm_vlan_port_t vlan_vp;

        vp = BCM_GPORT_VLAN_PORT_ID_GET(gport);
        /* Get the physical port or trunk the VP resides on */
        bcm_vlan_port_t_init(&vlan_vp);
        vlan_vp.vlan_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_tr2_vlan_vp_find(unit, &vlan_vp));
        phy_port_trunk = vlan_vp.port;
        *port_out = phy_port_trunk;
        *vp_out = vp;
        return BCM_E_NONE;
    } else
#ifdef BCM_TRIDENT_SUPPORT
    if (BCM_GPORT_IS_NIV_PORT(gport)) {
        bcm_niv_port_t   niv_port;
        bcm_niv_egress_t niv_egress;
        int              rv = BCM_E_NONE;
        int              count;

        vp = BCM_GPORT_NIV_PORT_ID_GET(gport);

        /* Get the physical port or trunk the VP resides on */
        bcm_niv_port_t_init(&niv_port);
        niv_port.niv_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_trident_niv_port_get(unit, &niv_port));
        if (niv_port.flags & BCM_NIV_PORT_MATCH_NONE) {
            bcm_niv_egress_t_init(&niv_egress);
            rv = bcm_trident_niv_egress_get(unit, niv_port.niv_port_id,
                                            1, &niv_egress, &count);
            if (BCM_FAILURE(rv)) {
                return BCM_E_PARAM;
            }

            if (niv_egress.flags & BCM_NIV_EGRESS_MULTICAST) {
                return BCM_E_PARAM;
            } else {
                phy_port_trunk = niv_egress.port;
            }
        } else {
            phy_port_trunk = niv_port.port;
        }
        *port_out = phy_port_trunk;
        *vp_out = vp;
        return BCM_E_NONE;
    } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH3_SUPPORT
    if (BCM_GPORT_IS_EXTENDER_PORT(gport)) {
        bcm_extender_port_t extender_port;
        bcm_extender_egress_t extender_egress;
        int count;
        vp = BCM_GPORT_EXTENDER_PORT_ID_GET(gport);

        /* Get the physical port or trunk the VP resides on */
        bcm_extender_port_t_init(&extender_port);
        extender_port.extender_port_id = gport;
        BCM_IF_ERROR_RETURN(bcm_tr3_extender_port_get(unit, &extender_port));
        if (extender_port.flags & BCM_EXTENDER_PORT_MATCH_NONE) {
            bcm_extender_egress_t_init(&extender_egress);
            BCM_IF_ERROR_RETURN(bcm_tr3_extender_egress_get_all(unit,
                        extender_port.extender_port_id, 1, &extender_egress, &count));
            if (count == 0) {
                /* No Extender egress object has been added to VP yet. */
                return BCM_E_CONFIG;
            }
            if (extender_egress.flags & BCM_EXTENDER_EGRESS_MULTICAST) {
                return BCM_E_PARAM;
            }
            phy_port_trunk = extender_egress.port;
        } else {
            phy_port_trunk = extender_port.port;
        }
        *port_out = phy_port_trunk;
        *vp_out = vp;
        return BCM_E_NONE;
    } else
#endif /* BCM_TRIUMPH3_SUPPORT */
#ifdef BCM_MPLS_SUPPORT
    if (BCM_GPORT_IS_MPLS_PORT(gport)) {
        bcm_mpls_port_t mpls_port;
        int vpn0;
 
        vp = BCM_GPORT_MPLS_PORT_ID_GET(gport);
        bcm_mpls_port_t_init(&mpls_port);
        mpls_port.mpls_port_id = gport;
        _BCM_MPLS_VPN_SET(vpn0,_BCM_MPLS_VPN_TYPE_VPWS,0);
        BCM_IF_ERROR_RETURN(bcm_tr_mpls_port_get(unit, 
                      vpn0, &mpls_port));
        phy_port_trunk = mpls_port.port;
        *port_out = phy_port_trunk;
        *vp_out = vp;
        return BCM_E_NONE;
    } else
#endif  /* BCM_MPLS_SUPPORT */
    {}

#endif  /* defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3) */
    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_tr2_mirror_svp_enable_get
 * Description:
 *      check if the ingress mirroring is enabled on the given vp.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      vp          - (IN)  virtual port number 
 *      enable      - (OUT) enable status 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_svp_enable_get(int unit, int vp,
                                       int *enable)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        source_vp_entry_t svp_entry;

        sal_memset(&svp_entry, 0, sizeof(source_vp_entry_t));
        BCM_IF_ERROR_RETURN(READ_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, 
                                &svp_entry));
        *enable = soc_SOURCE_VPm_field32_get(unit, &svp_entry, 
                                ING_MIRROR_ENABLEf);
        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_tr2_mirror_svp_enable_set
 * Description:
 *      Enable the ingress mirroring on the given vp.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      vp          - (IN)  virtual port number 
 *      enable      - (IN)  enable the given MTP 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_svp_enable_set(int unit, int vp,
                                       int enable)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        source_vp_entry_t svp_entry;

        sal_memset(&svp_entry, 0, sizeof(source_vp_entry_t));
        BCM_IF_ERROR_RETURN(READ_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, 
                      &svp_entry));
        soc_SOURCE_VPm_field32_set(unit, &svp_entry, 
                      ING_MIRROR_ENABLEf, enable);
        BCM_IF_ERROR_RETURN(WRITE_SOURCE_VPm(unit, 
                      MEM_BLOCK_ALL, vp, &svp_entry));
        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_tr2_mirror_dvp_enable_get
 * Description:
 *      check if the egress mirroring is enabled on the given vp.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      vp          - (IN)  virtual port number
 *      enable      - (OUT) enable status 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_dvp_enable_get(int unit, int vp,
                                       int *enable)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        soc_mem_t mem;
        void *entry;
        ing_dvp_table_entry_t dvp_entry;
        ing_dvp_2_table_entry_t dvp_2_entry;

        if (soc_feature(unit, soc_feature_td2p_dvp_mirroring)) {
            mem = ING_DVP_2_TABLEm;
            sal_memset(&dvp_2_entry, 0, sizeof(ing_dvp_2_table_entry_t));
            entry = &dvp_2_entry;
        } else {
            mem = ING_DVP_TABLEm;
            sal_memset(&dvp_entry, 0, sizeof(ing_dvp_table_entry_t));
            entry = &dvp_entry;
        }

        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, mem, MEM_BLOCK_ALL, vp, entry));
        *enable = soc_mem_field32_get(unit, mem, entry,
                EGR_MIRROR_ENABLEf);

        return BCM_E_NONE;
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_tr2_mirror_dvp_enable_set
 * Description:
 *      enable the egress mirroring on the given vp.
 * Parameters:
 *      unit        - (IN)  BCM device number
 *      gport       - (IN)  virtual port number 
 *      enable      - (IN)  enable the given MTP
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_dvp_enable_set(int unit, int vp,
                                       int enable)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        soc_mem_t mem;
        void *entry;
        ing_dvp_table_entry_t dvp_entry;
        ing_dvp_2_table_entry_t dvp_2_entry;

        if (soc_feature(unit, soc_feature_td2p_dvp_mirroring)) {
            mem = ING_DVP_2_TABLEm;
            sal_memset(&dvp_2_entry, 0, sizeof(ing_dvp_2_table_entry_t));
            entry = &dvp_2_entry;
        } else {
            mem = ING_DVP_TABLEm;
            sal_memset(&dvp_entry, 0, sizeof(ing_dvp_table_entry_t));
            entry = &dvp_entry;
        }

        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, mem, MEM_BLOCK_ALL, vp, entry));
        soc_mem_field32_set(unit, mem, entry, EGR_MIRROR_ENABLEf, enable);
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, mem, MEM_BLOCK_ALL, vp, entry));

        return BCM_E_NONE;
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *	  _bcm_esw_mirror_destination_find
 * Purpose:
 *	  Find mirror destination for all gport types. 
 * Parameters:
 *    unit    - (IN) BCM device number.
 *    port    - (IN) port, gport or mirror gport
 *    modid   - (IN) module id.
 *    flags   - (IN) BCM_MIRROR_PORT_DEST_* flags 
 *    mirror_dest - (OUT) mirror destination 
 * Returns:
 *	  BCM_E_XXX
 */

STATIC int 
_bcm_esw_mirror_destination_find(int unit, bcm_port_t port, bcm_module_t modid, 
                                 uint32 flags, bcm_mirror_destination_t *mirror_dest)
{
    if (NULL == mirror_dest) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(port)) {
        /* If gport passed, work with it directly */
        mirror_dest->gport = port;
    } else {
        _bcm_gport_dest_t gport_st;      /* Structure to construct a GPORT */

        /* If not gport then construct the gport from given parameters.*/
        if (flags & BCM_MIRROR_PORT_DEST_TRUNK) {
            /* Mirror destination is a trunk. */
            gport_st.gport_type = BCM_GPORT_TYPE_TRUNK;
            gport_st.tgid = port;
        } else {
            /* Convert port + mod to GPORT format. No trunking destination support. */
            if (-1 == modid) { 
                /* Get local modid. */
                BCM_IF_ERROR_RETURN(
                    _bcm_esw_local_modid_get(unit, &modid));
            } else if (!SOC_MODID_ADDRESSABLE(unit, modid)){
                return BCM_E_PARAM;
            }

            gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
            gport_st.modid = modid;
            gport_st.port = port;
        }
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_construct(unit, &gport_st, &(mirror_dest->gport)));
    }

    /* Adapt miror destination gport */
    BCM_IF_ERROR_RETURN(
        _bcm_mirror_gport_adapt(unit, &(mirror_dest->gport)));

    /* Find matching mirror destination */
    BCM_IF_ERROR_RETURN(
        _bcm_mirror_destination_match(unit, mirror_dest,
                                      &(mirror_dest->mirror_dest_id)));
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _bcm_mirror_destination_alloc
 * Purpose:
 *     Allocate mirror destination description.
 * Parameters:
 *      unit           - (IN) BCM device number. 
 *      mirror_dest_id - (OUT) Mirror destination id.
 * Returns:
 *      BCM_X_XXX
 */

/* Create a mirror (destination, encapsulation) pair. */
STATIC int 
_bcm_mirror_destination_alloc(int unit, bcm_gport_t *mirror_dest_id) 
{
    int idx;                          /* Mirror destinations iteration index.*/
    _bcm_mirror_dest_config_p  mdest; /* Mirror destination description.     */

    /* Input parameters check. */
    if (NULL == mirror_dest_id) {
        return (BCM_E_PARAM);
    }

    /* Find unused mirror destination & allocate it. */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
        mdest = MIRROR_CONFIG(unit)->dest_arr + idx;
        if (mdest->ref_count) {
            continue;
        }
        mdest->ref_count++;
        *mirror_dest_id = mdest->mirror_dest.mirror_dest_id;
        return (BCM_E_NONE);
    }

    /* All mirror destinations are used. */
    return (BCM_E_RESOURCE);
}

/*
 * Function:
 *     _bcm_mirror_destination_free
 * Purpose:
 *     Free mirror destination description.
 * Parameters:
 *      unit           - (IN) BCM device number. 
 *      mirror_dest_id - (IN) Mirror destination id.
 * Returns:
 *      BCM_X_XXX
 */

/* Create a mirror (destination, encapsulation) pair. */
STATIC int 
_bcm_mirror_destination_free(int unit, bcm_gport_t mirror_dest_id) 
{
    _bcm_mirror_dest_config_p  mdest_cfg; /* Mirror destination config.*/

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id); 

    if (mdest_cfg->ref_count > 0) {
        mdest_cfg->ref_count--;

        if (0 == mdest_cfg->ref_count) {
            sal_memset(&mdest_cfg->mirror_dest, 0,
                       sizeof(bcm_mirror_destination_t));
            mdest_cfg->mirror_dest.mirror_dest_id = mirror_dest_id;
            mdest_cfg->mirror_dest.gport = BCM_GPORT_INVALID;
        }
    } else {
        return BCM_E_NOT_FOUND;
    }

    return (BCM_E_NONE);
}

#ifdef BCM_TRIDENT2_SUPPORT
/* Search a mtp node in a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_search(int unit, bcm_gport_t mirror_dest_id,
                            bcm_gport_t gport, uint8 *found)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL;  /* current node */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);

    if (mdest_cfg->ref_count <= 0) {
        *found = FALSE;
        return (BCM_E_NONE);
    }

    cur_dest = mdest_cfg->next;
    *found = FALSE;

    for (i = 0; i < max_portcnt; i++) {
        if (NULL == cur_dest) {
            break;
        }

        if (cur_dest->mirror_dest.gport == gport) {
            *found = TRUE;
            break;
        }
        cur_dest = cur_dest->next;
    }

    return (BCM_E_NONE);
}

/* Add an MTP into a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_add(int unit, bcm_gport_t mirror_dest_id,
                         _bcm_mirror_dest_config_p mirror_dest_node)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL;  /* Current node */
    _bcm_mirror_dest_config_p prev_dest = NULL; /* Previous node */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);
    if (mdest_cfg->ref_count <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    prev_dest = mdest_cfg;
    cur_dest = mdest_cfg->next;

    /* Insert at tail, so find the last node */
    for (i = 0; i < max_portcnt; i++) {
        if (NULL == cur_dest) {
            break;
        } else {
            prev_dest = cur_dest;
            cur_dest = cur_dest->next;
        }
    }

    if (i < max_portcnt) {
        prev_dest->next = mirror_dest_node;
        mirror_dest_node->next = NULL;
    } else {
        /* Support 8 ports(include head node) at most */
        return (BCM_E_FULL);
    }

    return (BCM_E_NONE);
}

/* Update an MTP info in a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_update(int unit, bcm_gport_t mirror_dest_id,
                            bcm_mirror_destination_t *mirror_dest)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL;  /* Current node */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;
    uint8 found = FALSE;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);
    if (mdest_cfg->ref_count <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    cur_dest = mdest_cfg->next;
    for (i = 0; i < max_portcnt; i++) {
        if (NULL == cur_dest) {
            break;
        }

        if (cur_dest->mirror_dest.gport == mirror_dest->gport) {
            found = TRUE;
            break;
        }
        cur_dest = cur_dest->next;
    }

    if (found == TRUE) {
        sal_memcpy(&cur_dest->mirror_dest,
                   mirror_dest,
                   sizeof(bcm_mirror_destination_t));
    } else {
        return (BCM_E_NOT_FOUND);
    }

    return (BCM_E_NONE);
}

/* Get an MTP info from a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_get(int unit,
                         bcm_gport_t mirror_dest_id,
                         bcm_gport_t gport,
                         bcm_mirror_destination_t *mirror_dest)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL;  /* Current node */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;
    uint8 found = FALSE;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);
    if (mdest_cfg->ref_count <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    cur_dest = mdest_cfg->next;
    for (i = 0; i < max_portcnt; i++) {
        if (NULL == cur_dest) {
            break;
        }

        if (cur_dest->mirror_dest.gport == gport) {
            found = TRUE;
            break;
        }
        cur_dest = cur_dest->next;
    }

    if (found == TRUE) {
        sal_memcpy(mirror_dest,
                   &cur_dest->mirror_dest,
                   sizeof(bcm_mirror_destination_t));
    } else {
        return (BCM_E_NOT_FOUND);
    }

    return (BCM_E_NONE);
}

/* Traverse MTP of a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_traverse(int unit,
                              bcm_gport_t mirror_dest_id,
                              bcm_mirror_destination_traverse_cb cb,
                              void *user_data)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL;  /* Current node */
    bcm_mirror_destination_t mirror_dest;  /* User cb mirror destination.    */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;
    int rv = BCM_E_NONE;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);
    if (mdest_cfg->ref_count <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    cur_dest = mdest_cfg->next;
    for (i = 0; i < max_portcnt; i++) {
        if (NULL == cur_dest) {
            break;
        }

        mirror_dest = cur_dest->mirror_dest;
        rv = (*cb)(unit, &mirror_dest, user_data);
        if (BCM_FAILURE(rv)) {
#ifdef BCM_CB_ABORT_ON_ERR
            if (SOC_CB_ABORT_ON_ERR(unit)) {
                return rv;
            }
#endif
        }
        cur_dest = cur_dest->next;
    }

    return (BCM_E_NONE);
}

/* Get MTP gport array from a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_gport_get(int unit, bcm_gport_t mirror_dest_id,
                               bcm_gport_t *gport_array, int *count)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL; /* Current node */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);
    if (mdest_cfg->ref_count <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    cur_dest = mdest_cfg->next;
    for (i = 0; i < max_portcnt; i++) {
        if (NULL == cur_dest) {
            break;
        }
        gport_array[i] = cur_dest->mirror_dest.gport;
        cur_dest = cur_dest->next;
        *count = i + 1;
    }

    return (BCM_E_NONE);
}


/* Delete an MTP from a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_delete(int unit, bcm_gport_t mirror_dest_id,
                            bcm_gport_t gport)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p prev_dest = NULL;
    _bcm_mirror_dest_config_p cur_dest = NULL;
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;
    uint8 found = FALSE;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);
    if (mdest_cfg->ref_count <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    prev_dest = mdest_cfg;
    cur_dest = mdest_cfg->next;
    for (i = 0; i < max_portcnt; i++) {
        if (NULL == cur_dest) {
            break;
        }

        if (cur_dest->mirror_dest.gport == gport) {
            found = TRUE;
            break;
        }
        prev_dest = cur_dest;
        cur_dest = cur_dest->next;
    }

    if (found == TRUE) {
        prev_dest->next = cur_dest->next;
        cur_dest->next = NULL;
        sal_free(cur_dest);
    } else {
        return (BCM_E_NOT_FOUND);
    }

    return (BCM_E_NONE);
}

/* Delete all MTPs of a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_mtp_delete_all(int unit,
                                bcm_gport_t mirror_dest_id)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p next_dest = NULL;
    _bcm_mirror_dest_config_p free_dest = NULL;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);
    if (mdest_cfg->ref_count <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    free_dest = mdest_cfg->next;
    mdest_cfg->next = NULL;
    while (free_dest) {
        next_dest = free_dest->next;
        free_dest->next = NULL;
        sal_free(free_dest);
        free_dest = next_dest;
    }

    return (BCM_E_NONE);
}

/* Free all MTP nodes of one mirror destination array. */
STATIC int
_bcm_mirror_dest_array_mtp_free(int unit,
                                _bcm_mirror_config_p cfg_ptr)
{
    _bcm_mirror_config_p ptr;
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p next_dest = NULL;
    _bcm_mirror_dest_config_p free_dest = NULL;
    int i = 0, dest_count = 0;

    if (NULL == cfg_ptr) {
        return (BCM_E_PARAM);
    }

    ptr = cfg_ptr;
    dest_count = ptr->dest_count;
    for (i = 0; i < dest_count; i++) {
        mdest_cfg = ptr->dest_arr + i;

        if (mdest_cfg->ref_count > 0) {
            free_dest = mdest_cfg->next;
            mdest_cfg->next = NULL;

            while (free_dest) {
                next_dest = free_dest->next;
                free_dest->next = NULL;
                sal_free(free_dest);
                free_dest = next_dest;
            }
        }
    }

    return (BCM_E_NONE);
}

/* Set a shared-id mirror destination. */
STATIC int
_bcm_esw_mirror_shared_dest_set(int unit,
                               bcm_mirror_destination_t *mirror_dest)
{
    _bcm_mirror_dest_config_p mir_dest_node = NULL;
    int rv = BCM_E_NONE;
    uint8 found = FALSE;

    if (mirror_dest->flags & BCM_MIRROR_DEST_WITH_ID) {
        if (0 != MIRROR_DEST_REF_COUNT(unit, mirror_dest->mirror_dest_id)) {
            if (mirror_dest->flags & BCM_MIRROR_DEST_REPLACE) {
                /* Replace a shared-id mirror destination */
                *(MIRROR_DEST(unit, mirror_dest->mirror_dest_id)) = *mirror_dest;
                (MIRROR_DEST(unit, mirror_dest->mirror_dest_id))->flags &=
                    (BCM_MIRROR_DEST_TUNNELS |
                     BCM_MIRROR_DEST_INT_PRI_SET | BCM_MIRROR_DEST_REPLACE |
                     BCM_MIRROR_DEST_FIELD | BCM_MIRROR_DEST_PORT |
                     BCM_MIRROR_DEST_ID_SHARE);
            } else if (mirror_dest->flags & BCM_MIRROR_DEST_MTP_ADD) {
                /* Add an MTP into a shared-id mirror destination */
                BCM_IF_ERROR_RETURN(
                    _bcm_mirror_dest_mtp_search(
                        unit, mirror_dest->mirror_dest_id,
                        mirror_dest->gport, &found));
                if (TRUE == found) {
                    return (BCM_E_EXISTS);
                }
                mir_dest_node = sal_alloc(sizeof(_bcm_mirror_dest_config_t),
                                          "Mirror destination config node");

                if (NULL == mir_dest_node) {
                    rv = (BCM_E_MEMORY);
                    return rv;
                }

                sal_memset(mir_dest_node, 0,
                           sizeof(_bcm_mirror_dest_config_t));
                sal_memcpy(&mir_dest_node->mirror_dest, mirror_dest,
                           sizeof(bcm_mirror_destination_t));
                rv= _bcm_mirror_dest_mtp_add(
                        unit, mirror_dest->mirror_dest_id,
                        mir_dest_node);
                if (BCM_FAILURE(rv)) {
                    if (NULL != mir_dest_node) {
                        sal_free(mir_dest_node);
                    }
                }
            } else if (mirror_dest->flags & BCM_MIRROR_DEST_MTP_REPLACE) {
                /* Replace an MTP of a shared-id mirror destination */
                rv = _bcm_mirror_dest_mtp_update(
                         unit, mirror_dest->mirror_dest_id,
                         mirror_dest);
            } else if (mirror_dest->flags & BCM_MIRROR_DEST_MTP_DELETE) {
                /* Delete an MTP from a shared-id mirror destination */
                rv = _bcm_mirror_dest_mtp_delete(
                         unit, mirror_dest->mirror_dest_id,
                         mirror_dest->gport);
            } else {
                rv = (BCM_E_PARAM);
            }
        } else {
            /* Create a new shared-id mirror destination with id. */
            MIRROR_DEST_REF_COUNT(unit, mirror_dest->mirror_dest_id) = 1;
            *(MIRROR_DEST(unit, mirror_dest->mirror_dest_id)) = *mirror_dest;
            (MIRROR_DEST(unit, mirror_dest->mirror_dest_id))->flags &=
                (BCM_MIRROR_DEST_TUNNELS |
                 BCM_MIRROR_DEST_INT_PRI_SET | BCM_MIRROR_DEST_REPLACE |
                 BCM_MIRROR_DEST_FIELD | BCM_MIRROR_DEST_PORT |
                 BCM_MIRROR_DEST_ID_SHARE);
            rv = (BCM_E_NONE);
        }
    } else {
        /* Create a new shared-id mirror destination. */
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_destination_alloc(unit, &mirror_dest->mirror_dest_id));

        *(MIRROR_DEST(unit, mirror_dest->mirror_dest_id)) = *mirror_dest;
        (MIRROR_DEST(unit, mirror_dest->mirror_dest_id))->flags &=
            (BCM_MIRROR_DEST_TUNNELS |
             BCM_MIRROR_DEST_INT_PRI_SET | BCM_MIRROR_DEST_REPLACE |
             BCM_MIRROR_DEST_FIELD | BCM_MIRROR_DEST_PORT |
             BCM_MIRROR_DEST_ID_SHARE);
        rv = (BCM_E_NONE);
    }

    return rv;
}
#endif /* BCM_TRIDENT2_SUPPORT */

/*
 * Function:
 *     _bcm_esw_mirror_destination_create
 * Purpose:
 *     Helper function to API that creates mirror destination description.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      mirror_dest  - (IN) Mirror destination description.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int 
_bcm_esw_mirror_destination_create(int unit,
                                   bcm_mirror_destination_t *mirror_dest)
{
#ifdef BCM_TRIDENT2_SUPPORT
    if (mirror_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
        return _bcm_esw_mirror_shared_dest_set(unit, mirror_dest);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    if (mirror_dest->flags & BCM_MIRROR_DEST_WITH_ID) {
        /* Check mirror destination id */
        if ((0 == BCM_GPORT_IS_MIRROR(mirror_dest->mirror_dest_id)) || 
           (BCM_GPORT_MIRROR_GET(mirror_dest->mirror_dest_id) >=
                                    MIRROR_DEST_CONFIG_COUNT(unit))) {
            return (BCM_E_BADID);
        }

        /* Check if mirror destination is being updated  */
        if (0 != MIRROR_DEST_REF_COUNT(unit, mirror_dest->mirror_dest_id)) { 
            if (0 == (mirror_dest->flags & BCM_MIRROR_DEST_REPLACE)) {
                return (BCM_E_EXISTS);
            }
        } else {
            MIRROR_DEST_REF_COUNT(unit, mirror_dest->mirror_dest_id) = 1;
        }
    } else {
        /* Allocate new mirror destination. */
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_destination_alloc(unit, &mirror_dest->mirror_dest_id));
    }

    /* Set mirror destination configuration. */
    
    *(MIRROR_DEST(unit, mirror_dest->mirror_dest_id)) = *mirror_dest; 
    (MIRROR_DEST(unit, mirror_dest->mirror_dest_id))->flags &=
        (BCM_MIRROR_DEST_TUNNELS |
         BCM_MIRROR_DEST_INT_PRI_SET | BCM_MIRROR_DEST_REPLACE |
         BCM_MIRROR_DEST_FIELD | BCM_MIRROR_DEST_PORT);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_esw_mirror_ingress_get
 * Description:
 * 	    Get the mirroring per ingress enabled/disabled status
 * Parameters:
 *  	unit -   (IN)  BCM device number
 *  	port -   (IN)  The port to check
 *  	enable - (OUT) Place to store boolean return value for on/off
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_ingress_get(int unit, bcm_port_t port, int *enable)
{
    int vp = VP_PORT_INVALID;
    bcm_module_t mod_out;
    bcm_trunk_t tgid_out;
#ifdef BCM_TRIDENT2PLUS_SUPPORT
    int rv = BCM_E_NONE;
#endif
    
#ifdef BCM_TRIDENT2PLUS_SUPPORT
    if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
        _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
        if (!_bcm_xgs5_subport_coe_gport_local(unit, port)) {
            return BCM_E_PORT;
        }

        PORT_LOCK(unit);
        rv = bcm_esw_port_lport_field_get(unit, port, LPORT_PROFILE_LPORT_TAB,
                                          MIRRORf, (uint32*)enable);
        PORT_UNLOCK(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    } else
#endif
    {
        if (BCM_GPORT_IS_SET(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
#ifdef BCM_TRIDENT_SUPPORT
            if (BCM_GPORT_IS_NIV_PORT(port)) {
                bcm_niv_port_t   niv_port;

                bcm_niv_port_t_init(&niv_port);
                niv_port.niv_port_id = port;
                BCM_IF_ERROR_RETURN(bcm_trident_niv_port_get(unit, &niv_port));
                if (niv_port.flags & BCM_NIV_PORT_MATCH_NONE) {
                    /* getting vp is enough in this case */
                    vp = BCM_GPORT_NIV_PORT_ID_GET(port);
                    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeNiv)) {
                        return BCM_E_NOT_FOUND;
                    }
                }
            } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH3_SUPPORT
            if (BCM_GPORT_IS_EXTENDER_PORT(port)) {
                bcm_extender_port_t extender_port;

                bcm_extender_port_t_init(&extender_port);
                extender_port.extender_port_id = port;
                BCM_IF_ERROR_RETURN(bcm_tr3_extender_port_get(unit, &extender_port));
                if (extender_port.flags & BCM_EXTENDER_PORT_MATCH_NONE) {
                    /* getting vp is enough in this case */
                    vp = BCM_GPORT_EXTENDER_PORT_ID_GET(port);
                    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeExtender)) {
                        return BCM_E_NOT_FOUND;
                    }
                }
            }
#endif /* BCM_TRIUMPH3_SUPPORT */
#endif /* BCM_TRIUMPH2_SUPPORT && INCLUDE_L3 */

            /* if VP has not been resolved */
            if (VP_PORT_INVALID == vp){
                BCM_IF_ERROR_RETURN(
                    _bcm_esw_gport_resolve(unit, port, &mod_out,
                                           &port, &tgid_out, &vp));
            }
        }

        if (VP_PORT_INVALID != vp) {
            BCM_IF_ERROR_RETURN(
                _bcm_tr2_mirror_svp_enable_get(unit, vp, enable));
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_port_mirror_enable_get(unit, port, enable));
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *	    _bcm_esw_mirror_ingress_set
 * Description:
 * 	    Set the mirroring per ingress enabled/disabled status
 * Parameters:
 *  	unit -   (IN)  BCM device number
 *  	port -   (IN)  The port to check
 *  	enable - (IN) Place to store boolean return value for on/off
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_ingress_set(int unit, bcm_port_t port, int enable)
{
    int vp = VP_PORT_INVALID;
    bcm_module_t mod_out;
    bcm_trunk_t tgid_out;
#ifdef BCM_TRIDENT2PLUS_SUPPORT
    int rv = BCM_E_NONE;
#endif

#ifdef BCM_TRIDENT2PLUS_SUPPORT
    if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
        _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
        if (!_bcm_xgs5_subport_coe_gport_local(unit, port)) {
            return BCM_E_PORT;
        }

        PORT_LOCK(unit);
        rv = bcm_esw_port_lport_field_set(unit, port, LPORT_PROFILE_LPORT_TAB,
                                          MIRRORf, enable);
        PORT_UNLOCK(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    } else
#endif
    {
        if (BCM_GPORT_IS_SET(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
#ifdef BCM_TRIDENT_SUPPORT
            if (BCM_GPORT_IS_NIV_PORT(port)) {
                bcm_niv_port_t   niv_port;

                bcm_niv_port_t_init(&niv_port);
                niv_port.niv_port_id = port;
                BCM_IF_ERROR_RETURN(bcm_trident_niv_port_get(unit, &niv_port));
                if (niv_port.flags & BCM_NIV_PORT_MATCH_NONE) {
                    /* getting vp is enough in this case */
                    vp = BCM_GPORT_NIV_PORT_ID_GET(port);
                    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeNiv)) {
                        return BCM_E_NOT_FOUND;
                    }
                }
            } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH3_SUPPORT
            if (BCM_GPORT_IS_EXTENDER_PORT(port)) {
                bcm_extender_port_t extender_port;

                bcm_extender_port_t_init(&extender_port);
                extender_port.extender_port_id = port;
                BCM_IF_ERROR_RETURN(bcm_tr3_extender_port_get(unit, &extender_port));
                if (extender_port.flags & BCM_EXTENDER_PORT_MATCH_NONE) {
                    /* getting vp is enough in this case */
                    vp = BCM_GPORT_EXTENDER_PORT_ID_GET(port);
                    if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeExtender)) {
                        return BCM_E_NOT_FOUND;
                    }
                }
            }
#endif /* BCM_TRIUMPH3_SUPPORT */
#endif /* BCM_TRIUMPH2_SUPPORT && INCLUDE_L3 */

            /* if VP has not been resolved */
            if (VP_PORT_INVALID == vp){
                BCM_IF_ERROR_RETURN(
                    _bcm_esw_gport_resolve(unit, port, &mod_out,
                                           &port, &tgid_out, &vp));
            }
        }

        if (VP_PORT_INVALID != vp) {
            BCM_IF_ERROR_RETURN(
                _bcm_tr2_mirror_svp_enable_set(unit, vp, enable));
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_port_mirror_enable_set(unit, port, enable));
        }
    }

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_VERSION_1_1                SOC_SCACHE_VERSION(1,1)
#define BCM_WB_VERSION_1_2                SOC_SCACHE_VERSION(1,2)
#define BCM_WB_VERSION_1_3                SOC_SCACHE_VERSION(1,3)
#define BCM_WB_VERSION_1_4                SOC_SCACHE_VERSION(1,4)
#define BCM_WB_VERSION_1_5                SOC_SCACHE_VERSION(1,5)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_5


#ifdef BCM_TRX_SUPPORT
STATIC int
_bcm_esw_mirror_mtp_entry_trunk_get(int unit, void *mtp_entry,
                                    bcm_gport_t *port)
{
    bcm_module_t tp_mod;
    bcm_port_t tp_port;
    bcm_gport_t tgid;
    int rv = BCM_E_NONE;

    /* We're working with IM_MTP_INDEXm, but it is equivalent for
     * all of the .._MTP_INDEXm tables. */
    if (soc_mem_field_valid(unit, IM_MTP_INDEXm, TGIDf)) {
        tgid = soc_IM_MTP_INDEXm_field32_get(unit, mtp_entry, TGIDf);
    } else {
        if (0 == soc_IM_MTP_INDEXm_field32_get(unit, mtp_entry, RTAGf)) {
            /* No ports in trunk, so the trunk id is cached */
            if (soc_mem_field_valid(unit, IM_MTP_INDEXm, PORT_NUM_7f)) {
                tgid = soc_IM_MTP_INDEXm_field32_get(unit, mtp_entry,
                                                     PORT_NUM_7f);
            } else {
                /* This case should not occur. */
                return BCM_E_INTERNAL;
            }
        } else {
            tp_port = soc_IM_MTP_INDEXm_field32_get(unit, mtp_entry, PORT_NUMf);
            tp_mod = soc_IM_MTP_INDEXm_field32_get(unit, mtp_entry, MODULE_IDf);
            rv = _bcm_esw_trunk_port_property_get(unit, tp_mod,
                                                    tp_port, &tgid);
            if (BCM_FAILURE(rv) || (tgid == -1)) {
                return rv;
            }
        }
    }

    BCM_IF_ERROR_RETURN 
        (_bcm_mirror_gport_construct(unit, tgid, 0,
                                     BCM_MIRROR_PORT_DEST_TRUNK, port));
    return BCM_E_NONE;
}
#endif /* BCM_TRX_SUPPORT */

int
_bcm_esw_mirror_mtp_to_modport(int unit, int mtp_index, int modport,
                                int flags, bcm_module_t *modid,
                                bcm_gport_t *port)
{
    im_mtp_index_entry_t im_mtp;
    em_mtp_index_entry_t em_mtp;

    if (0 != (flags & BCM_MIRROR_PORT_INGRESS)) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_read(unit, IM_MTP_INDEXm, MEM_BLOCK_ALL,
                          mtp_index, &im_mtp));
#ifdef BCM_TRX_SUPPORT
        if (soc_mem_field_valid(unit, IM_MTP_INDEXm, Tf)) {
            if (0 != soc_IM_MTP_INDEXm_field32_get(unit, &im_mtp, Tf)) {
                BCM_IF_ERROR_RETURN 
                    (_bcm_esw_mirror_mtp_entry_trunk_get(unit, &im_mtp,
                                                         port));
                *modid = 0;
            } else {
                *port = soc_IM_MTP_INDEXm_field32_get(unit, &im_mtp,
                                                      PORT_NUMf);
                *modid = soc_IM_MTP_INDEXm_field32_get(unit, &im_mtp,
                                                       MODULE_IDf);
            }
        } else
#endif /* BCM_TRX_SUPPORT */
        {
            *port =
                soc_IM_MTP_INDEXm_field32_get(unit, &im_mtp, PORT_TGIDf);
            *modid =
                soc_IM_MTP_INDEXm_field32_get(unit, &im_mtp, MODULE_IDf);
        }
    } else if (0 != (flags & BCM_MIRROR_PORT_EGRESS)) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_read(unit, EM_MTP_INDEXm, MEM_BLOCK_ALL,
                          mtp_index, &em_mtp));
#ifdef BCM_TRX_SUPPORT
        if (soc_mem_field_valid(unit, EM_MTP_INDEXm, Tf)) {
            if (0 != soc_EM_MTP_INDEXm_field32_get(unit, &em_mtp, Tf)) {
                BCM_IF_ERROR_RETURN 
                    (_bcm_esw_mirror_mtp_entry_trunk_get(unit, &em_mtp,
                                                         port));
                *modid = 0;
            } else {
                *port = soc_EM_MTP_INDEXm_field32_get(unit, &em_mtp,
                                                      PORT_NUMf);
                *modid = soc_EM_MTP_INDEXm_field32_get(unit, &em_mtp,
                                                       MODULE_IDf);
            }
        } else
#endif /* BCM_TRX_SUPPORT */
        {
            *port =
                soc_EM_MTP_INDEXm_field32_get(unit, &em_mtp, PORT_TGIDf);
            *modid =
                soc_EM_MTP_INDEXm_field32_get(unit, &em_mtp, MODULE_IDf);
        }
#ifdef BCM_TRIUMPH2_SUPPORT
    } else if (soc_feature(unit, soc_feature_egr_mirror_true) &&
               (0 != (flags & BCM_MIRROR_PORT_EGRESS_TRUE))) {
        ep_redirect_em_mtp_index_entry_t epm_mtp;

        BCM_IF_ERROR_RETURN 
            (soc_mem_read(unit, EP_REDIRECT_EM_MTP_INDEXm,
                          MEM_BLOCK_ALL, mtp_index, &epm_mtp));
        if (0 != soc_EP_REDIRECT_EM_MTP_INDEXm_field32_get(unit,
                                                           &epm_mtp, Tf)) {
            BCM_IF_ERROR_RETURN 
                (_bcm_esw_mirror_mtp_entry_trunk_get(unit, &epm_mtp,
                                                         port));
            *modid = 0;
        } else {
            *port = soc_EP_REDIRECT_EM_MTP_INDEXm_field32_get(unit, &epm_mtp,
                                                              PORT_NUMf);
            *modid =
                soc_EP_REDIRECT_EM_MTP_INDEXm_field32_get(unit, &epm_mtp,
                                                          MODULE_IDf);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    } else {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_TRUNK(*port)) {
        if (modport) {
            /* Put into modport gport format */
            BCM_IF_ERROR_RETURN 
                (_bcm_mirror_gport_construct(unit, *port, *modid, 0, port));
        } else {
            /* Translate into normalized (modport, port) form */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                         *modid, *port, 
                                         modid, port));
        }
    }

    return BCM_E_NONE;
}

#ifdef BCM_FIELD_SUPPORT
/*
 * Function:
 *  	_bcm_esw_mirror_field_group_reload 
 * Purpose:
 *  	Used as a callback routine to traverse over field groups 
 * Parameters:
 *	    unit        - (IN) BCM device number.
 *      group       - (IN) Group id
 *      user_data   - (IN) User data pointer
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_field_group_reload(int unit, bcm_field_group_t group,
                                   void *user_data)
{
    int entry_count, entry_num;
    int alloc_sz, rv = BCM_E_NONE, flags, idx;
    bcm_field_entry_t *entry_array, entry_id;
    bcm_mirror_destination_t  mirror_dest;
    bcm_gport_t gport;
    uint32 param0, param1;
    bcm_field_qset_t group_qset;

    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_field_entry_multi_get(unit, group, 0, NULL, &entry_num));
    if (entry_num == 0) {
        /* In Level 1 the FP may detect extra groups if a slice if the group
           PBMP is not ALL. These will come across as dummy groups with
           no entries. */
        return BCM_E_NONE;
    }

    alloc_sz = sizeof(bcm_field_entry_t) * entry_num;
    entry_array = sal_alloc(alloc_sz, "Field IDs");
    if (NULL == entry_array) {
        return BCM_E_MEMORY;
    }
    sal_memset(entry_array, 0, alloc_sz);

    rv = bcm_esw_field_entry_multi_get(unit, group, entry_num,
                                       entry_array, &entry_count);
    if (BCM_FAILURE(rv)) {
        sal_free(entry_array);
        return rv;
    }
    if (entry_count != entry_num) {
        /* Why didn't we get the number of ID's we were told existed? */
        sal_free(entry_array);
        return BCM_E_INTERNAL;
    }

    for (entry_count = 0; entry_count < entry_num; entry_count++) {
        entry_id = entry_array[entry_count];
        rv = bcm_esw_field_action_get(unit, entry_id,
                                      bcmFieldActionMirrorIngress,
                                      &param0, &param1);
        if (BCM_SUCCESS(rv)) {
            gport = param1;
            if (!BCM_GPORT_IS_SET(gport)) {
                rv = _bcm_mirror_gport_construct(unit, param1, param0,
                                                 0, &gport);
                if (BCM_FAILURE(rv)) {
                    break;
                }
            }
            flags = BCM_MIRROR_PORT_INGRESS;
            bcm_mirror_destination_t_init(&mirror_dest);
            rv = _bcm_esw_mirror_destination_find(unit, gport, 0,
                                                  flags, &mirror_dest); 
            if (BCM_E_NOT_FOUND == rv) {
                rv = BCM_E_INTERNAL;
                /* Should have recovered the destination already. */
            }
            if (BCM_FAILURE(rv)) {
                break;
            }
            if (soc_feature(unit, soc_feature_mirror_flexible)) {
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                    if (mirror_dest.mirror_dest_id ==
                        MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx) && 
                        !MIRROR_CONFIG_SHARED_MTP(unit, idx).egress) {
                        break;
                    }
                }
                if (idx < BCM_MIRROR_MTP_COUNT) {
                    MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
                    MIRROR_DEST_REF_COUNT(unit,
                        MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx))++;
                    /* If we found a mirror action, then mirroring is enabled */
                    MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
                } else {
                    rv = BCM_E_INTERNAL;
                    break;
                }
            } else {
                for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
                    if (mirror_dest.mirror_dest_id ==
                        MIRROR_CONFIG_ING_MTP_DEST(unit, idx)) {
                        break;
                    }
                }
                if (idx < MIRROR_CONFIG(unit)->ing_mtp_count) {
                    MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)++;
                    MIRROR_DEST_REF_COUNT(unit,
                           MIRROR_CONFIG_ING_MTP_DEST(unit, idx))++;
                    /* If we found a mirror action, then mirroring is enabled */
                    MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
                } else {
                    rv = BCM_E_INTERNAL;
                    break;
                }
            }
        } else if (rv != BCM_E_NOT_FOUND) {
            break;
        } else {
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "Mirror module reload, ignore FP error report\n")));
        }

        rv = bcm_esw_field_action_get(unit, entry_id,
                                      bcmFieldActionMirrorEgress,
                                      &param0, &param1);
        if (BCM_SUCCESS(rv)) {
            gport = param1;
            if (!BCM_GPORT_IS_SET(gport)) {
                rv = _bcm_mirror_gport_construct(unit, param1, param0,
                                                 0, &gport);
                if (BCM_FAILURE(rv)) {
                    break;
                }
            }

            /* Initialize the qset */
            BCM_FIELD_QSET_INIT(group_qset);

            /* Get the group Qset info */
            rv = bcm_esw_field_group_get(unit, group, &group_qset);
            if (BCM_FAILURE(rv)) {
                break;
            }

            /* Check if Mirroring is set for Egress Stage */
            if (BCM_FIELD_QSET_TEST(group_qset, bcmFieldQualifyStageEgress)) {
                flags = BCM_MIRROR_PORT_EGRESS_TRUE;
            } else {
                flags = BCM_MIRROR_PORT_EGRESS;
            }

            bcm_mirror_destination_t_init(&mirror_dest);
            rv = _bcm_esw_mirror_destination_find(unit, gport, 0,
                                                  flags, &mirror_dest); 
            if (BCM_E_NOT_FOUND == rv) {
                rv = BCM_E_INTERNAL;
                /* Should have recovered the destination already. */
            }
            if (BCM_FAILURE(rv)) {
                break;
            }

            if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                    if ((mirror_dest.mirror_dest_id
                        == MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx))) {
                        break;
                    }
                }
                if (idx < BCM_MIRROR_MTP_COUNT) {
                    MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)++;
                    MIRROR_DEST_REF_COUNT(unit, mirror_dest.mirror_dest_id)++;
                    /* If we found a mirror action, then mirroring is enabled */
                    MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
                } else {
                    rv = BCM_E_INTERNAL;
                    break;
                }
            } else if (soc_feature(unit, soc_feature_mirror_flexible)) {
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                    if (mirror_dest.mirror_dest_id ==
                        MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx) && 
                        MIRROR_CONFIG_SHARED_MTP(unit, idx).egress) {
                        break;
                    }
                }
                if (idx < BCM_MIRROR_MTP_COUNT) {
                    MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) +=
                        (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
                    MIRROR_DEST_REF_COUNT(unit,
                           MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx))++;
                    /* If we found a mirror action, then mirroring is enabled */
                    MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
                } else {
                    rv = BCM_E_INTERNAL;
                    break;
                }
            } else {
                for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                    if (mirror_dest.mirror_dest_id ==
                        MIRROR_CONFIG_EGR_MTP_DEST(unit, idx)) {
                        break;
                    }
                }
                if (idx < MIRROR_CONFIG(unit)->egr_mtp_count) {
                    MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)++;
                    MIRROR_DEST_REF_COUNT(unit,
                           MIRROR_CONFIG_EGR_MTP_DEST(unit, idx))++;
                    /* If we found a mirror action, then mirroring is enabled */
                    MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
                } else {
                    rv = BCM_E_INTERNAL;
                    break;
                }
            }
        } else if (rv != BCM_E_NOT_FOUND) {
            break;
        } else {
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "Mirror module reload, ignore FP error report\n")));
        }
    }

    if (BCM_E_NOT_FOUND == rv) {
        rv = BCM_E_NONE; /* Do not propagate this error */
    }

    sal_free(entry_array);
    return rv;
}
#endif /* BCM_FIELD_SUPPORT */


#ifdef BCM_XGS12_FABRIC_SUPPORT
STATIC int
_bcm_xgs12_fabric_mirror_reinit(int unit)
{
    uint32 mirbmap;
    pbmp_t pbmp;
    bcm_port_t port;
  	 
    PBMP_HG_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_ING_MIRTOBMAPr(unit, port, &mirbmap));
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, mirbmap);
        if (SOC_PBMP_NOT_NULL(pbmp)) {
            MIRROR_CONFIG(unit)->mode = BCM_MIRROR_L2;
            break;
        }
    }

    /*
     * Cannot recover MTP info for 5675
     */

    return BCM_E_NONE;
}
#endif /* BCM_XGS12_FABRIC_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
/*
 * Function:
 *  	_bcm_td_mirror_tunnel_reload 
 * Purpose:
 *  	Restores mirroring tunnel destination encap info
 *      for warm boot recovery
 * Parameters:
 *	unit        - (IN) BCM device number.
 *      mirror_dest  - (IN) Mirror destination description.
 *      profile_index   -  (IN) Encap profile index
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_td_mirror_tunnel_reload(int unit, bcm_mirror_destination_t *mirror_dest,
                             uint32 profile_index)
{
    egr_mirror_encap_control_entry_t control_entry;
    egr_mirror_encap_data_1_entry_t data_1_entry;
    egr_mirror_encap_data_2_entry_t data_2_entry;
    void *entries[EGR_MIRROR_ENCAP_ENTRIES_NUM];
    int optional_header;
    uint32 hw_buffer[_BCM_TD_MIRROR_V4_GRE_BUFFER_SZ]; /* Max size needed */

    entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL] = &control_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1] = &data_1_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2] = &data_2_entry;
    
    /* Tunnel type? */
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_get(unit, EGR_MIRROR_ENCAP(unit),
                             profile_index, 1, entries));

    optional_header =
        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_get(unit, &control_entry,
                                      RSPAN__ADD_OPTIONAL_HEADERf);
    if (soc_EGR_MIRROR_ENCAP_CONTROLm_field32_get(unit, &control_entry,
                       ENTRY_TYPEf) == BCM_TD_MIRROR_ENCAP_TYPE_SFLOW) {
        /* SFLOW recovery */
        soc_mem_mac_addr_get(unit, EGR_MIRROR_ENCAP_DATA_1m,
                             &data_1_entry, SFLOW__SFLOW_HEADER_DAf,
                             mirror_dest->dst_mac);
        soc_mem_mac_addr_get(unit, EGR_MIRROR_ENCAP_DATA_1m,
                             &data_1_entry, SFLOW__SFLOW_HEADER_SAf,
                             mirror_dest->src_mac);
        hw_buffer[0] =
            soc_EGR_MIRROR_ENCAP_DATA_1m_field32_get(unit, &data_1_entry,
                                      SFLOW__SFLOW_HEADER_VLAN_TAGf);
        mirror_dest->vlan_id = (bcm_vlan_t) (hw_buffer[0] & 0xffff);
        mirror_dest->tpid = (uint16) ((hw_buffer[0] >> 16) & 0xffff);
        soc_EGR_MIRROR_ENCAP_DATA_1m_field_get(unit, &data_1_entry,
                         SFLOW__SFLOW_HEADER_V4f, hw_buffer);

        /* See _bcm_trident_mirror_ipv4_gre_tunnel_set for the encoding */
        mirror_dest->version = 4;
        mirror_dest->dst_addr = hw_buffer[0];
        mirror_dest->src_addr = hw_buffer[1];
        mirror_dest->ttl = (uint8) ((hw_buffer[2] >> 24) & 0xff);
        mirror_dest->df = (uint8) ((hw_buffer[3] >> 14) & 0x1);
        mirror_dest->tos = (uint8) ((hw_buffer[4] >> 16) & 0xff);

        soc_EGR_MIRROR_ENCAP_DATA_1m_field_get(unit, &data_1_entry,
                         SFLOW__SFLOW_HEADER_UDPf, hw_buffer);
        mirror_dest->udp_dst_port = hw_buffer[1] & 0xffff;
        mirror_dest->udp_src_port = (hw_buffer[1] >> 16) & 0xffff;

        mirror_dest->flags |= BCM_MIRROR_DEST_TUNNEL_SFLOW;

    } else if (soc_EGR_MIRROR_ENCAP_CONTROLm_field32_get(unit, &control_entry,
                       ENTRY_TYPEf) == BCM_TD_MIRROR_ENCAP_TYPE_ERSPAN) {
        /* ERSPAN recovery */
        soc_mem_mac_addr_get(unit, EGR_MIRROR_ENCAP_DATA_1m,
                             &data_1_entry, ERSPAN__ERSPAN_HEADER_DAf,
                             mirror_dest->dst_mac);
        soc_mem_mac_addr_get(unit, EGR_MIRROR_ENCAP_DATA_1m,
                             &data_1_entry, ERSPAN__ERSPAN_HEADER_SAf,
                             mirror_dest->src_mac);

        mirror_dest->gre_protocol =
            soc_EGR_MIRROR_ENCAP_DATA_1m_field32_get(unit, &data_1_entry,
                                      ERSPAN__ERSPAN_HEADER_GREf);

        hw_buffer[0] =
            soc_EGR_MIRROR_ENCAP_DATA_1m_field32_get(unit, &data_1_entry,
                                      ERSPAN__ERSPAN_HEADER_VLAN_TAGf);

        mirror_dest->vlan_id = (bcm_vlan_t) (hw_buffer[0] & 0xffff);
        mirror_dest->tpid = (uint16) ((hw_buffer[0] >> 16) & 0xffff);

        soc_EGR_MIRROR_ENCAP_DATA_1m_field_get(unit, &data_1_entry,
                         ERSPAN__ERSPAN_HEADER_V4f, hw_buffer);
        /* See _bcm_trident_mirror_ipv4_gre_tunnel_set for the encoding */
        mirror_dest->version = 4;
        mirror_dest->dst_addr = hw_buffer[0];
        mirror_dest->src_addr = hw_buffer[1];
        mirror_dest->ttl = (uint8) ((hw_buffer[2] >> 24) & 0xff);
        mirror_dest->df = (uint8) ((hw_buffer[3] >> 14) & 0x1);
        mirror_dest->tos = (uint8) ((hw_buffer[4] >> 16) & 0xff);

        mirror_dest->flags |= BCM_MIRROR_DEST_TUNNEL_IP_GRE;
    } else {
        /* RSPAN recovery */
        if (BCM_TD_MIRROR_HEADER_ONLY == optional_header) {
            hw_buffer[0] =
                soc_EGR_MIRROR_ENCAP_DATA_1m_field32_get(
                    unit, &data_1_entry, RSPAN__RSPAN_VLAN_TAGf);

            mirror_dest->vlan_id = (bcm_vlan_t) (hw_buffer[0] & 0xffff);
            mirror_dest->tpid = (uint16) ((hw_buffer[0] >> 16) & 0xffff);

            mirror_dest->flags |= BCM_MIRROR_DEST_TUNNEL_L2;
        }
    }

    if (BCM_TD_MIRROR_HEADER_TRILL == optional_header) {
        /* TRILL recovery */
        soc_EGR_MIRROR_ENCAP_DATA_2m_field_get(unit, &data_2_entry,
                                               HEADER_DATAf, hw_buffer);

        mirror_dest->trill_dst_name =
            (hw_buffer[0] >> BCM_TD_MIRROR_TRILL_DEST_NAME_OFFSET) &
            _BCM_TD_MIRROR_TRILL_NAME_MASK;
        mirror_dest->trill_src_name =
            (hw_buffer[1] & _BCM_TD_MIRROR_TRILL_NAME_MASK);
        mirror_dest->trill_hopcount =
            ((hw_buffer[1] >> BCM_TD_MIRROR_TRILL_HOPCOUNT_OFFSET) &
             _BCM_TD_MIRROR_TRILL_HOPCOUNT_MASK);

        mirror_dest->flags |= BCM_MIRROR_DEST_TUNNEL_TRILL;
    } else if (BCM_TD_MIRROR_HEADER_VNTAG == optional_header) {
        /* NIV recovery */
        hw_buffer[0] =
            soc_EGR_MIRROR_ENCAP_DATA_2m_field32_get(unit, &data_2_entry,
                                                     VNTAG_HEADERf);

        if (0 != (hw_buffer[0] & _BCM_TD_MIRROR_NIV_LOOP_BIT)) {
            mirror_dest->niv_flags = BCM_MIRROR_NIV_LOOP;
        }

        mirror_dest->niv_src_vif =
            (hw_buffer[0] & _BCM_TD_MIRROR_NIV_SRC_VIF_MASK);
        mirror_dest->niv_dst_vif =
            ((hw_buffer[0] >> _BCM_TD_MIRROR_NIV_DST_VIF_OFFSET) &
             _BCM_TD_MIRROR_NIV_DST_VIF_MASK);

        mirror_dest->flags |= BCM_MIRROR_DEST_TUNNEL_NIV;
    } else if (BCM_MIRROR_HEADER_ETAG == optional_header) {
        if(soc_feature(unit, soc_feature_port_extension)) {
            /* ETAG recovery */
            soc_EGR_MIRROR_ENCAP_DATA_2m_field_get(unit, &data_2_entry,
                                                   HEADER_DATAf, hw_buffer);

            mirror_dest->etag_dst_vid =
                (hw_buffer[0] & _BCM_MIRROR_ETAG_DST_VID_MASK);
            mirror_dest->etag_src_vid =
                ((hw_buffer[0] >> _BCM_MIRROR_ETAG_SRC_VID_OFFSET) &
                 _BCM_MIRROR_ETAG_SRC_VID_MASK);

            mirror_dest->flags |= BCM_MIRROR_DEST_TUNNEL_ETAG; 
        }        
    } /* Else no additional header to recover */

    return BCM_E_NONE;
}
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

/*
 * Function:
 *      _bcm_mirror_sflow_mtp_ref_count_recover
 * Purpose:
 *      Restores virtual port mtp usage reference counts for warm boot recovery
 * Parameters:
 *          unit        - (IN) BCM device number.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_mirror_sflow_mtp_ref_count_recover(int unit)
{
#if defined(BCM_TOMAHAWK_SUPPORT)
    uint32 reg_val;
    int mc_enable;
    int mtp_index;
    int mtp_slot, mtp_bit; 
    int i;
    uint32 index_val[BCM_MIRROR_MTP_COUNT];
    soc_field_t mtp_idxf[BCM_MIRROR_MTP_COUNT] = {
        MTP_INDEX0f,MTP_INDEX1f,MTP_INDEX2f,MTP_INDEX3f};

    if (!soc_feature(unit, soc_feature_sflow_ing_mirror)) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(READ_SFLOW_ING_MIRROR_CONFIGr(unit, &reg_val));
    mc_enable = soc_reg_field_get(unit, SFLOW_ING_MIRROR_CONFIGr,
                                          reg_val, MIRROR_ENABLEf);

    /* Read mirror control structure to get programmed mtp indexes. */
    for (i = 0; i < BCM_MIRROR_MTP_COUNT; i++) {
        index_val[i] = soc_reg_field_get(unit, SFLOW_ING_MIRROR_CONFIGr,
                                          reg_val, mtp_idxf[i]);
    }
    /* Read mirror control register to check if mtp index is used. */
    for (mtp_slot = 0; mtp_slot < BCM_MIRROR_MTP_COUNT;
                          mtp_slot++) {

        mtp_bit = 1 << mtp_slot;
        /* Check if MTP slot is enabled on port */
        if (!(mc_enable & mtp_bit)) {
            continue;
        }
        /* MTP index is enabled and direction matches */
        mtp_index = index_val[mtp_slot];

        if (!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)++;
            MIRROR_DEST_REF_COUNT(unit,
               MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index))++;
        }
    }
    return BCM_E_NONE;
#else
    return BCM_E_NONE;
#endif
}

/*
 * Function:
 *      _bcm_mirror_vp_mtp_ref_count_recover
 * Purpose:
 *      Restores virtual port mtp usage reference counts for warm boot recovery
 * Parameters:
 *          unit        - (IN) BCM device number.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_mirror_vp_mtp_ref_count_recover(int unit)
{
    uint8 *mem_buf = NULL;
    int i, index_min, index_max;
    int mirror_enable;
    int rv;
    soc_mem_t mem;
    int j;
    void *entry_ptr;
    int mtp_index;
    struct {
        soc_mem_t mem;
        soc_field_t field;
        int egress; 
    } vp_mirror[2];

    vp_mirror[0].mem = SOURCE_VPm;
    vp_mirror[0].field = ING_MIRROR_ENABLEf;
    vp_mirror[0].egress = FALSE;

    if (soc_feature(unit, soc_feature_td2p_dvp_mirroring)) {
        mem = ING_DVP_2_TABLEm;
    } else {
        mem = ING_DVP_TABLEm;
    }
    vp_mirror[1].mem = mem;
    vp_mirror[1].field = EGR_MIRROR_ENABLEf;
    vp_mirror[1].egress = TRUE;

    for (j = 0; j < 2; j++) {
        if (SOC_MEM_IS_VALID(unit, vp_mirror[j].mem) && 
            SOC_MEM_FIELD_VALID(unit, vp_mirror[j].mem, vp_mirror[j].field)) {
            mem_buf = soc_cm_salloc(unit, SOC_MEM_TABLE_BYTES(unit, vp_mirror[j].mem),
                    "SDVP_buffer");
            if (NULL == mem_buf) {
                return BCM_E_MEMORY;
            }
            sal_memset(mem_buf, 0, SOC_MEM_TABLE_BYTES(unit, vp_mirror[j].mem));
            index_min = soc_mem_index_min(unit, vp_mirror[j].mem);
            index_max = soc_mem_index_max(unit, vp_mirror[j].mem);
            rv = soc_mem_read_range(unit, vp_mirror[j].mem, MEM_BLOCK_ANY,
                    index_min, index_max, mem_buf);
            if (SOC_FAILURE(rv)) {
                soc_cm_sfree(unit, mem_buf);
                return rv;
            }
    
            for (i = 0; i <= (index_max - index_min); i++) {
                entry_ptr = soc_mem_table_idx_to_pointer(unit, 
                          vp_mirror[j].mem, void *, mem_buf, i);
                mirror_enable = soc_mem_field32_get(unit, vp_mirror[j].mem,
                                   entry_ptr, vp_mirror[j].field);
                for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT; 
                                                          mtp_index++) {
                    if (mirror_enable & (1 << mtp_index)) {
                        if (!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                            if (vp_mirror[j].egress !=
                                MIRROR_CONFIG_SHARED_MTP(unit, mtp_index).egress) {
                                /* global mtp selection type(ingress/egress) doesn't match
                                 * the virtual port mtp usage type
                                 */
                                soc_cm_sfree(unit, mem_buf);
                                return BCM_E_INTERNAL;
                            }
                            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)++;
                            MIRROR_DEST_REF_COUNT(unit,
                            MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index))++;
                        }
                        MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)++;
                        MIRROR_DEST_REF_COUNT(unit,
                        MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index))++;
                    }
                }
            }
            soc_cm_sfree(unit, mem_buf);
        }
    }
    return BCM_E_NONE;
}

STATIC int
_bcm_esw_mirror_method_reinit(int unit)
{
    int rv;
    uint16 recovered_ver;
    soc_scache_handle_t scache_handle;
    uint8 *method_scache;

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_MIRROR, 1);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &method_scache,
                                 BCM_WB_DEFAULT_VERSION, &recovered_ver);
    if (BCM_E_NOT_FOUND == rv) {
        return BCM_E_NONE;
    } else if (BCM_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "mirror_method_scache error \n")));
        return rv;
    }
    if (method_scache != NULL) {
        /* Retrieve MTP method from scache */
        sal_memcpy(&(_bcm_mirror_mtp_method_init[unit]), method_scache,
                   sizeof(_bcm_mirror_mtp_method_init[unit]));
        method_scache += sizeof(_bcm_mirror_mtp_method_init[unit]);
    }
    return BCM_E_NONE;
}

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
STATIC int
_bcm_esw_mirror_dest_tunnel_flags_get(int unit, int egress, int offset,
                                      uint32 *flags)
{
    egr_im_mtp_index_entry_t egr_mtp_entry;
    uint32 profile_index = 0;
    soc_mem_t mtp_index_mem, encap_control_mem;
    egr_mirror_encap_control_entry_t control_entry;
    int optional_header;
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        /*
         * EGR_*_MTP_INDEX tables are arranged as blocks of
         * BCM_SWITCH_TRUNK_MAX_PORTCNT entries, with
         * BCM_MIRROR_MTP_COUNT sets. Check the
         * first entry of each block to see if the
         * encap enable is set. Track one reference count
         * for each set, by each destination type (IM, EM, TRUE EM)
         */
        if (egress) {
            mtp_index_mem = EGR_EM_MTP_INDEXm;
        } else {
            mtp_index_mem = EGR_IM_MTP_INDEXm;
        }

        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, mtp_index_mem, MEM_BLOCK_ANY,
                          offset, &egr_mtp_entry));
        if (soc_mem_field32_get(unit, mtp_index_mem,
                                &egr_mtp_entry,
                                MIRROR_ENCAP_ENABLEf)) {
            profile_index =
                soc_mem_field32_get(unit, mtp_index_mem,
                                    &egr_mtp_entry,
                                    MIRROR_ENCAP_INDEXf);
            if ((EGR_MIRROR_ENCAP(unit) == NULL) ||
                (EGR_MIRROR_ENCAP(unit)->tables == NULL)) {
                 return BCM_E_INIT;
            }
            encap_control_mem = EGR_MIRROR_ENCAP(unit)->tables[0].mem;
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, encap_control_mem, MEM_BLOCK_ANY,
                              profile_index, &control_entry));
            optional_header =
                soc_EGR_MIRROR_ENCAP_CONTROLm_field32_get(unit, &control_entry,
                                          RSPAN__ADD_OPTIONAL_HEADERf);
            if (soc_EGR_MIRROR_ENCAP_CONTROLm_field32_get(unit, &control_entry,
                               ENTRY_TYPEf) == BCM_TD_MIRROR_ENCAP_TYPE_SFLOW) {
                *flags |= BCM_MIRROR_DEST_TUNNEL_SFLOW;
            } else if (soc_EGR_MIRROR_ENCAP_CONTROLm_field32_get(unit, &control_entry,
                       ENTRY_TYPEf) == BCM_TD_MIRROR_ENCAP_TYPE_ERSPAN) {
                *flags |= BCM_MIRROR_DEST_TUNNEL_IP_GRE;
            } else {
                *flags |= BCM_MIRROR_DEST_TUNNEL_L2;
            }
            if (BCM_TD_MIRROR_HEADER_TRILL == optional_header) {
                *flags |= BCM_MIRROR_DEST_TUNNEL_TRILL;
            } else if (BCM_TD_MIRROR_HEADER_VNTAG == optional_header) {
                *flags |= BCM_MIRROR_DEST_TUNNEL_NIV;
            } else if (BCM_MIRROR_HEADER_ETAG == optional_header) {
                if(soc_feature(unit, soc_feature_port_extension)) {
                    *flags |= BCM_MIRROR_DEST_TUNNEL_ETAG;
                }
            }
        }
    }
    return BCM_E_NONE;
}
#endif /* BCM_TRIDENT_SUPPORT) || BCM_GREYHOUND_SUPPORT */

STATIC int
_bcm_esw_mirror_scache_version_incremental_size_get(int unit,
                                                    uint16 version,
                                                    int *alloc_size)
{
    int ing_mtp_count = 0, egr_mtp_count = 0;
    int alloc_sz = 0;
#ifdef BCM_TRIUMPH2_SUPPORT
    int egr_true_mtp_count = 0;
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (alloc_size == NULL) {
        return BCM_E_PARAM;
    }

    egr_mtp_count = BCM_MIRROR_MTP_COUNT;
    ing_mtp_count = BCM_MIRROR_MTP_COUNT;
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        egr_true_mtp_count = BCM_MIRROR_MTP_COUNT;
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (version == BCM_WB_VERSION_1_1) {
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            /* Slot-MTP mapping information for respective slot types */
            alloc_sz += (sizeof(bcm_gport_t) * BCM_MIRROR_MTP_COUNT) * BCM_MTP_SLOT_TYPES;
            /* Reference counter for respective slot types */
            alloc_sz += (sizeof(int) * BCM_MIRROR_MTP_COUNT) * BCM_MTP_SLOT_TYPES;
            /* BCM_MIRROR_DEST_FIELD flag */
            alloc_sz += sizeof(uint16);
            /* Reference counter for MTP slot */
            alloc_sz += sizeof(int) * BCM_MIRROR_MTP_COUNT;
            /* For ING MTP reference counter */
            alloc_sz += sizeof(int) * ing_mtp_count;
            /* For ING MTP reference counter */
            alloc_sz += sizeof(int) * egr_mtp_count;
            /* For destination reference counter */
            alloc_sz += sizeof(int) * (egr_mtp_count + ing_mtp_count);
#ifdef BCM_TRIUMPH2_SUPPORT
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                alloc_sz += sizeof(int) * egr_true_mtp_count;
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
            /* For pbmp_mtp_slot_used */
            alloc_sz += sizeof(bcm_pbmp_t) * BCM_MIRROR_MTP_COUNT;

            *alloc_size = alloc_sz;
            return BCM_E_NONE;
        } else {
            *alloc_size = 0;
            return BCM_E_NONE;
        }
    } else if (version == BCM_WB_VERSION_1_2) {
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            /* Save destination flags for flexible mtp mirror */
            alloc_sz += sizeof(uint32) * (egr_mtp_count + ing_mtp_count);
#ifdef BCM_TRIUMPH2_SUPPORT
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                alloc_sz += sizeof(uint32) * egr_true_mtp_count;
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
            *alloc_size = alloc_sz;
            return BCM_E_NONE;
        } else {
            *alloc_size = 0;
            return BCM_E_NONE;
        }
    } else if (version == BCM_WB_VERSION_1_3) {
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            /* Save destination flags for non-flexible mtp mirror */
            alloc_sz += sizeof(uint32) * BCM_MIRROR_MTP_COUNT;
#ifdef BCM_TRIUMPH2_SUPPORT
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                alloc_sz += sizeof(uint32) * egr_true_mtp_count;
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
            *alloc_size = alloc_sz;
            return BCM_E_NONE;
        } else {
            *alloc_size = 0;
            return BCM_E_NONE;
        }
    } else if (version == BCM_WB_VERSION_1_4) {
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            /* For Shared MTP ref_count */
            alloc_sz += sizeof(int) * BCM_MIRROR_MTP_COUNT;
            /* For Egress True MTP ref_count */
#ifdef BCM_TRIUMPH2_SUPPORT
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                alloc_sz += sizeof(int) * egr_true_mtp_count;
            }
#endif /* BCM_TRIUMPH2_SUPPORT */

            /* For destination ref_count */
            alloc_sz += sizeof(int) * BCM_MIRROR_MTP_COUNT;
#ifdef BCM_TRIUMPH2_SUPPORT
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                alloc_sz += sizeof(int) * egr_true_mtp_count;
            }
#endif /* BCM_TRIUMPH2_SUPPORT */

            /* For MIRROR_CONFIG_MODE */
            alloc_sz += sizeof(int);
            *alloc_size = alloc_sz;
            return BCM_E_NONE;
        } else {
            *alloc_size = 0;
            return BCM_E_NONE;
        }
    } else if (version == BCM_WB_VERSION_1_5) {
        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                /* For Ingress,Egress MTP destination gport */
                alloc_sz += sizeof(bcm_gport_t) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                            (ing_mtp_count + egr_mtp_count);
                /* For Ingress,Egress MTP destination flags */
                alloc_sz += sizeof(uint32) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                            (ing_mtp_count + egr_mtp_count);
            } else {
                /* For Shared MTP destination gport */
                alloc_sz += sizeof(bcm_gport_t) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                            BCM_MIRROR_MTP_COUNT;
                /* For Shared MTP destination flags */
                alloc_sz += sizeof(uint32) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                            BCM_MIRROR_MTP_COUNT;
            }
            *alloc_size = alloc_sz;
            return BCM_E_NONE;
        } else {
            *alloc_size = 0;
            return BCM_E_NONE;
        }
    }

    return BCM_E_NONE;
}

#if defined(BCM_TRIDENT_SUPPORT)
STATIC int
_bcm_td_mirror_destination_pri_recover(int unit,
                                       bcm_mirror_destination_t *mirror_dest,
                                       int offset,
                                       int flags)
{
    int mdest_flags = 0;
    egr_im_mtp_index_entry_t egr_im_mtp_index_entry;
    egr_em_mtp_index_entry_t egr_em_mtp_index_entry;
    egr_ep_redirect_em_mtp_index_entry_t egr_em_redirect_mtp_index_entry;

    if (mirror_dest == NULL) {
        return BCM_E_PARAM;
    }

    mdest_flags = mirror_dest->flags;
    if (mdest_flags & BCM_MIRROR_DEST_INT_PRI_SET) {
        if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                BCM_IF_ERROR_RETURN
                    (READ_EGR_IM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset,
                                            &egr_im_mtp_index_entry));
                mirror_dest->int_pri =
                    soc_EGR_IM_MTP_INDEXm_field32_get(unit,
                                                      &egr_im_mtp_index_entry,
                                                      NEW_INT_PRIf);
            }

            if (flags & BCM_MIRROR_PORT_EGRESS) {
                BCM_IF_ERROR_RETURN
                    (READ_EGR_EM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset,
                                            &egr_em_mtp_index_entry));
                mirror_dest->int_pri =
                    soc_EGR_EM_MTP_INDEXm_field32_get(unit,
                                                      &egr_em_mtp_index_entry,
                                                      NEW_INT_PRIf);
            }
        }

        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
                BCM_IF_ERROR_RETURN
                    (READ_EGR_EP_REDIRECT_EM_MTP_INDEXm(
                         unit, MEM_BLOCK_ANY, offset,
                         &egr_em_redirect_mtp_index_entry));
                mirror_dest->int_pri =
                    soc_EGR_EP_REDIRECT_EM_MTP_INDEXm_field32_get(
                        unit, &egr_em_redirect_mtp_index_entry, NEW_INT_PRIf);
            }
        }
    }

    return BCM_E_NONE;
}
#endif

#ifdef BCM_TRIDENT2_SUPPORT
/* Save MTP gport info of a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_gport_sync(int unit, bcm_gport_t mirror_dest_id,
                            uint8 *scache)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL; /* Current node */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;
    uint8 *scache_ptr = scache;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);

    if (mdest_cfg->ref_count > 0) {
        cur_dest = mdest_cfg->next;

        for (i = 0; i < max_portcnt; i++) {
            if (NULL == cur_dest) {
                break;
            }
            sal_memcpy(scache_ptr, &(cur_dest->mirror_dest.gport),
                       sizeof(bcm_gport_t));
            scache_ptr += sizeof(bcm_gport_t);
            cur_dest = cur_dest->next;
        }
    }
    return (BCM_E_NONE);
}

/* Save MTP flags info of a shared-id mirror destination. */
STATIC int
_bcm_mirror_dest_flags_sync(int unit, bcm_gport_t mirror_dest_id,
                            uint8 *scache)
{
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config head node */
    _bcm_mirror_dest_config_p cur_dest = NULL; /* Current node */
    int i = 0, max_portcnt = BCM_SWITCH_TRUNK_MAX_PORTCNT;
    uint8 *scache_ptr = scache;

    if (!BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);

    if (mdest_cfg->ref_count > 0) {
        cur_dest = mdest_cfg->next;

        for (i = 0; i < max_portcnt; i++) {
            if (NULL == cur_dest) {
                break;
            }
            sal_memcpy(scache_ptr, &(cur_dest->mirror_dest.flags),
                       sizeof(uint32));
            scache_ptr += sizeof(uint32);
            cur_dest = cur_dest->next;
        }
    }
    return (BCM_E_NONE);
}

/* Rtag recovery for shared-id mirror destination */
STATIC int
_bcm_td2_mirror_destination_rtag_recover(int unit,
                                         bcm_mirror_destination_t *mirror_dest,
                                         int offset,
                                         int flags)
{
    im_mtp_index_entry_t im_mtp_index_entry;
    em_mtp_index_entry_t em_mtp_index_entry;

    if (mirror_dest == NULL) {
        return BCM_E_PARAM;
    }

    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            BCM_IF_ERROR_RETURN
                (READ_IM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset,
                                    &im_mtp_index_entry));
            if (soc_IM_MTP_INDEXm_field32_get(unit,
                                              &im_mtp_index_entry,
                                              Tf)) {
                mirror_dest->rtag =
                    soc_IM_MTP_INDEXm_field32_get(unit,
                                                  &im_mtp_index_entry,
                                                  RTAGf);
            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS) {
            BCM_IF_ERROR_RETURN
                (READ_EM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset,
                                    &em_mtp_index_entry));
            if (soc_EM_MTP_INDEXm_field32_get(unit,
                                              &em_mtp_index_entry,
                                              Tf)) {
                mirror_dest->rtag =
                    soc_EM_MTP_INDEXm_field32_get(unit,
                                                  &em_mtp_index_entry,
                                                  RTAGf);
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  	_bcm_td2_mirror_shared_dest_recover
 * Purpose:
 *  	Recover mirror destination for shared-id mirror destination.
 * Parameters:
 *      unit        - (IN) BCM device number.
 *      flag        - (IN) Direction flag.
 *      mir_dest_id - (IN) Mirror dest id.
 *      dest_flags  - (IN) Mirror dest flag.
 *      mtp_gport   - (IN) Mirror dest gport array for mirror dest node.
 *      mtp_flags   - (IN) Mirror dest flags array for mirror dest node.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_td2_mirror_shared_dest_recover(int unit,
                                    int flag,
                                    bcm_gport_t mir_dest_id,
                                    int dest_flags,
                                    int mtp_index,
                                    bcm_gport_t *mtp_gport,
                                    int *mtp_flags)
{
    int i, offset = 0;
    bcm_mirror_destination_t mir_dest;
    egr_im_mtp_index_entry_t egr_mtp_entry;
    uint32 profile_index;
    uint8 found = FALSE;
    uint8 egress = 0;

    if (!soc_feature(unit, soc_feature_mirror_flexible)) {
        return (BCM_E_UNAVAIL);
    }

    if (!BCM_GPORT_IS_MIRROR(mir_dest_id)) {
        return (BCM_E_PARAM);
    }

    if (NULL == mtp_gport || NULL == mtp_flags) {
        return (BCM_E_PARAM);
    }

    if (flag & BCM_MIRROR_PORT_INGRESS) {
        egress = FALSE;
    } else if (flag & BCM_MIRROR_PORT_EGRESS) {
        egress = TRUE;
    } else {
        return (BCM_E_PARAM);
    }

    /* Alloc dest_id first */
    if (!MIRROR_DEST_REF_COUNT(unit, mir_dest_id)) {
        bcm_mirror_destination_t_init(&mir_dest);
        mir_dest.flags = dest_flags;
        mir_dest.flags |= BCM_MIRROR_DEST_WITH_ID;
        mir_dest.mirror_dest_id = mir_dest_id;

        BCM_IF_ERROR_RETURN(
            _bcm_td2_mirror_destination_rtag_recover(
                unit, &mir_dest, mtp_index, flag));

        BCM_IF_ERROR_RETURN(
            _bcm_esw_mirror_destination_create(unit, &mir_dest));
    }

    offset = mtp_index * BCM_SWITCH_TRUNK_MAX_PORTCNT;
    for (i = 0; i < BCM_SWITCH_TRUNK_MAX_PORTCNT; i++, offset++) {
         bcm_mirror_destination_t_init(&mir_dest);
         mir_dest.flags = mtp_flags[offset];

        if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_dest_tunnel_flags_get(
                     unit, egress,
                     offset,
                     &mir_dest.flags));
        }

        mir_dest.gport = mtp_gport[offset];
        if (!BCM_GPORT_IS_SET(mir_dest.gport)) {
            continue;
        }
        /* Adapt miror destination gport */
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_gport_adapt(unit, &(mir_dest.gport)));

        BCM_IF_ERROR_RETURN(
            _bcm_td_mirror_destination_pri_recover(
                unit, &mir_dest,
                offset, flag));

        if (!egress) {
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ANY,
                              offset, &egr_mtp_entry));
            if (soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                    &egr_mtp_entry,
                                    MIRROR_ENCAP_ENABLEf)) {
                profile_index =
                    soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                        &egr_mtp_entry,
                                        MIRROR_ENCAP_INDEXf);
                BCM_IF_ERROR_RETURN
                    (_bcm_egr_mirror_encap_entry_reference(
                         unit, profile_index));
                /* Tunnel type info recovery */
                BCM_IF_ERROR_RETURN
                    (_bcm_td_mirror_tunnel_reload(unit,
                                                  &mir_dest,
                                                  profile_index));
            }
        } else {
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ANY,
                              offset, &egr_mtp_entry));
            if (soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                    &egr_mtp_entry,
                                    MIRROR_ENCAP_ENABLEf)) {
                profile_index =
                    soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                        &egr_mtp_entry,
                                        MIRROR_ENCAP_INDEXf);
                BCM_IF_ERROR_RETURN
                    (_bcm_egr_mirror_encap_entry_reference(
                         unit, profile_index));
                /* Tunnel type info recovery */
                BCM_IF_ERROR_RETURN
                    (_bcm_td_mirror_tunnel_reload(unit,
                                                  &mir_dest,
                                                  profile_index));
            }
        }

        /* We have scratch memory of the destination IDs */
        mir_dest.mirror_dest_id = mir_dest_id;
        mir_dest.flags |= BCM_MIRROR_DEST_WITH_ID |
                          BCM_MIRROR_DEST_MTP_ADD;
        found = FALSE;
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_dest_mtp_search(
                unit,
                mir_dest.mirror_dest_id,
                mir_dest.gport,
                &found));
        if (!found) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_mirror_destination_create(
                    unit, &mir_dest));
        }
    }

    if(!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index) =
            mir_dest_id;
        MIRROR_CONFIG_SHARED_MTP(unit, mtp_index).egress =
            (0 != (flag & BCM_MIRROR_PORT_EGRESS));
    } else {
        if (!egress) {
            MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index) =
                mir_dest_id;
        } else {
            MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_index) =
                mir_dest_id;
        }
    }

    return (BCM_E_NONE);
}

#endif /* BCM_TRIDENT2_SUPPORT */

STATIC int
_bcm_esw_directed_flexible_mirror_recover(int unit)
{
    soc_scache_handle_t       scache_handle;
    int                       rv, idx, slot;
    uint16                    recovered_ver, dest_field_bmp;
    uint8                     *mtp_scache_p = NULL;
    _bcm_mirror_dest_config_p  mdest = NULL;

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_MIRROR, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &mtp_scache_p, BCM_WB_DEFAULT_VERSION,
                                 &recovered_ver);
    if (BCM_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit, "mtp_scache error \n")));
        return rv;
    }
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        if (recovered_ver >= BCM_WB_VERSION_1_1) {
            /* Skip ING/EGR mtp destination id */
            mtp_scache_p += sizeof(bcm_gport_t) * (MIRROR_CONFIG(unit)->ing_mtp_count);
            mtp_scache_p += sizeof(bcm_gport_t) * (MIRROR_CONFIG(unit)->egr_mtp_count);
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                mtp_scache_p += sizeof(bcm_gport_t) *
                                (MIRROR_CONFIG(unit)->egr_true_mtp_count);
            }
            /* Slot-MTP mapping information for respective slot types */
            for (idx = BCM_MTP_SLOT_TYPE_PORT; idx < BCM_MTP_SLOT_TYPES; idx++) {
                for (slot = 0; slot < MIRROR_CONFIG(unit)->mtp_slot_count[idx];
                     slot++) {
                    sal_memcpy(&MIRROR_CONFIG_TYPE_MTP_SLOT(unit, slot, idx),
                               mtp_scache_p,
                               sizeof(bcm_gport_t));
                    mtp_scache_p += sizeof(bcm_gport_t);
                }
            }
            /* Reference counter for respective slot types */
            for (idx = BCM_MTP_SLOT_TYPE_PORT; idx < BCM_MTP_SLOT_TYPES; idx++) {
                for (slot = 0; slot < MIRROR_CONFIG(unit)->mtp_slot_count[idx];
                     slot++) {
                    sal_memcpy(&MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, slot, idx),
                               mtp_scache_p,
                               sizeof(int));
                    mtp_scache_p += sizeof(int);
                }
            }
            /* BCM_MIRROR_DEST_FIELD flag */
            sal_memcpy(&dest_field_bmp, mtp_scache_p, sizeof(uint16));
            mtp_scache_p += sizeof(uint16);
            for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
                mdest = MIRROR_CONFIG(unit)->dest_arr + idx;;
                if (dest_field_bmp & (1 << idx)) {
                    mdest->mirror_dest.flags |= BCM_MIRROR_DEST_FIELD;
                }
            }
            /* Reference counter for MTP slot */
            for (slot = 0; slot < BCM_MIRROR_MTP_COUNT; slot++) {
                sal_memcpy(&MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, slot),
                           mtp_scache_p,
                           sizeof(int));
                mtp_scache_p += sizeof(int);
            }
            /* Reference counter for ING MTP */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
                sal_memcpy(&MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx),
                           mtp_scache_p, sizeof(int));
                mtp_scache_p += sizeof(int);
            }
            /* Reference counter for EGR MTP */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                sal_memcpy(&MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx),
                           mtp_scache_p,
                           sizeof(int));
                mtp_scache_p += sizeof(int);
            }
            /* Reference counter for destination */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
                mdest = MIRROR_CONFIG(unit)->dest_arr + idx;
                sal_memcpy(&(mdest->ref_count), mtp_scache_p, sizeof(int));
                mtp_scache_p += sizeof(int);
            }
            /* Port bitmap for MTP slot used */
            for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                sal_memcpy(&MIRROR_CONFIG_PBMP_MTP_SLOT_USED(unit, idx),
                           mtp_scache_p,
                           sizeof(bcm_pbmp_t));
                mtp_scache_p += sizeof(bcm_pbmp_t);
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *  	_bcm_esw_mirror_reload 
 * Purpose:
 *  	Restores mirroring destination for warm boot recovery
 * Parameters:
 *	    unit        - (IN) BCM device number.
 *      directed    - (IN) indication if directed mirroring is used.
 * Returns:
 *  	BCM_E_XXX
 */


STATIC int
_bcm_esw_mirror_reload(int unit, int directed)
{
    soc_scache_handle_t       scache_handle;
    uint8                     *mtp_scache, *mtp_scache_p = NULL;
    int                       mc_enable, enable, enabled = FALSE;
    int	                      idx, port_ix, flags, rv;
    bcm_module_t              modid = 0;
    bcm_gport_t               gport;
    bcm_gport_t               mtp_gport[3 * BCM_MIRROR_MTP_COUNT] = {0};
    /* Max MTP * mirror types (ING, EGR, TRUE EGR) */
    bcm_mirror_destination_t  mirror_dest;
    uint32                    reg_val = 0;
    int                       stale_scache = FALSE;
#ifdef BCM_TRIUMPH2_SUPPORT
    uint32                    ms_reg; /* MTP mode register value     */
    int                       mtp_type = 0, mtp_index;
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
    mirror_control_entry_t mc_entry; /* MTP control memory value */
#endif /* BCM_TRIDENT_SUPPORT */
    bcm_pbmp_t                all_pbmp;
    uint16                    recovered_ver = 0;
    uint32                    dest_flags[3 * BCM_MIRROR_MTP_COUNT] = {0};
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    int                       max_num_trunk_ports = 0;
    int                       mtp_port_count = 0;
#if defined(BCM_METROLITE_SUPPORT)
    uint16 dev_id;
    uint8 rev_id;
#endif

#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT
    uint32                    non_flexible_mtp_dest_flags[2 * BCM_MIRROR_MTP_COUNT] = {0};
#endif /* BCM_TRIUMPH2_SUPPORT */
    int                       extra_scache_size = 0;
#ifdef BCM_TRIUMPH2_SUPPORT
    int shared_mtp_ref_count[BCM_MIRROR_MTP_COUNT] = {0};
    int egr_true_mtp_ref_count[BCM_MIRROR_MTP_COUNT] = {0};
    int mirror_dest_ref_count[3 * BCM_MIRROR_MTP_COUNT] = {0};
    int mirror_config_mode = 0;
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_TRIDENT2_SUPPORT
    bcm_gport_t  ing_mtp_dest_gport
                     [BCM_MIRROR_MTP_COUNT*BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
    bcm_gport_t  egr_mtp_dest_gport
                     [BCM_MIRROR_MTP_COUNT*BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
    bcm_gport_t  shared_mtp_dest_gport
                     [BCM_MIRROR_MTP_COUNT*BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
    bcm_gport_t  ing_mtp_dest_flags
                     [BCM_MIRROR_MTP_COUNT*BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
    bcm_gport_t  egr_mtp_dest_flags
                     [BCM_MIRROR_MTP_COUNT*BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
    bcm_gport_t  shared_mtp_dest_flags
                     [BCM_MIRROR_MTP_COUNT*BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
#endif /* BCM_TRIDENT2_SUPPORT */

#ifdef BCM_XGS12_FABRIC_SUPPORT
    if (SOC_IS_XGS12_FABRIC(unit)) {
        return _bcm_xgs12_fabric_mirror_reinit(unit);
    }
#endif /* BCM_XGS12_FABRIC_SUPPORT */

    BCM_PBMP_CLEAR(all_pbmp);
    BCM_PBMP_ASSIGN(all_pbmp, PBMP_ALL(unit));
#ifdef BCM_KATANA2_SUPPORT
    if (SOC_IS_KATANA2(unit) && soc_feature(unit, soc_feature_flex_port)) {
        BCM_IF_ERROR_RETURN(_bcm_kt2_flexio_pbmp_update(unit, &all_pbmp));
    }
    if (soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe)) {
        _bcm_kt2_subport_pbmp_update(unit, &all_pbmp);
    }
#endif

    if (SOC_IS_XGS3_SWITCH(unit)) {
        PBMP_ITER(all_pbmp, port_ix) {
            /* Higig port should never drop directed mirror packets
               so setting is always enabled and need not to be considered here*/
            if (IS_ST_PORT(unit, port_ix)) {
                continue;
            }
#if defined(BCM_TRIDENT_SUPPORT)
            if (soc_feature(unit, soc_feature_mirror_control_mem)) {
                BCM_IF_ERROR_RETURN
                    (READ_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY,
                                          port_ix, &mc_entry));
                mc_enable = soc_MIRROR_CONTROLm_field32_get(unit, &mc_entry,
                                                             M_ENABLEf);
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            {
                BCM_IF_ERROR_RETURN
                    (READ_MIRROR_CONTROLr(unit, port_ix, &reg_val));
                mc_enable = soc_reg_field_get(unit, MIRROR_CONTROLr,
                                              reg_val, M_ENABLEf);
            }
            if (mc_enable) {
                enabled = TRUE;
                break;
            }
        }
        
        MIRROR_CONFIG_MODE(unit) =
            enabled ? BCM_MIRROR_L2 : BCM_MIRROR_DISABLE;
    }    

    /* Recover stored destination gports, if available */
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_MIRROR, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &mtp_scache, BCM_WB_DEFAULT_VERSION,
                                 &recovered_ver);

    if (BCM_E_NOT_FOUND == rv) {
        mtp_scache = NULL;
    } else if (BCM_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "mtp_scache error \n")));
        return rv;
    } else {
        mtp_scache_p = mtp_scache;
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                sal_memcpy(&(mtp_gport[idx]),
                           mtp_scache_p,
                           sizeof(bcm_gport_t));
                mtp_scache_p += sizeof(bcm_gport_t);
            }
        } else {
            for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
                sal_memcpy(&(mtp_gport[idx]),
                           mtp_scache_p,
                           sizeof(bcm_gport_t));
                mtp_scache_p += sizeof(bcm_gport_t);
            }

            for (idx = 0;
                 idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                sal_memcpy(&(mtp_gport[idx + BCM_MIRROR_MTP_COUNT]),
                           mtp_scache_p,
                           sizeof(bcm_gport_t));
                mtp_scache_p += sizeof(bcm_gport_t);
            }
        }
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            for (idx = 0;
                 idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                sal_memcpy(&(mtp_gport[idx + (2 * BCM_MIRROR_MTP_COUNT)]),
                           mtp_scache_p,
                           sizeof(bcm_gport_t));
                mtp_scache_p += sizeof(bcm_gport_t);
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

        /* Recover destination flags */
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            if (recovered_ver >= BCM_WB_VERSION_1_2) {
                int alloc_sz = 0;

                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_scache_version_incremental_size_get(unit,
                                                             BCM_WB_VERSION_1_1,
                                                             &alloc_sz));

                /* Directed flexible mirror recovery is done later, skip it to recover flags first */
                mtp_scache_p += alloc_sz;
                for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
                    sal_memcpy(&(dest_flags[idx]),
                               mtp_scache_p,
                               sizeof(uint32));
                    mtp_scache_p += sizeof(uint32);
                }

                for (idx = 0;
                     idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                    sal_memcpy(&(dest_flags[idx + BCM_MIRROR_MTP_COUNT]),
                               mtp_scache_p,
                               sizeof(uint32));
                    mtp_scache_p += sizeof(uint32);
                }

#ifdef BCM_TRIUMPH2_SUPPORT
                 if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                     for (idx = 0;
                          idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                         sal_memcpy(&(dest_flags[idx + (2 * BCM_MIRROR_MTP_COUNT)]),
                                    mtp_scache_p,
                                    sizeof(uint32));
                         mtp_scache_p += sizeof(uint32);
                     }
                 }
#endif /* BCM_TRIUMPH2_SUPPORT */
            } else {
                int alloc_sz = 0;
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_scache_version_incremental_size_get(unit,
                                                             BCM_WB_VERSION_1_2,
                                                             &alloc_sz));
                extra_scache_size += alloc_sz;
            }
        }

#ifdef BCM_TRIUMPH2_SUPPORT
        /* Recover destination flags for non-flexible mtp mirror */
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            if (recovered_ver >= BCM_WB_VERSION_1_3) {
                /* Recover flags of shared mtp destination */
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                    sal_memcpy(&(non_flexible_mtp_dest_flags[idx]),
                               mtp_scache_p,
                               sizeof(uint32));
                    mtp_scache_p += sizeof(uint32);
                }

                /* Recover flags of egress true mtp destination */
                if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                    for (idx = 0;
                         idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                        sal_memcpy(&(non_flexible_mtp_dest_flags[idx + BCM_MIRROR_MTP_COUNT]),
                                   mtp_scache_p,
                                   sizeof(uint32));
                        mtp_scache_p += sizeof(uint32);
                    }
                 }
            } else {
                int alloc_sz = 0;
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_scache_version_incremental_size_get(unit,
                                                             BCM_WB_VERSION_1_3,
                                                             &alloc_sz));
                extra_scache_size += alloc_sz;
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit) &&
            NULL != mtp_scache_p) {
            if (recovered_ver >= BCM_WB_VERSION_1_4) {
                /* Recover Shared MTP ref_count */
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                    sal_memcpy(&(shared_mtp_ref_count[idx]),
                               mtp_scache_p,
                               sizeof(int));
                    mtp_scache_p += sizeof(int);
                }

                /* Recover Egress True MTP ref_count */
                if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                    for (idx = 0;
                         idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                        sal_memcpy(&(egr_true_mtp_ref_count[idx]),
                                   mtp_scache_p,
                                   sizeof(int));
                        mtp_scache_p += sizeof(int);
                    }
                }

                /* Recover Mirror Destination ref_count */
                for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
                    sal_memcpy(&(mirror_dest_ref_count[idx]),
                               mtp_scache_p, sizeof(int));
                    mtp_scache_p += sizeof(int);
                }

                /* Recover MIRROR_CONFIG_MODE */
                sal_memcpy(&mirror_config_mode, mtp_scache_p, sizeof(int));
                mtp_scache_p += sizeof(int);
            } else {
                int alloc_sz = 0;
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_scache_version_incremental_size_get(unit,
                                                             BCM_WB_VERSION_1_4,
                                                             &alloc_sz));
                extra_scache_size += alloc_sz;
            }
        }
#endif

#ifdef BCM_TRIDENT2_SUPPORT
        /* Recover MTP destination gport. */
        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            if (recovered_ver >= BCM_WB_VERSION_1_5) {
                if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                    /* Recover Ingress MTP destination gport. */
                    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT *
                         MIRROR_CONFIG(unit)->ing_mtp_count;
                         idx++) {
                        sal_memcpy(&(ing_mtp_dest_gport[idx]),
                                   mtp_scache_p,
                                   sizeof(bcm_gport_t));
                        mtp_scache_p += sizeof(bcm_gport_t);
                    }

                    /* Recover Egress MTP destination gport. */
                    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT *
                         MIRROR_CONFIG(unit)->egr_mtp_count;
                         idx++) {
                        sal_memcpy(&(egr_mtp_dest_gport[idx]),
                                   mtp_scache_p,
                                   sizeof(bcm_gport_t));
                        mtp_scache_p += sizeof(bcm_gport_t);
                    }

                    /* Recover Ingress MTP destination flags. */
                    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT *
                         MIRROR_CONFIG(unit)->ing_mtp_count;
                         idx++) {
                        sal_memcpy(&(ing_mtp_dest_flags[idx]),
                                   mtp_scache_p,
                                   sizeof(bcm_gport_t));
                        mtp_scache_p += sizeof(bcm_gport_t);
                    }

                    /* Recover Egress MTP destination flags. */
                    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT *
                         MIRROR_CONFIG(unit)->egr_mtp_count;
                         idx++) {
                        sal_memcpy(&(egr_mtp_dest_flags[idx]),
                                   mtp_scache_p,
                                   sizeof(bcm_gport_t));
                        mtp_scache_p += sizeof(bcm_gport_t);
                    }
                } else {
                    /* Recover Shared MTP destination gport. */
                    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT *
                         BCM_MIRROR_MTP_COUNT;
                         idx++) {
                        sal_memcpy(&(shared_mtp_dest_gport[idx]),
                                   mtp_scache_p,
                                   sizeof(bcm_gport_t));
                        mtp_scache_p += sizeof(bcm_gport_t);
                    }

                    /* Recover Shared MTP destination flags. */
                    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT *
                         BCM_MIRROR_MTP_COUNT;
                         idx++) {
                        sal_memcpy(&(shared_mtp_dest_flags[idx]),
                                   mtp_scache_p,
                                   sizeof(bcm_gport_t));
                        mtp_scache_p += sizeof(bcm_gport_t);
                    }
                }
            } else {
                int alloc_sz = 0;
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_scache_version_incremental_size_get(unit,
                                                             BCM_WB_VERSION_1_5,
                                                             &alloc_sz));
                extra_scache_size += alloc_sz;
            }
        }
#endif /* BCM_TRIDENT2_SUPPORT */
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &ms_reg));
        mtp_type = soc_reg_field_get(unit, MIRROR_SELECTr, ms_reg, MTP_TYPEf);
        /* ing_mtp_count works for both ingress and egress in shared mode */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
            bcm_mirror_destination_t_init(&mirror_dest);
            mirror_dest.flags = non_flexible_mtp_dest_flags[idx];
            flags = (mtp_type & (1 << idx)) ?
                     BCM_MIRROR_PORT_EGRESS : BCM_MIRROR_PORT_INGRESS;

            if (!(mirror_dest.flags & BCM_MIRROR_DEST_ID_SHARE)) {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_mtp_to_modport(unit, idx, TRUE, flags,
                                                    &modid, &gport));

                if (mirror_dest.flags & BCM_MIRROR_DEST_REPLACE) {
                    if (BCM_GPORT_IS_SET(gport)) {
                        mirror_dest.gport = gport;
                    } else {
                        _bcm_gport_dest_t gport_st;
                        gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
                        gport_st.modid = 0;
                        gport_st.port = gport;
                        BCM_IF_ERROR_RETURN(
                            _bcm_esw_gport_construct(unit,
                                                     &gport_st,
                                                     &(mirror_dest.gport)));
                    }
                    /* Adapt miror destination gport */
                    BCM_IF_ERROR_RETURN(
                        _bcm_mirror_gport_adapt(unit, &(mirror_dest.gport)));
                    rv = BCM_E_NOT_FOUND;
                } else {
                    rv = _bcm_esw_mirror_destination_find(unit, gport, 0,
                                                          flags, &mirror_dest);
                }
                if (BCM_E_NOT_FOUND == rv) {
                    if ((NULL != mtp_scache) && !stale_scache) {
                        if (BCM_GPORT_IS_MIRROR(mtp_gport[idx])) {
                            /* We have scratch memory of the destination IDs */
                            mirror_dest.mirror_dest_id = mtp_gport[idx];
                            mirror_dest.flags |= BCM_MIRROR_DEST_WITH_ID;
                            BCM_IF_ERROR_RETURN
                                (_bcm_esw_mirror_destination_create(unit,
                                                                    &mirror_dest));
                        } /* Else, we know there isn't an MTP here */
                    } else {
                        BCM_IF_ERROR_RETURN
                            (_bcm_esw_mirror_destination_create(unit, &mirror_dest));
                    }
                } else if (BCM_FAILURE(rv)) {
                    return rv;
                } else if ((NULL != mtp_scache) && !stale_scache &&
                           (BCM_GPORT_IS_MIRROR(mtp_gport[idx])) &&
                           (mirror_dest.mirror_dest_id != mtp_gport[idx])) {
                    /* Warm Boot Level 2, the destination doesn't match! */
                    SOC_IF_ERROR_RETURN
                        (soc_event_generate(unit, SOC_SWITCH_EVENT_STABLE_ERROR,
                                            SOC_STABLE_STALE, 0, 0));
                    stale_scache = TRUE;
                }
                if (BCM_GPORT_IS_MIRROR(mirror_dest.mirror_dest_id)) {
                    MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx) =
                        mirror_dest.mirror_dest_id;
                    if (!directed) {
                        MIRROR_CONFIG_SHARED_MTP(unit, idx).egress = FALSE;
                        MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
                        /* Egress update */
                        MIRROR_CONFIG_SHARED_MTP_DEST(unit,
                               BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX) =
                            mirror_dest.mirror_dest_id;
                        MIRROR_CONFIG_SHARED_MTP(unit,
                            BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX).egress = TRUE;
                        MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit,
                                      BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX)++;
                    } else {
                        MIRROR_CONFIG_SHARED_MTP(unit, idx).egress =
                            (0 != (flags & BCM_MIRROR_PORT_EGRESS));
                    }
                }
            }
#ifdef BCM_TRIDENT2_SUPPORT
            else {
                if ((NULL != mtp_scache) && !stale_scache) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_td2_mirror_shared_dest_recover(
                            unit, flags, mtp_gport[idx],
                            mirror_dest.flags, idx,
                            shared_mtp_dest_gport, shared_mtp_dest_flags));
                }
            }
#endif /* BCM_TRIDENT2_SUPPORT */
        }
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
    {
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
            bcm_mirror_destination_t_init(&mirror_dest);
            mirror_dest.flags = dest_flags[idx];

            if (!(mirror_dest.flags & BCM_MIRROR_DEST_ID_SHARE)) {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_mtp_to_modport(unit, idx, TRUE,
                                     BCM_MIRROR_PORT_INGRESS, &modid, &gport));

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
                if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
                    int max_num_trunk_ports;
#ifdef BCM_METROLITE_SUPPORT
                    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
                        max_num_trunk_ports = 4;
                    } else
#endif
                    {
                        max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
                    }

                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_dest_tunnel_flags_get(
                             unit, FALSE,
                             idx * max_num_trunk_ports,
                             &mirror_dest.flags));
                }
#endif

                if (BCM_GPORT_IS_SET(gport)) {
                    mirror_dest.gport = gport;
                } else {
                    _bcm_gport_dest_t gport_st;
                    gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
                    gport_st.modid = 0;
                    gport_st.port = gport;
                    BCM_IF_ERROR_RETURN(
                        _bcm_esw_gport_construct(unit, &gport_st,
                                                 &(mirror_dest.gport)));
                }
                /* Adapt miror destination gport */
                BCM_IF_ERROR_RETURN(
                    _bcm_mirror_gport_adapt(unit, &(mirror_dest.gport)));

#if defined(BCM_TRIDENT_SUPPORT)
                if (soc_feature(unit, soc_feature_mirror_flexible) &&
                    MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                    int max_num_trunk_ports;
#ifdef BCM_METROLITE_SUPPORT
                    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
                        max_num_trunk_ports = 4;
                    } else
#endif
                    {
                        max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
                    }

                    BCM_IF_ERROR_RETURN(
                        _bcm_td_mirror_destination_pri_recover(
                            unit, &mirror_dest,
                            idx * max_num_trunk_ports,
                            BCM_MIRROR_PORT_INGRESS));
                }
#endif

                if ((NULL != mtp_scache) && !stale_scache) {
                    if (BCM_GPORT_IS_MIRROR(mtp_gport[idx])) {
                        /* We have scratch memory of the destination IDs */
                        mirror_dest.mirror_dest_id = mtp_gport[idx];
                        mirror_dest.flags |= BCM_MIRROR_DEST_WITH_ID;
                        if (0 == MIRROR_DEST_REF_COUNT(unit, mirror_dest.mirror_dest_id)) {
                            BCM_IF_ERROR_RETURN(
                                _bcm_esw_mirror_destination_create(
                                    unit, &mirror_dest));
                        }
                    } /* Else, we know there isn't an MTP here */
                } else {
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_destination_create(unit, &mirror_dest));
                }


                if (BCM_GPORT_IS_MIRROR(mirror_dest.mirror_dest_id)) {
                    MIRROR_CONFIG_ING_MTP_DEST(unit, idx) =
                        mirror_dest.mirror_dest_id;
                    if (!directed) {
                        MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)++;
                    }
                }
            }
#ifdef BCM_TRIDENT2_SUPPORT
            else {
                if ((NULL != mtp_scache) && !stale_scache) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_td2_mirror_shared_dest_recover(
                            unit, BCM_MIRROR_PORT_INGRESS, mtp_gport[idx],
                            mirror_dest.flags, idx,
                            ing_mtp_dest_gport, ing_mtp_dest_flags));
                }
            }
#endif /* BCM_TRIDENT2_SUPPORT */
        }

        for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
            bcm_mirror_destination_t_init(&mirror_dest);
            mirror_dest.flags = dest_flags[idx + BCM_MIRROR_MTP_COUNT];

            if (!(mirror_dest.flags & BCM_MIRROR_DEST_ID_SHARE)) {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_mtp_to_modport(unit, idx, TRUE,
                                     BCM_MIRROR_PORT_EGRESS, &modid, &gport));

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
                if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
                    int max_num_trunk_ports;
#ifdef BCM_METROLITE_SUPPORT
                    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
                        max_num_trunk_ports = 4;
                    } else
#endif
                    {
                        max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
                    }

                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_dest_tunnel_flags_get(
                             unit, TRUE,
                             idx * max_num_trunk_ports,
                             &mirror_dest.flags));
                }
#endif

                if (BCM_GPORT_IS_SET(gport)) {
                    mirror_dest.gport = gport;
                } else {
                    _bcm_gport_dest_t gport_st;
                    gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
                    gport_st.modid = 0;
                    gport_st.port = gport;
                    BCM_IF_ERROR_RETURN(
                        _bcm_esw_gport_construct(unit, &gport_st, &(mirror_dest.gport)));
                }
                /* Adapt miror destination gport */
                BCM_IF_ERROR_RETURN(
                    _bcm_mirror_gport_adapt(unit, &(mirror_dest.gport)));

#if defined(BCM_TRIDENT_SUPPORT)
                if (soc_feature(unit, soc_feature_mirror_flexible) &&
                    MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                    int max_num_trunk_ports;
#ifdef BCM_METROLITE_SUPPORT
                    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
                        max_num_trunk_ports = 4;
                    } else
#endif
                    {
                        max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
                    }
                    BCM_IF_ERROR_RETURN(
                        _bcm_td_mirror_destination_pri_recover(
                            unit, &mirror_dest,
                            idx * max_num_trunk_ports,
                            BCM_MIRROR_PORT_EGRESS));
                }
#endif

                if ((NULL != mtp_scache) && !stale_scache) {
                    if (BCM_GPORT_IS_MIRROR(mtp_gport[idx +
                                                      BCM_MIRROR_MTP_COUNT])) {
                        rv = bcm_esw_mirror_destination_get(unit,
                                mtp_gport[idx + BCM_MIRROR_MTP_COUNT], &mirror_dest);
                        if (rv == BCM_E_NOT_FOUND) {
                            /* We have scratch memory of the destination IDs */
                            mirror_dest.mirror_dest_id = mtp_gport[idx +
                                BCM_MIRROR_MTP_COUNT];
                            mirror_dest.flags |= BCM_MIRROR_DEST_WITH_ID;
                            rv = _bcm_esw_mirror_destination_create(unit,
                                                                    &mirror_dest);
                            BCM_IF_ERROR_RETURN(rv);
                        }
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
                        if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
                            if (mirror_dest.flags & BCM_MIRROR_DEST_TUNNELS) {
                                if (BCM_FAILURE(rv) && (rv != BCM_E_EXISTS)) {
                                    return rv;
                                }
                            } else {
                                if (BCM_FAILURE(rv)) {
                                    return rv;
                                }
                            }
                        } else {
                            if (BCM_FAILURE(rv)) {
                                return rv;
                            }
                        }
#else
                        if (BCM_FAILURE(rv)) {
                                return rv;
                        }
#endif
                    } /* Else, we know there isn't an MTP here */
                } else {
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_destination_create(unit, &mirror_dest));
                }

                if (BCM_GPORT_IS_MIRROR(mirror_dest.mirror_dest_id)) {
                    MIRROR_CONFIG_EGR_MTP_DEST(unit, idx) =
                        mirror_dest.mirror_dest_id;
                    if (!directed) {
                        MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)++;
                    }
                }
            }
#ifdef BCM_TRIDENT2_SUPPORT
            else {
                if ((NULL != mtp_scache) && !stale_scache) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_td2_mirror_shared_dest_recover(
                            unit, BCM_MIRROR_PORT_EGRESS,
                            mtp_gport[idx + BCM_MIRROR_MTP_COUNT],
                            mirror_dest.flags, idx,
                            egr_mtp_dest_gport, egr_mtp_dest_flags));
                }
            }
#endif /* BCM_TRIDENT2_SUPPORT */
        }
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true) && directed) {
        for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
            BCM_IF_ERROR_RETURN 
                (_bcm_esw_mirror_mtp_to_modport(unit, idx, TRUE,
                          BCM_MIRROR_PORT_EGRESS_TRUE, &modid, &gport));
            bcm_mirror_destination_t_init(&mirror_dest);
            if (soc_feature(unit, soc_feature_mirror_flexible) &&
                MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                mirror_dest.flags = dest_flags[idx + (2 * BCM_MIRROR_MTP_COUNT)];
            } else {
                mirror_dest.flags = non_flexible_mtp_dest_flags[idx + BCM_MIRROR_MTP_COUNT];
            }

            if (BCM_GPORT_IS_SET(gport)) {
                mirror_dest.gport = gport;
            } else {
                _bcm_gport_dest_t gport_st;
                gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
                gport_st.modid = 0;
                gport_st.port = gport;
                BCM_IF_ERROR_RETURN(
                    _bcm_esw_gport_construct(unit,
                                             &gport_st,
                                             &(mirror_dest.gport)));
            }
            /* Adapt miror destination gport */
            BCM_IF_ERROR_RETURN(
                _bcm_mirror_gport_adapt(unit, &(mirror_dest.gport)));


#if defined(BCM_TRIDENT_SUPPORT)
            if (soc_feature(unit, soc_feature_mirror_flexible) &&
                MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                int max_num_trunk_ports;
#ifdef BCM_METROLITE_SUPPORT
                if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
                    max_num_trunk_ports = 4;
                } else
#endif
                {
                    max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
                }

                BCM_IF_ERROR_RETURN(
                    _bcm_td_mirror_destination_pri_recover(
                        unit, &mirror_dest,
                        idx * max_num_trunk_ports,
                        BCM_MIRROR_PORT_EGRESS_TRUE));
            }
#endif
            if ((NULL != mtp_scache) && !stale_scache) {
                if (BCM_GPORT_IS_MIRROR(mtp_gport[idx +
                                            (2 * BCM_MIRROR_MTP_COUNT)])) {
                    rv = bcm_esw_mirror_destination_get(unit,
                            mtp_gport[idx + (2 * BCM_MIRROR_MTP_COUNT)], &mirror_dest);
                    if (rv == BCM_E_NOT_FOUND) {
                        /* We have scratch memory of the destination IDs */
                        mirror_dest.mirror_dest_id =
                            mtp_gport[idx + (2 * BCM_MIRROR_MTP_COUNT)];
                        mirror_dest.flags |= BCM_MIRROR_DEST_WITH_ID;
                        rv = _bcm_esw_mirror_destination_create(unit,
                                                                &mirror_dest);
                        BCM_IF_ERROR_RETURN(rv);
                    }
                } /* Else, we know there isn't an MTP here */
            } else {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_destination_create(unit,
                                                        &mirror_dest));
            }

            if (BCM_GPORT_IS_MIRROR(mirror_dest.mirror_dest_id)) {
                MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx) =
                    mirror_dest.mirror_dest_id;
                if (!directed) {
                    MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)++;
                }
            }
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (directed) {
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &ms_reg));
            mtp_type = soc_reg_field_get(unit, MIRROR_SELECTr, ms_reg, MTP_TYPEf);
            /* In directed flexible mirror, the global MTP ingress or egress
             * status is recorded via MIRROR_CONFIG_MTP_MODE_BMP.
             * MIRROR_CONFIG_MTP_MODE_BMP(unit) should be recovered
             * from register MIRROR_SELECT.
             */
            if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                MIRROR_CONFIG_MTP_MODE_BMP(unit) = mtp_type;
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

        PBMP_ITER(all_pbmp, port_ix) {
#if defined(BCM_TRIDENT_SUPPORT)
            if (soc_feature(unit, soc_feature_mirror_control_mem)) {
                BCM_IF_ERROR_RETURN
                    (READ_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY,
                                          port_ix, &mc_entry));
                mc_enable = soc_MIRROR_CONTROLm_field32_get(unit, &mc_entry,
                                                             M_ENABLEf);
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            {
                BCM_IF_ERROR_RETURN
                    (READ_MIRROR_CONTROLr(unit, port_ix, &reg_val));
                mc_enable = soc_reg_field_get(unit, MIRROR_CONTROLr,
                                              reg_val, M_ENABLEf);
            }
            if (mc_enable) {
#ifdef BCM_TRIUMPH2_SUPPORT
                if (soc_feature(unit, soc_feature_mirror_flexible)) {
                    /* Read ingress mtp enable bitmap for source port. */
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_ingress_get(unit, port_ix, &enable));
                    for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT; 
                          mtp_index++) {
                        if (enable & (1 << mtp_index)) {
                            /* Used slot get MTP index*/
#if defined(BCM_TRIDENT_SUPPORT)
                            if (soc_feature(unit,
                                    soc_feature_mirror_control_mem)) {
                                idx = soc_MIRROR_CONTROLm_field32_get(unit,
                                                       &mc_entry,
                                           _mtp_index_field[mtp_index]);
                            } else
#endif /* BCM_TRIDENT_SUPPORT */
                            {
                                idx = soc_reg_field_get(unit,
                                            MIRROR_CONTROLr, reg_val,
                                            _mtp_index_field[mtp_index]);
                            }

                            if (!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                                /* Ingress or egress? */
                                if (mtp_type & (1 << mtp_index)) {
                                    /* Ingress mirroring was enabled, but type is
                                     * egress. */
                                    return BCM_E_INTERNAL;
                                } else if (TRUE ==
                                    MIRROR_CONFIG_SHARED_MTP(unit, idx).egress) {
                                    /* Mismatched ingress/egress settings */
                                    return BCM_E_INTERNAL;
                                } else {
                                    /* Ingress */
                                    MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
                                    MIRROR_DEST_REF_COUNT(unit,
                                       MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx))++;
                                }
                            }
                        }
                    }
                    /* Read ingress mtp enable bitmap for source port. */
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_egress_get(unit, port_ix, &enable));
                    for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT;
                          mtp_index++) {
                        if (enable & (1 << mtp_index)) {
                            /* Used slot get MTP index*/
#if defined(BCM_TRIDENT_SUPPORT)
                            if (soc_feature(unit,
                                    soc_feature_mirror_control_mem)) {
                                idx = soc_MIRROR_CONTROLm_field32_get(unit,
                                                       &mc_entry,
                                           _mtp_index_field[mtp_index]);
                            } else
#endif /* BCM_TRIDENT_SUPPORT */
                            {
                                idx = soc_reg_field_get(unit,
                                            MIRROR_CONTROLr, reg_val,
                                            _mtp_index_field[mtp_index]);
                            }
                            if (!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                                /* Ingress or egress? */
                                if (FALSE ==
                                    MIRROR_CONFIG_SHARED_MTP(unit, idx).egress) {
                                    /* Mismatched ingress/egress settings */
                                    return BCM_E_INTERNAL;
                                } else if (mtp_type & (1 << mtp_index)) {
                                    /* Egress */
                                    MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
                                    MIRROR_DEST_REF_COUNT(unit,
                                        MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx))++;
                                } else {
                                    /* Egress mirroring was enabled, but type is
                                     * ingress. */
                                    return BCM_E_INTERNAL;
                                }
                            }
                        }
                    }
                } else
#endif /* BCM_TRIUMPH2_SUPPORT */
                {
                    BCM_IF_ERROR_RETURN
                        (bcm_esw_mirror_ingress_get(unit, port_ix, &enable));
                    if (enable) {
                        idx = soc_reg_field_get(unit, MIRROR_CONTROLr,
                                                reg_val, IM_MTP_INDEXf);
                        MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)++;
                        MIRROR_DEST_REF_COUNT(unit,
                               MIRROR_CONFIG_ING_MTP_DEST(unit, idx))++;
                    }
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_egress_get(unit, port_ix, &enable));
                    if (enable) {
                        idx = soc_reg_field_get(unit, MIRROR_CONTROLr,
                                                reg_val, EM_MTP_INDEXf);
                        MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)++;
                        MIRROR_DEST_REF_COUNT(unit,
                               MIRROR_CONFIG_EGR_MTP_DEST(unit, idx))++;
                    }
                }
            }

#ifdef BCM_TRIUMPH2_SUPPORT
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                BCM_IF_ERROR_RETURN
                    (_bcm_port_mirror_egress_true_enable_get(unit, port_ix,
                                                           &enable));
                for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT; 
                      mtp_index++) {
                    if (enable & (1 << mtp_index)) {
                        /* Egress true mirroring doesn't need mtp_slot
                         * remapping. */
                        MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit,
                                                             mtp_index)++;
                        MIRROR_DEST_REF_COUNT(unit,
                               MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit,
                                                               mtp_index))++;
                    }
                }
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
        }
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            BCM_IF_ERROR_RETURN(_bcm_esw_directed_flexible_mirror_recover(unit));
        }
        /* recover virtual port mtp usage reference counters */
        BCM_IF_ERROR_RETURN(_bcm_mirror_vp_mtp_ref_count_recover(unit));

        /* recover sflow mtp usage reference counters */
        BCM_IF_ERROR_RETURN(_bcm_mirror_sflow_mtp_ref_count_recover(unit));

#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit) &&
            NULL != mtp_scache_p) {
            if (recovered_ver >= BCM_WB_VERSION_1_4) {
                _bcm_mirror_dest_config_p mdest = NULL;
                /* Recover Shared MTP ref_count */
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                    MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) =
                        shared_mtp_ref_count[idx];
                }

                /* Recover Egress True MTP ref_count */
                if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                    for (idx = 0;
                         idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                        MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx) =
                            egr_true_mtp_ref_count[idx];
                    }
                }

                /* Recover Mirror Destination ref_count */
                for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
                    mdest = MIRROR_CONFIG(unit)->dest_arr + idx;
                    mdest->ref_count = mirror_dest_ref_count[idx];
                }

                /* Recover MIRROR_CONFIG_MODE */
                MIRROR_CONFIG_MODE(unit) = mirror_config_mode;
            }
        }
#endif

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
        /* Recover EGR_MIRROR_ENCAP references from EGR_MTP &
         * EGR_PORT tables. */
        if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
            egr_im_mtp_index_entry_t egr_mtp_entry;
#ifdef BCM_TRIDENT_SUPPORT
            egr_port_entry_t egr_port_entry;
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_GREYHOUND_SUPPORT
            uint64 reg_val64;
#endif /* BCM_GREYHOUND_SUPPORT */
            uint32 profile_index;
            int offset;
            bcm_mirror_destination_t *mir_dest;
            uint8 done = FALSE;

            /*
             * EGR_*_MTP_INDEX tables are arranged as blocks of
             * BCM_SWITCH_TRUNK_MAX_PORTCNT entries, with
             * BCM_MIRROR_MTP_COUNT sets.  We only need to check the
             * first entry of each block to see if the
             * encap enable is set.  We track one reference count
             * for each set, by each destination type (IM, EM, TRUE EM)
             */
            offset = 0;
#ifdef BCM_METROLITE_SUPPORT
            if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
                max_num_trunk_ports = 4;
            } else
#endif
                max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;

#if defined(BCM_METROLITE_SUPPORT)
            /* No of MTP supported for 53460 and 53461 is
             * one for ingress and one for egress and
             * flexible mirroring is not supported  */
            soc_cm_get_id(unit, &dev_id, &rev_id);
            if( (dev_id == BCM53460_DEVICE_ID) ||
                (dev_id == BCM53461_DEVICE_ID) ) {
                mtp_port_count = 1;
            } else
#endif
                mtp_port_count = BCM_MIRROR_MTP_COUNT;

            for (mtp_index = 0; mtp_index < mtp_port_count;
                 mtp_index++, offset += max_num_trunk_ports) {

                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ANY,
                                                 offset, &egr_mtp_entry));
                if (soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                        &egr_mtp_entry,
                                        MIRROR_ENCAP_ENABLEf)) {
                    profile_index =
                        soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                            &egr_mtp_entry,
                                            MIRROR_ENCAP_INDEXf);
                    BCM_IF_ERROR_RETURN
                        (_bcm_egr_mirror_encap_entry_reference(unit,
                                                               profile_index));
                    /* Tunnel type info recovery */
                    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                        mir_dest =
                            MIRROR_DEST(unit,
                                MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index));
                        done = FALSE;
#ifdef BCM_TRIDENT2_SUPPORT
                        if (soc_feature(unit, soc_feature_mirror_flexible) &&
                            mir_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
                            done = TRUE;
                        }
#endif
                        if (!done) {
                            BCM_IF_ERROR_RETURN
                                (_bcm_td_mirror_tunnel_reload(unit,
                                                              mir_dest,
                                                              profile_index));
                        }
                    } else {
                        mir_dest =
                            MIRROR_DEST(unit,
                                MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index));

                        done = FALSE;
#ifdef BCM_TRIDENT2_SUPPORT
                        if (soc_feature(unit, soc_feature_mirror_flexible) &&
                            mir_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
                            done = TRUE;
                        }
#endif
                        if (!done) {
                            BCM_IF_ERROR_RETURN
                                (_bcm_td_mirror_tunnel_reload(unit,
                                                              mir_dest,
                                                              profile_index));
                        }
                    }
                }

                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ANY,
                                                 offset, &egr_mtp_entry));
                if (soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                        &egr_mtp_entry,
                                        MIRROR_ENCAP_ENABLEf)) {
                    profile_index =
                        soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                            &egr_mtp_entry,
                                            MIRROR_ENCAP_INDEXf);
                    BCM_IF_ERROR_RETURN
                        (_bcm_egr_mirror_encap_entry_reference(unit,
                                                               profile_index));
                    /* Tunnel type info recovery */
                    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                        mir_dest =
                            MIRROR_DEST(unit,
                                MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_index));

                        done = FALSE;
#ifdef BCM_TRIDENT2_SUPPORT
                        if (soc_feature(unit, soc_feature_mirror_flexible) &&
                            mir_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
                            done = TRUE;
                        }
#endif
                       if (!done) {
                            BCM_IF_ERROR_RETURN
                                (_bcm_td_mirror_tunnel_reload(unit,
                                                              mir_dest,
                                                              profile_index));
                        }
                    } else {
                        mir_dest =
                            MIRROR_DEST(unit,
                                MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index));

                        done = FALSE;
#ifdef BCM_TRIDENT2_SUPPORT
                        if (soc_feature(unit, soc_feature_mirror_flexible) &&
                            mir_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
                            done = TRUE;
                        }
#endif
                        if (!done) {
                            BCM_IF_ERROR_RETURN
                                (_bcm_td_mirror_tunnel_reload(unit,
                                                              mir_dest,
                                                              profile_index));
                        }
                    }

                }

                if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_read(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                                      MEM_BLOCK_ANY, offset, &egr_mtp_entry));
                    if (soc_mem_field32_get(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                                            &egr_mtp_entry,
                                            MIRROR_ENCAP_ENABLEf)) {
                        profile_index =
                            soc_mem_field32_get(unit,
                                                EGR_EP_REDIRECT_EM_MTP_INDEXm,
                                                &egr_mtp_entry,
                                                MIRROR_ENCAP_INDEXf);
                        BCM_IF_ERROR_RETURN
                            (_bcm_egr_mirror_encap_entry_reference(unit,
                                                             profile_index));
                        mir_dest =
                            MIRROR_DEST(unit,
                                MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, mtp_index));
                        /* Tunnel type info recovery */
                        BCM_IF_ERROR_RETURN
                            (_bcm_td_mirror_tunnel_reload(unit,
                                                          mir_dest,
                                                          profile_index));
                    }
                }
            }

            PBMP_ITER(all_pbmp, port_ix) {
#ifdef BCM_GREYHOUND_SUPPORT
                if (SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) ||
                    SOC_IS_GREYHOUND2(unit)) {
                    BCM_IF_ERROR_RETURN(READ_EGR_PORT_64r(unit, port_ix, &reg_val64));
                
                    if (0 != soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val64, 
                                                   MIRROR_ENCAP_ENABLEf)) {
                        profile_index =
                            soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val64, 
                                                  MIRROR_ENCAP_INDEXf);
                        BCM_IF_ERROR_RETURN
                            (_bcm_egr_mirror_encap_entry_reference(unit,
                                                         profile_index));
                    }
                } else
#endif /* BCM_GREYHOUND_SUPPORT */
                {
#ifdef BCM_TRIDENT_SUPPORT
                    BCM_IF_ERROR_RETURN
                        (READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port_ix,
                                        &egr_port_entry));
    
                    if (0 != soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                                       MIRROR_ENCAP_ENABLEf)) {
                        profile_index =
                            soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                                      MIRROR_ENCAP_INDEXf);
                        BCM_IF_ERROR_RETURN
                            (_bcm_egr_mirror_encap_entry_reference(unit,
                                                         profile_index));
                    }
#endif /* BCM_TRIDENT_SUPPORT */
                }
            }
        }
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

#ifdef BCM_FIELD_SUPPORT 
        if (soc_feature(unit, soc_feature_field)) {
            rv = bcm_esw_field_group_traverse(unit,
                                          _bcm_esw_mirror_field_group_reload,
                                          NULL);
            if (BCM_FAILURE(rv) && (BCM_E_INIT != rv)) {
                return rv;
            }
        }
#endif /* BCM_FIELD_SUPPORT */

        if (NULL == mtp_scache) {
            /* Cleanup unused null destination in Warm Boot Level 1 */
            flags = 0;
            BCM_IF_ERROR_RETURN
                (_bcm_mirror_gport_construct(unit, 0, 0, 0, &gport));
            rv = _bcm_esw_mirror_destination_find(unit, gport, 0,
                                                  flags, &mirror_dest);
            if (BCM_E_NOT_FOUND == rv) {
                /* Nothing to do */
            } else if (BCM_FAILURE(rv)) {
                return rv;
            } else if (MIRROR_DEST_REF_COUNT(unit,
                                             mirror_dest.mirror_dest_id) == 1) {
                BCM_IF_ERROR_RETURN 
                    (bcm_esw_mirror_destination_destroy(unit,
                                                mirror_dest.mirror_dest_id));
            }
        }

        if (extra_scache_size > 0) {
            BCM_IF_ERROR_RETURN
                (soc_scache_realloc(unit, scache_handle, extra_scache_size));
        }
    }

#if defined (BCM_TOMAHAWK_SUPPORT)
    /* Field callback functions for recovering Mirror Destination during warmboot */
    BCM_IF_ERROR_RETURN
        (_field_mirror_actions_recover_callback(unit, _bcm_mirror_config[unit]));
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *  	_bcm_esw_mirror_sync 
 * Purpose:
 *  	Stores mirroring destination for warm boot recovery
 * Parameters:
 *	    unit        - (IN)BCM device number.
 * Returns:
 *  	BCM_E_XXX
 */

int
_bcm_esw_mirror_sync(int unit)
{
    soc_scache_handle_t scache_handle;
    uint8               *mtp_scache;
    uint16              dest_field_bmp = 0;
    int                 idx;
    int                 rv;

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_MIRROR, 0);
    BCM_IF_ERROR_RETURN
        (_bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &mtp_scache, BCM_WB_DEFAULT_VERSION, NULL));

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
            sal_memcpy(mtp_scache,
                       &(MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx)),
                         sizeof(bcm_gport_t));
            mtp_scache += sizeof(bcm_gport_t);
        }
    } else {
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
            sal_memcpy(mtp_scache,
                       &(MIRROR_CONFIG_ING_MTP_DEST(unit, idx)),
                         sizeof(bcm_gport_t));
            mtp_scache += sizeof(bcm_gport_t);
        }

        for (idx = 0;
             idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
            sal_memcpy(mtp_scache,
                       &(MIRROR_CONFIG_EGR_MTP_DEST(unit, idx)),
                         sizeof(bcm_gport_t));
            mtp_scache += sizeof(bcm_gport_t);
        }
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        for (idx = 0;
             idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
            sal_memcpy(mtp_scache,
                       &(MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx)),
                       sizeof(bcm_gport_t));
            mtp_scache += sizeof(bcm_gport_t);
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
    /* Version_1_0 -> Version_1_1:
     * In directed flexible mirror mode, for each possible mirror entity,
     * it has respective mirror slot enable control and slot-MTP mapping
     * control. For port mirror, MIRROR field in PORT_TAB memory
     * controls if ingress mirror is enabled for a slot for a given port.
     * For sflow mirror, MIRROR_ENABLE field in SFLOW_ING_MIRROR_CONFIGr
     * control if mirror is enabled on a sflow for a given slot. For FP
     * mirror, the MIRROR field in FP_POLICY table play the same role.
     * In current implementation, there is no easy way to get the slot
     * enable information and slot-MTP mapping information,
     * so this information is stored in L2 warmboot.
     */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        _bcm_mirror_dest_config_p  mdest = NULL;
        int slot_type;
        bcm_gport_t dest_id = 0;
        uint32 flags = 0;
        /* Slot-MTP mapping information for respective slot types */
        for (slot_type = BCM_MTP_SLOT_TYPE_PORT; slot_type < BCM_MTP_SLOT_TYPES; slot_type++) {
            for (idx = 0; idx < MIRROR_CONFIG(unit)->mtp_slot_count[slot_type]; idx++) {
                sal_memcpy(mtp_scache,
                           &MIRROR_CONFIG_TYPE_MTP_SLOT(unit, idx, slot_type),
                           sizeof(bcm_gport_t));
                mtp_scache += sizeof(bcm_gport_t);
            }
        }
        /* Reference counter for respective slot types . */
        for (slot_type = BCM_MTP_SLOT_TYPE_PORT; slot_type < BCM_MTP_SLOT_TYPES; slot_type++) {
            for (idx = 0; idx < MIRROR_CONFIG(unit)->mtp_slot_count[slot_type]; idx++) {
                sal_memcpy(mtp_scache,
                           &MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, idx, slot_type),
                           sizeof(int));
                mtp_scache += sizeof(int);
            }
        }
        /* It is possible that FP module can create mirror destination implicitly
         * when _bcm_esw_mirror_fp_dest_add is called thru mod + port mirror dest.
         * In this case, A new destination will be created if there is no
         * destination match mod + port mirror dest. And the new destination will
         * be set with the flag BCM_MIRROR_DEST_FIELD.
         * During warmboot, the flag BCM_MIRROR_DEST_FIELD can not be recovered
         * from H/W.
         */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
            mdest = MIRROR_CONFIG(unit)->dest_arr + idx;;
            if (mdest->mirror_dest.flags & BCM_MIRROR_DEST_FIELD) {
                dest_field_bmp |= (1 << idx);
            }
        }
        sal_memcpy(mtp_scache, &dest_field_bmp, sizeof(uint16));
        mtp_scache += sizeof(uint16);
        /* Reference counter for MTP slot */
        for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
            sal_memcpy(mtp_scache,
                       &MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, idx),
                       sizeof(int));
            mtp_scache += sizeof(int);
        }
        /* Reference counter for ING MTP */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
            sal_memcpy(mtp_scache,
                       &MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx),
                       sizeof(int));
            mtp_scache += sizeof(int);
        }
        /* Reference counter for EGR MTP */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
            sal_memcpy(mtp_scache,
                       &MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx),
                       sizeof(int));
            mtp_scache += sizeof(int);
        }
        /* Reference counter for destination */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
            mdest = MIRROR_CONFIG(unit)->dest_arr + idx;
            sal_memcpy(mtp_scache, &(mdest->ref_count), sizeof(int));
            mtp_scache += sizeof(int);
        }
        /* Port bitmap for MTP slot used */
        for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
            sal_memcpy(mtp_scache,
                       &MIRROR_CONFIG_PBMP_MTP_SLOT_USED(unit, idx),
                       sizeof(bcm_pbmp_t));
            mtp_scache += sizeof(bcm_pbmp_t);
        }

        /* Version_1_1 -> Version_1_2:
        * The destination flags couldn't be recovered from hardware, so save it
        */
        /* Save flags of ING MTP destination */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
            dest_id = MIRROR_CONFIG_ING_MTP_DEST(unit, idx);
            flags = BCM_GPORT_IS_MIRROR(dest_id) ?
                MIRROR_DEST(unit, dest_id)->flags : 0;
            sal_memcpy(mtp_scache,
                       &flags,
                       sizeof(uint32));
            mtp_scache += sizeof(uint32);
        }
        /* Save flags for EGR MTP destination */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
            dest_id = MIRROR_CONFIG_EGR_MTP_DEST(unit, idx);
            flags = BCM_GPORT_IS_MIRROR(dest_id) ?
                MIRROR_DEST(unit, dest_id)->flags : 0;
            sal_memcpy(mtp_scache,
                       &flags,
                       sizeof(uint32));
            mtp_scache += sizeof(uint32);
        }

#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            for (idx = 0;
                 idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                dest_id = MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx);
                flags = BCM_GPORT_IS_MIRROR(dest_id) ?
                    MIRROR_DEST(unit, dest_id)->flags : 0;
                sal_memcpy(mtp_scache,
                           &flags,
                           sizeof(uint32));
                mtp_scache += sizeof(uint32);
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    /* Version_1_3: Save destination flags for non-flexible mtp mirror case */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        bcm_gport_t dest_id = 0;
        uint32 flags = 0;
        int ref_count = 0;
        _bcm_mirror_dest_config_p mdest = NULL;

        /* Save flags for Shared MTP destination */
        for (idx = 0;
             idx < BCM_MIRROR_MTP_COUNT; idx++) {
            dest_id = MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx);
            flags = BCM_GPORT_IS_MIRROR(dest_id) ?
                MIRROR_DEST(unit, dest_id)->flags : 0;
            sal_memcpy(mtp_scache,
                       &flags,
                       sizeof(uint32));
            mtp_scache += sizeof(uint32);
        }

        /* Save flags for egress true MTP destination */
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            for (idx = 0;
                 idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                dest_id = MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx);
                flags = BCM_GPORT_IS_MIRROR(dest_id) ?
                    MIRROR_DEST(unit, dest_id)->flags : 0;
                sal_memcpy(mtp_scache,
                           &flags,
                           sizeof(uint32));
                mtp_scache += sizeof(uint32);
            }
        }

        /* Version_1_4:
        * Save Shared MTP ref_count, Egress True MTP ref_count,
        * Mirror Destination ref_count,  MIRROR_CONFIG_MODE
        */
        /* Save Shared MTP ref_count */
        for (idx = 0;
             idx < BCM_MIRROR_MTP_COUNT; idx++) {
            ref_count = MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx);
            sal_memcpy(mtp_scache,
                       &ref_count,
                       sizeof(int));
            mtp_scache += sizeof(int);
        }

        /* Save Egress True MTP ref_count */
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            for (idx = 0;
                 idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
                ref_count = MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx);
                sal_memcpy(mtp_scache,
                           &ref_count,
                           sizeof(int));
                mtp_scache += sizeof(int);
            }
        }

        /* Save Mirror Destination ref_count */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
            mdest = MIRROR_CONFIG(unit)->dest_arr + idx;
            sal_memcpy(mtp_scache, &(mdest->ref_count), sizeof(int));
            mtp_scache += sizeof(int);
        }

        /* Save MIRROR_CONFIG_MODE */
        sal_memcpy(mtp_scache, &MIRROR_CONFIG_MODE(unit), sizeof(int));
        mtp_scache += sizeof(int);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIDENT2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        /* Version_1_5: Save MTP destination gport. */
        bcm_gport_t dest_id = 0;
        if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            /* Save Ingress MTP destination gport */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
                dest_id = MIRROR_CONFIG_ING_MTP_DEST(unit, idx);
                if (BCM_GPORT_IS_MIRROR(dest_id)) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_mirror_dest_gport_sync(unit,
                                                    dest_id,
                                                    mtp_scache));
                }
                mtp_scache += sizeof(bcm_gport_t) *
                              BCM_SWITCH_TRUNK_MAX_PORTCNT;
            }

            /* Save Egress MTP destination gport */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                dest_id = MIRROR_CONFIG_EGR_MTP_DEST(unit, idx);
                if (BCM_GPORT_IS_MIRROR(dest_id)) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_mirror_dest_gport_sync(unit,
                                                    dest_id,
                                                    mtp_scache));
                }
                mtp_scache += sizeof(bcm_gport_t) *
                              BCM_SWITCH_TRUNK_MAX_PORTCNT;
            }

            /* Save Ingress MTP destination flags */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
                dest_id = MIRROR_CONFIG_ING_MTP_DEST(unit, idx);
                if (BCM_GPORT_IS_MIRROR(dest_id)) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_mirror_dest_flags_sync(unit,
                                                    dest_id,
                                                    mtp_scache));
                }
                mtp_scache += sizeof(uint32) *
                              BCM_SWITCH_TRUNK_MAX_PORTCNT;
            }

            /* Save Egress MTP destination flags */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                dest_id = MIRROR_CONFIG_EGR_MTP_DEST(unit, idx);
                if (BCM_GPORT_IS_MIRROR(dest_id)) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_mirror_dest_flags_sync(unit,
                                                    dest_id,
                                                    mtp_scache));
                }
                mtp_scache += sizeof(uint32) *
                              BCM_SWITCH_TRUNK_MAX_PORTCNT;
            }
        } else {
            /* Save Shared MTP destination gport */
            for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                dest_id = MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx);
                if (BCM_GPORT_IS_MIRROR(dest_id)) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_mirror_dest_gport_sync(unit,
                                                    dest_id,
                                                    mtp_scache));
                }
                mtp_scache += sizeof(bcm_gport_t) *
                              BCM_SWITCH_TRUNK_MAX_PORTCNT;
            }

            /* Save Shared MTP destination flags */
            for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
                dest_id = MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx);
                if (BCM_GPORT_IS_MIRROR(dest_id)) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_mirror_dest_flags_sync(unit,
                                                    dest_id,
                                                    mtp_scache));
                }
                mtp_scache += sizeof(uint32) *
                              BCM_SWITCH_TRUNK_MAX_PORTCNT;
            }
        }
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_MIRROR, 1);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &mtp_scache, BCM_WB_DEFAULT_VERSION, NULL);
    if(rv == BCM_E_NOT_FOUND) {
        /* Scache not created as init was done in warmboot mode */
        BCM_IF_ERROR_RETURN(_bcm_esw_scache_ptr_get(unit, scache_handle, TRUE,
                                     sizeof(_bcm_mirror_mtp_method_init[unit]),
                                     &mtp_scache,
                                     BCM_WB_DEFAULT_VERSION, NULL));
    }

    sal_memcpy(mtp_scache, &(_bcm_mirror_mtp_method_init[unit]),
               sizeof(_bcm_mirror_mtp_method_init[unit]));
    mtp_scache += sizeof(_bcm_mirror_mtp_method_init[unit]);
    return BCM_E_NONE;
}


#else
#define _bcm_esw_mirror_reload(unit, directed)    (BCM_E_NONE)
#endif /* BCM_WARM_BOOT_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
/*
 * Function:
 *  	_bcm_trident_mirror_egr_dest_get
 * Purpose:
 *  	Get destination port bitmap for egress mirroring.
 * Parameters:
 *      unit        - (IN) BCM device number.
 *	    port        - (IN) port number.
 *      mtp_index   - (IN) mtp index 
 *      dest_bitmap - (OUT) destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_egr_dest_get(int unit, bcm_port_t port, int mtp_index,
                                 bcm_pbmp_t *dest_bitmap)
{
    emirror_control_entry_t entry;
    static const soc_mem_t mem[] = {
        EMIRROR_CONTROLm, EMIRROR_CONTROL1m,
        EMIRROR_CONTROL2m, EMIRROR_CONTROL3m
    };

    if (dest_bitmap == NULL) {
        return BCM_E_PARAM;
    }

    if ((mtp_index < 0) || (mtp_index >= BCM_MIRROR_MTP_COUNT)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, mem[mtp_index], MEM_BLOCK_ANY, port, &entry));
    soc_mem_pbmp_field_get(unit, mem[mtp_index], &entry, BITMAPf, dest_bitmap);

    return BCM_E_NONE;
}

/*
 * Function:
 *  	_bcm_trident_mirror_egr_dest_set
 * Purpose:
 *  	Set destination port bitmap for egress mirroring.
 * Parameters:
 *      unit        - (IN) BCM device number.
 *	    port        - (IN) port number.
 *      mtp_index   - (IN) mtp index
 *      dest_bitmap - (IN) destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_egr_dest_set(int unit, bcm_port_t port, int mtp_index,
                                 bcm_pbmp_t *dest_bitmap)
{
    emirror_control_entry_t entry;
    int cpu_hg_index = 0;

    static const soc_mem_t mem[] = {
        EMIRROR_CONTROLm, EMIRROR_CONTROL1m,
        EMIRROR_CONTROL2m, EMIRROR_CONTROL3m
    };

    if (dest_bitmap == NULL) {
        return BCM_E_PARAM;
    }
    if ((mtp_index < 0) || (mtp_index >= BCM_MIRROR_MTP_COUNT)) {
        return BCM_E_PARAM;
    }

    /* mtp_index is validated as an egress type previously */

    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, mem[mtp_index], MEM_BLOCK_ANY, port, &entry));
    soc_mem_pbmp_field_set(unit, mem[mtp_index], &entry,
                           BITMAPf, dest_bitmap);
    BCM_IF_ERROR_RETURN
        (soc_mem_write(unit, mem[mtp_index], MEM_BLOCK_ANY, port, &entry));

    /* Configure mirroring of CPU Higig packets as well */
    cpu_hg_index = SOC_IS_KATANA2(unit) ? SOC_INFO(unit).cpu_hg_pp_port_index :
                                          SOC_INFO(unit).cpu_hg_index;
    if (IS_CPU_PORT(unit, port) && cpu_hg_index != -1) {
        BCM_IF_ERROR_RETURN(soc_mem_write(unit, mem[mtp_index], MEM_BLOCK_ANY,
                                          cpu_hg_index, &entry));
    }

    return BCM_E_NONE;
}
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT)
/*
 * Function:
 *  	_bcm_triumph_mirror_egr_dest_get 
 * Purpose:
 *  	Get destination port bitmap for egress mirroring.
 * Parameters:
 *      unit        - (IN) BCM device number.
 *      port        - (IN) port number.
 *      mtp_index   - (IN) mtp index (mtp_slot for flex mirroring)
 *      dest_bitmap - (OUT) destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_triumph_mirror_egr_dest_get(int unit, bcm_port_t port, int mtp_index,
                                 bcm_pbmp_t *dest_bitmap)
{
    uint32 fval;
    uint64 mirror;               /* Egress mirror control reg value. */
    static const soc_reg_t reg[] = {
        EMIRROR_CONTROL_64r, EMIRROR_CONTROL1_64r,
        EMIRROR_CONTROL2_64r, EMIRROR_CONTROL3_64r
    };
#ifdef BCM_GREYHOUND2_SUPPORT
    uint64 fval64;
    static const soc_reg_t reg_lo[] = {
        EMIRROR_CONTROL_LO_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t reg_hi[] = {
        EMIRROR_CONTROL_HI_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
#endif
    /* Input parameters check. */
    if (NULL == dest_bitmap) {
        return BCM_E_PARAM;
    }
    if ((mtp_index < 0) || (mtp_index >= BCM_MIRROR_MTP_COUNT)) {
        return BCM_E_PARAM;
    }

#ifdef BCM_GREYHOUND2_SUPPORT
    if(soc_feature(unit, soc_feature_high_portcount_register)){
        BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg_lo[mtp_index], port, 0, &mirror));

        SOC_PBMP_CLEAR(*dest_bitmap);
        fval64 = soc_reg64_field_get(unit, reg_lo[mtp_index], mirror, BITMAP_LOf);
        SOC_PBMP_WORD_SET(*dest_bitmap, 0, COMPILER_64_LO(fval64));
        SOC_PBMP_WORD_SET(*dest_bitmap, 1, COMPILER_64_HI(fval64));
        BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg_hi[mtp_index], port, 0, &mirror));

        fval64 = soc_reg64_field_get(unit, reg_hi[mtp_index], mirror, BITMAP_LOf);
        SOC_PBMP_WORD_SET(*dest_bitmap, 2, COMPILER_64_LO(fval64));
        SOC_PBMP_WORD_SET(*dest_bitmap, 3, COMPILER_64_HI(fval64));
       
    }else
#endif /*BCM_GREYHOUND2_SUPPORT*/
    {

    /* mtp_index is validated as an egress type previously */
        BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg[mtp_index], port, 0, &mirror));

        SOC_PBMP_CLEAR(*dest_bitmap);
        fval = soc_reg64_field32_get(unit, reg[mtp_index], mirror, BITMAP_LOf);
        SOC_PBMP_WORD_SET(*dest_bitmap, 0, fval);

        if (soc_reg_field_valid(unit, reg[mtp_index], BITMAP_HIf)) {
            fval = soc_reg64_field32_get(unit, reg[mtp_index], mirror, BITMAP_HIf);
            SOC_PBMP_WORD_SET(*dest_bitmap, 1, fval);
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *  	_bcm_triumph_mirror_egr_dest_set 
 * Purpose:
 *  	Set destination port bitmap for egress mirroring.
 * Parameters:
 *      unit        - (IN) BCM device number.
 *      port        - (IN) Port number.
 *      mtp_index   - (IN) mtp slot number.
 *      dest_bitmap - (IN) Destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_triumph_mirror_egr_dest_set(int unit, bcm_port_t port, int mtp_index,
                                 bcm_pbmp_t *dest_bitmap)
{
    static const soc_reg_t reg[] = {
        EMIRROR_CONTROL_64r, EMIRROR_CONTROL1_64r,
        EMIRROR_CONTROL2_64r, EMIRROR_CONTROL3_64r
    };
    static const soc_reg_t hg_reg[] = {
        IEMIRROR_CONTROL_64r, IEMIRROR_CONTROL1_64r,
        IEMIRROR_CONTROL2_64r, IEMIRROR_CONTROL3_64r
    };
    uint32 values[2];
    soc_field_t fields[] = {BITMAP_LOf, BITMAP_HIf};
    int count;
#ifdef BCM_GREYHOUND2_SUPPORT    
    uint64 mirror, val64;
    static const soc_reg_t reg_lo[] = {
        EMIRROR_CONTROL_LO_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t reg_hi[] = {
        EMIRROR_CONTROL_HI_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t hg_reg_lo[] = {
        IEMIRROR_CONTROL_LO_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t hg_reg_hi[] = {
        IEMIRROR_CONTROL_HI_64r, INVALIDr,
        INVALIDr, INVALIDr
    };

#endif /*BCM_GREYHOUND2_SUPPORT*/
    /* Input parameters check. */
    if (NULL == dest_bitmap) {
        return BCM_E_PARAM;
    }
    if ((mtp_index < 0) || (mtp_index >= BCM_MIRROR_MTP_COUNT)) {
        return BCM_E_PARAM;
    }
    
    /* mtp_index is validated as an egress type previously */
    if ((mtp_index >= MIRROR_CONFIG(unit)->port_em_mtp_count) && 
        !(soc_feature(unit, soc_feature_mirror_flexible))) {
        /* Out of range */
        return BCM_E_PARAM;
    }
#ifdef BCM_GREYHOUND2_SUPPORT
    if(soc_feature(unit, soc_feature_high_portcount_register)){ 
        /*For Register LO*/
        BCM_IF_ERROR_RETURN(READ_EMIRROR_CONTROL_LO_64r(unit, port, &mirror));
        COMPILER_64_SET(val64, SOC_PBMP_WORD_GET(*dest_bitmap, 1),
                        SOC_PBMP_WORD_GET(*dest_bitmap, 0));
        soc_reg64_field_set(unit, reg_lo[mtp_index], &mirror,
                            BITMAP_LOf, val64);
        BCM_IF_ERROR_RETURN(WRITE_EMIRROR_CONTROL_LO_64r(unit, port, mirror));        
        /* Enable mirroring of CPU Higig packets as well */
        if (IS_CPU_PORT(unit, port)) {
            COMPILER_64_ZERO(mirror);
            BCM_IF_ERROR_RETURN(READ_IEMIRROR_CONTROL_LO_64r(unit, port, &mirror));
            soc_reg64_field_set(unit, hg_reg_lo[mtp_index], &mirror,
                                BITMAP_LOf, val64);
            BCM_IF_ERROR_RETURN(WRITE_IEMIRROR_CONTROL_LO_64r(unit, port, mirror));
        }
        /*For Register HI*/
        COMPILER_64_ZERO(mirror);
        COMPILER_64_ZERO(val64);
        BCM_IF_ERROR_RETURN(READ_EMIRROR_CONTROL_HI_64r(unit, port, &mirror));
        COMPILER_64_SET(val64, SOC_PBMP_WORD_GET(*dest_bitmap, 3),
                        SOC_PBMP_WORD_GET(*dest_bitmap, 2));
        soc_reg64_field_set(unit, reg_hi[mtp_index], &mirror,
                            BITMAP_LOf, val64);
        BCM_IF_ERROR_RETURN(WRITE_EMIRROR_CONTROL_HI_64r(unit, port, mirror));
        /* Enable mirroring of CPU Higig packets as well */
        if (IS_CPU_PORT(unit, port)) {
            COMPILER_64_ZERO(mirror);
            BCM_IF_ERROR_RETURN(READ_IEMIRROR_CONTROL_HI_64r(unit, port, &mirror));
            soc_reg64_field_set(unit, hg_reg_hi[mtp_index], &mirror,
                                BITMAP_LOf, val64);
            BCM_IF_ERROR_RETURN(WRITE_IEMIRROR_CONTROL_HI_64r(unit, port, mirror));
        }
    }else
#endif /*BCM_GREYHOUND2_SUPPORT*/
    {
        values[0] = SOC_PBMP_WORD_GET(*dest_bitmap, 0);
        count = 1;
        if (soc_reg_field_valid(unit, reg[mtp_index], BITMAP_HIf)) {
            values[1] = SOC_PBMP_WORD_GET(*dest_bitmap, 1);
            count++;
        }

        BCM_IF_ERROR_RETURN 
            (soc_reg_fields32_modify(unit, reg[mtp_index], port, count,
                                     fields, values));

        /* Enable mirroring of CPU Higig packets as well */
        if (IS_CPU_PORT(unit, port)) {
            BCM_IF_ERROR_RETURN 
                (soc_reg_fields32_modify(unit, hg_reg[mtp_index], port, count,
                                         fields, values));
        }
    }
    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH_SUPPORT */

#if defined(BCM_RAPTOR1_SUPPORT)
/*
 * Function:
 *  	_bcm_raptor_mirror_egr_dest_get 
 * Purpose:
 *  	Get destination port bitmap for egress mirroring.
 * Parameters:
 *	    unit        - (IN)BCM device number.
 *	    port        - (IN)port number.
 *      dest_bitmap - (OUT) destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_raptor_mirror_egr_dest_get(int unit, bcm_port_t port, 
                                bcm_pbmp_t *dest_bitmap)
{
    uint32 mirror;

    /* Input parameters check. */
    if (NULL == dest_bitmap) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(READ_EMIRROR_CONTROLr(unit, port, &mirror));
    SOC_PBMP_WORD_SET(*dest_bitmap, 0, 
        soc_reg_field_get(unit, EMIRROR_CONTROLr, mirror, BITMAPf));
    BCM_IF_ERROR_RETURN(READ_EMIRROR_CONTROL_HIr(unit, port, &mirror));
    SOC_PBMP_WORD_SET(*dest_bitmap, 1, 
        soc_reg_field_get(unit, EMIRROR_CONTROL_HIr, mirror, BITMAPf));

    return (BCM_E_NONE);
}


/*
 * Function:
 *  	_bcm_raptor_mirror_egr_dest_set 
 * Purpose:
 *  	Set destination port bitmap for egress mirroring.
 * Parameters:
 *	    unit        - (IN)BCM device number.
 *	    port        - (IN)Port number.
 *      dest_bitmap - (IN)Destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_raptor_mirror_egr_dest_set(int unit, bcm_port_t port, 
                                bcm_pbmp_t *dest_bitmap)
{
    uint32 value;
    soc_field_t field = BITMAPf;

    /* Input parameters check. */
    if (NULL == dest_bitmap) {
        return (BCM_E_PARAM);
    }
    value = SOC_PBMP_WORD_GET(*dest_bitmap, 0);

    BCM_IF_ERROR_RETURN
        (soc_reg_fields32_modify(unit, EMIRROR_CONTROLr, port, 
                                 1, &field, &value));

    /* Enable mirroring of CPU Higig packets as well */
    if (IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN
            (soc_reg_fields32_modify(unit, IEMIRROR_CONTROLr, port, 
                                     1, &field, &value));

    }

    value = SOC_PBMP_WORD_GET(*dest_bitmap, 1);

    BCM_IF_ERROR_RETURN
        (soc_reg_fields32_modify(unit, EMIRROR_CONTROL_HIr, port, 
                                 1, &field, &value));

    /* Enable mirroring of CPU Higig packets as well */
    if (IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN
            (soc_reg_fields32_modify(unit, IEMIRROR_CONTROL_HIr, port, 
                                     1, &field, &value));

    }
    return (BCM_E_NONE);
}

#endif /* BCM_RAPTOR_SUPPORT */

#if defined(BCM_XGS12_FABRIC_SUPPORT)
/*
 * Function:
 *	   _bcm_xgs_fabric_mirror_enable_set 
 * Purpose:
 *  	Enable/disable mirroring on a port & set mirror-to port.
 * Parameters:
 *	    unit - BCM device number
 *  	port - port number
 *   	enable - enable mirroring if non-zero
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_xgs_fabric_mirror_enable_set(int unit, int port, int enable)
{
    pbmp_t ppbm;
    int mport;

    if (!IS_HG_PORT(unit, port)) {
        return (BCM_E_UNAVAIL);
    }

    /* Clear port when disabling */
    if (enable && MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)) {
       BCM_IF_ERROR_RETURN(_bcm_mirror_destination_gport_parse(unit,
                                           MIRROR_CONFIG_ING_MTP_DEST(unit, 0),
                                           NULL , &mport, NULL));
    } else {
        mport = 0;
    }

    SOC_PBMP_CLEAR(ppbm);
    if (enable) {
        SOC_PBMP_PORT_ADD(ppbm, mport);
    }

#ifdef	BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        int    m5670;

        m5670 = soc_property_get(unit, spn_MIRROR_5670_MODE, 0);
        BCM_IF_ERROR_RETURN 
            (soc_reg_field32_modify(unit, ING_CTRLr, port,
                                    DISABLE_MIRROR_CHANGEf,
                                    (m5670 | !enable) ? 1 : 0));
    }
#endif	/* BCM_HERCULES15_SUPPORT */
    BCM_IF_ERROR_RETURN
        (WRITE_ING_MIRTOBMAPr(unit, port,
                              SOC_PBMP_WORD_GET(ppbm, 0)));
    return (BCM_E_NONE);
}
#endif /* BCM_XGS12_FABRIC_SUPPORT */

/*
 * Function:
 *     _bcm_mirror_dest_get_all
 * Purpose:
 *     Get all mirroring destinations.   
 * Parameters:
 *     unit             - (IN) BCM device number. 
 *     flags            - (IN) BCM_MIRROR_PORT_XXX flags.
 *     mirror_dest_size - (IN) Preallocated mirror_dest array size.
 *     mirror_dest      - (OUT)Filled array of port mirroring destinations
 *     mirror_dest_count - (OUT)Actual number of mirroring destinations filled.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_mirror_dest_get_all(int unit, uint32 flags, int mirror_dest_size,
                         bcm_gport_t *mirror_dest, int *mirror_dest_count)
{
    int idx = 0;
    int index = 0;

    /* Input parameters check. */
    if ((NULL == mirror_dest) || (NULL == mirror_dest_count)) {
        return (BCM_E_PARAM);
    }
#ifdef BCM_TRIUMPH2_SUPPORT 
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        /* Copy all used shared mirror destinations. */
        for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
            if ((index < mirror_dest_size) && 
                (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx))) {
                if ((!(MIRROR_CONFIG_SHARED_MTP(unit, idx).egress) &&
                    (flags & BCM_MIRROR_PORT_INGRESS)) ||
                    (MIRROR_CONFIG_SHARED_MTP(unit, idx).egress &&
                     (flags & BCM_MIRROR_PORT_EGRESS))) {
                    mirror_dest[index] =
                        MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx);
                    index++;
                }
            }
        } 
    } else {
#endif /* BCM_TRIUMPH2_SUPPORT */
        /* Copy all used ingress mirror destinations. */
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
                if ((index < mirror_dest_size) && 
                    (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx))) {
                    mirror_dest[index] =
                        MIRROR_CONFIG_ING_MTP_DEST(unit, idx);
                    index++;
                }
            }
        } 

        /* Copy all used egress mirror destinations. */
        if (flags & BCM_MIRROR_PORT_EGRESS) {
            for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                if ((index < mirror_dest_size) && 
                    (MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx))) {
                    mirror_dest[index] =
                        MIRROR_CONFIG_EGR_MTP_DEST(unit, idx);
                    index++;
                }
            }
        }

#ifdef BCM_TRIUMPH2_SUPPORT
    }

    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        /* Copy all used egress mirror destinations. */
        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            for (idx = 0;
                 idx < MIRROR_CONFIG(unit)->egr_true_mtp_count;
                 idx++) {
                if ((index < mirror_dest_size) && 
                    (MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx))) {
                    mirror_dest[index] =
                        MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx);
                    index++;
                }
            }
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    *mirror_dest_count = index;
    return (BCM_E_NONE);
}

#if defined(BCM_XGS3_SWITCH_SUPPORT)

#if defined(BCM_TRX_SUPPORT)

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)

/*
 * Function:
 *	    _bcm_trident_mirror_l2_tunnel_set
 * Purpose:
 *	   Prepare & write L2 mirror tunnel encapsulation on Trident.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     mirror_dest      - (IN) Mirror destination descriptor.
 *     flags      - (IN) Mirror direction flags.
 *     entries    - (IN) Pointer to EGR_MIRROR_ENCAP_* entries array
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_l2_tunnel_set(int unit,
                                  bcm_mirror_destination_t *mirror_dest,
                                  int flags, void **entries)
{
    uint32              hw_buffer;        /* HW buffer.                   */
    egr_mirror_encap_control_entry_t *control_entry_p;
    egr_mirror_encap_data_1_entry_t *data_1_entry_p;

    /* These entries were initialized by the calling function */
    control_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL];
    data_1_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1];

    /* Outer vlan tag. */
    hw_buffer = (((uint32)mirror_dest->tpid << 16) | 
                  (uint32)mirror_dest->vlan_id);

    /* Setup Mirror Control Memory */
    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         ENTRY_TYPEf, BCM_TD_MIRROR_ENCAP_TYPE_RSPAN);

    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         RSPAN__ADD_OPTIONAL_HEADERf,
                         BCM_TD_MIRROR_HEADER_ONLY);

    if (soc_feature(unit, soc_feature_trill)) {
        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                                      RSPAN__ADD_TRILL_OUTER_VLANf, 0);
    }

    /* Setup Mirror Data 1 Memory */
    soc_EGR_MIRROR_ENCAP_DATA_1m_field32_set(unit, data_1_entry_p,
                         RSPAN__RSPAN_VLAN_TAGf, hw_buffer);

    /* Profile entries will be committed to HW by the calling function. */

    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_trident_mirror_ipv4_gre_tunnel_set
 * Purpose:
 *	   Prepare IPv4 mirror tunnel encapsulation for Trident.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     mirror_dest      - (IN) Mirror destination descriptor.
 *     flags      - (IN) Mirror direction flags. 
 *     entries    - (IN) Pointer to EGR_MIRROR_ENCAP_* entries array
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_ipv4_gre_tunnel_set(int unit,
                                        bcm_mirror_destination_t *mirror_dest,
                                        int flags,
                                        void **entries)
{
    /*SW tunnel encap buffers.*/
    uint32 ip_buffer[_BCM_TD_MIRROR_V4_GRE_BUFFER_SZ];
    egr_mirror_encap_control_entry_t *control_entry_p;
    egr_mirror_encap_data_1_entry_t *data_1_entry_p;
    uint32 fldval;
    int                 idx;           /* Headers offset iterator.        */

    if (mirror_dest->df > 1) {
        return (BCM_E_PARAM);
    }

    /* These entries were initialized by the calling function */
    control_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL];
    data_1_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1];

    sal_memset(ip_buffer, 0,
               _BCM_TD_MIRROR_V4_GRE_BUFFER_SZ * sizeof(uint32));

    /* Setup Mirror Control Memory */
    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         ENTRY_TYPEf, BCM_TD_MIRROR_ENCAP_TYPE_ERSPAN);

    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         ERSPAN__ADD_OPTIONAL_HEADERf,
                         BCM_TD_MIRROR_HEADER_ONLY);

    /* coverity[result_independent_of_operands] */
    if (BCM_VLAN_VALID(BCM_VLAN_CTRL_ID(mirror_dest->vlan_id))) {
        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                             ERSPAN__ADD_ERSPAN_OUTER_VLANf, 1);
    }

    if (mirror_dest->flags & BCM_MIRROR_DEST_PAYLOAD_UNTAGGED) {
        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                                                  ERSPAN__UNTAG_PAYLOADf, 1);
    }

    /*
     *   Set mirror tunnel DA SA. 
     */

    soc_mem_mac_addr_set(unit, EGR_MIRROR_ENCAP_DATA_1m,
                         data_1_entry_p, ERSPAN__ERSPAN_HEADER_DAf,
                         mirror_dest->dst_mac);
    soc_mem_mac_addr_set(unit, EGR_MIRROR_ENCAP_DATA_1m,
                         data_1_entry_p, ERSPAN__ERSPAN_HEADER_SAf,
                         mirror_dest->src_mac);

    /* Set tpid & vlan id. */
    /* coverity[result_independent_of_operands] */
    if (BCM_VLAN_VALID(BCM_VLAN_CTRL_ID(mirror_dest->vlan_id))) {
        fldval = (((uint32)mirror_dest->tpid << 16) | 
                  (uint32)mirror_dest->vlan_id);    
    } else {
        fldval = 0;
        /* keep the consistent data between software and hardware */
        mirror_dest->tpid = 0;
        mirror_dest->vlan_id = 0;
    }
    soc_EGR_MIRROR_ENCAP_DATA_1m_field32_set(unit, data_1_entry_p,
                         ERSPAN__ERSPAN_HEADER_VLAN_TAGf, fldval);

    /* Set ether type to ip. 0x800  */
    soc_EGR_MIRROR_ENCAP_DATA_1m_field32_set(unit, data_1_entry_p,
                         ERSPAN__ERSPAN_HEADER_ETYPEf, 0x800);

    /* keep the consistent data between software and hardware */
    mirror_dest->gre_protocol =
        (0 != mirror_dest->gre_protocol) ? mirror_dest->gre_protocol : 0x88be;

    /* Set protocol to given value, or default of GRE. 0x88be */
    soc_EGR_MIRROR_ENCAP_DATA_1m_field32_set(unit, data_1_entry_p,
                                             ERSPAN__ERSPAN_HEADER_GREf,
                                             mirror_dest->gre_protocol);
    /*
     *   Set IPv4 header. 
     */
    /* Version + 5 word no options length.  + Tos */
    /* Length, Id, Flags, Fragmentation offset. */
    idx = 4;

    ip_buffer[idx--] |= ((uint32)(0x45 << 24) |
                         (uint32)((mirror_dest->tos) << 16));

    /* Do not fragment bit */
    ip_buffer[idx--] |= (uint32)((mirror_dest->df) << 14);
    /* Ttl, Protocol (GRE 0x2f)*/
    ip_buffer[idx--] = (((uint32)mirror_dest->ttl << 24) | (0x2f << 16));

    /* Src Ip. */
    ip_buffer[idx--] = mirror_dest->src_addr;

    /* Dst Ip. */
    ip_buffer[idx] = mirror_dest->dst_addr;

    soc_EGR_MIRROR_ENCAP_DATA_1m_field_set(unit, data_1_entry_p,
                         ERSPAN__ERSPAN_HEADER_V4f, ip_buffer);

    /* Profile entries will be committed to HW by the calling function. */

    return (BCM_E_NONE);
}

#if defined(BCM_TOMAHAWK_SUPPORT)
/*
 * Function:
 *	    _bcm_tomahawk_mirror_sflow_tunnel_set
 * Purpose:
 *	   Prepare sFlow mirror tunnel encapsulation for Trident.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     bcm_mirror_destination_t      - (IN) Mirror destination descriptor.
 *     flags      - (IN) Mirror direction flags. 
 *     entries    - (IN) Pointer to EGR_MIRROR_ENCAP_* entries array
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_tomahawk_mirror_sflow_tunnel_set(int unit,
                                      bcm_mirror_destination_t *mirror_dest,
                                      int flags,
                                      void **entries)
{
    /*SW tunnel encap buffers.*/
    uint32 ip_buffer[_BCM_TD_MIRROR_V4_GRE_BUFFER_SZ];
    egr_mirror_encap_control_entry_t *control_entry_p;
    egr_mirror_encap_data_1_entry_t *data_1_entry_p;
    uint32 fldval;
    int                 idx;           /* Headers offset iterator.        */

    if (!soc_feature(unit, soc_feature_sflow_ing_mirror)) {
        return BCM_E_UNAVAIL;
    }

    if (mirror_dest->df > 1) {
        return (BCM_E_PARAM);
    }

    /* These entries were initialized by the calling function */
    control_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL];
    data_1_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1];

    if (control_entry_p == NULL || data_1_entry_p == NULL) {
        return BCM_E_INTERNAL;
    }

    sal_memset(ip_buffer, 0,
               _BCM_TD_MIRROR_V4_GRE_BUFFER_SZ * sizeof(uint32));

    /* Setup Mirror Control Memory */
    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         ENTRY_TYPEf, BCM_TD_MIRROR_ENCAP_TYPE_SFLOW);

    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         SFLOW__ADD_OPTIONAL_HEADERf,
                         BCM_TD_MIRROR_HEADER_ONLY);

    if ((BCM_VLAN_CTRL_ID(mirror_dest->vlan_id)) > 0) {
        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                             SFLOW__ADD_SFLOW_OUTER_VLANf, 1);
    }

    if (mirror_dest->flags & BCM_MIRROR_DEST_PAYLOAD_UNTAGGED) {
        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                                                  SFLOW__UNTAG_PAYLOADf, 1);
    }

    /*
     *   Set mirror tunnel DA SA. 
     */

    soc_mem_mac_addr_set(unit, EGR_MIRROR_ENCAP_DATA_1m,
                         data_1_entry_p, SFLOW__SFLOW_HEADER_DAf,
                         mirror_dest->dst_mac);
    soc_mem_mac_addr_set(unit, EGR_MIRROR_ENCAP_DATA_1m,
                         data_1_entry_p, SFLOW__SFLOW_HEADER_SAf,
                         mirror_dest->src_mac);

    /* Set tpid & vlan id. */
    if ((BCM_VLAN_CTRL_ID(mirror_dest->vlan_id) > 0)) {
        fldval = (((uint32)mirror_dest->tpid << 16) | 
                  (uint32)mirror_dest->vlan_id);    
    } else {
        fldval = 0;
        /* keep the consistent data between software and hardware */
        mirror_dest->tpid = 0;
        mirror_dest->vlan_id = 0;
    }
    soc_EGR_MIRROR_ENCAP_DATA_1m_field32_set(unit, data_1_entry_p,
                         SFLOW__SFLOW_HEADER_VLAN_TAGf, fldval);

    /* Set ether type to ip. 0x800  */
    soc_EGR_MIRROR_ENCAP_DATA_1m_field32_set(unit, data_1_entry_p,
                         SFLOW__SFLOW_HEADER_ETYPEf, 0x800);

    /*
     *   Set IPv4 header. 
     */
    /* Version + 5 word no options length.  + Tos */
    /* Length, Id, Flags, Fragmentation offset. */
    idx = 4;

    ip_buffer[idx--] |= ((uint32)(0x45 << 24) |
                         (uint32)((mirror_dest->tos) << 16));
  
    /* Do not fragment bit */
    ip_buffer[idx--] |= (uint32)((mirror_dest->df) << 14);
    /* TTL, Protocol UDP */
    ip_buffer[idx--] = (((uint32)mirror_dest->ttl << 24) | (0x11 << 16));

    /* Src IP. */
    ip_buffer[idx--] = mirror_dest->src_addr;

    /* Dst IP. */
    ip_buffer[idx] = mirror_dest->dst_addr;

    soc_EGR_MIRROR_ENCAP_DATA_1m_field_set(unit, data_1_entry_p,
                         SFLOW__SFLOW_HEADER_V4f, ip_buffer);

    /* UDP header */
    idx = 1;
    ip_buffer[idx--] = ((uint32)(mirror_dest->udp_src_port << 16) |
                         (uint32)(mirror_dest->udp_dst_port));
    ip_buffer[0] = 0;
    soc_EGR_MIRROR_ENCAP_DATA_1m_field_set(unit, data_1_entry_p,
                         SFLOW__SFLOW_HEADER_UDPf, ip_buffer);
    return (BCM_E_NONE);
}
#endif

/*
 * Function:
 *	    _bcm_trident_mirror_trill_tunnel_set
 * Purpose:
 *	   Prepare Trill mirror tunnel encapsulation for Trident.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     mirror_dest      - (IN) Mirror destination descriptor.
 *     flags      - (IN) Mirror direction flags. 
 *     entries    - (IN) Pointer to EGR_MIRROR_ENCAP_* entries array
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_trill_tunnel_set(int unit,
                                     bcm_mirror_destination_t *mirror_dest,
                                     int flags,
                                     void **entries)
{
     /*SW tunnel encap buffer.*/
    uint32      trill_buffer[_BCM_TD_MIRROR_V4_GRE_BUFFER_SZ];
    /* index to end of TRILL portion of buffer */
    int         idx = _BCM_TD_MIRROR_TRILL_BUFFER_SZ - 1;
    egr_mirror_encap_control_entry_t *control_entry_p;
    egr_mirror_encap_data_2_entry_t *data_2_entry_p;
    
    /* These entries were initialized by the calling function */
    control_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL];
    data_2_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2];

    sal_memset(trill_buffer, 0,
               _BCM_TD_MIRROR_V4_GRE_BUFFER_SZ * sizeof(uint32));

    trill_buffer[idx--] = ((uint32)(BCM_TD_MIRROR_TRILL_VERSION << 
                                    BCM_TD_MIRROR_TRILL_VERSION_OFFSET) | 
                           (uint32)(mirror_dest->trill_hopcount << 
                                    BCM_TD_MIRROR_TRILL_HOPCOUNT_OFFSET) | 
                           (uint32)(mirror_dest->trill_src_name));
    trill_buffer[idx] = ((uint32)(mirror_dest->trill_dst_name << 
                                  BCM_TD_MIRROR_TRILL_DEST_NAME_OFFSET));

    soc_EGR_MIRROR_ENCAP_DATA_2m_field_set(unit, data_2_entry_p,
                                           HEADER_DATAf, trill_buffer);

    /* sFlow, ERSPAN and RSPAN use the same field and encoding. */
    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         ERSPAN__ADD_OPTIONAL_HEADERf,
                         BCM_TD_MIRROR_HEADER_TRILL);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_trident_mirror_niv_tunnel_set
 * Purpose:
 *	   Prepare NIV mirror tunnel encapsulation for Trident.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     mirror_dest      - (IN) Mirror destination descriptor.
 *     flags      - (IN) Mirror direction flags. 
 *     entries    - (IN) Pointer to EGR_MIRROR_ENCAP_* entries array
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_niv_tunnel_set(int unit,
                                   bcm_mirror_destination_t *mirror_dest,
                                   int flags,
                                   void **entries)
{
    /*SW tunnel encap buffers.*/
    uint32 niv_buffer;
    egr_mirror_encap_control_entry_t *control_entry_p;
    egr_mirror_encap_data_2_entry_t *data_2_entry_p;

    /* These entries were initialized by the calling function */
    control_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL];
    data_2_entry_p = entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2];

    niv_buffer = 0;
    niv_buffer = ((uint32) mirror_dest->niv_src_vif) & \
                  _BCM_TD_MIRROR_NIV_SRC_VIF_MASK;
    if (mirror_dest->niv_flags & BCM_MIRROR_NIV_LOOP) {
        niv_buffer |= _BCM_TD_MIRROR_NIV_LOOP_BIT;
    }
    niv_buffer |= ((uint32) mirror_dest->niv_dst_vif <<
                   _BCM_TD_MIRROR_NIV_DST_VIF_OFFSET);

    soc_EGR_MIRROR_ENCAP_DATA_2m_field32_set(unit, data_2_entry_p,
                                             VNTAG_HEADERf, niv_buffer);

    /* ERSPAN and RSPAN use the same field and encoding. */
    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         ERSPAN__ADD_OPTIONAL_HEADERf,
                         BCM_TD_MIRROR_HEADER_VNTAG);

    return (BCM_E_NONE);
}

#endif

/*
 * Function:
 *	    _bcm_mirror_etag_tunnel_set
 * Purpose:
 *	   Prepare ETAG mirror tunnel encapsulation.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     mirror_dest      - (IN) Mirror destination descriptor.
 *     flags      - (IN) Mirror direction flags. 
 *     entries    - (IN) Pointer to EGR_MIRROR_ENCAP_* entries array
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_mirror_etag_tunnel_set(int unit,
                            bcm_mirror_destination_t *mirror_dest,
                            int flags,
                            void **entries)
{
    /*SW tunnel encap buffers.*/
    uint32 etag_buffer;
    egr_mirror_encap_control_entry_t *control_entry_p;
    egr_mirror_encap_data_2_entry_t *data_2_entry_p;

    /* These entries were initialized by the calling function */
    control_entry_p = entries[BCM_EGR_MIRROR_ENCAP_ENTRIES_CONTROL];
    data_2_entry_p = entries[BCM_EGR_MIRROR_ENCAP_ENTRIES_DATA_2];

    etag_buffer = 0;

    etag_buffer |= ((uint32) mirror_dest->etag_src_vid & _BCM_MIRROR_ETAG_SRC_VID_MASK)
                    << _BCM_MIRROR_ETAG_SRC_VID_OFFSET;
    etag_buffer |= (uint32) mirror_dest->etag_dst_vid & _BCM_MIRROR_ETAG_DST_VID_MASK;

    soc_EGR_MIRROR_ENCAP_DATA_2m_field32_set(unit, data_2_entry_p,
                                             HEADER_DATAf, etag_buffer);

    /* ERSPAN and RSPAN use the same field and encoding. */
    soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, control_entry_p,
                         ERSPAN__ADD_OPTIONAL_HEADERf,
                         BCM_MIRROR_HEADER_ETAG);

    return (BCM_E_NONE);
}


/*
 * Function:
 *	    _bcm_trx_mirror_egr_erspan_write
 * Purpose:
 *	   Program HW buffer.  
 * Parameters:
 *	   unit     - (IN) BCM device number.
 *     index    - (IN) Mtp index.
 *     buffer   - (IN) Tunnel encapsulation buffer.
 *     flags    - (IN) Mirror direction flags.  
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trx_mirror_egr_erspan_write(int unit, int index, uint32 *buffer, int flags)
{
    bcm_mirror_destination_t *mirror_dest;/* Destination & encapsulation. */
    _bcm_mtp_config_p        mtp_cfg;     /* MTP configuration.           */
    egr_erspan_entry_t       hw_buf;      /* Hw table buffer              */

#if defined(BCM_TRIUMPH2_SUPPORT)
    uint32 erspan_enable;
#endif

    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

    /* Advance index according to flags. */
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        index += 4;
    } else if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) { /* True Egress */
        index += 8;
    }

    mirror_dest = MIRROR_DEST(unit, mtp_cfg->dest_id);

    /* Reset hw buffer. */
    sal_memset(&hw_buf, 0, sizeof(egr_erspan_entry_t));

#if defined(BCM_TRIUMPH2_SUPPORT)
    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, EGR_ERSPANm, MEM_BLOCK_ALL, index, &hw_buf));

    soc_EGR_ERSPANm_field_get(unit, &hw_buf, ERSPAN_ENABLEf, &erspan_enable);
    /* If there is a replace flag, check if the entry exsits in the memory.
           If no entry is found, return SOC_E_NOT_FOUND*/
    if ((mirror_dest->flags) & BCM_MIRROR_DEST_REPLACE) {
        if ((MIRROR_DEST_REF_COUNT(unit, mtp_cfg->dest_id) == 0) ||
            (erspan_enable == 0)) {
            return SOC_E_NOT_FOUND;
        }
    }
#endif

    /* Enable tunneling for mtp . */
    soc_mem_field32_set(unit, EGR_ERSPANm, &hw_buf, ERSPAN_ENABLEf, 1);

    /* Set untag payload flag. */
    if (mirror_dest->flags & BCM_MIRROR_DEST_PAYLOAD_UNTAGGED) {
        soc_EGR_ERSPANm_field32_set(unit, &hw_buf, UNTAG_PAYLOADf, 1);
    }

    /* Set tunnel header.. */
    /* coverity[result_independent_of_operands] */
    if (BCM_VLAN_VALID(BCM_VLAN_CTRL_ID(mirror_dest->vlan_id))) {
        soc_EGR_ERSPANm_field32_set(unit, &hw_buf, USE_TAGGED_HEADERf, 1);
        soc_EGR_ERSPANm_field_set(unit, &hw_buf, HEADER_TAGGEDf, buffer);
    } else {
        soc_EGR_ERSPANm_field_set(unit, &hw_buf, HEADER_UNTAGGEDf, buffer);
    }

    /* Write buffer to hw. */
    BCM_IF_ERROR_RETURN 
        (soc_mem_write(unit, EGR_ERSPANm, MEM_BLOCK_ALL, index, &hw_buf));

    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_trx_mirror_ipv4_gre_tunnel_set
 * Purpose:
 *	   Prepare IPv4 mirror tunnel encapsulation.
 * Parameters:
 *	   unit       - (IN) BCM device number
 *     index      - (IN) Mtp index.
 *     flags      - (IN) Mirror direction flags. 
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trx_mirror_ipv4_gre_tunnel_set(int unit, int index, int flags)
{
    bcm_mirror_destination_t *mirror_dest;/* Destination & encapsulation.   */
    uint32 buffer[_BCM_TRX_MIRROR_TUNNEL_BUFFER_SZ];/*SW tunnel encap buffer.*/
    _bcm_mtp_config_p   mtp_cfg;       /* Mtp configuration.              . */
    int                 idx;           /* Headers offset iterator.          */

    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

    mirror_dest = MIRROR_DEST(unit, mtp_cfg->dest_id);

    sal_memset(buffer, 0, _BCM_TRX_MIRROR_TUNNEL_BUFFER_SZ * sizeof(uint32));

    /*
     *   L2 Header. 
     */
    /* coverity[result_independent_of_operands] */
    idx = BCM_VLAN_VALID(BCM_VLAN_CTRL_ID(mirror_dest->vlan_id)) ? 10 : 9;
      
    /* Destination mac address. */
    buffer[idx--] = (((uint32)(mirror_dest->dst_mac)[0]) << 8 | \
                     ((uint32)(mirror_dest->dst_mac)[1]));

    buffer[idx--] = (((uint32)(mirror_dest->dst_mac)[2]) << 24 | \
                     ((uint32)(mirror_dest->dst_mac)[3]) << 16 | \
                     ((uint32)(mirror_dest->dst_mac)[4]) << 8  | \
                     ((uint32)(mirror_dest->dst_mac)[5])); 

    /* Source mac address. */
    buffer[idx--] = (((uint32)(mirror_dest->src_mac)[0]) << 24 | \
                     ((uint32)(mirror_dest->src_mac)[1]) << 16 | \
                     ((uint32)(mirror_dest->src_mac)[2]) << 8  | \
                     ((uint32)(mirror_dest->src_mac)[3])); 

    buffer[idx] = (((uint32)(mirror_dest->src_mac)[4]) << 24 | \
                   ((uint32)(mirror_dest->src_mac)[5]) << 16); 

    /* Set tpid & vlan id. */
    /* coverity[result_independent_of_operands] */
    if (BCM_VLAN_VALID(BCM_VLAN_CTRL_ID(mirror_dest->vlan_id))) {
        /* Tpid. */
        buffer[idx--] |= (((uint32)(mirror_dest->tpid >> 8)) << 8 | \
                          ((uint32)(mirror_dest->tpid & 0xff)));

        /* Priority,  Cfi, Vlan id. */
        buffer[idx] = (((uint32)(mirror_dest->vlan_id >> 8)) << 24 | \
                       ((uint32)(mirror_dest->vlan_id & 0xff) << 16));
    }

    /* Set ether type to ip. 0x800  */
    buffer[idx--] |= (uint32)(0x08 << 8);

    /*
     *   IPv4 header. 
     */
    /* Version + 5 word no options length.  + Tos */
    /* Length, Id, Flags, Fragmentation offset. */
    buffer[idx--] |= ((uint32)(0x45 << 24) | \
                      (uint32)(mirror_dest->tos) << 16);

    idx--;
    /* Ttl, Protocol (GRE 0x2f)*/
    buffer[idx--] = (((uint32)mirror_dest->ttl << 24) | (0x2f << 16));

    /* Src Ip. */
    buffer[idx--] = mirror_dest->src_addr;

    /* Dst Ip. */
    buffer[idx--] = mirror_dest->dst_addr;

    /*
     *   Gre header. 
     */

    /* Protocol. 0x88be */
    buffer[idx] = (0 != mirror_dest->gre_protocol) ?
        mirror_dest->gre_protocol : 0x88be;

    /* swap byte in tunnel buffer. */
    /*  _shr_bit_rev8(buffer[idx]); */

    BCM_IF_ERROR_RETURN
        (_bcm_trx_mirror_egr_erspan_write(unit, index, (uint32 *)buffer, flags));

    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_trx_mirror_rspan_write
 * Purpose:
 *	   Prepare & write L2 mirror tunnel encapsulation.
 * Parameters:
 *	   unit       - (IN) BCM device number
 *     index      - (IN) Mtp index.
 *     port- (IN) 
 *     flags      - (IN) Mirror direction flags.
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trx_mirror_rspan_write(int unit, int index, bcm_port_t port, int flags)
{
    bcm_mirror_destination_t *mirror_dest;/* Destination & encapsulation. */
    _bcm_mtp_config_p   mtp_cfg;          /* Mtp configuration.           */
    uint32              hw_buffer;        /* HW buffer.                   */


    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

    mirror_dest = MIRROR_DEST(unit, mtp_cfg->dest_id);

    /* Outer vlan tag. */
    hw_buffer = (((uint32)mirror_dest->tpid << 16) | 
                  (uint32)mirror_dest->vlan_id);

    BCM_IF_ERROR_RETURN(soc_reg_field32_modify(unit, EGR_RSPAN_VLAN_TAGr, 
                                               port, TAGf, hw_buffer));
    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_trx_mirror_l2_tunnel_set
 * Purpose:
 *	   Programm mirror L2 tunnel 
 * Parameters:
 *	   unit       - (IN) BCM device number
 *     index      - (IN) Mtp index.
 *     trunk_arr  - (IN) Mirror destinations array.
 *     flags      - (IN) Mirror direction flags.
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trx_mirror_l2_tunnel_set(int unit, int index, 
                              bcm_gport_t *trunk_arr, int flags)
{
    bcm_module_t  my_modid;    /* Local modid.                   */
    int           idx;         /* Trunk members iteration index. */
    bcm_module_t  mod_out;     /* Hw mapped modid.               */
    bcm_port_t    port_out;    /* Hw mapped port number.         */
    bcm_module_t  modid;       /* Application space modid.       */
    bcm_port_t    port;        /* Application space port number. */

    /* Input parameters check. */ 
    if (NULL == trunk_arr) {
        return (BCM_E_PARAM);
    }

    /* Get local base module id. */
    BCM_IF_ERROR_RETURN (bcm_esw_stk_my_modid_get(unit, &my_modid));

    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT; idx++) {
        modid = BCM_GPORT_MODPORT_MODID_GET(trunk_arr[idx]);
        port = BCM_GPORT_MODPORT_PORT_GET(trunk_arr[idx]);
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, modid, port, 
                                    &mod_out, &port_out));
        if (mod_out != my_modid) {
            /* Not a local front-panel port.
             * Use bcm_mirror_vlan_set for remote ports. */
            return BCM_E_PARAM;
        }

        if (0 == IS_E_PORT(unit, port_out)) {
            /* Not a local front-panel port.
             * Use bcm_mirror_vlan_set for remote ports. */
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_trx_mirror_rspan_write(unit, index, port_out, flags));
    }

    return (BCM_E_NONE);
}

#ifdef BCM_TOMAHAWK_SUPPORT
/*
 * Function:
 *	    _bcm_mirror_sflow_tunnel_set
 * Purpose:
 *	   Prepare sFlow mirror tunnel encapsulation.
 * Parameters:
 *     unit       - (IN) BCM device number
 *     index      - (IN) Mtp index.
 *     trunk_arr  - (IN) Mirror destinations array.
 *     flags      - (IN) Mirror direction flags.
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_mirror_sflow_tunnel_set(int unit, int index,
                           bcm_gport_t *trunk_arr, int flags)
{
    bcm_mirror_destination_t *mirror_dest; /* Destination & Encapsulation.*/
    _bcm_mtp_config_p   mtp_cfg;           /* MTP configuration .         */
    int rv = BCM_E_NONE;                   /* Operation return status.    */
    egr_mirror_encap_control_entry_t control_entry;
    egr_mirror_encap_data_1_entry_t data_1_entry;
    egr_mirror_encap_data_2_entry_t data_2_entry;
    void *entries[EGR_MIRROR_ENCAP_ENTRIES_NUM];
    uint32 profile_index;

    sal_memset(&control_entry, 0, sizeof(control_entry));
    sal_memset(&data_1_entry, 0, sizeof(data_1_entry));
    sal_memset(&data_2_entry, 0, sizeof(data_2_entry));

    entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL] = &control_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1] = &data_1_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2] = &data_2_entry;

    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

    mirror_dest = MIRROR_DEST(unit, mtp_cfg->dest_id);

   /* Configure sFlow headers */
   rv = _bcm_tomahawk_mirror_sflow_tunnel_set(unit, mirror_dest, flags, entries);

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_egr_mirror_encap_entry_add(unit, entries, &profile_index);
    }

    if (BCM_SUCCESS(rv)) {
        /* Supply the correct profile index to the selected MTPs */
        rv = _bcm_egr_mirror_encap_entry_mtp_update(unit, index,
                                                    profile_index, flags);
    }

    return rv;
}
#endif /* BCM_TOMAHAWK_SUPPORT */

#ifdef BCM_TRIDENT2_SUPPORT
/* Set mirror tunnel for a shared-id mirror destination. */
STATIC int
_bcm_td2_mirror_shared_dest_tunnel_set(int unit, int index,
                                       bcm_gport_t *trunk_arr, int flags,
                                       bcm_gport_t mir_dest_id)
{
    int rv = BCM_E_NONE;
    bcm_mirror_destination_t mirror_dest_node; /* Destination node */
    uint32 prof_idx[BCM_SWITCH_TRUNK_MAX_PORTCNT] = {-1,-1,-1,-1,-1,-1,-1,-1};
    egr_im_mtp_index_entry_t  egr_im_mtp_index_entry;
    egr_em_mtp_index_entry_t  egr_em_mtp_index_entry;
    egr_ep_redirect_em_mtp_index_entry_t  egr_em_redirect_mtp_index_entry;
    egr_mirror_encap_control_entry_t control_entry;
    egr_mirror_encap_data_1_entry_t data_1_entry;
    egr_mirror_encap_data_2_entry_t data_2_entry;
    void *entries[EGR_MIRROR_ENCAP_ENTRIES_NUM];
    int update_eme = FALSE;                /* EGR_MIRROR_ENCAP_* pointers */
    int max_num_trunk_ports = 0;
    int offset = 0;
    bcm_gport_t     mir_dest_gport_array[BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
    int             mir_dest_gport_count = 0, i, id;
    bcm_trunk_t     trunk = BCM_TRUNK_INVALID;
    bcm_module_t    modid = 0;
    bcm_port_t      port = -1;
    uint32          old_profile_index = -1;
    int             profile_ref_count = 0;

    if (NULL == trunk_arr || !BCM_GPORT_IS_MIRROR(mir_dest_id)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&control_entry, 0, sizeof(control_entry));
    sal_memset(&data_1_entry, 0, sizeof(data_1_entry));
    sal_memset(&data_2_entry, 0, sizeof(data_2_entry));

    entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL] = &control_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1] = &data_1_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2] = &data_2_entry;

    BCM_IF_ERROR_RETURN(
        _bcm_mirror_dest_mtp_gport_get(unit,
                                       mir_dest_id,
                                       mir_dest_gport_array,
                                       &mir_dest_gport_count));

    if (mir_dest_gport_count <= 0) {
        return (BCM_E_INIT);
    }

    max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
    offset = index * max_num_trunk_ports;

    /* Remove the previous profile index if any */
    for (i = 0; i < max_num_trunk_ports; i++, offset++) {
        sal_memset(&egr_im_mtp_index_entry, 0,
                   sizeof(egr_im_mtp_index_entry_t));
        sal_memset(&egr_em_mtp_index_entry, 0,
                   sizeof(egr_em_mtp_index_entry_t));
        sal_memset(&egr_em_redirect_mtp_index_entry, 0,
                   sizeof(egr_ep_redirect_em_mtp_index_entry_t));
        old_profile_index = -1;
        profile_ref_count = 0;

        if (flags & BCM_MIRROR_PORT_INGRESS) {
            rv = READ_EGR_IM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset,
                                        &egr_im_mtp_index_entry);
            if (BCM_SUCCESS(rv) &&
               (0 != soc_EGR_IM_MTP_INDEXm_field32_get(
                         unit,
                         &egr_im_mtp_index_entry,
                         MIRROR_ENCAP_ENABLEf))) {
                old_profile_index = soc_EGR_IM_MTP_INDEXm_field32_get(
                                        unit,
                                        &egr_im_mtp_index_entry,
                                        MIRROR_ENCAP_INDEXf);
            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS && BCM_SUCCESS(rv)) {
            rv = READ_EGR_EM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset,
                                        &egr_em_mtp_index_entry);
            if (BCM_SUCCESS(rv) &&
               (0 != soc_EGR_EM_MTP_INDEXm_field32_get(
                         unit,
                         &egr_em_mtp_index_entry,
                         MIRROR_ENCAP_ENABLEf))) {
                old_profile_index = soc_EGR_EM_MTP_INDEXm_field32_get(
                                        unit,
                                        &egr_em_mtp_index_entry,
                                        MIRROR_ENCAP_INDEXf);
            }
        }

        if (-1 != old_profile_index) {
            BCM_IF_ERROR_RETURN
                (_bcm_egr_mirror_encap_entry_ref_get(unit,
                                                     old_profile_index,
                                                     &profile_ref_count));
            if (profile_ref_count != 0) {
                rv = _bcm_egr_mirror_encap_entry_delete(unit,
                                                        old_profile_index);
            }
        }
    }

    for (i = 0; i < max_num_trunk_ports; i++) {
        sal_memset(&mirror_dest_node, 0, sizeof(bcm_mirror_destination_t));
        sal_memset(&control_entry, 0, sizeof(control_entry));
        sal_memset(&data_1_entry, 0, sizeof(data_1_entry));
        sal_memset(&data_2_entry, 0, sizeof(data_2_entry));

        BCM_IF_ERROR_RETURN(
            _bcm_mirror_dest_mtp_get(unit,
                                     mir_dest_id,
                                     trunk_arr[i],
                                     &mirror_dest_node));
        if (!(mirror_dest_node.flags & BCM_MIRROR_DEST_TUNNELS)) {
            continue;
        }

        if (mirror_dest_node.flags & BCM_MIRROR_DEST_TUNNEL_IP_GRE) {
            if (4 == mirror_dest_node.version) {
                if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
                    rv = _bcm_trident_mirror_ipv4_gre_tunnel_set(
                             unit,
                             &mirror_dest_node,
                             flags,
                             entries);
                    update_eme = TRUE;
                }
            } else {
                rv = (BCM_E_UNAVAIL);
            }
        } else if (mirror_dest_node.flags & BCM_MIRROR_DEST_TUNNEL_SFLOW) {
#if defined(BCM_TOMAHAWK_SUPPORT)
            rv = _bcm_tomahawk_mirror_sflow_tunnel_set(unit,
                                                       &mirror_dest_node,
                                                       flags,
                                                       entries);
            update_eme = TRUE;
#else
            rv = BCM_E_UNAVAIL;
#endif
        }

        if (BCM_SUCCESS(rv) &&
            (mirror_dest_node.flags & BCM_MIRROR_DEST_TUNNEL_L2)) {
            if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
                rv = _bcm_trident_mirror_l2_tunnel_set(unit,
                                                       &mirror_dest_node,
                                                       flags,
                                                       entries);
                update_eme = TRUE;
            }
        }
        if (BCM_SUCCESS(rv) &&
            (mirror_dest_node.flags & BCM_MIRROR_DEST_TUNNEL_TRILL)) {
            rv = _bcm_trident_mirror_trill_tunnel_set(unit,
                                                      &mirror_dest_node,
                                                      flags,
                                                      entries);
            update_eme = TRUE;
        }
        if (BCM_SUCCESS(rv) &&
            (mirror_dest_node.flags & BCM_MIRROR_DEST_TUNNEL_NIV)) {
            rv = _bcm_trident_mirror_niv_tunnel_set(unit,
                                                    &mirror_dest_node,
                                                    flags,
                                                    entries);
            update_eme = TRUE;
        }
        if (BCM_SUCCESS(rv) &&
            (mirror_dest_node.flags & BCM_MIRROR_DEST_TUNNEL_ETAG)) {
            if(soc_feature(unit, soc_feature_port_extension)) {
                rv = _bcm_mirror_etag_tunnel_set(unit,
                                                 &mirror_dest_node,
                                                 flags,
                                                 entries);
                update_eme = TRUE;
            }
        }
        if (BCM_SUCCESS(rv) && update_eme) {
            rv = _bcm_egr_mirror_encap_entry_add(unit, entries, prof_idx+i);
        }
    }

    offset = index * max_num_trunk_ports;
    for (i = 0; i < max_num_trunk_ports; i++, offset++) {
        BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(
                    unit, trunk_arr[i], &modid, &port, &trunk, &id));
        if (prof_idx[i] != -1) {
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_field32_modify(unit,
                                                EGR_IM_MTP_INDEXm,
                                                offset,
                                                MIRROR_ENCAP_ENABLEf, 1));

                    BCM_IF_ERROR_RETURN
                        (soc_mem_field32_modify(unit,
                                                EGR_IM_MTP_INDEXm,
                                                offset,
                                                MIRROR_ENCAP_INDEXf,
                                                prof_idx[i]));
            }
            /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
            if (flags & BCM_MIRROR_PORT_EGRESS) {
#ifdef BCM_TOMAHAWK_SUPPORT
                if (SOC_INFO(unit).th_tflow_enabled == 1) {
                    /* Enable encap only for sFlow tunnel header on LB port */
                    if (BCM_PBMP_MEMBER(PBMP_LB(unit), port)) {
                        BCM_IF_ERROR_RETURN
                            (soc_mem_field32_modify(unit,
                                                    EGR_IM_MTP_INDEXm,
                                                    offset,
                                                    MIRROR_ENCAP_ENABLEf,
                                                    1));
                        BCM_IF_ERROR_RETURN
                            (soc_mem_field32_modify(unit,
                                                    EGR_IM_MTP_INDEXm,
                                                    offset,
                                                    MIRROR_ENCAP_INDEXf,
                                                    prof_idx[i]));
                    }

                } else
#endif /* BCM_TOMAHAWK_SUPPORT */
                {
                        BCM_IF_ERROR_RETURN
                            (soc_mem_field32_modify(unit,
                                                    EGR_EM_MTP_INDEXm,
                                                    offset,
                                                    MIRROR_ENCAP_ENABLEf,
                                                    1));
                        BCM_IF_ERROR_RETURN
                            (soc_mem_field32_modify(unit,
                                                    EGR_EM_MTP_INDEXm,
                                                    offset,
                                                    MIRROR_ENCAP_INDEXf,
                                                    prof_idx[i]));
                }
            }
        }
    }

    return (BCM_E_NONE);
}
#endif /* BCM_TRIDENT2_SUPPORT */

/*
 * Function:
 *	    _bcm_trx_mirror_tunnel_set
 * Purpose:
 *	   Initialize mirror tunnel 
 * Parameters:
 *     unit       - (IN) BCM device number
 *     index      - (IN) Mtp index.
 *     trunk_arr  - (IN) 
 *     flags      - (IN) Mirror direction flags.
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trx_mirror_tunnel_set(int unit, int index,
                           bcm_gport_t *trunk_arr, int flags)
{
    bcm_mirror_destination_t *mirror_dest; /* Destination & Encapsulation.*/
    _bcm_mtp_config_p   mtp_cfg;           /* MTP configuration .         */
    int rv = BCM_E_NONE;                   /* Operation return status.    */
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    egr_mirror_encap_control_entry_t control_entry;
    egr_mirror_encap_data_1_entry_t data_1_entry;
    egr_mirror_encap_data_2_entry_t data_2_entry;
    void *entries[EGR_MIRROR_ENCAP_ENTRIES_NUM];
    uint32 profile_index;
    int update_eme = FALSE;                /* EGR_MIRROR_ENCAP_* pointers */
    int max_num_trunk_ports = 0;
    int offset = 0;

    sal_memset(&control_entry, 0, sizeof(control_entry));
    sal_memset(&data_1_entry, 0, sizeof(data_1_entry));
    sal_memset(&data_2_entry, 0, sizeof(data_2_entry));

    entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL] = &control_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1] = &data_1_entry;
    entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2] = &data_2_entry;
#endif /* TRIDENT || GREYHOUND  */
    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

    mirror_dest = MIRROR_DEST(unit, mtp_cfg->dest_id);

#ifdef BCM_TRIDENT2_SUPPORT
    if (mirror_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
        return _bcm_td2_mirror_shared_dest_tunnel_set(unit, index,
                                                      trunk_arr, flags,
                                                      mtp_cfg->dest_id);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    if (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_IP_GRE) {
        if (4 == mirror_dest->version) {
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
            if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
                rv = _bcm_trident_mirror_ipv4_gre_tunnel_set(unit, mirror_dest,
                                                             flags, entries);
                update_eme = TRUE;
            } else 
#endif /* TRIDENT || GREYHOUND  */
            {
                rv = _bcm_trx_mirror_ipv4_gre_tunnel_set(unit, index, flags);
            }            
        } else {
            rv = (BCM_E_UNAVAIL);
        }
    } else if (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_SFLOW) {
#if defined(BCM_TOMAHAWK_SUPPORT)
        rv = _bcm_tomahawk_mirror_sflow_tunnel_set(unit, mirror_dest, flags, entries);
        update_eme = TRUE;
#else
        rv = BCM_E_UNAVAIL;
#endif
    }

    if (BCM_SUCCESS(rv) &&
        (0 != (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_L2))) {
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
        if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
            rv = _bcm_trident_mirror_l2_tunnel_set(unit, mirror_dest,
                                                   flags, entries);
            update_eme = TRUE;
        } else
#endif /* TRIDENT || GREYHOUND  */
        {
            rv = _bcm_trx_mirror_l2_tunnel_set(unit, index, trunk_arr, flags);
        }
    }
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    /*
     * Note: Trill and NIV features are checked when the mirror
     * destination is created.  Thus, the flags will not be
     * set on a device which doesn't support the feature.
     */
    if (BCM_SUCCESS(rv) &&
        (0 != (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_TRILL))) {
        rv = _bcm_trident_mirror_trill_tunnel_set(unit, mirror_dest,
                                                  flags, entries);
        update_eme = TRUE;
    }
    if (BCM_SUCCESS(rv) &&
        (0 != (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_NIV))) {
        rv = _bcm_trident_mirror_niv_tunnel_set(unit, mirror_dest,
                                                flags, entries);
        update_eme = TRUE;
    }
    
    if (BCM_SUCCESS(rv) &&
        (0 != (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_ETAG))) {
        if(soc_feature(unit, soc_feature_port_extension)) {
            rv = _bcm_mirror_etag_tunnel_set(unit, mirror_dest,
                                                    flags, entries);
            update_eme = TRUE;
        }
    }

    /* Remove the previous profile index if any */
    if (BCM_SUCCESS(rv) && update_eme && 
        (mirror_dest->flags & BCM_MIRROR_DEST_REPLACE)) {
        uint32 old_profile_index = -1;
        int profile_ref_count = 0;
        egr_im_mtp_index_entry_t  egr_im_mtp_index_entry;
        egr_em_mtp_index_entry_t  egr_em_mtp_index_entry;
#if defined(BCM_TRIDENT_SUPPORT)
        egr_ep_redirect_em_mtp_index_entry_t  egr_em_redirect_mtp_index_entry;
#endif /* TRIDENT */
#ifdef BCM_METROLITE_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
        max_num_trunk_ports = 4;
    } else
#endif
    max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;

        offset = index * max_num_trunk_ports;

        if (flags & BCM_MIRROR_PORT_INGRESS) {
            rv = READ_EGR_IM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset, 
                                        &egr_im_mtp_index_entry);
            if (BCM_SUCCESS(rv) &&
               (0 != soc_EGR_IM_MTP_INDEXm_field32_get(unit, 
                                                    &egr_im_mtp_index_entry,
                                                    MIRROR_ENCAP_ENABLEf))) {
                 old_profile_index =
                     soc_EGR_IM_MTP_INDEXm_field32_get(unit, 
                                                    &egr_im_mtp_index_entry,
                                                    MIRROR_ENCAP_INDEXf);

            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS && BCM_SUCCESS(rv)) {
            rv = READ_EGR_EM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset, 
                                        &egr_em_mtp_index_entry);
            if (BCM_SUCCESS(rv) &&
               (0 != soc_EGR_EM_MTP_INDEXm_field32_get(unit, 
                                                    &egr_em_mtp_index_entry,
                                                    MIRROR_ENCAP_ENABLEf))) {
                old_profile_index =
                     soc_EGR_EM_MTP_INDEXm_field32_get(unit, 
                                                       &egr_em_mtp_index_entry,
                                                       MIRROR_ENCAP_INDEXf);

            }
        }

#if defined(BCM_TRIDENT_SUPPORT)
        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE && BCM_SUCCESS(rv)) {
            rv = READ_EGR_EP_REDIRECT_EM_MTP_INDEXm(unit, MEM_BLOCK_ANY, offset, 
                                             &egr_em_redirect_mtp_index_entry);
            if (BCM_SUCCESS(rv) &&
               (0 != soc_EGR_EP_REDIRECT_EM_MTP_INDEXm_field32_get(unit, 
                                             &egr_em_redirect_mtp_index_entry,
                                             MIRROR_ENCAP_ENABLEf))) {
                old_profile_index =
                     soc_EGR_EP_REDIRECT_EM_MTP_INDEXm_field32_get(unit, 
                                              &egr_em_redirect_mtp_index_entry,
                                              MIRROR_ENCAP_INDEXf);

            }
        }
#endif /* TRIDENT */

        if (-1 != old_profile_index) {
            BCM_IF_ERROR_RETURN
                (_bcm_egr_mirror_encap_entry_ref_get(unit,
                                                     old_profile_index,
                                                     &profile_ref_count));
            if (profile_ref_count != 0) {
                rv = _bcm_egr_mirror_encap_entry_delete(unit,
                                                        old_profile_index);
            }
        }
    }

    if (BCM_SUCCESS(rv) && update_eme) {
        rv = _bcm_egr_mirror_encap_entry_add(unit, entries, &profile_index);
    }

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        if (BCM_SUCCESS(rv) && update_eme) {
            /* Supply the correct profile index to the selected MTPs */
            rv = _bcm_egr_mirror_encap_entry_mtp_enable(unit, index,
                                                        trunk_arr, flags);
        }
    }
#endif

    if (BCM_SUCCESS(rv) && update_eme) {
        /* Supply the correct profile index to the selected MTPs */
        rv = _bcm_egr_mirror_encap_entry_mtp_update(unit, index,
                                                    profile_index, flags);
    }
#endif /* TRIDENT || GREYHOUND  */

    return (rv);
}
#endif /* BCM_TRX_SUPPORT */


#if defined(BCM_TRIDENT_SUPPORT)
/*
 * Function:
 *	    _bcm_trident_mtp_init
 * Purpose:
 *	   Initialize mirror target port for TRIDENT devices. 
 * Parameters:
 *	   unit       - (IN)BCM device number
 *     index      - (IN)Mtp index.
 *     trunk_arr  - (IN)Trunk members array. 
 *     flags      - (IN)Filled entry flags(BCM_MIRROR_PORT_INGRESS/EGRESS
 *                    or both. In case both flags are specied
 *                    ingress & egress configuration is assumed to be
 *                    idential.
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_trident_mtp_init(int unit, int index, bcm_gport_t *trunk_arr, int flags)
{
    bcm_gport_t               mirror_dest;
    bcm_port_t                port_out;
    bcm_module_t              mod_out;
    _bcm_mtp_config_p         mtp_cfg;
    int                       offset;
    int                       idx, id, isLocal;
    bcm_trunk_t               trunk = BCM_TRUNK_INVALID;
    bcm_module_t              modid = 0;
    bcm_port_t                port = -1;
    im_mtp_index_entry_t      mtp_entry, *mtp;
    uint32                    egr_mtp;
    int                       member_count = 0, rtag, encap_index;
    uint32                    replace_flag = 0;
    egr_im_mtp_index_entry_t  egr_mtp_index_entry;
    int                       max_num_trunk_ports = 0;
#ifdef BCM_TRIDENT2_SUPPORT
    int                       mir_dest_flag = 0;
#endif /* BCM_TRIDENT2_SUPPORT */

    /* HW does not store the trunk ID here, so during Warm Boot
     * we must take a mod,port and reverse map it to the trunk
     * This will work because we will have the T bit to indicate
     * a trunk, and the trunk module recovery takes place before
     * mirror, so the reverse mapping is available in SW. */
    static const soc_field_t port_field[] = {
        PORT_NUM_0f, PORT_NUM_1f, PORT_NUM_2f, PORT_NUM_3f, 
        PORT_NUM_4f, PORT_NUM_5f, PORT_NUM_6f, PORT_NUM_7f};
    static const soc_field_t module_field[] = {
            MODULE_ID_0f, MODULE_ID_1f, MODULE_ID_2f, MODULE_ID_3f, 
            MODULE_ID_4f, MODULE_ID_5f, MODULE_ID_6f, MODULE_ID_7f};

    mtp = &mtp_entry;

    /* Input parameters check */ 
    if (NULL == trunk_arr) {
        return (BCM_E_PARAM);
    }

    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);
    sal_memset(mtp, 0, sizeof(*mtp));

    /* Parse destination trunk / port & module. */
    mirror_dest = MIRROR_DEST_GPORT(unit, mtp_cfg->dest_id);
#ifdef BCM_TRIDENT2_SUPPORT
    mir_dest_flag = (MIRROR_DEST(unit, mtp_cfg->dest_id))->flags;
#endif /* BCM_TRIDENT2_SUPPORT */
    if (BCM_GPORT_IS_TRUNK(mirror_dest)) {
        trunk = BCM_GPORT_TRUNK_GET(mirror_dest);
        BCM_IF_ERROR_RETURN
            (_bcm_trunk_id_validate(unit, trunk));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_trunk_active_member_get(unit, trunk, NULL, 0, NULL, 
                                              &member_count));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_trunk_rtag_get(unit, trunk, &rtag));
        
        if (member_count > BCM_TD_MIRROR_TRUNK_MAX_PORTCNT) {
            member_count = BCM_TD_MIRROR_TRUNK_MAX_PORTCNT;
        }
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, Tf, 1);

        if (0 == member_count) {
            port = SOC_PORT_ADDR_MAX(unit);
            while (0 <= port) {
                if (!SOC_PORT_VALID(unit, port)) {
                    /* Found invalid port, use as black hole */
                    break;
                }
                port--;
            }
            if (port < 0) {
                /* Couldn't find an usused port, give up on empty trunk */
                return BCM_E_PORT;
            }

            /* Get local modid. */
            BCM_IF_ERROR_RETURN(_bcm_esw_local_modid_get(unit, &modid));

            soc_IM_MTP_INDEXm_field32_set(unit, mtp, COUNTf, 0);
            soc_IM_MTP_INDEXm_field32_set(unit, mtp, RTAGf, 0);

            soc_IM_MTP_INDEXm_field32_set(unit, mtp,
                                          module_field[0], modid);
            soc_IM_MTP_INDEXm_field32_set(unit, mtp,
                                          port_field[0], port);

            /* Cache trunk ID for Warm Boot */
            soc_IM_MTP_INDEXm_field32_set(unit, mtp,
                                          port_field[7], trunk);
        } else {
            /* If RTAG isn't RTAG7, then we must fill out all of the
             * trunk destination modport slots. */
            soc_IM_MTP_INDEXm_field32_set(unit, mtp, COUNTf,
                                          (member_count - 1));
            soc_IM_MTP_INDEXm_field32_set(unit, mtp, RTAGf, rtag);
#ifdef BCM_METROLITE_SUPPORT
            if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
                max_num_trunk_ports = 4;
            } else
#endif
                max_num_trunk_ports = BCM_TD_MIRROR_TRUNK_MAX_PORTCNT;
            for (idx = 0; idx < max_num_trunk_ports; idx++) {
                /* trunk variable will not be used from this point,
                 * OK to update */
                if ((7 == rtag) && (idx == member_count)) {
                    break;
                }
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_gport_resolve(unit,
                                            trunk_arr[idx % member_count],
                                            &modid, &port, &trunk, &id));
#if defined(BCM_HGPROXY_COE_SUPPORT)
                if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
                    BCM_GPORT_IS_SET(trunk_arr[idx % member_count]) &&
                    _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT (unit,
                        trunk_arr[idx % member_count])) {
                } else
#endif
#if defined(BCM_KATANA2_SUPPORT)
                if (BCM_GPORT_IS_SET(trunk_arr[idx % member_count]) &&
                    _BCM_KT2_GPORT_IS_LINKPHY_OR_SUBTAG_SUBPORT_GPORT (unit,
                        trunk_arr[idx % member_count])) {
                } else
#endif
                {
                    if ((-1 != id) || (BCM_TRUNK_INVALID != trunk)) {
                        return BCM_E_PARAM;
                    }
                }

                soc_IM_MTP_INDEXm_field32_set(unit, mtp,
                                              module_field[idx], modid);
                soc_IM_MTP_INDEXm_field32_set(unit, mtp,
                                              port_field[idx], port);

            }
        }

    }
#ifdef BCM_TRIDENT2_SUPPORT
    else if (mir_dest_flag & BCM_MIRROR_DEST_ID_SHARE) {
        bcm_gport_t     mir_dest_gport_array[BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
        int             mir_dest_gport_count = 0;

        BCM_IF_ERROR_RETURN(
            _bcm_mirror_dest_mtp_gport_get(unit,
                                           mtp_cfg->dest_id,
                                           mir_dest_gport_array,
                                           &mir_dest_gport_count));
        if (mir_dest_gport_count <= 0) {
            return (BCM_E_INIT);
        }
        rtag = (MIRROR_DEST(unit, mtp_cfg->dest_id))->rtag;
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, Tf, 1);
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, COUNTf,
                                          (mir_dest_gport_count - 1));
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, RTAGf, rtag);

        max_num_trunk_ports = BCM_TD_MIRROR_TRUNK_MAX_PORTCNT;
        for (idx = 0; idx < max_num_trunk_ports; idx++) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_gport_resolve(unit,
                                        trunk_arr[idx],
                                        &modid, &port, &trunk, &id));

            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, modid, &isLocal));
            if (TRUE == isLocal) {
                _bcm_esw_stk_modmap_map(unit,BCM_STK_MODMAP_SET, modid, port,
                                        &modid, &port);
            }

            soc_IM_MTP_INDEXm_field32_set(unit, mtp, module_field[idx], modid);
            soc_IM_MTP_INDEXm_field32_set(unit, mtp, port_field[idx], port);
        }
    }
#endif /* BCM_TRIDENT2_SUPPORT */
    else {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_resolve(unit, mirror_dest,
                                    &modid, &port, &trunk, &id));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_modid_is_local(unit, modid, &isLocal));
        if (TRUE == isLocal) {
            _bcm_esw_stk_modmap_map(unit,BCM_STK_MODMAP_SET, modid, port,
                                    &modid, &port);
        }
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, Tf, 0);
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, COUNTf, 0);
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, MODULE_IDf, modid);
        soc_IM_MTP_INDEXm_field32_set(unit, mtp, PORT_NUMf, port);

#ifdef BCM_TRIDENT2PLUS_SUPPORT
        if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
            _bcm_xgs5_subport_coe_mod_port_local(unit, modid, port)) {
            int local_port = 0;
            int mod_port_index = 0;
            modport_map_subport_mirror_entry_t subport_m_entry;
            soc_mem_t mems[] = {MODPORT_MAP_SUBPORT_M0m, MODPORT_MAP_SUBPORT_M1m,
                                MODPORT_MAP_SUBPORT_M2m, MODPORT_MAP_SUBPORT_M3m};

            BCM_IF_ERROR_RETURN(
                _bcmi_coe_subport_mod_port_physical_port_get(
                unit, modid, port, &local_port));

            BCM_IF_ERROR_RETURN(
                _bcm_esw_src_mod_port_table_index_get(unit, modid, port, &mod_port_index));

            sal_memset(&subport_m_entry, 0, sizeof(subport_m_entry));
            soc_mem_field32_set(unit, mems[index], &subport_m_entry, ENABLEf, 1);
            soc_mem_field32_set(unit, mems[index], &subport_m_entry, DESTf, local_port);

            SOC_IF_ERROR_RETURN(
                soc_mem_write(unit, mems[index], MEM_BLOCK_ALL, 
                              mod_port_index, &subport_m_entry));
        }
#endif
    }

    /* HW write. based on mirrored traffic direction. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, IM_MTP_INDEXm, MEM_BLOCK_ALL, index, mtp));
    }

    /* EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, EM_MTP_INDEXm, MEM_BLOCK_ALL, index, mtp));
    }

#ifdef BCM_TRIUMPH3_SUPPORT
    /* EP_REDIRECT_EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        ep_redirect_em_mtp_index_entry_t      ep_mtp_entry;

        sal_memset(&ep_mtp_entry, 0, sizeof(ep_mtp_entry));

        soc_EP_REDIRECT_EM_MTP_INDEXm_field32_set(unit, &ep_mtp_entry, 
               Tf, soc_IM_MTP_INDEXm_field32_get(unit, mtp, Tf));
        soc_EP_REDIRECT_EM_MTP_INDEXm_field32_set(unit, &ep_mtp_entry, 
               COUNTf,soc_IM_MTP_INDEXm_field32_get(unit, mtp,COUNTf));
        soc_EP_REDIRECT_EM_MTP_INDEXm_field32_set(unit, &ep_mtp_entry, 
               RTAGf,soc_IM_MTP_INDEXm_field32_get(unit, mtp,RTAGf));
        soc_EP_REDIRECT_EM_MTP_INDEXm_field32_set(unit, &ep_mtp_entry, 
               MODULE_IDf, 
               soc_IM_MTP_INDEXm_field32_get(unit, mtp, MODULE_IDf));
        soc_EP_REDIRECT_EM_MTP_INDEXm_field32_set(unit, &ep_mtp_entry, 
               PORT_NUMf, 
               soc_IM_MTP_INDEXm_field32_get(unit, mtp, PORT_NUMf));

        for (idx = 0; idx < max_num_trunk_ports; idx++) {
            soc_EP_REDIRECT_EM_MTP_INDEXm_field32_set(unit, &ep_mtp_entry,
               module_field[idx],
               soc_IM_MTP_INDEXm_field32_get(unit, mtp, module_field[idx]));
            soc_EP_REDIRECT_EM_MTP_INDEXm_field32_set(unit, &ep_mtp_entry,
               port_field[idx],
            soc_IM_MTP_INDEXm_field32_get(unit, mtp, port_field[idx]));
        }
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, EP_REDIRECT_EM_MTP_INDEXm,
                           MEM_BLOCK_ALL, index, &ep_mtp_entry));
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIDENT2_SUPPORT
    if (mir_dest_flag & BCM_MIRROR_DEST_ID_SHARE) {
        bcm_mirror_destination_t mirror_dest;
        soc_mem_t mem;

        max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
        offset = index * max_num_trunk_ports;

        for (idx = 0; idx < max_num_trunk_ports; idx++, offset++) {
            egr_mtp = 0;
            sal_memset(&mirror_dest, 0x0, sizeof(bcm_mirror_destination_t));
            BCM_IF_ERROR_RETURN(
                _bcm_mirror_dest_mtp_get(unit,
                                         mtp_cfg->dest_id,
                                         trunk_arr[idx],
                                         &mirror_dest));

            if (flags & BCM_MIRROR_PORT_INGRESS) {
                mem = EGR_IM_MTP_INDEXm;
            } else if (flags & BCM_MIRROR_PORT_EGRESS) {
                mem = EGR_EM_MTP_INDEXm;
            } else {
                return BCM_E_CONFIG;
            }

            BCM_IF_ERROR_RETURN(
                soc_mem_read(unit, mem, MEM_BLOCK_ANY, offset, &egr_mtp));
            BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit,
                                                       trunk_arr[idx],
                                                       &modid,
                                                       &port,
                                                       &trunk,
                                                       &id));
            BCM_IF_ERROR_RETURN(
                _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, modid, port,
                                        &mod_out, &port_out));

            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp,
                                              MTP_DST_PORTf, port_out);
            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp,
                                              MTP_DST_MODIDf, mod_out);

            if (mirror_dest.flags & BCM_MIRROR_DEST_INT_PRI_SET) {
                soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp,
                                                  CHANGE_INT_PRIf, 1);
                soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp,
                                                  NEW_INT_PRIf,
                                                  mirror_dest.int_pri);
            }
            /* HW write. based on mirrored traffic direction. */
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                                   offset, &egr_mtp));
            }

            /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
            if (flags & BCM_MIRROR_PORT_EGRESS) {
#ifdef BCM_TOMAHAWK_SUPPORT
                if (SOC_INFO(unit).th_tflow_enabled == 1) {
                        BCM_IF_ERROR_RETURN
                            (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                                           offset, &egr_mtp));
                } else
#endif /* BCM_TOMAHAWK_SUPPORT */
                {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_write(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ALL,
                                       offset, &egr_mtp));
                }
            }
        }

        BCM_IF_ERROR_RETURN
            (_bcm_trx_mirror_tunnel_set(unit, index, trunk_arr, flags));
        return (BCM_E_NONE);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    if (BCM_GPORT_IS_TRUNK(mirror_dest) && (0 == member_count)) {
        /* Traffic shouldn't proceed, don't configure other elements */
        return BCM_E_NONE;
    }

    replace_flag = (MIRROR_DEST(unit, mtp_cfg->dest_id))->flags;
    encap_index = index;

#ifdef BCM_METROLITE_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
        max_num_trunk_ports = 4;
    } else
#endif
    max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;

    offset = index * max_num_trunk_ports;
    for (idx = 0; idx < max_num_trunk_ports; idx++, offset++) {
        egr_mtp = 0;
        BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(
                            unit, trunk_arr[idx],&modid,&port,&trunk, &id));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, modid, port, 
                                    &mod_out, &port_out));
         
        soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp, 
                                          MTP_DST_PORTf, port_out);
        soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp, 
                                          MTP_DST_MODIDf, mod_out);

        if ((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
            & (BCM_MIRROR_DEST_TUNNELS)) {
            if (replace_flag & BCM_MIRROR_DEST_REPLACE) {
                soc_mem_t mem;

                if (flags & BCM_MIRROR_PORT_INGRESS) {
                    mem = EGR_IM_MTP_INDEXm;
                } else if (flags & BCM_MIRROR_PORT_EGRESS) {
                    mem = EGR_EM_MTP_INDEXm;
                } else if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
                    mem = EGR_EP_REDIRECT_EM_MTP_INDEXm;
                } else {
                    return BCM_E_CONFIG;
                }

                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, mem, MEM_BLOCK_ANY, offset,
                                             &egr_mtp_index_entry));
                if (1 == soc_mem_field32_get(unit, mem,
                                                 &egr_mtp_index_entry,
                                                 MIRROR_ENCAP_ENABLEf)) {
                    encap_index = soc_mem_field32_get(unit, mem,
                                                 &egr_mtp_index_entry,
                                                 MIRROR_ENCAP_INDEXf);

                }
            }
            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp, 
                                              MIRROR_ENCAP_INDEXf, encap_index);
        }

#ifdef BCM_TOMAHAWK_SUPPORT
        if ((SOC_INFO(unit).th_tflow_enabled == 1)
            && (flags & BCM_MIRROR_PORT_EGRESS)) {
            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp,
                                        MIRROR_ENCAP_INDEXf, encap_index);
        }
#endif /* BCM_TOMAHAWK_SUPPORT */

        if ((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
                & (BCM_MIRROR_DEST_INT_PRI_SET)) {
            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp, 
                    CHANGE_INT_PRIf, 1);         
            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &egr_mtp, 
                    NEW_INT_PRIf, 
                    (MIRROR_DEST(unit, mtp_cfg->dest_id))->int_pri);         
        } 
        /* HW write. based on mirrored traffic direction. */
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                               offset, &egr_mtp));
        }

        /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
        if (flags & BCM_MIRROR_PORT_EGRESS) {
#ifdef BCM_TOMAHAWK_SUPPORT
            if (SOC_INFO(unit).th_tflow_enabled == 1) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                                       offset, &egr_mtp));
            } else
#endif /* BCM_TOMAHAWK_SUPPORT */
            {
                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ALL,
                                   offset, &egr_mtp));
            }
        }

        /* EGR_EP_REDIRECT_EM_MTP_INDEX has same layout as
         * EGR_IM_MTP_INDEX */
        if (soc_feature(unit, soc_feature_egr_mirror_true) &&
            (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                               MEM_BLOCK_ALL, offset, &egr_mtp));
        }
    }

    if((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
        & (BCM_MIRROR_DEST_TUNNELS)) {
        BCM_IF_ERROR_RETURN
            (_bcm_trx_mirror_tunnel_set(unit, index, trunk_arr, flags));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_td_mtp_reset
 * Purpose:
 *	   Reset mirror target port for TRIDENT devices. 
 * Parameters:
 *	   unit       - (IN)BCM device number
 *     index      - (IN)Mtp index.
 *     flags      - (IN)Filled entry flags(BCM_MIRROR_PORT_INGRESS/EGRESS
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_td_mtp_reset(int unit, int index, int flags)
{
    int                         offset;
    int                         idx, encap_present;
    uint32                      mtp[SOC_MAX_MEM_BYTES/4];
    uint32                      mirror_select, mtp_type, encap_index = 0;
#ifdef BCM_TRIDENT2_SUPPORT
    _bcm_mtp_config_p           mtp_cfg; /* MTP configuration.  */
    bcm_mirror_destination_t    *mirror_dest; /* Destination & Encapsulation.*/
    int                         profile_ref_count = 0;
#endif /* BCM_TRIDENT2_SUPPORT */
    egr_im_mtp_index_entry_t    entry;
    int max_num_trunk_ports = 0;
    sal_memset(mtp, 0, sizeof(mtp));

#ifdef BCM_TRIDENT2_SUPPORT
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);
    mirror_dest = MIRROR_DEST(unit, mtp_cfg->dest_id);
#endif /* BCM_TRIDENT2_SUPPORT */

    /* HW write. based on mirrored traffic direction. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, IM_MTP_INDEXm, MEM_BLOCK_ALL, index, mtp));
    }

    /* EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, EM_MTP_INDEXm, MEM_BLOCK_ALL, index, mtp));
    }

#ifdef BCM_TRIUMPH3_SUPPORT
    /* EP_REDIRECT_EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, EP_REDIRECT_EM_MTP_INDEXm,
                           MEM_BLOCK_ALL, index, &mtp));
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Reset MTP_SELECT register to 0 */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &mirror_select));
        mtp_type = soc_reg_field_get(unit, MIRROR_SELECTr,
                                     mirror_select, MTP_TYPEf);
        mtp_type &= ~(1 << index);
        soc_reg_field_set(unit, MIRROR_SELECTr, &mirror_select,
                          MTP_TYPEf, mtp_type);
        BCM_IF_ERROR_RETURN(WRITE_MIRROR_SELECTr(unit, mirror_select));
    }

#ifdef BCM_TRIDENT2_SUPPORT
    if (mirror_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
        max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;
        offset = index * max_num_trunk_ports;
        sal_memset(mtp, 0, sizeof(mtp));

        for (idx = 0; idx < max_num_trunk_ports; idx++, offset++) {
            /* HW write. based on mirrored traffic direction. */
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ANY,
                                  offset, &entry));
                if (soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                        &entry, MIRROR_ENCAP_ENABLEf)) {
                    encap_index =
                        soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                            &entry, MIRROR_ENCAP_INDEXf);
                }

                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                                   offset, mtp));
            }

            /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
            if (flags & BCM_MIRROR_PORT_EGRESS) {
#ifdef BCM_TOMAHAWK_SUPPORT
                if (SOC_INFO(unit).th_tflow_enabled == 1) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_read(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ANY,
                                      offset, &entry));
                    if (soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                            &entry, MIRROR_ENCAP_ENABLEf)) {
                        encap_index =
                            soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                                &entry, MIRROR_ENCAP_INDEXf);
                    }
                } else
#endif /* BCM_TOMAHAWK_SUPPORT */
                {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_read(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ANY,
                                      offset, &entry));
                    if (soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                            &entry, MIRROR_ENCAP_ENABLEf)) {
                        encap_index =
                            soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                                &entry, MIRROR_ENCAP_INDEXf);
                    }
                }

#ifdef BCM_TOMAHAWK_SUPPORT
                if (SOC_INFO(unit).th_tflow_enabled == 1) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                                       offset, mtp));
                } else
#endif /* BCM_TOMAHAWK_SUPPORT */
                {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_write(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ALL,
                                       offset, mtp));
                }
            }

            BCM_IF_ERROR_RETURN
                (_bcm_egr_mirror_encap_entry_ref_get(unit,
                                                     encap_index,
                                                     &profile_ref_count));
            if (profile_ref_count != 0) {
                BCM_IF_ERROR_RETURN
                    (_bcm_egr_mirror_encap_entry_delete(unit, encap_index));
            }
        }
        return (BCM_E_NONE);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    encap_present = FALSE;
#ifdef BCM_METROLITE_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_four_port_trunk)) {
        max_num_trunk_ports = 4;
    } else
#endif
        max_num_trunk_ports = BCM_SWITCH_TRUNK_MAX_PORTCNT;

    offset = index * max_num_trunk_ports;
    sal_memset(mtp, 0, sizeof(mtp));

    for (idx = 0; idx < max_num_trunk_ports; idx++, offset++) {
        /* HW write. based on mirrored traffic direction. */
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            if ((0 == idx) && !encap_present) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ANY, 
                                                 offset, &entry));
                if (soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm, 
                                        &entry, MIRROR_ENCAP_ENABLEf)) {
                    encap_index =
                        soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm, 
                                            &entry, MIRROR_ENCAP_INDEXf);
                    encap_present = TRUE;
                }
            }

            BCM_IF_ERROR_RETURN 
                (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL, 
                               offset, mtp));
        }

        /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
        if (flags & BCM_MIRROR_PORT_EGRESS) {
            if ((0 == idx) && !encap_present) {
#ifdef BCM_TOMAHAWK_SUPPORT
                if (SOC_INFO(unit).th_tflow_enabled == 1) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_read(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ANY,
                                      offset, &entry));
                    if (soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                            &entry, MIRROR_ENCAP_ENABLEf)) {
                        encap_index =
                            soc_mem_field32_get(unit, EGR_IM_MTP_INDEXm,
                                    &entry, MIRROR_ENCAP_INDEXf);
                        encap_present = TRUE;
                    }
                } else
#endif /* BCM_TOMAHAWK_SUPPORT */
                {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_read(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ANY,
                                      offset, &entry));
                    if (soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                            &entry, MIRROR_ENCAP_ENABLEf)) {
                        encap_index =
                            soc_mem_field32_get(unit, EGR_EM_MTP_INDEXm,
                                    &entry, MIRROR_ENCAP_INDEXf);
                        encap_present = TRUE;
                    }
                }
            }

#ifdef BCM_TOMAHAWK_SUPPORT
            if (SOC_INFO(unit).th_tflow_enabled == 1) {
                BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                               offset, mtp));
            } else
#endif /* BCM_TOMAHAWK_SUPPORT */
            {
                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ALL,
                                   offset, mtp));
            }
        }

        /* EP_REDIRECT_EM_MTP_INDEX has same layout as IM_MTP_INDEX */
        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            if ((0 == idx) && !encap_present) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                                  MEM_BLOCK_ANY, offset, &entry));
                if (soc_mem_field32_get(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm, 
                                        &entry, MIRROR_ENCAP_ENABLEf)) {
                    encap_index =
                        soc_mem_field32_get(unit,
                                            EGR_EP_REDIRECT_EM_MTP_INDEXm, 
                                            &entry, MIRROR_ENCAP_INDEXf); 
                    encap_present = TRUE;
                }
            }
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                               MEM_BLOCK_ALL, offset, mtp));
        }
    }

    /* At most one of the direction flags is true, and the whole
     * set of trunk block copies are identical.  We need only
     * one delete of our reference to the profile table.
     */
    if (encap_present) {
        BCM_IF_ERROR_RETURN
            (_bcm_egr_mirror_encap_entry_delete(unit, encap_index));
    }

    return (BCM_E_NONE);
}

#endif /* BCM_TRIDENT_SUPPORT */


#if defined(BCM_FIREBOLT_SUPPORT) 
/*
 * Function:
 *	    _bcm_fbx_mtp_init
 * Purpose:
 *	   Initialize mirror target port for FBX devices. 
 * Parameters:
 *	   unit       - (IN)BCM device number
 *     index      - (IN)Mtp index.
 *     trunk_arr  - (IN)Trunk members array. 
 *     flags      - (IN)Filled entry flags(BCM_MIRROR_PORT_INGRESS/EGRESS
 *                    or both. In case both flags are specied
 *                    ingress & egress configuration is assumed to be
 *                    idential.
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_fbx_mtp_init(int unit, int index, bcm_gport_t *trunk_arr, int flags)
{
    bcm_gport_t         mirror_dest;
    bcm_port_t          port_out;
    bcm_module_t        mod_out;
    _bcm_mtp_config_p   mtp_cfg;
    int                 offset;
    int                 idx, id;
    bcm_trunk_t         trunk = BCM_TRUNK_INVALID;
    bcm_module_t        modid = 0;
    bcm_port_t          port = -1;
    uint32              mtp = 0;
    int                 isLocal;
    int                 member_count = 0;
#ifdef BCM_GREYHOUND_SUPPORT
    int                 encap_index;
    uint32              replace_flag = 0;
    egr_im_mtp_index_entry_t  egr_mtp_index_entry;
#endif /* BCM_GREYHOUND_SUPPORT */

    /* Input parameters check */ 
    if (NULL == trunk_arr) {
        return (BCM_E_PARAM);
    }

    /* Get mtp configuration structure by direction & index. */
    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

    /* Parse destination trunk / port & module. */
    mirror_dest = MIRROR_DEST_GPORT(unit, mtp_cfg->dest_id);
    if (BCM_GPORT_IS_TRUNK(mirror_dest)) {
        trunk = BCM_GPORT_TRUNK_GET(mirror_dest);
        /* Get active trunk members*/
        BCM_IF_ERROR_RETURN
                    (_bcm_trunk_id_validate(unit, trunk));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_trunk_active_member_get(unit, trunk, NULL,
                                              0, NULL, &member_count));
    } else {
        /* If MODPORT GPORT provided resolve already had happened */
        if (BCM_GPORT_IS_MODPORT(mirror_dest)) {
        modid = BCM_GPORT_MODPORT_MODID_GET(mirror_dest);
        port  = BCM_GPORT_MODPORT_PORT_GET(mirror_dest);
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, mirror_dest, &modid,  
                                       &port, &trunk, &id));
            if (BCM_TRUNK_INVALID != trunk || id != -1) {
                return BCM_E_PORT;
            }
        }
        BCM_IF_ERROR_RETURN(
            _bcm_esw_modid_is_local(unit, modid, &isLocal));
        if (TRUE == isLocal) {
            BCM_IF_ERROR_RETURN(
            _bcm_esw_stk_modmap_map(unit,BCM_STK_MODMAP_SET, modid, port,
                                        &modid, &port));
        }
    }

    /* Hw buffer preparation. */
    if (soc_feature(unit, soc_feature_trunk_group_overlay)) {
        if (BCM_GPORT_IS_TRUNK(mirror_dest)) {
            soc_IM_MTP_INDEXm_field32_set(unit, &mtp, Tf, 1);
            soc_IM_MTP_INDEXm_field32_set(unit, &mtp, TGIDf, trunk);
        } else {
            soc_IM_MTP_INDEXm_field32_set(unit, &mtp, MODULE_IDf, modid);
            soc_IM_MTP_INDEXm_field32_set(unit, &mtp, PORT_NUMf, port);
        }
    } else {
        if (BCM_GPORT_IS_TRUNK(mirror_dest)) {
            modid = BCM_TRUNK_TO_MODIDf(unit, trunk);
            port  = BCM_TRUNK_TO_TGIDf(unit, trunk);
        }
        soc_IM_MTP_INDEXm_field32_set(unit, &mtp, MODULE_IDf, modid);
        soc_IM_MTP_INDEXm_field32_set(unit, &mtp, PORT_TGIDf, port);
    }

    /* HW write. based on mirrored traffic direction. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, IM_MTP_INDEXm, MEM_BLOCK_ALL, index, &mtp));
    }

    /* EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, EM_MTP_INDEXm, MEM_BLOCK_ALL, index, &mtp));
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    /* EP_REDIRECT_EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, EP_REDIRECT_EM_MTP_INDEXm,
                           MEM_BLOCK_ALL, index, &mtp));
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (BCM_GPORT_IS_TRUNK(mirror_dest) && (0 == member_count)) {
        /* Traffic shouldn't proceed, don't configure other elements */
        return BCM_E_NONE;
    }

#ifdef BCM_GREYHOUND_SUPPORT
    replace_flag = (MIRROR_DEST(unit, mtp_cfg->dest_id))->flags;
    encap_index = index;
#endif /* BCM_GREYHOUND_SUPPORT */

    offset = index * BCM_SWITCH_TRUNK_MAX_PORTCNT;
    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT; idx++, offset++) {
        mtp = 0;
        if (BCM_GPORT_IS_MODPORT(trunk_arr[idx])) {
            modid = BCM_GPORT_MODPORT_MODID_GET(trunk_arr[idx]);
            port = BCM_GPORT_MODPORT_PORT_GET(trunk_arr[idx]);
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, trunk_arr[idx], &modid,  
                                       &port, &trunk, &id));
            if (BCM_TRUNK_INVALID != trunk || id != -1) {
                return BCM_E_PORT;
            }
        }
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, modid, port, 
                                    &mod_out, &port_out));
         
        soc_EGR_IM_MTP_INDEXm_field32_set(unit, &mtp, MTP_DST_PORTf, port_out);
        soc_EGR_IM_MTP_INDEXm_field32_set(unit, &mtp, MTP_DST_MODIDf, mod_out);

#ifdef BCM_GREYHOUND_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_encap_profile) &&
            (MIRROR_DEST(unit, mtp_cfg->dest_id))->flags
            & (BCM_MIRROR_DEST_TUNNELS)) {
            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &mtp, 
                                              MIRROR_ENCAP_ENABLEf, 1);         
            if (replace_flag & BCM_MIRROR_DEST_REPLACE) {
                soc_mem_t mem;

                if (flags & BCM_MIRROR_PORT_INGRESS) {
                    mem = EGR_IM_MTP_INDEXm;
                } else if (flags & BCM_MIRROR_PORT_EGRESS) {
                    mem = EGR_EM_MTP_INDEXm;
                } else {
                    return BCM_E_CONFIG;
                }

                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, mem, MEM_BLOCK_ANY, offset,
                                             &egr_mtp_index_entry));
                if (1 == soc_mem_field32_get(unit, mem,
                                                 &egr_mtp_index_entry,
                                                 MIRROR_ENCAP_ENABLEf)) {
                    encap_index = soc_mem_field32_get(unit, mem,
                                                 &egr_mtp_index_entry,
                                                 MIRROR_ENCAP_INDEXf);

                }
            }
            soc_EGR_IM_MTP_INDEXm_field32_set(unit, &mtp, 
                                              MIRROR_ENCAP_INDEXf, encap_index);
        }
#endif /* BCM_GREYHOUND_SUPPORT */

        /* HW write. based on mirrored traffic direction. */
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            BCM_IF_ERROR_RETURN 
                (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                               offset, &mtp));
        }

        /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
        if (flags & BCM_MIRROR_PORT_EGRESS) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ALL,
                               offset, &mtp));
        }

#ifdef BCM_TRIUMPH2_SUPPORT
        /* EGR_EP_REDIRECT_EM_MTP_INDEX has same layout as others */
        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                               MEM_BLOCK_ALL, offset, &mtp));
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    }

#if defined(BCM_TRX_SUPPORT)
    if((MIRROR_DEST(unit, mtp_cfg->dest_id))->flags) {
        BCM_IF_ERROR_RETURN(_bcm_trx_mirror_tunnel_set(unit, index, 
                                                       trunk_arr, flags));
    }
#endif /* BCM_TRX_SUPPORT */

    return (BCM_E_NONE);
}

/*
 * Function:
 *	    _bcm_fbx_mtp_reset
 * Purpose:
 *	   Reset mirror target port for FBX devices. 
 * Parameters:
 *	   unit       - (IN)BCM device number
 *     index      - (IN)Mtp index.
 *     flags      - (IN)Filled entry flags(BCM_MIRROR_PORT_INGRESS/EGRESS
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_fbx_mtp_reset(int unit, int index, int flags)
{
    int             offset;
    int             idx;
    uint32          mtp = 0;
    uint32          mirror_select, mtp_type;

    /* HW write. based on mirrored traffic direction. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, IM_MTP_INDEXm, MEM_BLOCK_ALL, index, &mtp));
    }

    /* EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, EM_MTP_INDEXm, MEM_BLOCK_ALL, index, &mtp));
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    /* EP_REDIRECT_EM_MTP_INDEX has same layout as IM_MTP_INDEX */
    if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, EP_REDIRECT_EM_MTP_INDEXm,
                           MEM_BLOCK_ALL, index, &mtp));
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Reset MTP_SELECT register to 0 if exists */
    if (SOC_REG_FIELD_VALID(unit, MIRROR_SELECTr, MTP_TYPEf)) {
        BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &mirror_select));
        mtp_type = soc_reg_field_get(unit, MIRROR_SELECTr, 
                                     mirror_select, MTP_TYPEf);
        mtp_type &= ~(1 << index);
        soc_reg_field_set(unit, MIRROR_SELECTr, &mirror_select, 
                          MTP_TYPEf, mtp_type);
        BCM_IF_ERROR_RETURN(WRITE_MIRROR_SELECTr( unit, mirror_select));
    }


#ifdef BCM_TRX_SUPPORT
        if (SOC_MEM_IS_VALID(unit, EGR_ERSPANm)) {
            egr_erspan_entry_t      hw_buf;
            int                     egr_idx;

            /* Reset hw buffer. */
            sal_memset(&hw_buf, 0, sizeof(egr_erspan_entry_t));
            
            /* Get egr_erspan index by direction */
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                egr_idx = index;
            } else if (flags & BCM_MIRROR_PORT_EGRESS) {
                egr_idx = index + 4;
            } else { /* True Egress */
                egr_idx = index + 8;
            }
            BCM_IF_ERROR_RETURN(
               soc_mem_write(unit, EGR_ERSPANm, MEM_BLOCK_ALL,
                             egr_idx, &hw_buf));
        }
#endif /* BCM_TRX_SUPPORT */

    offset = index * BCM_SWITCH_TRUNK_MAX_PORTCNT;
    for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT; idx++, offset++) {
        mtp = 0;

        /* HW write. based on mirrored traffic direction. */
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            BCM_IF_ERROR_RETURN 
                (soc_mem_write(unit, EGR_IM_MTP_INDEXm, MEM_BLOCK_ALL,
                               offset, &mtp));
        }

        /* EGR_EM_MTP_INDEX has same layout as EGR_IM_MTP_INDEX */
        if (flags & BCM_MIRROR_PORT_EGRESS) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_EM_MTP_INDEXm, MEM_BLOCK_ALL,
                               offset, &mtp));
        }

#ifdef BCM_TRIUMPH2_SUPPORT
        /* EGR_EP_REDIRECT_EM_MTP_INDEX has same layout as others */
        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                               MEM_BLOCK_ALL, offset, &mtp));
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    }
    return (BCM_E_NONE);
}
#endif /* BCM_FIREBOLT_SUPPORT */

/*
 * Function:
 *	    _bcm_xgs3_mtp_reset
 * Purpose:
 *	   Reset mirror target port for XGS3 devices. 
 * Parameters:
 *	   unit     - (IN)BCM device number
 *     index    - (IN)Mtp index.
 *     flags    - (IN)Filled entry flags(BCM_MIRROR_PORT_INGRESS/EGRESS
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mtp_reset(int unit, int index, int flags)
{
    int rv = BCM_E_UNAVAIL;      /* Operation return status. */
#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        return _bcm_td_mtp_reset(unit, index, flags);
    }
#endif
#if defined(BCM_FIREBOLT_SUPPORT) 
    if (SOC_IS_FBX(unit)) {
       rv = _bcm_fbx_mtp_reset(unit, index, flags);
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    return rv;
}


/*
 * Function:
 *	    _bcm_xgs3_mtp_init
 * Purpose:
 *	   Initialize mirror target port for XGS3 devices. 
 * Parameters:
 *	   unit     - (IN)BCM device number
 *     index    - (IN)Mtp index.
 *     flags    - (IN)Filled entry flags(BCM_MIRROR_PORT_INGRESS/EGRESS
 *                    or both. In case both flags are specied
 *                    ingress & egress configuration is assumed to be
 *                    idential.
 * Returns:
 *	   BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mtp_init(int unit, int index, int flags)
{
    _bcm_mtp_config_p    mtp_cfg;
                         /* Initialized to 0's to turn off false coverity alarm */
    bcm_gport_t          gport[BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0}; 
    bcm_gport_t          mirror_dest;
    int                  idx;
    int                  rv = BCM_E_UNAVAIL;
    int                  active_member_count = 0;
    bcm_trunk_member_t   active_member_array[BCM_SWITCH_TRUNK_MAX_PORTCNT];
#ifdef BCM_TRIDENT2_SUPPORT
    int                  mir_dest_flag = 0;
#endif /* BCM_TRIDENT2_SUPPORT */

    mtp_cfg = MIRROR_CONFIG_MTP(unit, index, flags);

    /* Destination port/trunk id validation. */
    mirror_dest = MIRROR_DEST_GPORT(unit, mtp_cfg->dest_id);
#ifdef BCM_TRIDENT2_SUPPORT
    mir_dest_flag = (MIRROR_DEST(unit, mtp_cfg->dest_id))->flags;
#endif /* BCM_TRIDENT2_SUPPORT */
    if (BCM_GPORT_IS_TRUNK(mirror_dest)) {
        rv = _bcm_trunk_id_validate(unit, BCM_GPORT_TRUNK_GET(mirror_dest));
        if (BCM_FAILURE(rv)) {
            return (BCM_E_PORT);
        }

        /* get only active trunk members and the count */
        rv = _bcm_esw_trunk_active_member_get(unit, 
                                              BCM_GPORT_TRUNK_GET(mirror_dest),
                                              NULL,
                                              BCM_SWITCH_TRUNK_MAX_PORTCNT,
                                              active_member_array, 
                                              &active_member_count);
        if (BCM_FAILURE(rv)) {
            return (BCM_E_PORT);
        }

        if (0 < active_member_count) {
            /* Fill gport array with trunk member ports. */
            for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT; idx++) {
                 gport[idx] = 
                       active_member_array[idx % active_member_count].gport;
            }
        } /* else must pass along the trunk ID for zero member trunks */
    }
#ifdef BCM_TRIDENT2_SUPPORT
    else if(mir_dest_flag & BCM_MIRROR_DEST_ID_SHARE) {
        bcm_gport_t     mir_dest_gport_array[BCM_SWITCH_TRUNK_MAX_PORTCNT] = {0};
        int             mir_dest_gport_count = 0;
        bcm_module_t    modid;
        bcm_port_t      port;
        bcm_trunk_t     tgid;
        int             id;

        BCM_IF_ERROR_RETURN(
            _bcm_mirror_dest_mtp_gport_get(unit,
                                           mtp_cfg->dest_id,
                                           mir_dest_gport_array,
                                           &mir_dest_gport_count));
        if (mir_dest_gport_count <= 0) {
            return BCM_E_INIT;
        }

        /* Check ports are valid */
        for (idx = 0; idx < mir_dest_gport_count; idx++) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, mir_dest_gport_array[idx],
                                       &modid, &port, &tgid,  &id));

            if ((-1 != id) || (BCM_TRUNK_INVALID != tgid)) {
                return BCM_E_PORT;
            }
            if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
                return (BCM_E_BADID);
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port)) {
                return (BCM_E_PORT);
            }
        }

        /* Fill gport array with destination port */
        for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT; idx++) {
             gport[idx] =
                   mir_dest_gport_array[idx % mir_dest_gport_count];
        }
    }
#endif /* BCM_TRIDENT2_SUPPORT */
    else {
        bcm_module_t    modid;
        bcm_port_t      port;
        bcm_trunk_t     tgid;
        int             id;

        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, mirror_dest,  &modid,  
                                   &port, &tgid,  &id));

#if defined(BCM_HGPROXY_COE_SUPPORT)
        if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
            BCM_GPORT_IS_SET(mirror_dest) &&
            _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT (unit,
                mirror_dest)) {
        } else
#endif

#if defined(BCM_KATANA2_SUPPORT)
        if (BCM_GPORT_IS_SET(mirror_dest) &&
            _BCM_KT2_GPORT_IS_LINKPHY_OR_SUBTAG_SUBPORT_GPORT (unit,
                mirror_dest)) {
        } else
#endif
        {
            if ((-1 != id) || (BCM_TRUNK_INVALID != tgid)) {
                return BCM_E_PORT;
            }
        }

        if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
            return (BCM_E_BADID);
        }
        if (!SOC_PORT_ADDRESSABLE(unit, port)) {
            return (BCM_E_PORT);
        }
        /* Fill gport array with destination port only. */
        for (idx = 0; idx < BCM_SWITCH_TRUNK_MAX_PORTCNT; idx++) {
            gport[idx] = mirror_dest;
        }
    }
#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        return _bcm_trident_mtp_init(unit, index, gport, flags);
    } 
#endif
#if defined(BCM_FIREBOLT_SUPPORT) 
    if (SOC_IS_FBX(unit)) {
       rv = _bcm_fbx_mtp_init(unit, index, gport, flags);
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    return rv;
}

#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_TRIDENT_SUPPORT)
/*
 * Function:
 *      _bcm_tr2_mirror_trunk_update
 * Description:
 *      Update mtp programming based on trunk port membership.
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      tid        - (IN)  Trunk id. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_trunk_update(int unit, bcm_trunk_t tid)
{
    int idx;                        /* Mtp iteration index.     */
    bcm_gport_t  gport;             /* Mirror destination.      */
    bcm_gport_t  mirror_dest_id;    /* Mirror destination.      */
    int rv = BCM_E_NONE;            /* Operation return status. */
    int egress;                     /* Mirror ingress/egres     */

    /* Initilize mirror destination. */
    BCM_GPORT_TRUNK_SET(gport, tid);

    MIRROR_LOCK(unit);
    /* Ingress mirroring destions update */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
            if (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)) { 
                mirror_dest_id = MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx);
                egress = MIRROR_CONFIG_SHARED_MTP(unit, idx).egress;
                if (MIRROR_DEST_GPORT(unit, mirror_dest_id) == gport) {
                    rv = _bcm_xgs3_mtp_init(unit, idx, (TRUE == egress) ? 
                                            BCM_MIRROR_PORT_EGRESS : 
                                            BCM_MIRROR_PORT_INGRESS);
                    if (BCM_FAILURE(rv)) {
                        break;
                    }
                }
            }
        }
    } else {
        /* Check all used ingress mirror destinations. */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
            if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)) {
                mirror_dest_id = MIRROR_CONFIG_ING_MTP_DEST(unit, idx);
                if (MIRROR_DEST_GPORT(unit, mirror_dest_id) == gport) {
                    rv = _bcm_xgs3_mtp_init(unit, idx,
                                            BCM_MIRROR_PORT_INGRESS);
                    if (BCM_FAILURE(rv)) {
                        break;
                    }
                }
            }
        }

        if (BCM_SUCCESS(rv)) {
            /* Check all used egress mirror destinations. */
            for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
                if (MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)) {
                    mirror_dest_id = MIRROR_CONFIG_EGR_MTP_DEST(unit, idx);
                    if (MIRROR_DEST_GPORT(unit, mirror_dest_id) == gport) {
                        rv = _bcm_xgs3_mtp_init(unit, idx,
                                                BCM_MIRROR_PORT_EGRESS);
                        if (BCM_FAILURE(rv)) {
                            break;
                        }
                    }
                }
            }
        }
    }

    /* True egress mirroring destinations update */
    if (BCM_SUCCESS(rv) &&
        soc_feature(unit, soc_feature_egr_mirror_true)) {
        for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
            if (MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)) { 
                mirror_dest_id = MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx);
                if (MIRROR_DEST_GPORT(unit, mirror_dest_id) == gport) {
                    rv = _bcm_xgs3_mtp_init(unit, idx,
                                            BCM_MIRROR_PORT_EGRESS_TRUE);
                    if (BCM_FAILURE(rv)) {
                        break;
                    }
                }
            }
        }
    }

    MIRROR_UNLOCK(unit);
    return (rv);
}
#endif /* BCM_TRIUMPH2_SUPPORT || BCM_TRIDENT_SUPPORT */

/*
 * Function:
 *      _bcm_xgs3_mirror_trunk_update
 * Description:
 *      Update mtp programming based on trunk port membership.
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      tid        - (IN)  Trunk id. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_mirror_trunk_update(int unit, bcm_trunk_t tid)
{
    int idx;                        /* Mtp iteration index.     */
    bcm_gport_t  gport;             /* Mirror destination.      */
    bcm_gport_t  mirror_dest_id;    /* Mirror destination.      */
    int rv = BCM_E_NONE;            /* Operation return status. */

    /* Check if mirroring enabled on the device. */
    if (!MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }
#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        return _bcm_tr2_mirror_trunk_update(unit, tid);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Initilize mirror destination. */
    BCM_GPORT_TRUNK_SET(gport, tid);

    MIRROR_LOCK(unit);
    /* Ingress mirroring destions update */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
        if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)) { 
            mirror_dest_id = MIRROR_CONFIG_ING_MTP_DEST(unit, idx);
            if (MIRROR_DEST_GPORT(unit, mirror_dest_id) == gport) {
                rv = _bcm_xgs3_mtp_init(unit, idx, BCM_MIRROR_PORT_INGRESS);
                if (BCM_FAILURE(rv)) {
                    MIRROR_UNLOCK(unit);
                    return (rv);
                }
            }
        }
    }

    /* Egress mirroring destinations update */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
        if (MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)) { 
            mirror_dest_id = MIRROR_CONFIG_EGR_MTP_DEST(unit, idx);
            if (MIRROR_DEST_GPORT(unit, mirror_dest_id) == gport) {
                rv = _bcm_xgs3_mtp_init(unit, idx, BCM_MIRROR_PORT_EGRESS);
                if (BCM_FAILURE(rv)) {
                    break;
                }
            }
        }
    }

    MIRROR_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      _bcm_tr2_mirror_ingress_mtp_reserve
 * Description:
 *      Reserve a mirror-to port for Triumph2 like devices
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      dest_id    - (IN)  Mirror destination id.
 *      index_used - (OUT) MTP index reserved
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the a mirror-to port is reserved more than once
 *      (without being unreserved) then the same MTP index 
 *      will be returned for each call.
 */
STATIC int
_bcm_tr2_mirror_ingress_mtp_reserve(int unit, bcm_gport_t dest_id, 
                                     int *index_used)
{
    int     rv;
    int     idx = _BCM_MIRROR_INVALID_MTP;
    int     rspan = FALSE; 
    uint32  flags = 0;

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        bcm_mirror_destination_t mdest;    /* Mirror destination info. */

        /* Get mirror destination descriptor. */
        BCM_IF_ERROR_RETURN
            (bcm_esw_mirror_destination_get(unit, dest_id, &mdest));

        rspan = (0 != (mdest.flags & BCM_MIRROR_DEST_TUNNEL_L2));
        flags = mdest.flags;
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Look for existing MTP in use */
    rv = _bcm_tr2_mirror_shared_mtp_match(unit, dest_id, FALSE, &idx);
    if (BCM_SUCCESS(rv)) {
#ifdef BCM_TRIDENT2_SUPPORT
        if (flags & BCM_MIRROR_DEST_ID_SHARE) {
            /*
             * Add MTP ref_count here and ref_count can be adjusted later.
             * No matter update or add, always refresh H/W.
             */
            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
        } else
#endif /* BCM_TRIDENT2_SUPPORT */
        {
            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
            if (!(flags & BCM_MIRROR_DEST_REPLACE)) {
                *index_used = idx;
                return (rv);
            }
        }
    }

    if (idx == _BCM_MIRROR_INVALID_MTP) {
       /* Reserve free index */
       for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
           if (0 == MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)) {
               if (rspan && (0 == idx)) {
                   /* Do not use MTP 0 for RSPAN on Trident
                    * and later devices */
                    continue;
               }
               break;
           }
       }
    }

    if (idx < BCM_MIRROR_MTP_COUNT) {
        if (BCM_FAILURE(rv)) {
           /* Mark mtp as used. */
           MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx) = dest_id;
           MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
           MIRROR_CONFIG_SHARED_MTP(unit, idx).egress = FALSE;
           MIRROR_DEST_REF_COUNT(unit, dest_id)++;
        }

        /* Write MTP registers */
        rv = _bcm_xgs3_mtp_init(unit, idx, BCM_MIRROR_PORT_INGRESS);
        if (BCM_FAILURE(rv)) {
            MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx) = BCM_GPORT_INVALID;
            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) = 0;
            if (MIRROR_DEST_REF_COUNT(unit, dest_id) > 0) {
                MIRROR_DEST_REF_COUNT(unit, dest_id)--;
            }
        } else {
            if (SOC_REG_FIELD_VALID(unit, MIRROR_SELECTr, MTP_TYPEf)) {
                uint32 rval, fval;
                BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &rval));
                fval = soc_reg_field_get(unit, MIRROR_SELECTr, rval, MTP_TYPEf);
                fval &= ~(1 << idx);
                soc_reg_field_set(unit, MIRROR_SELECTr, &rval, MTP_TYPEf, fval);
                BCM_IF_ERROR_RETURN(WRITE_MIRROR_SELECTr(unit, rval));
                if (SOC_REG_FIELD_VALID(unit, EGR_MIRROR_SELECTr, MTP_TYPEf)) {
                    BCM_IF_ERROR_RETURN(WRITE_EGR_MIRROR_SELECTr(unit, rval));
                }        
            }

        }
        *index_used = idx;
    } else {
        rv = BCM_E_RESOURCE;
    } 
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_mirror_ingress_mtp_reserve
 * Description:
 *      Reserve a mirror-to port
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      dest_id    - (IN)  Mirror destination id.
 *      index_used - (OUT) MTP index reserved
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the a mirror-to port is reserved more than once
 *      (without being unreserved) then the same MTP index 
 *      will be returned for each call.
 */
STATIC int
_bcm_xgs3_mirror_ingress_mtp_reserve(int unit, bcm_gport_t dest_id, 
                                     int *index_used)
{
    int rv;                                  /* Operation return status. */
    int idx = _BCM_MIRROR_INVALID_MTP;       /* Mtp iteration index.     */
    int rspan = FALSE; 
    uint32 flags = 0;
    bcm_mirror_destination_t mdest;    /* Mirror destination info. */
    
    /* Input parameters check. */
    if (NULL == index_used) {
        return (BCM_E_PARAM);
    }

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        return _bcm_tr2_mirror_ingress_mtp_reserve(unit, dest_id, index_used);
    }
    /* Get mirror destination descriptor. */
    BCM_IF_ERROR_RETURN
        (bcm_esw_mirror_destination_get(unit, dest_id, &mdest));
    flags = mdest.flags;

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        rspan = (0 != (mdest.flags & BCM_MIRROR_DEST_TUNNEL_L2));
    }
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

    /* Look for existing MTP in use */
    rv = _bcm_esw_mirror_ingress_mtp_match(unit, dest_id, &idx);
    if (BCM_SUCCESS(rv)) {
#ifdef BCM_TRIDENT2_SUPPORT
        if (flags & BCM_MIRROR_DEST_ID_SHARE) {
            /*
             * Add MTP ref_count here and ref_count can be adjusted later.
             * No matter update or add, always refresh H/W.
             */
            MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)++;
        } else
#endif /* BCM_TRIDENT2_SUPPORT */
        {
            MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)++;
            if (!(flags & BCM_MIRROR_DEST_REPLACE)) {
                *index_used = idx;
                return (rv);
            }
        }
    }

    if (idx == _BCM_MIRROR_INVALID_MTP) {
        /* Reserve free index */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
             if (0 == MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)) {
                 if (rspan && (0 == idx)) {
                     /* Do not use MTP 0 for RSPAN on Trident
                      * and later devices */
                      continue;
                 }
                 break;
             }
        }
    }

    if (idx < MIRROR_CONFIG(unit)->ing_mtp_count) {
        if (BCM_FAILURE(rv)) {
            /* Mark mtp as used. */
            MIRROR_CONFIG_ING_MTP_DEST(unit, idx) = dest_id;
            MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx)++;
            MIRROR_DEST_REF_COUNT(unit, dest_id)++;
        } 

        /* Write MTP registers */
        rv = _bcm_xgs3_mtp_init(unit, idx, BCM_MIRROR_PORT_INGRESS);
        if (BCM_FAILURE(rv)) {
            MIRROR_CONFIG_ING_MTP_DEST(unit, idx) = BCM_GPORT_INVALID;
            MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, idx) = 0;
            if (MIRROR_DEST_REF_COUNT(unit, dest_id) > 0) {
                MIRROR_DEST_REF_COUNT(unit, dest_id)--;
            }
        }
        *index_used = idx;
    } else {
        rv = BCM_E_RESOURCE;
    } 

    return (rv);
}

/*
 * Function:
 *      _bcm_tr2_mirror_egress_mtp_reserve
 * Description:
 *      Reserve a mirror-to port for Triumph2 like devices
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      dest_id    - (IN)  Mirror destination id.
 *      is_port    - (IN)  Reservation is for port based mirror
 *      index_used - (OUT) MTP index reserved
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the a mirror-to port is reserved more than once
 *      (without being unreserved) then the same MTP index 
 *      will be returned for each call.
 */
STATIC int
_bcm_tr2_mirror_egress_mtp_reserve(int unit, bcm_gport_t dest_id, 
                                   int is_port, int *index_used)
{
    int    port_limit = 0;
    int    rv;
    int    idx = _BCM_MIRROR_INVALID_MTP;
    int    rspan = FALSE; 
    uint32 flags = 0;

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        bcm_mirror_destination_t mdest;    /* Mirror destination info. */

        /* Get mirror destination descriptor. */
        BCM_IF_ERROR_RETURN
            (bcm_esw_mirror_destination_get(unit, dest_id, &mdest));

        rspan = (0 != (mdest.flags & BCM_MIRROR_DEST_TUNNEL_L2));
        flags = mdest.flags;
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Look for existing MTP in use */
    rv = _bcm_tr2_mirror_shared_mtp_match(unit, dest_id, TRUE, &idx);
    if (BCM_SUCCESS(rv)) {
#ifdef BCM_TRIDENT2_SUPPORT
        if (flags & BCM_MIRROR_DEST_ID_SHARE) {
            /*
             * Add MTP ref_count here and ref_count can be adjusted later.
             * No matter update or add, always refresh H/W.
             */
            if (is_port) {
                MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
            } else {
                MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) +=
                    (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
            }
        } else
#endif /* BCM_TRIDENT2_SUPPORT */
        {
            if (is_port) {
                MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
            } else {
                MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) +=
                    (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
            }
            if (!(flags & BCM_MIRROR_DEST_REPLACE)) {
                *index_used = idx;
                return (rv);
            }
        }
    }

    if (idx == _BCM_MIRROR_INVALID_MTP) { 
        /* Reserve free index */
        if (MIRROR_CONFIG(unit)->port_em_mtp_count > 1) {
            for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
               if (is_port && 
                   (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) & 
                    BCM_MIRROR_MTP_REF_PORT_MASK) &&
                   (TRUE == MIRROR_CONFIG_SHARED_MTP(unit, idx).egress)) {
                      port_limit++;
                      if (port_limit > MIRROR_CONFIG(unit)->port_em_mtp_count) {
                          return (BCM_E_RESOURCE);
                      }
               }
               if (0 == MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)) {
                   if (rspan && (0 == idx)) {
                      /* Do not use MTP 0 for RSPAN on Trident
                       * and later devices */
                       continue;
                   }
                   break;
               }
            }
        } else {
            /* Not directed mirroring mode */
            if (0 != MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit,
                BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX)) {
                return (BCM_E_RESOURCE);
            } else {
                idx = BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX;
            }
        }
    }

    if (idx < BCM_MIRROR_MTP_COUNT) {
        if (BCM_FAILURE(rv)) {
           /* Mark mtp as used. */
           MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx) = dest_id;
           if (is_port) {
               MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx)++;
           } else {
               MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) +=
                             (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
           }
           MIRROR_CONFIG_SHARED_MTP(unit, idx).egress = TRUE;
           MIRROR_DEST_REF_COUNT(unit, dest_id)++;
        }

        /* Write MTP registers */
        rv = _bcm_xgs3_mtp_init(unit, idx, BCM_MIRROR_PORT_EGRESS);
        if (BCM_FAILURE(rv)) {
            MIRROR_CONFIG_SHARED_MTP_DEST(unit, idx) = BCM_GPORT_INVALID;
            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, idx) = 0;
            if (MIRROR_DEST_REF_COUNT(unit, dest_id) > 0) {
                MIRROR_DEST_REF_COUNT(unit, dest_id)--;
            }
        } else {
            if (SOC_REG_FIELD_VALID(unit, MIRROR_SELECTr, MTP_TYPEf)) {
                uint32 rval, fval;
                BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &rval));
                fval = soc_reg_field_get(unit, MIRROR_SELECTr, rval, MTP_TYPEf);
                fval |= (1 << idx);
                soc_reg_field_set(unit, MIRROR_SELECTr, &rval, MTP_TYPEf, fval);
                BCM_IF_ERROR_RETURN(WRITE_MIRROR_SELECTr(unit, rval));
#ifdef BCM_TOMAHAWK_SUPPORT
                if (SOC_INFO(unit).th_tflow_enabled == 1) {
                    /* All MTPs in the EGR_MIRROR_SELECT control
                     * should be configured as ingress.
                     */
                    rval = 0;
                }
#endif /* BCM_TOMAHAWK_SUPPORT */
                if (SOC_REG_FIELD_VALID(unit, EGR_MIRROR_SELECTr, MTP_TYPEf)) {
                    BCM_IF_ERROR_RETURN(WRITE_EGR_MIRROR_SELECTr(unit, rval));
                }        
            }

        }
        *index_used = idx;
    } else {
        rv = BCM_E_RESOURCE;
    } 
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_mirror_egress_mtp_reserve
 * Description:
 *      Reserve a mirror-to port
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      dest_id    - (IN)  Mirror destination id.
 *      is_port    - (IN)  Reservation is for port based mirror
 *      index_used - (OUT) MTP index reserved
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the a mirror-to port is reserved more than once
 *      (without being unreserved) then the same MTP index 
 *      will be returned for each call.
 */
STATIC int
_bcm_xgs3_mirror_egress_mtp_reserve(int unit, bcm_gport_t dest_id, int is_port,
                                     int *index_used)
{
    int rv;                                  /* Operation return status.*/
    int idx = _BCM_MIRROR_INVALID_MTP;       /* Mtp iteration index.    */
    int port_limit = 0;    /* How many mtp in use by port based mirroring */
    int rspan = FALSE; 
    uint32 flags = 0;
    bcm_mirror_destination_t mdest;    /* Mirror destination info. */
    
    /* Input parameters check. */
    if (NULL == index_used) {
        return (BCM_E_PARAM);
    }

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        return _bcm_tr2_mirror_egress_mtp_reserve(unit, dest_id, is_port, 
                                                  index_used);
    }

    /* Get mirror destination descriptor. */
    BCM_IF_ERROR_RETURN
        (bcm_esw_mirror_destination_get(unit, dest_id, &mdest));
    flags = mdest.flags;
    
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        rspan = (0 != (mdest.flags & BCM_MIRROR_DEST_TUNNEL_L2));
    }
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

    /* Look for existing MTP in use */
    rv = _bcm_esw_mirror_egress_mtp_match(unit, dest_id, &idx);
    if (BCM_SUCCESS(rv)) {
#ifdef BCM_TRIDENT2_SUPPORT
        if (flags & BCM_MIRROR_DEST_ID_SHARE) {
            /*
             * Add MTP ref_count here and ref_count can be adjusted later.
             * No matter update or add, always refresh H/W.
             */
            if (is_port) {
                MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)++;
            } else {
                MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx) +=
                    (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
            }
        } else
#endif /* BCM_TRIDENT2_SUPPORT */
        {
            if (is_port) {
                MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)++;
            } else {
                MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx) +=
                    (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
            }
            if (!(flags & BCM_MIRROR_DEST_REPLACE)) {
                *index_used = idx;
                return (rv);
            }
        }
    }

    if (idx == _BCM_MIRROR_INVALID_MTP) {
       /* Reserve free index */
       for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
            if (is_port &&
               (MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx) &
                BCM_MIRROR_MTP_REF_PORT_MASK)) {
                port_limit++;
                if (port_limit > MIRROR_CONFIG(unit)->port_em_mtp_count) {
                    return (BCM_E_RESOURCE);
                }
            }
            if (0 == MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)) {
                if (rspan && (0 == idx)) {
                    /* Do not use MTP 0 for RSPAN on Trident
                     * and later devices */
                     continue;
                }
                break;
            }
       }
    }

    if (idx < MIRROR_CONFIG(unit)->egr_mtp_count) {
        if (BCM_FAILURE(rv)) {
           /* Mark mtp as used. */
           MIRROR_CONFIG_EGR_MTP_DEST(unit, idx) = dest_id;
           if (is_port) {
               MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx)++;
           } else {
               MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx) +=
                  (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
           }
           MIRROR_DEST_REF_COUNT(unit, dest_id)++;
        }

        /* Write MTP registers */
        rv = _bcm_xgs3_mtp_init(unit, idx, BCM_MIRROR_PORT_EGRESS);
        if (BCM_FAILURE(rv)) {
            MIRROR_CONFIG_EGR_MTP_DEST(unit, idx) = BCM_GPORT_INVALID;
            MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, idx) = 0;
            if (MIRROR_DEST_REF_COUNT(unit, dest_id) > 0) {
                MIRROR_DEST_REF_COUNT(unit, dest_id)--;
            }
        } 
        *index_used = idx;
    } else {
        rv = BCM_E_RESOURCE;
    } 

    return (rv);
}

#ifdef BCM_TRIUMPH2_SUPPORT
/*
 * Function:
 *      _bcm_xgs3_mirror_egress_true_mtp_reserve
 * Description:
 *      Reserve a mirror-to port
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      dest_id    - (IN)  Mirror destination id.
 *      index_used - (OUT) MTP index reserved
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the a mirror-to port is reserved more than once
 *      (without being unreserved) then the same MTP index 
 *      will be returned for each call.
 */
STATIC int
_bcm_xgs3_mirror_egress_true_mtp_reserve(int unit, bcm_gport_t dest_id,
                                     int *index_used)
{
    int rv;                                  /* Operation return status.*/
    int idx = _BCM_MIRROR_INVALID_MTP;       /* Mtp iteration index.    */
    int rspan = FALSE; 
    uint32 flags = 0;
    bcm_mirror_destination_t mdest;    /* Mirror destination info. */

    /* Input parameters check. */
    if (NULL == index_used) {
        return (BCM_E_PARAM);
    }
    
    /* Get mirror destination descriptor. */
    BCM_IF_ERROR_RETURN
        (bcm_esw_mirror_destination_get(unit, dest_id, &mdest));
    flags = mdest.flags;

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        rspan = (0 != (mdest.flags & BCM_MIRROR_DEST_TUNNEL_L2));
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Look for existing MTP in use */
    rv = _bcm_esw_mirror_egress_true_mtp_match(unit, dest_id, &idx);
    if (BCM_SUCCESS(rv)) {
        MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)++;
        if (!(flags & BCM_MIRROR_DEST_REPLACE)) {
            *index_used = idx;
            return (rv);
        }
    }

    if (idx == _BCM_MIRROR_INVALID_MTP) {
       /* Reserve free index */
       for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
           if (0 == MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)) {
               if (rspan && (0 == idx)) {
                   /* Do not use MTP 0 for RSPAN on Trident
                    * and later devices */
                   continue;
               }
               break;
           }
       }
    }

    if ((idx < MIRROR_CONFIG(unit)->egr_true_mtp_count) ||
        (SOC_IS_APOLLO(unit) &&
         soc_feature(unit, soc_feature_ipfix_flow_mirror) &&
         (MIRROR_CONFIG(unit)->egr_true_mtp_count == 0))) {
        if ((rv == BCM_E_NOT_FOUND) && (MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx) != 0)) {
           MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx) = 0;
        }

        if (BCM_FAILURE(rv)) {
           /* Mark mtp as used. */
           MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx) = dest_id;
           MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx)++;
           MIRROR_DEST_REF_COUNT(unit, dest_id)++;
        }
 
        /* Write MTP registers */
        rv = _bcm_xgs3_mtp_init(unit, idx, BCM_MIRROR_PORT_EGRESS_TRUE);
        if (BCM_FAILURE(rv)) {
            MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, idx) = BCM_GPORT_INVALID;
            MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, idx) = 0;
            if (MIRROR_DEST_REF_COUNT(unit, dest_id) > 0) {
                MIRROR_DEST_REF_COUNT(unit, dest_id)--;
            }
        }
        *index_used = idx;
    } else {
        rv = BCM_E_RESOURCE;
    } 

    return (rv);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      _bcm_tr2_mirror_ingress_mtp_unreserve
 * Description:
 *      Free ingress  mirror-to port for Triumph_2 like devices
 * Parameters:
 *      unit       - (IN) BCM device number
 *      mtp_index  - (IN) MTP index. 
 *      egress     - (IN) Ingress/Egress indication
 *      is_port    - (IN) Port based mirrorring indication
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Mtp index completely freed only when reference count gets to 0. 
 */
STATIC int
_bcm_tr2_mirror_mtp_unreserve(int unit, int mtp_index, int egress, int is_port)
{
    int rv = BCM_E_NONE;                     /* Operation return status.*/
    bcm_gport_t mirror_dest;                 /* Mirror destination id.  */

    /* Input parameters check. */
    if ((mtp_index < 0) || (mtp_index >= BCM_MIRROR_MTP_COUNT)) {
        return (BCM_E_PARAM);
    }

    /* If MTP is not in use - do nothing */
    if (0 == MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)) {
        return (rv);
    }

    /* Decrement mtp index reference count. */
    if ((MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index) > 0) &&
        (egress == MIRROR_CONFIG_SHARED_MTP(unit, mtp_index).egress)) {
        if (is_port) {
            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)--;
        } else {
            MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index) -= (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
        }
    }

    if (0 == MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)) {
        /* Write MTP registers */
        mirror_dest = MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index);
        rv = _bcm_xgs3_mtp_reset(unit, mtp_index, egress ? BCM_MIRROR_PORT_EGRESS : BCM_MIRROR_PORT_INGRESS);
        MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index) = BCM_GPORT_INVALID;
        if (MIRROR_DEST_REF_COUNT(unit, mirror_dest) > 0) {
            MIRROR_DEST_REF_COUNT(unit, mirror_dest)--;
        }
    }
    return (rv);
}


/*
 * Function:
 *      _bcm_xgs3_mirror_ingress_mtp_unreserve
 * Description:
 *      Free ingress  mirror-to port
 * Parameters:
 *      unit       - (IN) BCM device number
 *      mtp_index  - (IN) MTP index. 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Mtp index completely freed only when reference count gets to 0. 
 */
STATIC int
_bcm_xgs3_mirror_ingress_mtp_unreserve(int unit, int mtp_index)
{
    int rv = BCM_E_NONE;                     /* Operation return status.*/
    bcm_gport_t mirror_dest;                 /* Mirror destination id.  */

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        return _bcm_tr2_mirror_mtp_unreserve(unit, mtp_index, FALSE, TRUE);
    }

    /* Input parameters check. */
    if (mtp_index >= MIRROR_CONFIG(unit)->ing_mtp_count) {
        return (BCM_E_PARAM);
    }

    /* If MTP is not in use - do nothing */
    if (0 == MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, mtp_index)) {
        return (rv);
    }


    /* Decrement mtp index reference count. */
    if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, mtp_index) > 0) {
        MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, mtp_index)--;
    }

    if (0 == MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, mtp_index)) {
        /* Write MTP registers */
        mirror_dest = MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index);
        rv = _bcm_xgs3_mtp_reset(unit, mtp_index, BCM_MIRROR_PORT_INGRESS);
        MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index) = BCM_GPORT_INVALID;
        if (MIRROR_DEST_REF_COUNT(unit, mirror_dest) > 0) {
            MIRROR_DEST_REF_COUNT(unit, mirror_dest)--;
        }
    }
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_mirror_egress_mtp_unreserve
 * Description:
 *      Free egress  mirror-to port
 * Parameters:
 *      unit       - (IN) BCM device number
 *      mtp_index  - (IN) MTP index. 
 *      is_port    - (IN) Port based mirror indication.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Mtp index completely freed only when reference count gets to 0. 
 */
STATIC int
_bcm_xgs3_mirror_egress_mtp_unreserve(int unit, int mtp_index, int is_port)
{
    int rv = BCM_E_NONE;                     /* Operation return status.*/
    bcm_gport_t mirror_dest;                 /* Mirror destination id.  */

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        return _bcm_tr2_mirror_mtp_unreserve(unit, mtp_index, TRUE, is_port);
    }

    /* Input parameters check. */
    if (mtp_index >= MIRROR_CONFIG(unit)->egr_mtp_count) {
        return (BCM_E_PARAM);
    }

    if (0 == MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, mtp_index)) {
        return (rv);
    }

    /* Decrement mtp index reference count. */
    if (MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, mtp_index) > 0) {
        if (is_port) {
            MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, mtp_index)--;
        } else {
            MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, mtp_index) -= (1 << BCM_MIRROR_MTP_REF_FP_OFFSET);
        }
    }


    if (0 == MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, mtp_index)) {
        /* Write MTP registers */
        mirror_dest = MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_index);
        rv = _bcm_xgs3_mtp_reset(unit, mtp_index, BCM_MIRROR_PORT_EGRESS);
        MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_index) = BCM_GPORT_INVALID;
        if (MIRROR_DEST_REF_COUNT(unit, mirror_dest) > 0) { 
            MIRROR_DEST_REF_COUNT(unit, mirror_dest)--;
        }
    }
    return (rv);
}

#ifdef BCM_TRIUMPH2_SUPPORT
/*
 * Function:
 *      _bcm_xgs3_mirror_egress_true_mtp_unreserve
 * Description:
 *      Free egress  mirror-to port
 * Parameters:
 *      unit       - (IN) BCM device number
 *      mtp_index  - (IN) MTP index. 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Mtp index completely freed only when reference count gets to 0. 
 */
STATIC int
_bcm_xgs3_mirror_egress_true_mtp_unreserve(int unit, int mtp_index)
{
    int rv = BCM_E_NONE;                     /* Operation return status.*/
    bcm_gport_t mirror_dest;                 /* Mirror destination id.  */

    /* Input parameters check. */
    if (mtp_index >= MIRROR_CONFIG(unit)->egr_true_mtp_count) {
        if (!(SOC_IS_APOLLO(unit) &&
            (MIRROR_CONFIG(unit)->egr_true_mtp_count == 0) &&
            soc_feature(unit, soc_feature_ipfix_flow_mirror))) {
            return (BCM_E_PARAM);
        }
    }

    if (0 == MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, mtp_index)) {
        return (rv);
    }

    /* Decrement mtp index reference count. */
    if (MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, mtp_index) > 0) {
        MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, mtp_index)--;
    }


    if (0 == MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, mtp_index)) {
        /* Write MTP registers */
        mirror_dest = MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, mtp_index);
        rv = _bcm_xgs3_mtp_reset(unit, mtp_index, BCM_MIRROR_PORT_EGRESS_TRUE);
        MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, mtp_index) = BCM_GPORT_INVALID;
        if (MIRROR_DEST_REF_COUNT(unit, mirror_dest) > 0) { 
            MIRROR_DEST_REF_COUNT(unit, mirror_dest)--;
        }
    }
    return (rv);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      _bcm_xgs3_mirror_mtp_reserve 
 * Description:
 *      Reserve a mirror-to port
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      dest_port  - (IN)  Mirror destination gport.
 *      flags      - (IN)  Mirrored traffic direction. 
 *      index_used - (OUT) MTP index reserved
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the a mirror-to port is reserved more than once
 *      (without being unreserved) then the same MTP index 
 *      will be returned for each call.
 *      Direction should be INGRESS, EGRESS, or EGRESS_TRUE.
 */
STATIC int
_bcm_xgs3_mirror_mtp_reserve(int unit, bcm_gport_t gport, 
                            uint32 flags, int *index_used)
{
    /* Input parameters check. */
    if (NULL == index_used) {
        return (BCM_E_PARAM);
    }

    /* Allocate & initialize mtp based on mirroring direction. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        return _bcm_xgs3_mirror_ingress_mtp_reserve(unit, gport, index_used);
    } else if (flags & BCM_MIRROR_PORT_EGRESS) {
        return _bcm_xgs3_mirror_egress_mtp_reserve(unit, gport, TRUE, index_used);
#ifdef BCM_TRIUMPH2_SUPPORT
    } else if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        return _bcm_xgs3_mirror_egress_true_mtp_reserve(unit, gport,
                                                        index_used);
#endif /* BCM_TRIUMPH2_SUPPORT */
    } 
    return (BCM_E_PARAM);
}


/*
 * Function:
 *      _bcm_xgs3_mirror_mtp_unreserve 
 * Description:
 *      Free a mirror-to port
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      mtp_index  - (IN)  MTP index. 
 *      is_port    - (IN)  Port based mirror indication.
 *      flags      - (IN)  Mirrored traffic direction. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_mtp_unreserve(int unit, int mtp_index, int is_port, 
                               uint32 flags)
{

    /* Free & reset mtp based on mirroring direction. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        return _bcm_xgs3_mirror_ingress_mtp_unreserve(unit, mtp_index);
    } else if (flags & BCM_MIRROR_PORT_EGRESS) {
        return _bcm_xgs3_mirror_egress_mtp_unreserve(unit, mtp_index, is_port);
#ifdef BCM_TRIUMPH2_SUPPORT
    } else if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        return _bcm_xgs3_mirror_egress_true_mtp_unreserve(unit, mtp_index);
#endif /* BCM_TRIUMPH2_SUPPORT */
    } 
    return (BCM_E_PARAM);
}

#ifdef BCM_TRIUMPH2_SUPPORT

/*
 * Function:
 *     _bcm_xgs3_mtp_slot_port_indexes_get
 * Purpose:
 *      Retrieve port MTP indexes in MTP slots
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Local mirror port
 *      mtp_indexes  -  (OUT) MTP index in MTP slots for the given port
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_mtp_slot_port_indexes_get(int unit, bcm_port_t port,
                                    uint32 mtp_indexes[BCM_MIRROR_MTP_COUNT])
{
    uint32 mc_val;
    mirror_control_entry_t mc_entry;
    int mtp_slot;
    int vp = VP_PORT_INVALID;
    bcm_module_t mod_out;
    bcm_trunk_t tgid_out;
    int  num_local_ports = 0;
    
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &mod_out,
                                   &port, &tgid_out, &vp));
        
        if (BCM_TRUNK_INVALID != tgid_out) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_trunk_local_members_get(unit, tgid_out, 1,
                                                 &port, &num_local_ports));
        }
    }

    /* Read mirror control structure to get programmed mtp indexes. */
    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        BCM_IF_ERROR_RETURN
            (READ_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY, port, &mc_entry));
        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            mtp_indexes[mtp_slot] =
                soc_mem_field32_get(unit, MIRROR_CONTROLm, &mc_entry,
                                    _mtp_index_field[mtp_slot]);
        }
    } else {
        BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &mc_val));
        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            mtp_indexes[mtp_slot] =
                soc_reg_field_get(unit, MIRROR_CONTROLr, mc_val,
                                  _mtp_index_field[mtp_slot]);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_xgs3_mtp_slot_port_index_set
 * Purpose:
 *      Set a port's MTP index in the given MTP slot
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Local mirror port
 *      mtp_slot     -  (IN) MTP slot in which to install the MTP index.
 *      mtp_index    -  (IN) HW mtp index.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_mtp_slot_port_index_set(int unit, bcm_port_t port,
                                  int mtp_slot, int mtp_index)
{
    uint32 mc_val;
    int cpu_hg_index = 0;
    mirror_control_entry_t mc_entry;
    int vp = VP_PORT_INVALID;
    bcm_module_t mod_out;
    bcm_trunk_t tgid_out = BCM_TRUNK_INVALID;
    bcm_port_t   local_ports[SOC_MAX_NUM_PORTS];
    int  num_local_ports = 0;
    int  port_index = 0;

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &mod_out,
                                   &port, &tgid_out, &vp));
    }
    
    if (BCM_TRUNK_INVALID != tgid_out) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_trunk_local_members_get(unit, tgid_out,
                                             SOC_MAX_NUM_PORTS,
                                             local_ports,
                                             &num_local_ports));
    } else {
        local_ports[0] = port;
        num_local_ports = 1;
    }

    
    /* Non-UC fields are only needed for egress mirroring,
     * but configuring them unconditionally will
     * simplify logic without changing device behavior. */
    for (port_index = 0; port_index < num_local_ports; port_index++) {
        port = local_ports[port_index];
        if (soc_feature(unit, soc_feature_mirror_control_mem)) {
            BCM_IF_ERROR_RETURN
                (READ_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY, port, &mc_entry));
            soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                                _mtp_index_field[mtp_slot], mtp_index);
            soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                                _non_uc_mtp_index_field[mtp_slot], mtp_index);
            BCM_IF_ERROR_RETURN
                (WRITE_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY, port, &mc_entry));

            cpu_hg_index = SOC_IS_KATANA2(unit) ?
                           SOC_INFO(unit).cpu_hg_pp_port_index :
                           SOC_INFO(unit).cpu_hg_index;
            if (IS_CPU_PORT(unit, port) && cpu_hg_index != -1) {
                BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY,
                                    cpu_hg_index, &mc_entry));
                soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                                    _mtp_index_field[mtp_slot], mtp_index);
                soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                            _non_uc_mtp_index_field[mtp_slot], mtp_index);
                BCM_IF_ERROR_RETURN(WRITE_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY,
                                    cpu_hg_index, &mc_entry));
            }
        } else {
            BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &mc_val));
            soc_reg_field_set(unit, MIRROR_CONTROLr, &mc_val,
                                _mtp_index_field[mtp_slot], mtp_index);
            soc_reg_field_set(unit, MIRROR_CONTROLr, &mc_val,
                             _non_uc_mtp_index_field[mtp_slot], mtp_index);
            BCM_IF_ERROR_RETURN(WRITE_MIRROR_CONTROLr(unit, port, mc_val));

            if (IS_CPU_PORT(unit, port)) {
                BCM_IF_ERROR_RETURN(READ_IMIRROR_CONTROLr(unit, port, &mc_val));
                soc_reg_field_set(unit, IMIRROR_CONTROLr, &mc_val,
                                _mtp_index_field[mtp_slot], mtp_index);
                soc_reg_field_set(unit, IMIRROR_CONTROLr, &mc_val,
                             _non_uc_mtp_index_field[mtp_slot], mtp_index);
                BCM_IF_ERROR_RETURN(WRITE_IMIRROR_CONTROLr(unit, port, mc_val));
            }
        }
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_xgs3_mtp_index_port_slot_get
 * Purpose:
 *      Get a port's MTP slot for the given MTP index
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Local mirror port
 *      enables      -  (IN) Bitmap of port's enables
 *      egress       -  (IN) Ingress/Egress indication
 *      mtp_index    -  (IN) HW mtp index.
 *      slot_type    -  (IN) mtp slot types for port,FP, SFLOW, etc.
 *      mtp_slot_p    -  (OUT) MTP slot of the MTP index on the port.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_mtp_index_port_slot_get(int unit, bcm_port_t port,
                                  uint32 enables, int egress,
                                  int mtp_index, int slot_type, int *mtp_slot_p)
{
    int mtp_slot, mtp_bit;          /* MTP type value & bitmap            */ 
    uint32 index_val[BCM_MIRROR_MTP_COUNT];

    /* Verify we are still coherent */
    if (egress) {
        if (enables != (enables & MIRROR_CONFIG_MTP_MODE_BMP(unit))) {
            /* Out of sync! */
            return BCM_E_INTERNAL;
        }
    } else {
        if (enables != (enables & ~MIRROR_CONFIG_MTP_MODE_BMP(unit))) {
            /* Out of sync! */
            return BCM_E_INTERNAL;
        }
    }

    /* The enables are of MTP slots, but we have the MTP index
     * Determine which slot has the index */

    if (slot_type == BCM_MTP_SLOT_TYPE_PORT) {
        /* Read mirror control structure to get programmed mtp slots. */
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_mtp_slot_port_indexes_get(unit, port, index_val));
    } else {
        /* retrieve from software records. Should be in sync with hardware */
        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            index_val[mtp_slot] =
                MIRROR_CONFIG_TYPE_MTP_SLOT(unit, mtp_slot, slot_type);
        }
    }

    BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
        mtp_bit = 1 << mtp_slot;
        /* Check if MTP slot is enabled on port */
        if (!(enables & mtp_bit)) {
            continue;
        }

        if (mtp_index == index_val[mtp_slot]) {
            *mtp_slot_p = mtp_slot;
            return BCM_E_NONE;            
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_tr2_mirror_mtp_slot_update
 * Description:
 *      Write the MTP_TYPE data for the MTP slot configuration
 *      when it is changed. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_mtp_slot_update(int unit)
{
    uint32 ms_reg;             /* MTP mode register value     */

    BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &ms_reg));
    soc_reg_field_set(unit, MIRROR_SELECTr, &ms_reg, MTP_TYPEf,
                      MIRROR_CONFIG_MTP_MODE_BMP(unit));
    BCM_IF_ERROR_RETURN(WRITE_MIRROR_SELECTr(unit, ms_reg));
    BCM_IF_ERROR_RETURN(WRITE_EGR_MIRROR_SELECTr(unit, ms_reg));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_xgs3_mtp_type_slot_reserve
 * Purpose:
 *      Record used MTP slots forPort/ FP/IPFIX usage. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      port_enables -  (IN) Bitmap of port's enables (Port only)
 *      port         -  (IN) Local mirror port (Port only)
 *      mtp_type     -  (IN) Port/FP/IPFIX.
 *      mtp_index    -  (IN) Allocated hw mtp index.
 *      mtp_slot_p   -  (OUT) MTP slot in which to install the MTP index.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_mtp_type_slot_reserve(int unit, uint32 flags, uint32 port_enables,
                                bcm_port_t port, int mtp_type,
                                int mtp_index, int *mtp_slot_p)
{
    int mtp_slot, mtp_bit, free_ptr = -1, free_slot = -1;
    int egress = FALSE;
    int port_mirror;
    bcm_port_t port_ix;
    uint32 index_val[BCM_MIRROR_MTP_COUNT];
    bcm_pbmp_t all_pbmp;
    int          vp = VP_PORT_INVALID;
    bcm_module_t mod_out;
    bcm_trunk_t  tgid_out;
    bcm_port_t   local_port;
    
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &mod_out,
                                   &local_port, &tgid_out, &vp));
    } else {
        local_port = port;
    }

    if (flags & BCM_MIRROR_PORT_EGRESS) {
        egress = TRUE;
    }

    port_mirror = (mtp_type == BCM_MTP_SLOT_TYPE_PORT);
    if (port_mirror) {
        /* Read mirror control structure to get programmed mtp indexes. */
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_mtp_slot_port_indexes_get(unit, port, index_val));
    }

    BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
        mtp_bit = 1 << mtp_slot;
        if (egress) {
            if (!(MIRROR_CONFIG_MTP_MODE_BMP(unit) & mtp_bit)) {
                if (MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit,
                                                     mtp_slot) == 0) {
                    /* MTP Container is undecided, note for later */
                    if (free_ptr < 0) {
                        /* Record unallocated MTP container */
                        free_ptr = mtp_slot;
                    }
                } /* Else, container already used for ingress mirrors */
                continue;
            } else {
                /* Already an egress slot, is it the same MTP? */
                if (port_mirror && (mtp_index != index_val[mtp_slot])) {
                    /* No, keep searching */
                    continue;
                }
            }
        } else {
            if (MIRROR_CONFIG_MTP_MODE_BMP(unit) & mtp_bit) {
                /* Slot configured for egress mirroring, skip */
                continue;
            }
        }
        if (port_mirror) {
            if (!(port_enables & mtp_bit)) { /* Slot unused on this port */
                if ((flags & BCM_MIRROR_PORT_INGRESS) &&
                     BCM_PBMP_MEMBER(
                         MIRROR_CONFIG_PBMP_MTP_SLOT_USED(unit, mtp_slot), 
                         local_port)) {
                    /* The slost is used by the vp created on the port, skip */
                    continue;
                }
                
                if (free_slot < 0) {
                    /* Record free slot */
                    free_slot = mtp_slot;
                }
            } else {
                /* Check if mtp is already installed. */
                if (mtp_index == index_val[mtp_slot]) {
                    /* Match - return mtp_slot */
                    *mtp_slot_p = mtp_slot;
                    return BCM_E_EXISTS;
                }
            }
        } else {
            if (MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot, mtp_type)) {
                if (MIRROR_CONFIG_TYPE_MTP_SLOT(unit, mtp_slot,
                                                mtp_type) == mtp_index) {
                    /* Match - return mtp_slot */
                    *mtp_slot_p = mtp_slot;
                    MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot,
                                                     mtp_type)++;
                    MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, mtp_slot)++;
                    return BCM_E_NONE;
                }
            } else { /* Slot unused on this port */
                if (free_slot < 0) {
                    /* Record free slot */
                    free_slot = mtp_slot;
                }
            }
        }
    }

    /* Use previous allocated slot if available. Otherwise use unallocated
     * MTP continaner.  If neither, we're out of resources. */
    if (free_slot < 0) {
        if (free_ptr < 0) {
            return BCM_E_RESOURCE;
        } else {
            free_slot = free_ptr;
        }
    }

    mtp_slot = free_slot;
    mtp_bit = 1 << free_slot;

    /* Record references and new MTP mode allocation if necessary */
    if (egress && !(MIRROR_CONFIG_MTP_MODE_BMP(unit) & mtp_bit)) {
        /* Update MTP_MODE */
        MIRROR_CONFIG_MTP_MODE_BMP(unit) |= mtp_bit;
    }
    BCM_IF_ERROR_RETURN(_bcm_tr2_mirror_mtp_slot_update(unit));
    MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, mtp_slot)++;

    BCM_PBMP_CLEAR(all_pbmp);
    BCM_PBMP_ASSIGN(all_pbmp, PBMP_ALL(unit));
#ifdef BCM_KATANA2_SUPPORT
    if (SOC_IS_KATANA2(unit) && soc_feature(unit, soc_feature_flex_port)) {
        BCM_IF_ERROR_RETURN(_bcm_kt2_flexio_pbmp_update(unit, &all_pbmp));
    }
    if (soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe)) {
        _bcm_kt2_subport_pbmp_update(unit, &all_pbmp);
    }
#endif

    if (port_mirror) {
        if (egress) {
            /* Must set all ports for egress mirroring */
            PBMP_ITER(all_pbmp, port_ix) {
                BCM_IF_ERROR_RETURN
                    (_bcm_xgs3_mtp_slot_port_index_set(unit, port_ix,
                                                       mtp_slot, mtp_index));
            }
        } else {
            /* Only this port for ingress mirroring */
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_mtp_slot_port_index_set(unit, port,
                                                   mtp_slot, mtp_index));
        }
        BCM_PBMP_PORT_ADD(MIRROR_CONFIG_PBMP_MTP_SLOT_USED(unit, mtp_slot),
                          local_port);
    } else {
        MIRROR_CONFIG_TYPE_MTP_SLOT(unit, mtp_slot, mtp_type) = mtp_index;
    }
    MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot, mtp_type)++;

    /* This configures the mirror behavior, but the enables are set in 
     * the calling functions. */
    *mtp_slot_p = mtp_slot;

    return BCM_E_NONE;
}     

/*
 * Function:
 *     _bcm_xgs3_mtp_type_slot_unreserve
 * Purpose:
 *      Clear a used MTP slot for Port/FP/IPFIX usage. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      port_enables -  (IN) Bitmap of port's enables (Port only)
 *      port         -  (IN) Local mirror port (Port only)
 *      mtp_type     -  (IN) Port/FP/IPFIX.
 *      mtp_index    -  (IN) Allocated hw mtp index.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_mtp_type_slot_unreserve(int unit, uint32 flags,
                                  bcm_port_t port, int mtp_type,
                                  int mtp_index)
{
    int comb_enables;
    int mtp_slot, mtp_bit;
    int egress = FALSE;
    bcm_port_t port_ix;
    int port_mirror;
    uint32 index_val[BCM_MIRROR_MTP_COUNT];
    int          vp = VP_PORT_INVALID;
    bcm_module_t mod_out;
    bcm_trunk_t  tgid_out;
    bcm_port_t   local_port;
    
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &mod_out,
                                   &local_port, &tgid_out, &vp));
    } else {
        local_port = port;
    }
    
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        egress = TRUE;
    }

    port_mirror = (mtp_type == BCM_MTP_SLOT_TYPE_PORT);
    if (port_mirror) {
        /* Read mirror control structure to get programmed mtp indexes. */
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_mtp_slot_port_indexes_get(unit, port, index_val));
    }

    /* Make an effective enable bitmap for "in use" */
    comb_enables = 0;
    BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
        if (MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot, mtp_type)) {
            comb_enables |= (1 << mtp_slot);
        }
    }

    if (egress) {
        comb_enables &= MIRROR_CONFIG_MTP_MODE_BMP(unit);
        /* Only egress slots */
    } else {
        comb_enables &= ~MIRROR_CONFIG_MTP_MODE_BMP(unit);
        /* Only ingress slots */
    }

    BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
        mtp_bit = 1 << mtp_slot;
        if (!(comb_enables & mtp_bit)) {
            continue;
        }

        if (port_mirror) {
            if (index_val[mtp_slot] == mtp_index) {
                /* Removed mtp was found -> disable it. */

                /* Calling function should have already disabled
                 * ipipe mirroring on port. */

                /* Reset ipipe mirroring mtp index. */
                if (!egress) {
                    BCM_IF_ERROR_RETURN
                        (_bcm_xgs3_mtp_slot_port_index_set(unit, port,
                                                       mtp_slot, 0));
                }

                if (MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, mtp_slot) > 0) {
                    MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, mtp_slot)--;
                }
                if (egress) {
                    if (!(MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit,
                                                           mtp_slot))) {
                        /* Free MTP_MODE */
                        MIRROR_CONFIG_MTP_MODE_BMP(unit) &= ~mtp_bit;

                        /* Must clear all ports for egress mirroring */
                        PBMP_ALL_ITER(unit, port_ix) {
                            BCM_IF_ERROR_RETURN
                                (_bcm_xgs3_mtp_slot_port_index_set(unit, port_ix,
                                                       mtp_slot,0));
                        }
                        BCM_IF_ERROR_RETURN
                            (_bcm_tr2_mirror_mtp_slot_update(unit));
                    }
                }
                if (MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot,
                                                     mtp_type) > 0) { 
                    MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot,
                                                     mtp_type)--;
                }
                
                BCM_PBMP_PORT_REMOVE(
                    MIRROR_CONFIG_PBMP_MTP_SLOT_USED(unit, mtp_slot),
                    local_port);
                break;
            }
        } else {
            if (MIRROR_CONFIG_TYPE_MTP_SLOT(unit, mtp_slot,
                                            mtp_type) == mtp_index) {
                /* Found! */
                if (MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, mtp_slot) > 0) {
                    MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, mtp_slot)--;
                }
                if (egress &&
                    !(MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit,
                                                       mtp_slot))) {
                    /* Free MTP_MODE */
                    MIRROR_CONFIG_MTP_MODE_BMP(unit) &= ~mtp_bit;

                    BCM_IF_ERROR_RETURN
                        (_bcm_tr2_mirror_mtp_slot_update(unit));
                }

                if (MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot,
                                                     mtp_type) > 0) { 
                    MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot,
                                                     mtp_type)--;
                }
                if (0 == MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit, mtp_slot,
                                                          mtp_type)) {
                    MIRROR_CONFIG_TYPE_MTP_SLOT(unit, mtp_slot,
                                                mtp_type) = 0;
                }
                break;
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr2_mirror_ipipe_egress_mtp_install
 * Description:
 *      Install IPIPE egress reserved mtp index into 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_ipipe_egress_mtp_install(int unit, bcm_port_t port,
                                         int mtp_index)
{
    int enable;            /* Used mtp bit map.           */
    int mtp_slot;          /* MTP type value              */ 
    
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_egress_get(unit, port, &enable));

    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_mtp_type_slot_reserve(unit,
                                             BCM_MIRROR_PORT_EGRESS,
                                             enable, port,
                                             BCM_MTP_SLOT_TYPE_PORT,
                                             mtp_index, &mtp_slot));
    } else {
        /* Otherwise the slot and index is 1-1 */
        mtp_slot = mtp_index;
    }

    /* if mtp is enabled on port - inform caller */
    if (enable & (1 << mtp_slot)) {
        return (BCM_E_EXISTS);
    }

    /* Update egress enables */
    enable |= (1 << mtp_slot);
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_egress_set(unit, port, enable));

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_tr2_mirror_egress_true_mtp_install
 * Description:
 *      Install egress true mirroring reserved mtp index into 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_egress_true_mtp_install(int unit, bcm_port_t port,
                                        int mtp_index)
{
    int enable;                /* Used mtp bit map.           */

    /* Read mtp egress true mtp enable bitmap for source port. */
    BCM_IF_ERROR_RETURN
        (_bcm_port_mirror_egress_true_enable_get(unit, port, &enable));

    if (!(enable & (1 << mtp_index))) {
        enable |= (1 << mtp_index);
        BCM_IF_ERROR_RETURN
            (_bcm_port_mirror_egress_true_enable_set(unit, port, enable));
    } else {
        /* GNATS: Nothing to do?  Ref counts? */
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr2_mirror_port_ipipe_dest_get
 * Description:
 *      Get IPIPE ingress/egress mirroring destinations for the
 *      specific port.
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      array_sz   - (IN)  Sizeof dest_array parameter.
 *      dest_array - (OUT) Mirror to port array.
 *      egress     - (IN)  (TRUE/FALSE) Egress mirror.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_port_ipipe_dest_get(int unit, bcm_port_t port,
                                    int array_sz, bcm_gport_t *dest_array,
                                    int egress, int vp)
{
    int index;                      /* Destination iteration index.       */
    int mtp_index;                  /* MTP index                          */
    int mtp_slot, mtp_bit;          /* MTP type value & bitmap            */ 
    int comb_enables;               /* Combined port/MTP type enable bits */
    int flexible_mtp;               /* MTP type reconfig permitted        */
    uint32 index_val[BCM_MIRROR_MTP_COUNT];

    flexible_mtp = MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit);

    /* Fold together the port enables and the MTP direction */
    if (egress) {
        if (vp != VP_PORT_INVALID) {
            BCM_IF_ERROR_RETURN(_bcm_tr2_mirror_dvp_enable_get(unit, vp, 
                                  &comb_enables));
        } else {
            /* Read mtp egress mtp enable bitmap for source port. */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_egress_get(unit, port, &comb_enables));
        }
        if (flexible_mtp) {
            comb_enables &= MIRROR_CONFIG_MTP_MODE_BMP(unit);
        }
    } else {
        if (vp != VP_PORT_INVALID) {
            BCM_IF_ERROR_RETURN(_bcm_tr2_mirror_svp_enable_get(unit, vp, 
                                  &comb_enables));
        } else {
            /* Read mtp ingress mtp enable bitmap for source port. */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_ingress_get(unit, port, &comb_enables));
        }
        if (flexible_mtp) {
            comb_enables &= ~MIRROR_CONFIG_MTP_MODE_BMP(unit);
        }
    }

    if (!comb_enables) {
        return (BCM_E_NONE);
    }

    if (vp != VP_PORT_INVALID) {
        if (BCM_GPORT_IS_TRUNK(port)) {
            bcm_trunk_t tid = 0;
            bcm_port_t local_member;
            int local_member_count;
            tid = BCM_GPORT_TRUNK_GET(port);
            BCM_IF_ERROR_RETURN(_bcm_esw_trunk_local_members_get(unit, tid,
                                1, &local_member, &local_member_count));
            port = local_member;
        } else {           
            BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
        }
    }

#ifdef BCM_TRIDENT2PLUS_SUPPORT
    if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
        _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN(
            _bcmi_coe_subport_physical_port_get(unit, port, &port));
    }
#endif

    /* Read mirror control structure to get programmed mtp indexes. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_mtp_slot_port_indexes_get(unit, port, index_val));

    index = 0;
    /* Read mirror control register to compare programmed mtp indexes. */
    BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
        mtp_bit = 1 << mtp_slot;
        /* Check if MTP slot is enabled on port */
        if (!(comb_enables & mtp_bit)) {
            continue;
        }

        /* MTP index is enabled and direction matches */
        mtp_index = index_val[mtp_slot];

        if (flexible_mtp) {
            if (egress) {
                dest_array[index] =
                    MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_index);
            } else {
                dest_array[index] =
                    MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index);
            }
        } else {
            /* Validate MTP index direction matches for shared mode */
            if (MIRROR_CONFIG_SHARED_MTP(unit, mtp_index).egress != egress) {
                continue;
            }

            dest_array[index] =
                MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index);
        }
        index++;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_tr2_mirror_port_egress_true_dest_get
 * Description:
 *      Get IP ingress/egress mirroring destinations for the specific port.
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      array_sz   - (IN)  Sizeof dest_array parameter.
 *      dest_array - (OUT) Mirror to port array.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_port_egress_true_dest_get(int unit, bcm_port_t port,
                                          int array_sz,
                                          bcm_gport_t *dest_array)
{
    int enable;                /* Mirror enable bitmap.       */
    int index;                 /* Destination iteration index.*/
    int mtp_index;

    /* Input parameters check. */
    if ((NULL == dest_array) || (0 == array_sz)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination array. */
    for (index = 0; index < array_sz; index ++) {
        dest_array[index] = BCM_GPORT_INVALID;
    }

    /* Read mtp egress true mtp enable bitmap for source port. */
    BCM_IF_ERROR_RETURN
        (_bcm_port_mirror_egress_true_enable_get(unit, port, &enable));

    if (!enable) {
        return (BCM_E_NONE);
    }

    index = 0;

    /* Egress true mirroring uses 1-1 MTP index to slot mapping */
    for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT;  mtp_index++) {
        if (enable & (1 << mtp_index)) {
            dest_array[index] =
                MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, mtp_index);
            index++;
        }
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_tr2_mirror_ipipe_egress_mtp_uninstall
 * Description:
 *      Reset IPIPE egress reserved mtp index from 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_ipipe_egress_mtp_uninstall(int unit, bcm_port_t port,
                                           int mtp_index)
{
    int enable;            /* Used mtp bit map.           */
    int mtp_slot;          /* MTP type value              */ 

    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_egress_get(unit, port, &enable));

    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_mtp_index_port_slot_get(unit, port, enable, TRUE,
                               mtp_index, BCM_MTP_SLOT_TYPE_PORT,  &mtp_slot));
    } else {
        /* Otherwise the slot and index is 1-1 */
        mtp_slot = mtp_index;
    }

    /* if mtp is not enabled on port - do nothing */
    if (!(enable & (1 << mtp_slot))) {
        return (BCM_E_NOT_FOUND);
    }

    /* Update egress enables */
    enable &= ~(1 << mtp_slot);
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_egress_set(unit, port, enable));

    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_mtp_type_slot_unreserve(unit,
                                               BCM_MIRROR_PORT_EGRESS,
                                               port,
                                               BCM_MTP_SLOT_TYPE_PORT,
                                               mtp_index));
            
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_tr2_mirror_egress_true_mtp_uninstall
 * Description:
 *      Uninstall egress true mirroring reserved mtp index from 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_mirror_egress_true_mtp_uninstall(int unit, bcm_port_t port,
                                          int mtp_index)
{
    int enable;                /* Used mtp bit map.           */
    int rv = BCM_E_NOT_FOUND;  /* Operation return status.    */

    /* Read mtp egress true mtp enable bitmap for source port. */
    BCM_IF_ERROR_RETURN
        (_bcm_port_mirror_egress_true_enable_get(unit, port, &enable));

    if ((enable & (1 << mtp_index))) {
        enable &= ~(1 << mtp_index);
        rv =_bcm_port_mirror_egress_true_enable_set(unit, port, enable);
    }

    return rv;
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      _bcm_xgs3_mirror_ingress_mtp_install
 * Description:
 *      Install ingress reserved mtp index into 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_ingress_mtp_install(int unit, bcm_port_t port,
                                     int mtp_index)
{
    uint32 reg_val;            /* MTP control register value. */
    int enable = 0;            /* Used mtp bit map.           */
    int rv = BCM_E_RESOURCE;   /* Operation return status.    */
    int hw_mtp;                /* Hw installed mtp index.     */
    int mtp_slot_check = 0;      /* MTP Slot map status.      */
    int orig_enable_mtp_map = 0; /* Original Ingress enable   */
    int flags = 0;               /* Slot Check Flag           */

    /* Read mtp ingress mtp enable bitmap for source port. */
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_ingress_get(unit, port, &enable));

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        int mtp_slot;          /* MTP type value              */

        if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
           BCM_IF_ERROR_RETURN
                (_bcm_xgs3_mtp_type_slot_reserve(unit,
                                                 BCM_MIRROR_PORT_INGRESS,
                                                 enable, port,
                                                 BCM_MTP_SLOT_TYPE_PORT,
                                                 mtp_index, &mtp_slot));
        } else {
            /* Otherwise the slot and index is 1-1 */
            mtp_slot = mtp_index;
        }

        /* if mtp is enabled on port - inform caller */
        if (enable & (1 << mtp_slot)) {
            return (BCM_E_EXISTS);
        }

        /* Enable mirroring on the port. */
        enable |= (1 << mtp_slot);
        return _bcm_esw_mirror_ingress_set(unit, port, enable);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Check if slot container are occupied earlier by other module , ignore if not set*/
    if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
        orig_enable_mtp_map = enable;
        flags = _BCM_MIRROR_SLOT_INGRESS | _BCM_MIRROR_SLOT_PORT;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_mtp_slot_valid_get(unit,
                                         flags,
                                         &mtp_slot_check));
        enable = enable | mtp_slot_check;
    }

    /* Read mirror control register to compare programmed mtp indexes. */
    BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &reg_val));

    if (!(enable & BCM_MIRROR_MTP_ONE)) {
        /* Mtp one is available */
        soc_reg_field_set(unit, MIRROR_CONTROLr, &reg_val,
                          IM_MTP_INDEXf, mtp_index);

        /* Retreive orig enable on port */
        if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
            enable = orig_enable_mtp_map;
            MIRROR_CONFIG_ING_MTP_SLOT_OWNER
                        (unit, _BCM_MIRROR_SLOT_CONT0)
                                    |= _BCM_MIRROR_SLOT_OWNER_PORT;
            MIRROR_CONFIG_ING_MTP_SLOT_REF
                        (unit, _BCM_MIRROR_SLOT_CONT0)++;
        }

        /* Set mtp index in Mirror control. */
        BCM_IF_ERROR_RETURN(WRITE_MIRROR_CONTROLr(unit, port, reg_val));

        /* Enable ingress mirroring on the port. */
        enable |= BCM_MIRROR_MTP_ONE;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_mirror_ingress_set(unit, port, enable));
        if (IS_HG_PORT(unit, port)) {
            BCM_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IMIRROR_CONTROLr, 
                                        port, IM_MTP_INDEXf,
                                        mtp_index));
        }
        rv = (BCM_E_NONE);
    } else {
        /* Mtp one is in use */
        /* Check if mtp is already installed. */
        hw_mtp = soc_reg_field_get(unit, MIRROR_CONTROLr, reg_val,
                                   IM_MTP_INDEXf);
        if (mtp_index == hw_mtp) {
            rv = (BCM_E_EXISTS);
        }
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) && BCM_FAILURE(rv) &&
        soc_reg_field_valid(unit, MIRROR_CONTROLr, IM_MTP_INDEX1f)) {
        if (!(enable & BCM_MIRROR_MTP_TWO)) {
            /* Mtp two is available */
            soc_reg_field_set(unit, MIRROR_CONTROLr, &reg_val,
                              IM_MTP_INDEX1f, mtp_index);

            /* Retreive orig enable on port */
            if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
                enable = orig_enable_mtp_map;
                MIRROR_CONFIG_ING_MTP_SLOT_OWNER
                            (unit, _BCM_MIRROR_SLOT_CONT1)
                                    |= _BCM_MIRROR_SLOT_OWNER_PORT;
                MIRROR_CONFIG_ING_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT1)++;
            }

            /* Set mtp index in Mirror control. */
            BCM_IF_ERROR_RETURN(WRITE_MIRROR_CONTROLr(unit, port, reg_val));

            /* Enable ingress mirroring on the port. */
            enable |= BCM_MIRROR_MTP_TWO;
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_ingress_set(unit, port, enable));
            if (IS_HG_PORT(unit, port)) {
                BCM_IF_ERROR_RETURN
                    (soc_reg_field32_modify(unit, IMIRROR_CONTROLr, 
                                            port, IM_MTP_INDEX1f,
                                            mtp_index));
            }
            rv = (BCM_E_NONE);
        } else {
            /* Mtp two is in use */
            /* Check if mtp is already installed. */
            hw_mtp = soc_reg_field_get(unit, MIRROR_CONTROLr,reg_val,
                                       IM_MTP_INDEX1f);
            if (mtp_index == hw_mtp) {
                rv = (BCM_E_EXISTS);
            }
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_mirror_egress_mtp_install
 * Description:
 *      Install egress reserved mtp index into 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_egress_mtp_install(int unit, bcm_port_t port, int mtp_index)
{
    uint32 reg_val;            /* MTP control register value. */
    int enable = 0;            /* Used mtp bit map.           */
    int port_enable = 0;       /* This port's enabled mtp bit map.*/
    int hw_mtp;                /* Hw installed mtp index.     */
    int rv = BCM_E_RESOURCE;   /* Operation return status.    */
    uint32 values[2];
    soc_field_t fields[2] = {EM_MTP_INDEXf, NON_UC_EM_MTP_INDEXf};
    bcm_port_t port_iterator;
    int mtp_slot_check = 0;      /* MTP Slot map status.      */
    int orig_enable_mtp_map = 0; /* Original Egress enable   */
    int flags = 0;               /* Slot Check Flag           */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        return _bcm_tr2_mirror_ipipe_egress_mtp_install(unit, port,
                                                        mtp_index);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    values[0] = values[1] = mtp_index;

    /* Read mtp egress mtp enable bitmap for source port. */
    BCM_IF_ERROR_RETURN
        (_bcm_esw_mirror_egress_get(unit, port, &port_enable));
    /* Read mtp egress mtp enable bitmap for any ports. */
    BCM_IF_ERROR_RETURN
        (_bcm_esw_mirror_egress_get(unit, BCM_GPORT_INVALID, &enable));

    /* Check if slot container are occupied earlier by other module , ignore if not set*/
    if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
        orig_enable_mtp_map = enable;
        flags = _BCM_MIRROR_SLOT_EGRESS | _BCM_MIRROR_SLOT_PORT;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_mtp_slot_valid_get(unit,
                                         flags,
                                         &mtp_slot_check));
        enable = enable | mtp_slot_check;
    }

    /* Read mirror control register to compare programmed mtp indexes. */
    BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &reg_val));

    if (!(enable & BCM_MIRROR_MTP_ONE)) {
        /* Mtp one is available */

        /* Retreive orig enable on port */
        if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
            enable = orig_enable_mtp_map;
            MIRROR_CONFIG_EGR_MTP_SLOT_OWNER
                        (unit, _BCM_MIRROR_SLOT_CONT0)
                                |= _BCM_MIRROR_SLOT_OWNER_PORT;
            MIRROR_CONFIG_EGR_MTP_SLOT_REF
                        (unit, _BCM_MIRROR_SLOT_CONT0)++;
        }

        /* Set all ingress ports to know the MTP index for the
         * egress port to be mirrored. */
        PBMP_ALL_ITER(unit, port_iterator) {
            BCM_IF_ERROR_RETURN 
                (soc_reg_fields32_modify(unit, MIRROR_CONTROLr,
                                         port_iterator, 2, fields, values));
        }
        /* Also enable the CPU's HG flow */
        BCM_IF_ERROR_RETURN 
            (soc_reg_fields32_modify(unit, IMIRROR_CONTROLr,
                                     CMIC_PORT(unit), 2, fields, values));

        /* Enable egress mirroring. */
        port_enable |= BCM_MIRROR_MTP_ONE;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_mirror_egress_set(unit, port, port_enable));

        rv = (BCM_E_NONE);
    } else {
        /* Mtp one is in use */
        /* Check if mtp is already installed on this port. */
        hw_mtp = soc_reg_field_get (unit, MIRROR_CONTROLr, reg_val,
                                    EM_MTP_INDEXf);

        if (mtp_index == hw_mtp) {
            if (port_enable & BCM_MIRROR_MTP_ONE) {
                rv = (BCM_E_EXISTS);
            } else {
                /* MTP one already configured the correct MTP on other ports.
                 * This port must be enabled for it. */
                port_enable |= BCM_MIRROR_MTP_ONE;
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_egress_set(unit, port, port_enable));
                rv = (BCM_E_NONE);
            }
        }
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) && BCM_FAILURE(rv) &&
        soc_reg_field_valid(unit, MIRROR_CONTROLr, EM_MTP_INDEX1f)) {
        if (!(enable & BCM_MIRROR_MTP_TWO)) {
            /* Mtp two is available */

            /* Retreive orig enable on port */
            if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
                enable = orig_enable_mtp_map;
                MIRROR_CONFIG_EGR_MTP_SLOT_OWNER
                            (unit, _BCM_MIRROR_SLOT_CONT1)
                                    |= _BCM_MIRROR_SLOT_OWNER_PORT;
                MIRROR_CONFIG_EGR_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT1)++;
            }

            /* Set all ingress ports to know the MTP index for the
             * egress port to be mirrored. */
            fields[0] = EM_MTP_INDEX1f;
            fields[1] = NON_UC_EM_MTP_INDEX1f;
            PBMP_ALL_ITER(unit, port_iterator) {
                BCM_IF_ERROR_RETURN 
                    (soc_reg_fields32_modify(unit, MIRROR_CONTROLr,
                                             port_iterator,
                                             2, fields, values));
            }
            /* Also enable the CPU's HG flow */
            BCM_IF_ERROR_RETURN 
                (soc_reg_fields32_modify(unit, IMIRROR_CONTROLr,
                                         CMIC_PORT(unit),
                                         2, fields, values));

            /* Enable ingress mirroring on the port. */
            port_enable |= BCM_MIRROR_MTP_TWO;
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_egress_set(unit, port, port_enable));

            rv = (BCM_E_NONE);
        } else {
            /* Mtp one is in use */
            /* Check if mtp is already installed on this port. */
            hw_mtp = soc_reg_field_get(unit, MIRROR_CONTROLr, reg_val,
                                       EM_MTP_INDEX1f);
            if (mtp_index == hw_mtp) {
                if (port_enable & BCM_MIRROR_MTP_TWO) {
                    rv = (BCM_E_EXISTS);
                } else {
                    /* MTP one already configured the correct MTP
                     * on other ports.
                     * This port must be enabled for it. */
                    port_enable |= BCM_MIRROR_MTP_TWO;
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_mirror_egress_set(unit, port,
                                                    port_enable));
                    rv = (BCM_E_NONE);
                }
            }
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_mirror_port_ingress_dest_get
 * Description:
 *      Get ingress mirroring destinations for the specific port 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      array_sz   - (IN)  Sizeof dest_array parameter.
 *      dest_array - (OUT) Mirror to port array.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_port_ingress_dest_get(int unit, bcm_port_t port,
                               int array_sz, bcm_gport_t *dest_array, int vp)
{
    uint32 mtp_value;          /* MTP index value.            */
    uint32 reg_val;            /* MTP control register value. */ 
    int enable;                /* Mirror enable bitmap.       */
    int index;                 /* Destination iteration index.*/

    /* Input parameters check. */
    if ((NULL == dest_array) || (0 == array_sz)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination array. */
    for (index = 0; index < array_sz; index ++) {
        dest_array[index] = BCM_GPORT_INVALID;
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        return _bcm_tr2_mirror_port_ipipe_dest_get(unit, port, array_sz,
                                                   dest_array, FALSE, vp);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */


    /* Read mtp ingress mtp enable bitmap for source port. */
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_ingress_get(unit, port, &enable));

    if (!enable) {
        return (BCM_E_NONE);
    }

    /* Read mirror control register to compare programmed mtp indexes. */
    BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &reg_val));

    index = 0;

    if (enable & BCM_MIRROR_MTP_ONE) {
        /* Mtp one is in use */
        mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr, 
                                      reg_val, IM_MTP_INDEXf);

        dest_array[index] = MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_value);
        index++;
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) && (index < array_sz) ){
        if (enable & BCM_MIRROR_MTP_TWO) {
            /* Mtp two is in use */
            mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr, 
                                          reg_val, IM_MTP_INDEX1f);

            dest_array[index] = MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_value);
            index++;
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_mirror_port_egress_dest_get
 * Description:
 *      Get egress mirroring  destinations for the specific port 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      array_sz   - (IN)  Sizeof dest_array parameter.
 *      dest_array - (OUT) Mirror to port array.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_port_egress_dest_get(int unit, bcm_port_t port, int array_sz,
                                      bcm_gport_t *dest_array, int vp)
{
    uint32 mtp_value;          /* MTP index value.            */
    uint32 reg_val;            /* MTP control register value. */ 
    int enable;                /* Mirror enable bitmap.       */
    int index;                 /* Destination iteration index.*/

    /* Input parameters check. */
    if ((NULL == dest_array) || (0 == array_sz)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination array. */
    for (index = 0; index < array_sz; index ++) {
        dest_array[index] = BCM_GPORT_INVALID;
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        return _bcm_tr2_mirror_port_ipipe_dest_get(unit, port, array_sz,
                                                   dest_array, TRUE, vp);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Read mtp ingress mtp enable enable bitmap for source port. */
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_egress_get(unit, port, &enable));

    if (!enable) {
        return (BCM_E_NONE);
    }

    /* Read mirror control register to compare programmed mtp indexes. */
    BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &reg_val));

    index = 0;

    if (enable & BCM_MIRROR_MTP_ONE) {
        /* Mtp one is in use */
        mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr, 
                                      reg_val, EM_MTP_INDEXf);

        dest_array[index] = MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_value);
        index++;
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) && (index < array_sz) ){
        if (enable & BCM_MIRROR_MTP_TWO) {
            /* Mtp two is in use */
            mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr, 
                                          reg_val, EM_MTP_INDEX1f);

            dest_array[index] = MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_value);
            index++;
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_mirror_ingress_mtp_uninstall
 * Description:
 *      Reset ingress reserved mtp index from 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_ingress_mtp_uninstall(int unit, bcm_port_t port, int mtp_index)
{
    int mtp_value;             /* MTP index value.            */
    uint32 reg_val;            /* MTP control register value. */ 
    int enable;                /* Mirror enable bitmap.       */
    int rv = BCM_E_NOT_FOUND;  /* Operation return status.    */

    /* Read mtp ingress mtp enable enable bitmap for source port. */
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_ingress_get(unit, port, &enable));

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        int mtp_slot;          /* MTP type value              */

        if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_mtp_index_port_slot_get(unit, port,
                                                   enable, FALSE,
                                mtp_index, BCM_MTP_SLOT_TYPE_PORT, &mtp_slot));
        } else {
            mtp_slot = mtp_index;
        }

        if (enable & (1 << mtp_slot)) {
            enable &= ~(1 << mtp_slot);
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_ingress_set(unit, port, enable));
            rv = BCM_E_NONE;
        }

        if (BCM_SUCCESS(rv) &&
            MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_mtp_type_slot_unreserve(unit,
                                                   BCM_MIRROR_PORT_INGRESS,
                                                   port,
                                                   BCM_MTP_SLOT_TYPE_PORT,
                                                   mtp_index));
            
        }
        return rv;
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (enable) {
        /* Read mirror control register to compare programmed mtp indexes. */
        BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &reg_val));
    }

    if (enable & BCM_MIRROR_MTP_ONE) {
        /* Mtp one is in use */
        mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr, 
                                      reg_val, IM_MTP_INDEXf);

        if (mtp_value == mtp_index) {
            /* Removed mtp was found -> disable it. */

            if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
                if(MIRROR_CONFIG_ING_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT0))
                        MIRROR_CONFIG_ING_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT0)--;

                if(MIRROR_CONFIG_ING_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT0) == 0)
                        MIRROR_CONFIG_ING_MTP_SLOT_OWNER
                            (unit, _BCM_MIRROR_SLOT_CONT0)
                                &= ~(_BCM_MIRROR_SLOT_OWNER_PORT);
            }

            /* Disable ingress mirroring on port. */
            enable &= ~BCM_MIRROR_MTP_ONE;
            BCM_IF_ERROR_RETURN 
                (_bcm_esw_mirror_ingress_set(unit, port, enable));

            /* Reset ingress mirroring mtp index. */
            BCM_IF_ERROR_RETURN 
                (soc_reg_field32_modify(unit, MIRROR_CONTROLr,
                                        port, IM_MTP_INDEXf, 0));

            if (IS_HG_PORT(unit, port)) {
                BCM_IF_ERROR_RETURN 
                    (soc_reg_field32_modify(unit, IMIRROR_CONTROLr, 
                                            port, IM_MTP_INDEXf, 0));
            }
            rv = (BCM_E_NONE);
        }
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) && BCM_FAILURE(rv)){
        if (enable & BCM_MIRROR_MTP_TWO) {
            /* Mtp one is in use */
            mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr, 
                                          reg_val, IM_MTP_INDEX1f);

            if (mtp_value == mtp_index) {
                /* Removed mtp was found -> disable it. */

                if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
                    if(MIRROR_CONFIG_ING_MTP_SLOT_REF
                                (unit, _BCM_MIRROR_SLOT_CONT1))
                        MIRROR_CONFIG_ING_MTP_SLOT_REF
                                (unit, _BCM_MIRROR_SLOT_CONT1)--;

                    if(MIRROR_CONFIG_ING_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT1) == 0)
                        MIRROR_CONFIG_ING_MTP_SLOT_OWNER
                            (unit, _BCM_MIRROR_SLOT_CONT1)
                                &= ~(_BCM_MIRROR_SLOT_OWNER_PORT);
                }

                /* Disable ingress mirroring on port. */
                enable &= ~BCM_MIRROR_MTP_TWO;
                BCM_IF_ERROR_RETURN 
                    (_bcm_esw_mirror_ingress_set(unit, port, enable));

                /* Reset ingress mirroring mtp index. */
                BCM_IF_ERROR_RETURN 
                    (soc_reg_field32_modify(unit, MIRROR_CONTROLr,
                                            port, IM_MTP_INDEX1f, 0));

                if (IS_HG_PORT(unit, port)) {
                    BCM_IF_ERROR_RETURN 
                        (soc_reg_field32_modify(unit, IMIRROR_CONTROLr, 
                                                port, IM_MTP_INDEX1f, 0));
                }
                rv = (BCM_E_NONE);
            }
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_mirror_egress_mtp_uninstall
 * Description:
 *      Reset egress reserved mtp index from 
 *      mirror control register. 
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      port       - (IN)  Mirror source gport.
 *      mtp_index  - (IN)  Mirror to port index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_egress_mtp_uninstall(int unit, bcm_port_t port,
                                      int mtp_index)
{
    int mtp_value;             /* MTP index value.            */
    uint32 reg_val;            /* MTP control register value. */ 
    int enable;                /* Mirror enable bitmap.       */
    int port_enable;           /* This port's enabled mtp bit map.*/
    int rv = BCM_E_NOT_FOUND;   /* Operation return status.    */
    uint32 values[2] = {0, 0};
    soc_field_t fields[2] = {EM_MTP_INDEXf, NON_UC_EM_MTP_INDEXf};
    bcm_port_t port_iterator;

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        return _bcm_tr2_mirror_ipipe_egress_mtp_uninstall(unit, port,
                                                          mtp_index);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Read mtp egress mtp enable bitmap for source port. */
    BCM_IF_ERROR_RETURN
        (_bcm_esw_mirror_egress_get(unit, port, &port_enable));

    if (port_enable) {
        /* Read mirror control register to compare programmed mtp indexes. */
        BCM_IF_ERROR_RETURN(READ_MIRROR_CONTROLr(unit, port, &reg_val));
    }

    if (port_enable & BCM_MIRROR_MTP_ONE) {
        /* Mtp one is in use */
        mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr,
                                      reg_val, EM_MTP_INDEXf);

        if (mtp_value == mtp_index) {
            /* Removed mtp was found -> disable it. */

            if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
                if(MIRROR_CONFIG_EGR_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT0))
                    MIRROR_CONFIG_EGR_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT0)--;

                if(MIRROR_CONFIG_EGR_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT0) == 0)
                    MIRROR_CONFIG_EGR_MTP_SLOT_OWNER
                            (unit, _BCM_MIRROR_SLOT_CONT0)
                                  &= ~(_BCM_MIRROR_SLOT_OWNER_PORT);
            }

            /* Disable egress mirroring. */
            port_enable &= ~BCM_MIRROR_MTP_ONE;
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_egress_set(unit, port, port_enable));

            /* Read mtp egress mtp enable bitmap for all ports. */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_egress_get(unit, BCM_GPORT_INVALID,
                                            &enable));

            if (0 == (enable & BCM_MIRROR_MTP_ONE)) {
                /* This egress MTP is no longer in use by any port.
                 * Reset egress mirroring mtp index. */
                PBMP_ALL_ITER(unit, port_iterator) {
                    BCM_IF_ERROR_RETURN 
                        (soc_reg_fields32_modify(unit, MIRROR_CONTROLr,
                                                 port_iterator,
                                                 2, fields, values));
                }
                /* Also enable the CPU's HG flow */
                BCM_IF_ERROR_RETURN 
                    (soc_reg_fields32_modify(unit, IMIRROR_CONTROLr,
                                             CMIC_PORT(unit),
                                             2, fields, values));
            }

            rv = (BCM_E_NONE);
        }
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) && BCM_FAILURE(rv)){
        if (port_enable & BCM_MIRROR_MTP_TWO) {
            /* Mtp two is in use */
            mtp_value = soc_reg_field_get(unit, MIRROR_CONTROLr,
                                          reg_val, EM_MTP_INDEX1f);

            if (mtp_value == mtp_index) {
                /* Removed mtp was found -> disable it. */

                if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {
                    if(MIRROR_CONFIG_EGR_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT1))
                        MIRROR_CONFIG_EGR_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT1)--;

                    if(MIRROR_CONFIG_EGR_MTP_SLOT_REF
                            (unit, _BCM_MIRROR_SLOT_CONT1) == 0)
                        MIRROR_CONFIG_EGR_MTP_SLOT_OWNER
                            (unit, _BCM_MIRROR_SLOT_CONT1)
                                &= ~(_BCM_MIRROR_SLOT_OWNER_PORT);
                }

                /* Disable ingress mirroring. */
                port_enable &= ~BCM_MIRROR_MTP_TWO;
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_egress_set(unit, port, port_enable));

                /* Read mtp egress mtp enable bitmap for all ports. */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_egress_get(unit, BCM_GPORT_INVALID,
                                                &enable));

                if (0 == (enable & BCM_MIRROR_MTP_TWO)) {
                    /* This egress MTP is no longer in use by any port.
                     * Reset egress mirroring mtp index. */
                    fields[0] = EM_MTP_INDEX1f;
                    fields[1] = NON_UC_EM_MTP_INDEX1f;
                    PBMP_ALL_ITER(unit, port_iterator) {
                        BCM_IF_ERROR_RETURN 
                            (soc_reg_fields32_modify(unit, MIRROR_CONTROLr,
                                                     port_iterator,
                                                     2, fields, values));
                    }
                    /* Also enable the CPU's HG flow */
                    BCM_IF_ERROR_RETURN 
                        (soc_reg_fields32_modify(unit, IMIRROR_CONTROLr,
                                                 CMIC_PORT(unit),
                                                 2, fields, values));
                }

                rv = (BCM_E_NONE);
            }
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return (rv);
}


/*
 * Function:
 *	   _bcm_xgs3_mirror_enable_set 
 * Purpose:
 *  	Enable/disable mirroring on a port. 
 * Parameters:
 *	    unit - BCM device number
 *  	port - port number
 *   	enable - enable mirroring if non-zero
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int 
_bcm_xgs3_mirror_enable_set(int unit, int port, int enable)
{
    int cpu_hg_index = 0;

    /* Higig port should never drop directed mirror packets */
    if (IS_ST_PORT(unit, port) && !MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
        enable = 1;
    }

    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        BCM_IF_ERROR_RETURN
            (soc_mem_field32_modify(unit, MIRROR_CONTROLm, port, M_ENABLEf,
                                    enable));

        cpu_hg_index = SOC_IS_KATANA2(unit) ?
                       SOC_INFO(unit).cpu_hg_pp_port_index :
                       SOC_INFO(unit).cpu_hg_index;
        if (IS_CPU_PORT(unit, port) && cpu_hg_index != -1) {
            BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, MIRROR_CONTROLm,
                                cpu_hg_index, M_ENABLEf, enable));
        }
    } else {
        BCM_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MIRROR_CONTROLr, port, M_ENABLEf,
                                    enable));
        /* Enable mirroring of CPU Higig packets as well */
        if (IS_CPU_PORT(unit, port)) {
            BCM_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IMIRROR_CONTROLr, port,
                                        M_ENABLEf, enable));
        }
    }
    return (BCM_E_NONE);
}
/*
 * Function:
 *      bcm_xgs3_mirror_egress_path_set
 * Description:
 *      Set egress mirror packet path for stack ring
 * Parameters:
 *      unit    - (IN) BCM device number
 *      modid   - (IN) Destination module ID (of mirror-to port)
 *      port    - (IN) Stack port for egress mirror packet
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This function should only be used for XGS3 devices stacked
 *      in a ring configuration with fabric devices that may block
 *      egress mirror packets when the mirror-to port is on a 
 *      different device than the egress port being mirrored.
 *      Currently the only such fabric device is BCM5675 rev A0.
 */
int
bcm_xgs3_mirror_egress_path_set(int unit, bcm_module_t modid, bcm_port_t port)
{
    alternate_emirror_bitmap_entry_t egr_bmp;

    if (!soc_feature(unit, soc_feature_egr_mirror_path)) {
        return (BCM_E_UNAVAIL);
    }

    if ( !SOC_MODID_ADDRESSABLE(unit, modid)){
        return (BCM_E_BADID);
    }
    if (!IS_ST_PORT(unit, port)) {
        return (BCM_E_PORT);
    }

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ALTERNATE_EMIRROR_BITMAPm,
                                     MEM_BLOCK_ANY, modid, &egr_bmp));
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) ) {
        soc_field_t bmapf, bmapf_zero;
        uint32 shift;

        if (port < 32) {
            bmapf = BITMAP_LOf;
            bmapf_zero = BITMAP_HIf;
            shift = port;
        } else {
            bmapf = BITMAP_HIf;
            bmapf_zero = BITMAP_LOf;
            shift = port - 32;
        }
        soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                  bmapf, 1 << shift);
        soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                  bmapf_zero, 0);
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_TRIUMPH3_SUPPORT)
    if (SOC_IS_TRIUMPH3(unit)) {
        soc_field_t bmap_w[] = {BITMAP_W0f, BITMAP_W1f}; 
        /* index will identicate which field to program with correct value */
        int index = (port < 32) ? 0 : 1;
        uint32 shift = (port < 32) ? port : port - 32;
        int i;

        for (i = 0; i < COUNTOF(bmap_w); i++) {
            if (index == i) {
                soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                          bmap_w[i], 1 << shift);
            } else {
                soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                          bmap_w[i], 0);
            }
        }

    } else 
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit)) {
        soc_field_t bmap_w[] = {BITMAP_W0f, BITMAP_W1f, BITMAP_W2f}; 
        /* index will identicate which field to program with correct value */
        int index = ((port < 32) ? 0 : ((port < 64) ? 1 : 2));
        uint32 shift = ((port < 32) ? port : ((port < 64) ? (port - 32) : (port - 64) ));
        int i;

        for (i = 0; i < COUNTOF(bmap_w); i++) {
            if (index == i) {
                soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                          bmap_w[i], 1 << shift);
            } else {
                soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                          bmap_w[i], 0);
            }
        }

    } else 
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANAX(unit) ) {
        soc_field_t bmapf, bmapf_zero;
        uint32 shift;

        if (port < 32) {
            bmapf = BITMAP_W0f;
            bmapf_zero = BITMAP_W1f;
            shift = port;
        } else {
            bmapf = BITMAP_W1f;
            bmapf_zero = BITMAP_W0f;
            shift = port - 32;
        }
        soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                  bmapf, 1 << shift);
        soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                  bmapf_zero, 0);
    } else
#endif /* BCM_KATANA_SUPPORT */
    {
   
#if defined(BCM_FIREBOLT_SUPPORT)
        if (SOC_IS_FBX(unit)) {
            port -= SOC_HG_OFFSET(unit);
            soc_ALTERNATE_EMIRROR_BITMAPm_field32_set(unit, &egr_bmp,
                                                      BITMAPf, 1 << port);
        } 
#endif /* BCM_FIREBOLT_SUPPORT */
    } 
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, ALTERNATE_EMIRROR_BITMAPm,
                                      MEM_BLOCK_ALL, modid, &egr_bmp));
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_mirror_egress_path_get
 * Description:
 *      Get egress mirror packet path for stack ring
 * Parameters:
 *      unit    - (IN) BCM device number
 *      modid   - (IN) Destination module ID (of mirror-to port)
 *      port    - (OUT)Stack port for egress mirror packet
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      See bcm_mirror_alt_egress_pbmp_set for more details.
 */
int
bcm_xgs3_mirror_egress_path_get(int unit, bcm_module_t modid, bcm_port_t *port)
{
    alternate_emirror_bitmap_entry_t egr_bmp;
    uint32 val, p, start = 0;

    if (NULL == port) {
        return (BCM_E_PARAM);
    }

    if (!soc_feature(unit, soc_feature_egr_mirror_path)) {
        return (BCM_E_UNAVAIL);
    }
    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
        return (BCM_E_BADID);
    }

    BCM_IF_ERROR_RETURN(soc_mem_read(unit, ALTERNATE_EMIRROR_BITMAPm,
                                     MEM_BLOCK_ANY, modid, &egr_bmp));
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit)) {
        val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, BITMAP_LOf);
        if (val == 0) {
            val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, 
                                                            BITMAP_HIf);
            start = 32;
        }
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_TRIUMPH3(unit)) {
        val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, 
                                                        BITMAP_W0f);
        if (val == 0) {
            val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, 
                                                            BITMAP_W1f);
            start = 32;
        }
        if (val == 0) {
            val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, 
                                                            BITMAP_W2f);
            start = 64;
        }
    } else 
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANAX(unit) ) {
        val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, BITMAP_W0f);
        if (val == 0) {
            val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, 
                                                            BITMAP_W1f);
            start = 32;
        }
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
    {
        val = soc_ALTERNATE_EMIRROR_BITMAPm_field32_get(unit, &egr_bmp, BITMAPf);
    }
    start --; 

    if (val == 0) {
        /* Return default egress port */
        return bcm_esw_topo_port_get(unit, modid, port);
    }
    for (p = start; val; p++)   {
        val >>= 1;
    }
    if (SOC_IS_FBX(unit) && !SOC_IS_TRIUMPH2(unit) && 
        !SOC_IS_APOLLO(unit) && !SOC_IS_VALKYRIE2(unit) 
        && !(SOC_IS_TD_TT(unit) || SOC_IS_TRIUMPH3(unit)) &&
        !(SOC_IS_KATANAX(unit))) {
        p += SOC_HG_OFFSET(unit);
    }
    *port = p;

    return (BCM_E_NONE);
}

/*
 * Function:
 *  	_bcm_xgs3_mirror_egr_dest_get 
 * Purpose:
 *  	Get destination port bitmap for egress mirroring.
 * Parameters:
 *      unit        - (IN) BCM device number.
 *	    port        - (IN) port number.
 *      mtp_index   - (IN) mtp index
 *      dest_bitmap - (OUT) destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_egr_dest_get(int unit, bcm_port_t port, int mtp_index,
                              bcm_pbmp_t *dest_bitmap)
{
    uint32  fval, mirror;
    static const soc_reg_t reg[] = {
        EMIRROR_CONTROLr, EMIRROR_CONTROL1r
    };
#ifdef BCM_GREYHOUND2_SUPPORT
    uint64 fval64, mirror64;
    static const soc_reg_t reg_lo[] = {
        EMIRROR_CONTROL_LO_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t reg_hi[] = {
        EMIRROR_CONTROL_HI_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
#endif /*BCM_GREYHOUND2_SUPPORT*/
    /* Input parameters check. */
    if (NULL == dest_bitmap) {
        return BCM_E_PARAM;
    }

    if ((mtp_index < 0) || 
        (mtp_index >= MIRROR_CONFIG(unit)->port_em_mtp_count)) {
        return BCM_E_PARAM;
    }
#ifdef BCM_GREYHOUND2_SUPPORT
    if(soc_feature(unit, soc_feature_high_portcount_register)){
        BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg_lo[mtp_index], port, 0, &mirror64));

        SOC_PBMP_CLEAR(*dest_bitmap);
        fval64 = soc_reg64_field_get(unit, reg_lo[mtp_index], mirror64, BITMAP_LOf);
        SOC_PBMP_WORD_SET(*dest_bitmap, 0, COMPILER_64_LO(fval64));
        SOC_PBMP_WORD_SET(*dest_bitmap, 1, COMPILER_64_HI(fval64));

        BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg_hi[mtp_index], port, 0, &mirror64));

        fval64 = soc_reg64_field_get(unit, reg_hi[mtp_index], mirror64, BITMAP_LOf);
        SOC_PBMP_WORD_SET(*dest_bitmap, 2, COMPILER_64_LO(fval64));
        SOC_PBMP_WORD_SET(*dest_bitmap, 3, COMPILER_64_HI(fval64));
       
    }else
#endif /*BCM_GREYHOUND2_SUPPORT*/
    {
        BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg[mtp_index], port, 0, &mirror));

        SOC_PBMP_CLEAR(*dest_bitmap);
        fval = soc_reg_field_get(unit, reg[mtp_index], mirror, BITMAPf);
        SOC_PBMP_WORD_SET(*dest_bitmap, 0, fval);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  	_bcm_xgs3_mirror_egr_dest_set 
 * Purpose:
 *  	Set destination port bitmap for egress mirroring.
 * Parameters:
 *      unit        - (IN) BCM device number.
 *      port        - (IN) Port number.
 *      mtp_index   - (IN) mtp index.
 *      dest_bitmap - (IN) Destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_xgs3_mirror_egr_dest_set(int unit, bcm_port_t port, int mtp_index,
                             bcm_pbmp_t *dest_bitmap)
{
    uint32      value;                  
    soc_field_t field = BITMAPf;

    static const soc_reg_t reg[] = {
        EMIRROR_CONTROLr, EMIRROR_CONTROL1r
    };
    static const soc_reg_t hg_reg[] = {
        IEMIRROR_CONTROLr, IEMIRROR_CONTROL1r
    };
#ifdef BCM_GREYHOUND2_SUPPORT
    uint64 mirror, val64;
    static const soc_reg_t reg_lo[] = {
        EMIRROR_CONTROL_LO_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t reg_hi[] = {
        EMIRROR_CONTROL_HI_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t hg_reg_lo[] = {
        IEMIRROR_CONTROL_LO_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
    static const soc_reg_t hg_reg_hi[] = {
        IEMIRROR_CONTROL_HI_64r, INVALIDr,
        INVALIDr, INVALIDr
    };
#endif /*BCM_GREYHOUND2_SUPPORT*/
    /* Input parameters checks. */
    if (NULL == dest_bitmap) {
        return BCM_E_PARAM;
    }
    if ((mtp_index < 0) || 
        (mtp_index >= MIRROR_CONFIG(unit)->port_em_mtp_count)) {
        return BCM_E_PARAM;
    }
#ifdef BCM_GREYHOUND2_SUPPORT
    if(soc_feature(unit, soc_feature_high_portcount_register)){
        /* For Register LO */
        BCM_IF_ERROR_RETURN(READ_EMIRROR_CONTROL_LO_64r(unit, port, &mirror));
        COMPILER_64_SET(val64, SOC_PBMP_WORD_GET(*dest_bitmap, 1),
                        SOC_PBMP_WORD_GET(*dest_bitmap, 0));
        soc_reg64_field_set(unit, reg_lo[mtp_index], &mirror,
                            BITMAP_LOf, val64);
        BCM_IF_ERROR_RETURN(WRITE_EMIRROR_CONTROL_LO_64r(unit, port, mirror));
        /* Enable mirroring of CPU Higig packets as well */
        if (IS_CPU_PORT(unit, port)) {
            COMPILER_64_ZERO(mirror);
            BCM_IF_ERROR_RETURN(READ_IEMIRROR_CONTROL_LO_64r(unit, port, &mirror));
            soc_reg64_field_set(unit, hg_reg_lo[mtp_index], &mirror,
                                BITMAP_LOf, val64);
            BCM_IF_ERROR_RETURN(WRITE_IEMIRROR_CONTROL_LO_64r(unit, port, mirror));
        }
        /* For Register HI */
        COMPILER_64_ZERO(mirror);
        COMPILER_64_ZERO(val64);
        BCM_IF_ERROR_RETURN(READ_EMIRROR_CONTROL_HI_64r(unit, port, &mirror));
        COMPILER_64_SET(val64, SOC_PBMP_WORD_GET(*dest_bitmap, 3),
                        SOC_PBMP_WORD_GET(*dest_bitmap, 2));
        soc_reg64_field_set(unit, reg_hi[mtp_index], &mirror,
                            BITMAP_LOf, val64);
        BCM_IF_ERROR_RETURN(WRITE_EMIRROR_CONTROL_HI_64r(unit, port, mirror));
        /* Enable mirroring of CPU Higig packets as well */
        if (IS_CPU_PORT(unit, port)) {
            COMPILER_64_ZERO(mirror);
            BCM_IF_ERROR_RETURN(READ_IEMIRROR_CONTROL_HI_64r(unit, port, &mirror));
            soc_reg64_field_set(unit, hg_reg_hi[mtp_index], &mirror,
                                BITMAP_LOf, val64);
            BCM_IF_ERROR_RETURN(WRITE_IEMIRROR_CONTROL_HI_64r(unit, port, mirror));
        }
    }else
#endif /*BCM_GREYHOUND2_SUPPORT*/
    {
        value = SOC_PBMP_WORD_GET(*dest_bitmap, 0);

        BCM_IF_ERROR_RETURN
            (soc_reg_fields32_modify(unit, reg[mtp_index],
                                     port, 1, &field, &value));

        /* Enable mirroring of CPU Higig packets as well */
        if (IS_CPU_PORT(unit, port)) {
            BCM_IF_ERROR_RETURN
                (soc_reg_fields32_modify(unit, hg_reg[mtp_index],
                                         port, 1, &field, &value));
        }
    }
    return BCM_E_NONE;
}

#endif /* BCM_XGS3_SWITCH_SUPPORT */

/*
 * Function:
 *  	_bcm_esw_mirror_egr_dest_set
 * Purpose:
 *  	Set destination port bitmap for egress mirroring.
 * Parameters:
 *      unit        - (IN) BCM device number
 *      port        - (IN) Port number
 *      mtp_index   - (IN) mtp index.
 *      dest_bitmap - (IN) destination port bitmap
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_egr_dest_set(int unit, bcm_port_t port, int mtp_index,
                             bcm_pbmp_t *dest_bitmap)
{
    int rv;

    /* Input parameters check */
    if (NULL == dest_bitmap) {
        return (BCM_E_PARAM);
    }
    if ((mtp_index < 0) || (mtp_index >= BCM_MIRROR_MTP_COUNT)) {
        return BCM_E_PARAM;
    }

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        rv = _bcm_trident_mirror_egr_dest_set(unit, port, mtp_index, dest_bitmap);
    } else
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TR_VL(unit)) {
        rv = _bcm_triumph_mirror_egr_dest_set(unit, port, mtp_index, dest_bitmap);
    } else
#endif /* BCM_TRIUMPH_SUPPORT */

#if defined(BCM_RAPTOR1_SUPPORT) 
    if (SOC_IS_RAPTOR(unit)) {
        rv = _bcm_raptor_mirror_egr_dest_set(unit, port, dest_bitmap);
    } else 
#endif /* BCM_RAPTOR1_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = _bcm_xgs3_mirror_egr_dest_set(unit, port, mtp_index, dest_bitmap);
    } else
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    {
        rv = BCM_E_UNAVAIL;
    }

    return (rv);
}


/*
 * Function:
 *  	_bcm_esw_mirror_egr_dest_get
 * Purpose:
 *      Get destination port bitmap for egress mirroring.
 * Parameters:
 *  	unit        - (IN) BCM device number.
 *  	port        - (IN) Port number.
 *      mtp_index   - (IN) mtp index.
 *      dest_bitmap - (OUT) destination port bitmap.
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_egr_dest_get(int unit, bcm_port_t port, int mtp_index,
                             bcm_pbmp_t *dest_bitmap)
{
    int rv;

    /* Input parameters check */
    if (NULL == dest_bitmap) {
        return (BCM_E_PARAM);
    }

    SOC_PBMP_CLEAR(*dest_bitmap);

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        rv = _bcm_trident_mirror_egr_dest_get(unit, port, mtp_index, dest_bitmap);
    } else
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TR_VL(unit)) {
        rv = _bcm_triumph_mirror_egr_dest_get(unit, port, mtp_index, dest_bitmap);
    } else
#endif /* BCM_TRIUMPH_SUPPORT */

#if defined(BCM_RAPTOR1_SUPPORT) 
    if (SOC_IS_RAPTOR(unit)) {
        rv = _bcm_raptor_mirror_egr_dest_get(unit, port, dest_bitmap);
    } else 
#endif /* BCM_RAPTOR1_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = _bcm_xgs3_mirror_egr_dest_get(unit, port, mtp_index, dest_bitmap);
    } else
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    {
        rv = BCM_E_UNAVAIL;
    }

    return (rv);
}

/*
 * Function:
 *	    _bcm_esw_mirror_egress_get
 * Description:
 * 	    Get the mirroring per egress enabled/disabled status
 * Parameters:
 *  	unit -   (IN)  BCM device number
 *  	port -   (IN)  The port to check
 *  	enable - (OUT) Place to store boolean return value for on/off
 * Returns:
 *  	BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_egress_get(int unit, bcm_port_t port, int *enable)
{
    bcm_port_t port_iterator;
    bcm_pbmp_t dest_bitmap;
    int value = 0;
    int mtp_slot, mtp_bit;          /* MTP type value & bitmap            */ 
    int vp = VP_PORT_INVALID;
    int status = 0;
    bcm_module_t mod_out;
    bcm_trunk_t tgid_out;
    
    /* mtp_slot == mtp_index unless directed flexible mirroring is used */
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &mod_out,
                                   &port, &tgid_out, &vp));
    }

    /* Get destination port bitmap from first valid port. */
    PBMP_ALL_ITER(unit, port_iterator) {
        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            mtp_bit = 1 << mtp_slot;

            /* Skip if not egress configured MTP */
            if (!SOC_WARM_BOOT(unit) && /* Else reloading info */
                soc_feature(unit, soc_feature_mirror_flexible)) {
                if (!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                    if (!MIRROR_CONFIG_SHARED_MTP(unit, mtp_slot).egress) {
                        continue;
                    }
                } else {
                    if (!(MIRROR_CONFIG_MTP_MODE_BMP(unit) & mtp_bit)) {
                        continue;
                    }
                }
            }

            if (vp != VP_PORT_INVALID) {
                BCM_IF_ERROR_RETURN(
                    _bcm_tr2_mirror_dvp_enable_get(unit, vp, &status));
                if (status & mtp_bit) {
                    value |= mtp_bit;
                }
            } else {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_egr_dest_get(unit, port_iterator, mtp_slot,
                                                  &dest_bitmap));
                if (BCM_GPORT_INVALID == port) {
                    /* Get the full EM enable status */
                    if (SOC_PBMP_NOT_NULL(dest_bitmap)) {
                        value |= mtp_bit;
                    }
                } else {
                    if (SOC_PBMP_MEMBER(dest_bitmap, port)) {
                        value |= mtp_bit;
                    }
                }
            }
        }
        /* Only care about finding one valid port worth of info */
        break;
    }

    *enable = value;
    return BCM_E_NONE;
}


/*
 * Function:
 * 	   _bcm_esw_mirror_egress_set
 * Description:
 *  	Enable or disable mirroring per egress
 * Parameters:
 *  	unit   - (IN) BCM device number
 *	port   - (IN) The port to affect
 *	enable - (IN) Boolean value for on/off
 * Returns:
 *	    BCM_E_XXX
 * Notes:
 *  	Mirroring must also be globally enabled.
 */
STATIC int
_bcm_esw_mirror_egress_set(int unit, bcm_port_t port, int enable)
{
    bcm_port_t port_iterator;
    bcm_pbmp_t dest_bitmap, all_pbmp;
    int mtp_slot, mtp_bit;          /* MTP type value & bitmap            */ 
    int vp = VP_PORT_INVALID;
    int status;
    bcm_module_t mod_out;
    bcm_trunk_t tgid_out;
    
    /* mtp_slot == mtp_index unless directed flexible mirroring is used */
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &mod_out,
                                   &port, &tgid_out, &vp));
    }

    BCM_PBMP_CLEAR(all_pbmp);
    BCM_PBMP_ASSIGN(all_pbmp, PBMP_ALL(unit));
#ifdef BCM_KATANA2_SUPPORT
    if (SOC_IS_KATANA2(unit) && soc_feature(unit, soc_feature_flex_port)) {
        BCM_IF_ERROR_RETURN(_bcm_kt2_flexio_pbmp_update(unit, &all_pbmp));
    }
    if (soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe)) {
        _bcm_kt2_subport_pbmp_update(unit, &all_pbmp);
    }
#endif

    PBMP_ITER(all_pbmp, port_iterator) {
        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            mtp_bit = 1 << mtp_slot;

            /* Skip if not egress configured MTP */
            if (!SOC_WARM_BOOT(unit) && /* Else reloading info */
                soc_feature(unit, soc_feature_mirror_flexible)) {
                if (!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                    if (!MIRROR_CONFIG_SHARED_MTP(unit, mtp_slot).egress) {
                        continue;
                    }
                } else {
                    if (!(MIRROR_CONFIG_MTP_MODE_BMP(unit) & mtp_bit)) {
                        continue;
                    }
                }
            }
            
            if (vp != VP_PORT_INVALID) {
                BCM_IF_ERROR_RETURN(
                    _bcm_tr2_mirror_dvp_enable_get(unit, vp, &status));
                if (enable & mtp_bit) {
                    status |= mtp_bit;
                } else {
                    status &= ~mtp_bit;
                }
                BCM_IF_ERROR_RETURN(
                    _bcm_tr2_mirror_dvp_enable_set(unit, vp, status));
            } else {

                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_egr_dest_get(unit, port_iterator, mtp_slot,
                                                  &dest_bitmap));

                /* Update egress destination bitmap. */
                if (enable & mtp_bit) {
                    SOC_PBMP_PORT_ADD(dest_bitmap, port);
                } else {
                    SOC_PBMP_PORT_REMOVE(dest_bitmap, port);
                }

                /* Write egress destination bitmap from each local port. */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_mirror_egr_dest_set(unit, port_iterator, mtp_slot,
                                                  &dest_bitmap));
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	  _bcm_esw_directed_mirroring_get
 * Purpose:
 *	  Check if  directed mirroring is enabled on the chip.
 * Parameters:
 *    unit    - (IN) BCM device number.
 *    enable  - (OUT)Directed mirror enabled.
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int
_bcm_esw_directed_mirroring_get(int unit, int *enable)
{
    int  rv = BCM_E_NONE;      /* Operation return status. */

    /* Input parameters check. */
    if (NULL == enable) {
        return (BCM_E_PARAM);
    }

    /* Read switch control to check if directed mirroring is enabled.*/
    rv = bcm_esw_switch_control_get(unit, bcmSwitchDirectedMirroring, enable);
    if (BCM_E_UNAVAIL == rv) {
        *enable = FALSE;
        rv = BCM_E_NONE;
    }
    return (rv);
}


/*
 * Function:
 *      _bcm_esw_mirror_mtp_reserve 
 * Description:
 *      Reserve a mirror-to port
 * Parameters:
 *      unit       - (IN)  BCM device number
 *      dest_id    - (IN)  Mirror destination id.
 *      flags      - (IN)  Mirrored traffic direction. 
 *      index_used - (OUT) MTP index reserved
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the a mirror-to port is reserved more than once
 *      (without being unreserved) then the same MTP index 
 *      will be returned for each call.
 *      Direction should be INGRESS, EGRESS, or EGRESS_TRUE.
 */
int
_bcm_esw_mirror_mtp_reserve(int unit, bcm_gport_t dest_id, 
                            uint32 flags, int *index_used)
{
    int rv = BCM_E_RESOURCE;            /* Operation return status. */

    /* Input parameters check. */
    if (NULL == index_used) {
        return (BCM_E_PARAM);
    }

    /* Allocate MTP index for mirror destination. */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = _bcm_xgs3_mirror_mtp_reserve(unit, dest_id, flags, index_used);
    } else
#endif
    {
        *index_used = 0;
        /*  If mirroring is already in use -> 
            make sure destination is identical, increment reference count.*/
        if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)) {
            /* Mirror destination match. check. */
            if (MIRROR_CONFIG_ING_MTP_DEST(unit, 0) == dest_id) {
                MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)++;
                rv = BCM_E_NONE;
            }
        } else { /* Mirroring not in use. */
            MIRROR_CONFIG_ING_MTP_DEST(unit, 0) = dest_id;
            MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)++;
            MIRROR_DEST_REF_COUNT(unit, dest_id)++;
            rv = BCM_E_NONE;
        }

        /* Ingress & Egress mtp are identical for xgs devices. */
        if (BCM_SUCCESS(rv)) {
            MIRROR_CONFIG_EGR_MTP(unit, 0) = MIRROR_CONFIG_ING_MTP(unit, 0);
        }
    }
    return (rv);
}


/*
 * Function:
 *      _bcm_esw_mirror_mtp_unreserve 
 * Description:
 *      Free  mirror-to port
 * Parameters:
 *      unit       - (IN)  BCM device number.
 *      mtp_index  - (IN)  MTP index. 
 *      is_port    - (IN)  Port based mirror indication.
 *      flags      - (IN)  Mirrored traffic direction. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_mirror_mtp_unreserve(int unit, int mtp_index, int is_port, 
                              uint32 flags)
{
    bcm_gport_t  mirror_dest;
    int          rv = BCM_E_NONE;

    /* Free MTP index for mirror destination. */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_mirror_mtp_unreserve(unit, mtp_index, is_port, flags));
    } else
#endif
    {
        /* Decrement reference counter & reset dest port    */
        /* if destination is no longer in use.              */
        if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0) > 0) {
            MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)--;
            if (0 == MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)) {
                mirror_dest = MIRROR_CONFIG_ING_MTP_DEST(unit, 0);
                MIRROR_CONFIG_ING_MTP_DEST(unit, 0)= BCM_GPORT_INVALID;
                if (MIRROR_DEST_REF_COUNT(unit, mirror_dest) > 0) {
                    MIRROR_DEST_REF_COUNT(unit, mirror_dest)--;
                }
            }
            MIRROR_CONFIG_EGR_MTP(unit, 0) = MIRROR_CONFIG_ING_MTP(unit, 0);
        }
    }
    return rv;
}
/*
 * Function:
 *	  _bcm_esw_mirror_deinit
 * Purpose:
 *	  Internal routine used to free mirror software module.
 *        control structures. 
 * Parameters:
 *        unit     - (IN) BCM device number.
 *        cfg_ptr  - (IN) Pointer to config structure.
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_deinit(int unit, _bcm_mirror_config_p *cfg_ptr)
{
    _bcm_mirror_config_p ptr;
    int mtp_type;

    /* Sanity checks. */
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    if (NULL != EGR_MIRROR_ENCAP(unit)) {
        BCM_IF_ERROR_RETURN
            (soc_profile_mem_destroy(unit, EGR_MIRROR_ENCAP(unit)));
        sal_free(EGR_MIRROR_ENCAP(unit));
        EGR_MIRROR_ENCAP(unit) = NULL;
    }
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

    /* If mirror config was not allocated we are done. */
    if (NULL == cfg_ptr) {
        return (BCM_E_PARAM);
    }

    ptr = *cfg_ptr;
    if (NULL == ptr) {
        return (BCM_E_NONE);
    }

    /* Free mirror destination information. */
    if (NULL != ptr->dest_arr) {
#ifdef BCM_TRIDENT2_SUPPORT
        BCM_IF_ERROR_RETURN(_bcm_mirror_dest_array_mtp_free(unit, ptr));
#endif /* BCM_TRIDENT2_SUPPORT */
        sal_free(ptr->dest_arr);
        ptr->dest_arr = NULL;
    }

    /* Free egress true mtp information. */
    if (NULL != ptr->egr_true_mtp) {
        sal_free(ptr->egr_true_mtp);
        ptr->egr_true_mtp = NULL;
    }

    /* Free MTP types records. */
    for (mtp_type = BCM_MTP_SLOT_TYPE_PORT;
         mtp_type < BCM_MTP_SLOT_TYPES; mtp_type++) {
        if (NULL != ptr->mtp_slot[mtp_type]) {
            sal_free(ptr->mtp_slot[mtp_type]);
        }
    }

    /* Free Legacy Ingress Slot Container */
    if (NULL != ptr->ing_slot_container) {
        sal_free(ptr->ing_slot_container);
        ptr->ing_slot_container = NULL;
    }

    /* Free Legacy Egress Slot Container */
    if (NULL != ptr->egr_slot_container) {
        sal_free(ptr->egr_slot_container);
        ptr->egr_slot_container = NULL;
    }

    /* Free egress mtp information. */
    if (NULL != ptr->egr_mtp) {
        sal_free(ptr->egr_mtp);
        ptr->egr_mtp = NULL;
    }

    /* Free ingress mtp information. */
    if (NULL != ptr->ing_mtp) {
        sal_free(ptr->ing_mtp);
        ptr->ing_mtp = NULL;
    }

    /* Free shared mtp information. */
    if (NULL != ptr->shared_mtp) {
        sal_free(ptr->shared_mtp);
        ptr->shared_mtp = NULL;
    }

    /* Destroy protection mutex. */
    if (NULL != ptr->mutex) {
        sal_mutex_destroy(ptr->mutex);
        ptr->mutex = NULL;
    }

    /* Free module configuration structue. */
    sal_free(ptr);
    *cfg_ptr = NULL;
    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_esw_mirror_enable_set 
 * Purpose:
 *	Enable/disable mirroring on a port
 * Parameters:
 *	unit - BCM device number
 *	port - port number
 *	enable - enable mirroring if non-zero
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *      For non-XGS3 devices this function will also set the
 *      mirror-to port.
 */
int
_bcm_esw_mirror_enable_set(int unit, int port, int enable)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        return _bcm_xgs3_mirror_enable_set(unit, port, enable);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_XGS12_FABRIC_SUPPORT)
    if (SOC_IS_XGS12_FABRIC(unit)) {
        return _bcm_xgs_fabric_mirror_enable_set(unit, port, enable);
    }
#endif /* BCM_XGS12_FABRIC_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *	  _bcm_esw_mirror_mode_set
 * Description:
 *	  Enable or disable mirroring.  Will wait for bcm_esw_mirror_to_set
 *        to be called to actually do the enable if needed.
 * Parameters:
 *        unit            - (IN)     BCM device number
 *	  mode            - (IN)     One of BCM_MIRROR_(DISABLE|L2|L2_L3)
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_mode_set(int unit, int mode)
{
    int    menable;            /* Enable mirroring flag.      */
    int      port;            /* Port iterator.              */
    int      omode;            /* Original mirroring mode.    */
#if defined (BCM_XGS3_SWITCH_SUPPORT)
    int      enable;            /* By direction mirror enable. */
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    int  rv = BCM_E_UNAVAIL;   /* Operation return status.    */
    bcm_pbmp_t all_pbmp;
#if defined BCM_KATANA2_SUPPORT 
    int    min_subport = SOC_INFO(unit).pp_port_index_min;
#endif

    /* Preserve original module configuration. */
    omode = MIRROR_CONFIG_MODE(unit);  

    /* Update module mode. */
    MIRROR_CONFIG_MODE(unit) = mode;
    menable = (BCM_MIRROR_DISABLE != mode) ? TRUE : FALSE;

    if (!menable) {
        /* If mirroring was originally off - we are done. */
        if (!SOC_IS_XGS12_FABRIC(unit) && (omode == BCM_MIRROR_DISABLE)) {
            return (BCM_E_NONE);
        }
    }

    /* Wait for mirror_to_set() */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        if ((BCM_GPORT_INVALID == MIRROR_CONFIG_SHARED_MTP_DEST(unit, 0)) &&
            !SOC_IS_XGS12_FABRIC(unit)) {
            return (BCM_E_NONE);
        }
    } else {
        if ((BCM_GPORT_INVALID == MIRROR_CONFIG_ING_MTP_DEST(unit, 0)) &&
            !SOC_IS_XGS12_FABRIC(unit)) {
            return (BCM_E_NONE);
        }
    }

    BCM_PBMP_CLEAR(all_pbmp);
    BCM_PBMP_ASSIGN(all_pbmp, PBMP_ALL(unit));
#ifdef BCM_KATANA2_SUPPORT
    if (SOC_IS_KATANA2(unit) && soc_feature(unit, soc_feature_flex_port)) {
        BCM_IF_ERROR_RETURN(_bcm_kt2_flexio_pbmp_update(unit, &all_pbmp));
    }
    if (soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe)) {
        _bcm_kt2_subport_pbmp_update(unit, &all_pbmp);
    }
#endif

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        PBMP_ITER(all_pbmp, port) {
            /* Skip special ports (loopback port, etc.) */
#if defined(BCM_KATANA2_SUPPORT)
            if ((soc_feature(unit, soc_feature_linkphy_coe) ||
                soc_feature(unit, soc_feature_subtag_coe)) &&
                (port >= min_subport)) {
            } else
#endif
            if (!IS_PORT(unit, port) && !IS_CPU_PORT(unit, port)) {
                continue;
            }

            rv = bcm_esw_mirror_ingress_get(unit, port, &enable);
            if (BCM_FAILURE(rv)) {
                break;
            }
            if (enable) {
                rv = _bcm_xgs3_mirror_ingress_mtp_install(unit, port, 0);
                if (BCM_E_EXISTS == rv) {
                    /* Configured from a previous mode. */
                    rv = BCM_E_NONE;
                } else if (BCM_FAILURE(rv)) {
                    break;
                }
            }

            rv = bcm_esw_mirror_egress_get(unit, port, &enable);
            if (BCM_FAILURE(rv)) {
                break;
            }

            if (enable) {
                rv = _bcm_xgs3_mirror_egress_mtp_install(unit, port, 0);
                if (BCM_E_EXISTS == rv) {
                    /* Configured from a previous mode. */
                    rv = BCM_E_NONE;
                } else if (BCM_FAILURE(rv)) {
                    break;
                }
            }

            rv = _bcm_esw_mirror_enable_set(unit, port, menable);
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
    } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_XGS_SWITCH_SUPPORT)
    if (SOC_IS_XGS_SWITCH(unit)) {
        PBMP_ITER(all_pbmp, port) {
            /* Skip special ports (loopback port, etc.) */
#if defined(BCM_KATANA2_SUPPORT)
            if ((soc_feature(unit, soc_feature_linkphy_coe) ||
                soc_feature(unit, soc_feature_subtag_coe)) &&
                (port >= min_subport)) {
            } else
#endif
            if (!IS_PORT(unit, port) && !IS_CPU_PORT(unit, port)) {
                continue;
            }

            rv = _bcm_esw_mirror_enable_set(unit, port, menable);
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
    } else 
#endif /* BCM_XGS_SWITCH_SUPPORT */

#if defined(BCM_XGS_FABRIC_SUPPORT)
    if (SOC_IS_XGS_FABRIC(unit)) {
        PBMP_ST_ITER(unit, port) {
            rv = _bcm_esw_mirror_enable_set(unit, port, menable);
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
    } else
#endif /* BCM_XGS_FABRIC_SUPPORT */
    {
        rv = BCM_E_UNAVAIL;
    }
    return (rv);
}

/*
 * Function:
 *	  _bcm_esw_mirror_hw_clear
 * Purpose:
 *	  Clear hw registers/tables & disable mirroring on the device.
 * Parameters:
 *    unit - (IN) BCM device number.
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int
_bcm_esw_mirror_hw_clear(int unit)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    int port;   /* Port iteration index. */
    bcm_pbmp_t pbmp, all_pbmp;
    int mtp_index;  /* MTP itteration index */
#if defined(BCM_TRIUMPH2_SUPPORT)
    mirror_control_entry_t mc_entry; /* MTP control memory value (Trident) */
    uint32 mc_reg;                 /* MTP control register value.  */
    int mtp_type_undir = FALSE;    /* MTP_TYPE set for undirected mode */
    int cpu_hg_index = 0;
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (SOC_IS_XGS3_SWITCH(unit)) {
        /* Stacking ports should never drop directed mirror packets */
        /* Other ports should default to no mirroring */
        BCM_PBMP_CLEAR(all_pbmp);
        BCM_PBMP_ASSIGN(all_pbmp, PBMP_ALL(unit));
#ifdef BCM_KATANA2_SUPPORT
        if (SOC_IS_KATANA2(unit) && soc_feature(unit, soc_feature_flex_port)) {
            BCM_IF_ERROR_RETURN(_bcm_kt2_flexio_pbmp_update(unit, &all_pbmp));
        }
        if (soc_feature(unit, soc_feature_linkphy_coe) ||
            soc_feature(unit, soc_feature_subtag_coe)) {
            _bcm_kt2_subport_pbmp_update(unit, &all_pbmp);
        }
#endif

#if defined(BCM_TRIUMPH2_SUPPORT)
        /* Initialize default mirror control settings */
        if (soc_feature(unit, soc_feature_mirror_flexible) ||
            (soc_feature(unit, soc_feature_egr_mirror_true))) {
            if (soc_feature(unit, soc_feature_mirror_control_mem)) {
                sal_memset(&mc_entry, 0, sizeof(mc_entry));
                for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT;
                     mtp_index++) {
                    soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                                        _mtp_index_field[mtp_index],
                                        mtp_index);
                    soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                                        _non_uc_mtp_index_field[mtp_index],
                                        mtp_index);
                }
            } else {
                mc_reg = 0;
                for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT;
                     mtp_index++) {
                    soc_reg_field_set(unit, MIRROR_CONTROLr, &mc_reg,
                                      _mtp_index_field[mtp_index],
                                          mtp_index);
                    soc_reg_field_set(unit, MIRROR_CONTROLr, &mc_reg,
                                      _non_uc_mtp_index_field[mtp_index],
                                      mtp_index);
                }
            }
        }

        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            if (MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
                /* Draco 1.5 mirroring mode. */
                mtp_type_undir = TRUE;
                MIRROR_CONFIG_SHARED_MTP(unit,
                      BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX).egress = TRUE;
            }
        }

#endif /* BCM_TRIUMPH2_SUPPORT */

        PBMP_ITER(all_pbmp, port) {
#if defined(BCM_TRIUMPH2_SUPPORT)
            /* Set up standard settings for flexible mirroring. */
            if (soc_feature(unit, soc_feature_mirror_flexible) ||
                (soc_feature(unit, soc_feature_egr_mirror_true) &&
                 IS_LB_PORT(unit, port))) {

                /* Set MTP mapping to 1-1 for flexible mirroring,
                   or egress true mirroring LB port */
                if (soc_feature(unit, soc_feature_mirror_control_mem)) {
                    BCM_IF_ERROR_RETURN
                        (WRITE_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY,
                                               port, &mc_entry));
                    cpu_hg_index = SOC_IS_KATANA2(unit) ?
                                   SOC_INFO(unit).cpu_hg_pp_port_index :
                                   SOC_INFO(unit).cpu_hg_index;
                    if (IS_CPU_PORT(unit, port) && cpu_hg_index != -1) {
                        BCM_IF_ERROR_RETURN
                            (WRITE_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY,
                                 cpu_hg_index, &mc_entry));
                    }
                } else {
                    BCM_IF_ERROR_RETURN
                        (WRITE_MIRROR_CONTROLr(unit, port, mc_reg));
                    if (IS_CPU_PORT(unit, port)) {
                        BCM_IF_ERROR_RETURN
                            (WRITE_IMIRROR_CONTROLr(unit, port, mc_reg));
                    }
                }
            }
#endif /* BCM_TRIUMPH2_SUPPORT */

            /* Reset global mirror enable bit after the mirror control
             * register is handled. */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_enable_set(unit, port, 
                                            IS_ST_PORT(unit, port) ? 1 : 0));

            /* Disable ingress mirroring. */
            BCM_IF_ERROR_RETURN(_bcm_esw_mirror_ingress_set(unit, port, 0));

            /* Disable egress mirroring for all MTP indexes */ 
            SOC_PBMP_CLEAR(pbmp);

            for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT; mtp_index++) {
                (void)(_bcm_esw_mirror_egr_dest_set(unit, port, mtp_index, 
                                                    &pbmp));
            }

#ifdef BCM_FIREBOLT_SUPPORT
            /* Clear RSPAN settings */
            if (SOC_REG_IS_VALID(unit, EGR_RSPAN_VLAN_TAGr)) {
                BCM_IF_ERROR_RETURN(WRITE_EGR_RSPAN_VLAN_TAGr(unit, port, 0));
            }
#endif /* BCM_FIREBOLT_SUPPORT */
        }

        if (SOC_MEM_IS_VALID(unit, IM_MTP_INDEXm)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_clear(unit, IM_MTP_INDEXm, COPYNO_ALL, 0));
        }
        if (SOC_MEM_IS_VALID(unit, EM_MTP_INDEXm)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_clear(unit, EM_MTP_INDEXm, COPYNO_ALL, 0));
        }
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_MEM_IS_VALID(unit, EP_REDIRECT_EM_MTP_INDEXm)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_clear(unit, EP_REDIRECT_EM_MTP_INDEXm,
                               COPYNO_ALL, 0));
        }
        if (SOC_MEM_IS_VALID(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_clear(unit, EGR_EP_REDIRECT_EM_MTP_INDEXm,
                               COPYNO_ALL, 0));
        }
        /* Clear settings of mirror_to_pbmp_set */
#ifdef BCM_GREYHOUND2_SUPPORT        
        if(soc_feature(unit, soc_feature_high_portcount_register)){
            uint64 val64, mirror;
            COMPILER_64_ZERO(val64);
            PBMP_ALL_ITER(unit, port) {
                BCM_IF_ERROR_RETURN(READ_IMIRROR_BITMAP_LOr(unit, port, &mirror));
                soc_reg64_field_set(unit, IMIRROR_BITMAP_LOr, &mirror,
                                    BITMAPf, val64);
                BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_LOr(unit, port, mirror));

            }
            PBMP_ALL_ITER(unit, port) {
                BCM_IF_ERROR_RETURN(READ_IMIRROR_BITMAP_HIr(unit, port, &mirror));
                soc_reg64_field_set(unit, IMIRROR_BITMAP_HIr, &mirror,
                                    BITMAPf, val64);
            BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_HIr(unit, port, mirror));
            }
        }else 
#endif /*BCM_GREYHOUND2_SUPPORT*/
        if (SOC_REG_IS_VALID(unit, IMIRROR_BITMAP_64r)) {
            uint32 values[2];
            soc_field_t fields[] = {BITMAP_LOf, BITMAP_HIf};
            values[0] = values[1] = 0;
            PBMP_ALL_ITER(unit, port) {
                BCM_IF_ERROR_RETURN
                    (soc_reg_fields32_modify(unit, IMIRROR_BITMAP_64r, port,
                                             2, fields, values));
            }
        } else     
#endif /* BCM_TRIUMPH2_SUPPORT */
        if (SOC_REG_IS_VALID(unit, IMIRROR_BITMAPr)) {
            PBMP_ALL_ITER(unit, port) {
                BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAPr(unit, port, 0));
            }
        }
#if defined(BCM_TRX_SUPPORT)
        if (SOC_MEM_IS_VALID(unit, EGR_ERSPANm)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_clear(unit, EGR_ERSPANm, COPYNO_ALL, 0));
        }
#endif /* BCM_TRX_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_mirror_control_mem)) {
            int i; 
            imirror_bitmap_entry_t entry;
            soc_mem_t   td_tt_mem_arr[] = { EMIRROR_CONTROLm, EMIRROR_CONTROL1m, 
                EMIRROR_CONTROL2m, EMIRROR_CONTROL3m, EGR_MIRROR_ENCAP_CONTROLm,
                EGR_MIRROR_ENCAP_DATA_1m, EGR_MIRROR_ENCAP_DATA_2m };
            /* Clear all valid memories upon init */
            for (i = 0; i < COUNTOF(td_tt_mem_arr); i++) {
                if (SOC_MEM_IS_VALID(unit, td_tt_mem_arr[i])) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_clear(unit, td_tt_mem_arr[i], COPYNO_ALL, 0));
                }
            }
            /* Clear settings of mirror_to_pbmp_set */
            sal_memset(&entry, 0, sizeof(imirror_bitmap_entry_t));
            PBMP_ALL_ITER(unit, port) {
                BCM_IF_ERROR_RETURN
                    (WRITE_IMIRROR_BITMAPm(unit, MEM_BLOCK_ANY, port, &entry));
            }
        }
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_GREYHOUND_SUPPORT)
        if (SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) ||
            SOC_IS_GREYHOUND2(unit)) {
            int j; 
            soc_mem_t   gh_mem_arr[] = { EGR_MIRROR_ENCAP_CONTROLm,
                EGR_MIRROR_ENCAP_DATA_1m, EGR_MIRROR_ENCAP_DATA_2m };
           
            /* Clear all valid memories upon init */
            for (j = 0; j < COUNTOF(gh_mem_arr); j++) {
                if (SOC_MEM_IS_VALID(unit, gh_mem_arr[j])) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_clear(unit, gh_mem_arr[j], COPYNO_ALL, 0));
                }
            }
        }
#endif /* BCM_GREYHOUND_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT)
        /* Clear settings of mirror_select register */
        if (SOC_REG_IS_VALID(unit, MIRROR_SELECTr)) {
            BCM_IF_ERROR_RETURN(
                 soc_reg_field32_modify(unit, MIRROR_SELECTr, REG_PORT_ANY, 
                                        MTP_TYPEf, mtp_type_undir ?
                                 BCM_MIRROR_MTP_FLEX_EGRESS_D15: 0x0));
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

        /* Mirror is disabled by default on the switch. */
        MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_DISABLE;
       
    } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    { 
        BCM_IF_ERROR_RETURN
            (_bcm_esw_mirror_mode_set(unit, BCM_MIRROR_DISABLE));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *   	_bcm_esw_mirror_stk_update
 * Description:
 *	Stack callback to re-program path for mirror-to-port when
 *      there is an alternate path available to the unit on which 
 *      MTP is present.
 * Parameters:
 *	unit   - (IN)BCM device number
 *      modid  - (IN)Module id. 
 *      port   - (IN)
 *      pbmp   - (IN)
 * Returns:
 *	    BCM_E_XXX
 */
int
_bcm_esw_mirror_stk_update(int unit, bcm_module_t modid, bcm_port_t port,
                           bcm_pbmp_t pbmp)
{
    /* Initialization check. */
    if (!MIRROR_INIT(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return (BCM_E_PORT);
    }

    if (SOC_PBMP_IS_NULL(pbmp)) {
        return (BCM_E_NONE);
    }

#ifdef BCM_HERCULES_SUPPORT
    if (SOC_IS_HERCULES(unit) || SOC_IS_HERCULES15(unit)) {
        bcm_port_t hg_port;
        bcm_pbmp_t uc_pbmp, mir_pbmp;

        BCM_IF_ERROR_RETURN
            (bcm_esw_stk_ucbitmap_get(unit, port, modid, &uc_pbmp));

        PBMP_HG_ITER(unit, hg_port) {
            uint32 mirbmap, old_bmap;

            BCM_IF_ERROR_RETURN (READ_ING_MIRTOBMAPr(unit, hg_port,  &mirbmap));
            old_bmap = mirbmap;
            SOC_PBMP_CLEAR(mir_pbmp);
            SOC_PBMP_WORD_SET(mir_pbmp, 0, mirbmap);

            if (SOC_PBMP_EQ(mir_pbmp, uc_pbmp)) {
                soc_reg_field_set(unit, ING_MIRTOBMAPr, &mirbmap, BMAPf,
                                  SOC_PBMP_WORD_GET(pbmp, 0));
                if (old_bmap != mirbmap) {
                    BCM_IF_ERROR_RETURN
                        (WRITE_ING_MIRTOBMAPr(unit, hg_port, mirbmap));
                }
            }
        }
    }
#endif
    return (BCM_E_NONE);
}

STATIC int
_bcm_esw_mirror_port_dest_mtp_ref_adjust(int unit, bcm_port_t port,
                                         int flags, int mtp_index,
                                         bcm_gport_t mirror_dest)
{
    int rv;
    bcm_mirror_destination_t mdest;  /* Mirror destination. */
    uint8 skip = TRUE;

    /* The device which doesn't support flexible mirror need not adjust */
    if (!soc_feature(unit, soc_feature_mirror_flexible)) {
        return BCM_E_NONE;
    }

    /* Get mirror destination descriptor. */
    BCM_IF_ERROR_RETURN
        (bcm_esw_mirror_destination_get(unit, mirror_dest, &mdest));

#ifdef BCM_TRIDENT2_SUPPORT
    if (mdest.flags & BCM_MIRROR_DEST_ID_SHARE) {
        skip = FALSE;
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    if (mdest.flags & BCM_MIRROR_DEST_REPLACE) {
        skip = FALSE;
    }

    /* Only mirror destination with replace flag need adjust */
    if (skip) {
        return BCM_E_NONE;
    }

    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_INGRESS,
                                                  mirror_dest);
            /* The ref_count of mtp_index for same src_port doesn't need to get increased twice */
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, mtp_index)--;
            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS) {
            rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_EGRESS,
                                                  mirror_dest);
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, mtp_index)--;
            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_EGRESS_TRUE,
                                                  mirror_dest);
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, mtp_index)--;
            }
        }
    } else {
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_INGRESS,
                                                  mirror_dest);
            /* The ref_count of mtp_index for same src_port doesn't need to get increased twice */
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_SHARED_MTP(unit, mtp_index).egress == FALSE) &&
                (MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)--;
            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS) {
            rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_EGRESS,
                                                  mirror_dest);
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_SHARED_MTP(unit, mtp_index).egress == TRUE) &&
                (MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)--;
            }
        }

        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_EGRESS_TRUE,
                                                  mirror_dest);
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(unit, mtp_index)--;
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_esw_mirror_port_ingress_dest_add 
 * Purpose:
 *      Add ingress mirroring destination to a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_esw_mirror_port_ingress_dest_add(int unit, bcm_port_t port,
                                                     bcm_gport_t mirror_dest)
{
    int mtp_index;           /* Mirror to port index.    */
    int rv;                  /* Operation return status. */

    /* Allocate MTP index for mirror destination. */
    rv = _bcm_esw_mirror_mtp_reserve(unit, mirror_dest,
                                     BCM_MIRROR_PORT_INGRESS, &mtp_index);
    /* Check for mtp allocation failure. */
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    if ((-1 != port) && 
        (BCM_GPORT_IS_SET(port) || SOC_PORT_VALID(unit, port))) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_port_dest_mtp_ref_adjust(unit, port,
                                                          BCM_MIRROR_PORT_INGRESS,
                                                          mtp_index,
                                                          mirror_dest));
            rv = _bcm_xgs3_mirror_ingress_mtp_install(unit, port, mtp_index);
            if (BCM_E_EXISTS == rv) {
               rv = BCM_E_NONE;
            }
        } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        {
            rv = bcm_esw_mirror_ingress_set(unit, port, TRUE);
        }

        /* Check for mtp enable failure. */
        if (BCM_FAILURE(rv)) {
            _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, TRUE, 
                                          BCM_MIRROR_PORT_INGRESS);
        }
    }
    return (rv);
}

/*
 * Function:
 *     _bcm_esw_mirror_port_egress_dest_add 
 * Purpose:
 *      Add egress mirroring destination to a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_esw_mirror_port_egress_dest_add(int unit, bcm_port_t port, 
                                     bcm_gport_t mirror_dest)
{
    int mtp_index;           /* Mirror to port index.    */
    int rv;                  /* Operation return status. */

    /* Allocate MTP index for mirror destination. */
    rv = _bcm_esw_mirror_mtp_reserve(unit, mirror_dest,
                                     BCM_MIRROR_PORT_EGRESS, &mtp_index);
    /* Check for mtp allocation failure. */
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    /* Enable MTP index on mirror source port */
    if ((-1 != port) && 
        (BCM_GPORT_IS_SET(port) || SOC_PORT_VALID(unit, port))) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_mirror_port_dest_mtp_ref_adjust(unit, port,
                                                          BCM_MIRROR_PORT_EGRESS,
                                                          mtp_index,
                                                          mirror_dest));
            /* Enable MTP index on mirror source port */
            rv = _bcm_xgs3_mirror_egress_mtp_install(unit, port, mtp_index);
            if (BCM_E_EXISTS == rv) {
               rv = BCM_E_NONE;
            }
        } else
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        {
            rv = bcm_esw_mirror_egress_set(unit, port, TRUE);
        }

        /* Check for mtp enable failure. */
        if (BCM_FAILURE(rv)) {
            _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, TRUE, 
                                          BCM_MIRROR_PORT_EGRESS);
        }
    }

    return (rv);
}

/*
 * Function:
 *     _bcm_esw_mtp_slot_valid_get
 * Purpose:
 *      Check for slot used/unused status in TRX devices not supporting
 *      flexible mirroring.
 * Parameters:
 *      unit               -  (IN) BCM device number.
 *      flags              -  (IN) _BCM_MIRROR_SLOT_XXX flags.
 *      mtp_slot_status    -  (OUT) Allocated slot container status.
 * Returns:
 *      BCM_X_XXX
 * Notes:
 *      Should not be called for devices supporting flexible mirroriing.
 */
int
_bcm_esw_mtp_slot_valid_get(int unit, uint32 flags, int *mtp_slot_status)
{
    int mtp_slot = 0, mtp_map = 0;

    /* Null Param Check */
    if (mtp_slot_status == NULL) {
        return BCM_E_PARAM;
    }

    /* Initialization check */
    if (!MIRROR_INIT(unit)) {
        return (BCM_E_INIT);
    }

    /* Check for Slot Container valid status */
    if (flags & _BCM_MIRROR_SLOT_INGRESS) {
        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            mtp_map = 1 << mtp_slot;
            if (flags & _BCM_MIRROR_SLOT_PORT) {
                if(MIRROR_CONFIG_ING_MTP_SLOT_OWNER
                       (unit, mtp_slot) & _BCM_MIRROR_SLOT_OWNER_FP) {
                    *mtp_slot_status |= mtp_map;
                }
            } else if (flags & _BCM_MIRROR_SLOT_FP) {
                if (MIRROR_CONFIG_ING_MTP_SLOT_OWNER
                        (unit, mtp_slot) & _BCM_MIRROR_SLOT_OWNER_PORT) {
                    *mtp_slot_status |= mtp_map;
                }
            } else {
                /* No other flag is supported */
                return BCM_E_PARAM;
            }
        }
    } else if (flags & _BCM_MIRROR_SLOT_EGRESS) {
        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            mtp_map = 1 << mtp_slot;
            if (flags & _BCM_MIRROR_SLOT_PORT) {
                if (MIRROR_CONFIG_EGR_MTP_SLOT_OWNER
                        (unit, mtp_slot) & _BCM_MIRROR_SLOT_OWNER_FP) {
                    *mtp_slot_status |= mtp_map;
                } else if (MIRROR_CONFIG_EGR_MTP_SLOT_OWNER
                        (unit, mtp_slot) & _BCM_MIRROR_SLOT_OWNER_PORT) {
                    /* Egress Slot Container within port are exclusive */
                    *mtp_slot_status |= mtp_map;
                }
            } else if(flags & _BCM_MIRROR_SLOT_FP) {
                if (MIRROR_CONFIG_EGR_MTP_SLOT_OWNER
                        (unit, mtp_slot) & _BCM_MIRROR_SLOT_OWNER_PORT) {
                    *mtp_slot_status |= mtp_map;
                }
            } else {
                /* No other flag is supported */
                return BCM_E_PARAM;
            }
        }
    } else {
        /* No other flag is supported */
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

#if defined(BCM_TRIUMPH2_SUPPORT)
/*
 * Function:
 *     _bcm_esw_mirror_port_egress_true_dest_add 
 * Purpose:
 *      Add egress_true mirroring destination to a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_esw_mirror_port_egress_true_dest_add(int unit, bcm_port_t port, 
                                          bcm_gport_t mirror_dest)
{
    int mtp_index;           /* Mirror to port index.    */
    int rv;                  /* Operation return status. */

    /* Allocate MTP index for mirror destination. */
    if (MIRROR_CONFIG(unit)->egr_true_mtp_count == 0) {
        return BCM_E_RESOURCE;
    }

    rv = _bcm_esw_mirror_mtp_reserve(unit, mirror_dest,
                                     BCM_MIRROR_PORT_EGRESS_TRUE, 
                                     &mtp_index);
    /* Check for mtp allocation failure. */
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    /* Enable MTP index on mirror source port */
    if ((-1 != port) && SOC_PORT_VALID(unit, port)) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_mirror_port_dest_mtp_ref_adjust(unit, port,
                                                      BCM_MIRROR_PORT_EGRESS_TRUE,
                                                      mtp_index,
                                                      mirror_dest));
        /* Enable MTP index on mirror source port */
        rv = _bcm_tr2_mirror_egress_true_mtp_install(unit, port, mtp_index);
        if (rv == BCM_E_EXISTS) {
           rv = BCM_E_NONE;
        }

        /* Check for mtp enable failure. */
        if (BCM_FAILURE(rv)) {
            _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, TRUE,
                                          BCM_MIRROR_PORT_EGRESS_TRUE);
        }
    }

    return (rv);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *     _bcm_esw_mirror_stacking_dest_update
 * Purpose:
 *      Update mirror_to bitmap for a system when stacking is enabled 
 * Parameters:
 *      unit         -  (IN) BCM device number.
 *      port         -  (IN) Port mirrored port.
 *      mirror_dest  -  (IN) Mirroring destination gport.
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_esw_mirror_stacking_dest_update(int unit, bcm_port_t port, 
                                     bcm_gport_t mirror_dest)
{
    int rv = BCM_E_NONE;
    bcm_module_t mymodid;       /* Local module id.                     */
    bcm_module_t rem_modid;     /* Remote module id.                    */
    bcm_port_t port_num;        /* Port number to get to rem_modid.     */
    bcm_pbmp_t pbmp;            /* Mirror destination bitmap.           */
    uint32 mirbmap;             /* Word 0 of mirror destination bitmap. */
    int idx;                    /* Trunk members iterator.              */
    int is_local_modid;         /* Check for local trunk port */

    if (SOC_IS_TD2_TT2(unit)) {
        imirror_bitmap_entry_t  entry;
        sal_memset(&entry, 0, sizeof(entry));

        if (mirror_dest != BCM_GPORT_INVALID) {
            if (BCM_GPORT_IS_TRUNK(mirror_dest)) {
                soc_mem_field32_set(unit, IMIRROR_BITMAPm, &entry,
                                    HG_TRUNK_IDf,
                                    BCM_GPORT_TRUNK_GET(mirror_dest));
                soc_mem_field32_set(unit, IMIRROR_BITMAPm, &entry, ISTRUNKf,
                                    1);
                soc_mem_field32_set(unit, IMIRROR_BITMAPm, &entry, ENABLEf, 1);
            } else {
                rem_modid = BCM_GPORT_MODPORT_MODID_GET(mirror_dest);
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_modid_is_local(unit, rem_modid,
                                             &is_local_modid));
                if (!is_local_modid) {
                    BCM_IF_ERROR_RETURN
                        (bcm_esw_stk_modport_get(unit, rem_modid,
                                                 &port_num));
                    soc_mem_field32_set(unit, IMIRROR_BITMAPm, &entry,
                                        EGRESS_PORTf, port_num);
                    soc_mem_field32_set(unit, IMIRROR_BITMAPm, &entry, ENABLEf,
                                        1);
                }
            }
        }
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, IMIRROR_BITMAPm, MEM_BLOCK_ANY, port,
                           &entry));
        if (IS_CPU_PORT(unit, port) && SOC_INFO(unit).cpu_hg_index != -1) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, IMIRROR_BITMAPm, MEM_BLOCK_ANY,
                               SOC_INFO(unit).cpu_hg_index, &entry));
        }
        return BCM_E_NONE;
    }

    /* Clear destination pbmp. */
    BCM_PBMP_CLEAR(pbmp);
    mirbmap = 0;

    /* 
     * Clear mirrorto bitmap if devices are not in draco mode 
     * or mirroring is off. 
     */ 
    if (MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit) &&
        (BCM_GPORT_INVALID != mirror_dest)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_local_modid_get(unit, &mymodid));

        if (BCM_GPORT_IS_TRUNK(mirror_dest)) {
            int member_count;
            bcm_trunk_member_t *member_array = NULL;
            bcm_module_t mod_out;
            bcm_port_t port_out;
            bcm_trunk_t tgid_out;
            int id_out;

            /* Get trunk member port/module pairs. */
            BCM_IF_ERROR_RETURN
                (bcm_esw_trunk_get(unit, BCM_GPORT_TRUNK_GET(mirror_dest),
                                    NULL, 0, NULL, &member_count));
            if (member_count > 0) {
                member_array = sal_alloc(sizeof(bcm_trunk_member_t) * member_count,
                        "trunk member array");
                if (NULL == member_array) {
                    return BCM_E_MEMORY;
                }
                rv = bcm_esw_trunk_get(unit, BCM_GPORT_TRUNK_GET(mirror_dest),
                        NULL, member_count, member_array, &member_count);
                if (BCM_FAILURE(rv)) {
                    sal_free(member_array);
                    return rv;
                }
            }

            /* Fill pbmp with trunk members from other modules . */
            for (idx = 0; idx < member_count; idx++) {
                rv = _bcm_esw_gport_resolve(unit, member_array[idx].gport,
                        &mod_out, &port_out, &tgid_out, &id_out);
                if (BCM_FAILURE(rv) || (-1 != tgid_out) || (-1 != id_out)) {
                    sal_free(member_array);
                    return rv;
                }
                rv = _bcm_esw_modid_is_local(unit, mod_out, &is_local_modid);
                if (BCM_FAILURE(rv)) {
                    sal_free(member_array);
                    return rv;
                }
                if (!is_local_modid) {
                    rv = bcm_esw_stk_modport_get(unit, mod_out, &port_num);
                    if (BCM_FAILURE(rv)) {
                        sal_free(member_array);
                        return rv;
                    }
                    /* Set local port used to reach remote module to pbmp. */
                    BCM_PBMP_PORT_SET(pbmp, port_num);
                }
            }

            if (NULL != member_array) {
                sal_free(member_array);
            }
        } else {
            rem_modid = BCM_GPORT_MODPORT_MODID_GET(mirror_dest);
            BCM_IF_ERROR_RETURN(
                _bcm_esw_modid_is_local(unit, rem_modid, &is_local_modid));
            if (!is_local_modid) {
                BCM_IF_ERROR_RETURN(bcm_esw_stk_modport_get(unit, rem_modid,
                                                            &port_num));
                /* Set local port used to reach remote module to pbmp. */
                BCM_PBMP_PORT_SET(pbmp, port_num);
            }
        }

        mirbmap = SOC_PBMP_WORD_GET(pbmp, 0);
        if (SOC_IS_FBX(unit)) {
            mirbmap >>= SOC_HG_OFFSET(unit);
        }
    }
#if defined(BCM_HERCULES_SUPPORT)
    if (SOC_IS_HERCULES(unit)) {
        return WRITE_ING_MIRTOBMAPr(unit, port, mirbmap);
    }
#endif /* BCM_HERCULES_SUPPORT */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (soc_feature(unit, soc_feature_egr_mirror_path)) {
#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_control_mem)) {
            imirror_bitmap_entry_t  entry;
            
            BCM_IF_ERROR_RETURN(soc_mem_read(unit, IMIRROR_BITMAPm, 
                                              MEM_BLOCK_ALL, port, &entry));
            soc_mem_pbmp_field_set(unit, IMIRROR_BITMAPm, &entry, BITMAPf, &pbmp);
            BCM_IF_ERROR_RETURN(soc_mem_write(unit, IMIRROR_BITMAPm, 
                                              MEM_BLOCK_ANY, port, &entry));
            if (IS_CPU_PORT(unit, port) &&
                (SOC_INFO(unit).cpu_hg_index != -1)) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, IMIRROR_BITMAPm, MEM_BLOCK_ANY, 
                                   SOC_INFO(unit).cpu_hg_index, &entry));
            }
        } else
#endif
#ifdef BCM_TRIUMPH2_SUPPORT
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit)) {
            uint32 values[2];
            soc_field_t fields[] = {BITMAP_LOf, BITMAP_HIf};
            values[0] = SOC_PBMP_WORD_GET(pbmp, 0);
            values[1] = SOC_PBMP_WORD_GET(pbmp, 1);
            BCM_IF_ERROR_RETURN
                (soc_reg_fields32_modify(unit, IMIRROR_BITMAP_64r, port,
                                         2, fields, values));
        } else
#endif /* BCM_TRIUMPH2_SUPPORT */
        {
#ifdef BCM_GREYHOUND2_SUPPORT
            if(soc_feature(unit, soc_feature_high_portcount_register)){
                uint64 mirbmap64;
                COMPILER_64_ZERO(mirbmap64);
                BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_LOr(unit, port, mirbmap64));
                BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_HIr(unit, port, mirbmap64));
                return (BCM_E_NONE);
            }else
#endif /* BCM_GREYHOUND2_SUPPORT */
            {
                return WRITE_IMIRROR_BITMAPr(unit, port, mirbmap);
            }
        } 
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _bcm_esw_mirror_port_ingress_dest_delete
 * Purpose:
 *      Delete ingress mirroring destination from a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_esw_mirror_port_ingress_dest_delete(int unit, bcm_port_t port, 
                                         bcm_gport_t mirror_dest) 
{
    int enable;              /* Mirror enable check.     */
    int mtp_index;           /* Mirror to port index.    */
    int rv;                  /* Operation return status. */

    /* Look for used MTP index */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        rv = _bcm_tr2_mirror_shared_mtp_match(unit, mirror_dest, FALSE, 
                                              &mtp_index);
    } else {
        rv = _bcm_esw_mirror_ingress_mtp_match(unit, mirror_dest, &mtp_index);
    }
    if (BCM_FAILURE(rv)) {
        return (BCM_E_NOT_FOUND);
    }

    if ((-1 != port) &&
        (BCM_GPORT_IS_SET(port) || SOC_PORT_VALID(unit, port))) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            /* Enable MTP index on mirror source port */
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_mirror_ingress_mtp_uninstall(unit, port, mtp_index));
        } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        {

            BCM_IF_ERROR_RETURN(bcm_esw_mirror_ingress_get(unit, port, &enable));
            if (!enable) {
                return (BCM_E_NOT_FOUND);
            }
            BCM_IF_ERROR_RETURN(bcm_esw_mirror_ingress_set(unit, port, FALSE));
        }
    }

    /* Free MTP index for mirror destination. */
    rv =  _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, TRUE, 
                                        BCM_MIRROR_PORT_INGRESS);

    return (rv);
}

/*
 * Function:
 *     _bcm_esw_mirror_port_egress_dest_delete
 * Purpose:
 *      Delete egress mirroring destination from a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_esw_mirror_port_egress_dest_delete(int unit, bcm_port_t port, 
                                        bcm_gport_t mirror_dest) 
{
    int enable;              /* Mirror enable check.     */
    int mtp_index;           /* Mirror to port index.    */
    int rv;                  /* Operation return status. */

    /* Look for used MTP index */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        rv = _bcm_tr2_mirror_shared_mtp_match(unit, mirror_dest, TRUE, 
                                              &mtp_index);
    } else {
        rv = _bcm_esw_mirror_egress_mtp_match(unit, mirror_dest, &mtp_index);
    }
    
    if (BCM_FAILURE(rv)) {
        return (BCM_E_NOT_FOUND);
    }

    if ((-1 != port) && 
        (BCM_GPORT_IS_SET(port) || SOC_PORT_VALID(unit, port))) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            /* Enable MTP index on mirror source port */
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_mirror_egress_mtp_uninstall(unit, port, mtp_index));
        } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        {
            BCM_IF_ERROR_RETURN(bcm_esw_mirror_egress_get(unit, port, &enable));
            if (!enable) {
                return (BCM_E_NONE);
            }
            BCM_IF_ERROR_RETURN(bcm_esw_mirror_egress_set(unit, port, FALSE));
        }
    }

    /* Free MTP index for mirror destination. */
    rv =  _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, TRUE, 
                                        BCM_MIRROR_PORT_EGRESS);

    return (rv);
}

#ifdef BCM_TRIUMPH2_SUPPORT
/*
 * Function:
 *     _bcm_esw_mirror_port_egress_true_dest_delete
 * Purpose:
 *      Delete egress true mirroring destination from a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_esw_mirror_port_egress_true_dest_delete(int unit, bcm_port_t port, 
                                        bcm_gport_t mirror_dest) 
{
    int mtp_index;           /* Mirror to port index.    */
    int rv;                  /* Operation return status. */

    if (MIRROR_CONFIG(unit)->egr_true_mtp_count == 0) {
        return BCM_E_NOT_FOUND;
    }

    /* Look for used MTP index */
    rv = _bcm_esw_mirror_egress_true_mtp_match(unit, mirror_dest,
                                               &mtp_index);
    if (BCM_FAILURE(rv)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Disable MTP index on mirror source port */
    if ((-1 != port) && SOC_PORT_VALID(unit, port)) {
        /* Enable MTP index on mirror source port */
        BCM_IF_ERROR_RETURN
            (_bcm_tr2_mirror_egress_true_mtp_uninstall(unit, port,
                                                        mtp_index));
    }

    /* Free MTP index for mirror destination. */
    rv =  _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, TRUE, 
                                        BCM_MIRROR_PORT_EGRESS_TRUE);

    return (rv);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

#if defined(BCM_FIELD_SUPPORT) && defined(BCM_XGS3_SWITCH_SUPPORT)
/*
 * Function:
 *     _bcm_esw_mirror_fp_dest_add 
 * Purpose:
 *      Add mirroring destination to field processor module. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      modid        -  (IN) Mirroring destination module.
 *      port         -  (IN) Mirroring destination port or GPORT. 
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mtp_index    -  (OUT) Allocated hw mtp index.
 * Returns:
 *      BCM_X_XXX
 */
int
_bcm_esw_mirror_fp_dest_add(int unit, int modid, int port, 
                            uint32 flags, int *mtp_index) 
{
    bcm_mirror_destination_t mirror_dest;  /* Mirror destination.          */
    bcm_gport_t     mirror_dest_id;  /* Mirror destination id.       */
    int             rv = BCM_E_NONE; /* Operation return status.     */
    uint32          destroy_flag = FALSE; /* mirror destination destroy */

    /* At least one packet direction must be specified. */
    if (!(flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS |
                   BCM_MIRROR_PORT_EGRESS_TRUE))) {
        return (BCM_E_PARAM);
    }

    /* Can't reserve multiple types of mtp in 1 shot. */
    if (((flags & BCM_MIRROR_PORT_INGRESS) &&
        (flags & (BCM_MIRROR_PORT_EGRESS | BCM_MIRROR_PORT_EGRESS_TRUE))) ||
        ((flags & BCM_MIRROR_PORT_EGRESS) &&
         (flags & BCM_MIRROR_PORT_EGRESS_TRUE))) {
        return (BCM_E_PARAM);
    }

    if (!soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return (BCM_E_PARAM);
    }

    /* Initialization check */
    if (!MIRROR_INIT(unit)) {
        return (BCM_E_INIT);
    }

    /* Create traditional mirror destination. */
    bcm_mirror_destination_t_init(&mirror_dest);

    if ((flags & BCM_MIRROR_PORT_EGRESS_TRUE) &&
        MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
        return (BCM_E_CONFIG);
    }

    MIRROR_LOCK(unit);

    if (BCM_GPORT_IS_MIRROR(port)) {
        rv = bcm_esw_mirror_destination_get(unit, port, &mirror_dest);
    } else {
        rv = _bcm_esw_mirror_destination_find(unit, port, modid, flags, &mirror_dest); 
        if (BCM_E_NOT_FOUND == rv) {
            mirror_dest.flags |= BCM_MIRROR_DEST_FIELD;
            rv = _bcm_esw_mirror_destination_create(unit, &mirror_dest);
            destroy_flag = TRUE;
        }       
    }
    if (BCM_FAILURE(rv)) {
        MIRROR_UNLOCK(unit);
        return rv;
    }
    mirror_dest_id = mirror_dest.mirror_dest_id;
    /* Single mirroring destination for ingress & egress. */
    if (MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
        if (BCM_GPORT_IS_TRUNK(mirror_dest.gport)) {
            if (destroy_flag) {
               (void)bcm_esw_mirror_destination_destroy(unit, 
                                                mirror_dest.mirror_dest_id); 
            }
            MIRROR_UNLOCK(unit);
            return (BCM_E_UNAVAIL);
        }

        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            if (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, 0) && 
                MIRROR_CONFIG_SHARED_MTP_DEST(unit, 0) != mirror_dest_id) {
                if (destroy_flag) {
                    (void)bcm_esw_mirror_destination_destroy(unit, 
                                         mirror_dest.mirror_dest_id); 
                }
                MIRROR_UNLOCK(unit);
                return (BCM_E_RESOURCE);
            }
        } else {
            if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)) {
                if ((MIRROR_CONFIG_ING_MTP_DEST(unit, 0) != mirror_dest_id) &&
                    (MIRROR_CONFIG_EGR_MTP_DEST(unit, 0) != mirror_dest_id)) {
                    if (destroy_flag) {
                       (void)bcm_esw_mirror_destination_destroy(unit, 
                                            mirror_dest.mirror_dest_id); 
                    }
                    MIRROR_UNLOCK(unit);
                    return (BCM_E_RESOURCE);
                }
            }
        }
    }
     

    /* Reserve & initialize mtp index based on traffic direction. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        rv = _bcm_xgs3_mirror_ingress_mtp_reserve(unit, mirror_dest_id, 
                                                  mtp_index);
    } else if (flags & BCM_MIRROR_PORT_EGRESS) {
        rv = _bcm_xgs3_mirror_egress_mtp_reserve(unit, mirror_dest_id,
                                                 FALSE, mtp_index);
#ifdef BCM_TRIUMPH2_SUPPORT
    } else if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        if (MIRROR_CONFIG(unit)->egr_true_mtp_count > 0) {
            rv = _bcm_xgs3_mirror_egress_true_mtp_reserve(unit, mirror_dest_id,
                                                      mtp_index);
        } else {
            rv = BCM_E_RESOURCE;
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    /* MTP slot reservation for FP's */
    if (BCM_SUCCESS(rv) &&
        soc_feature(unit, soc_feature_mirror_flexible) &&
        MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        int             mtp_slot;        /* Flexible mirroring slot */

        /* Determine a usable MTP slot for this FP entry */
        if (0 == (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
            rv = _bcm_xgs3_mtp_type_slot_reserve(unit, flags,
                                                 0, 0, /* Unused */
                                                 BCM_MTP_SLOT_TYPE_FP,
                                                 *mtp_index, &mtp_slot);
            if (BCM_SUCCESS(rv)) {
                *mtp_index |= (mtp_slot << BCM_MIRROR_MTP_FLEX_SLOT_SHIFT);
            }    
        } else {
            /* Egress true uses 1-1 mapping */
            *mtp_index |= (*mtp_index << BCM_MIRROR_MTP_FLEX_SLOT_SHIFT);
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Enable mirroring on a port.  */
    if (BCM_SUCCESS(rv)) { 
        if(!SOC_IS_XGS3_SWITCH(unit) || 
           (BCM_MIRROR_DISABLE == MIRROR_CONFIG_MODE(unit))) {
            rv = _bcm_esw_mirror_enable(unit);
            MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
        }
    } 

    if (BCM_FAILURE(rv) && destroy_flag) {
        (void)bcm_esw_mirror_destination_destroy(unit, mirror_dest.mirror_dest_id); 
    }
    MIRROR_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *     _bcm_esw_mirror_fp_dest_delete
 * Purpose:
 *      Delete fp mirroring destination.
 * Parameters:
 *      unit         -  (IN) BCM device number.
 *      mtp_index    -  (IN) Mirror destination index.
 *      flags        -  (IN) Mirror direction flags.
 * Returns:
 *      BCM_X_XXX
 * Notes: 
 */
int
_bcm_esw_mirror_fp_dest_delete(int unit, int mtp_index, uint32 flags)
{
    int rv = BCM_E_NONE;                      /* Operation return status. */
    bcm_mirror_destination_t mirror_dest;     /* Mirror destination.       */
    bcm_gport_t              mirror_dest_id;  /* Mirror destination id.    */

    mirror_dest_id = BCM_GPORT_INVALID;

    /* Input parameters check. */
    /* At least one packet direction must be specified. */
    if (!(flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS |
                   BCM_MIRROR_PORT_EGRESS_TRUE))) {
        return (BCM_E_PARAM);
    }

    /* Can't reserve multiple types of mtp in 1 shot. */
    if (((flags & BCM_MIRROR_PORT_INGRESS) &&
        (flags & (BCM_MIRROR_PORT_EGRESS | BCM_MIRROR_PORT_EGRESS_TRUE))) ||
        ((flags & BCM_MIRROR_PORT_EGRESS) &&
         (flags & BCM_MIRROR_PORT_EGRESS_TRUE))) {
        return (BCM_E_PARAM);
    }

    if (!soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return (BCM_E_PARAM);
    }

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    MIRROR_LOCK(unit);

    /* shifting the mtp_index back since flexible slot shift was done           
     * prior to this                                                            
     */                                                                         
    if (soc_feature(unit, soc_feature_mirror_flexible) &&                       
        MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {                         
        mtp_index &= BCM_MIRROR_MTP_FLEX_SLOT_MASK;                             
    }                                                                           
         
    if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
        mirror_dest_id = MIRROR_CONFIG_EGR_TRUE_MTP_DEST(unit, mtp_index);
        if (MIRROR_CONFIG(unit)->egr_true_mtp_count == 0) {
            rv = BCM_E_PARAM;
        }
    } else {
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                mirror_dest_id =
                    MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index);
        } else {
            if (flags & BCM_MIRROR_PORT_EGRESS) {
                mirror_dest_id = MIRROR_CONFIG_EGR_MTP_DEST(unit, mtp_index);
            }  else if (flags & BCM_MIRROR_PORT_INGRESS) {
                mirror_dest_id = MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index);
            } else {
                rv = BCM_E_PARAM;
            }

#ifdef BCM_TRIUMPH2_SUPPORT
            if (BCM_SUCCESS(rv) &&
                soc_feature(unit, soc_feature_mirror_flexible) &&
                MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
                rv = _bcm_xgs3_mtp_type_slot_unreserve(unit, flags,
                                                       0, /* Unused */
                                                       BCM_MTP_SLOT_TYPE_FP,
                                                       mtp_index);
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
        }
    }

    if (mirror_dest_id == BCM_GPORT_INVALID) {
        rv = _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, FALSE, flags);
        MIRROR_UNLOCK(unit);
        return rv;
    }

    /* Get mirror destination descriptor. */
    if (BCM_SUCCESS(rv)) {
        rv = bcm_esw_mirror_destination_get(unit, mirror_dest_id,
                                            &mirror_dest);
    }

    /* Free MTP index for mirror destination. */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, FALSE, flags);
    }

    /* Destroy mirror destination if it was created by fp action add. */ 
    if (BCM_SUCCESS(rv)) {
        if ((mirror_dest.flags & BCM_MIRROR_DEST_FIELD) &&
            (1 >= MIRROR_DEST_REF_COUNT(unit, mirror_dest.mirror_dest_id))) {
            rv = bcm_esw_mirror_destination_destroy(unit,
                                            mirror_dest.mirror_dest_id);
        }
    }

    MIRROR_UNLOCK(unit);
    return(rv);
}

/*
 * Function:
 *     _bcm_esw_mirror_fp_slot_add_ref
 * Purpose:
 *      Add FP Slot Reference in TRX devices not supporting
 *      flexible mirroring.
 * Parameters:
 *      unit               -  (IN) BCM device number.
 *      flags              -  (IN) _BCM_MIRROR_SLOT_XXX flags.
 *      mtp_slot           -  (IN) MTP Slot Container.
 * Returns:
 *      BCM_X_XXX
 * Notes:
 *      Should not be called for devices supporting flexible mirroriing.
 */
int
_bcm_esw_mirror_fp_slot_add_ref(int unit, uint32 flags, uint32 mtp_slot)
{
    /* Initialization check */
    if (!MIRROR_INIT(unit)) {
        return (BCM_E_INIT);
    }

    if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {

        /* Take Mirror Lock */
        MIRROR_LOCK(unit);

        /* At least one of valid flag should be set */
        if (!(flags & (_BCM_MIRROR_SLOT_FP | _BCM_MIRROR_SLOT_INGRESS |
                        _BCM_MIRROR_SLOT_EGRESS))) {
            return (BCM_E_PARAM);
        }

        /* Request cannot come from port */
        if (flags & _BCM_MIRROR_SLOT_PORT) {
            return (BCM_E_PARAM);
        }

        /* Ingress Slot Reserve From FP module */
        if ((flags & _BCM_MIRROR_SLOT_INGRESS) && (flags & _BCM_MIRROR_SLOT_FP)) {
            MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot)++;
            if (MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot)) {
                MIRROR_CONFIG_ING_MTP_SLOT_OWNER(unit, mtp_slot) |=
                    _BCM_MIRROR_SLOT_OWNER_FP;
            }
        }

        /* Egress Slot Reserve From FP module */
        if ((flags & _BCM_MIRROR_SLOT_EGRESS) && (flags & _BCM_MIRROR_SLOT_FP)) {
            MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot)++;
            if (MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot)) {
                MIRROR_CONFIG_EGR_MTP_SLOT_OWNER(unit, mtp_slot) |=
                    _BCM_MIRROR_SLOT_OWNER_FP;
            }
        }

        /* Release Mirror Lock */
        MIRROR_UNLOCK(unit);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_esw_mirror_fp_slot_del_ref
 * Purpose:
 *      Delete FP Slot Reference in TRX devices not supporting
 *      flexible mirroring.
 * Parameters:
 *      unit               -  (IN) BCM device number.
 *      flags              -  (IN) _BCM_MIRROR_SLOT_XXX flags.
 *      mtp_slot_map       -  (IN) MTP_SLOT_MAP.
 * Returns:
 *      BCM_X_XXX
 * Notes:
 *      Should not be called for devices supporting flexible mirroriing.
 */
int
_bcm_esw_mirror_fp_slot_del_ref(int unit, uint32 flags, uint32 mtp_slot_map)
{
    int mtp_slot = 0;

    /* Initialization check */
    if (!MIRROR_INIT(unit)) {
        return (BCM_E_INIT);
    }

    if (MIRROR_SWITCH_IS_EXCLUSIVE(unit)) {

        /* Take Mirror Lock */
        MIRROR_LOCK(unit);

        /* At least one of valid flag should be set */
        if (!(flags & (_BCM_MIRROR_SLOT_FP | _BCM_MIRROR_SLOT_INGRESS |
                        _BCM_MIRROR_SLOT_EGRESS))) {
            return (BCM_E_PARAM);
        }

        /* Request cannot come from port */
        if (flags & _BCM_MIRROR_SLOT_PORT) {
            return (BCM_E_PARAM);
        }

        /* Ingress Slot Release From FP module */
        if ((flags & _BCM_MIRROR_SLOT_INGRESS) && (flags & _BCM_MIRROR_SLOT_FP)) {
            if (mtp_slot_map & 0x1) {
                mtp_slot = 0;
                if (MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot)) {
                    MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot)--;
                    if (MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot) == 0) {
                        MIRROR_CONFIG_ING_MTP_SLOT_OWNER(unit, mtp_slot) &=
                            ~(_BCM_MIRROR_SLOT_OWNER_FP);
                    }
                }
            }
            if (mtp_slot_map & 0x2) {
                mtp_slot = 1;
                if (MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot)) {
                    MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot)--;
                    if (MIRROR_CONFIG_ING_MTP_SLOT_REF(unit, mtp_slot) == 0) {
                        MIRROR_CONFIG_ING_MTP_SLOT_OWNER(unit, mtp_slot) &=
                            ~(_BCM_MIRROR_SLOT_OWNER_FP);
                    }
                }
            }
        }

        /* Egress Slot Release From FP module */
        if ((flags & _BCM_MIRROR_SLOT_EGRESS) && (flags & _BCM_MIRROR_SLOT_FP)) {
            if (mtp_slot_map & 0x1) {
                mtp_slot = 0;
                if (MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot)) {
                    MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot)--;
                    if (MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot) == 0) {
                        MIRROR_CONFIG_EGR_MTP_SLOT_OWNER(unit, mtp_slot) &=
                            ~(_BCM_MIRROR_SLOT_OWNER_FP);
                    }
                }
            }
            if (mtp_slot_map & 0x2) {
                mtp_slot = 1;
                if (MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot)) {
                    MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot)--;
                    if (MIRROR_CONFIG_EGR_MTP_SLOT_REF(unit, mtp_slot) == 0) {
                        MIRROR_CONFIG_EGR_MTP_SLOT_OWNER(unit, mtp_slot) &=
                            ~(_BCM_MIRROR_SLOT_OWNER_FP);
                    }
                }
            }
        }
        /* Release Mirror Lock */
        MIRROR_UNLOCK(unit);
    }
    return BCM_E_NONE;
}

#endif /* BCM_FIELD_SUPPORT  && BCM_XGS3_SWITCH_SUPPORT */

/*
 * Function:
 *	    _bcm_esw_mirror_enable
 * Purpose:
 *	    Set mirror enable = TRUE on all ports.
 * Parameters:
 *	    unit - (IN) BCM device number
 * Returns:
 *   	BCM_E_XXX
 * Notes:
 *      When egress or fp mirroring is enabled, we need to enable 
 *      mirroring on all ports even if mirroring is only explicitely
 *      enabled on a single port. This function ensures that the mirror
 *      enable bit is toggled correctly on all ports, when
 *      bcm_mirror_port_dest_xxx style apis are used by application.
 */
STATIC int
_bcm_esw_mirror_enable(int unit)
{
    bcm_port_t port;
    bcm_pbmp_t all_pbmp;

    BCM_PBMP_CLEAR(all_pbmp);
    BCM_PBMP_ASSIGN(all_pbmp, PBMP_ALL(unit));
#ifdef BCM_KATANA2_SUPPORT
    if (SOC_IS_KATANA2(unit) && soc_feature(unit, soc_feature_flex_port)) {
        BCM_IF_ERROR_RETURN(_bcm_kt2_flexio_pbmp_update(unit, &all_pbmp));
    }
    if (soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe)) {
        _bcm_kt2_subport_pbmp_update(unit, &all_pbmp);
    }
#endif
    PBMP_ITER(all_pbmp, port) {
        BCM_IF_ERROR_RETURN(_bcm_esw_mirror_enable_set(unit, port, TRUE));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *	_bcm_esw_mirror_port_dest_search
 * Purpose:
 *	Search to see if the the given port, destination exists.
 * Parameters:
 *	unit         - (IN) BCM device number
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *   	BCM_E_XXX
 * Notes:
 *      This should be called with only one of the
 *      INGRESS/EGRESS/TRUE_EGRESS flags set.
 */
STATIC int
_bcm_esw_mirror_port_dest_search(int unit, bcm_port_t port, 
                                 uint32 flags, bcm_gport_t mirror_dest)
{
    bcm_gport_t mirror_dest_list[BCM_MIRROR_MTP_COUNT];
    int mirror_dest_count, mtp;

    BCM_IF_ERROR_RETURN
        (bcm_esw_mirror_port_dest_get(unit, port, flags,
                                      BCM_MIRROR_MTP_COUNT,
                                      mirror_dest_list, &mirror_dest_count));

    for (mtp = 0; mtp < mirror_dest_count; mtp++) {
        if (mirror_dest_list[mtp] == mirror_dest) {
            return BCM_E_EXISTS;
        }
    }

    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *	  bcm_esw_mirror_deinit
 * Purpose:
 *	  Deinitialize mirror software module.
 * Parameters:
 *    unit - (IN) BCM device number.
 * Returns:
 *	  BCM_E_XXX
 */
int
bcm_esw_mirror_deinit(int unit)
{
#ifdef BCM_SHADOW_SUPPORT
    if (soc_feature(unit, soc_feature_no_mirror)) {
        return BCM_E_NONE;
    }
#endif
    /* Call internal sw structures clean up routine. */
    return _bcm_esw_mirror_deinit(unit, &MIRROR_CONFIG(unit));
}

/*
 * Function:
 *	  bcm_esw_mirror_init
 * Purpose:
 *	  Initialize mirror software system.
 * Parameters:
 *    unit - (IN) BCM device number.
 * Returns:
 *	  BCM_E_XXX
 */
int
bcm_esw_mirror_init(int unit)
{
    _bcm_mirror_config_p mirror_cfg_ptr;/* Mirror module config structue. */
    bcm_mirror_destination_t *mdest;    /* Mirror destinations iterator.  */
    int directed;                       /* Directed mirroring enable.     */
    int alloc_sz;                       /* Memory allocation size.        */
    int idx;                            /* MTP iteration index.           */
    int rv;                             /* Operation return status.       */
#if defined(BCM_METROLITE_SUPPORT)
    uint16 dev_id;
    uint8 rev_id;
#endif
#ifdef BCM_WARM_BOOT_SUPPORT
    uint8 *mtp_scache;
    int mtp_num;                        /* Maximum number of MTP dests    */
    uint8 *mirror_method_scache;
    soc_scache_handle_t scache_handle;  /* SCache reference number        */
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
    if (soc_feature(unit, soc_feature_no_mirror)) {
        return BCM_E_NONE;
    }
#endif

    /* Deinitialize the module if it was previously initialized. */
    if (NULL != MIRROR_CONFIG(unit)) {
        _bcm_esw_mirror_deinit(unit, &MIRROR_CONFIG(unit));
    }

    /* Allocate mirror config structure. */
    alloc_sz = sizeof(_bcm_mirror_config_t);
    mirror_cfg_ptr = sal_alloc(alloc_sz, "Mirror module");
    if (NULL == mirror_cfg_ptr) {
        return (BCM_E_MEMORY);
    }
    sal_memset(mirror_cfg_ptr, 0, alloc_sz);

    rv = _bcm_esw_directed_mirroring_get(unit, &directed);
    if (BCM_FAILURE(rv)) {
        _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
        return(BCM_E_INTERNAL);
    }
#if defined(BCM_METROLITE_SUPPORT)
    soc_cm_get_id(unit, &dev_id, &rev_id);
#endif
#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        rv = _bcm_esw_mirror_method_reinit(unit);
        if (BCM_FAILURE(rv)) {
            _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
            return(rv);
        }
    }
#endif
    if (directed) {
        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            if (_bcm_mirror_mtp_method_init[unit] ==
                BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE) {
                mirror_cfg_ptr->mtp_method =
                    BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE;
            } else {
                mirror_cfg_ptr->mtp_method =
                    BCM_MIRROR_MTP_METHOD_DIRECTED_LOCKED;
            }
        } else {
            mirror_cfg_ptr->mtp_method =
                BCM_MIRROR_MTP_METHOD_DIRECTED_LOCKED;
        }
    } else {
        mirror_cfg_ptr->mtp_method = BCM_MIRROR_MTP_METHOD_NON_DIRECTED;
    }
    _bcm_mirror_mtp_method_init[unit] =
        mirror_cfg_ptr->mtp_method;


#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        soc_mem_t mem;
        soc_mem_t mems[3];
        int mem_words[3];
        int mems_cnt;
#ifdef BCM_TRIDENT_SUPPORT
        egr_port_entry_t egr_port_entry;
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_GREYHOUND_SUPPORT
        uint64 reg_val;
#endif /* BCM_GREYHOUND_SUPPORT */
        int port;
        bcm_pbmp_t all_pbmp;

        mem = EGR_MIRROR_ENCAP_CONTROLm;
        if (SOC_MEM_IS_VALID(unit, mem)) {
            if (NULL == EGR_MIRROR_ENCAP(unit)) {
                EGR_MIRROR_ENCAP(unit) =
                    sal_alloc(sizeof(soc_profile_mem_t),
                              "EGR_MIRROR_ENCAP Profile Mems");
                if (NULL == EGR_MIRROR_ENCAP(unit)) {
                    _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
                    return BCM_E_MEMORY;
                }
                soc_profile_mem_t_init(EGR_MIRROR_ENCAP(unit));

                mems_cnt = 0;
                mems[mems_cnt] = mem;
                mem_words[mems_cnt] =
                    sizeof(egr_mirror_encap_control_entry_t) /
                    sizeof(uint32);
                mems_cnt++;

                mem = EGR_MIRROR_ENCAP_DATA_1m;
                if (SOC_MEM_IS_VALID(unit, mem)) {
                    mems[mems_cnt] = mem;
                    mem_words[mems_cnt] =
                        sizeof(egr_mirror_encap_data_1_entry_t) /
                        sizeof(uint32);
                    mems_cnt++;
                }

                mem = EGR_MIRROR_ENCAP_DATA_2m;
                if (SOC_MEM_IS_VALID(unit, mem)) {
                    mems[mems_cnt] = mem;
                    mem_words[mems_cnt] =
                        sizeof(egr_mirror_encap_data_2_entry_t) /
                        sizeof(uint32);
                    mems_cnt++;
                }
                rv = soc_profile_mem_create(unit, mems, mem_words,
                                            mems_cnt,
                                      EGR_MIRROR_ENCAP(unit));
                if (rv < 0) {
                    _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
                    return rv;
                }

                BCM_PBMP_CLEAR(all_pbmp);
                BCM_PBMP_ASSIGN(all_pbmp, PBMP_ALL(unit));
#ifdef BCM_KATANA2_SUPPORT
                if (SOC_IS_KATANA2(unit) &&
                    soc_feature(unit, soc_feature_flex_port)) {
                    BCM_IF_ERROR_RETURN(
                        _bcm_kt2_flexio_pbmp_update(unit, &all_pbmp));
                }
                if (soc_feature(unit, soc_feature_linkphy_coe) ||
                    soc_feature(unit, soc_feature_subtag_coe)) {
                    _bcm_kt2_subport_pbmp_update(unit, &all_pbmp);
                }
#endif
                PBMP_ITER(all_pbmp, port) { /* Intialize the Encap index */
#ifdef BCM_GREYHOUND_SUPPORT
                    if (SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) ||
                        SOC_IS_GREYHOUND2(unit)) {
                        rv = READ_EGR_PORT_64r(unit, port, &reg_val);
            
                        if (BCM_SUCCESS(rv)) {
                            soc_reg64_field32_set(unit, EGR_PORT_64r, &reg_val, 
                                                  MIRROR_ENCAP_ENABLEf, 0);
                            soc_reg64_field32_set(unit, EGR_PORT_64r, &reg_val, 
                                                  MIRROR_ENCAP_INDEXf, 0);
                            rv = WRITE_EGR_PORT_64r(unit, port, reg_val);
                        }
                    } else
#endif /* BCM_GREYHOUND_SUPPORT */
                    {
#ifdef BCM_TRIDENT_SUPPORT
                    rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);
                    if (BCM_SUCCESS(rv)) {
                        soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                                  MIRROR_ENCAP_ENABLEf, 0);
                        soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                                  MIRROR_ENCAP_INDEXf, 0);
                        rv = WRITE_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);
                    }
#endif /* BCM_TRIDENT_SUPPORT */
                    }
                }
            }
        }
    }
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */


#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        mirror_cfg_ptr->egr_true_mtp_count = BCM_MIRROR_MTP_COUNT;
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_METROLITE_SUPPORT)
    /* No of MTP supported for 53460 and 53461 is
     * one for ingress and one for egress and
     * flexible mirroring is not supported  */
    if( (dev_id == BCM53460_DEVICE_ID) ||
        (dev_id == BCM53461_DEVICE_ID) ) {
        mirror_cfg_ptr->egr_mtp_count = 1;
        mirror_cfg_ptr->ing_mtp_count = 1;
    } else
#endif
    {
        mirror_cfg_ptr->egr_mtp_count = BCM_MIRROR_MTP_COUNT;
        mirror_cfg_ptr->ing_mtp_count = BCM_MIRROR_MTP_COUNT;
    }

#ifdef BCM_WARM_BOOT_SUPPORT 
    /* Determine maximum number used in any config */
    mtp_num =
        mirror_cfg_ptr->egr_mtp_count + mirror_cfg_ptr->ing_mtp_count;
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
       mtp_num += mirror_cfg_ptr->egr_true_mtp_count;
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    alloc_sz = sizeof(bcm_gport_t) * mtp_num;
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        (mirror_cfg_ptr->mtp_method == BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE)) {
        /* Slot-MTP mapping information for respective slot types */
        alloc_sz += (sizeof(bcm_gport_t) * BCM_MIRROR_MTP_COUNT) * BCM_MTP_SLOT_TYPES;
        /* Reference counter for respective slot types */
        alloc_sz += (sizeof(int) * BCM_MIRROR_MTP_COUNT) * BCM_MTP_SLOT_TYPES;
        /* BCM_MIRROR_DEST_FIELD flag */
        alloc_sz += sizeof(uint16);
        /* Reference counter for MTP slot */
        alloc_sz += sizeof(int) * BCM_MIRROR_MTP_COUNT;
        /* For ING MTP reference counter */
        alloc_sz += sizeof(int) * (mirror_cfg_ptr->ing_mtp_count);
        /* For ING MTP reference counter */
        alloc_sz += sizeof(int) * (mirror_cfg_ptr->egr_mtp_count);
        /* For destination reference counter */
        alloc_sz += sizeof(int) * (mirror_cfg_ptr->egr_mtp_count +
                                   mirror_cfg_ptr->ing_mtp_count);
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            alloc_sz += sizeof(int) * mirror_cfg_ptr->egr_true_mtp_count;
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        /* For pbmp_mtp_slot_used */
        alloc_sz += sizeof(bcm_pbmp_t) * BCM_MIRROR_MTP_COUNT;
        /* For ING MTP destination flags */
        alloc_sz += sizeof(uint32) * (mirror_cfg_ptr->ing_mtp_count);
        /* For ING MTP destination flags */
        alloc_sz += sizeof(uint32) * (mirror_cfg_ptr->egr_mtp_count);
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            alloc_sz += sizeof(uint32) * (mirror_cfg_ptr->egr_true_mtp_count);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    }

    /* For non-flexible mtp destination flags */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        (mirror_cfg_ptr->mtp_method != BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE)) {
        /* For Shared MTP destination flags */
        alloc_sz += sizeof(uint32) * BCM_MIRROR_MTP_COUNT;
        /* For egress true MTP destination flags */
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            alloc_sz += sizeof(uint32) * (mirror_cfg_ptr->egr_true_mtp_count);
        }
        /* For Shared MTP ref_count */
        alloc_sz += sizeof(int) * BCM_MIRROR_MTP_COUNT;
        /* For Egress True MTP ref_count */
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            alloc_sz += sizeof(int) * (mirror_cfg_ptr->egr_true_mtp_count);
        }
        /* For Mirror Destination ref_count */
        alloc_sz += sizeof(int) * BCM_MIRROR_MTP_COUNT;
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            alloc_sz += sizeof(int) * (mirror_cfg_ptr->egr_true_mtp_count);
        }
        /* For MIRROR_CONFIG_MODE */
        alloc_sz += sizeof(int);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIDENT2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        if (mirror_cfg_ptr->mtp_method ==
            BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE) {
            /* For Ingress,Egress MTP destination gport */
            alloc_sz += sizeof(bcm_gport_t) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                        (mirror_cfg_ptr->ing_mtp_count +
                         mirror_cfg_ptr->egr_mtp_count);
            /* For Ingress,Egress MTP destination flags */
            alloc_sz += sizeof(uint32) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                        (mirror_cfg_ptr->ing_mtp_count +
                         mirror_cfg_ptr->egr_mtp_count);
        } else {
            /* For Shared MTP destination gport */
            alloc_sz += sizeof(bcm_gport_t) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                        BCM_MIRROR_MTP_COUNT;
            /* For Shared MTP destination flags */
            alloc_sz += sizeof(uint32) * BCM_SWITCH_TRUNK_MAX_PORTCNT *
                        BCM_MIRROR_MTP_COUNT;
        }
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    SOC_SCACHE_HANDLE_SET(scache_handle,
                          unit, BCM_MODULE_MIRROR, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle,
                                 (0 == SOC_WARM_BOOT(unit)),
                                 alloc_sz,
                                 &mtp_scache, BCM_WB_DEFAULT_VERSION, NULL);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
        return rv;
    }
    /* Allocate space for mirror_method */
    alloc_sz = sizeof(_bcm_mirror_mtp_method_init[unit]);
    SOC_SCACHE_HANDLE_SET(scache_handle,
                          unit, BCM_MODULE_MIRROR, 1);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle,
                                 (0 == SOC_WARM_BOOT(unit)),
                                 alloc_sz,
                                 &mirror_method_scache, BCM_WB_DEFAULT_VERSION, NULL);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
        return rv;
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    if (!directed) {
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            mirror_cfg_ptr->egr_true_mtp_count = 0;
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        mirror_cfg_ptr->egr_mtp_count = 1;
        mirror_cfg_ptr->ing_mtp_count = 1;
    } else if (!soc_feature(unit, soc_feature_mirror_flexible)
               && SOC_IS_TRX(unit)) {
        /* Limited egress mirroring bitmap registers */
        mirror_cfg_ptr->egr_mtp_count = 2;
        
        if (SOC_IS_ENDURO(unit)) {
            /* Limited ingress mirroring bitmap registers */
            mirror_cfg_ptr->ing_mtp_count = 2;
        }
    }

    if (!directed || soc_feature(unit, soc_feature_mirror_flexible)) {
        mirror_cfg_ptr->port_em_mtp_count = mirror_cfg_ptr->egr_mtp_count;
        mirror_cfg_ptr->port_im_mtp_count = mirror_cfg_ptr->ing_mtp_count;
    } else {
        if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANEX(unit) &&
            !SOC_IS_GREYHOUND(unit) && !SOC_IS_GREYHOUND2(unit)) {
            mirror_cfg_ptr->port_em_mtp_count = 2;
            mirror_cfg_ptr->port_im_mtp_count = 2;
        } else {
            mirror_cfg_ptr->port_em_mtp_count = 1;
            mirror_cfg_ptr->port_im_mtp_count = 1;
        }
    }
    if (!directed) {
        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            mirror_cfg_ptr->mtp_dev_mask = (BCM_MIRROR_MTP_ONE |
                                            BCM_MIRROR_MTP_THREE);
        } else {
            /* Only one MTP permitted in non-directed mode */
            mirror_cfg_ptr->mtp_dev_mask = BCM_XGS3_MIRROR_MTP_MASK;
        }
    } else if (soc_feature(unit, soc_feature_mirror_flexible)) {
        mirror_cfg_ptr->mtp_dev_mask = BCM_TR2_MIRROR_MTP_MASK;
    } else if (SOC_IS_HURRICANEX(unit) || SOC_IS_GREYHOUND(unit) ||
                SOC_IS_GREYHOUND2(unit)) {
        mirror_cfg_ptr->mtp_dev_mask = BCM_XGS3_MIRROR_MTP_MASK;
    } else if (SOC_IS_TRX(unit)) {
        mirror_cfg_ptr->mtp_dev_mask = BCM_TRX_MIRROR_MTP_MASK;
    } else {
        mirror_cfg_ptr->mtp_dev_mask = BCM_XGS3_MIRROR_MTP_MASK;
    }
#if defined(BCM_METROLITE_SUPPORT)
    if( (dev_id == BCM53460_DEVICE_ID) ||
                (dev_id == BCM53461_DEVICE_ID) ) {
        mirror_cfg_ptr->egr_mtp_count =1;
        mirror_cfg_ptr->ing_mtp_count = 1;
        mirror_cfg_ptr->port_em_mtp_count = 1;
        mirror_cfg_ptr->port_im_mtp_count = 1;
        mirror_cfg_ptr->mtp_dev_mask = BCM_XGS3_MIRROR_MTP_MASK;
    }
#endif
        /* Allocate mirror destinations structure. */
    if (!directed) {
        mirror_cfg_ptr->dest_count = 1;
    } else if (soc_feature(unit, soc_feature_mirror_flexible) &&
               (BCM_MIRROR_MTP_METHOD_DIRECTED_LOCKED ==
                mirror_cfg_ptr->mtp_method)) {
        mirror_cfg_ptr->dest_count = BCM_MIRROR_MTP_COUNT;
    } else {
        mirror_cfg_ptr->dest_count = 
            (mirror_cfg_ptr->egr_mtp_count + mirror_cfg_ptr->ing_mtp_count);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true) && (directed)) {
        mirror_cfg_ptr->dest_count += mirror_cfg_ptr->egr_true_mtp_count;
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
    alloc_sz = 
        mirror_cfg_ptr->dest_count * sizeof(_bcm_mirror_dest_config_t);

    mirror_cfg_ptr->dest_arr = sal_alloc(alloc_sz, "Mirror destinations");
    if (NULL == mirror_cfg_ptr->dest_arr) {
        _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
        return (BCM_E_MEMORY);
    }
    sal_memset(mirror_cfg_ptr->dest_arr, 0, alloc_sz);
    for (idx = 0; idx < mirror_cfg_ptr->dest_count; idx++) {
        mdest = &mirror_cfg_ptr->dest_arr[idx].mirror_dest;
        BCM_GPORT_MIRROR_SET(mdest->mirror_dest_id, idx);
    }

    /* Allocate mirror destinations structure. */
    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        (BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE !=
                mirror_cfg_ptr->mtp_method)) {
        alloc_sz = BCM_MIRROR_MTP_COUNT * sizeof(_bcm_mtp_config_t);
        mirror_cfg_ptr->shared_mtp = sal_alloc(alloc_sz, "Shared MTP indexes");
        if (NULL == mirror_cfg_ptr->shared_mtp) {
            _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
            return (BCM_E_MEMORY);
        }
        sal_memset(mirror_cfg_ptr->shared_mtp, 0, alloc_sz);
    } else {
        /* Allocate egress mirror destinations structure. */
        alloc_sz = mirror_cfg_ptr->egr_mtp_count * sizeof(_bcm_mtp_config_t);
        mirror_cfg_ptr->egr_mtp  = sal_alloc(alloc_sz, "Egress MTP indexes");
        if (NULL == mirror_cfg_ptr->egr_mtp) {
            _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
            return (BCM_E_MEMORY);
        }
        sal_memset(mirror_cfg_ptr->egr_mtp, 0, alloc_sz);

        /* Allocate ingress mirror destinations structure. */
        alloc_sz = mirror_cfg_ptr->ing_mtp_count * sizeof(_bcm_mtp_config_t);
        mirror_cfg_ptr->ing_mtp  = sal_alloc(alloc_sz, "Ingress MTP indexes");
        if (NULL == mirror_cfg_ptr->ing_mtp) {
            _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
            return (BCM_E_MEMORY);
        }
        sal_memset(mirror_cfg_ptr->ing_mtp, 0, alloc_sz);
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        /* Allocate egress true mirror destinations structure. */
        alloc_sz =
            mirror_cfg_ptr->egr_true_mtp_count * sizeof(_bcm_mtp_config_t);
        mirror_cfg_ptr->egr_true_mtp  = sal_alloc(alloc_sz,
                                                  "Egress true MTP indexes");
        if (NULL == mirror_cfg_ptr->egr_true_mtp) {
            _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
            return (BCM_E_MEMORY);
        }
        sal_memset(mirror_cfg_ptr->egr_true_mtp, 0, alloc_sz);
    }

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        (BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE ==
        mirror_cfg_ptr->mtp_method)) {
        int mtp_type;

        for (mtp_type = BCM_MTP_SLOT_TYPE_PORT;
             mtp_type < BCM_MTP_SLOT_TYPES; mtp_type++) {
            /* Allocate MTP types records. */
            mirror_cfg_ptr->mtp_slot_count[mtp_type] = 4;

            alloc_sz = mirror_cfg_ptr->mtp_slot_count[mtp_type] *
                sizeof(_bcm_mtp_config_t);
            mirror_cfg_ptr->mtp_slot[mtp_type]  = sal_alloc(alloc_sz,
                                                      "Typed MTP indexes");
            if (NULL == mirror_cfg_ptr->mtp_slot[mtp_type]) {
                _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
                return (BCM_E_MEMORY);
            }
            sal_memset(mirror_cfg_ptr->mtp_slot[mtp_type], 0, alloc_sz);
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Ingress/Egress Slot Reference Containers for devices
       not supporting flex mirroring */
    if(_bcm_switch_mirror_exclusive_config[unit] == _BCM_SWITCH_MIRROR_EXCLUSIVE) {
        int mtp_slot_cnt = 0;
        int mtp_slot = 0;

        /* Count Number of Slot Container */
        BCM_MIRROR_MTP_ITER(mirror_cfg_ptr->mtp_dev_mask, mtp_slot) {
            mtp_slot_cnt++;
        }

        /* Allocate Ing/Egr Slot Ref Containers */
        alloc_sz = mtp_slot_cnt * sizeof(_bcm_mtp_slot_config_t);

        mirror_cfg_ptr->ing_slot_container =
            sal_alloc(alloc_sz, "Ingress Slot Container");

        mirror_cfg_ptr->egr_slot_container =
            sal_alloc(alloc_sz, "Egress Slot Container");

        if ((NULL == mirror_cfg_ptr->ing_slot_container) ||
            (NULL == mirror_cfg_ptr->egr_slot_container)) {
            _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
            return (BCM_E_MEMORY);
        }
        sal_memset(mirror_cfg_ptr->ing_slot_container, 0, alloc_sz);
        sal_memset(mirror_cfg_ptr->egr_slot_container, 0, alloc_sz);
    }

    /* Create protection mutex. */
    mirror_cfg_ptr->mutex = sal_mutex_create("Meter module mutex");
    if (NULL == mirror_cfg_ptr->mutex) {
        _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
        return (BCM_E_MEMORY);
    } 

    /* Take protection mutex for initial state setting & hw clear. */
    sal_mutex_take(mirror_cfg_ptr->mutex, sal_mutex_FOREVER);

    MIRROR_CONFIG(unit) = mirror_cfg_ptr;

#ifdef BCM_WARM_BOOT_SUPPORT 
    if (SOC_WARM_BOOT(unit)) {
        /* Reload mirror configuration info from HW */
        rv = _bcm_esw_mirror_reload(unit, directed);
    } else 
#endif /* BCM_WARM_BOOT_SUPPORT */
    {
        /* Clear memories/registers. */
        rv  = _bcm_esw_mirror_hw_clear(unit);
    }

    if (BCM_FAILURE(rv)) {
        MIRROR_UNLOCK(unit);
        _bcm_esw_mirror_deinit(unit, &mirror_cfg_ptr);
        MIRROR_CONFIG(unit) = NULL;
        return (BCM_E_FAIL);
    }

    MIRROR_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *        _bcmi_esw_mirror_flex_port_init
 * Purpose:
 *        Initialize mirror configuration on a port which is
 *        being added/removed(flexing) in the runtime.
 * Parameters:
 *    unit - (IN) BCM device number.
 *    port - local port number
 *    enable - TRUE when adding a port, FALSE when removing a port
 * Returns:
 *        BCM_E_XXX
 * Note:
 * apply the same code to both port attach and detach
 * attach: have a default state for mirror packet encap configuration
 * detach: clear to prevent misleading warmboot during hardware recovery
 */

STATIC int
_bcmi_esw_mirror_flex_port_init (int unit, bcm_port_t port, int enable)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    mirror_control_entry_t mc_entry;
    uint32 mc_reg;
#endif /* BCM_TRIUMPH2_SUPPORT */
    int mtp_index;
    bcm_pbmp_t pbmp;
    int mode;            /* mirroring mode. */
    int rv = BCM_E_NONE;

#if defined(BCM_TRIUMPH2_SUPPORT)
    /* Initialize default mirror control settings */
    if (soc_feature(unit, soc_feature_mirror_flexible) ||
        (soc_feature(unit, soc_feature_egr_mirror_true))) {
        if (soc_feature(unit, soc_feature_mirror_control_mem)) {
            sal_memset(&mc_entry, 0, sizeof(mc_entry));
            for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT;
                 mtp_index++) {
                soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                                _mtp_index_field[mtp_index],
                                enable? mtp_index : 0);
                soc_mem_field32_set(unit, MIRROR_CONTROLm, &mc_entry,
                                _non_uc_mtp_index_field[mtp_index],
                                enable? mtp_index: 0);
            }
        } else {
            mc_reg = 0;
            for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT;
                     mtp_index++) {
                soc_reg_field_set(unit, MIRROR_CONTROLr, &mc_reg,
                              _mtp_index_field[mtp_index],
                                          enable? mtp_index: 0);
                soc_reg_field_set(unit, MIRROR_CONTROLr, &mc_reg,
                                      _non_uc_mtp_index_field[mtp_index],
                                      enable? mtp_index: 0);
            }
        }
    }

    /* Set up standard settings for flexible mirroring. */
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        /* Set MTP mapping to 1-1 for flexible mirroring */
         if (soc_feature(unit, soc_feature_mirror_control_mem)) {
             BCM_IF_ERROR_RETURN(WRITE_MIRROR_CONTROLm(unit, MEM_BLOCK_ANY,
                                               port, &mc_entry));
         } else {
             BCM_IF_ERROR_RETURN(WRITE_MIRROR_CONTROLr(unit, port, mc_reg));
         }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Reset global mirror enable bit after the mirror control
     * register is handled. */
    mode = MIRROR_CONFIG_MODE(unit);
    if (enable) {
        BCM_IF_ERROR_RETURN(_bcm_esw_mirror_enable_set(unit, port,
                                            IS_ST_PORT(unit, port) ? 1 : 
                                ((BCM_MIRROR_DISABLE != mode) ? 1 : 0)));
    } else {
        BCM_IF_ERROR_RETURN(_bcm_esw_mirror_enable_set(unit, port, 0));
    }

    /* Disable ingress mirroring. */
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_ingress_set(unit, port, 0));

    /* Disable egress mirroring for all MTP indexes */
    SOC_PBMP_CLEAR(pbmp);

    for (mtp_index = 0; mtp_index < BCM_MIRROR_MTP_COUNT; mtp_index++) {
        (void)(_bcm_esw_mirror_egr_dest_set(unit, port, mtp_index,
                                                    &pbmp));
    }

#ifdef BCM_FIREBOLT_SUPPORT
    /* Clear RSPAN settings */
    if (SOC_REG_IS_VALID(unit, EGR_RSPAN_VLAN_TAGr)) {
                BCM_IF_ERROR_RETURN(WRITE_EGR_RSPAN_VLAN_TAGr(unit, port, 0));
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT)
    /* Clear settings of mirror_to_pbmp_set */
#ifdef BCM_GREYHOUND2_SUPPORT
    if (soc_feature (unit, soc_feature_high_portcount_register)){
        uint64 val64;
        COMPILER_64_ZERO(val64);
        BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_LOr(unit, port, val64));
        BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_HIr(unit, port, val64));
    }else
#endif /*BCM_GREYHOUND2_SUPPORT*/
    if (SOC_REG_IS_VALID(unit, IMIRROR_BITMAP_64r)) {
        uint32 values[2];
        soc_field_t fields[] = {BITMAP_LOf, BITMAP_HIf};
        values[0] = values[1] = 0;
        BCM_IF_ERROR_RETURN(soc_reg_fields32_modify(unit, 
                 IMIRROR_BITMAP_64r, port, 2, fields, values));
    } else if (SOC_REG_IS_VALID(unit, IMIRROR_BITMAPr)) {
        BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAPr(unit, port, 0));
    }

#endif /* BCM_TRIUMPH2_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_control_mem)) {
        imirror_bitmap_entry_t entry;

        /* Clear settings of mirror_to_pbmp_set */
        sal_memset(&entry, 0, sizeof(imirror_bitmap_entry_t));
        BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAPm(unit, MEM_BLOCK_ANY, 
                    port, &entry));
        }
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        egr_port_entry_t egr_port_entry;

        rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);
        if (BCM_SUCCESS(rv)) {
            soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                      MIRROR_ENCAP_ENABLEf, 0);
            soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                      MIRROR_ENCAP_INDEXf, 0);
            rv = WRITE_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */
    return rv;

}

/*
 * Function:
 *        bcmi_esw_mirror_port_attach
 * Purpose:
 *        initialize the mirror per-port configuration in the runtime.
 * Parameters:
 *    unit - (IN) BCM device number.
 *    port - local port number
 * Returns:
 *        BCM_E_XXX
 */

int
bcmi_esw_mirror_port_attach (int unit, bcm_port_t port)
{
    return _bcmi_esw_mirror_flex_port_init(unit,port,TRUE);
}

/*
 * Function:
 *        bcmi_esw_mirror_port_dettach
 * Purpose:
 *        clear the mirror per-port configuration.
 * Parameters:
 *    unit - (IN) BCM device number.
 *    port - local port number
 * Returns:
 *        BCM_E_XXX
 */

int
bcmi_esw_mirror_port_detach (int unit, bcm_port_t port)
{
    return _bcmi_esw_mirror_flex_port_init(unit,port,FALSE);
}

/*
 * Function:
 *	  bcm_esw_mirror_mode_set
 * Description:
 *	  Enable or disable mirroring.  Will wait for bcm_esw_mirror_to_set
 *        to be called to actually do the enable if needed.
 * Parameters:
 *        unit - (IN) BCM device number
 *	  mode - (IN) One of BCM_MIRROR_(DISABLE|L2|L2_L3)
 * Returns:
 *	  BCM_E_XXX
 */
int
bcm_esw_mirror_mode_set(int unit, int mode)
{
    int      rv = BCM_E_UNAVAIL;   /* Operation return status.    */

    /* Initialization check */
    if (0 == MIRROR_INIT(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if ((BCM_MIRROR_L2 != mode) && 
        (BCM_MIRROR_L2_L3 != mode) && 
        (BCM_MIRROR_DISABLE != mode)) {
          return (BCM_E_PARAM);
    }

    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        /* Not supported for flexible MTP mode */
        return (BCM_E_CONFIG);
    }

    MIRROR_LOCK(unit);
    rv = _bcm_esw_mirror_mode_set(unit, mode);
    MIRROR_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *   	bcm_esw_mirror_mode_get
 * Description:
 *	    Get mirror mode. (L2/L2_L3/DISABLED).
 * Parameters:
 *	    unit - BCM device number
 *	    mode - (OUT) One of BCM_MIRROR_(DISABLE|L2|L2_L3)
 * Returns:
 *	    BCM_E_XXX
 */
int
bcm_esw_mirror_mode_get(int unit, int *mode)
{
    /* Initialization check */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if (NULL == mode) {
        return BCM_E_PARAM;
    }
    MIRROR_LOCK(unit);
    *mode = MIRROR_CONFIG_MODE(unit);
    MIRROR_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	   bcm_esw_mirror_to_set
 * Description:
 *	   Set the mirror-to port for all mirroring, enabling mirroring
 *	   if a mode has previously been set.
 * Parameters:
 *	   unit - (IN) BCM device number
 *	   port - (IN) The port to mirror all ingress/egress selections to
 * Returns:
 *	   BCM_E_XXX
 * Notes:
 *     When mirroring to a remote unit, the mirror-to port
 *     should be the appropriate stack port on the local unit.
 *     This will return BCM_E_CONFIG if the unit is configured for,
 *     or only supports directed mirroring.
 */
int
bcm_esw_mirror_to_set(int unit, bcm_port_t port)
{
    bcm_mirror_destination_t mirror_dest;    /* Destination port/trunk.     */
    int rv;                                  /* Operation return status.    */
    int mod_out, port_out;                   /* Module and port for mapping */
    bcm_gport_t gport;                       /* Local gport operations      */

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    gport = port;
    if (BCM_GPORT_IS_SET(gport)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, gport, &port));
    }
    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return (BCM_E_PORT);
    }

    bcm_mirror_destination_t_init(&mirror_dest);

    if (!MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
        return (BCM_E_CONFIG);
    }

    if (BCM_GPORT_IS_SET(gport)) {
        mirror_dest.gport = gport;
    } else {
        BCM_IF_ERROR_RETURN(bcm_esw_port_gport_get(unit, port, &gport));
        BCM_IF_ERROR_RETURN(
            _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                   SOC_GPORT_MODPORT_MODID_GET(gport),
                                   SOC_GPORT_MODPORT_PORT_GET(gport),
                                   &mod_out, &port_out));
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_gport_construct(unit, port_out, mod_out, 0, 
                                        &(mirror_dest.gport)));
    }

    /* Create traditional mirror destination. */
    BCM_GPORT_MIRROR_SET(mirror_dest.mirror_dest_id, 0);
    mirror_dest.flags = BCM_MIRROR_DEST_WITH_ID | BCM_MIRROR_DEST_REPLACE;
    
    MIRROR_LOCK(unit);

    rv = bcm_esw_mirror_destination_create(unit, &mirror_dest);
    if (BCM_FAILURE(rv)) {
        MIRROR_UNLOCK(unit);
        return (rv);
    }

    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        /* Ingress */
        MIRROR_CONFIG_SHARED_MTP_DEST(unit, 0) = mirror_dest.mirror_dest_id;
        MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, 0) = 1;
        /* Egress */
        MIRROR_CONFIG_SHARED_MTP_DEST(unit,
                             BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX) =
            mirror_dest.mirror_dest_id;
        MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit,
                             BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX) = 1;
    } else {
        MIRROR_CONFIG_ING_MTP_DEST(unit, 0) = mirror_dest.mirror_dest_id;
        MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0) = 1;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        /* Ingress & Egress configuration is identical. */
        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            /* For non-directed flexible mirroring, we use MTP 0 for
             * the ingress and MTP 2 for the egress. */
            rv = _bcm_xgs3_mtp_init(unit, 0, BCM_MIRROR_PORT_INGRESS);
            rv = _bcm_xgs3_mtp_init(unit,
                                    BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX,
                                    BCM_MIRROR_PORT_EGRESS);
        } else {
            MIRROR_CONFIG_EGR_MTP(unit, 0) = MIRROR_CONFIG_ING_MTP(unit, 0);
            /* Write MTP registers */
            rv = _bcm_xgs3_mtp_init(unit, 0, (BCM_MIRROR_PORT_EGRESS |
                                              BCM_MIRROR_PORT_INGRESS));
        }
        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    rv = bcm_esw_mirror_mode_set(unit, MIRROR_CONFIG_MODE(unit));
    MIRROR_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *	bcm_esw_mirror_to_get
 * Description:
 *	Get the mirror-to port for all mirroring
 * Parameters:
 *	unit - (IN)  BCM device number
 *	port - (OUT) The port to mirror all ingress/egress selections to
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_esw_mirror_to_get(int unit, bcm_port_t *port)
{
    uint32             flags;      /* Mirror destination flags. */
    int                rv;         /* Operation return status  */
    int                isGport;    /* Indicator on which format to return port */
    bcm_module_t       mymodid, modid;    /* module id to construct a gport */
    bcm_gport_t        gport = BCM_GPORT_INVALID;
    int                mod_out, port_out; /* To do a modmap mapping */
    int                mirror_cnt = 0; 
    _bcm_mtp_config_p  mtp_cfg;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if (NULL == port) {
        return (BCM_E_PARAM);
    }

    flags = 0;
    BCM_IF_ERROR_RETURN(
        bcm_esw_stk_my_modid_get(unit, &mymodid));

    MIRROR_LOCK(unit);
  
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        mtp_cfg = &MIRROR_CONFIG_SHARED_MTP(unit, 0);
        if (mtp_cfg != NULL) {
            mirror_cnt = MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, 0);
            gport = MIRROR_CONFIG_SHARED_MTP_DEST(unit, 0);
        }
    } else {
        mtp_cfg = &MIRROR_CONFIG_ING_MTP(unit, 0);
        if (mtp_cfg != NULL) {
            mirror_cnt = MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0);
            gport = MIRROR_CONFIG_ING_MTP_DEST(unit, 0);
        }
    }

    if (mirror_cnt) {
         rv = _bcm_mirror_destination_gport_parse(unit, gport ,&modid, 
                                                  port, &flags);
    } else {
        *port = -1;
        modid = mymodid;
        rv = BCM_E_NONE;
    }
    MIRROR_UNLOCK(unit);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    if (flags & BCM_MIRROR_PORT_DEST_TRUNK) { 
        return (BCM_E_CONFIG);
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    if (isGport) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET, mymodid, *port, 
                                   &mod_out, &port_out));
        BCM_IF_ERROR_RETURN
            (_bcm_mirror_gport_construct(unit, port_out, mod_out,
                                         flags, port)); 
    } else if (*port != -1) {
        BCM_GPORT_MODPORT_SET(gport, modid, *port);
        BCM_IF_ERROR_RETURN
            (bcm_esw_port_local_get(unit, gport, port));
    }
    
    return (BCM_E_NONE);
}

/*
 * Function:
 *   	bcm_esw_mirror_ingress_set
 * Description:
 *	    Enable or disable mirroring per ingress
 * Parameters:
 *   	unit   - (IN) BCM device number
 *	    port   - (IN) The port to affect
 *   	enable - (IN) Boolean value for on/off
 * Returns:
 *	    BCM_E_XXX
 * Notes:
 *	    Mirroring must also be globally enabled.
 */
int
bcm_esw_mirror_ingress_set(int unit, bcm_port_t port, int enable)
{
    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return (BCM_E_PORT);
    }

    if (IS_CPU_PORT(unit, port) && 
        !soc_feature(unit, soc_feature_cpuport_mirror)) {
        return (BCM_E_PORT);
    }

    /* Set ingress mirroring enable in port table. */
    return _bcm_esw_mirror_ingress_set(unit, port, 
                                       ((enable) ?  BCM_MIRROR_MTP_ONE : (0)));

}

/*
 * Function:
 * 	    bcm_esw_mirror_ingress_get
 * Description:
 * 	    Get the mirroring per ingress enabled/disabled status
 * Parameters:
 *	    unit   - (IN)  BCM device number
 *   	port   - (IN)  The port to check
 *	    enable - (OUT) Place to store boolean return value for on/off
 * Returns:
 *	    BCM_E_XXX
 */
int
bcm_esw_mirror_ingress_get(int unit, bcm_port_t port, int *enable)
{
    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if (NULL == enable) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return (BCM_E_PORT);
    }

    if (IS_CPU_PORT(unit, port) &&
        !soc_feature(unit, soc_feature_cpuport_mirror)) {
        return (BCM_E_PORT);
    }

    /* Get ingress mirroring enable from  port table. */
    return _bcm_esw_mirror_ingress_get(unit, port, enable);
}

/*
 * Function:
 * 	   bcm_esw_mirror_egress_set
 * Description:
 *  	Enable or disable mirroring per egress
 * Parameters:
 *  	unit   - (IN) BCM device number
 *	    port   - (IN) The port to affect
 *	    enable - (IN) Boolean value for on/off
 * Returns:
 *	    BCM_E_XXX
 * Notes:
 *  	Mirroring must also be globally enabled.
 */
int
bcm_esw_mirror_egress_set(int unit, bcm_port_t port, int enable)
{

    int rv;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    if (IS_CPU_PORT(unit, port) &&
        !soc_feature(unit, soc_feature_cpuport_mirror)) {
        return BCM_E_PORT;
    }

    MIRROR_LOCK(unit);

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        /* Enable third MTP for egress since this is for single MTP mode */
        if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            /* Update MTP_MODE */
            MIRROR_CONFIG_MTP_MODE_BMP(unit) |=
                BCM_MIRROR_MTP_FLEX_EGRESS_D15;

            rv = _bcm_tr2_mirror_mtp_slot_update(unit);
            if (BCM_FAILURE(rv)) {
                MIRROR_UNLOCK(unit);
                return (rv);
            }
        } else {
            MIRROR_CONFIG_SHARED_MTP(unit,
                      BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX).egress = TRUE;
        }
        rv = _bcm_esw_mirror_egress_set(unit, port, 
                     (enable) ? BCM_MIRROR_MTP_FLEX_EGRESS_D15 : (0));
    } else 
#endif /* BCM_TRIUMPH2_SUPPORT */
    {
        rv = _bcm_esw_mirror_egress_set(unit, port, 
                                        (enable) ?  BCM_MIRROR_MTP_ONE : (0));
    }

    MIRROR_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *	    bcm_esw_mirror_egress_get
 * Description:
 * 	    Get the mirroring per egress enabled/disabled status
 * Parameters:
 *  	unit -   (IN)  BCM device number
 *  	port -   (IN)  The port to check
 *  	enable - (OUT) Place to store boolean return value for on/off
 * Returns:
 *  	BCM_E_XXX
 */
int
bcm_esw_mirror_egress_get(int unit, bcm_port_t port, int *enable)
{
    int rv; 

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if (NULL == enable) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return (BCM_E_PORT);
    }

    if (IS_CPU_PORT(unit, port) &&
        !soc_feature(unit, soc_feature_cpuport_mirror)) {
        return (BCM_E_PORT);
    }

    MIRROR_LOCK(unit); 
    rv = _bcm_esw_mirror_egress_get(unit, port, enable);
    MIRROR_UNLOCK(unit);
    *enable = *enable ? 1 : 0;
    return (rv);
}

/*
 * Function:
 *      bcm_esw_mirror_to_pbmp_set
 * Description:
 *  	Set the mirror-to port bitmap for mirroring on a given port.
 * Parameters:
 *  	unit - (IN) BCM device number
 *  	port - (IN) The port to affect
 *      pbmp - (IN) The port bitmap of mirrored to ports for this port.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	This API interface is only supported on XGS fabric devices and
 *      production versions of XGS3 switch devices. For XGS3 devices
 *      this function is normally only used when the XGS3 device is
 *      stacked in a ring configuration with BCM567x fabric devices.
 */
int
bcm_esw_mirror_to_pbmp_set(int unit, bcm_port_t port, pbmp_t pbmp)
{
#if defined(BCM_KATANA2_SUPPORT)
    int min_subport = SOC_INFO(unit).pp_port_index_min;
#endif 
    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }
    /* Input parameters check. */
#if defined(BCM_KATANA2_SUPPORT)
    if ((soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe)) &&
        (port >= min_subport)) {
    } else
#endif
    if (!IS_PORT(unit, port) || !SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#if defined(BCM_HERCULES_SUPPORT)
    if (SOC_IS_HERCULES(unit)) {
        MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
        return WRITE_ING_MIRTOBMAPr(unit, port, SOC_PBMP_WORD_GET(pbmp, 0));
    }
#endif /* BCM_HERCULES_SUPPORT */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (soc_feature(unit, soc_feature_egr_mirror_path)) {
        int mport;

        /* Both ingress and egress ports must be stack ports */
        if (!IS_ST_PORT(unit, port)) {
            return (BCM_E_PORT);
        }
        
        PBMP_ITER(pbmp, mport) {
            if (!IS_ST_PORT(unit, mport)) {
                return (BCM_E_PORT);
            }
        }

#ifdef BCM_TRIDENT2_SUPPORT
    if (SOC_IS_TD2_TT2(unit)) {
        imirror_bitmap_entry_t  entry;
        int egr_port;

        sal_memset(&entry, 0, sizeof(entry));

        PBMP_ITER(pbmp, egr_port) {
            soc_mem_field32_set(unit, IMIRROR_BITMAPm, &entry,
                                EGRESS_PORTf, egr_port);
            soc_mem_field32_set(unit, IMIRROR_BITMAPm, &entry, ENABLEf,
                                1);
            break;
        }
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, IMIRROR_BITMAPm, MEM_BLOCK_ANY, port,
                           &entry));
        if (IS_CPU_PORT(unit, port) && SOC_INFO(unit).cpu_hg_index != -1) {
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, IMIRROR_BITMAPm, MEM_BLOCK_ANY,
                               SOC_INFO(unit).cpu_hg_index, &entry));
        }
        return BCM_E_NONE;
    }
#endif /* BCM_TRIDENT2_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_control_mem)) {
            imirror_bitmap_entry_t entry;
            soc_mem_pbmp_field_set(unit, IMIRROR_BITMAPm, &entry, BITMAPf,
                                   &pbmp);
            return (WRITE_IMIRROR_BITMAPm(unit, MEM_BLOCK_ANY, port, &entry));
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT

#ifdef BCM_GREYHOUND2_SUPPORT
        if(soc_feature (unit, soc_feature_high_portcount_register)){        
            uint64 mirror64, val64;
            BCM_IF_ERROR_RETURN(READ_IMIRROR_BITMAP_LOr(unit, port, &mirror64));
            COMPILER_64_SET(val64, SOC_PBMP_WORD_GET(pbmp, 1),
                            SOC_PBMP_WORD_GET(pbmp, 0));
            soc_reg64_field_set(unit, IMIRROR_BITMAP_LOr, &mirror64,
                                BITMAPf, val64);
            BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_LOr(unit, port, mirror64));

            COMPILER_64_ZERO(mirror64);
            COMPILER_64_ZERO(val64);
            BCM_IF_ERROR_RETURN(READ_IMIRROR_BITMAP_HIr(unit, port, &mirror64));
            COMPILER_64_SET(val64, SOC_PBMP_WORD_GET(pbmp, 3),
                            SOC_PBMP_WORD_GET(pbmp, 2));
            soc_reg64_field_set(unit, IMIRROR_BITMAP_HIr, &mirror64,
                                BITMAPf, val64);
            BCM_IF_ERROR_RETURN(WRITE_IMIRROR_BITMAP_HIr(unit, port, mirror64));
            return (BCM_E_NONE);    
        }else 
#endif /*BCM_GREYHOUND2_SUPPORT*/
        if (SOC_REG_IS_VALID(unit, IMIRROR_BITMAP_64r)) {
            uint32 values[2];
            soc_field_t fields[] = {BITMAP_LOf, BITMAP_HIf};

            values[0] = SOC_PBMP_WORD_GET(pbmp, 0);
            values[1] = SOC_PBMP_WORD_GET(pbmp, 1);
            return soc_reg_fields32_modify(unit, IMIRROR_BITMAP_64r, port,
                                           2, fields, values);
        } else 
#endif /* BCM_TRIUMPH2_SUPPORT */
        {
            uint32 mirbmap;

            mirbmap = SOC_PBMP_WORD_GET(pbmp, 0);
            if (SOC_IS_FBX(unit)) {
                mirbmap >>= SOC_HG_OFFSET(unit);
            }
            return WRITE_IMIRROR_BITMAPr(unit, port, mirbmap);
        }
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *   	bcm_esw_mirror_to_pbmp_get
 * Description:
 *	    Get the mirror-to port bitmap for mirroring on the
 *	    specified port.
 * Parameters:
 *	    unit - (IN) BCM device number
 *	    port - (IN) The port to mirror all ingress/egress selections to
 *      pbmp - (OUT) The port bitmap of mirror-to ports for this port.
 * Returns:
 *	    BCM_E_XXX
 */
int
bcm_esw_mirror_to_pbmp_get(int unit, bcm_port_t port, pbmp_t *pbmp)
{
#if defined(BCM_KATANA2_SUPPORT)
    int min_subport = SOC_INFO(unit).pp_port_index_min; 
#endif 
    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
#if defined(BCM_KATANA2_SUPPORT)
    if ((soc_feature(unit, soc_feature_linkphy_coe) ||
        soc_feature(unit, soc_feature_subtag_coe)) &&
        (port >= min_subport)) {
    } else
#endif
    if (!IS_PORT(unit, port) || !SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#if defined(BCM_HERCULES_SUPPORT)
    if (SOC_IS_HERCULES(unit)) {
        int rv;
        uint32 mirbmap;

        rv = READ_ING_MIRTOBMAPr(unit, port, &mirbmap);
        SOC_PBMP_CLEAR(*pbmp);
        SOC_PBMP_WORD_SET(*pbmp, 0, mirbmap);
        return rv;
    }
#endif /* BCM_HERCULES_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_path)) {
        int rv;

#ifdef BCM_TRIDENT2_SUPPORT
        if (SOC_IS_TD2_TT2(unit)) {
            imirror_bitmap_entry_t entry;
            int egr_port, tgid;
            int member_count;
            bcm_trunk_member_t *member_array = NULL;
            bcm_module_t mod_out;
            bcm_port_t port_out;
            bcm_trunk_t tgid_out;
            int id_out;
            int idx;
            int is_local_modid;

            BCM_IF_ERROR_RETURN
                (READ_IMIRROR_BITMAPm(unit, MEM_BLOCK_ANY, port, &entry));
            BCM_PBMP_CLEAR(*pbmp);
            if (!soc_mem_field32_get(unit, IMIRROR_BITMAPm, &entry, ENABLEf)) {
                return BCM_E_NONE;
            }
            if (soc_mem_field32_get(unit, IMIRROR_BITMAPm, &entry, ISTRUNKf)) {
                tgid = soc_mem_field32_get(unit, IMIRROR_BITMAPm, &entry,
                                           HG_TRUNK_IDf);

                /* Get trunk member port/module pairs. */
                BCM_IF_ERROR_RETURN
                    (bcm_esw_trunk_get(unit, tgid, NULL, 0, NULL,
                                       &member_count));
                if (member_count > 0) {
                    member_array =
                        sal_alloc(sizeof(bcm_trunk_member_t) * member_count,
                                  "trunk member array");
                    if (NULL == member_array) {
                        return BCM_E_MEMORY;
                    }
                    rv = bcm_esw_trunk_get(unit, tgid, NULL, member_count,
                                           member_array, &member_count);
                    if (BCM_FAILURE(rv)) {
                        sal_free(member_array);
                        return rv;
                    }
                }

                /* Fill pbmp with trunk members from other modules . */
                for (idx = 0; idx < member_count; idx++) {
                    rv = _bcm_esw_gport_resolve(unit, member_array[idx].gport,
                                                &mod_out, &port_out,
                                                &tgid_out, &id_out);
                    if (BCM_FAILURE(rv) || -1 != tgid_out || -1 != id_out) {
                        sal_free(member_array);
                        return rv;
                    }
                    rv = _bcm_esw_modid_is_local(unit, mod_out,
                                                 &is_local_modid);
                    if (BCM_FAILURE(rv)) {
                        sal_free(member_array);
                        return rv;
                    }
                    if (!is_local_modid) {
                        rv = bcm_esw_stk_modport_get(unit, mod_out, &egr_port);
                        if (BCM_FAILURE(rv)) {
                            sal_free(member_array);
                            return rv;
                        }
                        BCM_PBMP_PORT_ADD(*pbmp, egr_port);
                    }
                }
                sal_free(member_array);
            } else {
                egr_port = soc_mem_field32_get(unit, IMIRROR_BITMAPm, &entry,
                                                EGRESS_PORTf);
                BCM_PBMP_PORT_SET(*pbmp, egr_port); 
            }
            return BCM_E_NONE;
        }
#endif /* BCM_TRIDENT2_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_control_mem)) {
            imirror_bitmap_entry_t entry;
            BCM_IF_ERROR_RETURN
                (READ_IMIRROR_BITMAPm(unit, MEM_BLOCK_ANY, port, &entry));
            soc_mem_pbmp_field_get(unit, IMIRROR_BITMAPm, &entry, BITMAPf,
                                   pbmp);
            rv = BCM_E_NONE;
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT

#ifdef BCM_GREYHOUND2_SUPPORT
        if (soc_feature (unit, soc_feature_high_portcount_register)){
            uint64 mirror64, val64;
            BCM_IF_ERROR_RETURN(soc_reg_get(unit, IMIRROR_BITMAP_LOr, port, 0, &mirror64));
            SOC_PBMP_CLEAR(*pbmp);
            val64 = soc_reg64_field_get(unit, IMIRROR_BITMAP_LOr, mirror64, BITMAPf);
            SOC_PBMP_WORD_SET(*pbmp, 0, COMPILER_64_LO(val64));
            SOC_PBMP_WORD_SET(*pbmp, 1, COMPILER_64_HI(val64));

            COMPILER_64_ZERO(mirror64);
            COMPILER_64_ZERO(val64);
            BCM_IF_ERROR_RETURN(soc_reg_get(unit, IMIRROR_BITMAP_HIr, port, 0, &mirror64));
            val64 = soc_reg64_field_get(unit, IMIRROR_BITMAP_HIr, mirror64, BITMAPf);
            SOC_PBMP_WORD_SET(*pbmp, 2, COMPILER_64_LO(val64));
            SOC_PBMP_WORD_SET(*pbmp, 3, COMPILER_64_HI(val64));
            rv = BCM_E_NONE;
        }else
#endif /*BCM_GREYHOUND2_SUPPORT*/
        if (SOC_REG_IS_VALID(unit, IMIRROR_BITMAP_64r)) {
            uint64 mirbmap64;
            rv = READ_IMIRROR_BITMAP_64r(unit, port, &mirbmap64);
            SOC_PBMP_CLEAR(*pbmp);
            SOC_PBMP_WORD_SET(*pbmp, 0,
                soc_reg64_field32_get(unit, IMIRROR_BITMAP_64r,
                                      mirbmap64, BITMAP_LOf));
            SOC_PBMP_WORD_SET(*pbmp, 1,
                soc_reg64_field32_get(unit, IMIRROR_BITMAP_64r,
                                      mirbmap64, BITMAP_HIf));
        } else 
#endif /* BCM_TRIUMPH2_SUPPORT */
        {
            uint32 mirbmap;

            rv = READ_IMIRROR_BITMAPr(unit, port, &mirbmap);
            if (SOC_IS_FBX(unit)) {
                mirbmap <<= SOC_HG_OFFSET(unit);
            }
            SOC_PBMP_CLEAR(*pbmp);
            SOC_PBMP_WORD_SET(*pbmp, 0, mirbmap);
        }
        return rv;
    }
#endif
    return (BCM_E_UNAVAIL);
}

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
/*
 * Function:
 *      _bcm_trident_mirror_vlan_set
 * Description:
 *      Set VLAN for egressing mirrored packets on a port (RSPAN)
 *      This will support the legacy mode where RSPAN is a per-egress-port
 *      property.
 * Parameters:
 *      unit    - (IN) Bcm device number.
 *      port    - (IN) Mirror-to port to set (-1 for all ports).
 *      tpid    - (IN) Tag protocol id (0 to disable).
 *      vlan    - (IN) Virtual lan number (0 to disable).
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_vlan_set(int unit, bcm_port_t port,
                             uint16 tpid, uint16 vlan)
{
    int rv = BCM_E_NONE;
    uint32 profile_index, old_profile_index = 0;
    uint32 hw_buffer;
    egr_mirror_encap_control_entry_t control_entry;
    egr_mirror_encap_data_1_entry_t data_1_entry;
    egr_mirror_encap_data_2_entry_t data_2_entry;
#ifdef BCM_TRIDENT_SUPPORT
    egr_port_entry_t egr_port_entry;
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_GREYHOUND_SUPPORT
    uint64 reg_val;
#endif /* BCM_GREYHOUND_SUPPORT */
    void *entries[EGR_MIRROR_ENCAP_ENTRIES_NUM];

    hw_buffer = (uint32)(tpid << 16) | vlan;

    if (0 != hw_buffer) {
        sal_memset(&control_entry, 0, sizeof(control_entry));
        sal_memset(&data_1_entry, 0, sizeof(data_1_entry));
        sal_memset(&data_2_entry, 0, sizeof(data_2_entry));

        entries[0] = &control_entry;
        entries[1] = &data_1_entry;
        entries[2] = &data_2_entry;

        /* Setup Mirror Control Memory */
        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, &control_entry,
                             ENTRY_TYPEf, BCM_TD_MIRROR_ENCAP_TYPE_RSPAN);

        soc_EGR_MIRROR_ENCAP_CONTROLm_field32_set(unit, &control_entry,
                             RSPAN__ADD_OPTIONAL_HEADERf,
                                                  BCM_TD_MIRROR_HEADER_ONLY);

        /* Setup Mirror Data 1 Memory */
        soc_EGR_MIRROR_ENCAP_DATA_1m_field32_set(unit, &data_1_entry,
                             RSPAN__RSPAN_VLAN_TAGf, hw_buffer);

        rv = _bcm_egr_mirror_encap_entry_add(unit, entries, &profile_index);

        if (BCM_SUCCESS(rv)) {
#ifdef BCM_GREYHOUND_SUPPORT
            if (SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) ||
                SOC_IS_GREYHOUND2(unit)) {
                /* Remove the previous profile index if any */
                rv = READ_EGR_PORT_64r(unit, port, &reg_val);
    
                old_profile_index = -1;
                if (BCM_SUCCESS(rv) &&
                    (0 != soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val, 
                                                MIRROR_ENCAP_ENABLEf))) {
                    old_profile_index =
                        soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val, 
                                              MIRROR_ENCAP_INDEXf);
                }
    
                /* Supply the correct profile index to the egress port */
                soc_reg64_field32_set(unit, EGR_PORT_64r, &reg_val, 
                                      MIRROR_ENCAP_ENABLEf, 1);
                soc_reg64_field32_set(unit, EGR_PORT_64r, &reg_val, 
                                      MIRROR_ENCAP_INDEXf, profile_index);
                rv = WRITE_EGR_PORT_64r(unit, port, reg_val);
            } else
#endif /* BCM_GREYHOUND_SUPPORT */
            {
#ifdef BCM_TRIDENT_SUPPORT
            /* Remove the previous profile index if any */
            rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);

            old_profile_index = -1;
            if (BCM_SUCCESS(rv) &&
                (0 != soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                                MIRROR_ENCAP_ENABLEf))) {
                old_profile_index =
                    soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                              MIRROR_ENCAP_INDEXf);
            }

            /* Supply the correct profile index to the egress port */
            soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                      MIRROR_ENCAP_ENABLEf, 1);
            soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                      MIRROR_ENCAP_INDEXf, profile_index);
            rv = WRITE_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);

#endif /* BCM_TRIDENT_SUPPORT */
            }

            if (BCM_SUCCESS(rv) && (-1 != old_profile_index)) {
                rv = _bcm_egr_mirror_encap_entry_delete(unit,
                                                        old_profile_index);
            }
        }
    } else {
#ifdef BCM_GREYHOUND_SUPPORT
        if (SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) ||
            SOC_IS_GREYHOUND2(unit)) {
            rv = READ_EGR_PORT_64r(unit, port, &reg_val);
    
            if (BCM_SUCCESS(rv) &&
                (0 != soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val, 
                                            MIRROR_ENCAP_ENABLEf))) {
                old_profile_index =
                    soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val, 
                                          MIRROR_ENCAP_INDEXf);
                soc_reg64_field32_set(unit, EGR_PORT_64r, &reg_val, 
                                          MIRROR_ENCAP_ENABLEf, 0);
                soc_reg64_field32_set(unit, EGR_PORT_64r, &reg_val, 
                                          MIRROR_ENCAP_INDEXf, 0);
                rv = WRITE_EGR_PORT_64r(unit, port, reg_val);
    
                if (BCM_SUCCESS(rv)) {
                    rv = _bcm_egr_mirror_encap_entry_delete(unit,
                                                            old_profile_index);
                }
            } /* Else do nothing */
        } else
#endif /* BCM_GREYHOUND_SUPPORT */
        {
#ifdef BCM_TRIDENT_SUPPORT
        rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);

        if (BCM_SUCCESS(rv) &&
            (0 != soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                            MIRROR_ENCAP_ENABLEf))) {
            old_profile_index =
                soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                          MIRROR_ENCAP_INDEXf);
            soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                      MIRROR_ENCAP_ENABLEf, 0);
            soc_EGR_PORTm_field32_set(unit, &egr_port_entry,
                                      MIRROR_ENCAP_INDEXf, 0);
            rv = WRITE_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);

            if (BCM_SUCCESS(rv)) {
                rv = _bcm_egr_mirror_encap_entry_delete(unit,
                                                        old_profile_index);
            }
        } /* Else do nothing */
#endif /* BCM_TRIDENT_SUPPORT */
        }
    }

    return rv;
}

/*
 * Function:
 *      _bcm_trident_mirror_vlan_get
 * Description:
 *      Get VLAN for egressing mirrored packets on a port (RSPAN)
 *      This will support the legacy mode where RSPAN is a per-egress-port
 *      property.
 * Parameters:
 *      unit    - (IN) Bcm device number.
 *      port    - (IN) Mirror-to port to set (-1 for all ports).
 *      tpid    - (OUT) Tag protocol id (0 to disable).
 *      vlan    - (OUT) Virtual lan number (0 to disable).
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_trident_mirror_vlan_get(int unit, bcm_port_t port,
                             uint16 *tpid, uint16 *vlan)
{
    uint32 profile_index = 0;
    uint32 hw_buffer;
    egr_mirror_encap_control_entry_t control_entry;
    egr_mirror_encap_data_1_entry_t data_1_entry;
    egr_mirror_encap_data_2_entry_t data_2_entry;
#ifdef BCM_TRIDENT_SUPPORT
    egr_port_entry_t egr_port_entry;
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_GREYHOUND_SUPPORT
    uint64 reg_val;
#endif /* BCM_GREYHOUND_SUPPORT */
    void *entries[EGR_MIRROR_ENCAP_ENTRIES_NUM];

#ifdef BCM_GREYHOUND_SUPPORT
    if (SOC_IS_GREYHOUND(unit) || SOC_IS_HURRICANE3(unit) ||
        SOC_IS_GREYHOUND2(unit)) {
        BCM_IF_ERROR_RETURN(READ_EGR_PORT_64r(unit, port, &reg_val));
    
        if (0 == soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val, 
                                       MIRROR_ENCAP_ENABLEf)) {
            return BCM_E_NOT_FOUND;
        }
    
        profile_index =
            soc_reg64_field32_get(unit, EGR_PORT_64r, reg_val, 
                                  MIRROR_ENCAP_INDEXf);
    } else
#endif /* BCM_GREYHOUND_SUPPORT */
    {
#ifdef BCM_TRIDENT_SUPPORT
        BCM_IF_ERROR_RETURN
            (READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry));
    
        if (0 == soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                           MIRROR_ENCAP_ENABLEf)) {
            return BCM_E_NOT_FOUND;
        }
    
        profile_index =
            soc_EGR_PORTm_field32_get(unit, &egr_port_entry,
                                      MIRROR_ENCAP_INDEXf);
#endif /* BCM_TRIDENT_SUPPORT */
    }

    entries[0] = &control_entry;
    entries[1] = &data_1_entry;
    entries[2] = &data_2_entry;
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_get(unit, EGR_MIRROR_ENCAP(unit),
                             profile_index, 1, entries));


    if (soc_EGR_MIRROR_ENCAP_CONTROLm_field32_get(unit, &control_entry,
                       ENTRY_TYPEf) != BCM_TD_MIRROR_ENCAP_TYPE_RSPAN) {
        return BCM_E_CONFIG;
    }

    /* RSPAN recovery */
    hw_buffer =
        soc_EGR_MIRROR_ENCAP_DATA_1m_field32_get(unit, &data_1_entry,
                                                 RSPAN__RSPAN_VLAN_TAGf);

    *vlan = (bcm_vlan_t) (hw_buffer & 0xffff);
    *tpid = (uint16) ((hw_buffer >> 16) & 0xffff);
 
    return BCM_E_NONE;
}
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

/*
 * Function:
 *      bcm_esw_mirror_vlan_set
 * Description:
 *      Set VLAN for egressing mirrored packets on a port (RSPAN)
 * Parameters:
 *      unit    - (IN) Bcm device number.
 *      port    - (IN) Mirror-to port to set (-1 for all ports).
 *      tpid    - (IN) Tag protocol id (0 to disable).
 *      vlan    - (IN) Virtual lan number (0 to disable).
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mirror_vlan_set(int unit, bcm_port_t port,
                        uint16 tpid, uint16 vlan)
{
    int rv;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Vlan id range check. */ 
    if (!BCM_VLAN_VALID(vlan) && vlan != BCM_VLAN_NONE) {
        return (BCM_E_PARAM);
    } 


    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }

    if(!SOC_PORT_VALID(unit, port)) {
        return (BCM_E_PORT);
    }

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        rv = _bcm_trident_mirror_vlan_set(unit, port, tpid, vlan);
    } else
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        rv = WRITE_EGR_RSPAN_VLAN_TAGr(unit, port, (tpid << 16) | vlan);
    } else 
#endif /* BCM_FIREBOLT_SUPPORT */
    {
        rv = BCM_E_UNAVAIL;
    }
    return (rv);
}

/*
 * Function:
 *      bcm_esw_mirror_vlan_get
 * Description:
 *      Get VLAN for egressing mirrored packets on a port (RSPAN)
 * Parameters:
 *      unit    - (IN) BCM device number
 *      port    - (IN) Mirror-to port for which to get tag info
 *      tpid    - (OUT) tag protocol id
 *      vlan    - (OUT) virtual lan number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mirror_vlan_get(int unit, bcm_port_t port,
                        uint16 *tpid, uint16 *vlan)
{
    int rv;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if ((NULL == tpid) || (NULL == vlan)) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
    }

    if(!SOC_PORT_VALID(unit, port)) {
        return (BCM_E_PORT);
    }

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        rv = _bcm_trident_mirror_vlan_get(unit, port, tpid, vlan);
    } else
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        uint32 rspan;

        BCM_IF_ERROR_RETURN(READ_EGR_RSPAN_VLAN_TAGr(unit, port, &rspan));
        *tpid = (rspan >> 16);
        *vlan = (rspan & 0xFFF);

        rv = BCM_E_NONE;
    } else 
#endif /* BCM_FIREBOLT_SUPPORT */
    {
        rv = BCM_E_UNAVAIL;
    }
    return (rv);
}

/*
 * Function:
 *      bcm_esw_mirror_egress_path_set
 * Description:
 *      Set egress mirror packet path for stack ring
 * Parameters:
 *      unit    - (IN) BCM device number
 *      modid   - (IN) Destination module ID (of mirror-to port)
 *      port    - (IN) Stack port for egress mirror packet
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This function should only be used for XGS3 devices stacked
 *      in a ring configuration with fabric devices that may block
 *      egress mirror packets when the mirror-to port is on a 
 *      different device than the egress port being mirrored.
 *      Currently the only such fabric device is BCM5675 rev A0.
 */
int
bcm_esw_mirror_egress_path_set(int unit, bcm_module_t modid, bcm_port_t port)
{
    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input validation */
    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
        return BCM_E_BADID;
    }

    if (BCM_GPORT_IS_SET(port)) {
        bcm_module_t    tmp_modid;
        int             isLocal;

        BCM_IF_ERROR_RETURN(
            _bcm_mirror_gport_resolve(unit, port, &port, &tmp_modid));
        BCM_IF_ERROR_RETURN(_bcm_esw_modid_is_local(unit, tmp_modid, &isLocal));

        if (TRUE != isLocal) {
            return BCM_E_PORT;
        }

    } else {
        /* Actuall physical port passed */
        if (!SOC_PORT_VALID(unit, port) || 
            !SOC_PORT_ADDRESSABLE(unit, port)) {
            return BCM_E_PORT;
        }
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) { 
        return bcm_xgs3_mirror_egress_path_set(unit, modid, port);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_esw_mirror_egress_path_get
 * Description:
 *      Get egress mirror packet path for stack ring
 * Parameters:
 *      unit    - (IN) BCM device number
 *      modid   - (IN) Destination module ID (of mirror-to port)
 *      port    - (OUT)Stack port for egress mirror packet
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      See bcm_mirror_alt_egress_pbmp_set for more details.
 */
int
bcm_esw_mirror_egress_path_get(int unit, bcm_module_t modid, bcm_port_t *port)
{
#ifdef BCM_XGS3_SWITCH_SUPPORT
    int             rv, isGport;
    bcm_module_t    mod_out;
    bcm_port_t      port_out;
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if (NULL == port) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) { 
        rv = bcm_xgs3_mirror_egress_path_get(unit, modid, port);

        if (BCM_FAILURE(rv)) {
            return rv;
        }
        BCM_IF_ERROR_RETURN(
            bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        if (isGport) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET, modid, *port, 
                                       &mod_out, &port_out));
            BCM_IF_ERROR_RETURN(
                _bcm_mirror_gport_construct(unit, port_out, mod_out, 0, port)); 
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, modid, *port, 
                                       &mod_out, port));
        }

        return (BCM_E_NONE);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *      bcm_esw_mirror_port_set
 * Description:
 *      Set mirroring configuration for a port
 * Parameters:
 *      unit      - BCM device number
 *      port      - port to configure
 *      dest_mod  - module id of mirror-to port
 *                  (-1 for local port)
 *      dest_port - mirror-to port ( can be gport or mirror_gport)
 *      flags     - BCM_MIRROR_PORT_* flags
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Setting BCM_MIRROR_PORT_ENABLE without setting _INGRESS or
 *      _EGRESS allows the port to participate in bcm_l2_cache matches
 *      with the BCM_L2_CACHE_MIRROR bit set, and to participate in
 *      bcm_field lookups with the mirror action set.
 *
 *      If bcmSwitchDirectedMirroring is disabled for the unit and
 *      dest_mod is non-negative, then the dest_mod path is looked
 *      up using bcm_topo_port_get.
 *      If bcmSwitchDirectedMirroring is enabled for the unit and
 *      dest_mod is negative, then the local unit's modid is used
 *      as the dest_mod.
 */
int
bcm_esw_mirror_port_set(int unit, bcm_port_t port,
                        bcm_module_t dest_mod, bcm_port_t dest_port,
                        uint32 flags)
{
    int         rv;                        /* Operation return status.        */
    bcm_mirror_destination_t mirror_dest;  /* Mirror destination.             */
    uint32                   destroy_flag = FALSE; /* mirror dest destroy     */

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
#ifdef BCM_TRIDENT2PLUS_SUPPORT
        if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
            _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
        } else
#endif
        {
            BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
        }
    }

    /* Input parameters check */
    if (!soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return (BCM_E_PARAM);
    }

    /* If mirroring is completely disabled, remove all mirror destinations */
    if (flags == 0 && dest_mod == -1 && dest_port == -1) {
        flags = BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS;
        if (soc_feature(unit, soc_feature_egr_mirror_true)) {
            flags |= BCM_MIRROR_PORT_EGRESS_TRUE;
        }
        return bcm_esw_mirror_port_dest_delete_all(unit, port, flags);
    }

    /* Create traditional mirror destination. */
    bcm_mirror_destination_t_init(&mirror_dest);

    MIRROR_LOCK(unit);
    if (BCM_GPORT_IS_MIRROR(dest_port)) {
        rv = bcm_esw_mirror_destination_get(unit, dest_port, &mirror_dest);
    } else {
        rv = _bcm_esw_mirror_destination_find(unit, dest_port, dest_mod, flags, &mirror_dest); 
        if (BCM_E_NOT_FOUND == rv) {
            if ((flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS)) |
                (soc_feature(unit, soc_feature_egr_mirror_true) &&
                 (flags & BCM_MIRROR_PORT_EGRESS_TRUE))) {
                rv = _bcm_esw_mirror_destination_create(unit, &mirror_dest);
                destroy_flag = TRUE;
            } else {
                MIRROR_UNLOCK(unit); 
                return (BCM_E_NONE);
            }
        }       
    }
    if (BCM_FAILURE(rv)) {
        MIRROR_UNLOCK(unit);
        return rv;
    }

    /* Enable/Disable ingress mirroring. */
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        rv = bcm_esw_mirror_port_dest_add(unit, port, BCM_MIRROR_PORT_INGRESS,
                                          mirror_dest.mirror_dest_id);
        if (BCM_E_EXISTS == rv) {
            /* Since this function is set, not add, it is sufficient if
             * the destination already exists.  Clear the error. */
            rv = BCM_E_NONE;
        }
    } else {
        rv = bcm_esw_mirror_port_dest_delete(unit, port, BCM_MIRROR_PORT_INGRESS,
                                             mirror_dest.mirror_dest_id); 
        if (BCM_E_NOT_FOUND == rv) {
            /* There is no clean way to identify delete. -> 
               if destination is not found assume success. */ 
            rv = BCM_E_NONE;
        } 
    }

    if (BCM_FAILURE(rv)) {
        /* Delete unused mirror destination. */
        if (destroy_flag) {
            (void)bcm_esw_mirror_destination_destroy(unit, mirror_dest.mirror_dest_id);
        }
        MIRROR_UNLOCK(unit); 
        return (rv);
    }

    /* Enable/Disable egress mirroring. */
    if (flags & BCM_MIRROR_PORT_EGRESS) {
        rv = bcm_esw_mirror_port_dest_add(unit, port, BCM_MIRROR_PORT_EGRESS,
                                          mirror_dest.mirror_dest_id); 
        if (BCM_E_EXISTS == rv) {
            /* Since this function is set, not add, it is sufficient if
             * the destination already exists.  Clear the error. */
            rv = BCM_E_NONE;
        }
    } else {
        rv = bcm_esw_mirror_port_dest_delete(unit, port, BCM_MIRROR_PORT_EGRESS,
                                             mirror_dest.mirror_dest_id); 
        if (BCM_E_NOT_FOUND == rv) {
            /* There is no clean way to identify delete. -> 
               if destination is not found assume success. */ 
            rv = BCM_E_NONE;
        } 
    }

    if (BCM_FAILURE(rv)) {
        /* Delete unused mirror destination. */
        if (destroy_flag) {
            (void)bcm_esw_mirror_destination_destroy(unit, mirror_dest.mirror_dest_id);
        }
        MIRROR_UNLOCK(unit); 
        return (rv);
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        /* Enable/Disable egress true mirroring. */
        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            rv = bcm_esw_mirror_port_dest_add(unit, port,
                                              BCM_MIRROR_PORT_EGRESS_TRUE,
                                              mirror_dest.mirror_dest_id); 
            if (BCM_E_EXISTS == rv) {
                /* Since this function is set, not add, it is sufficient if
                 * the destination already exists.  Clear the error. */
                rv = BCM_E_NONE;
            }
        } else {
            rv = bcm_esw_mirror_port_dest_delete(unit, port,
                                                 BCM_MIRROR_PORT_EGRESS_TRUE,
                                                 mirror_dest.mirror_dest_id); 
            if (BCM_E_NOT_FOUND == rv) {
                /* There is no clean way to identify delete. -> 
                   if destination is not found assume success. */ 
                rv = BCM_E_NONE;
            } 
        }

        if (BCM_FAILURE(rv)) {
            /* Delete unused mirror destination. */
            if (destroy_flag) {
                (void)bcm_esw_mirror_destination_destroy(unit,
                                                 mirror_dest.mirror_dest_id);
            }
            MIRROR_UNLOCK(unit); 
            return (rv);
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */  

    /* Delete unused mirror destination. */
    if (1 >= MIRROR_DEST_REF_COUNT(unit, mirror_dest.mirror_dest_id)) {
        rv = bcm_esw_mirror_destination_destroy(unit, mirror_dest.mirror_dest_id);
    }

    MIRROR_UNLOCK(unit); 
    return (rv);
}
/*
 * Function:
 *      bcm_esw_mirror_port_get
 * Description:
 *      Get mirroring configuration for a port
 * Parameters:
 *      unit      - BCM device number
 *      port      - port to get configuration for
 *      dest_mod  - (OUT) module id of mirror-to port
 *      dest_port - (OUT) mirror-to port
 *      flags     - (OUT) BCM_MIRROR_PORT_* flags
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_mirror_port_get(int unit, bcm_port_t port,
                        bcm_module_t *dest_mod, bcm_port_t *dest_port,
                        uint32 *flags)
{
    bcm_gport_t mirror_dest_id;               /* Mirror destination  id.   */
    int enable = 0;                           /* Egress mirror is enabled. */
    int rv;                                   /* Operation return status.  */
    int mirror_dest_count = 0;                /* Mirror destination found. */
    int isGport;                              /* gport indicator */  
    bcm_mirror_destination_t    mirror_dest;  /* mirror destination struct */       
    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if ((NULL == flags) || (NULL == dest_mod) || (NULL == dest_port)) {
        return (BCM_E_PARAM);
    }

    bcm_mirror_destination_t_init(&mirror_dest);

    if (BCM_GPORT_IS_SET(port)) {
#ifdef BCM_TRIDENT2PLUS_SUPPORT
        if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
            _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
        } else
#endif
        {
            BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
        }
    }

    *flags = 0;

    /* Check if directed mirroring is enabled and gport required. */
    BCM_IF_ERROR_RETURN(bcm_esw_switch_control_get(unit, bcmSwitchUseGport, 
                                                   &isGport));
    
    MIRROR_LOCK(unit);

    /* Read port ingress mirroring destination ports. */
    rv = bcm_esw_mirror_port_dest_get(unit, port, BCM_MIRROR_PORT_INGRESS, 
                                      1, &mirror_dest_id,
                                      &mirror_dest_count);
    if (BCM_FAILURE(rv)) {
        MIRROR_UNLOCK(unit);
        return (rv);
    }

    if (mirror_dest_count) {
        rv = bcm_esw_mirror_destination_get(unit, mirror_dest_id,
                                            &mirror_dest);

        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }
        *flags |= BCM_MIRROR_PORT_INGRESS;

         /* Read mtp egress enable bitmap for source port. */
        rv = _bcm_esw_mirror_egress_get(unit, port, &enable);
        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }
        if (enable) {
            *flags |= BCM_MIRROR_PORT_EGRESS;
        } else {
#ifdef BCM_TRIUMPH2_SUPPORT
            if (soc_feature(unit, soc_feature_egr_mirror_true)) {
                rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_EGRESS_TRUE,
                                                  mirror_dest_id);
                if (BCM_E_EXISTS == rv) {
                    *flags |= BCM_MIRROR_PORT_EGRESS_TRUE;
                }
                rv = BCM_E_NONE;
            }
#endif
        }

        MIRROR_UNLOCK(unit);

        if (isGport) {
            *dest_port = mirror_dest.gport;
        } else if (BCM_GPORT_IS_TRUNK(mirror_dest.gport)) {
            *flags |= BCM_MIRROR_PORT_DEST_TRUNK;
            *dest_port = BCM_GPORT_TRUNK_GET(mirror_dest.gport);
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_mirror_gport_resolve(unit, mirror_dest.gport,
                                          dest_port, dest_mod));
            BCM_IF_ERROR_RETURN(
                _bcm_gport_modport_hw2api_map(unit, *dest_mod, *dest_port, 
                                              dest_mod, dest_port));
        }

        return (BCM_E_NONE);
    }

    /* Read port egress mirroring destination ports. */
    rv = bcm_esw_mirror_port_dest_get(unit, port, BCM_MIRROR_PORT_EGRESS, 
                                      1, &mirror_dest_id,
                                      &mirror_dest_count);

    if (BCM_FAILURE(rv)) {
        MIRROR_UNLOCK(unit);
        return (rv);
    }

    if (mirror_dest_count) {
        rv = bcm_esw_mirror_destination_get(unit, mirror_dest_id,
                                            &mirror_dest);

        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }
        *flags |= BCM_MIRROR_PORT_EGRESS;

        MIRROR_UNLOCK(unit);

        if (isGport) {
            *dest_port = mirror_dest.gport;
        } else if (BCM_GPORT_IS_TRUNK(mirror_dest.gport)) {
            *flags |= BCM_MIRROR_PORT_DEST_TRUNK;
            *dest_port = BCM_GPORT_TRUNK_GET(mirror_dest.gport);
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_mirror_gport_resolve(unit, mirror_dest.gport,
                                          dest_port, dest_mod));
            BCM_IF_ERROR_RETURN(
                _bcm_gport_modport_hw2api_map(unit, *dest_mod, *dest_port, 
                                              dest_mod, dest_port));
        }

        return (BCM_E_NONE);
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        /* Read port ingress mirroring destination ports. */
        rv = bcm_esw_mirror_port_dest_get(unit, port,
                                          BCM_MIRROR_PORT_EGRESS_TRUE, 
                                          1, &mirror_dest_id,
                                          &mirror_dest_count);

        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }

        if (mirror_dest_count) {
            rv = bcm_esw_mirror_destination_get(unit, mirror_dest_id,
                                                &mirror_dest);

            if (BCM_FAILURE(rv)) {
                MIRROR_UNLOCK(unit);
                return (rv);
            }
            *flags |= BCM_MIRROR_PORT_EGRESS_TRUE;

            MIRROR_UNLOCK(unit);

            if (isGport) {
                *dest_port = mirror_dest.gport;
            } else if (BCM_GPORT_IS_TRUNK(mirror_dest.gport)) {
                *flags |= BCM_MIRROR_PORT_DEST_TRUNK;
                *dest_port = BCM_GPORT_TRUNK_GET(mirror_dest.gport);
            } else {
                BCM_IF_ERROR_RETURN
                    (_bcm_mirror_gport_resolve(unit, mirror_dest.gport,
                                               dest_port, dest_mod));
                BCM_IF_ERROR_RETURN
                    (_bcm_gport_modport_hw2api_map(unit, *dest_mod, *dest_port, 
                                                   dest_mod, dest_port));
            }

            return (BCM_E_NONE);
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */  

    MIRROR_UNLOCK(unit);

    return (BCM_E_NONE);
}

STATIC int
_bcm_mirror_sflow_dest_delete(int unit, uint32 flags, bcm_gport_t mirror_dest) 
{
#if defined(BCM_TOMAHAWK_SUPPORT)
    int mc_enable;              /* Mirror enable check.     */
    int mtp_index = -1;           /* Mirror to port index.    */
    uint32 reg_val;
    int mtp_slot;
    soc_field_t mtp_idxf[BCM_MIRROR_MTP_COUNT] = {
        MTP_INDEX0f,MTP_INDEX1f,MTP_INDEX2f,MTP_INDEX3f};

    if (!soc_feature(unit, soc_feature_sflow_ing_mirror)) {
        return BCM_E_UNAVAIL;
    }

    if (!soc_feature(unit, soc_feature_mirror_flexible)) {
        return BCM_E_UNAVAIL;
    }

    /* only support BCM_MIRROR_PORT_INGRESS */
    if (flags & ~(BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_SFLOW)) {
        return BCM_E_UNAVAIL;
    }

    /* Look for used MTP index */
    if (!MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_tr2_mirror_shared_mtp_match(unit, 
                     mirror_dest, FALSE, &mtp_index));
    } else {
        BCM_IF_ERROR_RETURN(_bcm_esw_mirror_ingress_mtp_match(unit, 
                     mirror_dest, &mtp_index));
    }

    BCM_IF_ERROR_RETURN(READ_SFLOW_ING_MIRROR_CONFIGr(unit, &reg_val));
    mc_enable = soc_reg_field_get(unit, SFLOW_ING_MIRROR_CONFIGr,
                                          reg_val, MIRROR_ENABLEf);

    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_xgs3_mtp_index_port_slot_get(unit, 0, 
             mc_enable, FALSE, mtp_index, BCM_MTP_SLOT_TYPE_SFLOW,  &mtp_slot));
    } else {
        /* Otherwise the slot and index is 1-1 */
        mtp_slot = mtp_index;
    }

    if (mc_enable & (1 << mtp_slot)) {
        mc_enable &= ~(1 << mtp_slot);
        soc_reg_field_set(unit, SFLOW_ING_MIRROR_CONFIGr,
                                          &reg_val, MIRROR_ENABLEf,mc_enable);
        soc_reg_field_set(unit, SFLOW_ING_MIRROR_CONFIGr, &reg_val, 
                    mtp_idxf[mtp_slot], 0);
        BCM_IF_ERROR_RETURN(WRITE_SFLOW_ING_MIRROR_CONFIGr(unit, reg_val));
    }

    if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_xgs3_mtp_type_slot_unreserve(unit,
                                                   BCM_MIRROR_PORT_INGRESS,
                                                   0,
                                                   BCM_MTP_SLOT_TYPE_SFLOW,
                                                   mtp_index));
    }

    /* Free MTP index for mirror destination. */
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_mtp_unreserve(unit, mtp_index, TRUE,
                                        BCM_MIRROR_PORT_INGRESS));
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}

STATIC int
_bcm_mirror_sflow_dest_get(int unit, uint32 flags, int mirror_dest_size, 
                 bcm_gport_t *mirror_dest, int *mirror_dest_count) 
{
#if defined(BCM_TOMAHAWK_SUPPORT)
    uint32 reg_val;
    int mc_enable;
    int mtp_index;
    int index;                      
    int mtp_slot, mtp_bit; 
    int i;
    uint32 index_val[BCM_MIRROR_MTP_COUNT];
    soc_field_t mtp_idxf[BCM_MIRROR_MTP_COUNT] = {
        MTP_INDEX0f,MTP_INDEX1f,MTP_INDEX2f,MTP_INDEX3f};

    if (!soc_feature(unit, soc_feature_sflow_ing_mirror)) {
        return BCM_E_UNAVAIL;
    }
    /* only support BCM_MIRROR_PORT_INGRESS */
    if (flags & ~(BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_SFLOW)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(READ_SFLOW_ING_MIRROR_CONFIGr(unit, &reg_val));
    mc_enable = soc_reg_field_get(unit, SFLOW_ING_MIRROR_CONFIGr,
                                          reg_val, MIRROR_ENABLEf);
    *mirror_dest_count = 0;

    /* Read mirror control structure to get programmed mtp indexes. */
    for (i = 0; i < BCM_MIRROR_MTP_COUNT; i++) {
        index_val[i] = soc_reg_field_get(unit, SFLOW_ING_MIRROR_CONFIGr,
                                          reg_val, mtp_idxf[i]);
    }
    index = 0;
    /* Read mirror control register to compare programmed mtp indexes. */
    BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
        mtp_bit = 1 << mtp_slot;
        /* Check if MTP slot is enabled on port */
        if (!(mc_enable & mtp_bit)) {
            continue;
        }
        /* MTP index is enabled and direction matches */
        mtp_index = index_val[mtp_slot];

        if ( MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            mirror_dest[index] =
                    MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index);
        } else {
            mirror_dest[index] =
                MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index);
        }
        index++;
    }
    *mirror_dest_count = index;
   
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}

STATIC int
_bcm_mirror_sflow_dest_add(int unit, uint32 flags, bcm_gport_t mirror_dest) 
{
#if defined(BCM_TOMAHAWK_SUPPORT)
    int rv;
    uint32 reg_val;
    int i;
    int mc_enable;
    int mtp_index;
    soc_field_t mtp_idxf[BCM_MIRROR_MTP_COUNT] = {
            MTP_INDEX0f,MTP_INDEX1f,MTP_INDEX2f,MTP_INDEX3f};
    bcm_gport_t  mirror_dest_list[BCM_MIRROR_MTP_COUNT];
    int          mirror_dest_count;
    int mtp_slot;          /* MTP type value              */
    uint32       replace_flag = 0;
    bcm_mirror_destination_t mirr_dest;

    if (!soc_feature(unit, soc_feature_sflow_ing_mirror)) {
        return BCM_E_UNAVAIL;
    }
    if (!soc_feature(unit, soc_feature_mirror_flexible)) {
        return BCM_E_UNAVAIL;
    }

    /* only support BCM_MIRROR_PORT_INGRESS */
    if (flags & ~(BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_SFLOW)) {
        return BCM_E_UNAVAIL;
    }

    rv = bcm_esw_mirror_destination_get(unit, mirror_dest, &mirr_dest);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    replace_flag = (mirr_dest.flags & BCM_MIRROR_DEST_REPLACE) ? 1:0;

    rv = bcm_esw_mirror_port_dest_get(unit, -1,
                        flags,
                        BCM_MIRROR_MTP_COUNT,
                        mirror_dest_list, &mirror_dest_count);
    if (BCM_SUCCESS(rv)) {
        rv = BCM_E_NOT_FOUND;
        for (i = 0; i < mirror_dest_count; i++) {
            if (mirror_dest_list[i] == mirror_dest) {
                rv = BCM_E_EXISTS;
                break;
            }
        }
    }

    if ((BCM_E_NOT_FOUND != rv) &&
        !((BCM_E_EXISTS == rv) && (replace_flag == 1))) {
        return rv;
    }

    /* Allocate MTP index for mirror destination. */
    BCM_IF_ERROR_RETURN(_bcm_esw_mirror_mtp_reserve(unit, mirror_dest,
                                 BCM_MIRROR_PORT_INGRESS, &mtp_index));

    /* The ref_count of mtp_index for same src_port doesn't need to get increased twice */
    if ((BCM_E_EXISTS == rv) && (replace_flag == 1)) {
        if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_ING_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, mtp_index)--;
            }
        } else {
            if ((rv == BCM_E_EXISTS) &&
                (MIRROR_CONFIG_SHARED_MTP_DEST(unit, mtp_index) == mirror_dest)) {
                MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, mtp_index)--;
            }
        }
    }

    rv = READ_SFLOW_ING_MIRROR_CONFIGr(unit, &reg_val);
    if (BCM_SUCCESS(rv)) {
        mc_enable = soc_reg_field_get(unit, SFLOW_ING_MIRROR_CONFIGr,
                                          reg_val, MIRROR_ENABLEf);

        if (MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            rv = _bcm_xgs3_mtp_type_slot_reserve(unit,
                                                 BCM_MIRROR_PORT_INGRESS,
                                                 mc_enable, 0,
                                                 BCM_MTP_SLOT_TYPE_SFLOW,
                                                 mtp_index, &mtp_slot);
        } else {
            /* Otherwise the slot and index is 1-1 */
            mtp_slot = mtp_index;
        }

  
        if (BCM_SUCCESS(rv)) {
            if (mc_enable & (1 << mtp_slot)) {
                if (replace_flag == 1) {
                    rv = BCM_E_NONE;
                } else {
                rv = BCM_E_EXISTS;
                }
            } else {

                mc_enable |= 1 << mtp_slot;
                soc_reg_field_set(unit, SFLOW_ING_MIRROR_CONFIGr, &reg_val,
                            MIRROR_ENABLEf, mc_enable);
                soc_reg_field_set(unit, SFLOW_ING_MIRROR_CONFIGr, &reg_val,
                    mtp_idxf[mtp_slot], mtp_index);
                rv = WRITE_SFLOW_ING_MIRROR_CONFIGr(unit, reg_val);
            }
        } 
    }
    if (BCM_FAILURE(rv)) {
        _bcm_esw_mirror_mtp_unreserve(unit, mtp_index, FALSE, 
                                          BCM_MIRROR_PORT_INGRESS);
    }
    /* Enable mirroring on a port.  */
    if (BCM_SUCCESS(rv)) {
        if(!SOC_IS_XGS3_SWITCH(unit) ||
           (BCM_MIRROR_DISABLE == MIRROR_CONFIG_MODE(unit))) {
            rv = _bcm_esw_mirror_enable(unit);
            MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
        }
    }

    if (BCM_SUCCESS(rv)) {
#ifdef BCM_WARM_BOOT_SUPPORT
        SOC_CONTROL_LOCK(unit);
        SOC_CONTROL(unit)->scache_dirty = 1;
        SOC_CONTROL_UNLOCK(unit);
#endif
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *     bcm_esw_mirror_port_dest_add 
 * Purpose:
 *      Add mirroring destination to a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_esw_mirror_port_dest_add(int unit, bcm_port_t port, 
                              uint32 flags, bcm_gport_t mirror_dest) 
{
    int          rv = BCM_E_NONE;   /* Operation return status.      */
    int          orig_gport; 
    int          vp = VP_PORT_INVALID;
    int          vp_mirror = FALSE;
    bcm_gport_t  mirror_dest_list[BCM_MIRROR_MTP_COUNT];
    int          mirror_dest_count, mtp;
    uint32       replace_flag = 0;
    bcm_mirror_destination_t mirr_dest;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }
    orig_gport = port;

    if (-1 != port) { 
        if (BCM_GPORT_IS_SET(port)) {

            rv = _bcm_tr2_mirror_vp_port_get(unit, port,
                                             &vp, &port);
            if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                return rv;
            }
            rv = BCM_E_NONE;
            
            if (vp == VP_PORT_INVALID) {
#ifdef BCM_TRIDENT2PLUS_SUPPORT
                if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
                    _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
                } else
#endif
                {
                    BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
                }
            }
        }

        if (vp == VP_PORT_INVALID && !BCM_GPORT_IS_SET(port)) {
            if (!SOC_PORT_VALID(unit, port)) {
                return (BCM_E_PORT);
            }

            if (IS_CPU_PORT(unit, port) &&
                !soc_feature(unit, soc_feature_cpuport_mirror)) {
                return (BCM_E_PORT);
            }
        }
    }

    if (!soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return (BCM_E_PARAM);
    }

    if (!(flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS |
                   BCM_MIRROR_PORT_EGRESS_TRUE))) { 
        return (BCM_E_PARAM);
    }

    if (0 == BCM_GPORT_IS_MIRROR(mirror_dest)) {
        return (BCM_E_PARAM);
    }

    MIRROR_LOCK(unit);

    /* If destination is not valid */
    if (0 == MIRROR_DEST_REF_COUNT(unit, mirror_dest)) {
       MIRROR_UNLOCK(unit);
       return (BCM_E_NOT_FOUND);
    }

    MIRROR_UNLOCK(unit);

    if (flags & BCM_MIRROR_PORT_SFLOW) {
        MIRROR_LOCK(unit);
        rv = _bcm_mirror_sflow_dest_add(unit, flags, mirror_dest);
        MIRROR_UNLOCK(unit);
        return rv;
    }

    /* check supported conditions for vp mirroring */
    if (vp != VP_PORT_INVALID) {
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_flexible) && 
            !MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit) &&
            (!(flags & BCM_MIRROR_PORT_EGRESS_TRUE))) {
            vp_mirror = TRUE;
        }
#endif
        if (vp_mirror == FALSE) {
            return BCM_E_UNAVAIL;
        }
    }

    /* Directed  mirroring support check. */
    if (MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
        /* No mirroring to a trunk. */
        if (BCM_GPORT_IS_TRUNK(MIRROR_DEST_GPORT(unit, mirror_dest))) {
            return (BCM_E_UNAVAIL);
        } 

        if (soc_feature(unit, soc_feature_mirror_flexible)) {
            if (0 != (flags & BCM_MIRROR_PORT_INGRESS)) {
                if (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, 0)) {
                    if (MIRROR_CONFIG_SHARED_MTP_DEST(unit, 0) !=
                        mirror_dest) {
                        return (BCM_E_RESOURCE);
                    }
                }
            }
            if (0 != (flags & BCM_MIRROR_PORT_EGRESS)) {
                if (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit,
                           BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX)) {
                    if (MIRROR_CONFIG_SHARED_MTP_DEST(unit,
                               BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX) !=
                        mirror_dest) {
                        return (BCM_E_RESOURCE);
                    }
                }
            }
        } else {
            /* Single mirroring destination for ingress & egress. */
            if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)) {
                if (MIRROR_CONFIG_ING_MTP_DEST(unit, 0) != mirror_dest) {
                    return (BCM_E_RESOURCE);
                }
            }
            if (MIRROR_CONFIG_EGR_MTP_REF_COUNT(unit, 0)) {
                if (MIRROR_CONFIG_EGR_MTP_DEST(unit, 0) != mirror_dest) {
                    return (BCM_E_RESOURCE);
                }
            }
        }

        /* Some devices do not support non-directed mode */
        if (soc_feature(unit, soc_feature_directed_mirror_only)) {
            return (BCM_E_CONFIG);
        }

        /* True egress mode does not support non directed mirroring */
        if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
            return (BCM_E_CONFIG);
        }
    }

    MIRROR_LOCK(unit);

    rv = bcm_esw_mirror_destination_get(unit, mirror_dest, &mirr_dest);
    if (BCM_FAILURE(rv)) {
        MIRROR_UNLOCK(unit);
        return (rv);
    }

    replace_flag = (mirr_dest.flags & BCM_MIRROR_DEST_REPLACE) ? 1:0;

    if (flags & BCM_MIRROR_PORT_INGRESS) {
        if (!MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
            /* Is this port to MTP already configured? */
            if (vp == VP_PORT_INVALID) {
                rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_INGRESS,
                                                  mirror_dest);
            } else {
                rv = bcm_esw_mirror_port_dest_get(unit, orig_gport, 
                                      BCM_MIRROR_PORT_INGRESS,
                                      BCM_MIRROR_MTP_COUNT,
                                      mirror_dest_list, &mirror_dest_count);
                if (BCM_SUCCESS(rv)) {
                    rv = BCM_E_NOT_FOUND;
                    for (mtp = 0; mtp < mirror_dest_count; mtp++) {
                        if (mirror_dest_list[mtp] == mirror_dest) {
                            rv = BCM_E_EXISTS;
                            break;
                        }
                    }
                }
            }
          
            if ((BCM_E_NOT_FOUND != rv) && 
                !((BCM_E_EXISTS == rv) && (replace_flag == 1 || mirr_dest.flags
                                           & BCM_MIRROR_DEST_ID_SHARE))) {
                    MIRROR_UNLOCK(unit);
                    return rv;
            } 
        }

        rv = _bcm_esw_mirror_port_ingress_dest_add(
                unit, 
                (vp == VP_PORT_INVALID) ? port : orig_gport,
                mirror_dest);
    }

    if (BCM_SUCCESS(rv) && (flags & BCM_MIRROR_PORT_EGRESS)) {
        if (!MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
            /* Is this port to MTP already configured? */
            if (vp == VP_PORT_INVALID) {
                rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                                  BCM_MIRROR_PORT_EGRESS,
                                                  mirror_dest);
            } else {
                rv = bcm_esw_mirror_port_dest_get(unit, orig_gport, 
                                      BCM_MIRROR_PORT_EGRESS,
                                      BCM_MIRROR_MTP_COUNT,
                                      mirror_dest_list, &mirror_dest_count);
                if (BCM_SUCCESS(rv)) {
                    rv = BCM_E_NOT_FOUND;
                    for (mtp = 0; mtp < mirror_dest_count; mtp++) {
                        if (mirror_dest_list[mtp] == mirror_dest) {
                            rv = BCM_E_EXISTS;
                            break;
                        }
                    }
                }
            }
            if ((BCM_E_NOT_FOUND == rv) || 
                ((BCM_E_EXISTS == rv) && (replace_flag == 1 || mirr_dest.flags
                                          & BCM_MIRROR_DEST_ID_SHARE))) {
                rv = BCM_E_NONE;
            }
        }

        if (BCM_SUCCESS(rv)) {
            rv = _bcm_esw_mirror_port_egress_dest_add(
                    unit,
                    (vp == VP_PORT_INVALID) ? port : orig_gport,
                    mirror_dest);
        }

        /* Check for operation failure. */
        if (BCM_FAILURE(rv)) {
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                _bcm_esw_mirror_port_ingress_dest_delete(
                    unit,
                    (vp == VP_PORT_INVALID) ? port : orig_gport,
                    mirror_dest);
            }
        }
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (BCM_SUCCESS(rv) && (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        if (!MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
            /* Is this port to MTP already configured? */
            rv = _bcm_esw_mirror_port_dest_search(unit, port,
                                           BCM_MIRROR_PORT_EGRESS_TRUE,
                                                  mirror_dest);
            if (BCM_E_NOT_FOUND == rv) {
                rv = BCM_E_NONE;
            }
        }

        if (BCM_SUCCESS(rv)) {
            rv = _bcm_esw_mirror_port_egress_true_dest_add(unit, port,
                                                           mirror_dest);
        }

        /* Check for operation failure. */
        if (BCM_FAILURE(rv)) {
            if (flags & BCM_MIRROR_PORT_INGRESS) {
                _bcm_esw_mirror_port_ingress_dest_delete(
                    unit,
                    (vp == VP_PORT_INVALID) ? port : orig_gport,
                    mirror_dest);
            }
            if (flags & BCM_MIRROR_PORT_EGRESS) {
                _bcm_esw_mirror_port_egress_dest_delete(
                    unit, 
                    (vp == VP_PORT_INVALID) ? port : orig_gport,
                    mirror_dest);
            }
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */  
  

    /* Update stacking mirror destination bitmap. */
    if (vp == VP_PORT_INVALID) {
        if (BCM_SUCCESS(rv) && (-1 != port) &&
            !BCM_GPORT_IS_SET(port) && IS_ST_PORT(unit, port)) {
            rv = _bcm_esw_mirror_stacking_dest_update
                                 (unit, port, MIRROR_DEST_GPORT(unit, mirror_dest));
            /* Check for operation failure. */
            if (BCM_FAILURE(rv)) {
                if (flags & BCM_MIRROR_PORT_INGRESS) {
                    _bcm_esw_mirror_port_ingress_dest_delete(unit, port,
                                                             mirror_dest);
                }
                if (flags & BCM_MIRROR_PORT_EGRESS) {
                    _bcm_esw_mirror_port_egress_dest_delete(unit, port,
                                                            mirror_dest);
                }
#ifdef BCM_TRIUMPH2_SUPPORT
                if (flags & BCM_MIRROR_PORT_EGRESS_TRUE) {
                    _bcm_esw_mirror_port_egress_true_dest_delete(unit, port,
                                                             mirror_dest);
                }
#endif /* BCM_TRIUMPH2_SUPPORT */  
            }
        }
    }

    /* Enable mirroring on a port.  */
    if (BCM_SUCCESS(rv)) { 
        if(!SOC_IS_XGS3_SWITCH(unit) || 
           (BCM_MIRROR_DISABLE == MIRROR_CONFIG_MODE(unit))) {
            rv = _bcm_esw_mirror_enable(unit);
            MIRROR_CONFIG_MODE(unit) = BCM_MIRROR_L2;
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    MIRROR_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *     bcm_esw_mirror_port_dest_delete
 * Purpose:
 *      Remove mirroring destination from a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_esw_mirror_port_dest_delete(int unit, bcm_port_t port, 
                                uint32 flags, bcm_gport_t mirror_dest) 
{
    int final_rv = BCM_E_NONE;      /* Operation return status. */
    int rv = BCM_E_NONE;            /* Operation return status. */
    int vp = VP_PORT_INVALID;
    int vp_mirror = FALSE;
    int orig_gport = port; 
    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (-1 != port) { 
        if (BCM_GPORT_IS_SET(port)) {
            rv = _bcm_tr2_mirror_vp_port_get(unit, port,
                                      &vp, &port); 
            if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                return rv;
            }
            rv = BCM_E_NONE;
            
            if (vp == VP_PORT_INVALID) {
#ifdef BCM_TRIDENT2PLUS_SUPPORT
                if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
                    _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
                } else
#endif
                {
                    BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
                }
            }
        }

        if (vp == VP_PORT_INVALID && !BCM_GPORT_IS_SET(port)) {
            if (!SOC_PORT_VALID(unit, port)) {
                return (BCM_E_PORT);
            }

            if (IS_CPU_PORT(unit, port) &&
                !soc_feature(unit, soc_feature_cpuport_mirror)) {
                return (BCM_E_PORT);
            }
        }
    }

    if (!soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return (BCM_E_PARAM);
    }

    if (!(flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS |
                   BCM_MIRROR_PORT_EGRESS_TRUE))) { 
        return (BCM_E_PARAM);
    }

    if (0 == BCM_GPORT_IS_MIRROR(mirror_dest)) {
        return (BCM_E_PARAM);
    }

    /* check supported conditions for vp mirroring */
    if (vp != VP_PORT_INVALID) {
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            (!(flags & BCM_MIRROR_PORT_EGRESS_TRUE))) {
            vp_mirror = TRUE;
        }
#endif
        if (vp_mirror == FALSE) {
            return BCM_E_UNAVAIL;
        }
    }

    if (flags & BCM_MIRROR_PORT_SFLOW) {
        MIRROR_LOCK(unit);
        rv = _bcm_mirror_sflow_dest_delete(unit, flags, mirror_dest);
        MIRROR_UNLOCK(unit);
        return rv;
    }

    MIRROR_LOCK(unit);

    if ((flags & BCM_MIRROR_PORT_INGRESS) &&
        (BCM_GPORT_INVALID != mirror_dest)) {
        final_rv = _bcm_esw_mirror_port_ingress_dest_delete(
                       unit, 
                       (vp == VP_PORT_INVALID) ? port : orig_gport,
                       mirror_dest);
    }

    if ((flags & BCM_MIRROR_PORT_EGRESS) &&
        (BCM_GPORT_INVALID != mirror_dest)) {
        rv = _bcm_esw_mirror_port_egress_dest_delete(
                unit,
                (vp == VP_PORT_INVALID) ? port : orig_gport,
                mirror_dest);
        if (!BCM_FAILURE(final_rv)) {
            final_rv = rv;
        }
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if ((flags & BCM_MIRROR_PORT_EGRESS_TRUE) &&
        (BCM_GPORT_INVALID != mirror_dest)) {
        rv = _bcm_esw_mirror_port_egress_true_dest_delete(unit, port,
                                                          mirror_dest);
        if (!BCM_FAILURE(final_rv)) {
            final_rv = rv;
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /* Update stacking mirror destination bitmap. */
    if (vp == VP_PORT_INVALID) {
        if ((-1 != port) && !BCM_GPORT_IS_SET(port) && (IS_ST_PORT(unit, port))) {
            rv = _bcm_esw_mirror_stacking_dest_update(unit, port, BCM_GPORT_INVALID);
            if (!BCM_FAILURE(final_rv)) {
                final_rv = rv;
            }
        }
    } 

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    MIRROR_UNLOCK(unit);
    return (final_rv);
}

/*
 * Function:
 *     bcm_esw_mirror_port_dest_get
 * Purpose:
 *     Get port mirroring destinations.   
 * Parameters:
 *     unit             - (IN) BCM device number. 
 *     port             - (IN) Port mirrored port.
 *     flags            - (IN) BCM_MIRROR_PORT_XXX flags.
 *     mirror_dest_size - (IN) Preallocated mirror_dest array size.
 *     mirror_dest      - (OUT)Filled array of port mirroring destinations
 *     mirror_dest_count - (OUT)Actual number of mirroring destinations filled.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_esw_mirror_port_dest_get(int unit, bcm_port_t port, uint32 flags, 
                         int mirror_dest_size, bcm_gport_t *mirror_dest,
                         int *mirror_dest_count)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    int         idx;                    /* Mirror to port iteration index.  */
    int         index = 0;              /* Filled destinations index.       */
    bcm_gport_t mtp_dest[BCM_MIRROR_MTP_COUNT]; /* Mirror destinations array. */
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    int         rv = BCM_E_NONE;        /* Operation return status.         */
    int         vp = VP_PORT_INVALID;
    int         vp_mirror = FALSE;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (flags & BCM_MIRROR_PORT_SFLOW) {
        rv = _bcm_mirror_sflow_dest_get(unit,flags,mirror_dest_size, 
                 mirror_dest, mirror_dest_count);
        return rv;
    }
                
    if (BCM_GPORT_INVALID == port) {
        if (!MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
            return (BCM_E_PORT);
        }
        MIRROR_LOCK(unit);
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
            if (MIRROR_CONFIG_SHARED_MTP_REF_COUNT(unit, 0)) {
                if(NULL != mirror_dest) {
                    mirror_dest[0] = MIRROR_CONFIG_SHARED_MTP_DEST(unit, 0);
                }
                *mirror_dest_count = 1;    
            } else {
                if(NULL != mirror_dest) {
                    mirror_dest[0] = BCM_GPORT_INVALID;
                }
                *mirror_dest_count = 0; 
            }   
        } else {
            if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)) {
                if(NULL != mirror_dest) {
                    mirror_dest[0] = MIRROR_CONFIG_ING_MTP_DEST(unit, 0);
                }
                *mirror_dest_count = 1;    
            } else {
                if(NULL != mirror_dest) {
                    mirror_dest[0] = BCM_GPORT_INVALID;
                }
                *mirror_dest_count = 0;    
            }
        }
        MIRROR_UNLOCK(unit);
        return BCM_E_NONE;
    }

    /* Input parameters check. */
    if (BCM_GPORT_IS_SET(port)) {
        rv = _bcm_tr2_mirror_vp_port_get(unit, port,
                                      &vp, &port);
        if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
        rv = BCM_E_NONE; 
        
        if (vp == VP_PORT_INVALID) {
#ifdef BCM_TRIDENT2PLUS_SUPPORT
            if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
                _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
            } else
#endif
            {
                BCM_IF_ERROR_RETURN(bcm_esw_port_local_get(unit, port, &port));
            }
        }
    }

    if (vp == VP_PORT_INVALID && !BCM_GPORT_IS_SET(port)) {
        if (!SOC_PORT_VALID(unit, port)) {
            return (BCM_E_PORT);
        }

        if (IS_CPU_PORT(unit, port) &&
            !soc_feature(unit, soc_feature_cpuport_mirror)) {
            return (BCM_E_PORT);
        }
    }

    if ((0 != mirror_dest_size) && (NULL == mirror_dest)) {
        return (BCM_E_PARAM);
    }

    if (NULL == mirror_dest_count) {
        return (BCM_E_PARAM);
    }

    if (!soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return (BCM_E_PARAM);
    }

    if (!(flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS |
                   BCM_MIRROR_PORT_EGRESS_TRUE))) {
        return (BCM_E_PARAM);
    }

    /* check supported conditions for vp mirroring */
    if (vp != VP_PORT_INVALID) {
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_mirror_flexible) &&
            (!(flags & BCM_MIRROR_PORT_EGRESS_TRUE))) {
            vp_mirror = TRUE;
        }
#endif
        if (vp_mirror == FALSE) {
            return BCM_E_UNAVAIL;
        }
    }

    MIRROR_LOCK(unit);

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit))  {
        if (flags & BCM_MIRROR_PORT_INGRESS) {
            rv = _bcm_xgs3_mirror_port_ingress_dest_get(unit, port, 
                                                        BCM_MIRROR_MTP_COUNT,
                                                        mtp_dest, vp);
            if (BCM_SUCCESS(rv)) {
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) { 
                    if ((index < mirror_dest_size) && 
                        (BCM_GPORT_INVALID != mtp_dest[idx])) {
                        if(NULL != (mirror_dest + index)) {
                            mirror_dest[index] = mtp_dest[idx];
                        }
                        index++;
                    }
                }
            }
        }
        if ((flags & BCM_MIRROR_PORT_EGRESS) &&
            (index <  mirror_dest_size)) {
            rv = _bcm_xgs3_mirror_port_egress_dest_get(unit, port, 
                                                       BCM_MIRROR_MTP_COUNT,
                                                       mtp_dest, vp);
            if (BCM_SUCCESS(rv)) {
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) { 
                    if ((index < mirror_dest_size) && 
                        (BCM_GPORT_INVALID != mtp_dest[idx])) {
                        if(NULL != (mirror_dest + index)) {
                            mirror_dest[index] = mtp_dest[idx];
                        }
                        index++;
                    }
                }
            }
        }
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_egr_mirror_true) &&
            (flags & BCM_MIRROR_PORT_EGRESS_TRUE) &&
            (index <  mirror_dest_size)) {
            rv = _bcm_tr2_mirror_port_egress_true_dest_get(unit, port, 
                                                       BCM_MIRROR_MTP_COUNT,
                                                       mtp_dest);
            if (BCM_SUCCESS(rv)) {
                for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) { 
                    if ((index < mirror_dest_size) && 
                        (BCM_GPORT_INVALID != mtp_dest[idx])) {
                        if(NULL != (mirror_dest + index)) {
                            mirror_dest[index] = mtp_dest[idx];
                        }
                        index++;
                    }
                }
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        *mirror_dest_count = index;
    } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    {
        if (MIRROR_CONFIG_ING_MTP_REF_COUNT(unit, 0)) {
            if(NULL != mirror_dest) {
                mirror_dest[0] = MIRROR_CONFIG_ING_MTP_DEST(unit, 0);
            }
            *mirror_dest_count = 1;    
        } else {
            if(NULL != mirror_dest) {
                mirror_dest[0] = BCM_GPORT_INVALID;
            }
            *mirror_dest_count = 0;    
        }
    }

    MIRROR_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *     bcm_esw_mirror_port_dest_delete_all
 * Purpose:
 *      Remove all mirroring destinations from a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_esw_mirror_port_dest_delete_all(int unit, bcm_port_t port, uint32 flags) 
{
    bcm_gport_t mirror_dest[BCM_MIRROR_MTP_COUNT];
    int         mirror_dest_count;
    int         index;
    int         rv; 
    int vp = VP_PORT_INVALID;
    int orig_gport = port;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (!(flags & (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS |
                   BCM_MIRROR_PORT_EGRESS_TRUE))) { 
        return (BCM_E_PARAM);
    }

    MIRROR_LOCK(unit);

    if (-1 != port) {
        if (BCM_GPORT_IS_SET(port)) {
            rv = _bcm_tr2_mirror_vp_port_get(unit, port, &vp, &port);
            if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                MIRROR_UNLOCK(unit);
                return rv;
            }
            rv = BCM_E_NONE;
        }
    }

    if (flags & BCM_MIRROR_PORT_INGRESS) {
        if (-1 != port) {
            if (BCM_GPORT_IS_SET(port)) {
                if (vp == VP_PORT_INVALID) {
#ifdef BCM_TRIDENT2PLUS_SUPPORT
                    if (soc_feature(unit, soc_feature_hgproxy_subtag_coe) &&
                        _BCM_COE_GPORT_IS_SUBTAG_SUBPORT_PORT(unit, port)) {
                    } else
#endif
                    {
                        rv = bcm_esw_port_local_get(unit, port, &port);
                        if (BCM_FAILURE(rv))
                        {
                            MIRROR_UNLOCK(unit);
                            return rv;
                        }
                    }
                }
            }

            if(vp == VP_PORT_INVALID && !BCM_GPORT_IS_SET(port) &&
               !SOC_PORT_VALID(unit, port)) {
                MIRROR_UNLOCK(unit);
                return (BCM_E_PORT);
            }

            /* Read port ingress mirroring destination ports. */
            rv = bcm_esw_mirror_port_dest_get(
                     unit, (vp == VP_PORT_INVALID) ? port : orig_gport,
                     BCM_MIRROR_PORT_INGRESS, BCM_MIRROR_MTP_COUNT, mirror_dest,
                     &mirror_dest_count);
        } else {
            /* Get all ingress mirror destinations. */
            rv = _bcm_mirror_dest_get_all(unit, BCM_MIRROR_PORT_INGRESS, 
                                         BCM_MIRROR_MTP_COUNT, mirror_dest, 
                                         &mirror_dest_count);
        }
        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }

        /* Remove all ingress mirroring destination ports. */
        for (index = 0; index < mirror_dest_count; index++) {
            rv = bcm_esw_mirror_port_dest_delete(
                     unit, (vp == VP_PORT_INVALID) ? port : orig_gport,
                     BCM_MIRROR_PORT_INGRESS, mirror_dest[index]);
            if (BCM_FAILURE(rv)) {
                MIRROR_UNLOCK(unit);
                return (rv);
            }
        }
    }

    if (flags & BCM_MIRROR_PORT_EGRESS) {
        /* Read port egress mirroring destination ports. */
        if (-1 != port) {
            if (BCM_GPORT_IS_SET(port)) {
                if (vp == VP_PORT_INVALID) {
                    rv = bcm_esw_port_local_get(unit, port, &port);
                    if (BCM_FAILURE(rv))
                    {
                        MIRROR_UNLOCK(unit);
                        return rv;
                    }
                }
            }
            if(vp == VP_PORT_INVALID && !SOC_PORT_VALID(unit, port)) {
                MIRROR_UNLOCK(unit);
                return (BCM_E_PORT);
            }

            rv = bcm_esw_mirror_port_dest_get(
                     unit, (vp == VP_PORT_INVALID) ? port : orig_gport,
                     BCM_MIRROR_PORT_EGRESS, BCM_MIRROR_MTP_COUNT, mirror_dest,
                     &mirror_dest_count);
        } else {
            /* Get all egress mirror destinations. */
            rv = _bcm_mirror_dest_get_all(unit, BCM_MIRROR_PORT_EGRESS, 
                                          BCM_MIRROR_MTP_COUNT, mirror_dest, 
                                          &mirror_dest_count);
        }
        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }

        /* Remove all egress mirroring destination ports. */
        for (index = 0; index < mirror_dest_count; index++) {
            rv = bcm_esw_mirror_port_dest_delete(
                     unit, (vp == VP_PORT_INVALID) ? port : orig_gport,
                     BCM_MIRROR_PORT_EGRESS, mirror_dest[index]);
            if (BCM_FAILURE(rv)) {
                MIRROR_UNLOCK(unit);
                return (rv);
            }
        }
    }

    if (soc_feature(unit, soc_feature_egr_mirror_true) &&
        (flags & BCM_MIRROR_PORT_EGRESS_TRUE)) {
        /* Read port egress true mirroring destination ports. */
        if (-1 != port) {
            if (BCM_GPORT_IS_SET(port)) {
                rv = bcm_esw_port_local_get(unit, port, &port);
                if (BCM_FAILURE(rv))
                {
                    MIRROR_UNLOCK(unit);
                    return rv;
                }
            }
            if(!SOC_PORT_VALID(unit, port)) {
                MIRROR_UNLOCK(unit);
                return (BCM_E_PORT);
            }

            rv = bcm_esw_mirror_port_dest_get(unit, port,
                                              BCM_MIRROR_PORT_EGRESS_TRUE, 
                                              BCM_MIRROR_MTP_COUNT,
                                              mirror_dest, 
                                              &mirror_dest_count);
        } else {
            /* Get all egress mirror destinations. */
            rv = _bcm_mirror_dest_get_all(unit, BCM_MIRROR_PORT_EGRESS_TRUE, 
                                          BCM_MIRROR_MTP_COUNT, mirror_dest, 
                                          &mirror_dest_count);
        }
        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return (rv);
        }

        /* Remove all egress mirroring destination ports. */
        for (index = 0; index < mirror_dest_count; index++) {
            rv = bcm_esw_mirror_port_dest_delete(unit, port,
                                                 BCM_MIRROR_PORT_EGRESS_TRUE,
                                                 mirror_dest[index]); 
            if (BCM_FAILURE(rv)) {
                MIRROR_UNLOCK(unit);
                return (rv);
            }
        }
    }

    MIRROR_UNLOCK(unit);
    return (BCM_E_NONE);
}

/*
 * Function:
 *     bcm_esw_mirror_destination_create
 * Purpose:
 *     Create mirror destination description.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      mirror_dest  - (IN) Mirror destination description.
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_esw_mirror_destination_create(int unit, bcm_mirror_destination_t *mirror_dest) 
{
    bcm_module_t mymodid;           /* Local module id.              */
    bcm_module_t dest_mod;          /* Destination module id.        */
    bcm_port_t   dest_port;         /* Destination port number.      */
    bcm_mirror_destination_t  mirror_dest_check;
    int rv;   /* Operation return status. */
#ifdef BCM_TOMAHAWK_SUPPORT
    int match_idx;
#endif /* BCM_TOMAHAWK_SUPPORT */
    uint8 do_gport_check = TRUE;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if (NULL == mirror_dest) {
        return (BCM_E_PARAM);
    }

    if ((mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_WITH_SPAN_ID) ||
        (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_RSPAN)) {
        return (BCM_E_PARAM);
    }

    /* Check if device supports advanced mirroring mode. */
    if (mirror_dest->flags & (BCM_MIRROR_DEST_TUNNEL_IP_GRE |
                              BCM_MIRROR_DEST_PAYLOAD_UNTAGGED)) {

        if (0 == SOC_MEM_IS_VALID(unit, EGR_ERSPANm) && 
            0 == SOC_MEM_IS_VALID(unit, EGR_MIRROR_ENCAP_CONTROLm)) {
            return (BCM_E_UNAVAIL);
        }

        /* Bypass mode is enabled check. */
        if (SOC_MEM_IS_VALID(unit, EGR_ERSPANm) &&
            0 == soc_mem_index_count(unit, EGR_ERSPANm)) {
            return (BCM_E_UNAVAIL);
        }
        if (SOC_MEM_IS_VALID(unit, EGR_MIRROR_ENCAP_CONTROLm) && 
            0 == soc_mem_index_count(unit, EGR_MIRROR_ENCAP_CONTROLm)) {
            return (BCM_E_UNAVAIL);
        } 
    }
    /* Check if device supports mirroring trill and NIV tunneling. */
    if (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_TRILL) {
        if (!soc_feature(unit, soc_feature_trill)) {
            return (BCM_E_UNAVAIL);
        } else if (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_NIV) {
            /* Only one extra encap allowed. */
            return BCM_E_PARAM;
        } else if (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_ETAG) {
            /* Only one extra encap allowed. */
            return BCM_E_PARAM;    
        } /* else OK */
    }
    if ((mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_NIV)) {
        if(!soc_feature(unit, soc_feature_niv)) {
            return (BCM_E_UNAVAIL);
        } else if (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_ETAG) {
            /* Only one extra encap allowed. */
            return BCM_E_PARAM;    
        } /* else OK */
    }
    if ((mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_ETAG) &&
        !soc_feature(unit, soc_feature_port_extension)) {
        return (BCM_E_UNAVAIL);
    }

    /* Untagging payload supported only on IP tunnels. */
    if ((0 == (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_IP_GRE)) && 
        (mirror_dest->flags & BCM_MIRROR_DEST_PAYLOAD_UNTAGGED)) {
        return (BCM_E_UNAVAIL);
    }

    /* Can't do IP-GRE & L3 tunnel simultaneously. */
    if ((mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_IP_GRE) && 
        (mirror_dest->flags & BCM_MIRROR_DEST_TUNNEL_L2)) {
        return (BCM_E_CONFIG);
    }

    /* Check if device supports assigning Traffic Class to this MTP */
    if ((mirror_dest->flags & BCM_MIRROR_DEST_INT_PRI_SET) &&
        (!(SOC_IS_TD_TT(unit) || SOC_IS_TRIUMPH3(unit) ||
           SOC_IS_KATANAX(unit)))) {
        return BCM_E_PARAM;
    }

    /* Check if support share destination id */
    if ((mirror_dest->flags & BCM_MIRROR_DEST_ID_SHARE) &&
        !(SOC_IS_TD2_TT2(unit))) {
        return BCM_E_PARAM;
    }

    if (mirror_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
        if (!(mirror_dest->flags & BCM_MIRROR_DEST_MTP_ADD) &&
            !(mirror_dest->flags & BCM_MIRROR_DEST_MTP_DELETE)) {
            /*
                * First call will alloc mirror dest_id for shared-id mirror destination,
                * so do not need to check gport parameter.
                */
            do_gport_check = FALSE;
            /* Rtag check */
            if (mirror_dest->rtag <= bcmMirrorPscNone ||
                mirror_dest->rtag >= bcmMirrorPscCount) {
                return (BCM_E_PARAM);
            }
        }
    }

    if (do_gport_check) {
        /* Resolve miror destination gport */
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_gport_adapt(unit, &(mirror_dest->gport)));

        /* Verify mirror destination port/trunk. */
        if ((0 == BCM_GPORT_IS_MODPORT(mirror_dest->gport)) &&
            (0 == BCM_GPORT_IS_TRUNK(mirror_dest->gport)) &&
            (0 == BCM_GPORT_IS_DEVPORT(mirror_dest->gport))) {
            return (BCM_E_PORT);
        }
    }

    /* Get local modid. */
    BCM_IF_ERROR_RETURN(_bcm_esw_local_modid_get(unit, &mymodid));

    /* Directed  mirroring support check. */
    if (MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit)) {
        int isLocal;

        /* No mirroring to a trunk. */
        if (BCM_GPORT_IS_TRUNK(mirror_dest->gport)) {
            return (BCM_E_UNAVAIL);
        } 

        /* Some devices do not support non-directed mode */
        if (soc_feature(unit, soc_feature_directed_mirror_only)) {
            return (BCM_E_CONFIG);
        }

        /* Set mirror destination to outgoing port on local module. */
        dest_mod = BCM_GPORT_IS_DEVPORT(mirror_dest->gport) ? 
            mymodid : BCM_GPORT_MODPORT_MODID_GET(mirror_dest->gport);

        BCM_IF_ERROR_RETURN
            (_bcm_esw_modid_is_local(unit, dest_mod, &isLocal));

        if (FALSE == isLocal) {
            _bcm_gport_dest_t   gport_st;

            BCM_IF_ERROR_RETURN
                (bcm_esw_topo_port_get(unit, dest_mod, &dest_port));
            gport_st.gport_type = BCM_GPORT_TYPE_MODPORT;
            gport_st.modid = mymodid;
            gport_st.port = dest_port;
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_construct(unit, &gport_st, 
                                         &(mirror_dest->gport)));
        }
    }

#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_INFO(unit).th_tflow_enabled == 1) {
        if ((mirror_dest->flags & BCM_MIRROR_DEST_UPDATE_TUNNEL_SFLOW)) {
            rv = _bcm_esw_mirror_destination_find(unit, mirror_dest->gport,
                                                  0, mirror_dest->flags,
                                                  &mirror_dest_check);

            if (BCM_SUCCESS(rv)) {
                /* Update the existing mirror destination entry */
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->version = mirror_dest->version;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->tos = mirror_dest->tos;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->ttl = mirror_dest->ttl;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->src_addr = mirror_dest->src_addr;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->dst_addr = mirror_dest->dst_addr;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->tpid = mirror_dest->tpid;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->vlan_id = mirror_dest->vlan_id;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->udp_src_port = mirror_dest->udp_src_port;
                (MIRROR_DEST(unit, mirror_dest_check.mirror_dest_id))
                    ->udp_dst_port = mirror_dest->udp_dst_port;
                sal_memcpy((MIRROR_DEST(unit,
                                        mirror_dest_check.mirror_dest_id))
                                        ->src_mac, mirror_dest->src_mac, 6);
                sal_memcpy((MIRROR_DEST(unit,
                                        mirror_dest_check.mirror_dest_id))
                                        ->dst_mac, mirror_dest->dst_mac, 6);

                if (mirror_dest->flags & BCM_MIRROR_PORT_EGRESS) {
                    /* Get the mtp index of the mirror destination */
                    BCM_IF_ERROR_RETURN
                        (_bcm_tr2_mirror_shared_mtp_match(unit,
                                                          mirror_dest_check.mirror_dest_id,
                                                          TRUE, &match_idx));
                    /* Set sFlow headers */
                    BCM_IF_ERROR_RETURN
                        (_bcm_mirror_sflow_tunnel_set(unit, match_idx,
                                                      0, BCM_MIRROR_PORT_EGRESS));
                }
                return BCM_E_NONE;
            } else {
                return rv;
            }
        }
    }
#endif /* BCM_TOMAHAWK_SUPPORT */

    /* If we are NOT replacing an existing entry, check if it exists. */
    if (0 == (mirror_dest->flags & BCM_MIRROR_DEST_REPLACE)) {
#ifdef BCM_TRIDENT2_SUPPORT
        if (mirror_dest->flags & BCM_MIRROR_DEST_ID_SHARE) {
            /* Do nothing here */
        } else
#endif /* BCM_TRIDENT2_SUPPORT */
        {
            rv = _bcm_esw_mirror_destination_find(unit, mirror_dest->gport, 0,
                                                  mirror_dest->flags,
                                                  &mirror_dest_check);
            if (BCM_SUCCESS(rv)) {
                /* Entry exists and we are not replacing, error */
                return BCM_E_EXISTS;
            } else if (BCM_E_NOT_FOUND != rv) {
                /* If something else went wrong, error */
                return rv;
            }
        }
    }
    /* Else, create destination (which handles replace properly) */
    MIRROR_LOCK(unit);
    rv = _bcm_esw_mirror_destination_create(unit, mirror_dest);
    MIRROR_UNLOCK(unit);
    return (rv);
}


/*
 * Function:
 *     bcm_esw_mirror_destination_destroy
 * Purpose:
 *     Destroy mirror destination description.
 * Parameters:
 *      unit            - (IN) BCM device number. 
 *      mirror_dest_id  - (IN) Mirror destination id.
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_esw_mirror_destination_destroy(int unit, bcm_gport_t mirror_dest_id) 
{
    int rv;   /* Operation return status. */
#ifdef BCM_TRIDENT2_SUPPORT
    _bcm_mirror_dest_config_p mdest_cfg; /* Mirror destination config */
#endif /* BCM_TRIDENT2_SUPPORT */

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (0 == BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    MIRROR_LOCK(unit);

    /* If destination stil in use - > E_BUSY */
    if (1 < MIRROR_DEST_REF_COUNT(unit, mirror_dest_id)) {
        MIRROR_UNLOCK(unit);
        return (BCM_E_BUSY);
    }

#ifdef BCM_TRIDENT2_SUPPORT
    mdest_cfg = &MIRROR_DEST_CONFIG(unit, mirror_dest_id);

    if (mdest_cfg->mirror_dest.flags & BCM_MIRROR_DEST_ID_SHARE) {
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_dest_mtp_delete_all(unit, mirror_dest_id));
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    rv = _bcm_mirror_destination_free(unit, mirror_dest_id); 

    MIRROR_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *     bcm_esw_mirror_destination_get
 * Purpose:
 *     Get mirror destination description.
 * Parameters:
 *      unit            - (IN) BCM device number. 
 *      mirror_dest_id  - (IN) Mirror destination id.
 *      mirror_dest     - (IN/OUT)Mirror destination description.
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_esw_mirror_destination_get(int unit, bcm_gport_t mirror_dest_id, 
                                   bcm_mirror_destination_t *mirror_dest)
{
    bcm_mirror_destination_t    mirror_destination;
    bcm_port_t                  port, port_out;
    bcm_module_t                modid, modid_out;
    int                         rv = BCM_E_NONE;
#if defined(BCM_KATANA2_SUPPORT)
    int                         is_local_subport = FALSE;
    int                         pp_port = 0;
#endif

    bcm_mirror_destination_t_init(&mirror_destination);

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    if (BCM_GPORT_INVALID == mirror_dest_id) {
        /* Find the mirror destination id from the dest description */
        return _bcm_esw_mirror_destination_find(unit, mirror_dest->gport, 0,
                                                mirror_dest->flags,
                                                mirror_dest);
    }

    if (0 == BCM_GPORT_IS_MIRROR(mirror_dest_id)) {
        return (BCM_E_PARAM);
    }

    if (NULL == mirror_dest) {
        return (BCM_E_PARAM);
    }

    MIRROR_LOCK(unit);

    /* If destination is not valid */ 
    if (0 == MIRROR_DEST_REF_COUNT(unit, mirror_dest_id)) {
        MIRROR_UNLOCK(unit);
        return (BCM_E_NOT_FOUND);
    }

#ifdef BCM_TRIDENT2_SUPPORT
    if (((MIRROR_DEST(unit, mirror_dest_id))->flags & BCM_MIRROR_DEST_ID_SHARE)
        && (BCM_GPORT_IS_MODPORT(mirror_dest->gport))) {
        BCM_IF_ERROR_RETURN(
            _bcm_mirror_dest_mtp_get(unit, mirror_dest_id, mirror_dest->gport,
                                     &mirror_destination));
    } else
#endif /* BCM_TRIDENT2_SUPPORT */
    {
        mirror_destination  = *(MIRROR_DEST(unit, mirror_dest_id));
    }
    if (BCM_GPORT_IS_MODPORT(mirror_destination.gport)) {
        port = BCM_GPORT_MODPORT_PORT_GET(mirror_destination.gport);
        modid = BCM_GPORT_MODPORT_MODID_GET(mirror_destination.gport);

#if defined(BCM_KATANA2_SUPPORT)
        rv = _bcm_kt2_modport_is_local_coe_subport(unit, modid,
                 port, &is_local_subport);
        if (BCM_FAILURE(rv)) {
            MIRROR_UNLOCK(unit);
            return rv;
        }
        if (is_local_subport) {
            rv = _bcm_kt2_modport_to_pp_port_get(unit, modid, port, &pp_port);
            if (BCM_FAILURE(rv)) {
                MIRROR_UNLOCK(unit);
                return rv;
            }
            mirror_destination.gport = 0;
            _BCM_KT2_SUBPORT_PORT_ID_SET(mirror_destination.gport, pp_port);
            if (BCM_PBMP_MEMBER(SOC_INFO(unit).linkphy_pp_port_pbm, pp_port)) {
                _BCM_KT2_SUBPORT_PORT_TYPE_SET(mirror_destination.gport,
                    _BCM_KT2_SUBPORT_TYPE_LINKPHY);
            } else if (BCM_PBMP_MEMBER(
                SOC_INFO(unit).subtag_pp_port_pbm, pp_port)) {
                _BCM_KT2_SUBPORT_PORT_TYPE_SET(mirror_destination.gport,
                    _BCM_KT2_SUBPORT_TYPE_SUBTAG);
            } else {
                MIRROR_UNLOCK(unit);
                return BCM_E_PORT;
            }
        } else
#endif
        {
            if (NUM_MODID(unit) > 1 && port> 31) {
                rv = _bcm_esw_stk_modmap_map(unit,
                        BCM_STK_MODMAP_GET, modid, port, &modid_out, &port_out);
                if (BCM_FAILURE(rv)) {
                    MIRROR_UNLOCK(unit);
                    return rv;
                }
                if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
                    MIRROR_UNLOCK(unit);
                    return BCM_E_PORT;
                }
                if (!SOC_MODID_ADDRESSABLE(unit, modid_out)) {
                    MIRROR_UNLOCK(unit);
                    return BCM_E_PARAM;
                }
                port = port_out;
                modid = modid_out;
            }
            rv = _bcm_mirror_gport_construct(unit, port,modid, 0, 
                                            &(mirror_destination.gport));
            if (BCM_FAILURE(rv)) {
                MIRROR_UNLOCK(unit);
                return rv;
            }
        }
    }
    *mirror_dest = mirror_destination; 
    MIRROR_UNLOCK(unit);
    return (rv);
}


/*
 * Function:
 *     bcm_esw_mirror_destination_traverse
 * Purpose:
 *     Traverse installed mirror destinations
 * Parameters:
 *      unit      - (IN) BCM device number. 
 *      cb        - (IN) Mirror destination traverse callback.         
 *      user_data - (IN) User cookie
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_esw_mirror_destination_traverse(int unit, bcm_mirror_destination_traverse_cb cb, 
                                    void *user_data) 
{
    int idx;                                 /* Mirror destinations index.     */
    _bcm_mirror_dest_config_p  mdest;        /* Mirror destination description.*/
    bcm_mirror_destination_t   mirror_dest;  /* User cb mirror destination.    */
    int rv = BCM_E_NONE;

    /* Initialization check. */
    if (0 == MIRROR_INIT(unit)) {
        return BCM_E_INIT;
    }

    /* Input parameters check. */
    if (NULL == cb) {
        return (BCM_E_PARAM);
    }

    MIRROR_LOCK(unit);
    /* Iterate mirror destinations & call user callback for valid ones. */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
        mdest = &MIRROR_CONFIG(unit)->dest_arr[idx];
        if (0 == mdest->ref_count) {
            continue;
        }

        mirror_dest = mdest->mirror_dest;
#ifdef BCM_TRIDENT2_SUPPORT
        if (mirror_dest.flags & BCM_MIRROR_DEST_ID_SHARE) {
            rv = _bcm_mirror_dest_mtp_traverse(
                     unit, mirror_dest.mirror_dest_id,
                     cb, user_data);
        } else
#endif /* BCM_TRIDENT2_SUPPORT */
        {
            rv = (*cb)(unit, &mirror_dest, user_data);
        }
        if (BCM_FAILURE(rv)) {
#ifdef BCM_CB_ABORT_ON_ERR
            if (SOC_CB_ABORT_ON_ERR(unit)) {
                MIRROR_UNLOCK(unit);
                return rv;
            }
#endif
        }
    }
    MIRROR_UNLOCK(unit);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_mirror_lock
 * Purpose:
 *      Allow other modules to take the mirroring mutex
 * Parameters:
 *      unit - unit #
 * Returns:
 *      None
 */
void bcm_esw_mirror_lock(int unit) {
    MIRROR_LOCK(unit);
}

/*
 * Function:
 *      bcm_esw_mirror_unlock
 * Purpose:
 *      Allow other modules to give up the mirroring mutex
 * Parameters:
 *      unit - unit #
 * Returns:
 *      None
 */
void bcm_esw_mirror_unlock(int unit) {
    MIRROR_UNLOCK(unit);
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
STATIC int
my_i2xdigit(int digit)
{
    digit &= 0xf;

    return (digit > 9) ? digit - 10 + 'a' : digit + '0';
}

STATIC void
fmt_macaddr(char buf[SAL_MACADDR_STR_LEN], sal_mac_addr_t macaddr)
{
    int i;

    for (i = 0; i <= 5; i++) {
        *buf++ = my_i2xdigit(macaddr[i] >> 4);
        *buf++ = my_i2xdigit(macaddr[i]);
        *buf++ = ':';
    }

    *--buf = 0;
}

STATIC void
fmt_ip6addr(char buf[IP6ADDR_STR_LEN], ip6_addr_t ipaddr)
{
    sal_sprintf(buf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", 
            (((uint16)ipaddr[0] << 8) | ipaddr[1]),
            (((uint16)ipaddr[2] << 8) | ipaddr[3]),
            (((uint16)ipaddr[4] << 8) | ipaddr[5]),
            (((uint16)ipaddr[6] << 8) | ipaddr[7]),
            (((uint16)ipaddr[8] << 8) | ipaddr[9]),
            (((uint16)ipaddr[10] << 8) | ipaddr[11]),
            (((uint16)ipaddr[12] << 8) | ipaddr[13]),
            (((uint16)ipaddr[14] << 8) | ipaddr[15]));
}

/*
 * Function:
 *     _bcm_mirror_sw_dump
 * Purpose:
 *     Displays mirror software structure information.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_mirror_sw_dump(int unit)
{
    int             idx, mode;
    _bcm_mirror_config_t  *mcp = MIRROR_CONFIG(unit);
    bcm_mirror_destination_t *mdest;
    _bcm_mtp_config_p mtp_cfg;
    char ip6_str[IP6ADDR_STR_LEN];
    char mac_str[SAL_MACADDR_STR_LEN];
    bcm_gport_t gport;
    char        pfmt[SOC_PBMP_FMT_LEN];

    LOG_CLI((BSL_META_U(unit,
                        "\nSW Information Mirror - Unit %d\n"), unit));
    mode = MIRROR_CONFIG_MODE(unit);
    LOG_CLI((BSL_META_U(unit,
                        "  Mode       : %s\n"),
             (mode == BCM_MIRROR_DISABLE) ? "Disabled" :
             ((mode == BCM_MIRROR_L2) ? "L2" :
             ((mode == BCM_MIRROR_L2_L3) ? "L2_L3" : "Unknown"))));
    LOG_CLI((BSL_META_U(unit,
                        "  Dest Count : %4d\n"), mcp->dest_count));
    if (soc_feature(unit, soc_feature_mirror_flexible)) {
        LOG_CLI((BSL_META_U(unit,
                            "  Max Ing MTP for Port: %4d\n"), mcp->port_im_mtp_count));
        LOG_CLI((BSL_META_U(unit,
                            "  Max Eng MTP for Port: %4d\n"), mcp->port_em_mtp_count));
    } else {
        LOG_CLI((BSL_META_U(unit,
                            "  Ing MTP Count: %4d\n"), mcp->ing_mtp_count));
        LOG_CLI((BSL_META_U(unit,
                            "  Egr MTP Count: %4d\n"), mcp->egr_mtp_count));
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        LOG_CLI((BSL_META_U(unit,
                            "  Egr True MTP Count: %4d\n"),
                 mcp->egr_true_mtp_count));
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    LOG_CLI((BSL_META_U(unit,
                        "  Directed   : %s\n"),
             MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit) ? "Flexible" :
             (MIRROR_MTP_METHOD_IS_DIRECTED_LOCKED(unit) ?
             "Locked" : "No")));

    if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANEX(unit) &&
            !SOC_IS_GREYHOUND2(unit) &&
            !soc_feature(unit, soc_feature_mirror_flexible)) {
        LOG_CLI((BSL_META_U(unit,
                            "  Mirror Exclusive   : %s\n"),
                MIRROR_SWITCH_IS_EXCLUSIVE(unit) ? "Yes" : "No"));
    }

    /* Mirror destinations */
    for (idx = 0; idx < MIRROR_CONFIG(unit)->dest_count; idx++) {
        BCM_GPORT_MIRROR_SET(gport, idx);
        if (0 == MIRROR_DEST_REF_COUNT(unit, gport)) {
            continue;
        }

        mdest = MIRROR_DEST(unit, gport);

        LOG_CLI((BSL_META_U(unit,
                            "  Mirror dest(%d): 0x%08x  Ref count: %4d\n"),
                 idx, mdest->mirror_dest_id,
                 MIRROR_DEST_REF_COUNT(unit, gport)));
        LOG_CLI((BSL_META_U(unit,
                            "              Gport     : 0x%08x\n"),
                 mdest->gport));
        LOG_CLI((BSL_META_U(unit,
                            "              TOS       : 0x%02x\n"),
                 mdest->tos));
        LOG_CLI((BSL_META_U(unit,
                            "              TTL       : 0x%02x\n"),
                 mdest->ttl));
        LOG_CLI((BSL_META_U(unit,
                            "              IP Version: 0x%02x\n"),
                 mdest->version));
        if (mdest->version == 4) {
            LOG_CLI((BSL_META_U(unit,
                                "              Src IP    : 0x%08x\n"),
                     mdest->src_addr));
            LOG_CLI((BSL_META_U(unit,
                                "              Dest IP   : 0x%08x\n"),
                     mdest->dst_addr));
        } else {
            fmt_ip6addr(ip6_str, mdest->src6_addr);
            LOG_CLI((BSL_META_U(unit,
                                "              Src IP    : %-42s\n"),
                     ip6_str));
            fmt_ip6addr(ip6_str, mdest->dst6_addr);
            LOG_CLI((BSL_META_U(unit,
                                "              Dest IP   : %-42s\n"),
                     ip6_str));
        }
        fmt_macaddr(mac_str, mdest->src_mac);
        LOG_CLI((BSL_META_U(unit,
                            "              Src MAC   : %-18s\n"),
                 mac_str));
        fmt_macaddr(mac_str, mdest->dst_mac);
        LOG_CLI((BSL_META_U(unit,
                            "              Dest MAC  : %-18s\n"),
                 mac_str));
        LOG_CLI((BSL_META_U(unit,
                            "              Flow label: 0x%08x\n"),
                 mdest->flow_label));
        LOG_CLI((BSL_META_U(unit,
                            "              TPID      : 0x%04x\n"),
                 mdest->tpid));
        LOG_CLI((BSL_META_U(unit,
                            "              VLAN      : 0x%04x\n"),
                 mdest->vlan_id));
        
        LOG_CLI((BSL_META_U(unit,
                            "              Flags     :")));
        if (mdest->flags & BCM_MIRROR_DEST_REPLACE) {
            LOG_CLI((BSL_META_U(unit,
                                "  Replace")));
        }
        if (mdest->flags & BCM_MIRROR_DEST_WITH_ID) {
            LOG_CLI((BSL_META_U(unit,
                                "  ID provided")));
        }
        if (mdest->flags & BCM_MIRROR_DEST_TUNNEL_L2) {
            LOG_CLI((BSL_META_U(unit,
                                "  L2 tunnel")));
        }
        if (mdest->flags & BCM_MIRROR_DEST_TUNNEL_IP_GRE) {
            LOG_CLI((BSL_META_U(unit,
                                "  IP GRE tunnel")));
        }
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
        if (mdest->flags & BCM_MIRROR_DEST_TUNNEL_TRILL) {
            LOG_CLI((BSL_META_U(unit,
                                "  TRILL tunnel")));
        }
        if (mdest->flags & BCM_MIRROR_DEST_TUNNEL_NIV) {
            LOG_CLI((BSL_META_U(unit,
                                "  NIV tunnel")));
        }  
#endif /* TRIDENT  */
        if (soc_feature(unit, soc_feature_port_extension)) {
            if (mdest->flags & BCM_MIRROR_DEST_TUNNEL_ETAG) {
                LOG_CLI((BSL_META_U(unit,
                                    "  ETAG tunnel")));
            }
        }

        if (mdest->flags & BCM_MIRROR_DEST_PAYLOAD_UNTAGGED) {
            LOG_CLI((BSL_META_U(unit,
                                "  Untagged payload")));
        }
        if (mdest->flags & BCM_MIRROR_DEST_PORT) {
            LOG_CLI((BSL_META_U(unit,
                                "  Port destination")));
        }
        if (mdest->flags & BCM_MIRROR_DEST_FIELD) {
            LOG_CLI((BSL_META_U(unit,
                                "  Field destination")));
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
    }

    for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
        LOG_CLI((BSL_META_U(unit,
                             "  MTP(%d).pbmp_used=%s\n"),
                 idx, SOC_PBMP_FMT(MIRROR_CONFIG_PBMP_MTP_SLOT_USED(unit, idx),
                                   pfmt)));
    }

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        
        for (idx = 0; idx < BCM_MIRROR_MTP_COUNT; idx++) {
            mtp_cfg = &MIRROR_CONFIG_SHARED_MTP(unit, idx);
            if (0 == mtp_cfg->ref_count) {
                continue;
            }
            
            LOG_CLI((BSL_META_U(unit,
                                "  %s MTP(%d): 0x%08x  Ref count: %4d\n"),
                     (TRUE == mtp_cfg->egress) ? "Egr" : "Ing", 
                     idx, mtp_cfg->dest_id, mtp_cfg->ref_count));
        }
    } else {
        /* Ingress MTPs */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->ing_mtp_count; idx++) {
            mtp_cfg = &MIRROR_CONFIG_ING_MTP(unit, idx);
            if (0 == mtp_cfg->ref_count) {
                continue;
            }

            LOG_CLI((BSL_META_U(unit,
                                "  Ing MTP(%d): 0x%08x  Ref count: %4d\n"),
                     idx, mtp_cfg->dest_id, mtp_cfg->ref_count));
        }

        /* Egress MTPs */
        for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_mtp_count; idx++) {
            mtp_cfg = &MIRROR_CONFIG_EGR_MTP(unit, idx);
            if (0 == mtp_cfg->ref_count) {
                continue;
            }

            LOG_CLI((BSL_META_U(unit,
                                "  Egr MTP(%d): 0x%08x  Ref count: %4d\n"),
                     idx, mtp_cfg->dest_id, mtp_cfg->ref_count));
        }
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_egr_mirror_true)) {
        for (idx = 0; idx < MIRROR_CONFIG(unit)->egr_true_mtp_count; idx++) {
            mtp_cfg = &MIRROR_CONFIG_EGR_TRUE_MTP(unit, idx);
            if (0 == mtp_cfg->ref_count) {
                continue;
            }

            LOG_CLI((BSL_META_U(unit,
                                "  Egress True MTP(%d): 0x%08x  Ref count: %4d\n"),
                     idx, mtp_cfg->dest_id, mtp_cfg->ref_count));
        }
    }

    if (soc_feature(unit, soc_feature_mirror_flexible) &&
        MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) {
        int mtp_slot, mtp_bit, mtp_type;

        BCM_MIRROR_MTP_ITER(MIRROR_CONFIG_MTP_DEV_MASK(unit), mtp_slot) {
            mtp_bit = 1 << mtp_slot;
            if (0 != MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit, mtp_slot)) {
                LOG_CLI((BSL_META_U(unit,
                                    "  MTP slot(%d): %s  Ref count: %4d\n"),
                         mtp_slot,
                         MIRROR_CONFIG_MTP_MODE_BMP(unit) & mtp_bit ?
                         "Egress" : "Ingress",
                         MIRROR_CONFIG_MTP_MODE_REF_COUNT(unit,
                         mtp_slot)));
                for (mtp_type = BCM_MTP_SLOT_TYPE_PORT;
                     mtp_type < BCM_MTP_SLOT_TYPES;
                     mtp_type++) {
                    if (0 != MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit,
                                           mtp_slot, mtp_type)) {
                        LOG_CLI((BSL_META_U(unit,
                                            "      MTP type(%d): %5s  Ref count: %4d\n"),
                                 mtp_type,
                                 (BCM_MTP_SLOT_TYPE_PORT == mtp_type) ? "Port": (
                                 (BCM_MTP_SLOT_TYPE_FP == mtp_type) ? "Field":
                                 "IPFIX"),
                                 MIRROR_CONFIG_TYPE_MTP_REF_COUNT(unit,
                                 mtp_slot, mtp_type)));
                    }
                }
            }
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
    if (soc_feature(unit, soc_feature_mirror_encap_profile)) {
        egr_mirror_encap_control_entry_t control_entry;
        egr_mirror_encap_data_1_entry_t data_1_entry;
        egr_mirror_encap_data_2_entry_t data_2_entry;
        void *entries[EGR_MIRROR_ENCAP_ENTRIES_NUM];
        int i, rv, ref_count, num_entries;

        entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL] = &control_entry;
        entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1] = &data_1_entry;
        entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2] = &data_2_entry;

        num_entries = soc_mem_index_count(unit, EGR_MIRROR_ENCAP_CONTROLm);

        LOG_CLI((BSL_META_U(unit,
                            "\n  Egress encap profiles\n")));
        LOG_CLI((BSL_META_U(unit,
                            "    Number of entries: %d\n"), num_entries));

        for (i = 0; i < num_entries; i ++) {
            rv = soc_profile_mem_ref_count_get(unit,
                                               EGR_MIRROR_ENCAP(unit),
                                               i, &ref_count);
            if (SOC_FAILURE(rv)) {
                LOG_CLI((BSL_META_U(unit,
                                    " *** Error retrieving profile reference: %d ***\n"),
                         rv));
                break;
            }

            if (ref_count <= 0) {
                continue;
            }

            rv = soc_profile_mem_get(unit, EGR_MIRROR_ENCAP(unit),
                                     i, 1, entries);
            if (SOC_FAILURE(rv)) {
                LOG_CLI((BSL_META_U(unit,
                                    " *** Error retrieving profile data: %d ***\n"), rv));
                break;
            }

            LOG_CLI((BSL_META_U(unit,
                                "  %5d %8d\n"), i, ref_count));
            soc_mem_entry_dump(unit, EGR_MIRROR_ENCAP_CONTROLm,
                               entries[EGR_MIRROR_ENCAP_ENTRIES_CONTROL]);
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
            soc_mem_entry_dump(unit, EGR_MIRROR_ENCAP_DATA_1m,
                               entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_1]);
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
            soc_mem_entry_dump(unit, EGR_MIRROR_ENCAP_DATA_2m,
                               entries[EGR_MIRROR_ENCAP_ENTRIES_DATA_2]);
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }
#endif /* BCM_TRIDENT_SUPPORT || BCM_GREYHOUND_SUPPORT */

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
