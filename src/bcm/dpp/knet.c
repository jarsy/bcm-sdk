/*
 * $Id: knet.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        knet.c
 * Purpose:     Kernel Networking Management 
 *
 */

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/dcbformats.h>
#include <soc/knet.h>

#include <bcm/error.h>
#include <bcm/knet.h>
#include <bcm_int/petra_dispatch.h>
#include <bcm_int/common/rx.h>
#include <bcm_int/common/debug.h>

#include <shared/bsl.h>

#ifdef INCLUDE_KNET

/* Dune RX DMA Descriptor */
typedef struct {
        uint32  addr;                   /* T9.0: physical address */
#ifdef LE_HOST
        uint32  c_count:16,             /* Requested byte count */
                c_chain:1,              /* Chaining */
                c_sg:1,                 /* Scatter Gather */
                c_reload:1,             /* Reload */
                :13;                    /* Don't care */
#else
        uint32  :13,                    /* Don't care */
                c_reload:1,             /* Reload */
                c_sg:1,                 /* Scatter Gather */
                c_chain:1,              /* Chaining */
                c_count:16;             /* Requested byte count */
#endif
        uint32  dune_fp_rule;           /* bit 0-15 trap qualifier, bit 16-31 trap id */
        uint32  rsv1[1];                /* DW 3 */

#ifdef LE_HOST
        uint32  cpu_cos:6,              /* CPU COS, Egress Queue Number % 64 */
                :26;                    /* Reserved */
#else
        uint32  :26,                    /* Reserved */
                cpu_cos:6;              /* CPU COS, Egress Queue Number % 64 */
#endif

#ifdef LE_HOST
        uint32  ingress_port:8,         /* Ingress Port */
                :24;                    /* Reserved */
#else
        uint32  :24,                    /* Reserved */
                ingress_port:8;         /* Ingress Port */
#endif

        uint32  reason_hi;
#ifdef LE_HOST
        uint32  reason:25,              /* CPU opcode */
                :7;                     /* Reserved */
        uint32  :2,                     /* Reserved */
                srcport:5,              /* Source port */
                nh_index:13,            /* Next hop index */
                match_rule:10,          /* Matched Rule */
                match_rule_valid:1,     /* Matched Rule valid */
                decap_iptunnel:1;       /* Reserved */


#else
        uint32  :7,                     /* Reserved */
                reason:25;              /* CPU opcode */

        uint32  decap_iptunnel:1,       /* Reserved */
                match_rule_valid:1,     /* Matched Rule valid */
                match_rule:10,          /* Matched Rule */
                nh_index:13,            /* Next hop index */
                srcport:5,              /* Source port */
                :2;                     /* Reserved */

#endif

#ifdef LE_HOST
        uint32  :11,
                ingress_untagged:1,     /* Pkt came in untagged */
                outer_vid:12,           /* VID */
                outer_cfi:1,            /* CFI */
                outer_pri:3,            /* Priority */
                dscp_lo:4;              /* New DSCP */
#else
      uint32    dscp_lo:4,              /* New DSCP */
                outer_pri:3,            /* Priority (D)*/
                outer_cfi:1,            /* CFI (D)*/
                outer_vid:12,           /* VID (D)*/
                ingress_untagged:1,     /* Pkt came in untagged (D)*/
                :11;
#endif

        uint32  rsv4[5];          		/* DW 10-14 */

#ifdef  LE_HOST
        uint32  count:16,               /* Transferred byte count */
                end:1,                  /* End bit (RX) */
                start:1,                /* Start bit (RX) */
                error:1,                /* Cell Error (RX) */
                :12,                    /* Don't Care */
                done:1;                 /* Descriptor Done */
#else
        uint32  done:1,                 /* Descriptor Done */
                :12,                    /* Don't Care */
                error:1,                /* Cell Error (RX) */
                start:1,                /* Start bit (RX) */
                end:1,                  /* End bit (RX) */
                count:16;               /* Transferred byte count */
#endif
}dcb_rx_t;


STATIC int
_petra_trav_filter_clean(int unit, bcm_knet_filter_t *filter, void *user_data)
{
    return bcm_petra_knet_filter_destroy(unit, filter->id);
}

