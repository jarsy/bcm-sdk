/*
 * $Id: ipmc.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * IP Multicast API
 */

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <shared/bsl.h>

#include <soc/defs.h>


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
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/l3.h>

/* externs */
extern int
_bcm_caladan3_find_ipmc_route(int                      unit,
                            _caladan3_l3_fe_instance_t  *l3_fe,
                            int                      ipmc_index,
                            _caladan3_ipmc_route_t     **ipmc_rt);
extern void
_bcm_caladan3_insert_ipmc_route(_caladan3_l3_fe_instance_t *l3_fe,
                              int                     ipmc_index,
                              _caladan3_ipmc_route_t     *ipmc_rt);
extern int
_bcm_caladan3_alloc_ipmc_route(_caladan3_l3_fe_instance_t   *l3_fe,
                             _caladan3_ipmc_route_t      **ipmc_rt);
extern void
_bcm_caladan3_free_ipmc_route(_caladan3_l3_fe_instance_t *l3_fe,
                            _caladan3_ipmc_route_t **ipmc_rt);
extern int
_bcm_caladan3_ipmc_delete_all_reference_to_ipmc_index(_caladan3_l3_fe_instance_t *l3_fe, int ipmc_index);

/* locals */

void bcm_l3_caladan3_ip6_mcify(bcm_ip6_t addr)
{
    /* Does not check if the addr is mcast,
     * so that there is no need to maintain if addr
     * is already mcified
     */
    BCM_IP6_BYTE(addr, 0) = 0;
}

int bcm_l3_caladan3_ip6_zero_addr(bcm_ip6_t addr)
{
    bcm_ip6_t zero;
    sal_memset(zero, 0, sizeof(bcm_ip6_t));
    return (BCM_IP6_ADDR_EQ(addr, zero));
}

void
_bcm_caladan3_g3p1_map_ipmc_fte(int                      unit,
                         _caladan3_l3_fe_instance_t  *l3_fe,
                         bcm_ipmc_addr_t         *data,
                         soc_sbx_g3p1_ft_t         *fte,
                         int                      update);

/*
 * Function:
 *      _bcm_caladan3_g3p1_ipv6mc_find_route
 * Description:
 *      find route from ipmc rt hash based on (s,g,vid)
 *      s is zero for (*,g,vid)
 * Parameters:
 *      unit       - fe2k unit
 *      l3_fe      - L3 fe instance
 *      fe         - SB G2 Fe ptr
 *      data       - ipmc (s, g, vid) info
 *      ipmc_rt    - ipmc route 
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_NOT_FOUND - if not found
 * Notes:
 * Assumption:
 *      User will always do BCM_IPMC_USE_IPMC_INDEX. User
 *      needs to and will manage ipmc_index globally
 *      Hence this code is non-optimized.
 */

int
_bcm_caladan3_g3p1_ipv6mc_find_route(int                      unit,
                                  _caladan3_l3_fe_instance_t  *l3_fe,
                                  soc_sbx_g3p1_state_t    *fe,
                                  bcm_ipmc_addr_t         *data,
                                  _caladan3_ipmc_route_t     **ipmc_rt,
                                  soc_sbx_g3p1_v6mc_gv_t    *v6mcg,
                                  soc_sbx_g3p1_v6mc_sgv_t  *v6mcsg,
                                  soc_sbx_g3p1_ft_t         *fte)
{
    sbStatus_t              sb_ret;
    int                     status;
    int                     flag_null_s_addr = 0;
    uint32                     ftidx;
    uint32                  mc_vsi = 0;
    soc_sbx_g3p1_pv2e_t     pv2e;
    
    flag_null_s_addr = bcm_l3_caladan3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_caladan3_ip6_mcify(data->mc_ip6_addr);

    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
      LOG_ERROR(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                 unit, FUNCTION_NAME(),
                 bcm_errmsg(status), data->port_tgid, data->vid));
      return status; 
    }

    mc_vsi = pv2e.vlan;
    /* Mask out the Top byte */
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v6mc_gv_get(unit,
                                          mc_vsi,
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          v6mcg);
    } else {
        sb_ret = soc_sbx_g3p1_v6mc_sgv_get(unit,
                                           mc_vsi,
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          *(&data->s_ip6_addr[0]),
                                          *(&data->s_ip6_addr[4]),
                                          *(&data->s_ip6_addr[8]),
                                          *(&data->s_ip6_addr[12]),
                                          v6mcsg);
                               
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        /* caller will log the error if reqd */
        LOG_VERBOSE(BSL_LS_BCM_IPMC,
                    (BSL_META_U(unit,
                                "unit %d: [%s] error(%s) returned from soc_sbx_g3p1_ipv6mcxxcomp_get()\n"),
                     unit, FUNCTION_NAME(),
                     bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN
       (soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? v6mcg->ftidx : v6mcsg->ftidx;
    sb_ret = soc_sbx_g3p1_ft_get(unit,
                            ftidx,
                            fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_get\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status)));
        return status;
    }

    data->group = fte->oi;

    status = 
        _bcm_caladan3_find_ipmc_route(unit,
                                    l3_fe,
                                    data->group,
                                    ipmc_rt);
    
    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] %s ipmc-rt with s(" IPV6_FMT ")) g(" IPV6_FMT ")) vid(%d)\n"),
                 unit, FUNCTION_NAME(),
                 (status != BCM_E_NONE) ? "not-found" : "found",
                 IPV6_PFMT(data->s_ip6_addr),
                 IPV6_PFMT(data->mc_ip6_addr), data->vid));
    
    return status;
}

