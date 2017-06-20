
/*
 * $Id: ethdma.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        soc/ethdma.h
 * Purpose:     
 */

#ifndef _SOC_ETHDMA_H
#define _SOC_ETHDMA_H

#include <soc/types.h>
#include <soc/rx.h>
#include <soc/dma.h>


#ifdef INCLUDE_KNET

#define KNET_ETH_DCB_APITX_DONE     0x1
#define KNET_ETH_DCB_ETHTX_DONE     0x2
#define KNET_ETH_DCB_TX_ERR         0x4


#define KNET_ETH_DCB_APIRX_DONE     0x10
#define KNET_ETH_DCB_ETHRX_DONE     0x20
#define KNET_ETH_DCB_RX_ERR         0x40
#define KNET_ETH_DCB_RX_FILL        0x80


#endif


/*
 * Typedef: Ethernet DMA Control Block (dcb)
 * Purpose: Keep the control information of Tx/Rx frames
 */

typedef struct eth_dcb_s {
    struct eth_dcb_s *next;         /* ptr to next robo_dcb_t in a packet */
    struct eth_dcb_s *next_paddr;   /* ptr to next physical robo_dcb_t in a packet */
    uint16       len;               /* length of this buffer */
    sal_vaddr_t dcb_vaddr;		    /* 32-bit virtual address */
    sal_paddr_t dcb_paddr;          /* 32-bit physical address */
    uint32   desc_seqno;
    uint32  desc_status;
} eth_dcb_t;



/****************************************************************
 * BROADCOM tag for ROBO CHIPS
 ****************************************************************/
typedef union {
    uint32  brcm_val;   /* Raw 32-bit value */
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
        _cnt:14,    /* frame octet count */
        r0:9,     /* reserved */
        _src_portid:6;  /* source port id */
#else /* LE_HOST */
    uint32  _src_portid:6,  /* source port id */
        r0:9,     /* reserved */
        _cnt:14,    /* frame octet count */
            _op:3;      /* opcode of this frame */
#endif 
    } brcm_ucast;
/* define for common usage, before knowing what real type is */
#define    brcm_op              brcm_ucast._op
#define    brcm_cnt             brcm_ucast._cnt
#define    brcm_src_portid      brcm_ucast._src_portid
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
            _cnt:14,    /* frame octet count */
       r0:10,        /* reserved */
        _src_portid:5;  /* source port id */
#else /* LE_HOST */
    uint32  _src_portid:5,  /* source port id */
       r0:10,        /* reserved */
        _cnt:14,    /* frame octet count */
            _op:3;      /* opcode of this frame */
#endif
    } brcm_mcast;
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
       r0:24,        /* reserved */
        _dst_portid:5;  /* destination port id */
#else /* LE_HOST */
    uint32  _dst_portid:5,  /* destination port id */
       r0:24,        /* reserved */
            _op:3;      /* opcode of this frame */
#endif
    } brcm_egr_dir;
#define    brcm_dst_portid      brcm_egr_dir._dst_portid
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
       r0:24,        /* reserved */
        _src_portid:5;  /* source port id */
#else /* LE_HOST */
    uint32  _src_portid:5,  /* source port id */
       r0:24,        /* reserved */
            _op:3;      /* opcode of this frame */
#endif
    } brcm_igr_dir;
/* bcm5348/5347 */
      struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
        r0:2,     /* reserved */
        _dst_pbmp:27;   /* destination pbmp */
#else /* LE_HOST */
    uint32  _dst_pbmp:27,   /* destination pbmp */
        r0:2,     /* reserved */
            _op:3;      /* opcode of this frame */
#endif
    } brcm_multi_port_lo;
/* bcm5348/5347 */
      struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
        _dst_pbmp:29;   /* destination pbmp */
#else /* LE_HOST */
    uint32  _dst_pbmp:29,   /* destination pbmp */
            _op:3;      /* opcode of this frame */
#endif
    } brcm_multi_port_hi;
