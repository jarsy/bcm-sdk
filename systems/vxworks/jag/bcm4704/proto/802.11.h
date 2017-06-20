/*
 * Copyright 2001, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * Fundamental types and constants relating to 802.11 
 *
 * $Id: 802.11.h,v 1.1 2004/02/24 07:47:01 csm Exp $
 */

#ifndef _802_11_H_
#define _802_11_H_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

#ifndef _NET_ETHERNET_H_
#include <proto/ethernet.h>
#endif

/* enable structure packing */
#if !defined (__GNUC__)
#pragma pack(1)
#endif

/* some platforms require stronger medicine */
#if defined (__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#define	PACKED
#endif

/* Generic 802.11 frame constants */
#define DOT11_A3_HDR_LEN		24
#define DOT11_A4_HDR_LEN		30
#define DOT11_MAC_HDR_LEN		DOT11_A3_HDR_LEN
#define DOT11_FCS_LEN			4
#define DOT11_ICV_LEN			4
#define DOT11_ICV_AES_LEN		8


#define DOT11_KEY_INDEX_SHIFT		6
#define DOT11_IV_LEN			4
#define DOT11_IV_TKIP_LEN		8
#define DOT11_IV_AES_OCB_LEN		4
#define DOT11_IV_AES_CCM_LEN		8

#define DOT11_MAX_MPDU_BODY_LEN		2312
#define DOT11_MAX_MPDU_LEN		2346	/* body len + A4 hdr + FCS */
#define DOT11_MAX_SSID_LEN		32

/* dot11RTSThreshold */
#define DOT11_DEFAULT_RTS_LEN		2347
#define DOT11_MAX_RTS_LEN		2347

/* dot11FragmentationThreshold */
#define DOT11_MIN_FRAG_LEN		256
#define DOT11_MAX_FRAG_LEN		2346	/* Max frag is also limited by aMPDUMaxLength of the attached PHY */
#define DOT11_DEFAULT_FRAG_LEN		2346

/* dot11BeaconPeriod */
#define DOT11_MIN_BEACON_PERIOD		1
#define DOT11_MAX_BEACON_PERIOD		0xFFFF

/* dot11DTIMPeriod */
#define DOT11_MIN_DTIM_PERIOD		1
#define DOT11_MAX_DTIM_PERIOD		0xFF

/* 802.2 LLC/SNAP header used by 802.11 per 802.1H */
#define DOT11_LLC_SNAP_HDR_LEN	8
#define DOT11_OUI_LEN			3
struct dot11_llc_snap_header {
	uint8	dsap;				/* always 0xAA */
	uint8	ssap;				/* always 0xAA */
	uint8	ctl;				/* always 0x03 */
	uint8	oui[DOT11_OUI_LEN];		/* RFC1042: 0x00 0x00 0x00
						   Bridge-Tunnel: 0x00 0x00 0xF8 */
	uint16	type;				/* ethertype */
} PACKED;

/* RFC1042 header used by 802.11 per 802.1H */
#define RFC1042_HDR_LEN			(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN)

/* Generic 802.11 MAC header */
/*
 * N.B.: This struct reflects the full 4 address 802.11 MAC header.
 *		 The fields are defined such that the shorter 1, 2, and 3
 *		 address headers just use the first k fields.
 */
struct dot11_header {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	a1;		/* address 1 */
	struct ether_addr	a2;		/* address 2 */
	struct ether_addr	a3;		/* address 3 */
	uint16			seq;		/* sequence control */
	struct ether_addr	a4;		/* address 4 */
} PACKED;

/* Control frames */

struct dot11_rts_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
	struct ether_addr	ta;		/* transmitter address */
} PACKED;
#define	DOT11_RTS_LEN		16

struct dot11_cts_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
} PACKED;
#define	DOT11_CTS_LEN		10

struct dot11_ack_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
} PACKED;
#define	DOT11_ACK_LEN		10

struct dot11_ps_poll_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* AID */
	struct ether_addr	bssid;		/* receiver address, STA in AP */
	struct ether_addr	ta;		/* transmitter address */
} PACKED;
#define	DOT11_PS_POLL_LEN	16

struct dot11_cf_end_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
	struct ether_addr	bssid;		/* transmitter address, STA in AP */
} PACKED;
#define	DOT11_CS_END_LEN	16

