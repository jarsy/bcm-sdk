/*
 * $Id: ipmc.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * IP Multicast API
 */

#include <shared/bsl.h>

#include <soc/defs.h>

#ifdef INCLUDE_L3

#include <soc/drv.h>
#include <bcm/stack.h>
#include <bcm/vlan.h>
#include <bcm/trunk.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3.h>


#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/tunnel.h>
#include <bcm/pkt.h>
#include <shared/gport.h>

#include <bcm_int/common/multicast.h>

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/ipmc.h>


#include <bcm_int/sbx/caladan3/g3p1.h>


STATIC int
_bcm_caladan3_find_ipmc_index_port_in_ete(_caladan3_l3_ete_t         *m_ete,
                                        int                     ipmc_index,
                                        bcm_port_t              port,
                                        int                    *found_slot);
STATIC int
_bcm_caladan3_ipmc_flow_delete(int unit,
                             _caladan3_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t *data,
                             int  remove_egress);

#define V4MC_STR_SEL_RULE 0

/*
 * Flags that are valid on sbx
 *   BCM_IPMC_SOURCE_PORT_NOCHECK     no src port check (XGS)
 *   BCM_IPMC_REPLACE                 update existing entry 
 *   BCM_IPMC_IP6                     IPv6 entry (vs IPv4)
 */

#define _CALADAN3_BCM_IPMC_L3_SUPP_FLAGS (BCM_L3_WITH_ID  | \
                                      BCM_L3_TGID     | \
                                      BCM_L3_UNTAG)

#define _CALADAN3_BCM_IPMC_SUPP_FLAGS (BCM_IPMC_SOURCE_PORT_NOCHECK |       \
                                   BCM_IPMC_COPYTOCPU           |       \
                                   BCM_IPMC_IP6                 |       \
                                   BCM_IPMC_REPLACE)

#ifdef IPMC_DEVTEST
int skip_egress_intf_set = 0;

void
_bcm_caladan3_ipmc_dump_one_egress(_caladan3_l3_fe_instance_t *l3_fe,
                                 _caladan3_l3_intf_t         *l3_intf,
                                 _caladan3_l3_ete_t          *m_ete)
{
    int                      ii;

    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] intf(0x%x) ete(0x%x,%s,%d) ohi(0x%x)\n"),
                 l3_fe->fe_unit, FUNCTION_NAME(),
                 l3_intf->if_info.l3a_intf_id,
                 m_ete->l3_ete_hw_idx.ete_idx,
                 (m_ete->l3_ete_kvidop == _CALADAN3_L3_ETE_VIDOP__NOP)?"untag":"tag",
                 m_ete->l3_ete_kttl, m_ete->l3_ohi.ohi));

    for (ii = 0; ii < m_ete->l3_inuse_ue; ii++) {
        LOG_VERBOSE(BSL_LS_BCM_IPMC,
                    (BSL_META_U(unit,
                                "<0x%x,%d> "),
                     m_ete->l3_ete_u.vc_ipmc[ii].ipmc_index,
                     m_ete->l3_ete_u.vc_ipmc[ii].port));
    }
    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "\n")));
    return;
}

void
_bcm_caladan3_ipmc_dump_all_egress(_caladan3_l3_fe_instance_t *l3_fe,
                                 int                     ipmc_index,
                                 bcm_port_t              port)
{
    _caladan3_l3_intf_t         *l3_intf;
    _caladan3_l3_ete_t          *l3_ete;
    int                      ignore;
    
    _CALADAN3_ALL_L3INTF(l3_fe, l3_intf) {
        _CALADAN3_ALL_L3ETE_ON_INTF(l3_intf, l3_ete) {
            
            /**
             * We add to the first m_ete that we find.
             */
            if (l3_ete->l3_ete_ktype == _CALADAN3_L3_ETE__MCAST_IP) {
                
                if (ipmc_index &&
                    _bcm_caladan3_find_ipmc_index_port_in_ete(l3_ete,
                                                            ipmc_index,
                                                            port,
                                                            &ignore) == BCM_E_NONE) {
                    _bcm_caladan3_ipmc_dump_one_egress(l3_fe,
                                                     l3_intf,
                                                     l3_ete);
                } else if (!ipmc_index) {
                    _bcm_caladan3_ipmc_dump_one_egress(l3_fe,
                                                     l3_intf,
                                                     l3_ete);
                }
            }
            
        } _CALADAN3_ALL_L3ETE_ON_INTF_END(l3_intf, l3_ete);
        
    } _CALADAN3_ALL_L3INTF_END(l3_fe, l3_intf);
    
    return;
}
#endif /* IPMC_DEVTEST */

