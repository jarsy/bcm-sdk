/*
 * $Id: dmaOps.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include "systemInit.h"
#include "salIntf.h"
#include "memreg.h"

#ifndef DMAOPS_H
#define DMAOPS_H

#define SOC_DMA_ROUND_LEN(x)    (((x) + 3) & ~3)

/* Do not alter ext_dcb flag below */
#define SET_NOTIFY_CHN_ONLY(flags) do { \
        (flags) |=  DV_F_NOTIFY_CHN; \
        (flags) &= ~DV_F_NOTIFY_DSC; \
    } while (0)

/* Try to avoid all other flags */
#define         SOC_DMA_F_PKT_PROP      0x10000000/* 1 << 28 */

/*
 * Note DMA_F_INTR is NOT a normal flag.
 *    Interrupt mode is the default behavior and is ! POLLED mode.
 */
#define SOC_DMA_F_INTR          0x00      /* Interrupt Mode */

#define SOC_DMA_F_MBM           0x01      /* Modify bit MAP */
#define SOC_DMA_F_POLL          0x02      /* POLL mode */
#define SOC_DMA_F_TX_DROP       0x04      /* Drop if no ports */
#define SOC_DMA_F_JOIN          0x08      /* Allow low level DV joins */
#define SOC_DMA_F_DEFAULT       0x10      /* Default channel for type */
#define SOC_DMA_F_CLR_CHN_DONE  0x20      /* Clear Chain-done on start */

#define SOC_DMA_COS(_x)         ((_x) << 0)
#define SOC_DMA_COS_GET(_x)     (((_x) >> 0) & 7)
#define SOC_DMA_CRC_REGEN       (1 << 3)
#define SOC_DMA_CRC_GET(_x)     (((_x) >> 3) & 1)
#define SOC_DMA_RLD             (1 << 4)
#define SOC_DMA_HG              (1 << 20)
#define SOC_DMA_STATS           (1 << 21)
#define SOC_DMA_PURGE           (1 << 22)

/* 
 * In current type 3 descriptor, DMOD is 5 bits; DPORT is 4 bits.
 * TGID/PORT indicator is 1 bit, (we ignore it for now).
 *
 * For now we use 5 bits for mod, 6 bits for port (for tucana)
 * and 1 for trunk indicator
 */

#define _SDP_MSK 0x3f      /* 10:5 */
#define _SDP_S 5           /* 10:5 */
#define _SDM_MSK 0x1f      /* 15:11 */
#define _SDM_S 11          /* 15:11 */
#define _SDT_MSK 0x1       /* 16:16 */
#define _SDT_S 16          /* 16:16 */
#define _SMHOP_MSK 0x7     /* 19:17 */
#define _SMHOP_S 17        /* 19:17 */

/* 
 * type 9 descriptor, Higig, stats, purge bits
 */

#define _SHG_MSK 0x1       /* 20:20 */
#define _SHG_S 20          /* 20:20 */
#define _SSTATS_MSK 0x1    /* 21:21 */
#define _SSTATS_S 21       /* 21:21 */
#define _SPURGE_MSK 0x1    /* 22:22 */
#define _SPURGE_S 22       /* 22:22 */

#define SOC_DMA_DPORT_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SDP_S, _SDP_MSK)
#define SOC_DMA_DPORT_SET(flags, val) \
    SOC_SM_FLAGS_SET(flags, val, _SDP_S, _SDP_MSK)

#define SOC_DMA_DMOD_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SDM_S, _SDM_MSK)
#define SOC_DMA_DMOD_SET(flags, val) \
    SOC_SM_FLAGS_SET(flags, val, _SDM_S, _SDM_MSK)

#define SOC_DMA_DTGID_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SDT_S, _SDT_MSK)
#define SOC_DMA_DTGID_SET(flags, val) \
    SOC_SM_FLAGS_SET(flags, val, _SDT_S, _SDT_MSK)

#define SOC_DMA_MHOP_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SMHOP_S, _SMHOP_MSK)
#define SOC_DMA_MHOP_SET(flags, val) \
    SOC_SM_FLAGS_SET(flags, val, _SMHOP_S, _SMHOP_MSK)

#define SOC_DMA_HG_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SHG_S, _SHG_MSK)
#define SOC_DMA_HG_SET(flags, val) \
    SOC_SM_FLAGS_SET(flags, val, _SHG_S, _SHG_MSK)

#define SOC_DMA_STATS_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SSTATS_S, _SSTATS_MSK)
#define SOC_DMA_STATS_SET(flags, val) \
    SOC_SM_FLAGS_SET(flags, val, _SSTATS_S, _SSTATS_MSK)

#define SOC_DMA_PURGE_GET(flags) \
    SOC_SM_FLAGS_GET(flags, _SPURGE_S, _SPURGE_MSK)
#define SOC_DMA_PURGE_SET(flags, val) \
    SOC_SM_FLAGS_SET(flags, val, _SPURGE_S, _SPURGE_MSK)

/*
 * Defines:
 *      SOC_DMA_DV_FREE_CNT/SOC_DMA_DV_FREE_SIZE
 * Purpose:
 *      Defines the number of free DV's that the DMA code will cache
 *      to avoid calling alloc/free routines. FREE_CNT is number of
 *      dv_t's, and FREE_SIZE is the number of DCBs in the DV.
 * Notes:
 *      Allocation requests for DV's with < FREE_SIZE dcbs MAY result in
 *      an allocation of a DV with FREE_SIZE dcbs. Allocations with
 *      > FREE_SIZE will always result in calls to memory allocation
 *      routines.
 */

#define SOC_DMA_DV_FREE_CNT     16
#define SOC_DMA_DV_FREE_SIZE    8

/* Determines alloc size of DMA'able data in DVs */
#define SOC_DV_PKTS_MAX 16

#define DV_MAGIC_NUMBER 0xba5eba11

#define DV_NONE 0       /* Disable/Invalid */
#define DV_TX   1       /* Transmit data */
#define DV_RX   2       /* Receive data  */

/* Type for DMA channel number */
typedef void dcb_t;
typedef int8 dma_chan_t;
typedef int8 bcm_dma_chan_t;
typedef int8 dvt_t;           /* DV Type definition */