/* Management frame header */
struct dot11_management_header {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	da;		/* receiver address */
	struct ether_addr	sa;		/* transmitter address */
	struct ether_addr	bssid;		/* BSS ID */
	uint16			seq;		/* sequence control */
} PACKED;
#define	DOT11_MGMT_HDR_LEN	24

/* Management frame payloads */

struct dot11_bcn_prb {
	uint32			timestamp[2];
	uint16			beacon_interval;
	uint16			capability;
} PACKED;
#define	DOT11_BCN_PRB_LEN	12

struct dot11_auth {
	uint16			alg;		/* algorithm */
	uint16			seq;		/* sequence control */
	uint16			status;		/* status code */
} PACKED;
#define DOT11_AUTH_FIXED_LEN	6		/* length of auth frame without challenge info elt */

struct dot11_assoc_req {
	uint16			capability;	/* capability information */
	uint16			listen;		/* listen interval */
} PACKED;

struct dot11_assoc_resp {
	uint16			capability;	/* capability information */
	uint16			status;		/* status code */
	uint16			aid;		/* association ID */
} PACKED;

struct brcm_ie {
	uchar	id;		
	uchar	len;   
	uchar 	oui[3];
	uchar	ver;
	uchar	assoc;		/*  # of assoc STAs */
} PACKED;
#define BRCM_IE_LEN		7
typedef	struct brcm_ie brcm_ie_t;
#define BRCM_IE_VER		1


/* Macro to take a pointer to a beacon or probe response
 * header and return the char* pointer to the SSID info element
 */
#define BCN_PRB_SSID(hdr) ((char*)(hdr) + DOT11_MGMT_HDR_LEN + DOT11_BCN_PRB_LEN)

/* Authentication frame payload constants */
#define DOT11_OPEN_SYSTEM	0
#define DOT11_SHARED_KEY	1
#ifdef CCX
#define DOT11_LEAP_AUTH		0x80
#endif
#define DOT11_CHALLENGE_LEN	128

/* Frame control macros */
#define FC_PVER_MASK		0x3
#define FC_PVER_SHIFT		0
#define FC_TYPE_MASK		0xC
#define FC_TYPE_SHIFT		2
#define FC_SUBTYPE_MASK		0xF0
#define FC_SUBTYPE_SHIFT	4
#define FC_TODS			0x100
#define FC_TODS_SHIFT		8
#define FC_FROMDS		0x200
#define FC_FROMDS_SHIFT		9
#define FC_MOREFRAG		0x400
#define FC_MOREFRAG_SHIFT	10
#define FC_RETRY		0x800
#define FC_RETRY_SHIFT		11
#define FC_PM			0x1000
#define FC_PM_SHIFT		12
#define FC_MOREDATA		0x2000
#define FC_MOREDATA_SHIFT	13
#define FC_WEP			0x4000
#define FC_WEP_SHIFT		14
#define FC_ORDER		0x8000
#define FC_ORDER_SHIFT		15

/* sequence control macros */
#define SEQNUM_SHIFT		4
#define FRAGNUM_MASK		0xF

/* Frame Control type/subtype defs */

/* FC Types */
#define FC_TYPE_MNG		0
#define FC_TYPE_CTL		1
#define FC_TYPE_DATA		2

/* Management Subtypes */
#define FC_SUBTYPE_ASSOC_REQ		0
#define FC_SUBTYPE_ASSOC_RESP		1
#define FC_SUBTYPE_REASSOC_REQ		2
#define FC_SUBTYPE_REASSOC_RESP		3
#define FC_SUBTYPE_PROBE_REQ		4
#define FC_SUBTYPE_PROBE_RESP		5
#define FC_SUBTYPE_BEACON		8
#define FC_SUBTYPE_ATIM			9
#define FC_SUBTYPE_DISASSOC		10
#define FC_SUBTYPE_AUTH			11
#define FC_SUBTYPE_DEAUTH		12

/* Control Subtypes */
#define FC_SUBTYPE_PS_POLL		10
#define FC_SUBTYPE_RTS			11
#define FC_SUBTYPE_CTS			12
#define FC_SUBTYPE_ACK			13
#define FC_SUBTYPE_CF_END		14
#define FC_SUBTYPE_CF_END_ACK		15

