/*
 * $Id: dmaOps.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include <shared/bsl.h>

#include "vxWorks.h"
#include "taskLib.h"
#include "stdio.h"
#include "assert.h"
#include "dmaOps.h"
#include "salIntf.h"
#include "config.h"

#if defined(MIPSEL)
#define soc_htonl _shr_swap32

static
unsigned int
_shr_swap32(unsigned int i)
{
    i = (i << 16) | (i >> 16);

    return (i & 0xff00ffff) >> 8 | (i & 0xffff00ff) << 8;
}
#endif

soc_dma_manager_t _myDmaManager[MAX_DEVICES];

dvt_t
soc_dma_chan_dvt_get( int unit, dma_chan_t chan )
{
    return GET_SOC_DMA_MANAGER(unit)->soc_channels[ chan ].sc_type;
}


int
soc_dma_chan_config(int unit, dma_chan_t c, dvt_t type, uint32 flags)
{
    soc_dma_manager_t      *soc =    GET_SOC_DMA_MANAGER(unit);
    sdc_t               *sc = &soc->soc_channels[ c ];
    uint32              bits, cr;

    /* Define locals and turn flags into easy to use things */
    int         f_mbm       = (flags & SOC_DMA_F_MBM) != 0;
    int         f_interrupt = (flags & SOC_DMA_F_POLL) == 0;
    int         f_drop      = (flags & SOC_DMA_F_TX_DROP) != 0;
    int         f_default = (flags & SOC_DMA_F_DEFAULT) != 0;

    PRINTF_DEBUG(("soc_dma_chan_config: c=%d type=%d\n", c, type));

    assert(c >= 0 && c < N_DMA_CHAN);
    assert(!(flags & ~(SOC_DMA_F_MBM | SOC_DMA_F_POLL |
        SOC_DMA_F_TX_DROP | SOC_DMA_F_DEFAULT)));

    /* Initialize dummy Q entry */

    sc->sc_q            = NULL; /* No queued operations */
    sc->sc_dv_active    = NULL; /* No active operations */
    sc->sc_q_cnt        = 0;    /* Queue count starts at 0 */

    /* Initial state of channel */

    sc->sc_flags = 0;   /* Clear flags to start */

    /* Setup new mode */
    bits  = f_mbm ? DC_MOD_BITMAP(c) : DC_NO_MOD_BITMAP(c);
    bits |= f_drop ? DC_DROP_TX(c) : DC_NO_DROP_TX(c);

    if (type == DV_TX) {
        bits |= DC_MEM_TO_SOC(c);
        if (f_default) {
            soc->soc_dma_default_tx = sc;
        }
    } else if (type == DV_RX) {
        bits |= DC_SOC_TO_MEM(c);
        if (f_default) {
            soc->soc_dma_default_rx = sc;
        }
    } else if (type == DV_NONE) {
        f_interrupt = FALSE;    /* Force off */
    } else {
        assert(0);
    }

    sc->sc_type = type;

    cr = soc_pci_read(unit, CMIC_DMA_CTRL);
    cr &= ~DC_CHAN_MASK(c); /* Clear this channels bits */
    cr |= bits; /* Set new values */
    soc_pci_write(unit, CMIC_DMA_CTRL, cr);
    sc->sc_flags = flags;

    return 0;
}


#define DEFAULT_TX_DMA_CHANNEL   0
#define DEFAULT_RX_DMA_CHANNEL   1

static int
soc_dma_init(int unit)
{
    soc_pci_write(unit, CMIC_DMA_CTRL, 0); /* Known good state */

    SOC_IF_ERROR_RETURN
        (soc_dma_chan_config(unit, DEFAULT_TX_DMA_CHANNEL, DV_TX,
        SOC_DMA_F_MBM | SOC_DMA_F_DEFAULT));
    SOC_IF_ERROR_RETURN
        (soc_dma_chan_config(unit,  DEFAULT_RX_DMA_CHANNEL, DV_RX,
        SOC_DMA_F_MBM | SOC_DMA_F_DEFAULT));
    SOC_IF_ERROR_RETURN
        (soc_dma_chan_config(unit, 2, DV_NONE, SOC_DMA_F_MBM));
    SOC_IF_ERROR_RETURN
        (soc_dma_chan_config(unit, 3, DV_NONE, SOC_DMA_F_MBM));

    soc_pci_write(unit, 0x180, 0xffffffff);
    return 0;
}