/*
 * Function:
 *      _bcm_caladan3_alloc_ipmc_route
 * Description:
 *      allocate a new ipmc route
 * Parameters:
 *      l3_fe   - L3 fe instance pointer
 *      ipmc_rt - return ptr to ipmc route
 * Returns:
 *      BCM_E_NONE   - on success
 *      BCM_E_MEMORY - on failure
 * Assumption:
 */

int
_bcm_caladan3_alloc_ipmc_route(_caladan3_l3_fe_instance_t   *l3_fe,
                             _caladan3_ipmc_route_t      **ipmc_rt)
{

    *ipmc_rt = (_caladan3_ipmc_route_t *)
        sal_alloc(sizeof(_caladan3_ipmc_route_t),
                  "IPMC-rt");
    if (*ipmc_rt == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(*ipmc_rt, 0, sizeof(_caladan3_ipmc_route_t));

    DQ_INIT(&(*ipmc_rt)->id_link);

    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META("unit %d: [%s] allocated(0x%x) for ipmc-rt\n"),
                 l3_fe->fe_unit, FUNCTION_NAME(),
                 (unsigned int)*ipmc_rt));
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_free_ipmc_route
 * Description:
 *      Free ipmc route
 * Parameters:
 *      l3_fe   - L3 fe instance pointer
 *      ipmc_rt - ipmc route ptr to ptr
 * Returns:
 * Assumption:
 */

void
_bcm_caladan3_free_ipmc_route(_caladan3_l3_fe_instance_t *l3_fe,
                            _caladan3_ipmc_route_t **ipmc_rt)
{
    _caladan3_ipmc_route_t *rt = *ipmc_rt;
    
    if (!DQ_EMPTY(&rt->id_link)) {
        DQ_REMOVE(&rt->id_link);
    }

    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META("unit %d: [%s] freed(0x%x) for ipmc-rt\n"),
                 l3_fe->fe_unit, FUNCTION_NAME(),
                 (unsigned int)rt));
    
    sal_free(rt);
    *ipmc_rt = NULL;
}

/*
 * Function:
 *      _bcm_caladan3_insert_ipmc_route
 * Description:
 *      insert the route into the ipmc rt hash
 * Parameters:
 *      l3_fe      - L3 fe instance
 *      ipmc_index - ipmc index for this route
 *      ipmc_rt    - ipmc route 
 * Returns:
 * Notes:
 * Assumption:
 */

void
_bcm_caladan3_insert_ipmc_route(_caladan3_l3_fe_instance_t *l3_fe,
                              int                     ipmc_index,
                              _caladan3_ipmc_route_t     *ipmc_rt)
{
    int   hidx;
    
    hidx = _CALADAN3_IPMC_HASH_IDX(ipmc_index);

    ipmc_rt->ipmc_index = ipmc_index;

    DQ_INSERT_HEAD(&l3_fe->fe_ipmc_by_id[hidx],
                   &ipmc_rt->id_link);

    return;
}


/*
 * Function:
 *      _bcm_caladan3_find_ipmc_route
 * Description:
 *      find route from ipmc rt hash
 * Parameters:
 *      unit       - caladan3 unit
 *      l3_fe      - L3 fe instance
 *      ipmc_index - ipmc index
 *      ipmc_rt    - ipmc route 
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_NOT_FOUND - if not found
 * Notes:
 * Assumption:
 */