STATIC uint32
_dune_rx_reason_get(int unit, soc_rx_reasons_t *reasons)
{
    soc_rx_reason_t *map;
    uint32 reason = 0;
    uint32 mask = 1;
    int i;

    if ((map = (SOC_DCB_RX_REASON_MAPS(unit))[0]) == NULL) {
        return reason;
    }

    for (i = 0; i < 32; i++) {
        if (SOC_RX_REASON_GET(*reasons, map[i])) {
            reason |= mask;
        }
        mask <<= 1;
    }
    return reason;
}

STATIC uint32
_dune_rx_reason_hi_get(int unit, soc_rx_reasons_t *reasons)
{
    soc_rx_reason_t *map;
    uint32 reason = 0;
    uint32 mask = 1;
    int i;

    if ((map = (SOC_DCB_RX_REASON_MAPS(unit))[0]) == NULL) {
        return reason;
    }

    for (i = 32; i < 64; i++) {
        if (SOC_RX_REASON_GET(*reasons, map[i])) {
            reason |= mask;
        }
        mask <<= 1;
    }
    return reason;
}

static void
_dcb_rx_reasons_get(int unit, dcb_rx_t *dcb, soc_rx_reasons_t *reasons)
{
    soc_rx_reason_t *map;
    uint32 reason;
    uint32 mask;
    int i;

    SOC_RX_REASON_CLEAR_ALL(*reasons);

    if ((map = (SOC_DCB_RX_REASON_MAPS(unit))[0]) == NULL) {
        return;
    }

    reason = dcb->reason;
    mask = 1;
    for (i = 0; i < 32; i++) {
        if ((mask & reason)) {
            SOC_RX_REASON_SET(*reasons, map[i]);
        }
        mask <<= 1;
    }

    reason = dcb->reason_hi;
    mask = 1;
    for (i = 0; i < 32; i++) {
        if ((mask & reason)) {
            SOC_RX_REASON_SET(*reasons, map[i + 32]);
        }
        mask <<= 1;
    }

    return;
}

#endif /* INCLUDE_KNET */

