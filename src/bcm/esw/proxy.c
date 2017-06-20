/*
 * $Id: proxy.c,v 1.68 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>
#include <soc/drv.h>
#include <sal/core/libc.h>
#include <sal/core/sync.h>

#ifdef INCLUDE_L3
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/l3x.h>
#include <bcm/error.h>
#include <bcm/proxy.h>
#include <bcm/types.h>
#include <bcm/l3.h>

#include <bcm_int/esw/port.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/api_xlate_port.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/proxy.h>

#if defined(BCM_TRIDENT2_SUPPORT)
#include <bcm_int/esw/trident2.h>
#endif /* BCM_TRIDENT_SUPPORT*/

#define _BCM_PROXY_INFO_ENTRY_CREATED   0x00000001
#define _BCM_PROXY_INFO_GROUP_CREATED   0x00000002
#define _BCM_PROXY_INFO_FILTER_CREATED  0x00000004
#define _BCM_PROXY_INFO_INSTALLED       0x00000008
#define _BCM_PROXY_INFO_SERVER          0x00000010

#define _LOCK(unit) _bcm_proxy_control[unit].proxy_lock

#define _BCM_PROXY_LOCK(unit) \
   _LOCK(unit) ? sal_mutex_take(_LOCK(unit), sal_mutex_FOREVER) : -1

#define _BCM_PROXY_UNLOCK(unit) sal_mutex_give(_LOCK(unit))

typedef struct _bcm_proxy_info_s {
    int                      flags; /* _BCM_PROXY_INFO_* */
    bcm_port_t               client_port;
    bcm_proxy_proto_type_t   proto_type;
    bcm_module_t             server_modid;
    bcm_port_t               server_port;    
    bcm_proxy_mode_t         mode;
    bcm_field_entry_t        eid;
    bcm_field_group_t        gid;
    struct _bcm_proxy_info_s *next;
} _bcm_proxy_info_t;

typedef struct _bcm_proxy_control_s {
    sal_mutex_t       proxy_lock;
    _bcm_proxy_info_t *proxy_list;
    int               num_clients;
} _bcm_proxy_control_t;

typedef int (*proxy_install_method_t)(int unit,
                                      _bcm_proxy_info_t *dst,
                                      _bcm_proxy_info_t *src);

typedef int (*proxy_uninstall_method_t)(int unit,
                                        _bcm_proxy_info_t *src);

typedef int (*proxy_match_method_t)(_bcm_proxy_info_t *dst,
                                    _bcm_proxy_info_t *src);

typedef struct {
    proxy_install_method_t      install;
    proxy_uninstall_method_t    uninstall;
    proxy_match_method_t        match;
} _bcm_proxy_ifc_t;