#define    brcm_dst_pbmp      brcm_multi_port_hi._dst_pbmp
/* for bcm5395 */
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
           _tc:3,                   /* traffic class */
           _te:2,                   /* tag enforement */
           _ts:1,                   /* time stamp request */
        _dst_pbmp:23;   /* destination pbmp */
#else /* LE_HOST */
    uint32  _dst_pbmp:23,   /* destination pbmp */
            _ts:1,            /* time stamp request */
            _te:2,            /* tag enforement */
            _tc:3,            /* traffic class */
            _op:3;      /* opcode of this frame */
#endif
    } brcm_multi_port_tag_stamp;
#define    brcm_tc  brcm_multi_port_tag_stamp._tc
#define    brcm_te  brcm_multi_port_tag_stamp._te
#define    brcm_ts  brcm_multi_port_tag_stamp._ts

/* for bcm5395 IMP egress packet transfer*/
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
        _cnt:14,    /* frame octet count */
        _reason:7, /* reason code */
        _tc:3, /* traffic class */
        _src_portid:5;  /* source port id */
#else /* LE_HOST */
    uint32  _src_portid:5,  /* source port id */
        _tc:3, /* traffic class */
        _reason:7, /* reason code */
        _cnt:14,    /* frame octet count */
            _op:3;      /* opcode of this frame */
#endif 
    } brcm_5395_imp_egress_tag;
/* define for common usage, before knowing what real type is */
#define    brcm_5395_src_portid              brcm_5395_imp_egress_tag._src_portid
#define    brcm_5395_reason              brcm_5395_imp_egress_tag._reason
#define    brcm_5395_tc             brcm_5395_imp_egress_tag._tc


/* for bcm53242 */
      struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
              _tq:2,                    /* traffic queue */
              _te:2,                    /* tag enforement */
        _dst_pbmp:25;   /* destination pbmp */
#else /* LE_HOST */
    uint32  _dst_pbmp:25,   /* destination pbmp */
               _te:2,                   /* tag enforement */
               _tq:2,                   /* traffic queue */
            _op:3;      /* opcode of this frame */
#endif
    } brcm_tq_te_bmp;
#define     brcm_te_53242   brcm_tq_te_bmp._te
#define     brcm_tq_53242   brcm_tq_te_bmp._tq
/* for bcm53242 IMP egress packet transfer*/
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
        _cnt:12,    /* frame octet count */
        r0:1,     /* reserved */
        _cos:2, /* COS queue */
        _reason:6, /* reason code */
        _ermon:2,
        _src_portid:6;  /* source port id */
#else /* LE_HOST */
    uint32  _src_portid:6,  /* source port id */
        _ermon:2,
        _reason:6, /* reason code */
        _cos:2, /* COS queue */
        r0:1,     /* reserved */
        _cnt:12,    /* frame octet count */
            _op:3;      /* opcode of this frame */
#endif 
    } brcm_53242_imp_egress_tag;
/* define for common usage, before knowing what real type is */
#define    brcm_53242_src_portid             brcm_53242_imp_egress_tag._src_portid
#define    brcm_53242_reason              brcm_53242_imp_egress_tag._reason
#define    brcm_53242_cosq             brcm_53242_imp_egress_tag._cos

/* for bcm53115 */
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
           _tc:3,                   /* traffic class */
           _te:2,                   /* tag enforement */
           _ts:1,                   /* time stamp request */
           r0:16,                       /* reserved */
           _dst_pbmp:7;   /* destination pbmp */
#else /* LE_HOST */
    uint32  _dst_pbmp:7,   /* destination pbmp */
           r0:16,                       /* reserved */
           _ts:1,            /* time stamp request */
           _te:2,            /* tag enforement */
           _tc:3,            /* traffic class */
           _op:3;      /* opcode of this frame */
#endif
    } brcm_53115_multi_port_tag_stamp;
#define    brcm_tc_53115  brcm_53115_multi_port_tag_stamp._tc
#define    brcm_te_53115  brcm_53115_multi_port_tag_stamp._te
#define    brcm_ts_53115  brcm_53115_multi_port_tag_stamp._ts