int
_bcm_caladan3_g3p1_ipv4mc_find_route(int                      unit,
                                  _caladan3_l3_fe_instance_t  *l3_fe,
                                  soc_sbx_g3p1_state_t    *fe,
                                  bcm_ipmc_addr_t         *data,
                                  _caladan3_ipmc_route_t  **ipmc_rt,
                                  soc_sbx_g3p1_v4mc_gv_t  *v4mcg,
                                  soc_sbx_g3p1_v4mc_sgv_t *v4mcsg,
                                  soc_sbx_g3p1_ft_t       *fte)
{
    sbStatus_t              sb_ret;
    int                     status;
    int                     flag_null_s_addr = 0;
    uint32                     ftidx;
    uint32                  mc_vsi = 0;
    soc_sbx_g3p1_pv2e_t     pv2e;
    
    flag_null_s_addr = (data->s_ip_addr == 0)?1:0;

    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
      LOG_ERROR(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                 unit, FUNCTION_NAME(),
                 bcm_errmsg(status), data->port_tgid, data->vid));
      return status; 
    }

    mc_vsi = pv2e.vlan;
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v4mc_gv_get(unit,
                                          mc_vsi,
                                          (data->mc_ip_addr & 0x0fffffff),
                                          v4mcg);
    } else {
        sb_ret = soc_sbx_g3p1_v4mc_sgv_get(unit,
                                           mc_vsi,
                                           data->s_ip_addr,
                                           (data->mc_ip_addr & 0x0fffffff),
                                           v4mcsg);
                               
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        /* caller will log the error if reqd */
        LOG_VERBOSE(BSL_LS_BCM_IPMC,
                    (BSL_META_U(unit,
                                "unit %d: [%s] error(%s) returned from soc_sbx_g3p1_ipv4mcxxcomp_get()\n"),
                     unit, FUNCTION_NAME(),
                     bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN
       (soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? v4mcg->ftidx : v4mcsg->ftidx;
    sb_ret = soc_sbx_g3p1_ft_get(unit,
                            ftidx,
                            fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_get\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status)));
        return status;
    }

    data->group = fte->oi;

    status = 
        _bcm_caladan3_find_ipmc_route(unit,
                                    l3_fe,
                                    data->group,
                                    ipmc_rt);
    
    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] %s ipmc-rt with s(0x%08x) g(0x%08x) vid(%d)\n"),
                 unit, FUNCTION_NAME(),
                 (status != BCM_E_NONE) ? "not-found" : "found",
                 data->s_ip_addr, data->mc_ip_addr, data->vid));
    
    return status;
}

/*
 * Function:
 *      _bcm_caladan3_g3p1_ipv6mc_add
 * Description:
 *      add new ipmc route
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

#ifndef SB_G2_FE_RUN_LENGTH_ALL
#define SB_G2_FE_RUN_LENGTH_ALL 0xffffffffU
#endif

 int
_bcm_caladan3_g3p1_ipv6mc_add(int                     unit,
                          _caladan3_l3_fe_instance_t *l3_fe,
                          bcm_ipmc_addr_t        *data)
{
    soc_sbx_g3p1_v6mc_gv_t           v6mcg = {0};
    soc_sbx_g3p1_v6mc_sgv_t          v6mcsg = {0};
    soc_sbx_g3p1_ft_t                 fte;
    uint32                          fte_id = ~0;
    sbStatus_t                        sb_ret;
    int                               status, ignore_status;
    _caladan3_ipmc_route_t            *ipmc_rt;
    int                               flag_null_s_addr;
    int                               proccopy;
    soc_sbx_g3p1_pv2e_t               pv2e;
    soc_sbx_g3p1_lp_t                 lp;
    uint32                          mc_vsi = 0, 
                                      lp_idx = 0 ;

    if (( data->flags & BCM_IPMC_SOURCE_PORT_NOCHECK ) || 
       !(data->flags & BCM_IPMC_IP6)) {
       return BCM_E_PARAM;
    }

    /* Add the RPF related Changes
       1. Obtain the VSI, from Port, Vid.
       2. Lookup the PV2E to get the LP Index
       3. Lookup the LP Table to get the PID value
       4. Configure the Payload.rpfunion with the PID value.
       */
    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, 
                                    data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    /* In case the Pv2e Entry is not configured with LP entry, 
     * that would indicate an physical port 
     */
    if( pv2e.lpi == 0 ) {
        lp_idx = data->port_tgid;
    } else {
        lp_idx = pv2e.lpi;
    }
    
    status  = soc_sbx_g3p1_lp_get(l3_fe->fe_unit, lp_idx, &lp);

    if (status != BCM_E_NONE) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining lp(index-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), lp_idx));
       return status; 
    }

    /* BCM_IPMC_USE_IPMC_INDEX is always true here */
    status = _bcm_caladan3_find_ipmc_route(unit,
                                         l3_fe,
                                         data->group,
                                         &ipmc_rt);
    switch (status) {
    case BCM_E_NOT_FOUND:
        break;
    case BCM_E_NONE:
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] duplicate ipmc-add\n"),
                   unit, FUNCTION_NAME()));
        return BCM_E_EXISTS;
    default:
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) ipmc-index(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), data->group));
        return BCM_E_INTERNAL;
    }
 

    status = _bcm_caladan3_alloc_ipmc_route(l3_fe,
                                          &ipmc_rt);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) unable to allocate ipmc-rt\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status)));
        return status;
    }
    
    status = _sbx_caladan3_resource_alloc(unit,
                                     SBX_CALADAN3_USR_RES_FTE_IPMC,
                                     1,
                                     &fte_id,
                                     0);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] failure(%s) to allocate new fte\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
        return status;
    }


    proccopy = (data->flags & BCM_IPMC_COPYTOCPU)?1:0;
    bcm_l3_caladan3_ip6_mcify(data->mc_ip6_addr);
    flag_null_s_addr = bcm_l3_caladan3_ip6_zero_addr(data->s_ip6_addr);
    if (flag_null_s_addr) {
        v6mcg.proccopy = proccopy;
        v6mcg.rpfunion = lp.pid;
        v6mcg.ftidx = fte_id;
    } else {
        v6mcsg.proccopy = proccopy;
        v6mcsg.rpfunion = lp.pid;
        v6mcsg.ftidx = fte_id;
    }
    
    _bcm_caladan3_g3p1_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             FALSE);

    sb_ret = soc_sbx_g3p1_ft_set(unit, fte_id, &fte);
                               
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) setting fte(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), fte_id));
      
        /* free ipmc-rt, fte-id */
        _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_caladan3_resource_free(unit,
                                               SBX_CALADAN3_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) freeing fte(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), fte_id));
        }
        
        return status;
    }

    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x "
                             "QidLagUnion(0x%x) OI(0x%x)\n"),
                 unit, FUNCTION_NAME(),
                 data->group, fte_id,
                 fte.qid, fte.oi));
               
    ipmc_rt->ipmc_user_info = *data;
    
    _bcm_caladan3_insert_ipmc_route(l3_fe,
                                  data->group,
                                  ipmc_rt);
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v6mc_gv_set(unit,
                                          mc_vsi, 
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          &v6mcg);
    } else {
        sb_ret = soc_sbx_g3p1_v6mc_sgv_set(unit,
                                           mc_vsi, 
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          *(&data->s_ip6_addr[0]),
                                          *(&data->s_ip6_addr[4]),
                                          *(&data->s_ip6_addr[8]),
                                          *(&data->s_ip6_addr[12]),
                                           &v6mcsg);
    }
        
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) adding ipmc-flow(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), data->group));
        
        /* free ipmc-rt, fte-id */
        _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_caladan3_resource_free(unit,
                                               SBX_CALADAN3_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) freeing fte(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), fte_id));
            /* try other resource free .. */
        }

        return status;
    }
    
    return BCM_E_NONE;
}

 int