/* Data Subtypes */
#define FC_SUBTYPE_DATA			0
#define FC_SUBTYPE_DATA_CF_ACK		1
#define FC_SUBTYPE_DATA_CF_POLL		2
#define FC_SUBTYPE_DATA_CF_ACK_POLL	3
#define FC_SUBTYPE_NULL			4
#define FC_SUBTYPE_CF_ACK		5
#define FC_SUBTYPE_CF_POLL		6
#define FC_SUBTYPE_CF_ACK_POLL		7

/* type-subtype combos */
#define FC_KIND_MASK		(FC_TYPE_MASK | FC_SUBTYPE_MASK)

#define FC_KIND(t, s) (((t) << FC_TYPE_SHIFT) | ((s) << FC_SUBTYPE_SHIFT))

#define FC_ASSOC_REQ	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_ASSOC_REQ)
#define FC_ASSOC_RESP	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_ASSOC_RESP)
#define FC_REASSOC_REQ	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_REASSOC_REQ)
#define FC_REASSOC_RESP	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_REASSOC_RESP)
#define FC_PROBE_REQ	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_PROBE_REQ)
#define FC_PROBE_RESP	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_PROBE_RESP)
#define FC_DISASSOC	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_DISASSOC)
#define FC_AUTH		FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_AUTH)
#define FC_DEAUTH	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_DEAUTH)
#define FC_BEACON	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_BEACON)

#define FC_PS_POLL	FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_PS_POLL)
#define FC_RTS		FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_RTS)
#define FC_CTS		FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_CTS)
#define FC_ACK		FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_ACK)
#define FC_CF_END	FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_CF_END)
#define FC_CF_END_ACK	FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_CF_END_ACK)

#define FC_DATA		FC_KIND(FC_TYPE_DATA, FC_SUBTYPE_DATA)
#define FC_NULL_DATA	FC_KIND(FC_TYPE_DATA, FC_SUBTYPE_NULL)
#define FC_DATA_CF_ACK	FC_KIND(FC_TYPE_DATA, FC_SUBTYPE_DATA_CF_ACK)

/* Management Frames */

/* Management Frame Constants */

/* Fixed fields */
#define DOT11_MNG_AUTH_ALGO_LEN		2
#define DOT11_MNG_AUTH_SEQ_LEN		2
#define DOT11_MNG_BEACON_INT_LEN	2
#define DOT11_MNG_CAP_LEN		2
#define DOT11_MNG_AP_ADDR_LEN		6
#define DOT11_MNG_LISTEN_INT_LEN	2
#define DOT11_MNG_REASON_LEN		2
#define DOT11_MNG_AID_LEN		2
#define DOT11_MNG_STATUS_LEN		2
#define DOT11_MNG_TIMESTAMP_LEN		8

/* DUR/ID field in assoc resp is 0xc000 | AID */
#define DOT11_AID_MASK			0x3fff

/* Reason Codes */
#define DOT11_RC_RESERVED			0
#define DOT11_RC_UNSPECIFIED			1	/* Unspecified reason */
#define DOT11_RC_AUTH_INVAL			2	/* Previous authentication no longer valid */
#define DOT11_RC_DEAUTH_LEAVING			3	/* Deauthenticated because sending station is
							   leaving (or has left) IBSS or ESS */
#define DOT11_RC_INACTIVITY			4	/* Disassociated due to inactivity */
#define DOT11_RC_BUSY				5	/* Disassociated because AP is unable to handle
							   all currently associated stations */
#define DOT11_RC_INVAL_CLASS_2			6	/* Class 2 frame received from
							   nonauthenticated station */
#define DOT11_RC_INVAL_CLASS_3			7	/* Class 3 frame received from
							   nonassociated station */
#define DOT11_RC_DISASSOC_LEAVING		8	/* Disassociated because sending station is
							   leaving (or has left) BSS */
#define DOT11_RC_NOT_AUTH			9	/* Station requesting (re)association is
							   not authenticated with responding station */