/*
 * Typedef:
 *      dv_t
 * Purpose:
 *      Maps out the I/O Vec structure used to pass DMA chains to the
 *      the SOC DMA routines "DMA I/O Vector".
 * Notes:
 *      To start a DMA request, the caller must fill in:
 *        dv_op: operation to perform (DV_RX or DV_TX).
 *        dv_flags: Set DV_F_NOTIFY_DSC for descriptor done callout
 *                  Set DV_F_NOTIFY_CHN for chain done callout
 *                  Set DV_F_WAIT to suspend in dma driver
 *        dv_valid: # valid DCB entries (this field is initialized
 *                by soc_dma_dv_alloc, and set properly if
 *                soc_dma_add_dcb is called to build chain).
 *        dv_done_chain: NULL if no callout for chain done, or the
 *                address of the routine to call when chain done
 *                is seen. It is called with 2 parameters, the
 *                unit # and a pointer to the DV chain done has been
 *                seen on.
 *        dv_done_desc: NULL for synchronous call, or the address of
 *                the function to call on descriptor done. The function
 *                is with 3 parameters, the unit #, a pointer to
 *                the DV structure, and a pointer to the DCB completed.
 *                One call is made for EVERY DCB, and only if the
 *                DCB is DONE.
 *        dv_done_packet: NULL if no callout for packet done, or the
 *                address of the routine to call when packet done
 *                is seen. It has the same prototype as dv_done_desc.
 *        dv_public1 - 4: Not used by DMA routines,
 *                for use by caller.
 *
 *     Scatter/gather is implemented through multiple DCBs pointing to
 *     different buffers with the S/G bit set.  End of S/G chain (end of
 *     packet) is indicated by having S/G bit clear in the DCB.
 *
 *     Chains of packets can be associated with a single DV.  This
 *     keeps the HW busy DMA'ing packets even as interrupts are
 *     processed.  DVs can be chained (a software construction)
 *     which will start a new DV from this file rather than calling
 *     back.  This is not done much in our code.
 */
typedef union {
    uint8           u8;
    uint16          u16;
    uint32          u32;
    uint64          u64;
    sal_paddr_t     paddr;
    sal_vaddr_t     vaddr;
    void            *ptr;
} any_t;

typedef struct dv_s {
    struct dv_s *dv_next,            /* Queue pointers if required */
                *dv_chain;           /* Pointer to next DV in chain */
    int         dv_unit;             /* Unit dv is allocated on */
    uint32      dv_magic;            /* Used to indicate valid */
    dvt_t       dv_op;               /* Operation to be performed */
    dma_chan_t  dv_channel;          /* Channel queued on */
    int         dv_flags;                         /* Flags for operation */
    /* Fix soc_dma_dv_reset if you add flags */
#define DV_F_NOTIFY_DSC    0x01      /* Notify on dsc done */
#define DV_F_NOTIFY_CHN    0x02      /* Notify on chain done */
#define DV_F_COMBINE_DCB   0x04      /* Combine DCB where poss. */
#define DV_F_NEEDS_REFILL  0x10      /* Needs to be refilled */
    int16       dv_cnt;              /* # descriptors allocated */
    int16       dv_vcnt;             /* # descriptors valid */
    int16       dv_dcnt;             /* # descriptors done */
    void        (*dv_done_chain)(int u, struct dv_s *dv_chain);
    void        (*dv_done_desc)(int u, struct dv_s *dv, dcb_t *dcb);
    void        (*dv_done_packet)(int u, struct dv_s *dv, dcb_t *dcb);
    any_t       dv_public1;          /* For caller */
    any_t       dv_public2;          /* For caller */
    any_t       dv_public3;          /* For caller */
    any_t       dv_public4;          /* For caller */

    /*
     * Information for SOC_TX_PKT_PROPERTIES.
     * Normally, packets are completely independent across a DMA TX chain.
     * In order to program the cmic dma tx register effeciently, the data
     * in soc_tx_param must be consistent for all packets in the chain.
     */
    /* soc_tx_param_t tx_param; */

    /*
     * NOTE: dv_t structures must be allocated in DMA-safe memory
     * because of dv_dmabuf.
     * Buffer for gather-inserted data.  Possibly includes:
     *     HiGig hdr     (12 bytes)
     *     SL tag        (4 bytes)
     *     VLAN tag      (4 bytes)
     *     Dbl VLAN tag  (4 bytes)
     */
    uint8       dv_dmabuf[SOC_DV_PKTS_MAX][24];

#define SOC_DV_HG_HDR(dv, i) (&((dv)->dv_dmabuf[i][0]))
#define SOC_DV_SL_TAG(dv, i) (&((dv)->dv_dmabuf[i][12]))
#define SOC_DV_VLAN_TAG(dv, i) (&((dv)->dv_dmabuf[i][16]))
#define SOC_DV_VLAN_TAG2(dv, i) (&((dv)->dv_dmabuf[i][20]))

    dcb_t       *dv_dcb;
} dv_t;

/*
 * Typedef:
 *      sdc_t (SOC DMA Control)
 * Purpose:
 *      Each DMA channel on each SOC has one of these structures to
 *      track the currently active or queued operations.
 */

typedef struct sdc_s {
    dma_chan_t  sc_channel;          /* Channel # for reverse lookup */
    dvt_t       sc_type;             /* DV type that we accept */
    uint8       sc_flags;            /* See SDMA_CONFIG_XXX */
    dv_t        *sc_q;               /* Request queue head */
    dv_t        *sc_q_tail;          /* Request queue tail */
    dv_t        *sc_dv_active;       /* Pointer to individual active DV */
    int         sc_q_cnt;            /* # requests queued + active */
} sdc_t;

/***************************************************** pkt.h ****/
typedef struct bcm_pkt_s bcm_pkt_t;

typedef void (*bcm_pkt_cb_f)(int unit, bcm_pkt_t *pkt, void *cookie);

/* BCM packet gather block type */
typedef struct bcm_pkt_blk_s {
    uint8 *data;
    int len;
} bcm_pkt_blk_t;

struct bcm_pkt_s {
    bcm_pkt_blk_t    *pkt_data;      /* Pointer to array of data blocks */
    uint8             blk_count;     /* Number of blocks in array */

    /* TX/RX common members */
    uint8             unit;
    uint8             cos;           /* The local COS# to use */
    uint8             prio_int;      /* Internal priority of the packet */
    uint8             src_port;      /* Header/tag ONLY.  Use rx_port below */
    uint8             src_mod;
    uint8             dest_port;     /* Header/tag ONLY.  Use tx_pbmp below */
    uint8             dest_mod;
    uint8             opcode;

    /* Module header Op-Codes */
#define BCM_HG_OPCODE_CPU  0x00  /* CPU Frame */
#define BCM_HG_OPCODE_UC   0x01  /* Unicast Frame */
#define BCM_HG_OPCODE_BC   0x02  /* Broadcast or DLF frame */
#define BCM_HG_OPCODE_MC   0x03  /* Multicast Frame */
#define BCM_HG_OPCODE_IPMC 0x04  /* IP Multicast Frame */

    uint16            pkt_len;       /* According to flags */
    uint16            tot_len;       /* As transmitted or received */

    /* TX only members */
    bcm_pbmp_t        tx_pbmp;       /* Target ports */
    bcm_pbmp_t        tx_upbmp;      /* Untagged ports */
    bcm_pbmp_t        tx_l3pbmp;     /* L3 ports */
    uint8             pfm;           /* see BCM_PORT_PFM_* */

    /* RX only members */
    uint32            rx_reason;     /* opcode from packet */
    uint8             rx_unit;       /* Local rx unit */
    uint8             rx_port;       /* Local rx port; not in HG hdr */
    uint8             rx_cpu_cos;    /* CPU may get pkt on diff cos */
    uint8             rx_untagged;   /* The packet was untagged on ingress */

    /* Callback information */
    void             *cookie;        /* User data */
    void             *cookie2;       /* User data 2 */
    bcm_pkt_cb_f      call_back;     /* Callback function */

