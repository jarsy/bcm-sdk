/*
 * $Id: knet.c,v 1.3 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Kernel Networking Management
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/ethdma.h>
#include <soc/knet.h>

#include <bcm/knet.h>
#include <bcm_int/robo_dispatch.h>
#include <bcm_int/robo/rx.h>

#ifdef INCLUDE_KNET


STATIC int
_robo_rx_reason_get(int unit, bcm_rx_reasons_t *reasons, uint32 *reason)
{
    uint32 temp_reason = 0;
    bcm_rx_reasons_t support_reasons;
    
    *reason = 0;
    bcm_robo_rx_reasons_get(unit,&support_reasons);

    if (SOC_IS_TBX(unit)) {
        BCM_RX_REASON_ITER(*reasons, temp_reason) {
            if (BCM_RX_REASON_GET(support_reasons, temp_reason)){
                switch (temp_reason) {
                    case bcmRxReasonFilterMatch:
                        *reason |= _BCM_ROBO_TB_RX_REASON_CFP_CPU_COPY;
                        break;
                    case bcmRxReasonL2Cpu:
                        *reason |= _BCM_ROBO_TB_RX_REASON_ARL_KNOWN_DA_FORWARD;
                        break;
                    case bcmRxReasonL2DestMiss:
                        *reason |= _BCM_ROBO_TB_RX_REASON_ARL_UNKNOWN_DA_FLOOD;
                        break;
                    case bcmRxReasonControl:
                        *reason |= _BCM_ROBO_TB_RX_REASON_ARL_CONTROL_PKT;
                        break;
                    case bcmRxReasonProtocol:
                        *reason |= _BCM_ROBO_TB_RX_REASON_ARL_APPL_PKT;
                        break;
                    case bcmRxReasonEgressFilterRedirect:
                        *reason |= _BCM_ROBO_TB_RX_REASON_ARL_VLAN_FORWARD;
                        break;
                    case bcmRxReasonFilterRedirect:
                        *reason |= _BCM_ROBO_TB_RX_REASON_ARL_CFP_FORWARD;
                        break;
                    case bcmRxReasonLoopback:
                        *reason |= _BCM_ROBO_TB_RX_REASON_ARL_CPU_LOOPBACK;
                        break;
                    case bcmRxReasonSampleSource:
                        *reason |= _BCM_ROBO_TB_RX_REASON_INGRESS_SFLOW;
                        break;
                    case bcmRxReasonSampleDest:
                        *reason |= _BCM_ROBO_TB_RX_REASON_EGRESS_SFLOW;
                        break;
                    case bcmRxReasonL2Move:
                        *reason |= _BCM_ROBO_TB_RX_REASON_SA_MOVE;
                        break;
                    case bcmRxReasonL2SourceMiss:
                        *reason |= _BCM_ROBO_TB_RX_REASON_SA_UNKNOWN;
                        break;
                    case bcmRxReasonL2LearnLimit:
                        *reason |= _BCM_ROBO_TB_RX_REASON_SA_OVERLIMIT;
                        break;
                    case bcmRxReasonIngressFilter:
                        *reason |= _BCM_ROBO_TB_RX_REASON_VLAN_NONMEMBER;
                        break;
                    case bcmRxReasonUnknownVlan:
                        *reason |= _BCM_ROBO_TB_RX_REASON_VLAN_UNKNOWN;
                        break;
                }
            } else {
                return BCM_E_PARAM;
            }
        }
    }
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)||
        SOC_IS_ROBO_ARCH_FEX(unit)) {

        BCM_RX_REASON_ITER(*reasons, temp_reason) {
            if (BCM_RX_REASON_GET(support_reasons, temp_reason)){
                switch (temp_reason) {
                    case bcmRxReasonCpuLearn:
                        *reason |= BCM_ROBO_RX_REASON_SW_LEARN;
                        break;
                    case bcmRxReasonControl:
                        *reason |= BCM_ROBO_RX_REASON_PROTOCOL_TERMINATION;
                        break;
                    case bcmRxReasonProtocol:
                        *reason |= BCM_ROBO_RX_REASON_PROTOCOL_SNOOP;
                        break;
                    case bcmRxReasonExceptionFlood:
                        *reason |= BCM_ROBO_RX_REASON_EXCEPTION_FLOOD;
                        break;
                }
            } else {
                return BCM_E_PARAM;
            }
        }
    }


    return BCM_E_NONE;;
}


STATIC int
_robo_trav_filter_clean(int unit, bcm_knet_filter_t *filter, void *user_data)
{
    return bcm_robo_knet_filter_destroy(unit, filter->id);
}

#endif /* INCLUDE_KNET */