STATIC int
_bcm_esw_proxy_client_install_xgs3(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_client_uninstall_xgs3(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_uninstall(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_match(_bcm_proxy_info_t *dst, _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_server_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_server_uninstall(int unit,
                                _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_server_match(_bcm_proxy_info_t *dst, _bcm_proxy_info_t *src);

static _bcm_proxy_control_t _bcm_proxy_control[BCM_MAX_NUM_UNITS];

STATIC 
_bcm_proxy_ifc_t client_methods = {
    _bcm_esw_proxy_client_install,
    _bcm_esw_proxy_client_uninstall,
    _bcm_esw_proxy_client_match,
};

STATIC 
_bcm_proxy_ifc_t server_methods = {
    _bcm_esw_proxy_server_install,
    _bcm_esw_proxy_server_uninstall,
    _bcm_esw_proxy_server_match,
};

/***************************************************************** Utilities */

/*
 * Function:
 *      _bcm_esw_proxy_gport_resolve
 * Purpose:
 *      Decodes gport into port and module id
 * Parameters:
 *      unit         -  BCM Unit number
 *      gport        - GPORT 
 *      port_out     - (OUT) port encoded into gport 
 *      modid_out    - (OUT) modid encoded into gport
 *      isLocal      - Indicator that port encoded in gport must be local
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_esw_proxy_gport_resolve(int unit, bcm_gport_t gport,
                             bcm_port_t *port_out,
                             bcm_module_t *modid_out, int isLocal)
{

    bcm_port_t      port; 
    bcm_module_t    modid;
    bcm_trunk_t     tgid;
    int             id;

    if (NULL == port_out || NULL == modid_out) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, gport, &modid, &port, &tgid, &id));

    if ((-1 != id) || (BCM_TRUNK_INVALID != tgid)){
        return BCM_E_PORT;
    }

    if (isLocal) {
        int    ismymodid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_modid_is_local(unit, modid, &ismymodid));
        if (ismymodid != TRUE) {
            return BCM_E_PARAM;
        }
    } 

    *port_out = port;
    *modid_out = modid;
    
    return BCM_E_NONE;
}

/****************************************************************** Generics */

/*
 * Function:
 *      _bcm_esw_proxy_install
 * Purpose:
 *      Install proxy client or server
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_ifc  -  local port for which redirection is applied
 *      data           -  Packet type to classify for redirection
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_proxy_install(int unit,
                       _bcm_proxy_ifc_t *ifc,
                       _bcm_proxy_info_t *src)
{
    int rv = BCM_E_INTERNAL;
    _bcm_proxy_info_t *dst;

    /* Allocate proxy info structure */
    dst = sal_alloc(sizeof(_bcm_proxy_info_t), "bcm_esw_proxy");

    if (dst == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(dst, 0, sizeof(_bcm_proxy_info_t));

    if (_BCM_PROXY_LOCK(unit)) {
        sal_free(dst);
        return BCM_E_INIT;
    }

    rv = ifc->install(unit, dst, src);
        
    if (BCM_SUCCESS(rv)) {
        /* Put onto list */
        dst->next = _bcm_proxy_control[unit].proxy_list;
        _bcm_proxy_control[unit].proxy_list = dst;
    } else {
        sal_free(dst);
    }

    _BCM_PROXY_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_uninstall
 * Purpose:
 *      Uninstall proxy client or server
 * Parameters:
 *      unit           -  BCM Unit number
 *      ifc            -  client/server methods
 *      info           -  info record to uninstall
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_proxy_uninstall(int unit,
                         _bcm_proxy_ifc_t *ifc,
                         _bcm_proxy_info_t *src)
{
    int rv = BCM_E_INTERNAL;
    _bcm_proxy_info_t *dst;

    if (_BCM_PROXY_LOCK(unit)) {
        return BCM_E_INIT;
    }

    /* Uninstall the enabled proxy client if the hardware is still
       capable. */
    if (SOC_HW_ACCESS_DISABLE(unit) == 0) {
        rv = ifc->uninstall(unit, src);
    } else {
        rv = BCM_E_NONE;
    }
    /* Unlink from list */
    if (_bcm_proxy_control[unit].proxy_list == src) {
        /* Info at beginning of the list */
        _bcm_proxy_control[unit].proxy_list = src->next;
    } else {
        for (dst = _bcm_proxy_control[unit].proxy_list;
             dst != NULL;
             dst = dst->next) {
            if (dst->next == src) {
                dst->next = src->next;
                break;
            }
        }
    }
    _BCM_PROXY_UNLOCK(unit);

    sal_memset(src, 0, sizeof(_bcm_proxy_info_t));
    sal_free(src);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_find
 * Purpose:
 *      Find a previously created proxy client or server
 * Parameters:
 *      unit         -  BCM Unit number
 *      ifc          -  interface methods
 *      src          -  data to find
 *      dstp         -  data found, or NULL
 * Returns:
 *      BCM_E_NONE     - found
 *      BCM_E_NOTFOUND - not found
 *      BCM_E_INIT     - subsystem not initialized
 */

STATIC int
_bcm_esw_proxy_find(int unit,
                    _bcm_proxy_ifc_t *ifc,
                    _bcm_proxy_info_t *src,
                    _bcm_proxy_info_t **dstp)
{
    _bcm_proxy_info_t *dst;
    int rv = BCM_E_NOT_FOUND;

    if (_BCM_PROXY_LOCK(unit)) {
        return BCM_E_INIT;
    }

    for (dst = _bcm_proxy_control[unit].proxy_list;
         dst != NULL;
         dst = dst->next) {
        if (ifc->match(dst, src)) {
            rv = BCM_E_EXISTS;
            break;
        }
    }
    _BCM_PROXY_UNLOCK(unit);
    *dstp = dst;

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy
 * Purpose:
 *      Generic proxy client or server install or uninstall
 * Parameters:
 *      unit         -  BCM Unit number
 *      ifc          -  interface methods
 *      src          -  data to find
 * Returns:
 *      Pointer to _bcm_proxy_info_t if found
 *      NULL if not found
 */

STATIC int
_bcm_esw_proxy(int unit, _bcm_proxy_ifc_t *ifc,
               _bcm_proxy_info_t *src, int enable)
{
    int rv;
    _bcm_proxy_info_t *dst = NULL;

    rv = _bcm_esw_proxy_find(unit, ifc, src, &dst);

    if (enable) {

        if (rv != BCM_E_NOT_FOUND) {
            return rv;
        }

        rv = _bcm_esw_proxy_install(unit, ifc, src);

    } else {
        /* Disable */

        if (rv != BCM_E_EXISTS) {
            return rv;
        }
        
        rv = _bcm_esw_proxy_uninstall(unit, ifc, dst);

    }

    return rv;
}

/******************************************************************** Client */

/*
 * Function:
 *      _bcm_proxy_client_enabled
 * Purpose:
 *      Returns true if the proxy client is enabled on the given unit
 * Parameters:
 *      unit         -  BCM Unit number
 * Returns:
 *      TRUE or FALSE
 */

int
_bcm_proxy_client_enabled(int unit)
{
    return (_bcm_proxy_control[unit].num_clients > 0);
}

/*
 * Function:
 *      bcm_esw_proxy_client_set
 * Purpose:
 *      Enables redirection for a certain traffic type using either 
 *      FFP or FP rule
 * Parameters:
 *      unit         -  BCM Unit number
 *      client_port  -  local port for which redirection is applied
 *      proto_type   -  Packet type to classify for redirection
 *      server_modid -  Module ID of remote device which performs lookups
 *      server_port  -  Port on remote device where redirected packets are 
 *                      destined to
 *      enable       -  toggle to enable or disable redirection
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_client_set(int unit, bcm_port_t client_port, 
                         bcm_proxy_proto_type_t proto_type,
                         bcm_module_t server_modid, bcm_port_t server_port, 
                         int enable)
{
    _bcm_proxy_info_t src;

    if (BCM_GPORT_IS_SET(client_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, client_port, &client_port));
    }
    if (BCM_GPORT_IS_SET(server_port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_proxy_gport_resolve(unit, server_port, &server_port,
                                         &server_modid, 0));
    }

    if (!SOC_PORT_VALID(unit, client_port)) {
        return BCM_E_PORT;
    }
    if (server_port < 0 ) {
        return BCM_E_PORT;
    }

    sal_memset(&src, 0, sizeof(_bcm_proxy_info_t));
    src.client_port  = client_port;
    src.proto_type   = proto_type;
    src.server_modid = server_modid;
    src.server_port  = server_port;

    return _bcm_esw_proxy(unit, &client_methods, &src, enable);
}

/*
 * Function:
 *      _bcm_esw_proxy_client_match
 * Purpose:
 *      Return true if src client record matches dst
 * Parameters:
 *      dst - 
 *      src - 
 * Returns:
 *      True if matched
 */

STATIC int
_bcm_esw_proxy_client_match(_bcm_proxy_info_t *dst, _bcm_proxy_info_t *src)
{
        return (dst->client_port == src->client_port &&
                dst->proto_type == src->proto_type &&
                dst->server_modid == src->server_modid &&
                dst->server_port == src->server_port &&
                (dst->flags & _BCM_PROXY_INFO_SERVER) == 0);
}

/*
 * Function:
 *      _bcm_esw_proxy_server_match
 * Purpose:
 *      Return true if src server record matches dst
 * Parameters:
 *      dst - 
 *      src - 
 * Returns:
 *      True if matched
 */

STATIC int
_bcm_esw_proxy_server_match(_bcm_proxy_info_t *dst,
                            _bcm_proxy_info_t *src)
{
    return (dst->mode == src->mode &&
            dst->server_port == src->server_port &&
            (dst->flags & _BCM_PROXY_INFO_SERVER) != 0);
}

/*
 * Function:
 *      _bcm_esw_proxy_client_create_xgs3
 * Purpose:
 *      Creates proxy client using FP rules, doesn't clean up on error
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_create_xgs3(int unit, _bcm_proxy_info_t *bcm_proxy_info)
{
#ifdef BCM_XGS3_SWITCH_SUPPORT
    _field_stage_t         *stage_fc=NULL;  /* Stage Field control structure.   */
    bcm_port_config_t      pc;              /* Port Configuration structure.    */
    bcm_field_group_config_t fg;            /* Group configuration structure.   */
    int                    instance=0;      /* Pipe instance.                   */
    bcm_pbmp_t             mask_pbmp;       /* IPBM mask.                       */
    bcm_field_entry_t      eid;
    bcm_field_group_t      gid;
    bcm_pbmp_t             ingress_pbmp;
    bcm_port_t             client_port = bcm_proxy_info->client_port;
    bcm_proxy_proto_type_t proto_type = bcm_proxy_info->proto_type;
    bcm_module_t           server_modid = bcm_proxy_info->server_modid;
    bcm_port_t             server_port = bcm_proxy_info->server_port;

    BCM_PBMP_PORT_SET(ingress_pbmp, client_port);
   
    /* In Tomahawk, InPorts qualifier are not supported in Global Mode currently. 
     * In PerPipe mode, Group is created with port bitmaps of only the 
     * pipe to which the client_port belongs to.
     */
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc));
    
    bcm_port_config_t_init(&pc);
    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &pc));

    bcm_field_group_config_t_init(&fg);
    instance = SOC_INFO(unit).port_pipe[client_port];   
    mask_pbmp = PBMP_ALL(unit); 

    if (SOC_IS_TOMAHAWKX(unit)) {
        switch (stage_fc->oper_mode) {
            case bcmFieldGroupOperModeGlobal:
                /*TBD: InPorts qualifier in Global Mode is not supported
                 *in Tomahawk for now*/
                return BCM_E_UNAVAIL;
                break;
            case bcmFieldGroupOperModePipeLocal:
                fg.ports = pc.per_pipe[instance];
                fg.flags |= BCM_FIELD_GROUP_CREATE_WITH_PORT; 
                mask_pbmp = PBMP_PIPE(unit,instance);
                break;
            default: 
                return (BCM_E_INTERNAL);
        }
    }
           
    BCM_FIELD_QSET_INIT(fg.qset);
    BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyInPorts);

    switch(proto_type) {
    case BCM_PROXY_PROTO_IP4_ALL:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_IP6_ALL:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_IP4_MCAST:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyDstIp);
        break;
    case BCM_PROXY_PROTO_IP6_MCAST:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyIpType);
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyDstIp6High);
        break;
    case BCM_PROXY_PROTO_MPLS_UCAST:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_MPLS_MCAST:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_MPLS_ALL:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_IP6_IN_IP4:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyIpProtocol);
        break;
    case BCM_PROXY_PROTO_IP_IN_IP:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyIpProtocol);
        break;
    case BCM_PROXY_PROTO_GRE_IN_IP:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyIpProtocol);
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP4_UCAST:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyEtherType);
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyPacketRes);
        break;
    /*    coverity[equality_cond]    */
    case BCM_PROXY_PROTO_UNKNOWN_IP6_UCAST:
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyEtherType);
        BCM_FIELD_QSET_ADD(fg.qset, bcmFieldQualifyPacketRes);
        break;
    default:
        return BCM_E_PARAM;
    }
    
    BCM_IF_ERROR_RETURN(bcm_esw_field_group_config_create(unit, &fg));
    gid = fg.group;
   
    bcm_proxy_info->flags |= _BCM_PROXY_INFO_GROUP_CREATED;
    bcm_proxy_info->gid = gid;

    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_create(unit, gid, &eid));
    bcm_proxy_info->flags |= _BCM_PROXY_INFO_ENTRY_CREATED;
    bcm_proxy_info->eid = eid;

    BCM_IF_ERROR_RETURN
        (bcm_esw_field_qualify_InPorts(unit, eid, ingress_pbmp,
                                       mask_pbmp));

    switch(proto_type) {
    case BCM_PROXY_PROTO_IP4_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x0800, 0xffff));
        break;
    case BCM_PROXY_PROTO_IP6_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x86dd, 0xffff));
        break;
    case BCM_PROXY_PROTO_IP4_MCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_DstIp(unit, eid, 0xe0000000, 0xf0000000));
        break;
    case BCM_PROXY_PROTO_IP6_MCAST:
        {
            bcm_ip6_t addr, mask;
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_IpType(unit, eid, bcmFieldIpTypeIpv6));
            sal_memset(addr, 0, 16); /* BCM_IP6_ADDRLEN */
            addr[0] = 0xff;
            sal_memset(mask, 0, 16); /* BCM_IP6_ADDRLEN */
            mask[0] = 0xff;
            mask[1] = 0xff;
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_DstIp6High(unit, eid, addr, mask));
            break;
        }

    case BCM_PROXY_PROTO_MPLS_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x8847, 0xffff));
        break;
    case BCM_PROXY_PROTO_MPLS_MCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x8848, 0xffff));
        break;
    case BCM_PROXY_PROTO_MPLS_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x8840, 0xfff0));
        break;
    case BCM_PROXY_PROTO_IP6_IN_IP4:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpProtocol(unit, eid, 0x29, 0xff));
        break;
    case BCM_PROXY_PROTO_IP_IN_IP:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpProtocol(unit, eid, 0x4, 0xff));
        break;
    case BCM_PROXY_PROTO_GRE_IN_IP:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpProtocol(unit, eid, 0x2f, 0xff));
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP4_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x0800, 0xffff));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_PacketRes(unit, eid,
                                             BCM_FIELD_PKT_RES_L3UCUNKNOWN,
                                             0xf));
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP6_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x86dd, 0xffff));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_PacketRes(unit, eid,
                                             BCM_FIELD_PKT_RES_L3UCUNKNOWN,
                                             0xf));
        break;
        /* Defensive Default */
        /* coverity[dead_error_begin] */
    default:
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_field_action_add(unit, eid, bcmFieldActionRedirect, 
                                  server_modid, server_port));
    
     BCM_IF_ERROR_RETURN
         (bcm_esw_field_entry_install(unit, eid));
    
    bcm_proxy_info->flags |= _BCM_PROXY_INFO_INSTALLED;
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
} 