/* 10 and 11 are from TGh. */
#define DOT11_RC_BAD_PC				10	/* Unacceptable power capability element */
#define DOT11_RC_BAD_CHANNELS			11	/* Unacceptable supported channels element */
/* 12 is unused */
/* 13 through 23 taken from P802.11i/D3.0, November 2002 */
#define DOT11_RC_INVALID_WPA_IE			13	/* Invalid info. element */
#define DOT11_RC_MIC_FAILURE			14	/* Michael failure */
#define DOT11_RC_4WH_TIMEOUT			15	/* 4-way handshake timeout */
#define DOT11_RC_GTK_UPDATE_TIMEOUT		16	/* Group key update timeout */
#define DOT11_RC_WPA_IE_MISMATCH		17	/* WPA IE in 4-way handshake differs from (re-)assoc. request/probe response */
#define DOT11_RC_INVALID_MC_CIPHER		18	/* Invalid multicast cipher */
#define DOT11_RC_INVALID_UC_CIPHER		19	/* Invalid unicast cipher */
#define DOT11_RC_INVALID_AKMP			20	/* Invalid authenticated key management protocol */
#define DOT11_RC_BAD_WPA_VERSION		21	/* Unsupported WPA version */
#define DOT11_RC_INVALID_WPA_CAP		22	/* Invalid WPA IE capabilities */
#define DOT11_RC_8021X_AUTH_FAIL		23	/* 802.1X authentication failure */
#define DOT11_RC_MAX				23	/* Reason codes > 23 are reserved */

/* Status Codes */
#define DOT11_STATUS_SUCCESS			0	/* Successful */
#define DOT11_STATUS_FAILURE			1	/* Unspecified failure */
#define DOT11_STATUS_CAP_MISMATCH		10	/* Cannot support all requested capabilities
							   in the Capability Information field */
#define DOT11_STATUS_REASSOC_FAIL		11	/* Reassociation denied due to inability to
							   confirm that association exists */
#define DOT11_STATUS_ASSOC_FAIL			12	/* Association denied due to reason outside
							   the scope of this standard */
#define DOT11_STATUS_AUTH_MISMATCH		13	/* Responding station does not support the
							   specified authentication algorithm */
#define DOT11_STATUS_AUTH_SEQ			14	/* Received an Authentication frame with
							   authentication transaction sequence number
							   out of expected sequence */
#define DOT11_STATUS_AUTH_CHALLENGE_FAIL	15	/* Authentication rejected because of challenge failure */
#define DOT11_STATUS_AUTH_TIMEOUT		16	/* Authentication rejected due to timeout waiting
							   for next frame in sequence */
#define DOT11_STATUS_ASSOC_BUSY_FAIL		17	/* Association denied because AP is unable to
							   handle additional associated stations */
#define DOT11_STATUS_ASSOC_RATE_MISMATCH	18	/* Association denied due to requesting station
							   not supporting all of the data rates in the
							   BSSBasicRateSet parameter */
#define DOT11_STATUS_ASSOC_SHORT_REQUIRED	19	/* Association denied due to requesting station
							   not supporting the Short Preamble option */
#define DOT11_STATUS_ASSOC_PBCC_REQUIRED	20	/* Association denied due to requesting station
							   not supporting the PBCC Modulation option */
#define DOT11_STATUS_ASSOC_AGILITY_REQUIRED	21	/* Association denied due to requesting station
							   not supporting the Channel Agility option */
#define DOT11_STATUS_ASSOC_SHORTSLOT_REQUIRED	25	/* Association denied due to requesting station
							   not supporting the Short Slot Time option */
#define DOT11_STATUS_ASSOC_ERPBCC_REQUIRED	26	/* Association denied due to requesting station
							   not supporting the ER-PBCC Modulation option */
#define DOT11_STATUS_ASSOC_DSSOFDM_REQUIRED	27	/* Association denied due to requesting station
							   not supporting the DSS-OFDM option */

/* Info Elts, length of INFORMATION portion of Info Elts */
#define DOT11_MNG_DS_PARAM_LEN			1
#define DOT11_MNG_IBSS_PARAM_LEN		2

/* TIM Info element has 3 bytes fixed info in INFORMATION field,
 * followed by 1 to 251 bytes of Partial Virtual Bitmap */
#define DOT11_MNG_TIM_FIXED_LEN			3

/* TLV defines */
#define TLV_TAG_OFF		0
#define TLV_LEN_OFF		1
#define TLV_HDR_LEN		2
#define TLV_BODY_OFF		2