/* for bcm53115 IMP egress packet transfer*/
    struct {
#ifdef  BE_HOST
    uint32  _op:3,      /* opcode of this frame */
        r0:5,     /* reserved */
        _classification_id:8,    /* classification id */
        _reason:8, /* reason code */
        _tc:3, /* traffic class */
        _src_portid:5;  /* source port id */
#else /* LE_HOST */
    uint32  _src_portid:5,  /* source port id */
        _tc:3, /* traffic class */
        _reason:8, /* reason code */
        _classification_id:8,    /* classification id */
        r0:5,     /* reserved */
            _op:3;      /* opcode of this frame */
#endif 
    } brcm_53115_imp_egress_tag;
/* define for common usage, before knowing what real type is */
#define    brcm_53115_src_portid              brcm_53115_imp_egress_tag._src_portid
#define    brcm_53115_reason              brcm_53115_imp_egress_tag._reason
#define    brcm_53115_tc             brcm_53115_imp_egress_tag._tc

/* for bcm53280 */
    struct {
#ifdef  BE_HOST
    uint32  _op:4,               /* opcode of this frame */
            _tc:4,               /* traffic class */
            _dp:2,               /* drop precedence */
            _vid:12,             /* vlan id */
            _src_vpid:4,         /* ingress virtual port id */
            r0:1,                  /* reserved */
            _src_pid:5;          /* ingress physical port id */
#else /* LE_HOST */
    uint32  _src_pid:5,          /* ingress physical port id */
            r0:1,                  /* reserved */
            _src_vpid:4,         /* ingress virtual port id */
            _vid:12,             /* vlan id */
            _dp:2,               /* drop precedence */
            _tc:4,               /* traffic class */
            _op:4;               /* opcode of this frame */
#endif
    } brcm_53280_imp_egress_tag_hi;
#define    brcm_opcode_53280  brcm_53280_imp_egress_tag_hi._op
#define    brcm_tc_53280  brcm_53280_imp_egress_tag_hi._tc
#define    brcm_dp_53280  brcm_53280_imp_egress_tag_hi._dp
#define    brcm_egr_vid_53280  brcm_53280_imp_egress_tag_hi._vid
#define    brcm_vpid_53280  brcm_53280_imp_egress_tag_hi._src_vpid
#define    brcm_pid_53280  brcm_53280_imp_egress_tag_hi._src_pid

    struct {
#ifdef  BE_HOST
    uint32  _reason:16,          /* reason code */
            _learn_disable:1,    /* learn disable */
            r0:2,                  /* reserved */
            _fm_select:1,        /* flow id/mgid selection */
            _fm_id:12;           /* flow id/mgid */
#else /* LE_HOST */
    uint32  _fm_id:12,           /* flow id/mgid */
            _fm_select:1,        /* flow id/mgid selection */
            r0:2,                  /* reserved */
            _learn_disable:1,    /* learn disable */
            _reason:16;          /* reason code */
#endif
    } brcm_53280_imp_egress_tag_lo;
#define    brcm_reason_53280  brcm_53280_imp_egress_tag_lo._reason
#define    brcm_lrn_dis_53280  brcm_53280_imp_egress_tag_lo._learn_disable
#define    brcm_fm_sel_53280  brcm_53280_imp_egress_tag_lo._fm_select
#define    brcm_fm_id_53280  brcm_53280_imp_egress_tag_lo._fm_id

    struct {
#ifdef  BE_HOST
    uint32  _op:4,                  /* opcode of this frame */
            _tc:4,                  /* traffic class */
            _dp:2,                  /* drop precedence */
            _filter:8,              /* filter bypass */
            r0:1,                     /* reserved */
            _dst_id:13;             /* combined destination id */
#else /* LE_HOST */
    uint32  _dst_id:13,             /* combined destination id */
            r0:1,                     /* reserved */
            _filter:8,              /* filter bypass */
            _dp:2,                  /* drop precedence */
            _tc:4,                  /* traffic class */
            _op:4;                  /* opcode of this frame */
#endif
    } brcm_53280_imp_ingress_tag_hi;