int
_bcm_caladan3_find_ipmc_route(int                      unit,
                            _caladan3_l3_fe_instance_t  *l3_fe,
                            int                      ipmc_index,
                            _caladan3_ipmc_route_t     **ipmc_rt)
{
    int                  hidx;
    _caladan3_ipmc_route_t  *rt = NULL;

    hidx = _CALADAN3_IPMC_HASH_IDX(ipmc_index);
    
    _CALADAN3_IPMC_RT_PER_BKT(l3_fe, hidx, rt) {

        if (rt->ipmc_index == ipmc_index) {
            LOG_VERBOSE(BSL_LS_BCM_IPMC,
                        (BSL_META_U(unit,
                                    "unit %d: [%s] found ipmc-rt with ipmc-index(0x%x)\n"),
                         unit, FUNCTION_NAME(), ipmc_index));
            *ipmc_rt = rt;
            return BCM_E_NONE;
        }
        
    } _CALADAN3_IPMC_RT_PER_BKT_END(l3_fe, hidx, rt);

    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] ipmc-rt with ipmc-index(0x%x) not-found\n"),
                 unit, FUNCTION_NAME(), ipmc_index));
    
    return BCM_E_NOT_FOUND;
}



/*
 * Function:
 *      _bcm_caladan3_fill_data_key_from_index
 * Description:
 *      given ipmc_index, we fill the s,g info
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe instance
 *      fe         - SB fe ptr
 *      data       - new info for ipmc route
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */

STATIC int
_bcm_caladan3_fill_data_key_from_index(int                    unit,
                                     _caladan3_l3_fe_instance_t *l3_fe,
                                     int                     ipmc_index,
                                     bcm_ipmc_addr_t        *data)
{
    _caladan3_ipmc_route_t     *ipmc_rt;
    int                     status;

    status = _bcm_caladan3_find_ipmc_route(unit,
                                         l3_fe,
                                         ipmc_index,
                                         &ipmc_rt);
    if (status != BCM_E_NONE) {
        return status;
    }

    /* with index overwrites the <s,g,vid> from user */
    bcm_ipmc_addr_t_init(data);
    data->s_ip_addr = ipmc_rt->ipmc_user_info.s_ip_addr;
    data->mc_ip_addr = ipmc_rt->ipmc_user_info.mc_ip_addr;
    data->vid = ipmc_rt->ipmc_user_info.vid;
    sal_memcpy(data->s_ip6_addr, ipmc_rt->ipmc_user_info.s_ip6_addr, sizeof(bcm_ip6_t));
    sal_memcpy(data->mc_ip6_addr, ipmc_rt->ipmc_user_info.mc_ip6_addr, sizeof(bcm_ip6_t));
    data->mc_ip_addr = ipmc_rt->ipmc_user_info.mc_ip_addr;

    data->flags      = ipmc_rt->ipmc_user_info.flags;
    data->group      = ipmc_index;
    
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_caladan3_find_ipmc_index_port_in_ete
 * Description:
 *      given a sw ete find the <ipmc_index, port> in the usage field
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe instance
 *      l3_intf    - L3 intf ptr
 *      m_ete      - ipmc sw ete
 *      ipmc_index - ipmc index
 *      port       - port
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */

STATIC int
_bcm_caladan3_find_ipmc_index_port_in_ete(_caladan3_l3_ete_t         *m_ete,
                                        int                     ipmc_index,
                                        bcm_port_t              port,
                                        int                    *found_slot)
{
    int                ii;

    *found_slot = -1;
    for (ii = 0; ii < m_ete->l3_alloced_ue; ii++) {
        if ((m_ete->u.l3_ipmc[ii].ipmc_index == ipmc_index) &&
            (m_ete->u.l3_ipmc[ii].port == port)) {
            *found_slot = ii;
            return BCM_E_NONE;
        }
    }

    return BCM_E_NOT_FOUND;
}



/*
 * Function:
 *      _bcm_caladan3_ipmc_delete_all_reference_to_ipmc_index
 * Description:
 *      delete all egress references for given ipmc index
 * Parameters:
 *      l3_fe      - L3 fe ptr
 *      ipmc_index - ipmc index
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 */

int
_bcm_caladan3_ipmc_delete_all_reference_to_ipmc_index(_caladan3_l3_fe_instance_t *l3_fe,
                                                    int                     ipmc_index)
{
    _caladan3_l3_ete_t          *m_ete = NULL;
    _caladan3_l3_intf_t         *l3_intf = NULL;
    int                      ii;
    int                      status, last_error_status;

    last_error_status = BCM_E_NONE;

    _CALADAN3_ALL_IPMCETE_ON_ALLINTF(l3_fe, l3_intf, m_ete) {
        
        for (ii = 0; ii < m_ete->l3_inuse_ue; ii++) {
            if (m_ete->u.l3_ipmc[ii].ipmc_index == ipmc_index) {
                m_ete->l3_inuse_ue--;
                if (m_ete->l3_inuse_ue == 0) {
                    status = 
                        _bcm_caladan3_l3_sw_ete_destroy(l3_fe->fe_unit,
                                                      l3_fe,
                                                      l3_intf,
                                                      &m_ete);
                    if (status != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_IPMC,
                                  (BSL_META("unit %d: [%s] error(%s) destroy-m-ete on l3a-intf(0x%x)\n"),
                                   l3_fe->fe_unit, FUNCTION_NAME(),
                                   bcm_errmsg(status),
                                   l3_intf->if_info.l3a_intf_id));
                        last_error_status = status;
                    }
                    goto next_m_ete;
                } else {
                    /* copy the last into this slot. */            
                    m_ete->u.l3_ipmc[ii] = 
                        m_ete->u.l3_ipmc[m_ete->l3_inuse_ue];
                }
            }
        }
        
      next_m_ete:
        LOG_DEBUG(BSL_LS_BCM_IPMC,
                  (BSL_META("unit %d: [%s] moving to next m-ete\n"),
                   l3_fe->fe_unit, FUNCTION_NAME()));
        
    } _CALADAN3_ALL_IPMCETE_ON_ALLINTF_END(l3_fe, l3_intf, m_ete);
    
    
    return last_error_status;
}