_bcm_caladan3_g3p1_ipv4mc_add(int                     unit,
                          _caladan3_l3_fe_instance_t *l3_fe,
                          bcm_ipmc_addr_t        *data)
{
    soc_sbx_g3p1_v4mc_gv_t            v4mcg = {0};
    soc_sbx_g3p1_v4mc_sgv_t           v4mcsg = {0};
    soc_sbx_g3p1_ft_t                 fte;
    uint32                          fte_id = ~0;
    sbStatus_t                        sb_ret;
    int                               status, ignore_status;
    _caladan3_ipmc_route_t               *ipmc_rt;
    int                               flag_null_s_addr;
    int                               proccopy;
    soc_sbx_g3p1_pv2e_t               pv2e;
    soc_sbx_g3p1_lp_t                 lp;
    uint32                          mc_vsi = 0, 
                                      lp_idx = 0 ;

    if ((data->flags & BCM_IPMC_SOURCE_PORT_NOCHECK ) || 
        (data->flags & BCM_IPMC_IP6)) {
       return BCM_E_PARAM;
    }

    /* Add the RPF related Changes
       1. Obtain the VSI, from Port, Vid.
       2. Lookup the PV2E to get the LP Index
       3. Lookup the LP Table to get the PID value
       4. Configure the Payload.rpfunion with the PID value.
       */
    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, 
                                    data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    /* In case the Pv2e Entry is not configured with LP entry, 
     * that would indicate an physical port 
     */
    if( pv2e.lpi == 0 ) {
        lp_idx = data->port_tgid;
    } else {
        lp_idx = pv2e.lpi;
    }
    
    status  = soc_sbx_g3p1_lp_get(l3_fe->fe_unit, lp_idx, &lp);

    if (status != BCM_E_NONE) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining lp(index-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), lp_idx));
       return status; 
    }

    /* BCM_IPMC_USE_IPMC_INDEX is always true here */
    status = _bcm_caladan3_find_ipmc_route(unit,
                                         l3_fe,
                                         data->group,
                                         &ipmc_rt);
    switch (status) {
    case BCM_E_NOT_FOUND:
        break;
    case BCM_E_NONE:
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] duplicate ipmc-add\n"),
                   unit, FUNCTION_NAME()));
        return BCM_E_EXISTS;
    default:
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) ipmc-index(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), data->group));
        return BCM_E_INTERNAL;
    }
 
    status = _bcm_caladan3_alloc_ipmc_route(l3_fe,
                                          &ipmc_rt);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) unable to allocate ipmc-rt\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status)));
        return status;
    }
    
    status = _sbx_caladan3_resource_alloc(unit,
                                     SBX_CALADAN3_USR_RES_FTE_IPMC,
                                     1,
                                     &fte_id,
                                     0);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] failure(%s) to allocate new fte\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
        return status;
    }


    proccopy = (data->flags & BCM_IPMC_COPYTOCPU)?1:0;
    flag_null_s_addr = (data->s_ip_addr == 0)?1:0;
    if (flag_null_s_addr) {
        v4mcg.proccopy = proccopy;
        v4mcg.rpfunion = lp.pid;
        v4mcg.ftidx = fte_id;
    } else {
        v4mcsg.proccopy = proccopy;
        v4mcsg.rpfunion = lp.pid;
        v4mcsg.ftidx = fte_id;
    }
    
    _bcm_caladan3_g3p1_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             FALSE);

    sb_ret = soc_sbx_g3p1_ft_set(unit, fte_id, &fte);
                               
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) setting fte(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), fte_id));
      
        /* free ipmc-rt, fte-id */
        _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_caladan3_resource_free(unit,
                                               SBX_CALADAN3_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) freeing fte(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), fte_id));
        }
        
        return status;
    }

    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x "
                             "QidLagUnion(0x%x) OI(0x%x)\n"),
                 unit, FUNCTION_NAME(),
                 data->group, fte_id,
                 fte.qid, fte.oi));
               
    ipmc_rt->ipmc_user_info = *data;
    
    _bcm_caladan3_insert_ipmc_route(l3_fe,
                                  data->group,
                                  ipmc_rt);
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v4mc_gv_set(unit,
                                          mc_vsi,
                                          (data->mc_ip_addr & 0x0fffffff),
                                          &v4mcg);
    } else {
            sb_ret = soc_sbx_g3p1_v4mc_sgv_set(unit,
                                               mc_vsi, 
                                               data->s_ip_addr,
                                               (data->mc_ip_addr & 0x0fffffff),
                                               &v4mcsg);

    }
        
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) adding ipmc-flow(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), data->group));
        
        /* free ipmc-rt, fte-id */
        _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_caladan3_resource_free(unit,
                                               SBX_CALADAN3_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) freeing fte(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), fte_id));
            /* try other resource free .. */
        }

        return status;
    }
   
    return BCM_E_NONE;
}

 int
