/*
 * $Id: dmaTx.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include "salIntf.h"
#include "dmaOps.h"
#include "assert.h"

#define ENET_MAC_SIZE      6
#define ENET_FCS_SIZE      4
#define ENET_MIN_PKT_SIZE  64

#define _ON_ERROR_GOTO(act, rv, label) if (((rv) = (act)) < 0) goto label

#define _PROCESS_ERROR(unit, rv, dv, err_msg) \
    do { \
        if ((rv) < 0) { /* Error detected */ \
            if (dv != NULL) { \
                _tx_dv_free(unit, dv); \
            } \
            if (err_msg) { \
                PRINTF_ERROR(("bcm_tx: %s\n", err_msg)); \
            } \
        } \
    } while (0)

/* This is all the extra information associated with a DV for RX */
typedef struct tx_dv_info_s {
    volatile bcm_pkt_t *pkt[SOC_DV_PKTS_MAX];
    int                 pkt_count;
    volatile uint8      pkt_done_cnt;
    bcm_pkt_cb_f        chain_done_cb;
    void               *cookie;
} tx_dv_info_t;

#define SOC_DV_PKTS_MAX  16

#define TX_DV_NEXT(dv) ((dv_t *)((dv)->dv_public2.ptr))
#define TX_DV_NEXT_SET(dv, dv_next) \
    ((dv)->dv_public2.ptr = (void *)(dv_next))

#define TX_LOCK(_s)   _s = sal_splhi()
#define TX_UNLOCK(_s) sal_spl(_s)

void 
_tx_dv_free(int unit, dv_t *dv) 
{
    logMsg("dv_free!\n", 1, 2, 3, 4, 5, 6); /* DEL_ME */

    if (dv) {
        void *p = ((dcb12_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, 0))->addr;
        if (p != NULL) {
            cacheDmaFree(p);
        }
        soc_dma_dv_free(unit, dv);
    }
}


static dv_t *
_tx_dv_alloc(int unit, int pkt_count, int dcb_count,
             bcm_pkt_cb_f chain_done_cb, void *cookie, int per_pkt_cb) 
{
    dv_t *dv;
    /* tx_dv_info_t *dv_info; */

    if (pkt_count > SOC_DV_PKTS_MAX) {
        PRINTF_DEBUG(("TX array:  Cannot TX more than %d pkts. "
            "Attempting %d.\n", SOC_DV_PKTS_MAX, pkt_count));
        return NULL;
    }

    if ((dv = soc_dma_dv_alloc(unit, DV_TX, dcb_count)) == NULL) {
        return NULL;
    }

    /* PRINTF_DEBUG(("=> %p %p\n", dv, dv_info)); */

    dv->dv_done_chain = _tx_dv_free;

    return dv;
}


#define BCM_PKT_NO_VLAN_TAG(pkt) (((pkt)->flags & BCM_PKT_F_NO_VTAG) != 0)

static void
_get_mac_vlan_ptrs(dv_t *dv, bcm_pkt_t *pkt, uint8 **src_mac,
                   uint8 **vlan_ptr, int *block_offset,
                   int *byte_offset, int pkt_idx) 
{
    /* Assume everything is in block 0 */
    *src_mac = &pkt->pkt_data[0].data[sizeof(bcm_mac_t)];
    *block_offset = 0;

    if (BCM_PKT_NO_VLAN_TAG(pkt)) { /* Get VLAN from _vtag pkt member */
        *byte_offset = 2 * sizeof(bcm_mac_t);
        sal_memcpy(SOC_DV_VLAN_TAG(dv, pkt_idx), pkt->_vtag, sizeof(uint32));
        *vlan_ptr = SOC_DV_VLAN_TAG(dv, pkt_idx);

        if (pkt->pkt_data[0].len < 2 * sizeof(bcm_mac_t)) {
            /* Src MAC in block 1 */
            *src_mac = pkt->pkt_data[1].data;
            *block_offset = 1;
            *byte_offset = sizeof(bcm_mac_t);
        }
    } else { /* Packet has VLAN tag */
        *byte_offset = 2 * sizeof(bcm_mac_t) + sizeof(uint32);
        *vlan_ptr = &pkt->pkt_data[0].data[2 * sizeof(bcm_mac_t)];

        if (pkt->pkt_data[0].len < 2 * sizeof(bcm_mac_t)) {
            /* Src MAC in block 1; assume VLAN there too at first */
            *src_mac = pkt->pkt_data[1].data;
            *vlan_ptr = &pkt->pkt_data[1].data[sizeof(bcm_mac_t)];
            *block_offset = 1;
            *byte_offset = sizeof(bcm_mac_t) + sizeof(uint32);
            if (pkt->pkt_data[1].len < sizeof(bcm_mac_t) + sizeof(uint32)) {
                /* Oops, VLAN in block 2 */
                *vlan_ptr = pkt->pkt_data[2].data;
                *block_offset = 2;
                *byte_offset = sizeof(uint32);
            }
        } else if (pkt->pkt_data[0].len < 2 * sizeof(bcm_mac_t) + sizeof(uint32)) {
            /* VLAN in block 2 */
            *block_offset = 1;
            *byte_offset = sizeof(uint32);
            *vlan_ptr = pkt->pkt_data[1].data;
        }
    }
}