    /* Packet administration */
    uint32            flags;         /* See below */
    /* State flags */
#define BCM_PKT_F_HGHDR     0x1  /* HiGig header is active (Internal) */
#define BCM_PKT_F_SLTAG     0x2  /* SL tag is active */
#define BCM_PKT_F_NO_VTAG   0x4  /* Packet does not contain vlan tag */
#define BCM_PKT_F_TX_UNTAG  0x8  /* TX packet untagged (Internal) */

    /* Action flags */
#define BCM_TX_CRC_FLD      0xf0
#define BCM_TX_CRC_ALLOC    0x10
#define BCM_TX_CRC_REGEN    0x20
#define BCM_TX_CRC_APPEND   (BCM_TX_CRC_ALLOC + BCM_TX_CRC_REGEN)
#define BCM_TX_CRC_FORCE_ERROR  0x40
#define BCM_TX_NO_PAD       0x100    /* Don't pad runt packets */
#define BCM_TX_FAST_PATH    0x200    /* Fast path tx */
#define BCM_TX_PURGE        0x400    /* XGS3 Set PURGE bit in DCB(Internal) */

#define BCM_TX_RELIABLE     0x1000   /* Relay (tunnel) packet reliably. */
#define BCM_TX_BEST_EFFORT  0x2000   /* Use best effort to relay packet */

#define BCM_TX_PKT_PROP_ANY 0xf0000  /* All packet property fields */
#define BCM_TX_SRC_MOD      0x10000  /* Use the src_mod field from pkt */
#define BCM_TX_SRC_PORT     0x20000  /* Use the src_port field  '' */
#define BCM_TX_PRIO_INT     0x40000  /* Use the prio_int field  '' */
#define BCM_TX_PFM          0x80000  /* Use PFM field from pkt */

#define BCM_TX_ETHER        0x100000 /* Fully Mapped Packet TX */
#define BCM_TX_HG_READY     0x200000 /* Higig header in _higig */

    /* RX flags start at 0x1000000 */
    /* CRC_STRIP just means don't include the CRC in the length of the pkt */
#define BCM_RX_CRC_STRIP    0x1000000
#define BCM_RX_TUNNELLED    0x2000000 /* Packet was tunnelled.  */

    void             *next;               /* When linked into lists */
    int8             dma_channel;         /* DMA channel used; may be -1 */

    /* BCM, BCM application and SOC internal use only */
    bcm_pkt_blk_t    _pkt_data;           /* For single block packets */
    bcm_pkt_t       *_last_pkt;           /* To link to end of linked lists */
    void            *_dv;                 /* DV controlling this packet */
    int8             _idx;                /* Packet's index in the DV for RX */
    bcm_pkt_t       *_next;               /* For BCM layer linked lists */
    void            *alloc_ptr;           /* Pointer for deallocation */
    void            *trans_ptr;           /* Transport pointer associated w/ pkt */

    /* The following are always in NETWORK BYTE ORDER */
    uint8            _higig[12];          /* HiGig header value */
    uint8            _pb_hdr[12];         /* Pipe Bypass Header */
    uint8            _sltag[4];           /* SL tag value */
    uint8            _vtag[4];            /* VLAN tag if not in packet */
    uint8            _vtag2[4];           /* Second VLAN tag if not in packet */
};

/*
 * DCB Operations
 */
typedef struct {
    int         dcbtype;
    int         dcbsize;
    void        (*init)(dcb_t *dcb);
    /* composite helper functions */
    int         (*addtx)(struct dv_s *dv, sal_vaddr_t addr, uint32 count,
        pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm,
        uint32 flags, uint32 *hgh);
    int         (*addrx)(struct dv_s *dv, sal_vaddr_t addr, uint32 count,
        uint32 flags);
    uint32      (*intrinfo)(int unit, dcb_t *dcb, int tx, uint32 *count);
    /* basic dcb values */
    void        (*reqcount_set)(dcb_t *dcb, uint32 count);
    uint32      (*reqcount_get)(dcb_t *dcb);
    uint32      (*xfercount_get)(dcb_t *dcb);
    void        (*addr_set)(int unit, dcb_t *dcb, sal_vaddr_t addr);
    sal_vaddr_t (*addr_get)(int unit, dcb_t *dcb);
    sal_paddr_t (*paddr_get)(dcb_t *dcb);
    void        (*done_set)(dcb_t *dcb, int val);
    uint32      (*done_get)(dcb_t *dcb);
    void        (*sg_set)(dcb_t *dcb, int val);
    uint32      (*sg_get)(dcb_t *dcb);
    void        (*chain_set)(dcb_t *dcb, int val);
    uint32      (*chain_get)(dcb_t *dcb);
    void        (*reload_set)(dcb_t *dcb, int val);
    uint32      (*reload_get)(dcb_t *dcb);
    /* transmit dcb controls */
    void        (*tx_l2pbm_set)(dcb_t *dcb, pbmp_t pbm);
    void        (*tx_utpbm_set)(dcb_t *dcb, pbmp_t pbm);
    void        (*tx_l3pbm_set)(dcb_t *dcb, pbmp_t pbm);
    void        (*tx_crc_set)(dcb_t *dcb, int crc);
    void        (*tx_cos_set)(dcb_t *dcb, int cos);
    void        (*tx_destmod_set)(dcb_t *dcb, uint32 modid);
    void        (*tx_destport_set)(dcb_t *dcb, uint32 port);
    void        (*tx_opcode_set)(dcb_t *dcb, uint32 opcode);
    void        (*tx_srcmod_set)(dcb_t *dcb, uint32 modid);
    void        (*tx_srcport_set)(dcb_t *dcb, uint32 port);
    void        (*tx_prio_set)(dcb_t *dcb, uint32 prio);
    void        (*tx_pfm_set)(dcb_t *dcb, uint32 pfm);

    /* receive dcb controls */
    uint32      (*rx_untagged_get)(dcb_t *dcb);
    uint32      (*rx_crc_get)(dcb_t *dcb);
    uint32      (*rx_cos_get)(dcb_t *dcb);
    uint32      (*rx_destmod_get)(dcb_t *dcb);
    uint32      (*rx_destport_get)(dcb_t *dcb);
    uint32      (*rx_opcode_get)(dcb_t *dcb);
    uint32      (*rx_classtag_get)(dcb_t *dcb);
    uint32      (*rx_matchrule_get)(dcb_t *dcb);
    uint32      (*rx_start_get)(dcb_t *dcb);
    uint32      (*rx_end_get)(dcb_t *dcb);
    uint32      (*rx_error_get)(dcb_t *dcb);
    uint32      (*rx_prio_get)(dcb_t *dcb);
    uint32      (*rx_reason_get)(dcb_t *dcb);
    uint32      (*rx_ingport_get)(dcb_t *dcb);
    uint32      (*rx_srcport_get)(dcb_t *dcb);
    uint32      (*rx_srcmod_get)(dcb_t *dcb);
    uint32      (*rx_mcast_get)(dcb_t *dcb);
    uint32      (*rx_vclabel_get)(dcb_t *dcb);
    void        (*hg_set)(dcb_t *dcb, uint32 hg);
    uint32      (*hg_get)(dcb_t *dcb);
    void        (*stat_set)(dcb_t *dcb, uint32 stat);
    uint32      (*stat_get)(dcb_t *dcb);
    void        (*purge_set)(dcb_t *dcb, uint32 purge);
    uint32      (*purge_get)(dcb_t *dcb);
#ifdef  DEBUG
    /* miscellaneous debug helpers */
    uint32      (*tx_l2pbm_get)(dcb_t *dcb);
    uint32      (*tx_utpbm_get)(dcb_t *dcb);
    uint32      (*tx_l3pbm_get)(dcb_t *dcb);
    uint32      (*tx_crc_get)(dcb_t *dcb);
    uint32      (*tx_cos_get)(dcb_t *dcb);
    uint32      (*tx_destmod_get)(dcb_t *dcb);
    uint32      (*tx_destport_get)(dcb_t *dcb);
    uint32      (*tx_opcode_get)(dcb_t *dcb);
    uint32      (*tx_srcmod_get)(dcb_t *dcb);
    uint32      (*tx_srcport_get)(dcb_t *dcb);
    uint32      (*tx_prio_get)(dcb_t *dcb);
    uint32      (*tx_pfm_get)(dcb_t *dcb);

    void        (*dump)(int unit, dcb_t *dcb, char *prefix, int tx);
    void        (*reason_dump)(int unit, dcb_t *dcb, char *prefix);
#endif /* DEBUG */
} dcb_op_t;