/*
 * Function:
 *     bcm_petra_knet_init
 * Purpose:
 *     Initialize the kernel networking subsystem.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int 
bcm_petra_knet_init(int unit)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    bcm_knet_filter_t filter;

    rv = bcm_petra_knet_cleanup(unit);

    if (BCM_SUCCESS(rv)) {
        bcm_knet_filter_t_init(&filter);
        filter.type = BCM_KNET_FILTER_T_RX_PKT;
        filter.dest_type = BCM_KNET_DEST_T_BCM_RX_API;
        filter.priority = 255;
        sal_strcpy(filter.desc, "DefaultRxAPI");
        rv = bcm_petra_knet_filter_create(unit, &filter);
    }
    return rv; 
#endif
}

/*
 * Function:
 *      bcm_petra_knet_cleanup
 * Purpose:
 *      Clean up the kernel networking subsystem.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_petra_knet_cleanup(int unit)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;

    rv = bcm_petra_knet_filter_traverse(unit, _petra_trav_filter_clean, NULL);

    return rv; 
#endif
}

/*
 * Function:
 *      bcm_petra_knet_netif_create
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
bcm_petra_knet_netif_create(int unit, bcm_knet_netif_t *netif)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    kcom_msg_netif_create_t netif_create;

    sal_memset(&netif_create, 0, sizeof(netif_create));
    netif_create.hdr.opcode = KCOM_M_NETIF_CREATE;
    netif_create.hdr.unit = unit;

    switch (netif->type) {
    case BCM_KNET_NETIF_T_TX_CPU_INGRESS:
        netif_create.netif.type = KCOM_NETIF_T_VLAN;
        break;
    case BCM_KNET_NETIF_T_TX_LOCAL_PORT:
        netif_create.netif.type = KCOM_NETIF_T_PORT;
        break;
    case BCM_KNET_NETIF_T_TX_META_DATA:
        netif_create.netif.type = KCOM_NETIF_T_META;
        break;
    default:
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported interface type\n")));
        return BCM_E_PARAM;
    }

    if (netif->flags & BCM_KNET_NETIF_F_ADD_TAG) {
        netif_create.netif.flags |= KCOM_NETIF_F_ADD_TAG;
    }
    if (netif->flags & BCM_KNET_NETIF_F_RCPU_ENCAP) {
        netif_create.netif.flags |= KCOM_NETIF_F_RCPU_ENCAP;
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
 *      bcm_petra_knet_netif_destroy
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
bcm_petra_knet_netif_destroy(int unit, int netif_id)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    kcom_msg_netif_destroy_t netif_destroy;

    sal_memset(&netif_destroy, 0, sizeof(netif_destroy));
    netif_destroy.hdr.opcode = KCOM_M_NETIF_DESTROY;
    netif_destroy.hdr.unit = unit;

    netif_destroy.hdr.id = netif_id;
    
    return soc_knet_cmd_req((kcom_msg_t *)&netif_destroy,
                            sizeof(netif_destroy), sizeof(netif_destroy));
#endif
}

/*
 * Function:
 *      bcm_petra_knet_netif_get
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
bcm_petra_knet_netif_get(int unit, int netif_id, bcm_knet_netif_t *netif)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    kcom_msg_netif_get_t netif_get;

    sal_memset(&netif_get, 0, sizeof(netif_get));
    netif_get.hdr.opcode = KCOM_M_NETIF_GET;
    netif_get.hdr.unit = unit;

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
            netif->type = BCM_KNET_NETIF_T_TX_META_DATA;
            break;
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
 *      bcm_petra_knet_netif_traverse
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
bcm_petra_knet_netif_traverse(int unit, bcm_knet_netif_traverse_cb trav_fn,
                            void *user_data)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv, idx;
    bcm_knet_netif_t netif;
    kcom_msg_netif_list_t netif_list;

    if (trav_fn == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(&netif_list, 0, sizeof(netif_list));
    netif_list.hdr.opcode = KCOM_M_NETIF_LIST;
    netif_list.hdr.unit = unit;

    rv = soc_knet_cmd_req((kcom_msg_t *)&netif_list,
                          sizeof(netif_list.hdr), sizeof(netif_list));

    if (BCM_SUCCESS(rv)) {
        for (idx = 0; idx < netif_list.ifcnt; idx++) {
            rv = bcm_petra_knet_netif_get(unit, netif_list.id[idx], &netif);
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
 *      bcm_petra_knet_filter_create
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
bcm_petra_knet_filter_create(int unit, bcm_knet_filter_t *filter)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    int idx, pdx;
    int oob_size;
    int data_offset;
    uint32 reason;
    uint32 reason_hi;
    kcom_msg_filter_create_t filter_create;

    sal_memset(&filter_create, 0, sizeof(filter_create));
    filter_create.hdr.opcode = KCOM_M_FILTER_CREATE;
    filter_create.hdr.unit = unit;

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
    case BCM_KNET_DEST_T_CALLBACK:
        filter_create.filter.dest_type = KCOM_DEST_T_CB;
        break;
    default:
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported destination type\n")));
        return BCM_E_PARAM;
    }

    switch (filter->mirror_type) {
    case BCM_KNET_DEST_T_NULL:
        filter_create.filter.mirror_type = KCOM_DEST_T_NULL;
        break;
    case BCM_KNET_DEST_T_NETIF:
        filter_create.filter.mirror_type = KCOM_DEST_T_NETIF;
        break;
    case BCM_KNET_DEST_T_BCM_RX_API:
        filter_create.filter.mirror_type = KCOM_DEST_T_API;
        break;
    case BCM_KNET_DEST_T_CALLBACK:
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Cannot mirror to callback\n")));
        return BCM_E_PARAM;
    default:
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported mirror type\n")));
        return BCM_E_PARAM;
    }

    if (filter->flags & BCM_KNET_FILTER_F_STRIP_TAG) {
        filter_create.filter.flags |= KCOM_FILTER_F_STRIP_TAG;
    }

    filter_create.filter.dest_id = filter->dest_id;
    filter_create.filter.dest_proto = filter->dest_proto;
    filter_create.filter.mirror_id = filter->mirror_id;
    filter_create.filter.mirror_proto = filter->mirror_proto;

    filter_create.filter.priority = filter->priority;
    sal_strncpy(filter_create.filter.desc, filter->desc,
                sizeof(filter_create.filter.desc) - 1);

    oob_size = 0;
    if (filter->match_flags & ~BCM_KNET_FILTER_M_RAW) {
        oob_size = SOC_DCB_SIZE(unit);
    }

    /* Create inverted mask */
    for (idx = 0; idx < oob_size; idx++) {
        filter_create.filter.mask.b[idx] = 0xff;
    }
    reason = reason_hi = 0;
    reason = _dune_rx_reason_get(unit, &filter->m_reason);
    reason_hi = _dune_rx_reason_hi_get(unit, &filter->m_reason);

    /* Check if specified reason is supported */
    if (filter->match_flags & BCM_KNET_FILTER_M_REASON) {
        /* bcmRxReasonMirror to match mirrored packets and bcmRxReasonSampleSource to match snooped packets */
        /* only bcmRxReasonMirror and bcmRxReasonSampleSource are supported, and they are impossible to take affect at the same time */
        if ((reason != 0x20) && (reason_hi != 0x8)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "KNET: Unsupported Rx reason %x reason_hi %x\n"), reason, reason_hi));
            return BCM_E_PARAM;
        }
        if ((reason == 0x20) && (reason_hi == 0x8)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "KNET: Unsupported Rx reason Mirror and Snoop at the same time\n")));
            return BCM_E_PARAM;
        }
    }
    if (filter->match_flags & BCM_KNET_FILTER_M_SRC_MODPORT) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported filter to match source module port\n")));
        return BCM_E_PARAM;
    }
    if (filter->match_flags & BCM_KNET_FILTER_M_SRC_MODID) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported filter to match source module id\n")));
        return BCM_E_PARAM;
    }
    if (filter->match_flags & BCM_KNET_FILTER_M_ERROR) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "KNET: Unsupported filter to match error bit\n")));
        return BCM_E_PARAM;
    }

    if (SOC_DCB_TYPE(unit) == 28) {
        dcb_rx_t *dcb_data = (dcb_rx_t *)&filter_create.filter.data;
        dcb_rx_t *dcb_mask = (dcb_rx_t *)&filter_create.filter.mask;

        if (filter->match_flags & BCM_KNET_FILTER_M_VLAN) {
            dcb_data->outer_vid = filter->m_vlan;
            dcb_mask->outer_vid = 0;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_INGPORT) {
            dcb_data->srcport = filter->m_ingport;
            dcb_mask->srcport = 0;
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_REASON) {
            if (reason){
                dcb_data->reason = reason;
                dcb_mask->reason = 0;
            }
            if (reason_hi){
                dcb_data->reason_hi = reason_hi;
                dcb_mask->reason_hi = 0;
            }
        }
        if (filter->match_flags & BCM_KNET_FILTER_M_FP_RULE) {
            dcb_data->dune_fp_rule = filter->m_fp_rule;
            dcb_mask->dune_fp_rule = 0;
        }
    }

    /* Invert inverted mask */
    for (idx = 0; idx < oob_size; idx++) {
        filter_create.filter.mask.b[idx] ^= 0xff;
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
        filter_create.filter.oob_data_size = SOC_DCB_SIZE(unit);
    }

    /* Dump raw data for debugging purposes */
    for (idx = 0; idx < BYTES2WORDS(oob_size); idx++) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "OOB[%d]: 0x%08x [0x%08x]\n"), idx,
                     filter_create.filter.data.w[idx],
                     filter_create.filter.mask.w[idx]));
    }
    for (idx = 0; idx < filter_create.filter.pkt_data_size; idx++) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
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
 *      bcm_petra_knet_filter_destroy
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
bcm_petra_knet_filter_destroy(int unit, int filter_id)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    kcom_msg_filter_destroy_t filter_destroy;

    sal_memset(&filter_destroy, 0, sizeof(filter_destroy));
    filter_destroy.hdr.opcode = KCOM_M_FILTER_DESTROY;
    filter_destroy.hdr.unit = unit;

    filter_destroy.hdr.id = filter_id;

    return soc_knet_cmd_req((kcom_msg_t *)&filter_destroy,
                            sizeof(filter_destroy), sizeof(filter_destroy));
