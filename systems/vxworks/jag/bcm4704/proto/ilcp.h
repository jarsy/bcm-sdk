/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * ILine Control Protocol
 *
 * $Id: ilcp.h,v 1.1 2004/02/24 07:47:01 csm Exp $
 */

#ifndef _ILCP_H_
#define _ILCP_H_ 1

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif /* _TYPEDEFS_H_ */
#ifndef _NET_ETHERNET_H_
#include <proto/ethernet.h>
#endif /* _NET_ETHERNET_H_ */

/* enable structure packing */
#if defined(__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#pragma pack(1)
#define	PACKED
#endif

#define ETHERTYPE_ILCP	0x886c

/*
 * Following the 2byte ILCP ether_type is a 16bit ILCP subtype field
 * in one of two formats:
 *
 * subtypes 0-32767:
 *     8 bit subtype (0-127)
 *     8 bit length in bytes (0-255)
 *
 * subtypes 32768-65535:
 *     16 bit big-endian subtype
 *     16 bit big-endian length in bytes (0-65535)
 *
 * length is the number of additional bytes beyond the 4 or 6 byte header
 *
 * Reserved values:
 * 0 reserved
 * 5-15 reserved for iLine protocol assignments
 * 17-126 reserved, assignable
 * 127 reserved
 * 32768 reserved
 * 32769-65534 reserved, assignable
 * 65535 reserved
 */
#define	ILCP_SUBTYPE_RATE		1
#define	ILCP_SUBTYPE_LINK		2
#define	ILCP_SUBTYPE_CSA		3
#define	ILCP_SUBTYPE_LARQ		4
#define ILCP_SUBTYPE_VENDOR		5
#define	ILCP_SUBTYPE_FLH		17

#define ILCP_SUBTYPE_VENDOR_LONG	32769
#define ILCP_SUBTYPE_CERT		32770

typedef  struct ilcp_t8hdr
{
	uint8	subtype8;
	uint8	length;
} PACKED ilcp_t8hdr_t;

typedef  struct ilcp_t16hdr
{
	uint16	subtype16;
	uint16	length;
} PACKED ilcp_t16hdr_t;

#define	ISILCP(eh) (ntoh16(eh->ether_type) == ETHERTYPE_ILCP)

#define	ISRRCF(eh) ((ntoh16(eh->ether_type) == ETHERTYPE_ILCP) \
	&& (((ilcp_t8hdr_t*)&eh[1])->subtype8 == ILCP_SUBTYPE_RATE))

#define	ISLICF(eh) ((ntoh16(eh->ether_type) == ETHERTYPE_ILCP) \
	&& (((ilcp_t8hdr_t*)&eh[1])->subtype8 == ILCP_SUBTYPE_LINK))

#define	ISLARQ(eh) ((ntoh16(eh->ether_type) == ETHERTYPE_ILCP) \
	&& (((ilcp_t8hdr_t*)&eh[1])->subtype8 == ILCP_SUBTYPE_LARQ))

#define	ISCERT(eh) ((ntoh16(eh->ether_type) == ETHERTYPE_ILCP) \
	&& (ntoh16(((ilcp_t16hdr_t*)&eh[1])->subtype16) == ILCP_SUBTYPE_CERT))

#define	ISCERTDATA(eh) (ISCERT(eh) \
	&& (((ilcp_v0_cdcf_t *)&eh[1])->OpCode == CDCF_V0_OPCODE_TESTDATA))

/* this structure assumes numbands = 1 and numaddr = 1 */
typedef  struct rate_ctrl
{
	ilcp_t8hdr_t	h;
	uint8		rc_version;	/* rate control version */
	uint8		rc_opcode;	/* rate control opcode */
	uint8		rc_numbands;	/* number of bands listed */
	uint8		rc_numaddr;	/* number of addresses listed */
	uint8		rc_band0_pe;	/* payload encoding for band0 */
	uint8		rc_band0_rank;	/* rank (preference) for band0 */
	union {
		struct ether_addr rc_refaddr0;	/* refaddr0, assuming numbands = 1 */
		uint16		next_ethertype0; /* 0 - shows control, no encaps */
	} PACKED a;
	uint16		next_ethertype1; /* 0 - shows control, no encaps */
} PACKED rate_ctrl_t;