/*
 * Function:
 *      bcm_robo_knet_init
 * Purpose:
 *      Initialize the kernel networking subsystem.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_init(int unit)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    bcm_knet_filter_t filter;

    rv = bcm_robo_knet_cleanup(unit);

    if (BCM_SUCCESS(rv)) {
        bcm_knet_filter_t_init(&filter);
        filter.type = BCM_KNET_FILTER_T_RX_PKT;
        filter.dest_type = BCM_KNET_DEST_T_BCM_RX_API;
        filter.priority = 255;
        sal_strcpy(filter.desc, "DefaultRxAPI");
        rv = bcm_robo_knet_filter_create(unit, &filter);
    }
    return rv; 
#endif
}

/*
 * Function:
 *      bcm_robo_knet_cleanup
 * Purpose:
 *      Clean up the kernel networking subsystem.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_cleanup(int unit)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;

    rv = bcm_robo_knet_filter_traverse(unit, _robo_trav_filter_clean, NULL);

    return rv; 
#endif
}

/*
 * Function:
 *      bcm_robo_knet_netif_create
 * Purpose:
 *      Create a kernel network interface.
 * Parameters:
 *      unit - (IN) Unit number.
 *      netif - (IN/OUT) Network interface configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_netif_create(int unit, bcm_knet_netif_t *netif)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    kcom_msg_netif_create_t netif_create;
    int eth_unit;

    sal_memset(&netif_create, 0, sizeof(netif_create));
    netif_create.hdr.opcode = KCOM_M_NETIF_CREATE;
    soc_eth_dma_unit_get(unit,&eth_unit);
    netif_create.hdr.unit = eth_unit;
    netif_create.hdr.reserved = SOC_ROBO_CONTROL(unit)->arch_type;


    switch (netif->type) {
    case BCM_KNET_NETIF_T_TX_CPU_INGRESS:
        netif_create.netif.type = KCOM_NETIF_T_VLAN;
        break;
    case BCM_KNET_NETIF_T_TX_LOCAL_PORT:
        netif_create.netif.type = KCOM_NETIF_T_PORT;
        break;
    case BCM_KNET_NETIF_T_TX_META_DATA:
    default:
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported interface type\n")));
        return BCM_E_PARAM;
    }

    if (netif->flags & BCM_KNET_NETIF_F_RCPU_ENCAP) {
        return BCM_E_PARAM;
    }

    if (netif->flags & BCM_KNET_NETIF_F_ADD_TAG) {
        netif_create.netif.flags |= KCOM_NETIF_F_ADD_TAG;
    }

    netif_create.netif.vlan = netif->vlan;
    netif_create.netif.port = netif->port;
    sal_memcpy(netif_create.netif.macaddr, netif->mac_addr, 6);
    sal_memcpy(netif_create.netif.name, netif->name,
               sizeof(netif_create.netif.name) - 1);

    rv = soc_knet_cmd_req((kcom_msg_t *)&netif_create,
                          sizeof(netif_create), sizeof(netif_create));
    if (BCM_SUCCESS(rv)) {
        /* ID and interface name are assigned by kernel */
        netif->id = netif_create.netif.id;
        sal_memcpy(netif->name, netif_create.netif.name,
                   sizeof(netif->name) - 1);
    }
    return rv;
#endif
}