#define    brcm_filter_53280  brcm_53280_imp_ingress_tag_hi._filter
#define    brcm_dstid_53280  brcm_53280_imp_ingress_tag_hi._dst_id

    struct {
#ifdef  BE_HOST
    uint32  r0:1,                   /* reserved */
            _dnm:1,             /* Do not modify control(B0 only feature) */
            r1:6,                   /* reserved */
            _vid:12,              /* vlan id */
            _flow_id:12;          /* flow id */
#else /* LE_HOST */
    uint32  _flow_id:12,          /* flow id */
            _vid:12,              /* vlan id */
            r0:6,                   /* reserved */
            _dnm:1,             /* Do not modify control(B0 only feature) */
            r1:1;                   /* reserved */
#endif
    } brcm_53280_imp_ingress_tag_lo;
#define    brcm_ing_vid_53280  brcm_53280_imp_ingress_tag_lo._vid
#define    brcm_flow_id_53280  brcm_53280_imp_ingress_tag_lo._flow_id
#define    brcm_dnm_53280  brcm_53280_imp_ingress_tag_lo._dnm

} brcm_t;           /* Control bit definition */


/*
 * CPU Opcodes
 */

#define BRCM_OP_UCAST   0x00        /* Normal unicast frame */
#define BRCM_OP_MCAST   0x01        /* Normal multicast frame */
#define BRCM_OP_EGR_DIR 0x02        /* Egress directed frame */
#define BRCM_OP_IGR_IDR 0x03        /* Ingress directed frame */
#define   BRCM_OP_MULTI_PORT    0x3     /* Multicast Egress direct for bcm5324 */
#define   BRCM_OP_MULTI_PORT_24_52    0x4     /* Multicast Egress direct for bcm5348 */
#define   BRCM_OP_MULTI_PORT_LS    0x4     /* Multicast Egress direct for bcm53242 */
#define   BRCM_OP_MULTI_PORT_IMP_HS    0x5     /* Multicast Egress direct for bcm53242 */
#define BRCM_OP_EGR_TS          0x1     /* IMP Egress packets with time stamp*/
#define BRCM_OP_UCAST_53280   0xf        /* Normal unicast frame for bcm53280 */
#define BRCM_OP_EGR_DIR_53280 0x1        /* Egress directed frame for bcm53280 */

/*
  * Filter types to be bypassed
  */
#define SOC_FILTER_BYPASS_LAG           0x01 /* Bypass LAG resolution filter */
#define SOC_FILTER_BYPASS_TAGGED        0x02 /* Bypass Tag frame filter */
#define SOC_FILTER_BYPASS_PORT_MASK     0x04 /* Bypass Port mask filter */
#define SOC_FILTER_BYPASS_STP           0x08 /* Bypass STP filter */
#define SOC_FILTER_BYPASS_EAP           0x10 /* Bypass EAP filter */
#define SOC_FILTER_BYPASS_INGRESS_VLAN  0x20 /* Bypass Ingress vlan filter */
#define SOC_FILTER_BYPASS_EGRESS_VLAN   0x40 /* Bypass Egress vlan filter */
#define SOC_FILTER_BYPASS_SA            0x80 /* Bypass Source address  filter */
#define SOC_FILTER_BYPASS_ALL           0xff /* Bypass all filter types */