/* min length is when numbands = 1 and numaddr = 0 */
#define RC_MIN_LENGTH		8

#define RC_VERSION		0
#define RC_OPCODE_CHANGE	0
#define RC_OPCODE_TEST		1
#define RC_OPCODE_REPLY		2

/* "bands" are a rate concept */
#define MAX_BANDS		2
#define PE_NULL			0
#define PE_BAND1_2BPB		1
#define PE_BAND1_3BPB		2
#define PE_BAND1_4BPB		3
#define PE_BAND1_5BPB		4
#define PE_BAND1_6BPB		5
#define PE_BAND1_7BPB		6
#define PE_BAND1_8BPB		7
#define PE_HPNA1		8
#define PE_BAND2_2BPB		9
#define PE_BAND2_3BPB		10
#define PE_BAND2_4BPB		11
#define PE_BAND2_5BPB		12
#define PE_BAND2_6BPB		13
#define PE_BAND2_7BPB		14
#define PE_BAND2_8BPB		15

typedef  struct link_integ
{
	ilcp_t8hdr_t	h;
	uint8		li_version;
	uint8		li_pad;		/* for alignment */
	uint16		next_ethertype; /* 0 - shows control, no encaps */
} PACKED link_integ_t;

#define LI_MIN_LENGTH (sizeof(link_integ_t) - sizeof(ilcp_t8hdr_t))

#define LI_VERSION		0

#define LIST_TYPE_NONE		0	/* no list follows */
#define LIST_TYPE_CIS		1	/* Cardbus Information Structure */
#define LIST_TYPE_EPI		2	/* Epigram TLV */

/* li_force_ctr range: 32 to 63 */
#define LI_FORCE_MASK		0x1f
#define LI_FORCE_MIN		0x20

typedef  struct _csa
{
	ilcp_t8hdr_t	h;
	uint8		csa_version;		/* CSA version - 0 */
	uint8		csa_id_space;
	uint16		csa_mfr_id;
	uint16		csa_part_no;
	uint8		csa_hw_revision;
	uint8		csa_opcode;
	uint16		csa_mtu;		/* default is 1526 */
	struct ether_addr csa_sa;
	uint16		csa_pad;
	uint32		csa_cur_tx_set;
	uint32		csa_old_tx_set;
	uint32		csa_cur_rx_set;
	uint16		next_ethertype;
} PACKED csa_t;

#define CSA_MIN_LENGTH (sizeof(csa_t) - sizeof(ilcp_t8hdr_t))

#define CSA_MTU_DEFAULT		1526

#define CSA_VERSION		0

#define ID_SPACE_OTHER		0
#define ID_SPACE_JEDEC		1
#define ID_SPACE_PCI		2

#define CSA_OP_ANNOUNCE		0
#define CSA_OP_REQUEST		1

/*
 * HPNA versions detected or announced
 */
#define CSA_F_VER_B0		(uint32)0x00000001
#define CSA_F_VER_B1		(uint32)0x00000002
#define CSA_F_VER_B2		(uint32)0x00000004
#define CSA_F_VER_MASK		(CSA_F_VER_B0 | CSA_F_VER_B1 | CSA_F_VER_B2)
#define CSA_F_V2		(CSA_F_VER_B1) /* 2 */
#define CSA_F_V1		(CSA_F_VER_B0) /* 1 */
/*
 * Configuration/management options - force operational mode
 */
#define CSA_F_CFG_V1		(uint32)0x00000020
#define CSA_F_CFG_V1M2		(uint32)0x00000040
#define CSA_F_CFG_V2		(uint32)0x00000080
/*
 * Station Capabilities
 */