/*
 * Function:
 *      bcm_robo_knet_netif_destroy
 * Purpose:
 *      Destroy a kernel network interface.
 * Parameters:
 *      unit - (IN) Unit number.
 *      netif_id - (IN) Network interface ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_netif_destroy(int unit, int netif_id)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    kcom_msg_netif_destroy_t netif_destroy;
    int eth_unit;

    sal_memset(&netif_destroy, 0, sizeof(netif_destroy));
    netif_destroy.hdr.opcode = KCOM_M_NETIF_DESTROY;
    soc_eth_dma_unit_get(unit,&eth_unit);
    netif_destroy.hdr.unit = eth_unit;

    netif_destroy.hdr.id = netif_id;
    
    return soc_knet_cmd_req((kcom_msg_t *)&netif_destroy,
                            sizeof(netif_destroy), sizeof(netif_destroy));
#endif
}

/*
 * Function:
 *      bcm_robo_knet_netif_get
 * Purpose:
 *      Get a kernel network interface configuration.
 * Parameters:
 *      unit - (IN) Unit number.
 *      netif_id - (IN) Network interface ID
 *      netif - (OUT) Network interface configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_netif_get(int unit, int netif_id, bcm_knet_netif_t *netif)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    kcom_msg_netif_get_t netif_get;
    int eth_unit;
    
    sal_memset(&netif_get, 0, sizeof(netif_get));
    netif_get.hdr.opcode = KCOM_M_NETIF_GET;
    soc_eth_dma_unit_get(unit,&eth_unit);
    netif_get.hdr.unit = eth_unit;

    netif_get.hdr.id = netif_id;

    rv = soc_knet_cmd_req((kcom_msg_t *)&netif_get,
                          sizeof(netif_get.hdr), sizeof(netif_get));

    if (BCM_SUCCESS(rv)) {
        bcm_knet_netif_t_init(netif);

        switch (netif_get.netif.type) {
        case KCOM_NETIF_T_VLAN:
            netif->type = BCM_KNET_NETIF_T_TX_CPU_INGRESS;
            break;
        case KCOM_NETIF_T_PORT:
            netif->type = BCM_KNET_NETIF_T_TX_LOCAL_PORT;
            break;
        case KCOM_NETIF_T_META:
        default:
            /* Unknown type - defaults to VLAN */
            break;
        }

        if (netif_get.netif.flags & KCOM_NETIF_F_ADD_TAG) {
            netif->flags |= BCM_KNET_NETIF_F_ADD_TAG;
        }
        if (netif_get.netif.flags & KCOM_NETIF_F_RCPU_ENCAP) {
            netif->flags |= BCM_KNET_NETIF_F_RCPU_ENCAP;
        }

        netif->id = netif_get.netif.id;
        netif->vlan = netif_get.netif.vlan;
        netif->port = netif_get.netif.port;
        sal_memcpy(netif->mac_addr, netif_get.netif.macaddr, 6);
        sal_memcpy(netif->name, netif_get.netif.name,
                   sizeof(netif->name) - 1);
    }

    return rv;
#endif
}