/*
 * Function:
 *      _bcm_caladan3_ipmc_l3_encap_get
 * Description:
 *      given ipmc-index, port and intf, get the
 *      encap-id
 * Parameters:
 *      unit       - FE unit
 *      ipmc_index - ipmc index
 *      gport      - gport
 *      intf       - interface id
 *      encap_id   - returned encap_id
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 *      This routine is provided to be accessed from outside
 *      of IPMC module and hence locks and unlocks the resources
 *
 */
 
int
_bcm_caladan3_ipmc_l3_encap_get(int                     unit,
                              int                     ipmc_index,
                              bcm_gport_t             gport,
                              bcm_if_t                intf_id,
                              bcm_if_t               *encap_id)
{
    _caladan3_l3_fe_instance_t  *l3_fe;
    _caladan3_l3_intf_t         *l3_intf = NULL;
    _caladan3_l3_ete_t          *m_ete = NULL;
    int                      status;
    int                      ignore;
    int                      gport_modid;
    int                      gport_port;
    
    /* Validate data */    
    if ((encap_id == NULL) || !ipmc_index) {
        return BCM_E_PARAM;
    }
    if (!_CALADAN3_L3_USER_IFID_VALID(intf_id)) {
        return BCM_E_PARAM;
    }
    if (!BCM_GPORT_IS_MODPORT(gport)) {
        return BCM_E_PARAM;
    }
    gport_modid = BCM_GPORT_MODPORT_MODID_GET(gport);
    gport_port  = BCM_GPORT_MODPORT_PORT_GET(gport);
    if (!SOC_PORT_VALID(unit, gport_port)) {
        return BCM_E_PARAM;
    }
    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (gport_modid != l3_fe->fe_my_modid) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    intf_id = _CALADAN3_IFID_FROM_USER_HANDLE(intf_id);
    status  = _bcm_caladan3_l3_find_intf_by_ifid(l3_fe,
                                               intf_id,
                                               &l3_intf);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] ipmc-index(0x%x) port(%d) unable to find l3-interface with id(%d)\n"),
                   unit, FUNCTION_NAME(),
                   ipmc_index, gport_port, intf_id));
        L3_UNLOCK(unit);
        return status;
    }

    _CALADAN3_ALL_L3ETE_ON_INTF(l3_intf, m_ete) {

        if (m_ete->l3_ete_key.l3_ete_hk.type == _CALADAN3_L3_ETE__MCAST_IP) {
            status =
                _bcm_caladan3_find_ipmc_index_port_in_ete(m_ete,
                                                        ipmc_index,
                                                        gport_port,
                                                        &ignore);
            if (status == BCM_E_NONE) {
                *encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(m_ete->l3_ohi.ohi);
                L3_UNLOCK(unit);
                return BCM_E_NONE;
            }
        }
        
    } _CALADAN3_ALL_L3ETE_ON_INTF_END(l3_intf, m_ete);

    L3_UNLOCK(unit);
    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *      _bcm_caladan3_ipmc_sw_detach
 * Purpose:
 *      IPMC is being disabled, remove all
 *      software states and return hardware state
 *      to well known disabled state.
 * Parameters:
 *      unit - C3 unit
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      Assumes that bcm_l3_detach(unit) is not
 *      yet done.
 *      Cleanup of all ipmc_addr, egr_intf is done.
 *
 * Implementation Notes:
 *      
 */