/* Management Frame Information Element IDs */
#define DOT11_MNG_SSID_ID			0
#define DOT11_MNG_RATES_ID			1
#define DOT11_MNG_FH_PARMS_ID			2
#define DOT11_MNG_DS_PARMS_ID			3
#define DOT11_MNG_CF_PARMS_ID			4
#define DOT11_MNG_TIM_ID			5
#define DOT11_MNG_IBSS_PARMS_ID			6
#define DOT11_MNG_COUNTRY_ID			7
#define DOT11_MNG_HOPPING_PARMS_ID		8
#define DOT11_MNG_HOPPING_TABLE_ID		9
#define DOT11_MNG_REQUEST_ID			10
#define DOT11_MNG_CHALLENGE_ID			16
#define DOT11_MNG_ERP_ID			42
#define DOT11_MNG_NONERP_ID			47
#define DOT11_MNG_EXT_RATES_ID			50
#ifdef CCX
#define DOT11_MNG_AIRONET_ID			133
#endif
#define DOT11_MNG_WPA_ID			221
#define DOT11_MNG_PROPR_ID			221

/* ERP info element bit values */
#define DOT11_MNG_ERP_LEN			1	/* ERP is currently 1 byte long */
#define DOT11_MNG_NONERP_PRESENT		0x01	/* NonERP (802.11b) STAs are present in the BSS */
#define DOT11_MNG_USE_PROTECTION		0x02	/* Use protection mechanisms for ERP-OFDM frames */
#define DOT11_MNG_BARKER_PREAMBLE		0x04	/* Short Preambles: 0 == allowed, 1 == not allowed */

/* Capability Information Field */
#define DOT11_CAP_ESS				0x01
#define DOT11_CAP_IBSS				0x02
#define DOT11_CAP_POLLABLE			0x04
#define DOT11_CAP_POLL_RQ			0x08
#define DOT11_CAP_PRIVACY			0x10
#define DOT11_CAP_SHORT				0x20
#define DOT11_CAP_PBCC				0x40
#define DOT11_CAP_AGILITY			0x80
#define DOT11_CAP_CCK_OFDM			0x2000
#define DOT11_CAP_SHORTSLOT			0x0400

/* MLME Enumerations */
#define DOT11_BSSTYPE_INFRASTRUCTURE		0
#define DOT11_BSSTYPE_INDEPENDENT		1
#define DOT11_BSSTYPE_ANY			2
#define DOT11_SCANTYPE_ACTIVE			0
#define DOT11_SCANTYPE_PASSIVE			1

/* 802.11 A PHY constants */
#define APHY_SLOT_TIME		9
#define APHY_SIFS_TIME		16
#define APHY_DIFS_TIME		(APHY_SIFS_TIME + (2 * APHY_SLOT_TIME))
#define APHY_PREAMBLE_TIME	16
#define APHY_SIGNAL_TIME	4
#define APHY_SYMBOL_TIME	4
#define APHY_SERVICE_NBITS	16
#define APHY_TAIL_NBITS		6
#define	APHY_CWMIN		15

/* 802.11 B PHY constants */
#define BPHY_SLOT_TIME		20
#define BPHY_SIFS_TIME		10
#define BPHY_DIFS_TIME		50
#define BPHY_PLCP_TIME		192
#define BPHY_PLCP_SHORT_TIME	96
#define	BPHY_CWMIN		31

/* 802.11 G constants */
#define DOT11_OFDM_SIGNAL_EXTENSION	6

#define PHY_CWMAX		1023

#define	DOT11_MAXNUMFRAGS	16	/* max # fragments per MSDU */

/* dot11Counters Table - 802.11 spec., Annex D */
typedef struct d11cnt {
	uint32		txfrag;		/* dot11TransmittedFragmentCount */
	uint32		txmulti;	/* dot11MulticastTransmittedFrameCount */
	uint32		txfail;		/* dot11FailedCount */
	uint32		txretry;	/* dot11RetryCount */
	uint32		txretrie;	/* dot11MultipleRetryCount */
	uint32		rxdup;		/* dot11FrameduplicateCount */
	uint32		txrts;		/* dot11RTSSuccessCount */
	uint32		txnocts;	/* dot11RTSFailureCount */
	uint32		txnoack;	/* dot11ACKFailureCount */
	uint32		rxfrag;		/* dot11ReceivedFragmentCount */
	uint32		rxmulti;	/* dot11MulticastReceivedFrameCount */
	uint32		rxcrc;		/* dot11FCSErrorCount */
	uint32		txfrmsnt;	/* dot11TransmittedFrameCount */
	uint32		rxundec;	/* dot11WEPUndecryptableCount */
} d11cnt_t;

/* BRCM OUI */
#define BRCM_OUI		"\x00\x10\x18"