static int
_tx_pkt_desc_add(int unit, bcm_pkt_t *pkt, dv_t *dv, int pkt_idx) 
{
    int byte_offset = 0;
    int pkt_len = 0; /* Length calculation for byte padding */
    int tmp_len;
    int min_len = ENET_MIN_PKT_SIZE; /* Min length pkt before padded */
    uint8 *vlan_ptr;
    uint8 *src_mac;
    int block_offset = 0;
    int i;
    uint32 dcb_flags = 0;
    uint32 src_port, src_mod;
    uint32 prio_int;
    uint32 *pkt_hg_hdr;
    int clear_crc = FALSE;
    int crc_block_offset = 0;

    COMPILER_REFERENCE(pkt_idx); /* May not be ref'd depending on defines */

    /** TX_ETHER_MODE no flag to set at all */
    /* Calculate the DCB flags for this packet. */
    /* dcb_flags = _dcb_flags_get(unit, pkt, dv); */

    /* This is still used for 5695. */
    /* Decide which source mod/port to use */
    src_mod  = pkt->src_mod;
    src_port = pkt->src_port;
    prio_int = pkt->prio_int;

    if (pkt->pkt_data[0].len < sizeof(bcm_mac_t)) {
        return BCM_E_PARAM;
    }

    /* Get pointers to srcmac and vlan; check if bad block count */
    _get_mac_vlan_ptrs(dv, pkt, &src_mac, &vlan_ptr, &block_offset,
        &byte_offset, pkt_idx);
    if (block_offset >= pkt->blk_count) {
        return BCM_E_PARAM;
    }

    if (byte_offset >= pkt->pkt_data[block_offset].len) {
        byte_offset = 0;
        block_offset++;
    }

    #define BCM_PKT_HG_HDR(pkt)    ((pkt)->_higig)

    /*
     *  XGS3: Decide whether to put Higig header or PB (Pipe Bypass)
     *  header in the TX descriptor
     *  1.      Fabric mapped mode (HG header in descriptor)
     *  2.      Raw Ethernet packet steered mode (PB header in descriptor)
     */
    /** TX_ETHER_MODE */
    pkt_hg_hdr = (uint32 *)BCM_PKT_HG_HDR(pkt);

    /* Dest mac */
    SOC_IF_ERROR_RETURN(SOC_DCB_ADDTX(unit, dv,
        (sal_vaddr_t) pkt->pkt_data[0].data, sizeof(bcm_mac_t),
        pkt->tx_pbmp, pkt->tx_upbmp, pkt->tx_l3pbmp,
        dcb_flags, pkt_hg_hdr));

    pkt_len = ENET_MAC_SIZE;

    /* Source mac */
    SOC_IF_ERROR_RETURN(SOC_DCB_ADDTX(unit, dv,
        (sal_vaddr_t) src_mac, sizeof(bcm_mac_t),
        pkt->tx_pbmp, pkt->tx_upbmp, pkt->tx_l3pbmp,
        dcb_flags, pkt_hg_hdr));

    pkt_len += ENET_MAC_SIZE;

    SOC_IF_ERROR_RETURN(SOC_DCB_ADDTX(unit, dv,
        (sal_vaddr_t) vlan_ptr, sizeof(uint32),
        pkt->tx_pbmp, pkt->tx_upbmp, pkt->tx_l3pbmp,
        dcb_flags, pkt_hg_hdr));

    /*
     * If byte offset indicates we're not at the end of a packet's data
     * block, add a DCB for the rest of the current block.
     */
    if (byte_offset != 0) {
        tmp_len = pkt->pkt_data[block_offset].len - byte_offset;
        if (clear_crc && (block_offset == crc_block_offset)) {
            tmp_len -= ENET_FCS_SIZE;
        }
        SOC_IF_ERROR_RETURN(SOC_DCB_ADDTX(unit, dv,
            (sal_vaddr_t) &pkt->pkt_data[block_offset].data[byte_offset],
            tmp_len, pkt->tx_pbmp, pkt->tx_upbmp, pkt->tx_l3pbmp,
            dcb_flags, pkt_hg_hdr));
        block_offset++;
        pkt_len += tmp_len;
    }

    /* Add DCBs for the remainder of the blocks. */
    for (i = block_offset; i < pkt->blk_count; i++) {
        tmp_len = pkt->pkt_data[i].len;
        if (clear_crc && (i == crc_block_offset)) {
            tmp_len -= ENET_FCS_SIZE;
        }
        SOC_IF_ERROR_RETURN(SOC_DCB_ADDTX(unit, dv,
            (sal_vaddr_t) pkt->pkt_data[i].data,
            tmp_len, pkt->tx_pbmp, pkt->tx_upbmp, pkt->tx_l3pbmp,
            dcb_flags, pkt_hg_hdr));
        pkt_len += tmp_len;
    }

    /* If CRC allocated, adjust min length */
    if (pkt->flags & BCM_TX_CRC_ALLOC) {
        min_len = ENET_MIN_PKT_SIZE - ENET_FCS_SIZE;
    }

    /* Mark the end of the packet */
    soc_dma_desc_end_packet(dv);

    return BCM_E_NONE;
}