#define CSA_F_OPT_4MBAUD	(uint32)0x00010000
#define CSA_F_OPT_NOV1M2	(uint32)0x00020000
/*
 * In-use link-layer tx priorities
 */
#define CSA_F_PRI_0		(uint32)0x01000000
#define CSA_F_PRI_1		(uint32)0x02000000
#define CSA_F_PRI_2		(uint32)0x04000000
#define CSA_F_PRI_3		(uint32)0x08000000
#define CSA_F_PRI_4		(uint32)0x10000000
#define CSA_F_PRI_5		(uint32)0x20000000
#define CSA_F_PRI_6		(uint32)0x40000000
#define CSA_F_PRI_7		(uint32)0x80000000

/* Used when accessing flags in scb */
#define CSA_S_OPT_NO4MBAUD	0x01
#define CSA_S_OPT_NOV1M2	0x02

#define CSA_NUM_PRIORITIES 8

/* priority 1 shouldn't be included, but makes the remap function work */
#define DEFAULT_PRIORITY_SET (CSA_F_PRI_0 | CSA_F_PRI_7)


/*******************************************************************************
 * ARQ header 
 ******************************************************************************/

/*
 * A 32-bit ARQ header that begins with a 16-bit Ethertype field,
 * has only one sequence number, its own copy of frame priority,
 * and an 'extension bit' which is zero for basic ARQ.
 *
 * 16 bits ethertype (big-endian byte order)
 * 1 bit repeated nack/rtx flag
 * 3 bits nack count
 * 1 bit control pkt flag
 * 3 bits priority
 * 8 bits sequence number
 */
typedef  struct arq_header 
{
	ilcp_t8hdr_t	h;
	uint8		arq_version;

	uint8		arq_ctlbits;
	union
	{
		struct 
		{
			uint8 seqh_rsv;
			uint8 seql;
		} PACKED u8;
		uint16 seq16;
	} PACKED arq_seq;
	uint16		next_ethertype;
} PACKED arq_header_t;

#define LARQ_V0_LENGTH (sizeof(arq_header_t) - sizeof(ilcp_t8hdr_t))

typedef  struct arq_nack_header 
{
	ilcp_t8hdr_t	h;
	uint8		arq_version;

	uint8		arq_ctlbits;
	union 
	{
		struct 
		{
			uint8 seqh_rsv;
			uint8 seql;
		} PACKED u8;
		uint16 seq16;
	} PACKED arq_seq;
	uint8	arq_orig_dhost[ETHER_ADDR_LEN];
	uint16	next_ethertype;
} PACKED arq_nack_header_t;

#define LARQ_V0_NACK_LENGTH (sizeof(arq_nack_header_t) - sizeof(ilcp_t8hdr_t))


#define ARQ_MULT_SHIFT	(7)
#define ARQ_MULT_MASK	(1 << ARQ_MULT_SHIFT)
#define ARQ_NACK_SHIFT	(4)
#define ARQ_NACK_MASK	(7 << ARQ_NACK_SHIFT)
#define ARQ_NACK_MAX	(7)

#define ARQ_CTL_MASK	(1 << 3)
#define ARQ_DATAPKT_RTX_MASK	(1 << 6)
#define ARQ_DATAPKT_NEWSEQ_MASK	(1 << 5)
#define ARQ_DATAPKT_NORTX_MASK	(1 << 4)
#define ARQ_PRI_MASK	(7)
#define ARQ_PRI_MAX	(7)

#define ARQ_SEQ_NBITS 12
#define ARQ_SEQ_MASK	((1 << ARQ_SEQ_NBITS) - 1)

#define ARQ_RSV_NBITS (16 - ARQ_SEQ_NBITS)
#define ARQ_RSV_SHIFT (ARQ_SEQ_NBITS)
#define ARQ_RSV_MASK (((1 << (ARQ_RSV_NBITS)) - 1) << ARQ_SEQ_NBITS)