/*
 * Typedef:
 *  eth_dv_t
 * Purpose:
 *  Maps out the I/O Vec structure used to pass DMA chains to the
 *  the SOC DMA routines "DMA I/O Vector".
 * Notes:
 *  To start a DMA request, the caller must fill in:
 *              dv_op: operation to perform (DV_RX or DV_TX).
 *              dv_done_chain: NULL if no callout for chain done, or the
 *                      address of the routine to call when chain done
 *                      is seen. It is called with 2 parameters, the
 *                      unit # and a pointer to the DV chain done has been
 *                      seen on.
 *              dv_done_desc: NULL for synchronous call, or the address of
 *                      the function to call on descriptor done. The function
 *                      is with 3 parameters, the unit #, a pointer to
 *                      the DV structure, and a pointer to the DCB completed.
 *                      One call is made for EVERY DCB, and only if the
 *                      DCB is DONE.
 *              dv_done_packet: NULL if no callout for packet done, or the
 *                      address of the routine to call when packet done
 *                      is seen. It has the same prototype as dv_done_desc.
 *              dv_public1 - 4: Not used by DMA routines,
 *                      for use by caller.
 *
 *     Scatter/gather is implemented through multiple DCBs.
 *
 *     Chains of packets can be associated with a single DV.  This
 *     keeps the HW busy DMA'ing packets even as interrupts are
 *     processed.  DVs can be chained (a software construction)
 *     which will start a new DV from this file rather than calling
 *     back.  This is not done much in our code.
 */

typedef struct eth_dv_s {
    struct eth_dv_s *dv_next;               /* Queue pointers if required */
    int         dv_unit;                /* Unit dv is allocated on */
    uint32      dv_magic;               /* Used to indicate valid */
    dvt_t       dv_op;                  /* Operation to be performed */
    dma_chan_t  dv_channel;             /* Channel queued on */
    int         dv_flags;               /* Flags for operation */
    int16       dv_length;              /* Total actual length of dv,
                       including brcm tag */
    int16       dv_cnt;                 /* # descriptors allocated */
    int16       dv_vcnt;                /* # descriptors valid */
    int16       dv_dcnt;                /* # descriptors done */
    brcm_t      dv_tag;                 /* DMA control word */
    brcm_t      dv_tag2;                 /* DMA control word extension */
    void        (*dv_done_chain)(int u, struct eth_dv_s *dv_chain);
    void        (*dv_done_desc)(int u, struct eth_dv_s *dv, eth_dcb_t *dcb);
    void        (*dv_done_packet)(int u, struct eth_dv_s*dv, eth_dcb_t *dcb);
    any_t       dv_public1;             /* For caller */
    any_t       dv_public2;             /* For caller */
    any_t       dv_public3;             /* For caller */
    any_t       dv_public4;             /* For caller */
    sal_vaddr_t dv_dmabufhdr;           /* DA SA BRCM_type BRCM_tag */
    uint32      rx_timestamp;  /* Time stamp of time sync protocol */
#define SOC_ROBO_DV_DEST_MAC(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[0]))
#define SOC_ROBO_DV_DEST_MAC_53280(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[8]))
#define SOC_ROBO_DV_DEST_MAC_53020(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[4]))
#define SOC_ROBO_DV_SRC_MAC(dv)              (&(((uint8 *)(dv)->dv_dmabufhdr)[6]))
#define SOC_ROBO_DV_SRC_MAC_53280(dv)              (&(((uint8 *)(dv)->dv_dmabufhdr)[14]))
#define SOC_ROBO_DV_SRC_MAC_53020(dv)              (&(((uint8 *)(dv)->dv_dmabufhdr)[10]))
#define SOC_ROBO_DV_BRCM_TAG(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[12]))
#define SOC_ROBO_DV_BRCM_TAG_53280(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[0]))
#define SOC_ROBO_DV_VLAN_TAG(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[18]))
/* 4 bytes BRCM TAG for BCM53115 */
#define SOC_ROBO_DV_VLAN_TAG_53115(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[16]))
#define SOC_ROBO_DV_VLAN_TAG_53280(dv)             (&(((uint8 *)(dv)->dv_dmabufhdr)[20]))

    sal_vaddr_t dv_dmabufcrc;           /* inner_FCS outer_FCS */
#define SOC_DMA_TX_HDR    22            /* 2*MAC_ADDR(6) + BRCM TAG(2+4) +
                     * VLAN TAG(4, if need)
                     */
#define SOC_DMA_TX_HDR_53115    20            /* 2*MAC_ADDR(6) + BRCM TAG(4) +
                     * VLAN TAG(4, if need)
                     */
#define SOC_DMA_TX_HDR_53280    24            /* BRCM TAG(8) + 2*MAC_ADDR(6) + 
                     * VLAN TAG(4, if need)
                     */
#define SOC_DMA_TX_CRC    8             /* Two CRC append */
    eth_dcb_t       *dv_dcb;                /* linked list head of dcbs */
    uint32  dv_seq_no;
} eth_dv_t;

