/*
 * $Id: nsgmac.h,v 1.1.2.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom Gigabit Ethernet MAC defines.
 */
#ifndef _nsgmac_h_
#define _nsgmac_h_

/* chip interrupt bit error summary */
#define	I_ERRORS		(GMAC2_INTMASK_DESCRERREN_MASK | \
     GMAC2_INTMASK_DATAERREN_MASK | \
     GMAC2_INTMASK_DESCPROTOERREN_MASK  | GMAC2_INTMASK_XMTUFEN_MASK)
#define	DEF_INTMASK		(GMAC2_INTMASK_XMTINTEN_0_MASK | \
    GMAC2_INTMASK_XMTINTEN_1_MASK | GMAC2_INTMASK_XMTINTEN_2_MASK | \
    GMAC2_INTMASK_XMTINTEN_3_MASK | GMAC2_INTMASK_RCVINTEN_MASK | \
    I_ERRORS)

#define GMAC_RESET_DELAY 	2

/* CHECKME */
#define GMAC_MIN_FRAMESIZE	17	/* gmac can only send frames of
	                                 * size above 17 octetes.
	                                 */

#define LOOPBACK_MODE_DMA	0	/* loopback the packet at the DMA engine */
#define LOOPBACK_MODE_MAC	1	/* loopback the packet at MAC */
#define LOOPBACK_MODE_NONE	2	/* no Loopback */

#define DMAREG(ch, dir, qnum)	((dir == DMA_TX) ? \
	                         (void *)(uint*)(&(ch->regs->gmac2_xmtcontrol_0) + (0x10 * qnum)) : \
	                         (void *)(uint*)(&(ch->regs->gmac2_rcvcontrol) + (0x10 * qnum)))

/*
 * Add multicast address to the list. Multicast address are maintained as
 * hash table with chaining.
 */
typedef struct mclist {
	struct ether_addr mc_addr;	/* multicast address to allow */
	struct mclist *next;		/* next entry */
} mflist_t;

#define GMAC_HASHT_SIZE		16	/* hash table size */
#define GMAC_MCADDR_HASH(m)	((((uint8 *)(m))[3] + ((uint8 *)(m))[4] + \
	                         ((uint8 *)(m))[5]) & (GMAC_HASHT_SIZE - 1))

#define ETHER_MCADDR_CMP(x, y) ((((uint16 *)(x))[0] ^ ((uint16 *)(y))[0]) | \
				(((uint16 *)(x))[1] ^ ((uint16 *)(y))[1]) | \
				(((uint16 *)(x))[2] ^ ((uint16 *)(y))[2]))

#define SUCCESS			0
#define FAILURE			-1
#define TIMEOUT                 -2

typedef struct mcfilter {
					/* hash table for multicast filtering */
	mflist_t *bucket[GMAC_HASHT_SIZE];
} mcfilter_t;

#endif /* _nsgmac_h_ */