STATIC int
_bcm_caladan3_ipmc_sw_detach(int unit)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    int                     ii;
    
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_PARAM;
    }

    /* detach without init ? */
    if (!(l3_fe->fe_flags & _CALADAN3_L3_FE_FLG_IPMC_INIT)) {
        return BCM_E_PARAM;
    }

    /**
     * If ipmc state is still present then we need
     * to return error to user.
     */    
    for (ii=0; ii <_CALADAN3_IPMC_HASH_SIZE; ii++) {
        if (!DQ_EMPTY(&l3_fe->fe_ipmc_by_id[ii])) {
            return BCM_E_EXISTS;
        }
    }

    l3_fe->fe_flags &= ~_CALADAN3_L3_FE_FLG_IPMC_INIT;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_ipmc_sw_init
 * Purpose:
 *      Initialize internal software structures
 *      as well as any hardware initialization
 *      required for IPMC module
 * Parameters:
 *      unit - C3 unit where IPMC needs to be enabled
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      Assumes that bcm_l3_init(unit) is done.
 *
 * Implementation Notes:
 */

STATIC int
_bcm_caladan3_ipmc_sw_init(int unit)
{
    _caladan3_l3_fe_instance_t  *l3_fe;
    int                      ii;
    
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {        
        return BCM_E_PARAM;
    }

    /* Duplicate init ? */
    if (l3_fe->fe_flags & _CALADAN3_L3_FE_FLG_IPMC_INIT) {
        BCM_IF_ERROR_RETURN(_bcm_caladan3_ipmc_sw_detach(unit));
    }

    l3_fe->fe_flags |= _CALADAN3_L3_FE_FLG_IPMC_INIT;
    
    for (ii=0; ii <_CALADAN3_IPMC_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ipmc_by_id[ii]);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_caladan3_ipmc_remove_all
 * Purpose:
 *      Remove all the IPMC LTEs
 * Parameters:
 *      unit    - C3 unit
 *      l3_fe   - L3 fe ptr
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      Assumes that bcm_l3_detach(unit) is not
 *      yet done or is in-progress.
 *
 * Implementation Notes:
 *      
 */

STATIC int
_bcm_caladan3_ipmc_remove_all(int                      unit,
                            _caladan3_l3_fe_instance_t  *l3_fe)
{
    int                     status, last_error_status;
    int                     hi;
    dq_p_t                  ipmc_rt_elem;
    _caladan3_ipmc_route_t     *ipmc_rt = NULL;
    _caladan3_l3_ete_t         *m_ete = NULL;
    _caladan3_l3_intf_t        *l3_intf = NULL;
    bcm_ipmc_addr_t         data;

    last_error_status = BCM_E_NONE;
    
    for (hi = 0; hi < _CALADAN3_IPMC_HASH_SIZE; hi++) {

        if (DQ_EMPTY(&l3_fe->fe_ipmc_by_id[hi])) {
            continue;
        }
        
        do {
            ipmc_rt_elem = DQ_HEAD(&l3_fe->fe_ipmc_by_id[hi],
                                   dq_p_t);
            _CALADAN3_IPMC_RT_FROM_ID_DQ(ipmc_rt_elem, ipmc_rt);

            bcm_ipmc_addr_t_init(&data);
            data.s_ip_addr = ipmc_rt->ipmc_user_info.s_ip_addr;
            data.mc_ip_addr = ipmc_rt->ipmc_user_info.mc_ip_addr;
            data.vid = ipmc_rt->ipmc_user_info.vid;

            status = _bcm_caladan3_ipmc_flow_delete(unit,
                                                  l3_fe,
                                                  &data,
                                                  FALSE);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_IPMC,
                          (BSL_META_U(unit,
                                      "unit %d: [%s] error(%s) deleting ipmc-rt(%d)\n"),
                           unit, FUNCTION_NAME(), bcm_errmsg(status), 
                           ipmc_rt->ipmc_index));
                return status;
            }

        } while (!DQ_EMPTY(&l3_fe->fe_ipmc_by_id[hi]));
    }

    _CALADAN3_ALL_IPMCETE_ON_ALLINTF(l3_fe, l3_intf, m_ete) {
        status =
            _bcm_caladan3_l3_sw_ete_destroy(l3_fe->fe_unit,
                                          l3_fe,
                                          l3_intf,
                                          &m_ete);
        if (status != BCM_E_NONE) {
            last_error_status = status;
        }
        
    } _CALADAN3_ALL_IPMCETE_ON_ALLINTF_END(l3_fe, l3_intf, m_ete);
    
    return last_error_status;
}