/*
 * Function:
 *      bcm_robo_knet_netif_traverse
 * Purpose:
 *      Traverse kernel network interface objects
 * Parameters:
 *      unit - (IN) Unit number.
 *      trav_fn - (IN) User provided call back function
 *      user_data - (IN) User provided data used as input param for callback function
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_netif_traverse(int unit, bcm_knet_netif_traverse_cb trav_fn,
                            void *user_data)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv, idx, eth_unit;
    bcm_knet_netif_t netif;
    kcom_msg_netif_list_t netif_list;


    if (trav_fn == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(&netif_list, 0, sizeof(netif_list));
    netif_list.hdr.opcode = KCOM_M_NETIF_LIST;
    soc_eth_dma_unit_get(unit,&eth_unit);
    netif_list.hdr.unit = eth_unit;

    rv = soc_knet_cmd_req((kcom_msg_t *)&netif_list,
                          sizeof(netif_list.hdr), sizeof(netif_list));

    if (BCM_SUCCESS(rv)) {
        for (idx = 0; idx < netif_list.ifcnt; idx++) {
            rv = bcm_robo_knet_netif_get(unit, netif_list.id[idx], &netif);
            if (BCM_SUCCESS(rv)) {
                rv = trav_fn(unit, &netif, user_data);
            }
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
    }

    return rv;
#endif
}

/*
 * Function:
 *      bcm_robo_knet_filter_create
 * Purpose:
 *      Create a kernel packet filter.
 * Parameters:
 *      unit - (IN) Unit number.
 *      filter - (IN/OUT) Rx packet filter configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_filter_create(int unit, bcm_knet_filter_t *filter)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    int idx, pdx;
    int oob_size;
    int data_offset;
    uint32 reason;
    kcom_msg_filter_create_t filter_create;
    int eth_unit;
    uint32 oob_value = 0;

    sal_memset(&filter_create, 0, sizeof(filter_create));
    filter_create.hdr.opcode = KCOM_M_FILTER_CREATE;
    soc_eth_dma_unit_get(unit,&eth_unit);
    filter_create.hdr.unit = eth_unit;

    filter_create.hdr.reserved = SOC_ROBO_CONTROL(unit)->arch_type;


    filter_create.filter.type = KCOM_FILTER_T_RX_PKT;

    switch (filter->dest_type) {
    case BCM_KNET_DEST_T_NULL:
        filter_create.filter.dest_type = KCOM_DEST_T_NULL;
        break;
    case BCM_KNET_DEST_T_NETIF:
        filter_create.filter.dest_type = KCOM_DEST_T_NETIF;
        break;
    case BCM_KNET_DEST_T_BCM_RX_API:
        filter_create.filter.dest_type = KCOM_DEST_T_API;
        break;
    default:
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported destination type\n")));
        return BCM_E_PARAM;
    }

    if (filter->flags & BCM_KNET_FILTER_F_STRIP_TAG) {
        filter_create.filter.flags |= KCOM_FILTER_F_STRIP_TAG;
    }


    filter_create.filter.mirror_type= filter->mirror_type ;

    filter_create.filter.dest_id = filter->dest_id;

    filter_create.filter.priority = filter->priority;
    sal_strncpy(filter_create.filter.desc, filter->desc,
                sizeof(filter_create.filter.desc) - 1);

    oob_size = 0;
    if (filter->match_flags & ~BCM_KNET_FILTER_M_RAW) {
        if (SOC_IS_TBX(unit)) {            
            oob_size = 2 * sizeof(brcm_t);
        }
        if (SOC_IS_ROBO_ARCH_VULCAN(unit)||
            SOC_IS_ROBO_ARCH_FEX(unit)) {            
            oob_size = sizeof(brcm_t);
        }
    }


    /* Create inverted mask */
    for (idx = 0; idx < oob_size; idx++) {
        filter_create.filter.mask.b[idx] = 0xff;
    }

    rv = _robo_rx_reason_get(unit, &filter->m_reason, &reason);
    if (BCM_FAILURE(rv)){
        return rv;
    }

    /* Check if specified reason is supported */
    if (filter->match_flags & BCM_KNET_FILTER_M_REASON) {
        if (reason  == 0) {
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "KNET: Unsupported Rx reason\n")));
            return BCM_E_PARAM;
        }
    }
    if (SOC_IS_TBX(unit)) {
        brcm_t *tag_hi = (brcm_t *)&filter_create.filter.data;
        brcm_t *tag_lo = (brcm_t *)&filter_create.filter.data.w[1];        
        brcm_t *tag_mask_hi = (brcm_t *)&filter_create.filter.mask;
        brcm_t *tag_mask_lo = (brcm_t *)&filter_create.filter.mask.w[1]; 

        if (filter->match_flags & BCM_KNET_FILTER_M_VLAN) {
            tag_hi->brcm_egr_vid_53280 = filter->m_vlan;
            tag_mask_hi->brcm_egr_vid_53280 = 0;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_INGPORT) {
            tag_hi->brcm_pid_53280 = filter->m_ingport;
            tag_mask_hi->brcm_pid_53280 = 0;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_REASON) {
            tag_lo->brcm_reason_53280 = reason;
            tag_mask_lo->brcm_reason_53280 = ~reason;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_FP_RULE) {
            return BCM_E_PARAM;
        }
    }
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
        brcm_t *tag_hi = (brcm_t *)&filter_create.filter.data;
        brcm_t *tag_mask_hi = (brcm_t *)&filter_create.filter.mask;

        if (filter->match_flags & BCM_KNET_FILTER_M_INGPORT) {
            tag_hi->brcm_53115_src_portid = filter->m_ingport;
            tag_mask_hi->brcm_53115_src_portid = 0;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_REASON) {
            tag_hi->brcm_53115_reason = reason;
            tag_mask_hi->brcm_53115_reason = ~reason;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_VLAN) {
            return BCM_E_PARAM;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_FP_RULE) {
            return BCM_E_PARAM;
        }
    }

    if (SOC_IS_ROBO_ARCH_FEX(unit)){
        brcm_t *tag_hi = (brcm_t *)&filter_create.filter.data;
        brcm_t *tag_mask_hi = (brcm_t *)&filter_create.filter.mask;
    
        if (filter->match_flags & BCM_KNET_FILTER_M_INGPORT) {
            tag_hi->brcm_53242_src_portid = filter->m_ingport + 24;
            tag_mask_hi->brcm_53242_src_portid = 0;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_REASON) {
            tag_hi->brcm_53242_reason = reason;
            tag_mask_hi->brcm_53242_reason = ~reason;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_VLAN) {
            return BCM_E_PARAM;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_FP_RULE) {
            return BCM_E_PARAM;
        }

    }


    /* Invert inverted mask */
    for (idx = 0; idx < oob_size; idx++) {
        filter_create.filter.mask.b[idx] ^= 0xff;
    }

    /* if broadcom tag exists, convert host type to network type */
    oob_value = 0;
    if (oob_size) {
        for (idx = 0; idx < BYTES2WORDS(oob_size); idx++) {
            oob_value = filter_create.filter.data.w[idx];
                filter_create.filter.data.w[idx] = bcm_htonl(oob_value);
            oob_value = filter_create.filter.mask.w[idx];
                filter_create.filter.mask.w[idx] = bcm_htonl(oob_value);
        }
    }

    filter_create.filter.oob_data_size = oob_size;

    if (filter->match_flags & BCM_KNET_FILTER_M_RAW) {
        data_offset = 0;
        for (pdx = 0; pdx < filter->raw_size; pdx++) {
            if (filter->m_raw_mask[pdx] != 0) {
                data_offset = pdx;
                break;
            }
        }
        idx = oob_size;
        for (; pdx < filter->raw_size; pdx++) {
            /* Check for array overflow */
            if (idx >= KCOM_FILTER_BYTES_MAX) {
                return BCM_E_PARAM;
            }
            filter_create.filter.data.b[idx] = filter->m_raw_data[pdx];
            filter_create.filter.mask.b[idx] = filter->m_raw_mask[pdx];
            idx++;
        }
        filter_create.filter.pkt_data_offset = data_offset;
        filter_create.filter.pkt_data_size = filter->raw_size - data_offset;
    }

    /*
     * If no match flags are set we treat raw filter data as OOB data.
     * Note that this functionality is intended for debugging only.
     */
    if (filter->match_flags == 0) {
        for (idx = 0; idx < filter->raw_size; idx++) {
            /* Check for array overflow */
            if (idx >= KCOM_FILTER_BYTES_MAX) {
                return BCM_E_PARAM;
            }
            filter_create.filter.data.b[idx] = filter->m_raw_data[idx];
            filter_create.filter.mask.b[idx] = filter->m_raw_mask[idx];
        }
        filter_create.filter.oob_data_size = oob_size;
    }

    /* Dump raw data for debugging purposes */
    for (idx = 0; idx < BYTES2WORDS(oob_size); idx++) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "OOB[%d]: 0x%08x [0x%08x]\n"), idx,
                     filter_create.filter.data.w[idx],
                     filter_create.filter.mask.w[idx]));
    }
    for (idx = 0; idx < filter_create.filter.pkt_data_size; idx++) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "PKT[%d]: 0x%02x [0x%02x]\n"),
                     idx + filter_create.filter.pkt_data_offset,
                     filter_create.filter.data.b[idx + oob_size],
                     filter_create.filter.mask.b[idx + oob_size]));
    }

    rv = soc_knet_cmd_req((kcom_msg_t *)&filter_create,
                          sizeof(filter_create), sizeof(filter_create));

    if (BCM_SUCCESS(rv)) {
        /* ID is assigned by kernel */
        filter->id = filter_create.filter.id;
    }
    return rv;