/*
 * Always use sync TX
 */
static int
_bcm_tx_chain_send(int unit, dv_t *dv, int async) 
{
#if 0
    if (async) {
        dv->dv_flags |= DV_F_NOTIFY_CHN;
        SOC_IF_ERROR_RETURN(soc_dma_start(unit, -1, dv));
    } else {
#endif
        SOC_IF_ERROR_RETURN(soc_dma_wait(unit, dv));
#if 0
    }
#endif

    return BCM_E_NONE;
}


int 
_bcm_tx(int unit, bcm_pkt_t *pkt, void *cookie) 
{
    dv_t *dv = NULL;
    int rv = BCM_E_NONE;
    char *err_msg = NULL;

    if (!pkt || !pkt->pkt_data || pkt->blk_count <= 0) {
        return BCM_E_PARAM;
    }

    err_msg = "Could not allocate dv/dv info";
    #define TX_EXTRA_DCB_COUNT 0
    dv = _tx_dv_alloc(unit, 1, pkt->blk_count + TX_EXTRA_DCB_COUNT, NULL, cookie, 0);

    if (!dv) {
        rv = BCM_E_MEMORY;
        goto error;
    }

    err_msg = "Could not setup or add pkt to DV";
    _ON_ERROR_GOTO(_tx_pkt_desc_add(unit, pkt, dv, 0), rv, error);

    err_msg = "Could not send pkt";
    rv = _bcm_tx_chain_send(unit, dv, 0); /* Sync send */

#ifdef DEBUG
    {
        void *p = ((dcb12_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, 0))->addr;
        /* PRINTF_DEBUG(("=> %d : %d %p %p\n",
                     unit, SOC_DCB_SIZE(unit), p, pkt->pkt_data->data));*/
    }
#endif

    error:
    _PROCESS_ERROR(unit, rv, dv, err_msg);

    return rv;
}


static struct bcm_pkt_s tmpPkt;
static bcm_pkt_blk_t tmpBlk;
extern int dumpStatistic(int unit, int port);

int 
pkt_bcm_tx(int unit, uint8 *pdata, int32 len) 
{
    int     rv;
    sal_memset(&tmpPkt, 0, sizeof(struct bcm_pkt_s));

    tmpPkt.pkt_data = &tmpBlk;
    tmpBlk.data     = pdata;
    tmpBlk.len      = len+4;
    tmpPkt.blk_count = 1;
    tmpPkt.src_port = 0;
    tmpPkt.src_mod = unit;

    tmpPkt.flags = BCM_TX_ETHER | BCM_TX_CRC_APPEND;

    rv = _bcm_tx(unit, &tmpPkt, 0);

    return rv;
}