/*
 * Function:
 *      _bcm_caladan3_ipmc_key_validate
 * Description:
 *      Validate ipmc lookup key
 * Parameters:
 *      data              - IPMC entry information.
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_PARAM - on failure
 */

STATIC int
_bcm_caladan3_ipmc_key_validate(bcm_ipmc_addr_t *data)
{
    /* Input parameters check. */
    if (data == NULL) {
        return (BCM_E_PARAM);
    }

    /* Destination address must be multicast */
    /* Source address must be unicast        */
    if (data->flags & BCM_IPMC_IP6) {
        if (!BCM_IP6_MULTICAST(data->mc_ip6_addr)) {
            return (BCM_E_PARAM);
        } 
        if (BCM_IP6_MULTICAST(data->s_ip6_addr)) {
            return (BCM_E_PARAM);
        } 
    } else {
        if (!BCM_IP4_MULTICAST(data->mc_ip_addr)) {
            return (BCM_E_PARAM);
        } 
        if (BCM_IP4_MULTICAST(data->s_ip_addr)) {
            return (BCM_E_PARAM);
        }
    }

    /* Vlan id range check. */ 
    if (!BCM_VLAN_VALID(data->vid)) {
        return (BCM_E_PARAM);
    } 

    /* VRF-mcast not implemented */
    if (data->vrf != BCM_L3_VRF_DEFAULT) {
        return (BCM_E_PARAM);
    }

    if (data->flags & ~_CALADAN3_BCM_IPMC_SUPP_FLAGS) {
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_caladan3_ipmc_data_validate
 * Description:
 *      Validate ipmc data associated with
 *      a particular SG entry.
 * Parameters:
 *      data - IPMC entry information.
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_PARAM - on failure
 * Assumption:
 *      The SG key has already been validated
 */

STATIC int
_bcm_caladan3_ipmc_data_validate(int              unit,
                               int              my_modid,
                               bcm_ipmc_addr_t *data)
{
    int           status;
    bcm_pbmp_t    l2p, l2u;

    if ((data->cos < 0) || (data->cos > SBX_MAX_COS)) {
        return (BCM_E_PARAM);
    }
    
    status = bcm_vlan_port_get(unit,
                               data->vid,
                               &l2p,
                               &l2u);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) getting port info for vid(%d)\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status), data->vid));
        return status;
    }

    if (data->ts) {
        if (!SBX_TRUNK_VALID(data->port_tgid)) {
            return (BCM_E_PARAM);
        }
    } else {
        if (!SOC_PORT_VALID(unit, data->port_tgid)) {
            return (BCM_E_PARAM);
        }
    }

    /* to do ttl scoping or not */
#if 0
    if (data->ttl && !_BCM_TTL_VALID(data->ttl)) {
        return (BCM_E_PARAM);
    }

    /* no need to check data->v */
    
    if (data->cd) {
        return (BCM_E_PARAM);
    }
#endif

    if (!SOC_SBX_MODID_ADDRESSABLE(unit, data->mod_id)) {
        return (BCM_E_PARAM);
    }

    if (data->mod_id != my_modid) {
        return (BCM_E_PARAM);
    }
    
    if (data->lookup_class) {
        return (BCM_E_PARAM);
    }

    if ((data->distribution_class < 0) ||
        (data->distribution_class >= SOC_SBX_CFG(unit)->num_ds_ids)) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: invalid fabric distribution class %d\n"),
                   unit, data->distribution_class));
        return BCM_E_PARAM;
    }

    return (BCM_E_NONE);
}