int
soc_dma_abort( int unit )
{

    int32 channel, ctrl, to;
    int stat = 0;
    for( channel = 0; channel < N_DMA_CHAN; channel++ ) {

        soc_pci_write(unit, CMIC_DMA_STAT, DS_DMA_EN_CLR(channel));
        ctrl = soc_pci_read(unit, CMIC_DMA_CTRL);
        assert((ctrl & DC_ABORT_DMA(channel)) == 0);
        soc_pci_write(unit, CMIC_DMA_CTRL, ctrl | DC_ABORT_DMA(channel));

        to = 500000;

        while ((to >= 0) && ((soc_pci_read(unit, CMIC_DMA_STAT) & DS_DMA_ACTIVE(channel)) != 0)) {
            taskDelay( 10 );
            to -= 1000;
        }

        if ((soc_pci_read(unit, CMIC_DMA_STAT) & DS_DMA_ACTIVE(channel)) != 0) {
            PRINTF_ERROR(("soc_dma_abort_channel unit %d: "
                "channel %d abort timeout\n", unit, channel));
            stat = -1;
        }

        soc_pci_write(unit, CMIC_DMA_CTRL, ctrl);
        soc_pci_write(unit, CMIC_DMA_STAT, DS_DESC_DONE_CLR(channel));
        soc_pci_write(unit, CMIC_DMA_STAT, DS_CHAIN_DONE_CLR(channel));

#ifdef HELIX_PURGE_TX
        soc_dma_tx_purge(unit, channel);
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    }

    return stat;

}


#define soc_cm_l2p(u,a) ((uint32)(a) & 0x1fffffff)
#define soc_cm_p2l(u,a) ((uint32)(a) | 0xa0000000)

static sal_vaddr_t
dcb0_addr_get(int unit, dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    if (*d == 0) {
        return (sal_vaddr_t)0;
    } else {
        return (sal_vaddr_t)soc_cm_p2l(unit, *d);
    }
}


static void
dcb0_addr_set(int unit, dcb_t *dcb, sal_vaddr_t addr)
{
    uint32      *d = (uint32 *)dcb;

    if (addr == 0) {
        *d = 0;
    } else {
        *d = soc_cm_l2p(unit, (void *)addr);
    }
}


static sal_paddr_t
dcb0_paddr_get(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    return (sal_paddr_t)*d;
}


static void
dcb0_funcerr(int dt, char *name)
{
    PRINTF_ERROR(("ERROR: dcb%d_%s called\n", dt, name));
}


/*
 * DCB Type 9 Support
 */

#define DCB_MAX_REQCOUNT        0x7fff    /* 32KB */
#define SOC_SM_FLAGS_GET(flags, shift, mask) (((flags) >> (shift)) & (mask))
#define SOC_DMA_PURGE_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SPURGE_S, _SPURGE_MSK)
#define SOC_DMA_HG_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SHG_S, _SHG_MSK)
#define SOC_DCB_INFO_DONE       0x02
#define SOC_DCB_INFO_PKTEND     0x01

#define GETFUNCEXPR(_dt, _name, _expr)                                  \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                return _expr;                                           \
        }
#define GETFUNCFIELD(_dt, _name, _field)                                \
        GETFUNCEXPR(_dt, _name, d->_field)
#define GETFUNCERR(_dt, _name)                                          \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                COMPILER_REFERENCE(dcb);                                \
                dcb0_funcerr(_dt, #_name "_get");                       \
                return 0;                                               \
        }

#define SETFUNCEXPR(_dt, _name, _arg, _expr)                            \
        static void dcb##_dt##_##_name##_set(dcb_t *dcb, _arg) {        \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                _expr;                                                  \
        }
#define SETFUNCFIELD(_dt, _name, _field, _arg, _expr)                   \
        SETFUNCEXPR(_dt, _name, _arg, d->_field = _expr)
#define SETFUNCERR(_dt, _name, _type)                                   \
        static void dcb##_dt##_##_name##_set(dcb_t *dcb, _type val) {   \
                COMPILER_REFERENCE(dcb);                                \
                COMPILER_REFERENCE(val);                                \
                dcb0_funcerr(_dt, #_name "_set");                       \
        }

#define SETFUNCEXPRIGNORE(_dt, _name, _arg, _expr)                      \
        SETFUNCEXPR(_dt, _name, _arg, COMPILER_REFERENCE(d))
#if defined(MIPSEL)
#define GETHGFUNCEXPR(_dt, _name, _expr)                                \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                uint32  hgh[3];                                         \
                soc_higig_hdr_t *h = (soc_higig_hdr_t *)&hgh[0];        \
                hgh[0] = soc_htonl(d->mh0);                             \
                hgh[1] = soc_htonl(d->mh1);                             \
                hgh[2] = soc_htonl(d->mh2);                             \
                return _expr;                                           \
        }
#else
#define GETHGFUNCEXPR(_dt, _name, _expr)                                \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                soc_higig_hdr_t *h = (soc_higig_hdr_t *)&d->mh0;        \
                return _expr;                                           \
        }
#endif
#define GETHGFUNCFIELD(_dt, _name, _field)                              \
        GETHGFUNCEXPR(_dt, _name, h->hgp_overlay1._field)

#if defined(MIPSEL)
#define GETHG2FUNCEXPR(_dt, _name, _expr)                               \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                uint32  hgh[4];                                         \
                soc_higig2_hdr_t *h = (soc_higig2_hdr_t *)&hgh[0];      \
                hgh[0] = soc_htonl(d->mh0);                             \
                hgh[1] = soc_htonl(d->mh1);                             \
                hgh[2] = soc_htonl(d->mh2);                             \
                hgh[3] = soc_htonl(d->mh3);                             \
                return _expr;                                           \
        }
#else
#define GETHG2FUNCEXPR(_dt, _name, _expr)                               \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                soc_higig2_hdr_t *h = (soc_higig2_hdr_t *)&d->mh0;      \
                return _expr;                                           \
        }