#endif
}

/*
 * Function:
 *      bcm_robo_knet_filter_destroy
 * Purpose:
 *      Destroy a kernel packet filter.
 * Parameters:
 *      unit - (IN) Unit number.
 *      filter_id - (IN) Rx packet filter ID
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_filter_destroy(int unit, int filter_id)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    kcom_msg_filter_destroy_t filter_destroy;
    int eth_unit;

    sal_memset(&filter_destroy, 0, sizeof(filter_destroy));
    filter_destroy.hdr.opcode = KCOM_M_FILTER_DESTROY;
    soc_eth_dma_unit_get(unit,&eth_unit);
    filter_destroy.hdr.unit = eth_unit;

    filter_destroy.hdr.id = filter_id;

    return soc_knet_cmd_req((kcom_msg_t *)&filter_destroy,
                            sizeof(filter_destroy), sizeof(filter_destroy));
#endif
}

/*
 * Function:
 *      bcm_robo_knet_filter_get
 * Purpose:
 *      Get a kernel packet filter configuration.
 * Parameters:
 *      unit - (IN) Unit number.
 *      filter_id - (IN) Rx packet filter ID
 *      filter - (OUT) Rx packet filter configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_filter_get(int unit, int filter_id, bcm_knet_filter_t *filter)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    kcom_msg_filter_get_t filter_get;
    int idx, rdx, fdx, eth_unit;
    int oob_size = 0;

    sal_memset(&filter_get, 0, sizeof(filter_get));
    filter_get.hdr.opcode = KCOM_M_FILTER_GET;
    soc_eth_dma_unit_get(unit,&eth_unit);
    filter_get.hdr.unit = eth_unit;

    filter_get.hdr.id = filter_id;

    rv = soc_knet_cmd_req((kcom_msg_t *)&filter_get,
                          sizeof(filter_get.hdr), sizeof(filter_get));

    if (BCM_SUCCESS(rv)) {
        bcm_knet_filter_t_init(filter);

        switch (filter_get.filter.type) {
        case KCOM_FILTER_T_RX_PKT:
            filter->type = BCM_KNET_DEST_T_BCM_RX_API;
            break;
        default:
            /* Unknown type */
            break;
        }

        switch (filter_get.filter.dest_type) {
        case KCOM_DEST_T_NETIF:
            filter->dest_type = BCM_KNET_DEST_T_NETIF;
            break;
        case KCOM_DEST_T_API:
            filter->dest_type = BCM_KNET_DEST_T_BCM_RX_API;
            break;
        default:
            filter->dest_type = BCM_KNET_DEST_T_NULL;
            break;
        }

        if (filter_get.filter.flags & KCOM_FILTER_F_STRIP_TAG) {
            filter->flags |= BCM_KNET_FILTER_F_STRIP_TAG;
        }

        filter->dest_id = filter_get.filter.dest_id;

        filter->id = filter_get.filter.id;
        filter->priority = filter_get.filter.priority;
        sal_memcpy(filter->desc, filter_get.filter.desc,
                   sizeof(filter->desc) - 1);

        if (filter_get.filter.pkt_data_size) {
            filter->match_flags |= BCM_KNET_FILTER_M_RAW;
            rdx = filter_get.filter.pkt_data_offset;
            fdx = filter_get.filter.oob_data_size;
            for (idx = 0; idx < filter_get.filter.pkt_data_size; idx++) {
                filter->m_raw_data[rdx] = filter_get.filter.data.b[fdx];
                filter->m_raw_mask[rdx] = filter_get.filter.mask.b[fdx];
                rdx++;
                fdx++;
            }
            filter->raw_size = rdx;
        } else {
            /*
             * If a filter contains no raw packet data then we copy the OOB
             * data into the raw data buffer while raw_size remains zero.
             * Note that this functionality is intended for debugging only.
             */
            if (SOC_IS_TBX(unit)) { 		   
                oob_size = 2 * sizeof(brcm_t);
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)||
                SOC_IS_ROBO_ARCH_FEX(unit)) {			 
                oob_size = sizeof(brcm_t);
            }

            for (idx = 0; idx < oob_size; idx++) {
                filter->m_raw_data[idx] = filter_get.filter.data.b[idx];
                filter->m_raw_mask[idx] = filter_get.filter.mask.b[idx];
            }
        }
    }

    return rv;