/*
 * DMA Control Block - Type 12
 * Used on 56218 devices
 * 11 words
 */
typedef struct {
        uint32  addr;                   /* T12.0: physical address */
                                        /* T12.1: Control 0 */
#ifdef  LE_HOST
        uint32  c_count:16,             /* Requested byte count */
                c_chain:1,              /* Chaining */
                c_sg:1,                 /* Scatter Gather */
                c_reload:1,             /* Reload */
                c_hg:1,                 /* Higig (TX) */
                c_stat:1,               /* Reserved (TX) */
                c_pause:1,              /* Reserved (TX) */
                c_purge:1,              /* Purge packet (TX) */
                :9;                     /* Don't care */
#else
        uint32  :9,                     /* Don't care */
                c_purge:1,              /* Purge packet (TX) */
                c_pause:1,
                c_stat:1,
                c_hg:1,
                c_reload:1,
                c_sg:1,
                c_chain:1,
                c_count:16;
#endif  /* LE_HOST */
        uint32  mh0;                    /* T12.2: Module Header word 0 */
        uint32  mh1;                    /* T12.3: Module Header word 1 */
        uint32  mh2;                    /* T12.4: Module Header word 2 */
        uint32  mh3;                    /* T12.5: Module Header word 3 */
#ifdef  LE_HOST
                                        /* T12.6: RX Status 0 */
        uint32  :6,                     /* Reserved */
                regen_crc:1,            /* Regenerate CRC */
                switch_pkt:1,           /* Switched packet */
                src_hg:1,               /* Source is Higig */
                purge_cell:1,           /* Packet is marked Purged */
                pkt_aged:1,             /* Pkt is Aged */
                mtp_index:7,            /* Mirror-to-Port Index */
                l3uc:1,                 /* L3 UC */
                imirror:1,              /* Ingress Mirroring */
                emirror:1,              /* Egress Mirroring */
                cos:3,                  /* Packet Priority */
                cpu_cos:3,              /* CPU COS */
                chg_tos:1,              /* DSCP Changed */
                cell_error:1,           /* Cell CRC Checksum Error Detected */
                bpdu:1,                 /* BPDU Packet */
                add_vid:1,              /* VLAN ID Added */
                do_not_change_ttl:1;    /* FP indicator for L3 unicast */

                                        /* T12.7: RX Status 0 */
        uint32  reason:26;              /* CPU opcode */

                                        /* T12.8: RX Status 2 */
        uint32  dscp:8,                 /* New DSCP */
                srcport:6,              /* Source port */
                nh_index:8,             /* Next hop index */
                match_rule:10;          /* Matched Rule */

                                        /* T12.9: RX Status 3 */
        uint32  :14,
                decap_iptunnel:1,       /* Decap IP Tunneling Packet */
                ingress_untagged:1,     /* Pkt came in untagged */
                outer_vid:12,           /* VID */
                outer_cfi:1,            /* CFI */
                outer_pri:3;            /* Priority */
#else
                                        /* T12.6: RX Status 0 */
        uint32  do_not_change_ttl:1,    /* FP indicator for L3 unicast */
                add_vid:1,              /* VLAN ID Added */
                bpdu:1,                 /* BPDU Packet */
                cell_error:1,           /* Cell CRC Checksum Error Detected */
                chg_tos:1,              /* DSCP Changed */
                cpu_cos:3,              /* CPU COS */
                cos:3,                  /* Packet Priority */
                emirror:1,              /* Egress Mirroring */
                imirror:1,              /* Ingress Mirroring */
                l3uc:1,                 /* L3 UC */
                mtp_index:7,            /* Mirror-to-Port Index */
                pkt_aged:1,             /* Pkt is Aged */
                purge_cell:1,           /* Packet is marked Purged */
                src_hg:1,               /* Source is Higig */
                switch_pkt:1,           /* Switched packet */
                regen_crc:1,            /* Regenerate CRC */
                :6;
                                        /* T12.7: RX Status 1 */
        uint32  reason:26;              /* CPU opcode */
                                        /* T12.8: RX Status 2 */
        uint32  match_rule:10,          /* Matched Rule */
                nh_index:8,             /* Next hop index */
                srcport:6,              /* Source port */
                dscp:8;                 /* New DSCP */
                                        /* T12.9: RX Status 3 */
        uint32  outer_pri:3,            /* Priority (D)*/
                outer_cfi:1,            /* CFI (D)*/
                outer_vid:12,           /* VID (D)*/
                ingress_untagged:1,     /* Pkt came in untagged (D)*/
                decap_iptunnel:1,       /* Decap IP Tunneling Packet */
                :14;
#endif

                                        /* T12.10: DMA Status 0 */
#ifdef  LE_HOST
        uint32  count:16,               /* Transferred byte count */
                end:1,                  /* End bit (RX) */
                start:1,                /* Start bit (RX) */
                error:1,                /* Cell Error (RX) */
                dc:12,                  /* Don't Care */
                done:1;                 /* Descriptor Done */
#else
        uint32  done:1,
                dc:12,
                error:1,
                start:1,
                end:1,
                count:16;
#endif
} dcb12_t;


#define GET_SOC_DMA_MANAGER(_u)   (&_myDmaManager[(_u)])

#define SOC_DCB(_u) GET_SOC_DMA_MANAGER(_u)->dcb_op