#define dv_brcm_tag          dv_tag.brcm_val
#define dv_brcm_tag2         dv_tag2.brcm_val

/* define for common usage, before knowing what real type is */
#define dv_opcode            dv_tag.brcm_op
#define dv_mir               dv_tag.brcm_mir
#define dv_mo                dv_tag.brcm_mo
#define dv_len               dv_tag.brcm_cnt
#define dv_dst_devid         dv_tag.brcm_dst_devid
#define dv_src_devid         dv_tag.brcm_src_devid
#define dv_dst_portid        dv_tag.brcm_dst_portid
#define dv_src_portid        dv_tag.brcm_src_portid
#define dv_dst_pbmp         dv_tag.brcm_dst_pbmp
#define dv_dst_pbmp_53242   dv_tag.brcm_dst_pbmp_53242 

#define dv_tc   dv_tag.brcm_tc
#define dv_tc_53242   dv_tag.brcm_tq_53242
#define dv_te   dv_tag.brcm_te
#define dv_te_53242 dv_tag.brcm_te_53242
#define dv_ts   dv_tag.brcm_ts

#define dv_src_portid_53242 dv_tag.brcm_53242_src_portid
#define dv_reason_53242 dv_tag.brcm_53242_reason
#define dv_cosq_53242 dv_tag.brcm_53242_cosq

#define dv_src_portid_5395 dv_tag.brcm_5395_src_portid
#define dv_reason_5395 dv_tag.brcm_5395_reason
#define dv_tc_5395 dv_tag.brcm_5395_tc
#define SOC_ROBO_DV_TS_TX_EVENT_5395(dv) (dv->dv_tag.brcm_5395_tc & 0x1) 

#define dv_src_portid_53115 dv_tag.brcm_53115_src_portid
#define dv_reason_53115 dv_tag.brcm_53115_reason
#define dv_imp_egress_tc_53115 dv_tag.brcm_53115_tc
#define dv_tc_53115 dv_tag.brcm_tc_53115
#define dv_te_53115   dv_tag.brcm_te_53115
#define dv_ts_53115   dv_tag.brcm_ts_53115
#define SOC_ROBO_DV_TS_TX_EVENT_53115(dv) (dv->dv_tag.brcm_53115_tc & 0x1) 

#define dv_opcode_53280   dv_tag.brcm_opcode_53280
#define dv_tc_53280       dv_tag.brcm_tc_53280
#define dv_dp_53280       dv_tag.brcm_dp_53280
#define dv_egr_vid_53280  dv_tag.brcm_egr_vid_53280
#define dv_vport_id_53280 dv_tag.brcm_vpid_53280
#define dv_port_id_53280  dv_tag.brcm_pid_53280
#define dv_reason_53280   dv_tag2.brcm_reason_53280
#define dv_lrn_dis_53280  dv_tag2.brcm_lrn_dis_53280
#define dv_fm_sel_53280   dv_tag2.brcm_fm_sel_53280
#define dv_fm_id_53280    dv_tag2.brcm_fm_id_53280
#define dv_filter_53280   dv_tag.brcm_filter_53280
#define dv_dstid_53280    dv_tag.brcm_dstid_53280
#define dv_ts_53280       dv_tag2.brcm_ts_53280
#define dv_ing_vid_53280  dv_tag2.brcm_ing_vid_53280
#define dv_flow_id_53280  dv_tag2.brcm_flow_id_53280
#define dv_dnm_ctrl_53280  dv_tag2.brcm_dnm_53280