/*
 * Function:
 *      _bcm_esw_proxy_client_uninstall_xgs3
 * Purpose:
 *      Removes and destroys proxy client FP resources
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_uninstall_xgs3(int unit,
                                     _bcm_proxy_info_t *info)
{
#ifdef BCM_XGS3_SWITCH_SUPPORT
    bcm_field_entry_t eid   = info->eid;
    bcm_field_group_t gid   = info->gid;
    int flags               = info->flags;

    if (flags & _BCM_PROXY_INFO_INSTALLED) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_entry_remove(unit, eid));
    }

    if (flags & _BCM_PROXY_INFO_ENTRY_CREATED) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_entry_destroy(unit, eid));
    }

    if (flags & _BCM_PROXY_INFO_GROUP_CREATED) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_group_destroy(unit, gid));
    }

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      _bcm_esw_proxy_client_install_xgs3
 * Purpose:
 *      Creates proxy client using FP rules, clean up on error
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_install_xgs3(int unit,
                                   _bcm_proxy_info_t *info)
{
    int rv = BCM_E_INTERNAL;

    rv = _bcm_esw_proxy_client_create_xgs3(unit, info);

    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_L3,
                 (BSL_META_U(unit,
                             "Proxy: could not install client: %s\n"),
                  bcm_errmsg(rv)));
        /* Ignore errors from uninstall */
        _bcm_esw_proxy_client_uninstall_xgs3(unit, info);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_client_uninstall
 * Purpose:
 *      Uninstall proxy client for all device families
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_uninstall(int unit, _bcm_proxy_info_t *info)
{
    int rv = BCM_E_INTERNAL;

    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = _bcm_esw_proxy_client_uninstall_xgs3(unit, info);
    }

    if (BCM_SUCCESS(rv)) {
        _bcm_proxy_control[unit].num_clients--;
    }

    return rv;
}


/*
 * Function:
 *      _bcm_esw_proxy_client_install
 * Purpose:
 *      Install proxy client for all device families
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src)
{
    int rv = BCM_E_INTERNAL;

    dst->client_port  = src->client_port;
    dst->proto_type   = src->proto_type;
    dst->server_modid = src->server_modid;
    dst->server_port  = src->server_port;
    dst->server_port  = src->server_port;

    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = _bcm_esw_proxy_client_install_xgs3(unit, dst);
    }

    if (BCM_SUCCESS(rv)) {
        _bcm_proxy_control[unit].num_clients++;
    }
    return rv;
}

/******************************************************************** Server */

