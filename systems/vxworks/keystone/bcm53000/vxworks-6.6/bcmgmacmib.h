/*
 * Hardware-specific MIB definition for
 * Broadcom Home Networking Division
 * GbE Unimac core
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: bcmgmacmib.h,v 1.1 Broadcom SDK $
 */

#ifndef	_bcmgmacmib_h_
#define	_bcmgmacmib_h_


/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

/* GMAC MIB structure */

typedef struct _gmacmib {
	uint32	tx_good_octets;		/* 0x300 */
	uint32	tx_good_octets_high;	/* 0x304 */
	uint32	tx_good_pkts;		/* 0x308 */
	uint32	tx_octets;		/* 0x30c */
	uint32	tx_octets_high;		/* 0x310 */
	uint32	tx_pkts;		/* 0x314 */
	uint32	tx_broadcast_pkts;	/* 0x318 */
	uint32	tx_multicast_pkts;	/* 0x31c */
        uint32	tx_uni_pkts;		/* 0x320 */
	uint32	tx_len_64;		/* 0x324 */
	uint32	tx_len_65_to_127;	/* 0x328 */
	uint32	tx_len_128_to_255;	/* 0x32c */
	uint32	tx_len_256_to_511;	/* 0x330 */
	uint32	tx_len_512_to_1023;	/* 0x334 */
	uint32	tx_len_1024_to_max;	/* 0x338 */
	uint32	tx_len_max_to_jumbo;	/* 0x33c */
	uint32	tx_jabber_pkts;		/* 0x340 */
	uint32	tx_oversize_pkts;	/* 0x344 */
	uint32	tx_fragment_pkts;	/* 0x348 */
	uint32	tx_underruns;		/* 0x34c */
	uint32	tx_total_cols;		/* 0x350 */
	uint32	tx_single_cols;		/* 0x354 */
	uint32	tx_multiple_cols;	/* 0x358 */
	uint32	tx_excessive_cols;	/* 0x35c */
	uint32	tx_late_cols;		/* 0x360 */
	uint32	tx_defered;		/* 0x364 */
	uint32	tx_pause_pkts;		/* 0x368 */
	uint32	PAD[5];
	uint32	rx_good_octets;		/* 0x380 */
	uint32	rx_good_octets_high;	/* 0x384 */
	uint32	rx_good_pkts;		/* 0x388 */
	uint32	rx_octets;		/* 0x38c */
	uint32	rx_octets_high;		/* 0x390 */
	uint32	rx_pkts;		/* 0x394 */
	uint32	rx_broadcast_pkts;	/* 0x398 */
	uint32	rx_multicast_pkts;	/* 0x39c */
        uint32	rx_uni_pkts;		/* 0x3a0 */
	uint32	rx_len_64;		/* 0x3a4 */
	uint32	rx_len_65_to_127;	/* 0x3a8 */
	uint32	rx_len_128_to_255;	/* 0x3ac */
	uint32	rx_len_256_to_511;	/* 0x3b0 */
	uint32	rx_len_512_to_1023;	/* 0x3b4 */
	uint32	rx_len_1024_to_max;	/* 0x3b8 */
	uint32	rx_len_max_to_jumbo;	/* 0x3bc */
	uint32	rx_jabber_pkts;		/* 0x3c0 */
	uint32	rx_oversize_pkts;	/* 0x3c4 */
	uint32	rx_fragment_pkts;	/* 0x3c8 */
	uint32	rx_missed_pkts;		/* 0x3cc */
	uint32	rx_undersize;		/* 0x3d0 */
	uint32	rx_crc_errs;		/* 0x3d4 */
	uint32	rx_align_errs;		/* 0x3d8 */
	uint32	rx_symbol_errs;		/* 0x3dc */
	uint32	rx_pause_pkts;		/* 0x3e0 */
	uint32	rx_nonpause_pkts;	/* 0x3e4 */
	uint32 	rxq0_irc_drop;		/* 0x3e8 */
        uint32 	rxq1_irc_drop;		/* 0x3ec */
        uint32 	rxq2_irc_drop;		/* 0x3f0 */
        uint32 	rxq3_irc_drop;		/* 0x3f4 */
        uint32 	rx_cfp_drop;		/* 0x3f8 */
} gmacmib_t;

#define	GM_MIB_BASE		0x300
#define	GM_MIB_LIMIT		0x800

#endif	/* _bcmgmacmib_h_ */