/* WPA definitions */
#define WPA_VERSION		1
#define WPA_OUI			"\x00\x50\xF2"

#define WPA_OUI_LEN	3

/* WPA authentication modes */
#define WPA_AUTH_NONE		0	/* None */
#define WPA_AUTH_UNSPECIFIED	1	/* Unspecified authentication over 802.1X: default for WPA */
#define WPA_AUTH_PSK		2	/* Pre-shared Key over 802.1X */
#define WPA_AUTH_DISABLED	255	/* Legacy (i.e., non-WPA) */
				 
#define IS_WPA_AUTH(auth)	((auth) == WPA_AUTH_NONE || \
				 (auth) == WPA_AUTH_UNSPECIFIED || \
				 (auth) == WPA_AUTH_PSK)

/* WPA IE fixed portion */
typedef struct
{
	uint8 tag;	/* TAG */
	uint8 length;	/* TAG length */
	uint8 oui[3];	/* IE OUI */
	uint8 oui_type;	/* OUI type */
	struct {
		uint8 low;
		uint8 high;
	} version;	/* IE version */
} wpa_ie_fixed_t PACKED;
#define WPA_IE_OUITYPE_LEN	4
#define WPA_IE_FIXED_LEN	8
#define WPA_IE_TAG_FIXED_LEN	6

/* WPA suite/multicast suite */
typedef struct
{
	uint8 oui[3];
	uint8 type;
} wpa_suite_t PACKED, wpa_suite_mcast_t PACKED;
#define WPA_SUITE_LEN	4

/* WPA unicast suite list/key management suite list */
typedef struct
{
	struct {
		uint8 low;
		uint8 high;
	} count;
	wpa_suite_t list[1];
} wpa_suite_ucast_t PACKED, wpa_suite_auth_key_mgmt_t PACKED;
#define WPA_IE_SUITE_COUNT_LEN	2

/* WPA cipher suites */
#define WPA_CIPHER_NONE		0	/* None */
#define WPA_CIPHER_WEP_40	1	/* WEP (40-bit) */
#define WPA_CIPHER_TKIP		2	/* TKIP: default for WPA */
#define WPA_CIPHER_AES_OCB	3	/* AES (OCB) */
#define WPA_CIPHER_AES_CCM	4	/* AES (CCM) */
#define WPA_CIPHER_WEP_104	5	/* WEP (104-bit) */

#define IS_WPA_CIPHER(cipher)	((cipher) == WPA_CIPHER_NONE || \
				 (cipher) == WPA_CIPHER_WEP_40 || \
				 (cipher) == WPA_CIPHER_WEP_104 || \
				 (cipher) == WPA_CIPHER_TKIP || \
				 (cipher) == WPA_CIPHER_AES_OCB || \
				 (cipher) == WPA_CIPHER_AES_CCM)

/* WPA TKIP countermeasures parameters */
#define WPA_TKIP_CM_DETECT	60	/* multiple MIC failure window (seconds) */
#define WPA_TKIP_CM_BLOCK	60	/* countermeasures active window (seconds) */

#ifdef CCX

#define CKIP_CWMIN			6
#define CKIP_CWMAX			8
#define CKIP_NEG			10
#define	CKIP_MIC			0x08
#define CKIP_KP				0x10

#define CCX_DDP_LLC_SNAP_LEN	8
#define CCX_DDP_MSG_LEN		40
#define CCX_DDP_ROGUE_NAME_LEN	16
struct ccx_ddp_pkt_s {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint16 msg_len;
	uint8  msg_type;
	uint8  fcn_code;
	struct ether_addr dest_mac;
	struct ether_addr src_mac;
	uint16 fail_reason;
	struct ether_addr rogue_mac;
	uint8  rogue_name[CCX_DDP_ROGUE_NAME_LEN];
} PACKED;
typedef struct ccx_ddp_pkt_s	ccx_ddp_pkt_t;
#define CCX_DDP_PKT_LEN		(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + CCX_DDP_MSG_LEN)

#define	CCX_ROGUE_INVALID_AUTH	1
#define	CCX_ROGUE_LEAP_TIMEOUT	2
#define CCX_ROGUE_CHAN_FROM_AP	3
#define	CCX_ROGUE_CHAN_TO_AP	4

#endif /* CCX */

#undef PACKED
#if !defined (__GNUC__)
#pragma pack()
#endif

#endif /* _802_11_H_ */