_bcm_caladan3_g3p1_ipmc_add(int                     unit,
                          _caladan3_l3_fe_instance_t *l3_fe,
                          bcm_ipmc_addr_t        *data)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status = _bcm_caladan3_g3p1_ipv6mc_add(unit, l3_fe, data); 
    } else {
        status = _bcm_caladan3_g3p1_ipv4mc_add(unit, l3_fe, data);
    }
    return status;
}

/*
 * Function:
 *      _bcm_caladan3_g3p1_ipv6mc_replace
 * Description:
 *      replace the ipmc route with new information
 * Parameters:
 *      unit       - FE unit
 *      data       - new info for ipmc route
 *      l3_fe      - L3 fe instance
 *      fe         - SB fe ptr
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */
 int
_bcm_caladan3_g3p1_ipv6mc_replace(int                     unit,
                              _caladan3_l3_fe_instance_t *l3_fe,
                              bcm_ipmc_addr_t        *data)
{
    soc_sbx_g3p1_ft_t                fte;    
    _caladan3_ipmc_route_t              *ipmc_rt;
    soc_sbx_g3p1_v6mc_gv_t           v6mcg;
    soc_sbx_g3p1_v6mc_sgv_t          v6mcsg;
    sbStatus_t                       sb_ret;
    int                              status;
    int                              flag_null_s_addr;
    uint32                           ftidx = ~0;    
    soc_sbx_g3p1_pv2e_t              pv2e;    
    uint32                           mc_vsi = 0;

    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    status = _bcm_caladan3_find_ipmc_route(unit,
                                         l3_fe,
                                         data->group,
                                         &ipmc_rt);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] ipmc-rt(%d) not found\n"),
                   unit, FUNCTION_NAME(),
                   data->group));
        return status;
    }
    
    flag_null_s_addr = bcm_l3_caladan3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_caladan3_ip6_mcify(data->mc_ip6_addr);
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v6mc_gv_get(unit, 
                                          mc_vsi,
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          &v6mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v6mcg_get\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status)));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g3p1_v6mc_sgv_get(unit,
                                           mc_vsi,
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          *(&data->s_ip6_addr[0]),
                                          *(&data->s_ip6_addr[4]),
                                          *(&data->s_ip6_addr[8]),
                                          *(&data->s_ip6_addr[12]),
                                           &v6mcsg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v6mc_sgv_get\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status)));
            return status;
        }
    }
   
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx)); 
    ftidx += (flag_null_s_addr) ? v6mcg.ftidx : v6mcsg.ftidx;
    sb_ret = soc_sbx_g3p1_ft_get(unit, ftidx, &fte);

    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_get\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status)));
        return status;
    }
    
    /**
     * update ipmc-index
     */
    if (data->group != fte.oi) {
        DQ_REMOVE(&ipmc_rt->id_link);
        _bcm_caladan3_insert_ipmc_route(l3_fe,
                                      data->group,
                                      ipmc_rt);
    }

    /* Update does not require commit */
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v6mc_gv_update(unit,
                                             mc_vsi,
                                             *(&data->mc_ip6_addr[0]),
                                             *(&data->mc_ip6_addr[4]),
                                             *(&data->mc_ip6_addr[8]),
                                             *(&data->mc_ip6_addr[12]),
                                             &v6mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v6mc_gv_update for ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), data->group));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g3p1_v6mc_sgv_update(unit,
                                              mc_vsi,
                                             *(&data->mc_ip6_addr[0]),
                                             *(&data->mc_ip6_addr[4]),
                                             *(&data->mc_ip6_addr[8]),
                                             *(&data->mc_ip6_addr[12]),
                                             *(&data->s_ip6_addr[0]),
                                             *(&data->s_ip6_addr[4]),
                                             *(&data->s_ip6_addr[8]),
                                             *(&data->s_ip6_addr[12]),
                                              &v6mcsg);
        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v6mc_sgv_update for ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), data->group));
            return status;
        }
    }
    
    _bcm_caladan3_g3p1_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             TRUE);

    ftidx = (flag_null_s_addr) ? v6mcg.ftidx : v6mcsg.ftidx;
    sb_ret = soc_sbx_g3p1_ft_set(unit,
                                 ftidx,
                                 &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_set for ipmc-index(0x%x) fte(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status),
                   data->group, ftidx));
        return status;
    }
#if 0 
    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x valid(%d) drop(%d) "
                             "QidLagUnion(0x%x) ulMcGroup(0x%x)\n"),
                 unit, FUNCTION_NAME(),
                 data->group, pay4.m_ulFtIdx,
                 fte.ulValid, fte.ulDrop, fte.ulQidLagUnion,
                 fte.ulMcGroup));
#endif
    
    ipmc_rt->ipmc_user_info = *data;
    
    return BCM_E_NONE;
}

 int