/*
 * Function:
 *      _bcm_caladan3_ipmc_flow_delete
 * Description:
 *      delete the ipmc flow
 * Parameters:
 *      unit    - fe unit
 *      l3_fe   - L3 fe ptr
 *      data    - IPMC entry information.
 *      remove_egress - if true, remove the egress also
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on error
 * Assumption:
 */

STATIC int
_bcm_caladan3_ipmc_flow_delete(int                     unit,
                             _caladan3_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    int              status;

    status = _bcm_caladan3_g3p1_ipmc_flow_delete(unit, l3_fe,
                                               data, remove_egress);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_ipmc_init
 * Purpose:
 *      Initialize the IPMC module and enable IPMC support.
 * Parameters:
 *      unit - C3 unit where IPMC needs to be enabled
 * Returns:
 *      BCM_E_XXX
 *
 * Assumptions:
 *      This function has to be called before any other IPMC functions.
 * Implementation Notes:
 *      calls the internal software init
 */

int
bcm_caladan3_ipmc_init(int unit)
{
    int     status;
 
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    
    status = _bcm_caladan3_ipmc_sw_init(unit);
    
    L3_UNLOCK(unit);
   
    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "bcm_ipmc_init: unit=%d rv=%d(%s)\n"),
                 unit, status, bcm_errmsg(status)));
 
    return status;
}


/*
 * Function:
 *      bcm_caladan3_ipmc_detach
 * Purpose:
 *      Detach the IPMC module.
 * Parameters:
 *      unit - C3 unit where IPMC needs to be disabled
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX  - Failure
 *
 * Assumption:
 *
 * Implementation Notes:
 *      All relevant ipmc state maintained in SW and HW is cleared
 */

int
bcm_caladan3_ipmc_detach(int unit)
{
    int                       status;
    _caladan3_l3_fe_instance_t   *l3_fe;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    
    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    status = _bcm_caladan3_ipmc_remove_all(unit,
                                         l3_fe);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }
    
    status = _bcm_caladan3_ipmc_sw_detach(unit);

    L3_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *      bcm_caladan3_ipmc_add
 * Purpose:
 *      Add an IPMC forwarding entry
 * Parameters:
 *      unit - (IN) caladan3 unit
 *      data - (IN) info on the forwarding entry
 *      ipmc_index - (IN/OUT) mcGroup value
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      SB install key is (S,G,VLAN)
 *      IPv6 is not supported
 *      how to implement ttl threshold?
 *      mod_id is for the local unit
 *      even if the v is not set, the egr mapping is done
 *
 * Implementation Notes:
 */

int
bcm_caladan3_ipmc_add(int              unit,
                    bcm_ipmc_addr_t *data)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    int                     status;
   
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* validate user input */
    status = _bcm_caladan3_ipmc_key_validate(data);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_caladan3_ipmc_data_validate(unit,
                                            l3_fe->fe_my_modid,
                                            data);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (!_BCM_MULTICAST_IS_L3(data->group)) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }
    data->group = _BCM_MULTICAST_ID_GET(data->group);

    if (data->flags & BCM_IPMC_REPLACE) {
        status = _bcm_caladan3_g3p1_ipmc_replace(unit, l3_fe, data);
    } else {
        status = _bcm_caladan3_g3p1_ipmc_add(unit, l3_fe, data);
    }

    _BCM_MULTICAST_GROUP_SET(data->group, _BCM_MULTICAST_TYPE_L3, data->group);

    L3_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *      bcm_caladan3_ipmc_remove
 * Purpose:
 *      Remove an IPMC forwarding entry
 * Parameters:
 *      unit - (IN) caladan3 unit
 *      data - (IN) ipmc_index or (S,G) to delete the forwarding entry
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      - ipmc_index to (S,G) is a 1:1 mapping [Ref: firebolt or bcm_ipmc_get() api]
 *        If that is not the assumption, then we cannot remove the egress objects in
 *        remove.
 *
 */