#define SOC_DCB_INIT(_u, _dcb)  SOC_DCB(_u).init(_dcb)
#define SOC_DCB_ADDTX(_u, _dv, _addr, _count, _l2pbm, _utpbm, _l3pbm, _f, _hgh) \
    SOC_DCB(_u).addtx(_dv, _addr, _count, _l2pbm, _utpbm, _l3pbm, _f, _hgh)
#define SOC_DCB_ADDRX(_u, _dv, _addr, _count, _f) \
    SOC_DCB(_u).addrx(_dv, _addr, _count, _f)
#define SOC_DCB_INTRINFO(_u, _dcb, _tx, _countp) \
    SOC_DCB(_u).intrinfo(_u, _dcb, _tx, _countp)

#define SOC_DCB_TYPE(_u)        SOC_DCB(_u).dcbtype
#define SOC_DCB_SIZE(_u)        SOC_DCB(_u).dcbsize

/* flags returned from INTRINFO */
#define SOC_DCB_INFO_DONE       0x02
#define SOC_DCB_INFO_PKTEND     0x01

#define SOC_DCB_REQCOUNT_SET(_u, _dcb, _count) \
    SOC_DCB(_u).reqcount_set(_dcb, _count)
#define SOC_DCB_REQCOUNT_GET(_u, _dcb) \
    SOC_DCB(_u).reqcount_get(_dcb)
#define SOC_DCB_XFERCOUNT_GET(_u, _dcb) \
    SOC_DCB(_u).xfercount_get(_dcb)
#define SOC_DCB_ADDR_SET(_u, _dcb, _addr) \
    SOC_DCB(_u).addr_set(_u, _dcb, _addr)
#define SOC_DCB_ADDR_GET(_u, _dcb) \
    SOC_DCB(_u).addr_get(_u, _dcb)
#define SOC_DCB_PADDR_GET(_u, _dcb) \
    SOC_DCB(_u).paddr_get(_dcb)
#define SOC_DCB_DONE_SET(_u, _dcb, _val) \
    SOC_DCB(_u).done_set(_dcb, _val)
#define SOC_DCB_DONE_GET(_u, _dcb) \
    SOC_DCB(_u).done_get(_dcb)
#define SOC_DCB_SG_SET(_u, _dcb, _val) \
    SOC_DCB(_u).sg_set(_dcb, _val)
#define SOC_DCB_SG_GET(_u, _dcb) \
    SOC_DCB(_u).sg_get(_dcb)
#define SOC_DCB_CHAIN_SET(_u, _dcb, _val) \
    SOC_DCB(_u).chain_set(_dcb, _val)
#define SOC_DCB_CHAIN_GET(_u, _dcb) \
    SOC_DCB(_u).chain_get(_dcb)
#define SOC_DCB_RELOAD_SET(_u, _dcb, _val) \
    SOC_DCB(_u).reload_set(_dcb, _val)
#define SOC_DCB_RELOAD_GET(_u, _dcb) \
    SOC_DCB(_u).reload_get(_dcb)

#define SOC_DCB_TX_L2PBM_SET(_u, _dcb, _pbm) \
    SOC_DCB(_u).tx_l2pbm_set(_dcb, _pbm)
#define SOC_DCB_TX_UTPBM_SET(_u, _dcb, _pbm) \
    SOC_DCB(_u).tx_utpbm_set(_dcb, _pbm)
#define SOC_DCB_TX_L3PBM_SET(_u, _dcb, _pbm) \
    SOC_DCB(_u).tx_l3pbm_set(_dcb, _pbm)
#define SOC_DCB_TX_CRC_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_crc_set(_dcb, _val)
#define SOC_DCB_TX_COS_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_cos_set(_dcb, _val)
#define SOC_DCB_TX_DESTMOD_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_destmod_set(_dcb, _val)
#define SOC_DCB_TX_DESTPORT_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_destport_set(_dcb, _val)
#define SOC_DCB_TX_OPCODE_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_opcode_set(_dcb, _val)
#define SOC_DCB_TX_SRCMOD_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_srcmod_set(_dcb, _val)
#define SOC_DCB_TX_SRCPORT_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_srcport_set(_dcb, _val)
#define SOC_DCB_TX_PRIO_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_prio_set(_dcb, _val)
#define SOC_DCB_TX_PFM_SET(_u, _dcb, _val) \
    SOC_DCB(_u).tx_pfm_set(_dcb, _val)

#define SOC_DCB_RX_UNTAGGED_GET(_u, _dcb) \
    SOC_DCB(_u).rx_untagged_get(_dcb)
#define SOC_DCB_RX_CRC_GET(_u, _dcb) \
    SOC_DCB(_u).rx_crc_get(_dcb)
#define SOC_DCB_RX_COS_GET(_u, _dcb) \
    SOC_DCB(_u).rx_cos_get(_dcb)
#define SOC_DCB_RX_DESTMOD_GET(_u, _dcb) \
    SOC_DCB(_u).rx_destmod_get(_dcb)
#define SOC_DCB_RX_DESTPORT_GET(_u, _dcb) \
    SOC_DCB(_u).rx_destport_get(_dcb)
#define SOC_DCB_RX_OPCODE_GET(_u, _dcb) \
    SOC_DCB(_u).rx_opcode_get(_dcb)
#define SOC_DCB_RX_CLASSTAG_GET(_u, _dcb) \
    SOC_DCB(_u).rx_classtag_get(_dcb)
#define SOC_DCB_RX_MATCHRULE_GET(_u, _dcb) \
    SOC_DCB(_u).rx_matchrule_get(_dcb)
#define SOC_DCB_RX_START_GET(_u, _dcb) \
    SOC_DCB(_u).rx_start_get(_dcb)
#define SOC_DCB_RX_END_GET(_u, _dcb) \
    SOC_DCB(_u).rx_end_get(_dcb)
#define SOC_DCB_RX_ERROR_GET(_u, _dcb) \
    SOC_DCB(_u).rx_error_get(_dcb)
#define SOC_DCB_RX_PRIO_GET(_u, _dcb) \
    SOC_DCB(_u).rx_prio_get(_dcb)
#define SOC_DCB_RX_REASON_GET(_u, _dcb) \
    SOC_DCB(_u).rx_reason_get(_dcb)
#define SOC_DCB_RX_INGPORT_GET(_u, _dcb) \
    SOC_DCB(_u).rx_ingport_get(_dcb)
#define SOC_DCB_RX_SRCPORT_GET(_u, _dcb) \
    SOC_DCB(_u).rx_srcport_get(_dcb)
#define SOC_DCB_RX_SRCMOD_GET(_u, _dcb) \
    SOC_DCB(_u).rx_srcmod_get(_dcb)
#define SOC_DCB_RX_MCAST_GET(_u, _dcb) \
    SOC_DCB(_u).rx_mcast_get(_dcb)
#define SOC_DCB_RX_VCLABEL_GET(_u, _dcb) \
    SOC_DCB(_u).rx_vclabel_get(_dcb)

#define SOC_DCB_PURGE_SET(_u, _dcb, _val) \
    SOC_DCB(_u).purge_set(_dcb, _val)
#define SOC_DCB_PURGE_GET(_u, _dcb) \
    SOC_DCB(_u).purge_get(_dcb)