#define ARQ_GETPRI(ctl) ((ctl) & ARQ_PRI_MASK)
#define ARQ_SETPRI(ctl, pri) \
		ctl = (((pri) & ARQ_PRI_MASK) | ((ctl) & ~ARQ_PRI_MASK))
#define ARQ_GETPRI_HDR(parq) ARQ_GETPRI((parq)->arq_ctlbits)
#define ARQ_SETPRI_HDR(parq, pri) ARQ_SETPRI((parq)->arq_ctlbits, pri)

#define ARQ_GETNACK(ctl) (((ctl) & ARQ_NACK_MASK) >> ARQ_NACK_SHIFT)
#define ARQ_SETNACK(ctl, cnt) \
		ctl = ((((cnt) << ARQ_NACK_SHIFT) & ARQ_NACK_MASK) | ((pctl) & ~(ARQ_NACK_MASK)))
#define ARQ_GETNACK_HDR(parq) ARQ_GETNACK((parq)->arq_ctlbits)
#define ARQ_SETNACK_HDR(parq, pri) ARQ_SETNACK((parq)->arq_ctlbits, pri)

#define ARQ_GETMULT(ctl) (((ctl) & ARQ_MULT_MASK) >> ARQ_MULT_SHIFT)
#define ARQ_GETMULT_HDR(parq) ARQ_GETMULT((parq)->arq_ctlbits)

#define ARQ_GETSEQ(seq) (ntoh16((seq)) & ARQ_SEQ_MASK)
#define ARQ_SETSEQ(lseq, rseq) \
		(lseq = hton16((uint16)(((rseq) & ARQ_SEQ_MASK) | (ntoh16((uint16)lseq) & ~ARQ_SEQ_MASK))))
#define ARQ_GETSEQ_HDR(parq) (ntoh16((parq->arq_seq.seq16)) & ARQ_SEQ_MASK)
#define ARQ_SETSEQ_HDR(parq, rseq) \
		(parq->arq_seq.seq16 = hton16((uint16)(((rseq) & ARQ_SEQ_MASK) | \
		(ntoh16((uint16)parq->arq_seq.seq16) & ~ARQ_SEQ_MASK))))

#define ARQ_GETRSV(seq) ((ntoh16((seq)) >> ARQ_RSV_SHIFT) & (ARQ_RSV_MASK >> ARQ_RSV_SHIFT))
#define ARQ_SETRSV(lrsv, rrsv) \
		(lrsv = hton16((uint16)((((rrsv) << ARQ_RSV_SHIFT) & ARQ_RSV_MASK) | \
		(ntoh16((uint16)lrsv) & ~ARQ_RSV_MASK))))

#define LARQ_VERSION 0

/*
 * Vendor-specific frame headers
 */

typedef  struct vss_hdr
{
	ilcp_t8hdr_t	h;
	uint8		vss_version;
	uint8		vss_oui[3];
	uint8		vss_cmd;
	uint8		vss_subcmd;
	union 
	{
		uint16	args[1];
		uint16	next_ethertype;
	} PACKED vss_u;
} PACKED vss_hdr_t;

#define VSS_MIN_LENGTH (sizeof(vss_hdr_t) - sizeof(ilcp_t8hdr_t))
#define VSS_VERSION		0

typedef  struct vsl_hdr
{
	ilcp_t16hdr_t	h;
	uint8		vsl_version;
	uint8		vsl_oui[3];
	uint8		vsl_cmd;
	uint8		vsl_subcmd;
	union 
	{
		uint16	args[1];
		uint16	next_ethertype;
	} PACKED vsl_u;
} PACKED vsl_hdr_t;

#define VSL_MIN_LENGTH (sizeof(vsl_hdr_t) - sizeof(ilcp_t16hdr_t))
#define VSL_VERSION		0

/* Frame Length Header */
typedef struct flh_hdr
{
	ilcp_t8hdr_t	h;
	uint8		flh_version;
	uint8           flh_pad;
	uint16          flh_length;
	uint16          next_ethertype;
} PACKED flh_hdr_t;