#endif
#define GETHG2FUNCFIELD(_dt, _name, _field)                             \
        GETHG2FUNCEXPR(_dt, _name, h->ppd_overlay1._field)




static void dcb12_init(dcb_t *dcb) {
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = 0;
    d[5] = d[6] = d[7] = d[8] = d[9] = d[10] = 0;
}


static int
dcb12_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
           pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb12_t *d;      /* DCB */
    uint32 *di;     /* DCB integer pointer */
    uint32 paddr;   /* Packet buffer physical address */

    d = (dcb12_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
    d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    if (dv->dv_vcnt >= dv->dv_cnt) {
        return SOC_E_FULL;
    }
    if (dv->dv_vcnt > 0) {    /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;

    d->addr = paddr;
    d->c_count = count;
    d->c_sg = 1;

    d->c_stat = 1;
    d->c_purge = SOC_DMA_PURGE_GET(flags);
    if (SOC_DMA_HG_GET(flags)) {
        d->c_hg = 1;
        d->mh0 = hgh[0];
        d->mh1 = hgh[1];
        d->mh2 = hgh[2];
    }

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}


static int
dcb12_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb12_t *d;   /* DCB */
    uint32 *di;  /* DCB integer pointer */

    d = (dcb12_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) { /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}


static uint32
dcb12_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb12_t *d = (dcb12_t *)dcb; /* DCB */
    uint32 f;                  /* SOC_DCB_INFO_* flags */

    if (!d->done) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}