#define SOC_DCB_STAT_SET(_u, _dcb, _val) \
    SOC_DCB(_u).stat_set(_dcb, _val)
#define SOC_DCB_STAT_GET(_u, _dcb) \
    SOC_DCB(_u).stat_get(_dcb)
#define SOC_DCB_HG_SET(_u, _dcb, _val) \
    SOC_DCB(_u).hg_set(_dcb, _val)
#define SOC_DCB_HG_GET(_u, _dcb) \
    SOC_DCB(_u).hg_get(_dcb)

#define SOC_DCB_IDX2PTR(_u, _dcb, _i) \
    (dcb_t *)((char *)_dcb + (SOC_DCB_SIZE(_u) * (_i)))
#define SOC_DCB_PTR2IDX(_u, _dcb1, _dcb2) \
    (int)(((char *)_dcb1 - (char *)_dcb2) / SOC_DCB_SIZE(_u))

typedef struct _soc_stat
{
    uint32 dv_alloc;
    uint32 dv_free;
    uint32 dv_alloc_q;
    uint32 intr_chain;

    uint32 dma_tbyt;
    uint32 dma_tpkt;
    uint32 dma_rbyt;
    uint32 dma_rpkt;
} soc_stat_t;

typedef struct _soc_dma_manager
{
    uint32      soc_dma_lock;          /* Lock for updating DMA operations etc. */
#define SOC_DMA_LOCK(s)   ((s)->soc_dma_lock = sal_splhi())
#define SOC_DMA_UNLOCK(s) (void)sal_spl((s)->soc_dma_lock)
    sdc_t       soc_channels[N_DMA_CHAN];
    int         soc_max_channels;      /* maximum channel count */
    sdc_t       *soc_dma_default_rx;   /* Default RX channel */
    sdc_t       *soc_dma_default_tx;   /* Default TX channel */
    int         dma_droptx;            /* Any channels in drop tx mode */
    dv_t        *soc_dv_free;          /* Available DVs */
    int         soc_dv_free_cnt;       /* # on free list */
    int         soc_dv_cnt;            /* # allowed on free list */
    int         soc_dv_size;           /* Number DCBs in free list entries */
    uint32      *tx_purge_pkt;         /* DMA able buffer for TX Purge */
    soc_stat_t  stat;
    dcb_op_t    dcb_op;                /* DCB operations */

    sal_mutex_t schanMutex;            /* S-Channel mutual exclusion */
    sal_mutex_t miimMutex;
} soc_dma_manager_t;

extern soc_dma_manager_t _myDmaManager[];

typedef union soc_higig_hdr_u {
    struct {
        uint8   bytes[12];
    } overlay0;

#if defined(LE_HOST)

    struct {                           /* Byte # */
        uint32  start:8;               /* 0 */
        uint32  hgi:8;                 /* 1 */
        uint32  vlan_id_hi:4;          /* 2 */
        uint32  vlan_cfi:1;
        uint32  vlan_pri:3;
        uint32  vlan_id_lo:8;          /* 3 */
        uint32  opcode:3;              /* 4 */
        uint32  src_mod:5;
        uint32  src_port:6;            /* 5 */
        uint32  pfm:2;
        uint32  cos:3;                 /* 6 */
        uint32  dst_port:5;
        uint32  dst_mod:5;             /* 7 */
        uint32  cng:1;
        uint32  hdr_format:2;
        uint32  mirror:1;              /* 8 */
        uint32  mirror_done:1;
        uint32  mirror_only:1;
        uint32  ingress_tagged:1;
        uint32  dst_tgid:3;
        uint32  dst_t:1;
        uint32  _rsvd1:8;              /* 9 */
        uint32  _rsvd2:8;              /* 10 */
        uint32  _rsvd3:8;              /* 11 */
    } overlay1;

    struct {                           /* Byte # */
        uint32  _rsvd1:8;              /* 0 */
        uint32  _rsvd2:8;              /* 1 */
        uint32  _rsvd3:8;              /* 2 */
        uint32  _rsvd4:8;              /* 3 */
        uint32  _rsvd5:8;              /* 4 */
        uint32  tgid:6;                /* 5 */
        uint32  _rsvd6:2;
        uint32  _rsvd7:3;              /* 6 */
        uint32  l2mc_ptr_lo:5;
        uint32  l2mc_ptr_hi:5;         /* 7 */
        uint32  _rsvd8:3;
        uint32  ctag_hi:8;             /* 8 */
        uint32  ctag_lo:8;             /* 9 */
        uint32  _rsvd9:8;              /* 10 */
        uint32  _rsvd10:8;             /* 11 */
    } overlay2;

    struct {                           /* Byte # */
        uint32  start:8;               /* 0 */
        uint32  dst_mod_6:1;           /* 1 */
        uint32  src_mod_6:1;
        uint32  hdr_ext_len:3;
        uint32  _rsvd1:1;
        uint32  hgi:2;
        uint32  vlan_id_hi:4;          /* 2 */
        uint32  vlan_cfi:1;
        uint32  vlan_pri:3;
        uint32  vlan_id_lo:8;          /* 3 */
        uint32  opcode:3;              /* 4 */
        uint32  src_mod:5;
        uint32  src_port:6;            /* 5 */
        uint32  pfm:2;
        uint32  cos:3;                 /* 6 */
        uint32  dst_port:5;
        uint32  dst_mod:5;             /* 7 */
        uint32  cng:1;
        uint32  hdr_format:2;
        uint32  mirror:1;              /* 8 */
        uint32  mirror_done:1;
        uint32  mirror_only:1;
        uint32  ingress_tagged:1;
        uint32  dst_tgid:3;
        uint32  dst_t:1;
        uint32  vc_label_19_16:4;      /* 9 */
        uint32  label_present:1;
        uint32  l3:1;
        uint32  dst_mod_5:1;
        uint32  src_mod_5:1;
        uint32  vc_label_15_8:8;       /* 10 */
        uint32  vc_label_7_0:8;        /* 11 */
    } hgp_overlay1;

#else /* !LE_HOST */

    struct {                           /* Byte # */
        uint32  start:8;               /* 0 */
        uint32  hgi:8;                 /* 1 */
        uint32  vlan_pri:3;            /* 2 */
        uint32  vlan_cfi:1;
        uint32  vlan_id_hi:4;
        uint32  vlan_id_lo:8;          /* 3 */
        uint32  src_mod:5;             /* 4 */
        uint32  opcode:3;
        uint32  pfm:2;                 /* 5 */
        uint32  src_port:6;
        uint32  dst_port:5;            /* 6 */
        uint32  cos:3;
        uint32  hdr_format:2;          /* 7 */
        uint32  cng:1;
        uint32  dst_mod:5;
        uint32  dst_t:1;               /* 8 */
        uint32  dst_tgid:3;
        uint32  ingress_tagged:1;
        uint32  mirror_only:1;
        uint32  mirror_done:1;
        uint32  mirror:1;
        uint32  _rsvd1:8;              /* 9 */
        uint32  _rsvd2:8;              /* 10 */
        uint32  _rsvd3:8;              /* 11 */
    } overlay1;

    struct {                           /* Byte # */
        uint32  _rsvd1:8;              /* 0 */
        uint32  _rsvd2:8;              /* 1 */
        uint32  _rsvd3:8;              /* 2 */
        uint32  _rsvd4:8;              /* 3 */
        uint32  _rsvd5:8;              /* 4 */
        uint32  _rsvd6:2;              /* 5 */
        uint32  tgid:6;
        uint32  l2mc_ptr_lo:5;         /* 6 */
        uint32  _rsvd7:3;
        uint32  _rsvd8:3;              /* 7 */
        uint32  l2mc_ptr_hi:5;
        uint32  ctag_hi:8;             /* 8 */
        uint32  ctag_lo:8;             /* 9 */
        uint32  _rsvd9:8;              /* 10 */
        uint32  _rsvd10:8;             /* 11 */
    } overlay2;

    struct {                           /* Byte # */
        uint32  start:8;               /* 0 */
        uint32  hgi:2;                 /* 1 */
        uint32  _rsvd1:1;
        uint32  hdr_ext_len:3;
        uint32  src_mod_6:1;
        uint32  dst_mod_6:1;
        uint32  vlan_pri:3;            /* 2 */
        uint32  vlan_cfi:1;
        uint32  vlan_id_hi:4;
        uint32  vlan_id_lo:8;          /* 3 */
        uint32  src_mod:5;             /* 4 */
        uint32  opcode:3;
        uint32  pfm:2;                 /* 5 */
        uint32  src_port:6;
        uint32  dst_port:5;            /* 6 */
        uint32  cos:3;
        uint32  hdr_format:2;          /* 7 */
        uint32  cng:1;
        uint32  dst_mod:5;
        uint32  dst_t:1;               /* 8 */
        uint32  dst_tgid:3;
        uint32  ingress_tagged:1;
        uint32  mirror_only:1;
        uint32  mirror_done:1;
        uint32  mirror:1;
        uint32  src_mod_5:1;           /* 9 */
        uint32  dst_mod_5:1;
        uint32  l3:1;
        uint32  label_present:1;
        uint32  vc_label_19_16:4;
        uint32  vc_label_15_8:8;       /* 10 */
        uint32  vc_label_7_0:8;        /* 11 */
    } hgp_overlay1;
#endif   /* !LE_HOST */
} soc_higig_hdr_t;