#define FLH_MIN_LENGTH (sizeof(flh_hdr_t) - sizeof(ilcp_t8hdr_t))
#define	FLH_LEN		sizeof(flh_hdr_t)

/*
 * Our own OUI, plus some macros, to go with the commands defined below
 */
#define EPI_OUI_0 0x00
#define EPI_OUI_1 0x90
#define EPI_OUI_2 0x4c

#define SET_EPI_OUI(poui) \
		((poui)[0] = EPI_OUI_0, (poui)[1] = EPI_OUI_1, (poui)[2] = EPI_OUI_2)

#define MATCH_EPI_OUI(poui) \
		((poui)[0] == EPI_OUI_0 && (poui)[1] == EPI_OUI_1 && (poui)[2] == EPI_OUI_2)

/*
 * Command list for Vendor specific frames with Epigram OUI 
 */
#define VSS_CMD_DIAG 0
#define VSS_DIAG_ECHO_REQ 0
#define VSS_DIAG_ECHO_RSP 1
#define	VSS_CMD_FLH	5

/*
 * Cert/Diag protocol
 */

/* Cert/Diag command segment */
typedef struct ilcp_v0_cds {
	uint16	CSType;
	uint16	CSLength;			/* top 5 bits rsv, bottom 11 bits are nbytes */
	union {
		uint32 CSPad0;
		struct {
			uint8 CSData1;
			uint8 CSPad1[3];
		} PACKED D1;
		struct {
			uint16 CSData2;
			uint16 CSPad2;
		} PACKED D2;
		struct {
			uint8 CSData3[3];
			uint8 CSPad3;
		} PACKED D3;
		struct {
			uint32 CSData4;
		} PACKED D4;
	} PACKED CSData;
} PACKED ilcp_v0_cds_t;

typedef struct ilcp_v0_cds_mac {
	uint16	CSType;
	uint16	CSLength;
	uint8	mac[6];
	uint16 CSPad;
} PACKED ilcp_v0_cds_mac_t;

#define CS_V0_MIN_LEN			8

/* Cert/Diag basic frame - minimum frame has 1 min-size command segment */
typedef struct ilcp_v0_cdcf {
	ilcp_t16hdr_t	h;
	uint8	LSVersion;
	uint8	OpCode;
	uint16	Cert_Xsum;
	uint16	Cert_Pad;
	uint16	Cert_Seq_Tag;
	ilcp_v0_cds_t CommandSeg;
	uint16	NextEthertype;
} PACKED ilcp_v0_cdcf_t;

#define CDCF_V0_MIN_LEN			(10 + CS_V0_MIN_LEN)

#define CERT_SEQ_WRAP_MASK		0x8000
#define CERT_SEQ_MOD_MASK		0x10000

/* 1 msec */
#define CERT_TO_MSEC			64000

/* OpCodes */
/* required on all nodes that support CERT */
#define CDCF_V0_OPCODE_OK		0
#define CDCF_V0_OPCODE_ERROR		1
#define CDCF_V0_OPCODE_TESTDATA		2

#define CDCF_V0_OPCODE_ENABLE		8
#define CDCF_V0_OPCODE_DISABLE		9

#define CDCF_V0_OPCODE_CONFIGNODE	16
#define CDCF_V0_OPCODE_CONFIGSEND	17
#define CDCF_V0_OPCODE_STARTSEND	18
#define CDCF_V0_OPCODE_STOPSEND		19
#define CDCF_V0_OPCODE_ECHOREQUEST	20
#define CDCF_V0_OPCODE_CONFIGRECV	21
#define CDCF_V0_OPCODE_STOPRECV		22
#define CDCF_V0_OPCODE_DUMPSTATS	23
#define CDCF_V0_OPCODE_DUMPCONFIG	24
#define CDCF_V0_OPCODE_DUMP		25

#define CDCF_V0_OPCODE_REPORTNODE	32

#define CDCF_V0_OPCODE_VENDOR		64