/*
 * Function:
 *      _bcm_esw_proxy_server_set
 * Purpose:
 *      Enables various kinds of lookups for packets coming from remote
 *      (proxy client) devices (internal function)
 * Parameters:
 *      unit        -  BCM Unit number
 *      server_port -  Local port to which packets from remote devices are 
 *                     destined to
 *      mode        -  Indicates lookup type
 *      enable      -  TRUE to enable lookups
 *                     FALSE to disable lookups
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_proxy_server_set(int unit, bcm_port_t server_port, 
                          bcm_proxy_mode_t mode, int enable)
{
    
    if (BCM_GPORT_IS_SET(server_port)) {
        bcm_module_t    modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_proxy_gport_resolve(unit, server_port,
                                         &server_port, &modid, 1));
    }
    
    if (IS_HG_PORT(unit, server_port) || IS_CPU_PORT(unit, server_port)) {
        if ((mode != BCM_PROXY_MODE_HIGIG) && (mode != BCM_PROXY_MODE_HYBRID)) {
            return BCM_E_PARAM;
        }
           
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_FBX(unit)) {
            uint32 dst_bmp;
            bcm_pbmp_t pbmp;
            uint32 val, mod = 0;
            int vfp_enable = 0;
            int lport_idx;
            port_tab_entry_t ptab;
            
            if (!soc_feature(unit, soc_feature_higig_lookup)) {
                return BCM_E_UNAVAIL;
            }

            /*
             * Program PORT table V4L3_ENABLE, V6L3_ENABLE;
             * required for IP-IP tunneling on XGS3 devices.
             * This is not necessary for XGS4 and later.
             */

            
            if (!SOC_IS_TRX(unit)) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, 
                                  server_port, &ptab));
                soc_PORT_TABm_field32_set(unit, &ptab, V4L3_ENABLEf, 
                                          enable ? 1 : 0);
                soc_PORT_TABm_field32_set(unit, &ptab, V6L3_ENABLEf,
                                          enable ? 1 : 0);
                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, 
                                   server_port, &ptab));
            }

            /*
             * Program LPORT table for the HG port;
             * HIGIG_PACKET must not be set; MY_MODID must be
             * set appropriately, and V4/V6 enable bits
             */
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, 
                              server_port, &ptab));
            if (soc_mem_field_valid(unit, PORT_TABm, MY_MODIDf)) {
                mod = soc_PORT_TABm_field32_get(unit, &ptab, MY_MODIDf);
            }
            if (soc_mem_field_valid(unit, PORT_TABm, VFP_ENABLEf)) {
                vfp_enable = soc_PORT_TABm_field32_get(unit, &ptab, VFP_ENABLEf);
            }
            sal_memset(&ptab, 0, sizeof(port_tab_entry_t));
            if (soc_mem_field_valid(unit, LPORT_TABm, MY_MODIDf)) {
                soc_LPORT_TABm_field32_set(unit, &ptab, MY_MODIDf, mod);
            }
            if (soc_mem_field_valid(unit, PORT_TABm, VFP_ENABLEf)) {
                soc_LPORT_TABm_field32_set(unit, &ptab, VFP_ENABLEf, vfp_enable);
            }
            soc_LPORT_TABm_field32_set(unit, &ptab, V4L3_ENABLEf, 
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, V6L3_ENABLEf,
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, V4IPMC_ENABLEf,
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, V6IPMC_ENABLEf, 
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, IPMC_DO_VLANf,
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, FILTER_ENABLEf,
                                       enable ? 1 : 0);

            /* For Triumph, we use entry 0 so all remote ports
               will use this entry.  This is the behavior for previous
               devices */
            lport_idx = SOC_IS_TR_VL(unit) ? 0 : server_port;
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, LPORT_TABm, MEM_BLOCK_ALL, 
                               lport_idx, &ptab));
                    
            SOC_IF_ERROR_RETURN(READ_IHG_LOOKUPr
                                (unit, server_port, &val));
            if (BCM_PROXY_MODE_HIGIG == mode) {
                /* BCM_PROXY_MODE_HIGIG */
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                        HG_LOOKUP_ENABLEf, enable ? 1: 0);
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                        HYBRID_MODE_ENABLEf, 0);
            } else {
                /* BCM_PROXY_MODE_HYBRID */
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                        HG_LOOKUP_ENABLEf, 0);
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                        HYBRID_MODE_ENABLEf, enable ? 1: 0);
            }
            if (SOC_IS_TRX(unit)) {
                int ing_port;
                soc_reg_t reg;
                static soc_field_t fields[2] = { DST_MODIDf, DST_PORTf };
                uint32 values[2];
                /* Set "magic destination" to this server port to
                   match prior devices' behavior */
                reg = HG_LOOKUP_DESTINATIONr;
                values[0] = mod;
                values[1] = server_port;
                if (SOC_REG_INFO(unit, reg).regtype == soc_portreg) {
                    PBMP_HG_ITER(unit, ing_port) {
                        SOC_IF_ERROR_RETURN
                            (soc_reg_fields32_modify(unit, reg, ing_port,
                                                     2, fields, values));
                    }
                } else {
                    SOC_IF_ERROR_RETURN
                        (soc_reg_fields32_modify(unit, reg, REG_PORT_ANY,
                                                 2, fields, values));
                }
            } else {
                BCM_PBMP_PORT_SET(pbmp, server_port);
                dst_bmp = SOC_PBMP_WORD_GET(pbmp, 0);
                dst_bmp >>= SOC_HG_OFFSET(unit);
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                  DST_HG_LOOKUP_BITMAPf, dst_bmp);
            }
            if (SOC_REG_FIELD_VALID(unit, IHG_LOOKUPr,
                                    LOOKUP_WITH_MH_SRC_PORTf)) {
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                  LOOKUP_WITH_MH_SRC_PORTf, enable ? 1 :0);
            }
            if (SOC_REG_FIELD_VALID(unit, IHG_LOOKUPr,
                                    USE_MH_INTERNAL_PRIf)) {
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                  USE_MH_INTERNAL_PRIf, enable ? 1: 0);
            }
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, USE_MH_VIDf, 
                              enable ? 1: 0);
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, USE_MH_PKT_PRIf,
                              enable ? 1: 0);
            SOC_IF_ERROR_RETURN(WRITE_IHG_LOOKUPr(unit, server_port, val));
	}