#define SOC_HIGIG2_START        0xfc /* Higig2 Module Header */

typedef union soc_higig2_hdr_u {
    struct {
        uint8   bytes[16];
    } overlay0;

#if defined(LE_HOST)

    struct {                            /* Byte # */
        /* "Legacy" PPD Overlay 1 */
        uint32  start:8;                /* 0 */
        uint32  tc:4;                   /* 1 */
        uint32  mcst:1;
        uint32  _rsvd1:3;
        uint32  dst_mod:8;              /* 2 */
        uint32  dst_port:8;             /* 3 */
        uint32  src_mod:8;              /* 4 */
        uint32  src_port:8;             /* 5 */
        uint32  lbid:8;                 /* 6 */
        uint32  ppd_type:3;             /* 7 */
        uint32  _rsvd2:3;
        uint32  dp:2;
        uint32  mirror:1;               /* 8 */
        uint32  mirror_done:1;
        uint32  mirror_only:1;
        uint32  ingress_tagged:1;
        uint32  dst_tgid:3;
        uint32  dst_t:1;
        uint32  vc_label_19_16:4;       /* 9 */
        uint32  label_present:1;
        uint32  l3:1;
        uint32  _rsvd3:2;
        uint32  vc_label_15_8:8;        /* 10 */
        uint32  vc_label_7_0:8;         /* 11 */
        uint32  vlan_id_hi:4;           /* 12 */
        uint32  vlan_cfi:1;
        uint32  vlan_pri:3;
        uint32  vlan_id_lo:8;           /* 13 */
        uint32  opcode:3;               /* 14 */
        uint32  _rsvd4:2;
        uint32  src_t:1;
        uint32  pfm:2;
        uint32  _rsvd5:5;               /* 15 */
        uint32  hdr_ext_len:3;
   } ppd_overlay1;

    struct {                            /* Byte # */
        /* "Legacy" PPD Overlay 2 */
        uint32  start:8;                /* 0 */
        uint32  tc:4;                   /* 1 */
        uint32  mcst:1;
        uint32  _rsvd1:3;
        uint32  dst_mod:8;              /* 2 */
        uint32  dst_port:8;             /* 3 */
        uint32  src_mod:8;              /* 4 */
        uint32  src_port:8;             /* 5 */
        uint32  lbid:8;                 /* 6 */
        uint32  ppd_type:3;             /* 7 */
        uint32  _rsvd2:3;
        uint32  dp:2;
        uint32  ctag_hi:8;              /* 8 */
        uint32  ctag_lo:8;              /* 9 */
        uint32  _rsvd3:8;               /* 10 */
        uint32  _rsvd4:8;               /* 11 */
        uint32  vlan_id_hi:4;           /* 12 */
        uint32  vlan_cfi:1;
        uint32  vlan_pri:3;
        uint32  vlan_id_lo:8;           /* 13 */
        uint32  opcode:3;               /* 14 */
        uint32  _rsvd5:2;
        uint32  src_t:1;
        uint32  pfm:2;
        uint32  _rsvd6:5;               /* 15 */
        uint32  hdr_ext_len:3;
    } ppd_overlay2;

    struct {                            /* Byte # */
        /* "Next Gen" PPD Overlay 3 */
        uint32  start:8;                /* 0 */
        uint32  tc:4;                   /* 1 */
        uint32  mcst:1;
        uint32  _rsvd1:3;
        uint32  dst_mod:8;              /* 2 */
        uint32  dst_port:8;             /* 3 */
        uint32  src_mod:8;              /* 4 */
        uint32  src_port:8;             /* 5 */
        uint32  lbid:8;                 /* 6 */
        uint32  ppd_type:3;             /* 7 */
        uint32  _rsvd2:3;
        uint32  dp:2;
        uint32  _rsvd3:3;               /* 8 */
        uint32  lrne:1;
        uint32  opcode:4;
        uint32  _rsvd4:8;               /* 9 */
        uint32  vdst_pid:8;             /* 10 */
        uint32  vsrc_pid:8;             /* 11 */
        uint32  vs_id_31_24:8;          /* 12 */
        uint32  vs_id_23_16:8;          /* 13 */
        uint32  vs_id_15_8:8;           /* 14 */
        uint32  vs_id_7_0:8;            /* 15 */

    } ppd_overlay3;

#else /* !LE_HOST */

    struct {                            /* Byte # */
        /* "Legacy" PPD Overlay 1 */
        uint32  start:8;                /* 0 */
        uint32  _rsvd1:3;               /* 1 */
        uint32  mcst:1;
        uint32  tc:4;
        uint32  dst_mod:8;              /* 2 */
        uint32  dst_port:8;             /* 3 */
        uint32  src_mod:8;              /* 4 */
        uint32  src_port:8;             /* 5 */
        uint32  lbid:8;                 /* 6 */
        uint32  dp:2;                   /* 7 */
        uint32  _rsvd2:3;
        uint32  ppd_type:3;
        uint32  dst_t:1;                /* 8 */
        uint32  dst_tgid:3;
        uint32  ingress_tagged:1;
        uint32  mirror_only:1;
        uint32  mirror_done:1;
        uint32  mirror:1;
        uint32  _rsvd3:2;               /* 9 */
        uint32  l3:1;
        uint32  label_present:1;
        uint32  vc_label_19_16:4;
        uint32  vc_label_15_8:8;        /* 10 */
        uint32  vc_label_7_0:8;         /* 11 */
        uint32  vlan_pri:3;             /* 12 */
        uint32  vlan_cfi:1;
        uint32  vlan_id_hi:4;
        uint32  vlan_id_lo:8;           /* 13 */
        uint32  pfm:2;                  /* 14 */
        uint32  src_t:1;
        uint32  _rsvd4:2;
        uint32  opcode:3;
        uint32  hdr_ext_len:3;          /* 15 */
        uint32  _rsvd5:5;
   } ppd_overlay1;

    struct {                            /* Byte # */
        /* "Legacy" PPD Overlay 2 */
        uint32  start:8;                /* 0 */
        uint32  _rsvd1:3;               /* 1 */
        uint32  mcst:1;
        uint32  tc:4;
        uint32  dst_mod:8;              /* 2 */
        uint32  dst_port:8;             /* 3 */
        uint32  src_mod:8;              /* 4 */
        uint32  src_port:8;             /* 5 */
        uint32  lbid:8;                 /* 6 */
        uint32  dp:2;                   /* 7 */
        uint32  _rsvd2:3;
        uint32  ppd_type:3;
        uint32  ctag_hi:8;              /* 8 */
        uint32  ctag_lo:8;              /* 9 */
        uint32  _rsvd3:8;               /* 10 */
        uint32  _rsvd4:8;               /* 11 */
        uint32  vlan_pri:3;             /* 12 */
        uint32  vlan_cfi:1;
        uint32  vlan_id_hi:4;
        uint32  vlan_id_lo:8;           /* 13 */
        uint32  pfm:2;                  /* 14 */
        uint32  src_t:1;
        uint32  _rsvd5:2;
        uint32  opcode:3;
        uint32  hdr_ext_len:3;          /* 15 */
        uint32  _rsvd6:5;
    } ppd_overlay2;

    struct {                            /* Byte # */
        /* "Next Gen" PPD Overlay 3 */
        uint32  start:8;                /* 0 */
        uint32  _rsvd1:3;               /* 1 */
        uint32  mcst:1;
        uint32  tc:4;
        uint32  dst_mod:8;              /* 2 */
        uint32  dst_port:8;             /* 3 */
        uint32  src_mod:8;              /* 4 */
        uint32  src_port:8;             /* 5 */
        uint32  lbid:8;                 /* 6 */
        uint32  dp:2;                   /* 7 */
        uint32  _rsvd2:3;
        uint32  ppd_type:3;
        uint32  opcode:4;               /* 8 */
        uint32  lrne:1;
        uint32  _rsvd3:3;
        uint32  _rsvd4:8;               /* 9 */
        uint32  vdst_pid:8;             /* 10 */
        uint32  vsrc_pid:8;             /* 11 */
        uint32  vs_id_31_24:8;          /* 12 */
        uint32  vs_id_23_16:8;          /* 13 */
        uint32  vs_id_15_8:8;           /* 14 */
        uint32  vs_id_7_0:8;            /* 15 */

    } ppd_overlay3;

#endif /* !LE_HOST */

} soc_higig2_hdr_t;