/* range > 128 for vendor-specific opcodes */
#define CDCF_V0_OPCODE_DUMPSCB		130
#define CDCF_V0_OPCODE_DUMPLARQ		131
#define CDCF_V0_OPCODE_READREG		132
#define CDCF_V0_OPCODE_WRITEREG		133
#define CDCF_V0_OPCODE_WRITEREGS	134
#define CDCF_V0_OPCODE_CLEARREGS	135
#define CDCF_V0_OPCODE_CHEST		136

/* Command Segment IDs */
#define CDCF_V0_CSTYPE_NULL		0x0000	/* no data in this segment */
#define CDCF_V0_CSLEN_NULL		4
#define CDCF_V0_CSTYPE_ERROR		0x0001	/* error code */
#define CDCF_V0_CSLEN_ERROR		4
#define CDCF_V0_CSTYPE_INFOREPLY	0x0002	/* frame contains reply to dump request */
#define CDCF_V0_CSLEN_INFOREPLY		2
#define CDCF_V0_CSTYPE_TEXT		0x0003	/* ASCII text - variable len */
#define CDCF_V0_CSTYPE_DATA		0x0004	/* test data - variable len */

#define CDCF_V0_CSTYPE_REFSEQ		0x0005


#define CDCF_V0_CSTYPE_TXPE		0x0010
#define CDCF_V0_CSTYPE_TXPRI		0x0011
#define CDCF_V0_CSTYPE_LINKINT		0x0012
#define CDCF_V0_CSTYPE_TXDOWN		0x0013
#define CDCF_V0_CSTYPE_TXINHIBIT	0x0014
#define CDCF_V0_CSTYPE_TXUP		0x0015
#define CDCF_V0_CSTYPE_HPNAMODE		0x0016

/* required if node supports LARQ */
#define CDCF_V0_CSTYPE_LARQ		0x0020
/* required if node supports CSA */
#define CDCF_V0_CSTYPE_CSA		0x0021
#define CDCF_V0_CSTYPE_CSAHPNAMODE	0x0022
#define CDCF_V0_CSTYPE_OUI		0x0023

#define CDCF_V0_CSTYPE_VENDOR		0x0040

#define CDCF_V0_CSTYPE_SA		0x0081
#define CDCF_V0_CSLEN_SA		6
#define CDCF_V0_CSTYPE_NUM_DA		0x0082
#define CDCF_V0_CSLEN_NUM_DA		1
#define CDCF_V0_CSTYPE_DA		0x0083
#define CDCF_V0_CSLEN_DA		6
#define CDCF_V0_CSTYPE_DGEN_TYPE	0x0084
#define CDCF_V0_CSTYPE_DGEN_DATA	0x0085
#define CDCF_V0_CSTYPE_LENGTH		0x0086
#define CDCF_V0_CSTYPE_NPKTS		0x0087
#define CDCF_V0_CSTYPE_BURST_INT	0x0088
#define CDCF_V0_CSTYPE_BURST_NPKTS	0x0089
#define CDCF_V0_CSTYPE_NUMACKS		0x008a

#define CDCF_V0_CSTYPE_DATA_XSUM	0x008d
#define CDCF_V0_CSTYPE_TAG_XSUM		0x008e

#define CDCF_V0_CSTYPE_XMTFRAMES	0x0101	/* total xmt frames w/o err */
#define CDCF_V0_CSTYPE_XMTBYTES		0x0102	/* total xmt bytes w/o err */
#define CDCF_V0_CSTYPE_XMTERR		0x0103	/* transmit errors */
#define CDCF_V0_CSTYPE_XMTETIME		0x0104	/* elapsed time from xmt of first frame
						   to xmt of first 'end' frame */
