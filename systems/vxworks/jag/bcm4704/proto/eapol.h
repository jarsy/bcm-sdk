/*
 * 802.1x EAPOL definitions
 *
 * See
 * IEEE Std 802.1X-2001
 * IEEE 802.1X RADIUS Usage Guidelines
 *
 * Copyright (C) 2002 Broadcom Corporation
 *
 * $Id: eapol.h,v 1.1 2004/02/24 07:47:01 csm Exp $
 */

#ifndef _eapol_h_
#define _eapol_h_

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#pragma pack(1)
#define PACKED
#endif

/* EAPOL for 802.3/Ethernet */
typedef struct {
	struct ether_header eth;	/* 802.3/Ethernet header */
	unsigned char version;		/* EAPOL protocol version */
	unsigned char type;		/* EAPOL type */
	unsigned short length;		/* Length of body */
	unsigned char body[1];		/* Body (optional) */
} eapol_header_t;

#define EAPOL_HEADER_LEN 18

/* EAPOL version */
#define EAPOL_VERSION 1

/* EAPOL types */
#define EAP_PACKET		0
#define EAPOL_START		1
#define EAPOL_LOGOFF		2
#define EAPOL_KEY		3
#define EAPOL_ASF		4

#define EAPOL_WPA_KEY		254

/* EAPOL key header field sizes */
#define EAPOL_KEY_REPLAY_LEN	8
#define EAPOL_KEY_IV_LEN	16
#define EAPOL_KEY_SIG_LEN	16
#define EAPOL_WPA_KEY_NONCE_LEN	32
#define EAPOL_WPA_KEY_RSC_LEN	8
#define EAPOL_WPA_KEY_ID_LEN	8
#define EAPOL_WPA_KEY_MIC_LEN	16
#define EAPOL_WPA_MAX_KEY_SIZE	32
#define EAPOL_WPA_KEY_DATA_LEN	(EAPOL_WPA_MAX_KEY_SIZE + 8)

/* EAPOL-Key */
typedef struct {
	unsigned char type;			/* Key Descriptor Type */
	unsigned short length;			/* Key Length (unaligned) */
	unsigned char replay[EAPOL_KEY_REPLAY_LEN];	/* Replay Counter */
	unsigned char iv[EAPOL_KEY_IV_LEN];		/* Key IV */
	unsigned char index;				/* Key Flags & Index */
	unsigned char signature[EAPOL_KEY_SIG_LEN];	/* Key Signature */
	unsigned char key[1];				/* Key (optional) */
} PACKED eapol_key_header_t;

#define EAPOL_KEY_HEADER_LEN 44

/* WPA-EAPOL-Key */
typedef struct {
	unsigned char type;		/* Key Descriptor Type */
	unsigned short key_info;	/* Key Information (unaligned) */
	unsigned short key_len;		/* Key Length (unaligned) */
	unsigned char replay[EAPOL_KEY_REPLAY_LEN];	/* Replay Counter */
	unsigned char nonce[EAPOL_WPA_KEY_NONCE_LEN];	/* Nonce */
	unsigned char iv[EAPOL_KEY_IV_LEN];		/* Key IV */
	unsigned char rsc[EAPOL_WPA_KEY_RSC_LEN];	/* Key RSC */
	unsigned char id[EAPOL_WPA_KEY_ID_LEN];		/* Key ID */
	unsigned char mic[EAPOL_WPA_KEY_MIC_LEN];	/* Key MIC */
	unsigned short data_len;			/* Key Data Length */
	unsigned char data[EAPOL_WPA_KEY_DATA_LEN];	/* Key data */
} PACKED eapol_wpa_key_header_t;

#define EAPOL_WPA_KEY_LEN 95

/* WPA KEY KEY_INFO bits */
#define WPA_KEY_DESC_V1		0x01
#define WPA_KEY_DESC_V2		0x02
#define WPA_KEY_PAIRWISE	0x08
#define WPA_KEY_INDEX_0		0x00		
#define WPA_KEY_INDEX_1		0x10
#define WPA_KEY_INDEX_2		0x20
#define WPA_KEY_INDEX_3		0x30
#define WPA_KEY_INDEX_MASK	0x30
#define WPA_KEY_INDEX_SHIFT	0x04
#define WPA_KEY_INSTALL		0x40
#define WPA_KEY_ACK		0x80
#define WPA_KEY_MIC		0x100
#define WPA_KEY_SECURE		0x200
#define WPA_KEY_ERROR		0x400
#define WPA_KEY_REQ		0x800

/* EAPOL-Key types */
#define EAPOL_KEY_RC4	1

/* EAPOL-Key flags */
#define EAPOL_KEY_FLAGS_MASK	0x80
#define EAPOL_KEY_BROADCAST	0
#define EAPOL_KEY_UNICAST	0x80

/* EAPOL-Key index */
#define EAPOL_KEY_INDEX_MASK	0x7f

/* EAPOL 802.3/Ethernet type */
#ifndef ETH_P_EAPOL
#define ETH_P_EAPOL 0x888e
#endif

#undef PACKED

#endif /* _eapol_h_ */