/************** packet queue ******************/

#define RXQ_MAX    (1 << 7)             /* 128 */
#define RXQ_MASK   (RXQ_MAX - 1)

typedef struct netdrv_pkt_s {
    int   unit;
    void *data;                         /* pointer to MAC header */
    int   len;                          /* Length of raw packet data */
} netdrv_pkt_t;

typedef struct rxq_ctrl_s {
    netdrv_pkt_t *rxq_data;
    int     lock;
    int     head;
    int     tail;
    uint32  count;
    uint32  dropped;
} rxq_ctrl_t;

#define RXQ_EMPTY(rxq)  ((rxq)->head == (rxq)->tail)
#define RXQ_FULL(rxq)   ((((rxq)->tail + 1) & RXQ_MASK) == (rxq)->head)
#define RXQ_LOCK(rxq)   (rxq)->lock = sal_splhi()
#define RXQ_UNLOCK(rxq) sal_spl((rxq)->lock)

/*Don't lock -- operation in interrupt context*/
#define RXQ_ENQUEUE(rxq, _pPkt) \
    do \
    { \
        if (!RXQ_FULL(rxq)) \
        { \
            (rxq)->rxq_data[(rxq)->tail].unit = (_pPkt)->unit; \
            (rxq)->rxq_data[(rxq)->tail].data = (_pPkt)->_pkt_data.data; \
            (rxq)->rxq_data[(rxq)->tail].len  = (_pPkt)->tot_len; \
            (rxq)->tail++; \
            (rxq)->tail &= RXQ_MASK; \
            (rxq)->count++; \
        } \
        else \
        { \
            (rxq)->dropped++; \
        } \
    } while (0);

#define RXQ_DEQUEUE(rxq, _pPkt) \
    do \
    { \
        RXQ_LOCK(rxq); \
        if (!RXQ_EMPTY(rxq)) \
        { \
            (_pPkt)->unit = ((rxq)->rxq_data[(rxq)->head].unit); \
            (_pPkt)->_pkt_data.data = ((rxq)->rxq_data[(rxq)->head].data); \
            (_pPkt)->tot_len = ((rxq)->rxq_data[(rxq)->head].len); \
            (rxq)->head++; \
            (rxq)->head &= RXQ_MASK; \
            (rxq)->count--; \
        } \
        RXQ_UNLOCK(rxq); \
    } while (0);

/************************************************************************/

/* Single RX control structure, common for all units */
struct _rxctrl {
    /* Thread control */
    sal_sem_t         pkt_notify;             /* Semaphore for pkt thrd */
    volatile int      pkt_notify_given;       /* Semaphore already given */
    sal_thread_t      rx_tid;                 /* packet thread id */
    int               sleep_cur;              /* Current sleep time */
#define MIN_SLEEP_US 5000                     /* Always sleep at least 5 ms */
#define MAX_SLEEP_US 100000                   /* For non-rate limiting, .1 sec */
    volatile int      thread_running;         /* Input signal to thread */
    volatile int      thread_exit_complete;   /* Output signal from thread */
    volatile int      thread_pri;             /* Thread priority */
};

dv_t * soc_dma_dv_alloc(int unit, dvt_t op, int cnt);
void soc_dma_dv_free(int unit, dv_t *dv);

void soc_dma_desc_end_packet(dv_t *dv);

int soc_dma_start(int unit, dma_chan_t channel, dv_t *dv_chain);

#endif