#endif
}

/*
 * Function:
 *      bcm_robo_knet_filter_traverse
 * Purpose:
 *      Traverse kernel packet filter objects
 * Parameters:
 *      unit - (IN) Unit number.
 *      trav_fn - (IN) User provided call back function
 *      user_data - (IN) User provided data used as input param for callback function
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_knet_filter_traverse(int unit, bcm_knet_filter_traverse_cb trav_fn, 
                             void *user_data)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv, idx;
    bcm_knet_filter_t filter;
    kcom_msg_filter_list_t filter_list;
    int eth_unit;

    if (trav_fn == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(&filter_list, 0, sizeof(filter_list));
    filter_list.hdr.opcode = KCOM_M_FILTER_LIST;
    soc_eth_dma_unit_get(unit,&eth_unit);
    filter_list.hdr.unit = eth_unit;

    rv = soc_knet_cmd_req((kcom_msg_t *)&filter_list,
                          sizeof(filter_list.hdr), sizeof(filter_list));

    if (BCM_SUCCESS(rv)) {
        for (idx = 0; idx < filter_list.fcnt; idx++) {
            rv = bcm_robo_knet_filter_get(unit, filter_list.id[idx], &filter);

            if (BCM_SUCCESS(rv)) {
                rv = trav_fn(unit, &filter, user_data);
            }
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
    }

    return rv;
#endif
}