#endif
}

/*
 * Function:
 *      bcm_petra_knet_filter_get
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
bcm_petra_knet_filter_get(int unit, int filter_id, bcm_knet_filter_t *filter)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv;
    kcom_msg_filter_get_t filter_get;
    dcb_rx_t *dcb_data;
    dcb_rx_t *dcb_mask;
    int idx, rdx, fdx;

    sal_memset(&filter_get, 0, sizeof(filter_get));
    filter_get.hdr.opcode = KCOM_M_FILTER_GET;
    filter_get.hdr.unit = unit;

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
        case KCOM_DEST_T_CB:
            filter->dest_type = BCM_KNET_DEST_T_CALLBACK;
            break;
        default:
            filter->dest_type = BCM_KNET_DEST_T_NULL;
            break;
        }

        switch (filter_get.filter.mirror_type) {
        case KCOM_DEST_T_NETIF:
            filter->mirror_type = BCM_KNET_DEST_T_NETIF;
            break;
        case KCOM_DEST_T_API:
            filter->mirror_type = BCM_KNET_DEST_T_BCM_RX_API;
            break;
        case KCOM_DEST_T_CB:
            /* Should never get here, but keep for completeness */
            filter->mirror_type = BCM_KNET_DEST_T_CALLBACK;
            break;
        default:
            filter->mirror_type = BCM_KNET_DEST_T_NULL;
            break;
        }

        if (filter_get.filter.flags & KCOM_FILTER_F_STRIP_TAG) {
            filter->flags |= BCM_KNET_FILTER_F_STRIP_TAG;
        }

        filter->dest_id = filter_get.filter.dest_id;
        filter->dest_proto = filter_get.filter.dest_proto;
        filter->mirror_id = filter_get.filter.mirror_id;
        filter->mirror_proto = filter_get.filter.mirror_proto;

        filter->id = filter_get.filter.id;
        filter->priority = filter_get.filter.priority;
        sal_memcpy(filter->desc, filter_get.filter.desc,
                   sizeof(filter->desc) - 1);

        dcb_data = (dcb_rx_t *)&filter_get.filter.data;
        dcb_mask = (dcb_rx_t *)&filter_get.filter.mask;

        if ((dcb_mask->reason == 0x1ffffff) || (dcb_mask->reason_hi ==0xffffffff)) {
            filter->match_flags |= BCM_KNET_FILTER_M_REASON;
            _dcb_rx_reasons_get(unit, dcb_data, &filter->m_reason);
        }
        if (dcb_mask->outer_vid == 0xfff) {
            filter->match_flags |= BCM_KNET_FILTER_M_VLAN;
            filter->m_vlan = dcb_data->outer_vid;
        }
        if (dcb_mask->srcport == 0x1f) {
            filter->match_flags |= BCM_KNET_FILTER_M_INGPORT;
            filter->m_ingport = dcb_data->srcport;
        }
        if (dcb_mask->dune_fp_rule == 0xffffffff) {
            filter->match_flags |= BCM_KNET_FILTER_M_FP_RULE;
            filter->m_fp_rule = dcb_data->dune_fp_rule;
        }
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
            for (idx = 0; idx < SOC_DCB_SIZE(unit); idx++) {
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
 *      bcm_petra_knet_filter_traverse
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
bcm_petra_knet_filter_traverse(int unit, bcm_knet_filter_traverse_cb trav_fn, 
                             void *user_data)
{
#ifndef INCLUDE_KNET
    return BCM_E_UNAVAIL;
#else
    int rv, idx;
    bcm_knet_filter_t filter;
    kcom_msg_filter_list_t filter_list;

    if (trav_fn == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(&filter_list, 0, sizeof(filter_list));
    filter_list.hdr.opcode = KCOM_M_FILTER_LIST;
    filter_list.hdr.unit = unit;

    rv = soc_knet_cmd_req((kcom_msg_t *)&filter_list,
                          sizeof(filter_list.hdr), sizeof(filter_list));

    if (BCM_SUCCESS(rv)) {
        for (idx = 0; idx < filter_list.fcnt; idx++) {
            rv = bcm_petra_knet_filter_get(unit, filter_list.id[idx], &filter);
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
