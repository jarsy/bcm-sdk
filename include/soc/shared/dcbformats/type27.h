/*
 * $Id: type27.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        soc/shared/dcbformats/type27.h
 * Purpose:     Define dma control block (DCB) format for a type27 DCB
 *              used by the 88230 (sirius)
 *
 *              This file is shared between the SDK and the embedded applications.
 */

#ifndef _SOC_SHARED_DCBFORMATS_TYPE27_H
#define _SOC_SHARED_DCBFORMATS_TYPE27_H

/*
 * DMA Control Block - Type 27
 * Used on 88230 devices
 * 16 words
 */
typedef struct {
        uint32  addr;                   /* T27.0: physical address */
                                        /* T27.1: Control 0 */
#ifdef  LE_HOST
        uint32  c_count:16,             /* Requested byte count */
                c_chain:1,              /* Chaining */
                c_sg:1,                 /* Scatter Gather */
                c_reload:1,             /* Reload */
                c_hg:1,                 /* Higig (TX) */
                c_stat:1,               /* update stats (TX) */
                c_pause:1,              /* Pause packet (TX) */
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

        uint32  mh0;                    /* T27.2: Module Header word 0 */
        uint32  mh1;                    /* T27.3: Module Header word 1 */
        uint32  mh2;                    /* T27.4: Module Header word 2 */
        uint32  mh3;                    /* T27.5: Module Header word 3 */
        uint32  mh_ext0;                /* T27.6: Module Header Extension word 0 */

#ifdef  LE_HOST
        uint32  reserved;               /* T27.7: Reserved */

                                        /* T27.8 */
        uint32  oi2qb_queue_mc_hi:6,    /* Oi2qb multicast queue ID [17:12] */
                oi2qb_queue_req:18,     /* Oi2qb requeue queue ID [17:0] */
	        :10;                    /* Reserved */

                                        /* T27.9 */
        uint32  sfh_hdr_79_64:6,        /* Sbx Fabric Header [79:64] */
	        class_reso_addr:10,     /* Class resolution address [9:0] */
	        pkt_class:4,            /* Packet Class */
	        oi2qb_queue_mc_lo:12;   /* Oi2qb multicast queue ID [11:0] */

                                        /* T27.10 */	        
        uint32  sfh_hdr_63_32;          /* Sbx Fabric Header [63:32] */

                                        /* T27.11 SBX fabric header */
        uint32  sfh_hdr_31_0;           /* Sbx Fabric Header [31:0] */

                                        /* T27.12: PBE [70:64] */
        uint32  pbe_70_64:7,            /* PBE (egress cell control bus) [70:64] */
                :25;                    /* Reserved */

        uint32  pbe_63_32;              /* T27.13: PBE [63:32] */
        uint32  pbe_31_0;               /* T27.14: PBE [31:0] */

                                         /* T27.15: DMA Status 0 */
        uint32  count:16,                /* Transferred byte count */
                end:1,                   /* End bit (RX) */
                start:1,                 /* Start bit (RX) */
                error:1,                 /* Cell Error (RX) */
                pktdata_read_ecc_error:1,/* 2 bit ECC error while reading packet
                                            data from RX data Buffers.*/
                status_read_ecc_error:1, /* 2 bit ECC error while reading Status
                                            from RX Status Buffers.*/
                :10,                     /* Reserved */
                done:1;                  /* Descriptor Done */
#else
        uint32  reserved;               /* T27.7: Reserved */

                                        /* T27.8 */
        uint32  :10,                    /* Reserved */
	        oi2qb_queue_req:18,     /* Oi2qb requeue queue ID [17:0] */
	        oi2qb_queue_mc_hi:6;    /* Oi2qb multicast queue ID [17:12] */

                                        /* T27.9 */
        uint32  oi2qb_queue_mc_lo:12,   /* Oi2qb multicast queue ID [11:0] */
	        pkt_class:4,            /* Packet Class */
		class_reso_addr:10,     /* Class resolution address [9:0] */
	        sfh_hdr_79_64:6;        /* Sbx Fabric Header [79:64] */

                                        /* T27.10 */	        
        uint32  sfh_hdr_63_32;          /* Sbx Fabric Header [63:32] */

                                        /* T27.11 SBX fabric header */
        uint32  sfh_hdr_31_0;           /* Sbx Fabric Header [31:0] */

                                        /* T27.12: PBE [70:64] */
        uint32  :25,                    /* Reserved */
	        pbe_70_64:7;            /* PBE (egress cell control bus) [70:64] */

        uint32  pbe_63_32;              /* T27.13: PBE [63:32] */
        uint32  pbe_31_0;               /* T27.14: PBE [31:0] */

                                         /* T27.15: DMA Status 0 */
        uint32  done:1,                  /* Descriptor Done */
                :10,                     /* Reserved */
                status_read_ecc_error:1, /* 2 bit ECC error while reading Status
                                            from RX Status Buffers. */
                pktdata_read_ecc_error:1,/* 2 bit ECC error while reading packet
                                            data from RX data Buffers.*/
                error:1,                 /* Cell Error (RX) */
                start:1,                 /* Start bit (RX) */
                end:1,                   /* End bit (RX) */
                count:16;                /* Transferred byte count */
#endif
} dcb27_t;
#endif