_bcm_caladan3_g3p1_ipv4mc_replace(int                     unit,
                              _caladan3_l3_fe_instance_t *l3_fe,
                              bcm_ipmc_addr_t        *data)
{
    soc_sbx_g3p1_ft_t                fte;    
    _caladan3_ipmc_route_t              *ipmc_rt;
    soc_sbx_g3p1_v4mc_gv_t           v4mcg;
    soc_sbx_g3p1_v4mc_sgv_t          v4mcsg;
    sbStatus_t                       sb_ret;
    int                              status;
    int                              flag_null_s_addr;
    uint32                           ftidx = ~0;    
    soc_sbx_g3p1_pv2e_t              pv2e;    
    uint32                           mc_vsi = 0;

    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    status = _bcm_caladan3_find_ipmc_route(unit,
                                         l3_fe,
                                         data->group,
                                         &ipmc_rt);
    if (status != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] ipmc-rt(%d) not found\n"),
                   unit, FUNCTION_NAME(),
                   data->group));
        return status;
    }
    
    flag_null_s_addr = (data->s_ip_addr)?0:1;
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v4mc_gv_get(unit, 
                                          mc_vsi,
                                          (data->mc_ip_addr & 0x0fffffff),
                                          &v4mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v4mc_gv_get\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status)));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g3p1_v4mc_sgv_get(unit,
                                           mc_vsi,
                                           data->s_ip_addr,
                                           (data->mc_ip_addr & 0x0fffffff),
                                           &v4mcsg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v4mc_sgv_get\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status)));
            return status;
        }
    }
   
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx)); 
    ftidx += (flag_null_s_addr) ? v4mcg.ftidx : v4mcsg.ftidx;
    sb_ret = soc_sbx_g3p1_ft_get(unit, ftidx, &fte);

    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_get\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status)));
        return status;
    }
    
    /**
     * update ipmc-index
     */
    if (data->group != fte.oi) {
        DQ_REMOVE(&ipmc_rt->id_link);
        _bcm_caladan3_insert_ipmc_route(l3_fe,
                                      data->group,
                                      ipmc_rt);
    }

    /* Update does not require commit */
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v4mc_gv_update(unit,
                                             mc_vsi,
                                             (data->mc_ip_addr & 0x0fffffff),
                                             &v4mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v4mc_gv_update for ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), data->group));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g3p1_v4mc_sgv_update(unit,
                                              mc_vsi,
                                              data->s_ip_addr, 
                                              (data->mc_ip_addr & 0x0fffffff), 
                                               &v4mcsg);
        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v4mc_sgv_update for ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status), data->group));
            return status;
        }
    }
    
    _bcm_caladan3_g3p1_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             TRUE);
    ftidx = (flag_null_s_addr) ? v4mcg.ftidx : v4mcsg.ftidx;
    sb_ret = soc_sbx_g3p1_ft_set(unit,
                                 ftidx,
                                 &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_set for ipmc-index(0x%x) fte(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status),
                   data->group, ftidx));
        return status;
    }
#if 0 
    LOG_VERBOSE(BSL_LS_BCM_IPMC,
                (BSL_META_U(unit,
                            "unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x valid(%d) drop(%d) "
                             "QidLagUnion(0x%x) ulMcGroup(0x%x)\n"),
                 unit, FUNCTION_NAME(),
                 data->group, pay4.m_ulFtIdx,
                 fte.ulValid, fte.ulDrop, fte.ulQidLagUnion,
                 fte.ulMcGroup));
#endif
    
    ipmc_rt->ipmc_user_info = *data;
    
    return BCM_E_NONE;
}

 int
_bcm_caladan3_g3p1_ipmc_replace(int                     unit,
                              _caladan3_l3_fe_instance_t *l3_fe,
                              bcm_ipmc_addr_t        *data)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status = _bcm_caladan3_g3p1_ipv6mc_replace(unit, l3_fe, data); 
    } else {
        status = _bcm_caladan3_g3p1_ipv4mc_replace(unit, l3_fe, data);
    }
    return status;
}

/*
 * Function:
 *      _bcm_caladan3_g3p1_map_ipmc_fte
 * Description:
 *      map various FTE fields
 * Parameters:
 *      unit    - fe unit
 *      data    - IPMC entry information.
 *      fte     - ipmc fte to be filled
 *      update  - TRUE if updating
 * Returns:
 * Assumption:
 */

void
_bcm_caladan3_g3p1_map_ipmc_fte(int                      unit,
                         _caladan3_l3_fe_instance_t  *l3_fe,
                         bcm_ipmc_addr_t         *data,
                         soc_sbx_g3p1_ft_t         *fte,
                         int                      update)
{
    if (update == FALSE) {
        soc_sbx_g3p1_ft_t_init(fte);
    }

    if (data->distribution_class) {
        
        fte->lagbase = (data->distribution_class * SBX_MAX_COS);
        fte->lagsize = 3;
        fte->lag = 1;
    }  else {
        fte->lag = 0;
        fte->lagsize = 0;
        fte->lagbase = 0;
    }

    fte->qid = SBX_MC_QID_BASE;
    fte->oi        = data->group;
    fte->mc        = 1;
#if 0 
#ifdef INCLUDE_SBX_HIGIG
    fte->ulDestMod  = SBX_HIGIG_DEST_MOD_IPMC(data->group);
    fte->ulDestPort = SBX_HIGIG_DEST_PORT_IPMC(data->group);
    fte->ulOpcode   = BCM_HG_OPCODE_IPMC;
#endif
    if (data->flags & BCM_IPMC_SETPRI) {
        fte->ulUseRCos = TRUE;
        /* XXX: How to map cos to ulRCos */
        fte->ulRCos = data->cos;
    }
#endif  
}