#define CDCF_V0_CSTYPE_RCVFRAMES	0x0105	/* total rcv frames w/o err */
#define CDCF_V0_CSTYPE_RCVBYTES		0x0106	/* total rcv bytes w/o */
#define CDCF_V0_CSTYPE_RCVSEQMISS	0x0107	/* missed seq */
#define CDCF_V0_CSTYPE_RCVSEQERR	0x0108	/* rcv seq out of order */
#define CDCF_V0_CSTYPE_RCVDATAERR	0x0109	/* rcv data compare error */
#define CDCF_V0_CSTYPE_RCVERR		0x010a	/* any other errors */
#define CDCF_V0_CSTYPE_RCVETIME		0x010b	/* elapsed time from recv of first frame
						   to recv of first 'end' frame */
#define CDCF_V0_CSTYPE_RCVXSUMERR	0x010c	/* rcv checksum error */

#define CDCF_V0_CSTYPE_REGBASE		0x8000	/* base ID for reg addresses */

/* Broadcom (vendor) Specific codes */
/* These only work on the bridge */
#define CDCF_V0_BCM_RESTART		0x0201	/* restart system */
#define CDCF_V0_BCM_CFG_IP		0x0202	/* set IP */
#define CDCF_V0_BCM_CFG_IL_MAC		0x0203	/* set iLine MAC */
#define CDCF_V0_BCM_CFG_ETH_MAC		0x0204	/* set ethernet MAC */
#define CDCF_V0_BCM_DL_START		0x0205	/* Start download */
#define CDCF_V0_BCM_DL_DATA		0x0206	/* tftp data */
#define CDCF_V0_BCM_DL_DONE		0x0207	/* Download complete */
#define CDCF_V0_BCM_FW_QUERY		0x0208	/* Query firmware version */
/* These work on all platforms */
#define CDCF_V0_BCM_SETMSGLEVEL		0x0401	/* set debug message level */

/* Response to REPORTNODE: */
#define CDCF_V0_CSTYPE_PRI_ID		0x8301	/* primary vendor/device ID */
#define CDCF_V0_CSLEN_PRI_ID		4
#define CDCF_V0_CSTYPE_SUB_ID		0x8302	/* subsystem vendor/device ID */
#define CDCF_V0_CSLEN_SUB_ID		4
#define CDCF_V0_CSTYPE_MAC		0x8303	/* MAC address */
#define CDCF_V0_CSTYPE_SN		0x8304	/* serial number */
#define CDCF_V0_CSLEN_SN		32
#define CDCF_V0_CSTYPE_DEV_TYPE		0x8305	/* device type */
#define CDCF_V0_CSLEN_DEV_TYPE		32
#define CDCF_V0_CSTYPE_VEND_NAME	0x8306	/* vendor name */
#define CDCF_V0_CSLEN_VEND_NAME		32
#define CDCF_V0_CSTYPE_DRV_VERSION	0x8307	/* vendor driver ID/name */
#define CDCF_V0_CSLEN_DRV_VERSION	32
#define CDCF_V0_CSTYPE_DRV_DATE		0x8308	/* vendor driver date */
#define CDCF_V0_CSLEN_DRV_DATE		32
#define CDCF_V0_CSTYPE_MFG_DATE		0x8309	/* device date of mfg */
#define CDCF_V0_CSLEN_MFG_DATE		32