#define TAG_NO_ENFORCEMENT  0
#define UNTAG_ENFORCEMENT   1
#define TAG_ENFORCEMENT         2

extern int      soc_eth_dma_attach(int unit);
extern int      soc_eth_dma_detach(int unit);

extern int      soc_eth_dma_desc_add(eth_dv_t *dv, sal_vaddr_t addr, uint16 cnt); 
extern eth_dv_t     *soc_eth_dma_dv_alloc(int unit, dvt_t op, int cnt);
extern int      soc_eth_dma_dv_join(eth_dv_t *dv_list, eth_dv_t *dv_add);
extern void     soc_eth_dma_dv_reset(dvt_t, eth_dv_t *);
extern void     soc_eth_dma_dv_free(int unit, eth_dv_t *dv);
extern int      soc_eth_dma_start(int unit, eth_dv_t *dv_chain);
extern int      soc_eth_dma_rxstop(int unit);
extern int      soc_eth_dma_rxenable(int unit);


/* Wait on synchronous send - requires a context */
extern int      soc_eth_dma_wait(int unit, eth_dv_t *dv_chain);
extern int      soc_eth_dma_wait_timeout(int unit, eth_dv_t *dv_chain, int usec);

extern int      soc_eth_dma_dv_valid(eth_dv_t *dv);
extern void     soc_eth_dma_dump_dv(int unit, char *pfx, eth_dv_t *);
extern void     soc_eth_dma_dump_pkt(int unit, char *pfx, uint8 *addr, int len);
extern void     soc_eth_dma_ether_dump(int unit, char *pfx, uint8 *addr,
                                   int len, int offset);

extern int      et_soc_attach(int unit);
extern int  et_soc_detach(int unit);
extern int  soc_eth_dma_unit_get(int unit, int *eth_unit);

extern void     et_soc_debug(int unit);
extern int      et_soc_open(int unit);
extern int      et_soc_close(int unit);
extern int      et_soc_start(int unit, eth_dv_t *dv_chain);
extern int      et_soc_rx_chain(int unit, eth_dv_t *dv_chain);
extern eth_dv_t     *et_soc_rx_chain_get(int unit, int chan, int flags);
extern void     et_soc_rxmon_on(int unit);
extern void     et_soc_rxmon_off(int unit);

#ifdef INCLUDE_KNET
extern void et_soc_knet_sendnext(int unit);
extern void et_soc_done_knet_tx(int unit, uint32 seq_no);
extern int et_soc_done_knet_rx(int unit, int chan);
extern void et_soc_knet_rxfill(int unit, int chan);
extern void et_soc_knet_rx_event_update(int unit, int chan, uint32 seq_no);
extern int et_soc_knet_check_rxq(int unit, int chan);

#endif

extern void soc_eth_dma_reinit(int unit);
extern void soc_eth_dma_hwrxoff_get(int unit, uint32* hwrxoff);

extern void soc_eth_dma_occupancy_set(int dma_id, int unit);
extern void soc_eth_dma_occupancy_get(int dma_id, int *unit);

extern void soc_eth_dma_start_channel(int unit, int channel);
extern void soc_eth_dma_abort_channel(int unit, int channel);


#if defined(KEYSTONE)
/* socEthDmaClassifyXXX, types for soc_eth_dma_classify_setup()  */
typedef enum soc_eth_dma_classify_e {
    socEthDmaClassifyOam,
    socEthDmaClassifyCount
} soc_eth_dma_classify_t;

#define SOC_ETH_DMA_CLASSIFY_DISABLE -1

extern void soc_eth_dma_classify_setup(int unit, int type, int chan);
extern void soc_eth_dma_default_drop_enable(int unit, int drop_enable);

#endif

/* flags for et_soc_rx_chain_get */
#define ET_RXCHAIN_F_GET_ALL_DV     0
#define ET_RXCHAIN_F_GET_DONE_DV    1
#define ET_RXCHAIN_F_GET_FREE_DV    2




#endif /* _SOC_ETHDMA_H */