/*
 * Function:
 *      _bcm_caladan3_g3p1_ipv6mc_flow_delete
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

 int
_bcm_caladan3_g3p1_ipv6mc_flow_delete(int                     unit,
                             _caladan3_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    soc_sbx_g3p1_state_t *fe;
    soc_sbx_g3p1_v6mc_gv_t v6mcg;
    soc_sbx_g3p1_v6mc_sgv_t v6mcsg;
    soc_sbx_g3p1_ft_t      fte;
    _caladan3_ipmc_route_t     *ipmc_rt;
    int                     status;
    int                     ipmc_index;
    int                     flag_null_s_addr;
    uint32                  ftidx = ~0;
    soc_sbx_g3p1_pv2e_t     pv2e;
    uint32                  mc_vsi = 0;
    int                     ipmc_group_valid;

    status     = BCM_E_NONE;
    ipmc_index = -1;
    flag_null_s_addr = bcm_l3_caladan3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_caladan3_ip6_mcify(data->mc_ip6_addr);
  
    fe = (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (_BCM_MULTICAST_IS_L3(data->group)) {
        ipmc_group_valid = TRUE;
        data->group = _BCM_MULTICAST_ID_GET(data->group);
    } else {
        ipmc_group_valid = FALSE;
    }

    if (ipmc_group_valid) {
        status = _bcm_caladan3_find_ipmc_route(unit,
                                             l3_fe,
                                             data->group,
                                             &ipmc_rt);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] unable to find ipmc-rt based on ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(), data->group));
            return status;
        }

        /**
         * XXX: It is bad that we need to keep the user
         * data for this.
         */
        /* Need this???? if (ipmc_rt->ipmc_user_info.flags & BCM_IPMC_IP6)*/
            sal_memcpy(data->s_ip6_addr, 
                 ipmc_rt->ipmc_user_info.s_ip6_addr, BCM_IP6_ADDRLEN);
            sal_memcpy(data->mc_ip6_addr,
                 ipmc_rt->ipmc_user_info.mc_ip6_addr, BCM_IP6_ADDRLEN);
            mc_vsi = ipmc_rt->ipmc_user_info.vid;

    } else {
       status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
       if( status != BCM_E_NONE ) {
          LOG_ERROR(BSL_LS_BCM_IPMC,
                    (BSL_META_U(unit,
                                "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                     unit, FUNCTION_NAME(),
                     bcm_errmsg(status), data->port_tgid, data->vid));
          return status; 
        }

        mc_vsi = pv2e.vlan;

        status = _bcm_caladan3_g3p1_ipv6mc_find_route(unit,
                                                   l3_fe,
                                                   fe,
                                                   data,
                                                   &ipmc_rt,
                                                   &v6mcg,
                                                   &v6mcsg,
                                                   &fte);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] (%s) finding ipmc-rt based on <s,g,v>(" IPV6_FMT ")," IPV6_FMT "),%d)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status),
                       IPV6_PFMT(data->s_ip6_addr),
                       IPV6_PFMT(data->mc_ip6_addr), mc_vsi));
            return status;
        }
    }
    

    if (ipmc_group_valid) {    
        if (flag_null_s_addr) {
            status = soc_sbx_g3p1_v6mc_gv_get(unit,
                                              mc_vsi,
                                              *(&data->mc_ip6_addr[0]),
                                              *(&data->mc_ip6_addr[4]),
                                              *(&data->mc_ip6_addr[8]),
                                              *(&data->mc_ip6_addr[12]),
                                              &v6mcg);
        } else {
            status = soc_sbx_g3p1_v6mc_sgv_get(unit,
                                               mc_vsi,
                                              *(&data->mc_ip6_addr[0]),
                                              *(&data->mc_ip6_addr[4]),
                                              *(&data->mc_ip6_addr[8]),
                                              *(&data->mc_ip6_addr[12]),
                                              *(&data->s_ip6_addr[0]),
                                              *(&data->s_ip6_addr[4]),
                                              *(&data->s_ip6_addr[8]),
                                              *(&data->s_ip6_addr[12]),
                                               &v6mcsg);
        }
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling v6mc_xxx_get(" IPV6_FMT ")," IPV6_FMT "),%d)\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status),
                       IPV6_PFMT(data->s_ip6_addr), 
                       IPV6_PFMT(data->mc_ip6_addr), mc_vsi));
            return status;
        }
    }
    
    /**
     * Remove the LTE for this ipmc flow
     */
    if (flag_null_s_addr) {
        status = soc_sbx_g3p1_v6mc_gv_remove(unit,
                                             mc_vsi,
                                              *(&data->mc_ip6_addr[0]),
                                              *(&data->mc_ip6_addr[4]),
                                              *(&data->mc_ip6_addr[8]),
                                              *(&data->mc_ip6_addr[12]));
                                   
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v6mc_gv_remove\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    } else {
        status = soc_sbx_g3p1_v6mc_sgv_remove(unit,
                                              mc_vsi,
                                              *(&data->mc_ip6_addr[0]),
                                              *(&data->mc_ip6_addr[4]),
                                              *(&data->mc_ip6_addr[8]),
                                              *(&data->mc_ip6_addr[12]),
                                              *(&data->s_ip6_addr[0]),
                                              *(&data->s_ip6_addr[4]),
                                              *(&data->s_ip6_addr[8]),
                                              *(&data->s_ip6_addr[12]));
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v6mc_sgv_remove\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    }
    
    _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
    
    /**
     * Now remove the FTE that was attached for this
     * ipmc flow.
     */
   BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? v6mcg.ftidx :  v6mcsg.ftidx;

    status = soc_sbx_g3p1_ft_get(unit, ftidx, &fte);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_vlan_ft_base_get\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    /* save ipmc_index for later */
    ipmc_index = fte.oi;
    
    /* Invalidate the FTE */
    soc_sbx_g3p1_ft_t_init(&fte);
    
    status = soc_sbx_g3p1_ft_set(unit, ftidx, &fte);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_set\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    if (remove_egress) {
        status =
            _bcm_caladan3_ipmc_delete_all_reference_to_ipmc_index(l3_fe,
                                                                ipmc_index);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) removing all egress for ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status),
                       ipmc_index));
            return status;
        }
    }
    
    return status;
}

 int