int
bcm_caladan3_ipmc_remove(int              unit,
                       bcm_ipmc_addr_t *data)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    int                     status;
    int                     ipmc_group_valid = FALSE;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (_BCM_MULTICAST_IS_L3(data->group)) {
        ipmc_group_valid = TRUE;
        data->group = _BCM_MULTICAST_ID_GET(data->group);

        status = _bcm_caladan3_fill_data_key_from_index(unit,
                                                      l3_fe,
                                                      data->group,
                                                      data);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }
    }
    
    /* validate user input */
    status = _bcm_caladan3_ipmc_key_validate(data);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }
    
    /* Restore data->group since _bcm_caladan3_ipmc_flow_delete needs
     * to know if user specified a valid data->group.
     */
    if (ipmc_group_valid) {
        _BCM_MULTICAST_GROUP_SET(data->group, _BCM_MULTICAST_TYPE_L3,
                                 data->group);
    }

    status = _bcm_caladan3_ipmc_flow_delete(unit,
                                          l3_fe,
                                          data,
                                          TRUE);

    _BCM_MULTICAST_GROUP_SET(data->group, _BCM_MULTICAST_TYPE_L3, data->group);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_caladan3_ipmc_remove_all
 * Purpose:
 *      Remove all IPMC forwarding entries on this unit
 * Parameters:
 *      unit - (IN) caladan3 unit
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 * Implementation Notes:
 *
 */

int
bcm_caladan3_ipmc_remove_all(int unit)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    int                     status;


    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    status = _bcm_caladan3_ipmc_remove_all(unit, l3_fe);
    L3_UNLOCK(unit);    
    return status;
}



/*
 * Function:
 *      bcm_caladan3_ipmc_find
 * Purpose:
 *      Find an IPMC forwarding entry
 * Parameters:
 *      unit - (IN)     caladan3 unit
 *      data - (IN/OUT) info on the forwarding entry
 *             (IN)     (s,g,vid)
 *             (OUT)    ipmc_index 
 *             (IN)     Alternatively accepts ipmc_index
 *                      and returns (s,g,vid)
 *             
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 * Implementation Notes:
 */

int
bcm_caladan3_ipmc_find(int              unit,
                     bcm_ipmc_addr_t *data)
{
    _caladan3_l3_fe_instance_t *l3_fe;
    int                     status = BCM_E_NONE;

    if (data == NULL) {
        return BCM_E_PARAM;
    }
    if (!_BCM_MULTICAST_IS_L3(data->group)) {
        /* Destination address must be multicast */
        /* Source address must be unicast        */
        if (data->flags & BCM_IPMC_IP6) {
            if (!BCM_IP6_MULTICAST(data->mc_ip6_addr)) {
                return (BCM_E_PARAM);
            } 
            if (BCM_IP6_MULTICAST(data->s_ip6_addr)) {
                return (BCM_E_PARAM);
            } 
        } else {
            if (!BCM_IP4_MULTICAST(data->mc_ip_addr)) {
                return (BCM_E_PARAM);
            } 
            if (BCM_IP4_MULTICAST(data->s_ip_addr)) {
                return (BCM_E_PARAM);
            }
        }

        /* Vlan id range check. */ 
        if (!BCM_VLAN_VALID(data->vid)) {
            return (BCM_E_PARAM);
        } 

        /* VRF-mcast not implemented */
        if (data->vrf != BCM_L3_VRF_DEFAULT) {
            return (BCM_E_PARAM);
        }

        /* IPv6 not supported in this release. */ 
        if (data->flags & BCM_IPMC_IP6) {
            return (BCM_E_PARAM);
        }
    }
    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_caladan3_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (_BCM_MULTICAST_IS_L3(data->group)) {
        data->group = _BCM_MULTICAST_ID_GET(data->group);
        status = _bcm_caladan3_fill_data_key_from_index(unit,
                                                      l3_fe,
                                                      data->group,
                                                      data);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }
    }

    status = _bcm_caladan3_g3p1_ipmc_find(unit, l3_fe, data);

    _BCM_MULTICAST_GROUP_SET(data->group, _BCM_MULTICAST_TYPE_L3, data->group);

    L3_UNLOCK(unit);
    return status;
}


/* 
 * Function:
 *     _bcm_caladan3_ipmc_flush_cache
 * Purpose:
 *     Unconditionally flush the ilib cached transactions to hw
 *     Flush ipv4mcsg, ipv4mcg, ipv6mcsg, ipv6mcg tables
 * Parameters:
 *     unit - BCM Device number
 * Returns:
 *     BCM_E_NONE      - Success
 */
int _bcm_caladan3_ipmc_flush_cache(int unit)
{
    int  rv = BCM_E_UNAVAIL;
    return rv; 
}

#endif  /* INCLUDE_L3 */