SETFUNCFIELD(12, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(12, reqcount, c_count)
GETFUNCFIELD(12, xfercount, count)
/* addr_set, addr_get, paddr_get - Same as DCB 0 */
SETFUNCFIELD(12, done, done, int val, val ? 1 : 0)
GETFUNCFIELD(12, done, done)
SETFUNCFIELD(12, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(12, sg, c_sg)
SETFUNCFIELD(12, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(12, chain, c_chain)
SETFUNCFIELD(12, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(12, reload, c_reload)
SETFUNCERR(12, tx_l2pbm, pbmp_t)
SETFUNCERR(12, tx_utpbm, pbmp_t)
SETFUNCERR(12, tx_l3pbm, pbmp_t)
SETFUNCERR(12, tx_crc, int)
SETFUNCERR(12, tx_cos, int)
SETFUNCERR(12, tx_destmod, uint32)
SETFUNCERR(12, tx_destport, uint32)
SETFUNCERR(12, tx_opcode, uint32)
SETFUNCERR(12, tx_srcmod, uint32)
SETFUNCERR(12, tx_srcport, uint32)
SETFUNCERR(12, tx_prio, uint32)
SETFUNCERR(12, tx_pfm, uint32)
GETFUNCFIELD(12, rx_start, start)
GETFUNCFIELD(12, rx_end, end)
GETFUNCFIELD(12, rx_error, error)
/* Fields extracted from MH/PBI */
/* DCB 9 specific fields */
SETFUNCFIELD(12, hg, c_hg, uint32 hg, hg)
GETFUNCFIELD(12, hg, c_hg)
SETFUNCFIELD(12, stat, c_stat, uint32 stat, stat)
GETFUNCFIELD(12, stat, c_stat)
SETFUNCFIELD(12, purge, c_purge, uint32 purge, purge)
GETFUNCFIELD(12, purge, c_purge)

GETHG2FUNCFIELD(12, rx_destmod, dst_mod)
GETHG2FUNCFIELD(12, rx_destport, dst_port)
GETHG2FUNCFIELD(12, rx_opcode, opcode)
GETHG2FUNCFIELD(12, rx_prio, vlan_pri) /* outer_pri */
GETHG2FUNCEXPR(12, rx_untagged, !h->ppd_overlay1.ingress_tagged)
GETHG2FUNCFIELD(12, rx_srcport, src_port)
GETHG2FUNCFIELD(12, rx_srcmod, src_mod)
GETFUNCFIELD(12, rx_ingport, srcport)
GETFUNCFIELD(12, rx_crc, regen_crc)
GETFUNCFIELD(12, rx_cos, cpu_cos)
GETFUNCFIELD(12, rx_matchrule, match_rule)
GETFUNCFIELD(12, rx_reason, reason)
GETHG2FUNCEXPR(12, rx_mcast, ((h->ppd_overlay1.dst_mod << 8) |
                              (h->ppd_overlay1.dst_port)))
GETHG2FUNCEXPR(12, rx_vclabel, ((h->ppd_overlay1.vc_label_19_16 << 16) |
                              (h->ppd_overlay1.vc_label_15_8 << 8) |
                              (h->ppd_overlay1.vc_label_7_0)))
GETHG2FUNCEXPR(12, rx_classtag, ((h->ppd_overlay2.ctag_hi << 8) |
                              (h->ppd_overlay2.ctag_lo)))
/*unused: GETFUNCEXPR(12, rx_mirror, ((d->imirror) | (d->emirror)))*/
    
#ifdef DEBUG
GETFUNCERR(12, tx_l2pbm)
GETFUNCERR(12, tx_utpbm)
GETFUNCERR(12, tx_l3pbm)
GETFUNCERR(12, tx_crc)
GETFUNCERR(12, tx_cos)
GETFUNCERR(12, tx_destmod)
GETFUNCERR(12, tx_destport)
GETFUNCERR(12, tx_opcode)
GETFUNCERR(12, tx_srcmod)
GETFUNCERR(12, tx_srcport)
GETFUNCERR(12, tx_prio)
GETFUNCERR(12, tx_pfm)
#endif  /* DEBUG */


#ifdef DEBUG
static void
dcb12_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb12_t *d = (dcb12_t *)dcb;
    uint8 *h = (uint8 *)&d->mh0;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    LOG_CLI((BSL_META_U(unit,
                        "%s\t%s\n"), prefix, ps));
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    LOG_CLI((BSL_META_U(unit,
                        "%s\ttype %d %ssg %schain %sreload %shg %sstat %spause %spurge\n"),
             prefix,
             SOC_DCB_TYPE(unit),
             SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
             SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
             SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
             SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
             SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
             d->c_pause ? "" : "!",
             SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!"));
    LOG_CLI((BSL_META_U(unit,
                        "%s\taddr %p reqcount %d xfercount %d\n"),
             prefix,
             (void *)SOC_DCB_ADDR_GET(unit, dcb),
             SOC_DCB_REQCOUNT_GET(unit, dcb),
             SOC_DCB_XFERCOUNT_GET(unit, dcb)));
    if (!tx) {
        LOG_CLI((BSL_META_U(unit,
                            "%s\t%sdone %sstart %send %serror\n"),
                 prefix,
                 SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!"));
    }
#ifdef BCM_FIREBOLT_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
    (SOC_DCB_TYPE(unit) == 9)) {
        dcb0_reason_dump(unit, dcb, prefix);
        LOG_CLI((BSL_META_U(unit,
                            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n"),
                 prefix,
                 d->add_vid ? "" : "!",
                 d->bpdu ? "" : "!",
                 d->cell_error ? "" : "!",
                 d->chg_tos ? "" : "!",
                 d->emirror ? "" : "!",
                 d->imirror ? "" : "!"
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n"),
                 prefix,
                 d->l3ipmc ? "" : "!",
                 d->l3only ? "" : "!",
                 d->l3uc ? "" : "!",
                 d->pkt_aged ? "" : "!",
                 d->purge_cell ? "" : "!",
                 d->src_hg ? "" : "!"
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n"),
                 prefix,
                 d->switch_pkt ? "" : "!",
                 d->regen_crc ? "" : "!",
                 d->decap_iptunnel ? "" : "!",
                 d->ingress_untagged ? "" : "!"
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  cpu_cos=%d cos=%d l3_intf=%d mpt_index=%d reason=%08x\n"),
                 prefix,
                 d->cpu_cos,
                 d->cos,
                 d->l3_intf,
                 (d->mpt_index_hi << 2) | d->mpt_index_lo,
                 d->reason
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  match_rule=%d nh_index=%d\n"),
                 prefix,
                 d->match_rule,
                 d->nh_index
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n"),
                 prefix,
                 d->srcport,
                 (d->dscp_hi << 4) | d->dscp_lo,
                 d->outer_pri,
                 d->outer_cfi,
                 d->outer_vid
                 ));
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
    (SOC_DCB_TYPE(unit) == 10)) {
        dcb10_t *d = (dcb10_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        LOG_CLI((BSL_META_U(unit,
                            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n"),
                 prefix,
                 d->add_vid ? "" : "!",
                 d->bpdu ? "" : "!",
                 d->cell_error ? "" : "!",
                 d->chg_tos ? "" : "!",
                 d->emirror ? "" : "!",
                 d->imirror ? "" : "!"
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n"),
                 prefix,
                 d->l3ipmc ? "" : "!",
                 d->l3only ? "" : "!",
                 d->l3uc ? "" : "!",
                 d->pkt_aged ? "" : "!",
                 d->purge_cell ? "" : "!",
                 d->src_hg ? "" : "!"
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  %sswitch_pkt %sregen_crc %spkt_not_changed\n"),
                 prefix,
                 d->switch_pkt ? "" : "!",
                 d->regen_crc ? "" : "!",
                 d->pkt_not_changed ? "" : "!"
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  cpu_cos=%d cos=%d l3_intf=%d mpt_index=%d reason=%08x\n"),
                 prefix,
                 d->cpu_cos,
                 d->cos,
                 d->l3_intf,
                 d->mpt_index,
                 ((d->reason_hi << 6) | (d->reason_lo))
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  decap_iptunnel=%d match_rule=%d nh_index=%d\n"),
                 prefix,
                 d->decap_iptunnel,
                 d->match_rule,
                 ((d->nh_index_hi << 1) | (d->nh_index_lo))
                 ));
        LOG_CLI((BSL_META_U(unit,
                            "%s  srcport=%d dscp=%d ecn=%d %sing_untagged\n"),
                 prefix,
                 d->srcport,
                 d->dscp,
                 d->ecn,
                 d->ingress_untagged ? "" : "!"
                 ));
    }
#endif /* BCM_EASYRIDER_SUPPORT */
}
#endif  /* DEBUG */

dcb_op_t dcb12_op = {
    9,
    sizeof(dcb12_t),
    dcb12_init,
    dcb12_addtx,
    dcb12_addrx,
    dcb12_intrinfo,
    dcb12_reqcount_set,
    dcb12_reqcount_get,
    dcb12_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb12_done_set,
    dcb12_done_get,
    dcb12_sg_set,
    dcb12_sg_get,
    dcb12_chain_set,
    dcb12_chain_get,
    dcb12_reload_set,
    dcb12_reload_get,
    dcb12_tx_l2pbm_set,
    dcb12_tx_utpbm_set,
    dcb12_tx_l3pbm_set,
    dcb12_tx_crc_set,
    dcb12_tx_cos_set,
    dcb12_tx_destmod_set,
    dcb12_tx_destport_set,
    dcb12_tx_opcode_set,
    dcb12_tx_srcmod_set,
    dcb12_tx_srcport_set,
    dcb12_tx_prio_set,
    dcb12_tx_pfm_set,
    dcb12_rx_untagged_get,
    dcb12_rx_crc_get,
    dcb12_rx_cos_get,
    dcb12_rx_destmod_get,
    dcb12_rx_destport_get,
    dcb12_rx_opcode_get,
    dcb12_rx_classtag_get,
    dcb12_rx_matchrule_get,
    dcb12_rx_start_get,
    dcb12_rx_end_get,
    dcb12_rx_error_get,
    dcb12_rx_prio_get,
    dcb12_rx_reason_get,
    dcb12_rx_ingport_get,
    dcb12_rx_srcport_get,
    dcb12_rx_srcmod_get,
    dcb12_rx_mcast_get,
    dcb12_rx_vclabel_get,
    dcb12_hg_set,
    dcb12_hg_get,
    dcb12_stat_set,
    dcb12_stat_get,
    dcb12_purge_set,
    dcb12_purge_get,
#ifdef  DEBUG
    dcb12_tx_l2pbm_get,
    dcb12_tx_utpbm_get,
    dcb12_tx_l3pbm_get,
    dcb12_tx_crc_get,
    dcb12_tx_cos_get,
    dcb12_tx_destmod_get,
    dcb12_tx_destport_get,
    dcb12_tx_opcode_get,
    dcb12_tx_srcmod_get,
    dcb12_tx_srcport_get,
    dcb12_tx_prio_get,
    dcb12_tx_pfm_get,

    dcb12_dump,
    dcb0_reason_dump,
#endif /* DEBUG */
};

void
soc_dma_dv_reset(dvt_t op, dv_t *dv)
{
    dv->dv_op      = op;
    dv->dv_channel = -1;
    dv->dv_vcnt    = 0;
    dv->dv_dcnt    = 0;
    /* don't clear all flags */
    dv->dv_flags   &= ~(DV_F_NOTIFY_DSC | DV_F_NOTIFY_CHN);
    dv->dv_chain   = NULL;
    dv->dv_public1.ptr = NULL;
    dv->dv_public2.ptr = NULL;
    dv->dv_public3.ptr = NULL;
    dv->dv_public4.ptr = NULL;
    /*    sal_memset(&dv->tx_param, 0, sizeof(soc_tx_param_t));*/
}


dv_t *
soc_dma_dv_alloc(int unit, dvt_t op, int cnt)
{
    soc_dma_manager_t       *soc =   GET_SOC_DMA_MANAGER(unit);
    dv_t                    *dv;

    assert(cnt > 0);

    /* Always bump up DCB count to free list size */
    if (cnt < soc->soc_dv_size) {
        cnt = soc->soc_dv_size;
    }

    /* Check if we can use one off the free list */

    SOC_DMA_LOCK(soc);
    soc->stat.dv_alloc++;
    if ((cnt == soc->soc_dv_size) && (soc->soc_dv_free_cnt > 0)) {
        dv = soc->soc_dv_free;

        soc->soc_dv_free = dv->dv_chain;
        soc->soc_dv_free_cnt--;
        soc->stat.dv_alloc_q++;
        SOC_DMA_UNLOCK(soc);
    } else {
        SOC_DMA_UNLOCK(soc);
        dv = soc_cm_salloc(unit, sizeof(dv_t), "soc_dma_dv_alloc");
        if (dv == NULL) {
            return(dv);
        }
        dv->dv_dcb = soc_cm_salloc(unit, SOC_DCB_SIZE(unit) * cnt,
            "sdma_dv_alloc");
        if (dv->dv_dcb == NULL) {
            soc_cm_sfree(unit, dv);
            return(NULL);
        }
        dv->dv_unit = unit;
        dv->dv_cnt = cnt;
        dv->dv_flags = ((op == DV_TX) ? DV_F_COMBINE_DCB : 0);
    }
    dv->dv_done_packet = NULL;
    dv->dv_done_desc = NULL;
    dv->dv_done_chain = NULL;
    dv->dv_magic = DV_MAGIC_NUMBER;
    soc_dma_dv_reset(op, dv);     /* Reset standard fields */

    return(dv);
}


uint32 soc_intr_enable(int unit, uint32 mask);

static void
_soc_dma_start_channel(int unit, sdc_t *sc)
{
    dv_t *dv;

    if (NULL != (dv = sc->sc_dv_active = sc->sc_q)) {
        /* Set up DMA descriptor address */
        soc_pci_write(unit, CMIC_DMA_DESC(sc->sc_channel),
            soc_cm_l2p(unit, sc->sc_q->dv_dcb));
        /* Start DMA (Actually starts when the done bit is cleared below) */
        soc_pci_write(unit, CMIC_DMA_STAT,
            DS_DMA_EN_SET(sc->sc_channel));
        /*
         * Clear CHAIN_DONE if required, and re-enable the
         * interrupt. This is required to support multiple DMA
         */

        if (sc->sc_flags & SOC_DMA_F_CLR_CHN_DONE) {
            soc_pci_write(unit, CMIC_DMA_STAT,
                DS_CHAIN_DONE_CLR(sc->sc_channel));
        } else {
            sc->sc_flags |= SOC_DMA_F_CLR_CHN_DONE;
        }

        (void)soc_intr_enable(unit, IRQ_CHAIN_DONE(sc->sc_channel));
    } else {
        sc->sc_q_tail = NULL;
    }
}


static void
_soc_dma_start(int unit, sdc_t *sc, dv_t *dv_chain)
{
    soc_dma_manager_t      *soc =    GET_SOC_DMA_MANAGER(unit);
    int                 i;
    dv_t                *dv;
    dcb_t               *dcb;

    assert(sc->sc_type == dv_chain->dv_op);
    assert(!(dv_chain->dv_flags & DV_F_NOTIFY_CHN) ||
        dv_chain->dv_done_chain);
    assert(!(dv_chain->dv_flags & DV_F_NOTIFY_DSC) ||
        dv_chain->dv_done_desc);

    /* If DK_DMA set, dump out info on request, before queued on channel */
    /* Clean cache of any dirty data */

    for (dv = dv_chain; dv; dv = dv->dv_chain) {
        soc_cm_sflush(unit, dv->dv_dcb,
            dv->dv_vcnt * SOC_DCB_SIZE(unit));

        for (i = 0; i < dv->dv_vcnt; i++) {
            uint32      cnt;
            sal_vaddr_t addr;

            dcb = SOC_DCB_IDX2PTR(unit, dv->dv_dcb, i);
            cnt = SOC_DCB_REQCOUNT_GET(unit, dcb);
            addr = SOC_DCB_ADDR_GET(unit, dcb);
            if (dv_chain->dv_op == DV_RX) {
                soc_cm_sinval(unit, (void *)addr, cnt);
            } else {
                assert(dv_chain->dv_op == DV_TX);
                soc_cm_sflush(unit, (void *)addr, cnt);
            }
        }
    }

    /* Mark Channel # */

    dv_chain->dv_channel = sc->sc_channel;

    SOC_DMA_LOCK(soc);
    dv_chain->dv_next = NULL;
    if (sc->sc_q_cnt != 0) {
        sc->sc_q_tail->dv_next = dv_chain;
    } else {
        sc->sc_q = dv_chain;
    }
    sc->sc_q_tail = dv_chain;
    sc->sc_q_cnt++;
    if (sc->sc_dv_active == NULL) {  /* Start DMA if channel not active */
        _soc_dma_start_channel(unit, sc);
    }
    SOC_DMA_UNLOCK(soc);
}


static sdc_t *
soc_dma_channel(int unit, dma_chan_t channel, dv_t *dv_chain)
{
    soc_dma_manager_t *soc = GET_SOC_DMA_MANAGER(unit);
    sdc_t             *cd;

    if (channel < 0) {      /* Choose a channel */
        switch(dv_chain->dv_op) {
            case DV_TX:     return(soc->soc_dma_default_tx);
            case DV_RX:     return(soc->soc_dma_default_rx);
            default:        return(NULL);
        }
    } else if (channel < 0 || channel >= soc->soc_max_channels) {
        return NULL;
    } else {
        cd = &soc->soc_channels[channel];
        return((cd->sc_type == dv_chain->dv_op) ? cd : NULL);
    }
}


int
soc_dma_start(int unit, dma_chan_t channel, dv_t *dv_chain)
{
    sdc_t *sc;       /* Channel pointer */

    /* Fire off DMA operation, let interrupt handler free channel */

    sc = soc_dma_channel(unit, channel, dv_chain);
    if (sc == NULL) {
        return(SOC_E_RESOURCE);
    }

    _soc_dma_start(unit, sc, dv_chain);
    return(SOC_E_NONE);
}


void
soc_dma_desc_end_packet(dv_t *dv)
{
    dcb_t *d;

    if (dv->dv_vcnt > 0) {
        d = SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt - 1);
        SOC_DCB_SG_SET(dv->dv_unit, d, 0);
    }
}


int
soc_dma_attach(int unit, int reset)
{
    soc_dma_manager_t *soc = GET_SOC_DMA_MANAGER(unit);
    int i;

    soc->soc_dv_size = SOC_DMA_DV_FREE_SIZE;
    soc->soc_dv_cnt  = SOC_DMA_DV_FREE_CNT;

    soc->stat.dv_alloc   = 0; /* Init Alloc count */
    soc->stat.dv_free    = 0; /* Init Free count */
    soc->stat.dv_alloc_q = 0; /* Init Alloc from Q count */
    soc->soc_dv_free_cnt = 0; /* Init Free list Q Count */

    soc->soc_dma_default_tx = NULL;
    soc->soc_dma_default_rx = NULL;
    soc->soc_max_channels = N_DMA_CHAN;

    soc->dcb_op = dcb12_op;
#if 0
    /*
     * Allocate a 64 byte packet for TX purge packet
     */
    if (soc->tx_purge_pkt == NULL) {
        soc->tx_purge_pkt = soc_cm_salloc(unit, 64, "tx_purge");
    }
#endif

    soc_pci_write(unit, CMIC_DMA_CTRL, 0); /* Known good state */

    for (i = 0; i < soc->soc_max_channels; i++) {
        sdc_t   *s = &soc->soc_channels[i];

        sal_memset(s, 0, sizeof(*s)); /* Start off 0 */
        s->sc_type    = DV_NONE;
        s->sc_channel = i;
    }

    return soc_dma_init(unit);
}


static dv_t *
soc_dma_process_done_desc(int unit, dv_t *dv_chain, dv_t *dv)
{
    dcb_t       *dcb;
    int         i;
    int         tx = (dv_chain->dv_op == DV_TX);
    soc_stat_t  *stat = &GET_SOC_DMA_MANAGER(unit)->stat;
    uint32      flags, count;

    for (; dv != NULL; dv = dv->dv_chain) {
        /* Process all DCBs in the current DV. */

        /* Clean cache of any dirty data */
        soc_cm_sinval(unit, dv->dv_dcb,
            dv->dv_vcnt * SOC_DCB_SIZE(unit));

        for (i = dv->dv_dcnt; i < dv->dv_vcnt; i++) {
            dcb = SOC_DCB_IDX2PTR(unit, dv->dv_dcb, i);
            flags = SOC_DCB_INTRINFO(unit, dcb, tx, &count);
            if (flags) {
                if ((dv_chain->dv_flags & DV_F_NOTIFY_DSC) &&
                dv_chain->dv_done_desc) {
                    dv_chain->dv_done_desc(unit, dv, dcb);
                }

                if (tx) {
                    stat->dma_tbyt += count;
                    if (flags & SOC_DCB_INFO_PKTEND) {
                        if (dv_chain->dv_done_packet) {
                            dv_chain->dv_done_packet(unit, dv, dcb);
                        }
                        stat->dma_tpkt++;
                    }
                } else {
#define MIN_PAYLOAD_SIZE (64-4-12)    /* 4:VLAN, 12: DA+SA */
                    stat->dma_rbyt += count;
                    if (flags & SOC_DCB_INFO_PKTEND) {
                        if (dv_chain->dv_done_packet) {
                            dv_chain->dv_done_packet(unit, dv, dcb);
                        }
                        stat->dma_rpkt++;
                    } else if (count >= MIN_PAYLOAD_SIZE) {
                        /* pkt size > dma_len & !pkt_end */
                        if (dv_chain->dv_done_packet) {
                            dv_chain->dv_done_packet(unit, dv, dcb);
                        }
                    }
                }
            } else {
                dv->dv_dcnt = i;
                return(dv);
            }
        }
    }

    return(dv);
}

#ifdef POLLING_MODE
void
soc_dma_done_desc(int unit, uint32 chan)
{
    dma_chan_t  c = (dma_chan_t)chan;
    soc_dma_manager_t   *soc =  GET_SOC_DMA_MANAGER(unit);
    sdc_t               *sc = &soc->soc_channels[c];
    dv_t *dv_chain = sc->sc_q; /* Pick up request */
    dv_t *dv_active = sc->sc_dv_active;

    /* Clear interrupt */
    soc_pci_write(unit, CMIC_DMA_STAT, DS_DESC_DONE_CLR(c));

    /*
     * Reap done descriptors, and possibly notify callers of descriptor
     * completion.
     */
    sc->sc_dv_active = soc_dma_process_done_desc(unit, dv_chain, dv_active);
}
#endif  /* POLLING_MODE */

void
soc_dma_done_chain(int unit, uint32 chan)
{
    dma_chan_t c = (dma_chan_t)chan;
    soc_dma_manager_t *soc = GET_SOC_DMA_MANAGER(unit);
    sdc_t *sc = &soc->soc_channels[c];
    dv_t  *dv_chain;   /* Head of current chain */
    dv_t  *dv_active;  /* Current DV needed if DV chaining */

    assert(sc->sc_q_cnt > 0); /* Be sure there is at least one */
    assert(sc->sc_q != NULL); /* And a pointer too */

    soc->stat.intr_chain++;

    dv_chain  = sc->sc_q;          /* Pick up request */
    dv_active = sc->sc_dv_active;  /* DV in processing - MAY BE NULL */
    sc->sc_q  = dv_chain->dv_next; /* Unlink current DV */
    sc->sc_q_cnt--;                /* Decrement */

    /*
     * To support multiple DMA on platforms that support it, the
     * CHAIN_DONE bit must be left pending until the start of the next
     * operation. Here we clear DESC_DONE, and mask the CHAIN_DONE
     * interrupt. CHAIN_DONE is cleared when we start the next
     * operation.
     */
    (void)soc_intr_disable(unit, IRQ_CHAIN_DONE(c));
    soc_pci_write(unit, CMIC_DMA_STAT, DS_DMA_EN_CLR(c));
    soc_pci_write(unit, CMIC_DMA_STAT, DS_DESC_DONE_CLR(c));
#ifdef POLLING_MODE
    soc_pci_write(unit, CMIC_DMA_STAT, DS_CHAIN_DONE_CLR(c));
#endif

    _soc_dma_start_channel(unit, sc);

    soc_dma_process_done_desc(unit, dv_chain, dv_active);

    if (dv_chain->dv_flags & DV_F_NOTIFY_CHN) {
        if (dv_chain->dv_done_chain == NULL) {
            PRINTF_ERROR((
                "_soc_dma_done_chain: NULL callback: unit=%d chain=%p\n",
                unit, (void *)dv_chain));
        } else {
            dv_chain->dv_done_chain(unit, dv_chain);
        }
    }
}


void
soc_dma_dv_free(int unit, dv_t *dv)
{
    soc_dma_manager_t      *soc =    GET_SOC_DMA_MANAGER(unit);
    SOC_DMA_LOCK(soc);

    soc->stat.dv_free++;
    assert(dv->dv_magic == DV_MAGIC_NUMBER);
    dv->dv_magic = 0;
    if ((dv->dv_cnt == soc->soc_dv_size) &&
    (soc->soc_dv_free_cnt < soc->soc_dv_cnt)) {
        assert(dv);
        assert(dv->dv_dcb);
        dv->dv_chain = soc->soc_dv_free;
        soc->soc_dv_free = dv;
        soc->soc_dv_free_cnt++;
        SOC_DMA_UNLOCK(soc);
    } else {
        SOC_DMA_UNLOCK(soc);
        if (dv->dv_dcb) {
            soc_cm_sfree(unit, dv->dv_dcb);
        }
        soc_cm_sfree(unit, dv);
    }
}


void
soc_dma_poll_channel(int unit, dma_chan_t c)
{
    if (soc_pci_read(unit, CMIC_DMA_STAT) & DS_CHAIN_DONE_TST(c)) {
        soc_dma_done_chain(unit, (uint32)c);
    }
}


#define dv_sem          dv_public4.ptr
#define dv_poll         dv_public4.ptr

void
soc_dma_wait_done(int unit, dv_t *dv_chain)
{
    sal_sem_give((sal_sem_t)dv_chain->dv_sem);
}


void
soc_dma_poll_done(int unit, dv_t *dv_chain)
{
    *(volatile int *)dv_chain->dv_poll = TRUE;
}


extern void _tx_dv_free(int unit, dv_t *dv);

int
soc_dma_wait_timeout(int unit, dv_t *dv_chain, int usec)
{
    int         rv = SOC_E_NONE;
    sdc_t       *sc;
#ifdef POLLING_MODE
    volatile int done;
    sal_usecs_t start_time;
    int diff_time;
#endif

    if ((sc = soc_dma_channel(unit, -1, dv_chain)) == NULL) {
        return(SOC_E_RESOURCE);
    }

#ifdef POLLING_MODE
    dv_chain->dv_sem = (int)NULL;
    dv_chain->dv_done_chain = soc_dma_poll_done;
    dv_chain->dv_poll = (void *) &done;
    done = FALSE;
#else
    dv_chain->dv_sem = sal_sem_create("dv_sem", sal_sem_BINARY, 0);
    if (!dv_chain->dv_sem) {
        return -1;
    }
    dv_chain->dv_done_chain = soc_dma_wait_done;
#endif

    SET_NOTIFY_CHN_ONLY(dv_chain->dv_flags);

    _soc_dma_start(unit, sc, dv_chain);

#ifdef POLLING_MODE
    start_time = sal_time_usecs();
    diff_time = 0;
    while (!done) {
        soc_dma_poll_channel(unit, sc->sc_channel);
        if ((usec != sal_sem_FOREVER) && !done) {
            diff_time = SAL_USECS_SUB(sal_time_usecs(), start_time);
            if ((diff_time <0) || (diff_time > usec)) {
                rv = SOC_E_TIMEOUT;
                break;
            }
        }
    }
#else
    if (sal_sem_take((sal_sem_t)dv_chain->dv_sem, sal_sem_FOREVER)) {
        rv = SOC_E_TIMEOUT;
    }
    sal_sem_destroy((sal_sem_t)dv_chain->dv_sem);
#endif

    /* Free DV (note packet DMA memory is freed in NetdrvSend) */
    soc_dma_dv_free(unit, dv_chain);
    /* _tx_dv_free(unit, dv_chain); */

    return(rv);
}


int
soc_dma_wait(int unit, dv_t *dv_chain)
{
    return soc_dma_wait_timeout(unit, dv_chain, sal_sem_FOREVER);
}