_bcm_caladan3_g3p1_ipv4mc_flow_delete(int                     unit,
                             _caladan3_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    soc_sbx_g3p1_state_t        *fe;
    soc_sbx_g3p1_v4mc_gv_t      v4mcg;
    soc_sbx_g3p1_v4mc_sgv_t     v4mcsg;
    soc_sbx_g3p1_ft_t           fte;
    _caladan3_ipmc_route_t     *ipmc_rt;
    int                         status;
    int                         ipmc_index;
    int                         flag_null_s_addr;
    uint32                      ftidx = ~0;
    soc_sbx_g3p1_pv2e_t         pv2e;
    uint32                      mc_vsi = 0;
    int                         ipmc_group_valid;

    status     = BCM_E_NONE;
    ipmc_index = -1;
    flag_null_s_addr = (data->s_ip_addr)?0:1;
  
    fe = (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (_BCM_MULTICAST_IS_L3(data->group)) {
        ipmc_group_valid = TRUE;
        data->group = _BCM_MULTICAST_ID_GET(data->group);
    } else {
        ipmc_group_valid = FALSE;
    }

    if (ipmc_group_valid) {
        status = _bcm_caladan3_find_ipmc_route(unit,
                                             l3_fe,
                                             data->group,
                                             &ipmc_rt);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] unable to find ipmc-rt based on ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(), data->group));
            return status;
        }

        /**
         * XXX: It is bad that we need to keep the user
         * data for this.
         */
        data->s_ip_addr   = ipmc_rt->ipmc_user_info.s_ip_addr;
        data->mc_ip_addr  = ipmc_rt->ipmc_user_info.mc_ip_addr;
        mc_vsi = ipmc_rt->ipmc_user_info.vid;

    } else {
       status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
       if( status != BCM_E_NONE ) {
          LOG_ERROR(BSL_LS_BCM_IPMC,
                    (BSL_META_U(unit,
                                "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                     unit, FUNCTION_NAME(),
                     bcm_errmsg(status), data->port_tgid, data->vid));
          return status; 
        }

        mc_vsi = pv2e.vlan;

        status = _bcm_caladan3_g3p1_ipv4mc_find_route(unit,
                                                   l3_fe,
                                                   fe,
                                                   data,
                                                   &ipmc_rt,
                                                   &v4mcg,
                                                   &v4mcsg,
                                                   &fte);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] (%s) finding ipmc-rt based on <s,g,v>(0x%x,0x%x,%d)\n"),
                       unit, FUNCTION_NAME(),
                       bcm_errmsg(status),
                       data->s_ip_addr, data->mc_ip_addr, mc_vsi));
            return status;
        }
    }
    
    if (ipmc_group_valid) {
        if (flag_null_s_addr) {
            status = soc_sbx_g3p1_v4mc_gv_get(unit,
                                              mc_vsi,
                                              (data->mc_ip_addr & 0x0fffffff),
                                              &v4mcg);
        } else {
            status = soc_sbx_g3p1_v4mc_sgv_get(unit,
                                               mc_vsi,
                                               data->s_ip_addr,
                                               (data->mc_ip_addr & 0x0fffffff), 
                                               &v4mcsg);
        }
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v4mc_gv[x]_get(0x%x,0x%x,%d)\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status),
                       data->s_ip_addr, data->mc_ip_addr, mc_vsi));
            return status;
        }
    }
    
    /**
     * Remove the LTE for this ipmc flow
     */
    if (flag_null_s_addr) {
        status = soc_sbx_g3p1_v4mc_gv_remove(unit,
                                             mc_vsi,
                                             (data->mc_ip_addr & 0x0fffffff));
                                   
        if (BCM_FAILURE(status)) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v4mc_gv_remove\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    } else {
        status = soc_sbx_g3p1_v4mc_sgv_remove(unit,
                                              mc_vsi,
                                              data->s_ip_addr,
                                              (data->mc_ip_addr & 0x0fffffff));
        if (BCM_FAILURE(status)) {
                        LOG_ERROR(BSL_LS_BCM_IPMC,
                                  (BSL_META_U(unit,
                                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_v4mc_sgv_remove\n"),
                                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    }
    
    _bcm_caladan3_free_ipmc_route(l3_fe, &ipmc_rt);
    
    /**
     * Now remove the FTE that was attached for this
     * ipmc flow.
     */
   BCM_IF_ERROR_RETURN
        (soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? v4mcg.ftidx :  v4mcsg.ftidx;
    status = soc_sbx_g3p1_ft_get(unit, ftidx, &fte);

    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_vlan_ft_base_get\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    /* save ipmc_index for later */
    ipmc_index = fte.oi;
    
    /* Invalidate the FTE */
    soc_sbx_g3p1_ft_t_init(&fte);
    
    status = soc_sbx_g3p1_ft_set(unit, ftidx, &fte);
    if (BCM_FAILURE(status)) {
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) calling soc_sbx_g3p1_ft_set\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    if (remove_egress) {
        status =
            _bcm_caladan3_ipmc_delete_all_reference_to_ipmc_index(l3_fe,
                                                                ipmc_index);
        if (status != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_IPMC,
                      (BSL_META_U(unit,
                                  "unit %d: [%s] error(%s) removing all egress for ipmc-index(0x%x)\n"),
                       unit, FUNCTION_NAME(), bcm_errmsg(status),
                       ipmc_index));
            return status;
        }
    }
    
    return status;
}

 int
_bcm_caladan3_g3p1_ipmc_flow_delete(int                     unit,
                             _caladan3_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status =  _bcm_caladan3_g3p1_ipv6mc_flow_delete(unit, l3_fe,
                               data, remove_egress);
    } else {
        status =  _bcm_caladan3_g3p1_ipv4mc_flow_delete(unit, l3_fe,
                               data, remove_egress);
    }
    return status;
}


/*
 * Function:
 *      _bcm_caladan3_g3p1_ipv6mc_find
 * Description:
 *      find the ipmc flow and fill in data
 * Parameters:
 *      unit    - fe unit
 *      l3_fe   - L3 fe ptr
 *      fe      - SB fe
 *      data    - ipmc flow info (IN/OUT)
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on error
 * Assumption:
 */
int
_bcm_caladan3_g3p1_ipv6mc_find(int                      unit,
                      _caladan3_l3_fe_instance_t  *l3_fe,
                      bcm_ipmc_addr_t         *data)
{
    soc_sbx_g3p1_v6mc_gv_t    v6mcg;
    soc_sbx_g3p1_v6mc_sgv_t   v6mcsg;
    soc_sbx_g3p1_ft_t       fte;
    sbStatus_t              sb_ret;
    int                     status;
    int                     flag_null_s_addr;
    uint32                  ftidx = ~0;
    soc_sbx_g3p1_pv2e_t     pv2e;
    uint32                  mc_vsi = 0;
    
    flag_null_s_addr = bcm_l3_caladan3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_caladan3_ip6_mcify(data->mc_ip6_addr);

    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, 
                                     data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;


    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v6mc_gv_get(unit,
                                          mc_vsi,
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          &v6mcg);
    } else {
        sb_ret = soc_sbx_g3p1_v6mc_sgv_get(unit,
                                           mc_vsi,
                                          *(&data->mc_ip6_addr[0]),
                                          *(&data->mc_ip6_addr[4]),
                                          *(&data->mc_ip6_addr[8]),
                                          *(&data->mc_ip6_addr[12]),
                                          *(&data->s_ip6_addr[0]),
                                          *(&data->s_ip6_addr[4]),
                                          *(&data->s_ip6_addr[8]),
                                          *(&data->s_ip6_addr[12]),
                                          &v6mcsg);
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) unable to get LTE\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));
    ftidx += (flag_null_s_addr) ? v6mcg.ftidx : v6mcsg.ftidx;
    sb_ret = soc_sbx_g3p1_ft_get(unit,
                            ftidx,
                            &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) unable to get fte(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), 
                   (flag_null_s_addr) ? v6mcg.ftidx : v6mcsg.ftidx));
        return status;
    }
    
    data->group = fte.oi;
    data->mod_id     = l3_fe->fe_my_modid;

