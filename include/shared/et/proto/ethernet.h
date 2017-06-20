/*******************************************************************************
 * $Id: ethernet.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * From FreeBSD 2.2.7: Fundamental constants relating to ethernet.
 ******************************************************************************/

#ifndef _NET_ETHERNET_H_	/* use native BSD ethernet.h when available */
#define _NET_ETHERNET_H_

#ifndef _TYPEDEFS_H_
#include "typedefs.h"
#endif

#if defined(__arm) && defined(__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#define	PACKED
#endif

/*
 * The number of bytes in an ethernet (MAC) address.
 */
#define	ETHER_ADDR_LEN		6

/*
 * The number of bytes in the type field.
 */
#define	ETHER_TYPE_LEN		2

/*
 * The number of bytes in the trailing CRC field.
 */
#define	ETHER_CRC_LEN		4

/*
 * The length of the combined header.
 */
#define	ETHER_HDR_LEN		(ETHER_ADDR_LEN*2+ETHER_TYPE_LEN)

/*
 * The minimum packet length.
 */
#define	ETHER_MIN_LEN		64

/*
 * The minimum packet user data length.
 */
#define	ETHER_MIN_DATA		46

/*
 * The maximum packet length.
 */
#define	ETHER_MAX_LEN		1518

/*
 * The maximum packet user data length.
 */
#define	ETHER_MAX_DATA		1500

/*
 * Used to uniquely identify a 802.1q VLAN-tagged header.
 */
#define	VLAN_TAG			0x8100

/*
 * Located after dest & src address in ether header.
 */
#define VLAN_FIELDS_OFFSET		(ETHER_ADDR_LEN * 2)

/*
 * 4 bytes of vlan field info.
 */
#define VLAN_FIELDS_SIZE		4

/* location of pri bits in 16-bit vlan fields */
#define VLAN_PRI_SHIFT			13

/* 3 bits of priority */
#define VLAN_PRI_MASK			7

/* 802.1X ethertype */
#define ETHER_TYPE_802_1X	0x888e

/* WPA driver message ethertype - private between wlc and nas */
#define ETHER_TYPE_WLC_WPA_MSG	0x9999

/*
 * A macro to validate a length with
 */
#define	ETHER_IS_VALID_LEN(foo)	\
	((foo) >= ETHER_MIN_LEN && (foo) <= ETHER_MAX_LEN)


#ifndef __INCif_etherh /* Quick workaround for VxWorks */
/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct	ether_header {
	uint8	ether_dhost[ETHER_ADDR_LEN];
	uint8	ether_shost[ETHER_ADDR_LEN];
	uint16	ether_type;
} PACKED ;

/*
 * Structure of a 48-bit Ethernet address.
 */
struct	ether_addr {
	uint8 octet[ETHER_ADDR_LEN];
} PACKED ;
#elif defined(VX_BSD4_3)
/*
 * Structure of a 48-bit Ethernet address.
 */
struct	ether_addr {
	uint8 octet[ETHER_ADDR_LEN];
} PACKED ;
#endif

/*
 * Takes a pointer, returns true if a 48-bit multicast address
 * (including broadcast, since it is all ones)
 */
#define ETHER_ISMULTI(ea) (((uint8 *)(ea))[0] & 1)


/* compare two ethernet addresses - assumes the pointers can be referenced as shorts */
#define	ether_cmp(a, b)	(!(((short*)a)[0] == ((short*)b)[0]) | \
			 !(((short*)a)[1] == ((short*)b)[1]) | \
			 !(((short*)a)[2] == ((short*)b)[2]))

/* copy an ethernet address - assumes the pointers can be referenced as shorts */
#define	ether_copy(s, d) { \
		((short*)d)[0] = ((short*)s)[0]; \
		((short*)d)[1] = ((short*)s)[1]; \
		((short*)d)[2] = ((short*)s)[2]; }

/*
 * Takes a pointer, returns true if a 48-bit broadcast (all ones)
 */
#define ETHER_ISBCAST(ea) ((((uint8 *)(ea))[0] &		\
			    ((uint8 *)(ea))[1] &		\
			    ((uint8 *)(ea))[2] &		\
			    ((uint8 *)(ea))[3] &		\
			    ((uint8 *)(ea))[4] &		\
			    ((uint8 *)(ea))[5]) == 0xff)

static const struct ether_addr ether_bcast = {{255, 255, 255, 255, 255, 255}};

/*
 * Takes a pointer, returns true if a 48-bit null address (all zeros)
 */
#define ETHER_ISNULLADDR(ea) ((((uint8 *)(ea))[0] |		\
			    ((uint8 *)(ea))[1] |		\
			    ((uint8 *)(ea))[2] |		\
			    ((uint8 *)(ea))[3] |		\
			    ((uint8 *)(ea))[4] |		\
			    ((uint8 *)(ea))[5]) == 0)

#undef PACKED

#endif /* _NET_ETHERNET_H_ */