#endif /* BCM_XGS3_SWITCH_SUPPORT */
     } else {

	if (mode != BCM_PROXY_MODE_LOOPBACK) {
	    return BCM_E_PARAM;
	}
        
	if (enable) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_loopback_set(unit, server_port, 
                                           BCM_PORT_LOOPBACK_MAC));
	} else {
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_loopback_set(unit, server_port, 
                                           BCM_PORT_LOOPBACK_NONE));
	}
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_proxy_server_get
 * Purpose:
 *      Get the lookup mode of XGS3 device.
 * Parameters:
 *      unit        -  BCM Unit number
 *      server_port -  Local port to which packets from remote devices are 
 *                     destined to
 *      mode        -  proxy server mode
 *      enable      -  (OUT) server status
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_server_get(int unit, bcm_port_t server_port, 
                         bcm_proxy_mode_t mode, int *enable)
{
    int rv = BCM_E_NONE;
    *enable = FALSE;

    if (BCM_GPORT_IS_SET(server_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, server_port, &server_port));
    }

    if (!(IS_HG_PORT(unit, server_port) || IS_CPU_PORT(unit, server_port))) {
        int status;

	if (mode != BCM_PROXY_MODE_LOOPBACK) {
	    return BCM_E_PARAM;
	}
    /*    coverity[uninit_use_in_call : FALSE]    */
	    
        rv = bcm_esw_port_loopback_get(unit, server_port, &status);
        if (BCM_SUCCESS(rv)) {
            if (status == BCM_PORT_LOOPBACK_MAC ||
                status == BCM_PORT_LOOPBACK_PHY) {
                *enable = TRUE;
            } 
        }
    } else {

        if ((mode != BCM_PROXY_MODE_HIGIG) && (mode != BCM_PROXY_MODE_HYBRID)) {
	        return BCM_E_PARAM;
        }

#ifdef BCM_XGS3_SWITCH_SUPPORT
	if (soc_feature(unit, soc_feature_higig_lookup)) {
            int port, hg_enable;
            uint32 val, dst_bmp;
            bcm_pbmp_t pbmp, r_pbmp;

            if (SOC_IS_TRX(unit)) {
                uint32 hgld;
                /* Set "magic destination" to this server port to
                   match prior devices' behavior */
                SOC_IF_ERROR_RETURN
                    (READ_HG_LOOKUP_DESTINATIONr(unit, &hgld));
                if (server_port == soc_reg_field_get(unit,
                                                    HG_LOOKUP_DESTINATIONr,
                                                    hgld,
                                                    DST_PORTf)) {
                    SOC_IF_ERROR_RETURN
                        (READ_IHG_LOOKUPr(unit, server_port, &val));
                    if (BCM_PROXY_MODE_HIGIG == mode) {
                        hg_enable = soc_reg_field_get(unit, IHG_LOOKUPr, val,
                                                      HG_LOOKUP_ENABLEf);
                    } else {
                        hg_enable = soc_reg_field_get(unit, IHG_LOOKUPr, val,
                                                      HYBRID_MODE_ENABLEf);
                    }
                    *enable = hg_enable;
                }
            } else {
                PBMP_HG_ITER(unit, port) {
                    SOC_PBMP_CLEAR(r_pbmp);
                    BCM_PBMP_PORT_SET(pbmp, server_port);

                    SOC_IF_ERROR_RETURN(READ_IHG_LOOKUPr(unit, port, &val));
                    hg_enable = soc_reg_field_get(unit, IHG_LOOKUPr, val, 
                                                  HG_LOOKUP_ENABLEf);
                    dst_bmp = soc_reg_field_get(unit, IHG_LOOKUPr, val, 
                                                DST_HG_LOOKUP_BITMAPf);
                    dst_bmp <<= SOC_HG_OFFSET(unit);
                    SOC_PBMP_WORD_SET(r_pbmp, 0, dst_bmp);
                    BCM_PBMP_AND(pbmp, r_pbmp);

                    if (hg_enable && BCM_PBMP_EQ(pbmp, r_pbmp)) {
                        *enable = TRUE;
                        break;
                    }
                }
            }
	}
#endif
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_server
 * Purpose:
 *      Enable/disable proxy server
 * Parameters:
 *      unit        -  BCM Unit number
 *      dst         -  Proxy server data
 *      enable      -  server enable/diable flag
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_server(int unit, _bcm_proxy_info_t *src, int enable)
{
    int rv = BCM_E_INTERNAL;


    rv = _bcm_esw_proxy_server_set(unit, src->server_port, src->mode, enable);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_server_install
 * Purpose:
 *      Install and enable proxy server
 * Parameters:
 *      unit        -  BCM Unit number
 *      dst         -  Proxy server saved data
 *      src         -  Proxy server initial data
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_server_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src)
{
    dst->server_port  = src->server_port;
    dst->mode  = src->mode;
    dst->flags |= (_BCM_PROXY_INFO_INSTALLED|_BCM_PROXY_INFO_SERVER);

    return _bcm_esw_proxy_server(unit, dst, 1);
}

/*
 * Function:
 *      _bcm_esw_proxy_server_uninstall
 * Purpose:
 *      Uninstall and disable proxy server
 * Parameters:
 *      unit        -  BCM Unit number
 *      dst         -  Proxy server saved data
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_server_uninstall(int unit, _bcm_proxy_info_t *src)
{
    return _bcm_esw_proxy_server(unit, src, 0);
}

/*
 * Function:
 *      bcm_esw_proxy_server_set
 * Purpose:
 *      Enables various kinds of lookups for packets coming from remote
 *      (proxy client)
 * Parameters:
 *      unit        -  BCM Unit number
 *      server_port -  Local port to which packets from remote devices are 
 *                     destined to
 *      mode        -  Indicates lookup type
 *      enable      -  TRUE to enable lookups
 *                     FALSE to disable lookups
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_server_set(int unit, bcm_port_t server_port, 
                          bcm_proxy_mode_t mode, int enable)
{
    _bcm_proxy_info_t src;

    if (BCM_GPORT_IS_SET(server_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, server_port, &server_port));
    }

    sal_memset(&src, 0, sizeof(_bcm_proxy_info_t));
    src.server_port  = server_port;
    src.mode  = mode;

    return _bcm_esw_proxy(unit, &server_methods, &src, enable);
}

/*
 * Function:
 *      bcm_esw_proxy_server_port_set
 * Purpose:
 *      This API enables various kinds of lookups on XGS3 device on
 *      behalf of remote legacy or XGS3 devices on a per ingress port
 *      basis.  The local ingress port is expected to be a Higig stack
 *      port.  The server_gport in the proxy_server structure is the
 *      target BCM_GPORT_PROXY (modid,port) destination which
 *      indicates that hybrid proxy lookup is required.
 * Parameters:
 *      unit - (IN) Unit number.
 *      local_ingress_port - (IN) Incoming Higig stack port
 *      proxy_server - (IN) Proxy server configuration
 *      enable - (IN) Enable proxy server on selected Higig stack port
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_proxy_server_port_set(int unit, bcm_gport_t local_ingress_port, 
                              bcm_proxy_server_t *proxy_server, 
                              int enable)
{
    bcm_module_t    modid;
    bcm_gport_t     server_gport, server_port;
    uint32          val, field_num, rmsp;
    static soc_field_t fields[3] =
        { DST_MODIDf, DST_PORTf, DST_PORT_MASKf };
    uint32          values[3];
    int i = 0;
    uint32 flow_type[_BCM_PROXY_RESERVED_DGLP_NUMBER] = {0};
    uint32 reg_val[_BCM_PROXY_RESERVED_DGLP_NUMBER] = {0};
    static soc_reg_t config_regs[] = {
        ING_PACKET_PROCESSING_CONTROL_0r,
        ING_PACKET_PROCESSING_CONTROL_1r
    };    
    static soc_field_t config_reg_fields[] = {
        ING_PACKET_PROCESSING_ENABLE_0f,
        ING_PACKET_PROCESSING_ENABLE_1f
    };

    if (!soc_feature(unit, soc_feature_proxy_port_property)) {
        return BCM_E_UNAVAIL;
    }

    if ((NULL == proxy_server) && enable) {
        /* No configuration to enable */
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(local_ingress_port)) {
        /* Note, modid is a throwaway arg here */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_proxy_gport_resolve(unit, local_ingress_port,
                                          &local_ingress_port, &modid, 1));
    } else if (!SOC_PORT_VALID(unit, local_ingress_port)) { 
        return BCM_E_PORT; 
    }

    if (!(IS_HG_PORT(unit, local_ingress_port) ||
          IS_CPU_PORT(unit, local_ingress_port))) {
        
        /* Only Higig ports (including the CPU port as
         * Higig) are valid.
         */
        return BCM_E_PORT;
    }

    SOC_IF_ERROR_RETURN(READ_IHG_LOOKUPr(unit, local_ingress_port, &val));

    if (enable) {
        server_gport = proxy_server->server_gport;
        if (!BCM_GPORT_IS_PROXY(server_gport)) {
            /* The server gport is required to be in
             * BCM_GPORT_PROXY format */
            return BCM_E_PORT;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_esw_proxy_gport_resolve(unit, server_gport,
                                          &server_port, &modid, 0));
    
        if (BCM_PROXY_MODE_HIGIG == proxy_server->mode) {
            /* BCM_PROXY_MODE_HIGIG */
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                              HG_LOOKUP_ENABLEf, 1);
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                              HYBRID_MODE_ENABLEf, 0);
        } else if (BCM_PROXY_MODE_HYBRID == proxy_server->mode) {
            /* BCM_PROXY_MODE_HYBRID */
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                              HG_LOOKUP_ENABLEf, 0);
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                              HYBRID_MODE_ENABLEf, 1);
        } else if (BCM_PROXY_MODE_SECOND_PASS == proxy_server->mode) {
            if (!soc_feature(unit, soc_feature_hg_proxy_second_pass)) {
                return BCM_E_UNAVAIL;
            }

            if ((proxy_server->flow_type > bcmProxySecondPassFlowTypeTrillTermination) ||
                (proxy_server->flow_type < bcmProxySecondPassFlowTypeNone)) {
                return BCM_E_PARAM;
            }

            /* if exist, update the entry */
            for(i = 0; i < _BCM_PROXY_RESERVED_DGLP_NUMBER; i++) {
                SOC_IF_ERROR_RETURN
                    (soc_reg32_get(unit, config_regs[i], REG_PORT_ANY, 0, &reg_val[i]));
                flow_type[i] = soc_reg_field_get(unit,config_regs[i], reg_val[i], FLOW_TYPEf);
                
                if (proxy_server->flow_type == flow_type[i]) {
                    soc_reg_field_set(unit,config_regs[i],
                        &reg_val[i], FLOW_TYPEf,proxy_server->flow_type);
                    
                    soc_reg_field_set(unit,config_regs[i],
                        &reg_val[i], DST_MODIDf, modid);
                    soc_reg_field_set(unit,config_regs[i],
                        &reg_val[i], DST_PORTf, server_port);
					
                    SOC_IF_ERROR_RETURN
                        (soc_reg32_set(unit, config_regs[i], REG_PORT_ANY, 0, reg_val[i]));
                    
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        config_reg_fields[i], 1);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        HG_LOOKUP_ENABLEf, 0);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        HYBRID_MODE_ENABLEf, 0);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        USE_MH_VIDf, 1);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        USE_MH_PKT_PRIf, 1);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        REMOVE_MH_SRC_PORTf, 1);

                    SOC_IF_ERROR_RETURN
                        (WRITE_IHG_LOOKUPr(unit, local_ingress_port, val));
                    return BCM_E_NONE;
                }
            }

            /* if not exist, create it */
            for(i = 0; i < _BCM_PROXY_RESERVED_DGLP_NUMBER; i++) {
                if (bcmProxySecondPassFlowTypeNone == flow_type[i]) {
                    soc_reg_field_set(unit,config_regs[i],
                        &reg_val[i], FLOW_TYPEf,proxy_server->flow_type);
                    
                    soc_reg_field_set(unit,config_regs[i],
                        &reg_val[i], DST_MODIDf, modid);
                    soc_reg_field_set(unit,config_regs[i],
                        &reg_val[i], DST_PORTf, server_port);
					
                    SOC_IF_ERROR_RETURN
                        (soc_reg32_set(unit, config_regs[i], REG_PORT_ANY, 0, reg_val[i]));
                    
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        config_reg_fields[i], 1);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        HG_LOOKUP_ENABLEf, 0);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        HYBRID_MODE_ENABLEf, 0);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        USE_MH_VIDf, 1);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        USE_MH_PKT_PRIf, 1);
                    soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                        REMOVE_MH_SRC_PORTf, 1);

                    SOC_IF_ERROR_RETURN
                        (WRITE_IHG_LOOKUPr(unit, local_ingress_port, val));

                    return BCM_E_NONE;
                }
            }
            
            return BCM_E_FULL;
        } else {
            /* We don't support BCM_PROXY_MODE_LOOPBACK in this per-port
             * version of the API.
             */
            return BCM_E_PARAM;
        }

        /* Invert sense for HW settings */
        soc_reg_field_set(unit, IHG_LOOKUPr, &val, USE_MH_VIDf,
                proxy_server->flags & BCM_PROXY_SERVER_KEEP_VID ? 0 : 1);

        soc_reg_field_set(unit, IHG_LOOKUPr, &val, USE_MH_PKT_PRIf,
                proxy_server->flags & BCM_PROXY_SERVER_KEEP_PRIORITY ? 0 :1);

        /* Set "magic destination" server port to trigger proxy lookups */
        values[0] = modid;
        values[1] = server_port;

        if (SOC_REG_FIELD_VALID(unit, HG_LOOKUP_DESTINATIONr,
                                 DST_PORT_MASKf) &&
            (proxy_server->flags & BCM_PROXY_SERVER_MASK_PORT)) {
            values[2] = 1;
            field_num = 3;
        } else {
            field_num = 2;
        }

        SOC_IF_ERROR_RETURN
            (soc_reg_fields32_modify(unit, HG_LOOKUP_DESTINATIONr,
                                     local_ingress_port, field_num,
                                     fields, values));

        SOC_IF_ERROR_RETURN
            (WRITE_IHG_LOOKUPr(unit, local_ingress_port, val));
    } else {
        if (NULL == proxy_server) {
            return BCM_E_PARAM;
        }
        if (BCM_PROXY_MODE_SECOND_PASS == proxy_server->mode) {
            if (soc_feature(unit, soc_feature_hg_proxy_second_pass)) {

                if ((proxy_server->flow_type > bcmProxySecondPassFlowTypeTrillTermination) ||
                    (proxy_server->flow_type < bcmProxySecondPassFlowTypeNone)) {
                    return BCM_E_PARAM;
                }

                for(i = 0; i < _BCM_PROXY_RESERVED_DGLP_NUMBER; i++) {
                    SOC_IF_ERROR_RETURN
                        (soc_reg32_get(unit, config_regs[i], REG_PORT_ANY, 0, &reg_val[i]));

                    flow_type[i] = soc_reg_field_get(unit,config_regs[i], reg_val[i], FLOW_TYPEf);

                    if (proxy_server->flow_type == flow_type[i]) {
                        int dglp_is_shared = 0;
                        bcm_gport_t local_port = 0;
                        uint32 return_val = 0;

                        soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                            config_reg_fields[i], 0);
                        soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                            USE_MH_VIDf, 0);                        
                        soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                            USE_MH_PKT_PRIf, 0);
                        soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                            REMOVE_MH_SRC_PORTf, 0);

                        SOC_IF_ERROR_RETURN
                            (WRITE_IHG_LOOKUPr(unit, local_ingress_port, val));

                        BCM_PBMP_ITER(PBMP_ALL(unit),local_port) {
                            SOC_IF_ERROR_RETURN
                                (READ_IHG_LOOKUPr(unit, local_port, &return_val));
                            if (soc_reg_field_get(unit, IHG_LOOKUPr, return_val, config_reg_fields[i])) {
                                dglp_is_shared = 1;
                            }
                        }
    
                        if(!dglp_is_shared) {    
                            soc_reg_field_set(unit,config_regs[i],
                                &reg_val[i], FLOW_TYPEf,0);
                            
                            soc_reg_field_set(unit,config_regs[i],
                                &reg_val[i], DST_MODIDf, 0);
                            soc_reg_field_set(unit,config_regs[i],
                                &reg_val[i], DST_PORTf, 0);
                            
                            SOC_IF_ERROR_RETURN
                                (soc_reg32_set(unit, config_regs[i], REG_PORT_ANY, 0, reg_val[i]));
                        }
                
                        return BCM_E_NONE;
                    }
                }
                return BCM_E_NOT_FOUND;
            } else {
                return BCM_E_UNAVAIL;
            }
        }
        /* Clear server configuration, but retain the independent
         * source port knockout configuration .
         */
        rmsp = soc_reg_field_get(unit, IHG_LOOKUPr, val, 
                                 REMOVE_MH_SRC_PORTf);
        val = 0;
        soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                          REMOVE_MH_SRC_PORTf, rmsp);
        SOC_IF_ERROR_RETURN
            (WRITE_IHG_LOOKUPr(unit, local_ingress_port, val));

        /* Clear the HG_LOOKUP_DESTINATION config */
        SOC_IF_ERROR_RETURN
            (soc_reg32_set(unit, HG_LOOKUP_DESTINATIONr,
                           local_ingress_port, 0, 0));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_proxy_server_port_get
 * Purpose:
 *      This API retrieves the current proxy server configuration on a
 *      given Higig stack port, if any.
 * Parameters:
 *      unit - (IN) Unit number.
 *      local_ingress_port - (IN) Incoming Higig stack port
 *      proxy_server - (OUT) Proxy server configuration
 *      enable - (OUT) Enable proxy server on selected Higig stack port
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_proxy_server_port_get(int unit, bcm_gport_t local_ingress_port, 
                              bcm_proxy_server_t *proxy_server, 
                              int *enable)
{
    bcm_module_t    modid;
    bcm_gport_t     server_port;
    uint32          cfg_val, dst_val;
    int             hglkup, hybrid;
    bcm_proxy_mode_t mode;
    bcm_module_t reserved_modid = 0;
    bcm_gport_t reserved_port = 0;
    bcm_gport_t server_gport = 0;
    int i = 0;
    uint32 flow_type[_BCM_PROXY_RESERVED_DGLP_NUMBER] = {0};
    uint32 reg_val[_BCM_PROXY_RESERVED_DGLP_NUMBER] = {0};
    static soc_reg_t config_regs[] = {
        ING_PACKET_PROCESSING_CONTROL_0r,
        ING_PACKET_PROCESSING_CONTROL_1r,
    };    
    static soc_field_t config_reg_fields[] = {
        ING_PACKET_PROCESSING_ENABLE_0f,
        ING_PACKET_PROCESSING_ENABLE_1f
    };


    if (!soc_feature(unit, soc_feature_proxy_port_property)) {
        return BCM_E_UNAVAIL;
    }

    if ((NULL == proxy_server) || (NULL == enable)) {
        /* No way to return configuration */
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(local_ingress_port)) {
        /* Note, modid is a throwaway arg here */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_proxy_gport_resolve(unit, local_ingress_port,
                                          &local_ingress_port, &modid, 1));
    } else if (!SOC_PORT_VALID(unit, local_ingress_port)) { 
        return BCM_E_PORT; 
    }

    if (!(IS_HG_PORT(unit, local_ingress_port) ||
          IS_CPU_PORT(unit, local_ingress_port))) {
        
        /* Only Higig ports (including the CPU port as
         * Higig) are valid.
         */
        return BCM_E_PORT;
    }

    SOC_IF_ERROR_RETURN
        (READ_IHG_LOOKUPr(unit, local_ingress_port, &cfg_val));

    if (BCM_PROXY_MODE_SECOND_PASS == proxy_server->mode) {
        
        if (!soc_feature(unit, soc_feature_hg_proxy_second_pass)) {
            return BCM_E_UNAVAIL;
        }

        for(i = 0; i < _BCM_PROXY_RESERVED_DGLP_NUMBER; i++) {
            SOC_IF_ERROR_RETURN
                (soc_reg32_get(unit, config_regs[i], REG_PORT_ANY, 0, &reg_val[i]));
            flow_type[i] = soc_reg_field_get(unit,config_regs[i], reg_val[i], FLOW_TYPEf);

            if (proxy_server->flow_type == flow_type[i]) {
                
                reserved_modid = soc_reg_field_get(unit,
                                    config_regs[i], reg_val[i], DST_MODIDf);

                reserved_port = soc_reg_field_get(unit,
                                    config_regs[i], reg_val[i], DST_PORTf);

                BCM_GPORT_PROXY_SET(server_gport, reserved_modid, reserved_port);

                proxy_server->server_gport = server_gport;
                    
                *enable = soc_reg_field_get(unit, IHG_LOOKUPr, cfg_val, 
                                 config_reg_fields[i]);
                
                return BCM_E_NONE;
            }
        }
        
        return BCM_E_NOT_FOUND;
    }

    hglkup = soc_reg_field_get(unit, IHG_LOOKUPr, cfg_val,
                               HG_LOOKUP_ENABLEf);
    hybrid = soc_reg_field_get(unit, IHG_LOOKUPr, cfg_val,
                               HYBRID_MODE_ENABLEf);

    if (hglkup) {
        if (hybrid) {
            /* Illegal configuration */
            return BCM_E_INTERNAL;
        } else {
            *enable = TRUE;
            mode = BCM_PROXY_MODE_HIGIG;
        }
    } else {
        if (hybrid) {
            *enable = TRUE;
            mode = BCM_PROXY_MODE_HYBRID;
        } else {
            *enable = FALSE;
            /* Nothing else to do */
            return BCM_E_NONE;
        }
    }

    bcm_proxy_server_t_init(proxy_server);

    proxy_server->mode = mode;

    /* Invert sense from HW settings */
    if (0 == soc_reg_field_get(unit, IHG_LOOKUPr, cfg_val, USE_MH_VIDf)) {
        proxy_server->flags |= BCM_PROXY_SERVER_KEEP_VID;
    }

    if (0 == soc_reg_field_get(unit, IHG_LOOKUPr, cfg_val, USE_MH_PKT_PRIf)) {
        proxy_server->flags |= BCM_PROXY_SERVER_KEEP_PRIORITY;
    }

    
    SOC_IF_ERROR_RETURN
        (soc_reg32_get(unit, HG_LOOKUP_DESTINATIONr,
                       local_ingress_port, 0, &dst_val));

    if (SOC_REG_FIELD_VALID(unit, HG_LOOKUP_DESTINATIONr,
                            DST_PORT_MASKf) &&
        soc_reg_field_get(unit, HG_LOOKUP_DESTINATIONr,
                          dst_val, DST_PORT_MASKf)) {
        proxy_server->flags |= BCM_PROXY_SERVER_MASK_PORT;
    }

    modid = soc_reg_field_get(unit, HG_LOOKUP_DESTINATIONr,
                              dst_val, DST_MODIDf);
    server_port = soc_reg_field_get(unit, HG_LOOKUP_DESTINATIONr,
                                    dst_val, DST_PORTf);
    BCM_GPORT_PROXY_SET(proxy_server->server_gport, modid, server_port);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_proxy_cleanup_data
 * Purpose:
 *      Uninstall and release proxy data
 * Parameters:
 *      unit        -  BCM Unit number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_cleanup_data(int unit)
{
    _bcm_proxy_ifc_t *ifc;

    if (_BCM_PROXY_LOCK(unit)) {
        return BCM_E_INIT;
    }

    /* Cleanup clients/servers */

    while (_bcm_proxy_control[unit].proxy_list) {
        if (_bcm_proxy_control[unit].proxy_list->flags &
            _BCM_PROXY_INFO_SERVER) {
            ifc = &server_methods;
        } else {
            ifc = &client_methods;
        }
        (void)_bcm_esw_proxy_uninstall
            (unit, ifc, _bcm_proxy_control[unit].proxy_list);
    }

    _BCM_PROXY_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_proxy_init
 * Purpose:
 *      Initialize the Proxy subsystem
 * Parameters:
 *      unit        -  BCM Unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_init(int unit)
{
    int rv = BCM_E_NONE;

    if (_LOCK(unit) == NULL) {
        _LOCK(unit) = sal_mutex_create("bcm_proxy_lock");
        rv = (_LOCK(unit) == NULL) ? BCM_E_MEMORY : BCM_E_NONE;
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_esw_proxy_cleanup_data(unit);
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_proxy_cleanup
 * Purpose:
 *      Deinitialize the Proxy subsystem
 * Parameters:
 *      unit        -  BCM Unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_cleanup(int unit)
{
    if (_LOCK(unit) != NULL) {
        (void) _bcm_esw_proxy_cleanup_data(unit);
        sal_mutex_destroy(_LOCK(unit));
        _LOCK(unit) = NULL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_proxy_egress_create
 * Purpose:
 *      Create Proxy Egress forwarding object.
 * Parameters:
 *      unit    - (IN)  bcm device.
 *      flags   - (IN)  BCM_PROXY_REPLACE: replace existing.
 *                      BCM_PROXY_WITH_ID: intf argument is given.
 *      proxy_egress     - (IN) Egress forwarding destination.
 *      proxy_if_id    - (OUT) proxy interface id pointing to Egress object.
 *                      This is an IN argument if either BCM_PROXY_REPLACE
 *                      or BCM_PROXY_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_proxy_egress_create(int unit, uint32 flags, bcm_proxy_egress_t *proxy_egress, 
                         bcm_if_t *proxy_if_id)
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_TRIDENT2_SUPPORT)
    if (SOC_IS_TD2_TT2(unit) && soc_feature(unit, soc_feature_l3)) {
        bcm_proxy_egress_t proxy_egress_local;

        /* Input parameters check. */
        if ((NULL == proxy_egress) || (NULL == proxy_if_id)) {
            return (BCM_E_PARAM);
        }

        /* Copy provided structure to local so it can be modified. */
        sal_memcpy(&proxy_egress_local, proxy_egress, sizeof(bcm_proxy_egress_t));

        L3_LOCK(unit);
        rv = bcm_td2_proxy_egress_create(unit, flags, &proxy_egress_local, proxy_if_id);
        L3_UNLOCK(unit);
    }
#endif /* BCM_TRIDENT2_SUPPORT*/

    return rv;
}

/*
 * Function:
 *      bcm_esw_proxy_egress_destroy
 * Purpose:
 *      Destroy Proxy Egress forwarding destination.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      proxy_if_id    - (IN) proxy interface id pointing to Egress object
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_proxy_egress_destroy(int unit, bcm_if_t proxy_if_id)
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_TRIDENT2_SUPPORT)
    if (SOC_IS_TD2_TT2(unit) && soc_feature(unit, soc_feature_l3)) {

        L3_LOCK(unit);
        rv = bcm_td2_proxy_egress_destroy(unit, proxy_if_id);
        L3_UNLOCK(unit);
    }
#endif /* BCM_TRIDENT2_SUPPORT*/

    return rv;
}

/*
 * Function:
 *      bcm_esw_proxy_egress_get
 * Purpose:
 *      Get Proxy Egress forwarding destination.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      proxy_if_id    - (IN) Proxy Egress destination
 *      proxy_egress    - (OUT) Egress forwarding destination.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_proxy_egress_get (int unit, bcm_if_t proxy_if_id, bcm_proxy_egress_t *proxy_egress) 
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_TRIDENT2_SUPPORT)
    if (SOC_IS_TD2_TT2(unit) && soc_feature(unit, soc_feature_l3)) {
        /* Input parameters check. */
        if (NULL == proxy_egress) {
            return (BCM_E_PARAM);
        }

        L3_LOCK(unit);
        rv = bcm_td2_proxy_egress_get (unit, proxy_if_id, proxy_egress);
        L3_UNLOCK(unit);
    }
#endif /* BCM_TRIDENT2_SUPPORT*/

    return rv;
}


/*
 * Function:
 *      bcm_esw_proxy_egress_traverse
 * Purpose:
 *      Goes through proxy egress objects table and runs the user callback
 *      function at each valid egress objects entry passing back the
 *      information for that object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function. 
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_proxy_egress_traverse(int unit, 
                           bcm_proxy_egress_traverse_cb trav_fn, void *user_data)
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_TRIDENT2_SUPPORT)
    if (SOC_IS_TD2_TT2(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_td2_proxy_egress_traverse(unit, trav_fn, user_data);
        L3_UNLOCK(unit);

    }
#endif /* BCM_TRIDENT2_SUPPORT*/

    return rv;
}

#else /* INCLUDE_L3 */
int _bcm_esw_proxy_not_empty;
#endif /* INCLUDE_L3 */