#if 0  
    if (pay4.m_ulSilentRpfCheck == TRUE) {
        data->flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
    } else {
        data->flags &= ~BCM_IPMC_SOURCE_PORT_NOCHECK;
    }
#endif  
    
    data->ts        = 0;
    data->port_tgid = 0;
#if 0 
    if (fte.ulUseRCos) {
        data->cos = fte.ulRCos;
    } else {
        data->cos = 0;
    }
#endif
    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_ipv4mc_find(int                      unit,
                      _caladan3_l3_fe_instance_t  *l3_fe,
                      bcm_ipmc_addr_t         *data)
{
    soc_sbx_g3p1_v4mc_gv_t      v4mcg;
    soc_sbx_g3p1_v4mc_sgv_t     v4mcsg;
    soc_sbx_g3p1_ft_t           fte;
    sbStatus_t                  sb_ret;
    int                         status;
    int                         flag_null_s_addr;
    uint32                      ftidx = ~0;
    soc_sbx_g3p1_pv2e_t         pv2e;
    uint32                      mc_vsi = 0;
    
    flag_null_s_addr = data->s_ip_addr?0:1;
    status  = SOC_SBX_G3P1_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       LOG_ERROR(BSL_LS_BCM_IPMC,
                 (BSL_META_U(unit,
                             "unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n"),
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;


    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g3p1_v4mc_gv_get(unit,
                                          mc_vsi,
                                          (data->mc_ip_addr & 0x0fffffff),
                                          &v4mcg);
    } else {
        sb_ret = soc_sbx_g3p1_v4mc_sgv_get(unit,
                                           mc_vsi,
                                           data->s_ip_addr,
                                           (data->mc_ip_addr & 0x0fffffff),
                                           &v4mcsg);
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) unable to get LTE\n"),
                   unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));
    ftidx += (flag_null_s_addr) ? v4mcg.ftidx : v4mcsg.ftidx;
    sb_ret = soc_sbx_g3p1_ft_get(unit,
                            ftidx,
                            &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        LOG_ERROR(BSL_LS_BCM_IPMC,
                  (BSL_META_U(unit,
                              "unit %d: [%s] error(%s) unable to get fte(0x%x)\n"),
                   unit, FUNCTION_NAME(),
                   bcm_errmsg(status), 
                   (flag_null_s_addr) ? v4mcg.ftidx : v4mcsg.ftidx));
        return status;
    }
    
    data->group = fte.oi;
    data->mod_id     = l3_fe->fe_my_modid;

#if 0  
    if (pay4.m_ulSilentRpfCheck == TRUE) {
        data->flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
    } else {
        data->flags &= ~BCM_IPMC_SOURCE_PORT_NOCHECK;
    }
#endif  
    
    data->ts        = 0;
    data->port_tgid = 0;
#if 0 
    if (fte.ulUseRCos) {
        data->cos = fte.ulRCos;
    } else {
        data->cos = 0;
    }
#endif
    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_ipmc_find(int                      unit,
                      _caladan3_l3_fe_instance_t  *l3_fe,
                      bcm_ipmc_addr_t         *data)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status =  _bcm_caladan3_g3p1_ipv6mc_find(unit, l3_fe, data);
    } else {
        status =  _bcm_caladan3_g3p1_ipv4mc_find(unit, l3_fe, data);
    }
    return status;
}

#endif