/* Device types for CDCF_V0_CSTYPE_DEV_TYPE: */
#define	CDCF_V0_DEVICE_RESERVED		0	/* Reserved */
#define	CDCF_V0_DEVICE_PCINIC		1	/* PCI (miniPCI, Cardbus) NIC */
#define	CDCF_V0_DEVICE_USBNIC		2	/* USB NIC */
#define	CDCF_V0_DEVICE_BRIDGE		10	/* Other Bridges */
#define	CDCF_V0_DEVICE_CM_BRIDGE	11	/* Cable Modem Bridge */
#define	CDCF_V0_DEVICE_DSL_BRIDGE	12	/* DSL Modem Bridge */
#define	CDCF_V0_DEVICE_WL_BRIDGE	13	/* Wireless Bridge */
#define	CDCF_V0_DEVICE_V90_BRIDGE	14	/* V90 Bridge */
#define	CDCF_V0_DEVICE_ETH_BRIDGE	15	/* Ethernet Bridge */
#define	CDCF_V0_DEVICE_HPNA_BRIDGE	16	/* HPNA Bridge */
#define	CDCF_V0_DEVICE_ROUTER		20	/* Other Router */
#define	CDCF_V0_DEVICE_CM_ROUTER	21	/* Cable Modem Router */
#define	CDCF_V0_DEVICE_DSL_ROUTER	22	/* DSL Router */
#define	CDCF_V0_DEVICE_WL_ROUTER	23	/* Wireless Router */
#define	CDCF_V0_DEVICE_V90_ROUTER	24	/* V90 Router */
#define	CDCF_V0_DEVICE_ETH_ROUTER	25	/* Ethernet Router */
#define	CDCF_V0_DEVICE_HPNA_ROUTER	26	/* HPNA Router */
#define	CDCF_V0_DEVICE_DISK		30	/* Disk Device */
#define	CDCF_V0_DEVICE_TAPE		31	/* Backup Device */
#define	CDCF_V0_DEVICE_CD_DVD		32	/* CD/DVD Device */
#define	CDCF_V0_DEVICE_PRINTER		33	/* Printer */
#define	CDCF_V0_DEVICE_PRTSVR		34	/* Print Server */
#define	CDCF_V0_DEVICE_SCANNER		35	/* Scanner */
#define	CDCF_V0_DEVICE_FAX		36	/* FAX */
#define	CDCF_V0_DEVICE_POTS		40	/* Phone */
#define	CDCF_V0_DEVICE_AUDIO_SRC	41	/* Audio Source Device */
#define	CDCF_V0_DEVICE_SPEAKER		42	/* Audio Out Device */
#define	CDCF_V0_DEVICE_VIDEO_SRC	43	/* Video Source Device */
#define	CDCF_V0_DEVICE_DISPLAY		44	/* Video Out Device */
#define	CDCF_V0_DEVICE_CABLE		45	/* Cable Set-top box */
#define	CDCF_V0_DEVICE_SAT		46	/* Satellite Set-top box */
#define	CDCF_V0_DEVICE_OTHER		255	/* Unspecified */

/* Error codes */
#define CDCF_V0_ERR_UNK			1		/* generic error */
#define CDCF_V0_ERR_UNSUP_OP		2		/* unsupported opcode */
#define CDCF_V0_ERR_INVALID_PARAM	3		/* invalid parameter (config value or
							   data gen setting) */
#define CDCF_V0_ERR_UNSUP_CMDSEG	4		/* unsupported command segment type */
#define CDCF_V0_ERR_UNSUP_DGEN		5		/* unsupported data generator */
#define CDCF_V0_ERR_INVALID_SEQ		6		/* unsupported tag generator */
#define CDCF_V0_ERR_INVALID_FRAME	7		/* command frame was malformed */
#define CDCF_V0_ERR_INVALID_OP		8		/* invalid opcode (not valid in
							   node's current state) */

#define CDCF_V0_ERR_INCORRECT_OUI 	9		/* Not our OUI */

/* Data Generators */
#define CDCF_V0_DGEN_FIXED		1		/* CC_data repeaded to fill packet */ 
#define CDCF_V0_DGEN_RND_F0		2		/* Fixed size frames, data: LFSR, 
							   init w/CC_data, poly #0 tbd */
#define CDCF_V0_DGEN_RND_V0		3		/* Var size (min to CC_length, uniform), 
							   data: LFSR, init w/CC_data, poly tbd */ 
#define CDCF_V0_DGEN_RND_F1		4		/* Fixed size frames, data: LFSR,
							   init w/CC_data, poly #1 tbd */
#define CDCF_V0_DGEN_RND_V2		5		/* Var size (min to CC_length, uniform),
							   data: LFSR, init w/CC_data, poly tbd */

#undef PACKED
#if !defined(__GNUC__)
#pragma pack()
#endif

#endif /* _ILCP_H_ */
