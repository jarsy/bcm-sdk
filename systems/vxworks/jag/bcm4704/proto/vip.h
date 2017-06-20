/*******************************************************************************
 * $Id: vip.h,v 1.1 2004/02/24 07:47:01 csm Exp $
 * vip.h - top level definitions for iLine10 protocol
 ******************************************************************************/

#ifndef _VIP_H_
#define _VIP_H_

#include "typedefs.h"

/*******************************************************************************
 * header defs
 ******************************************************************************/

#define VIP_HDR_NBITS_PER_BAUD 2
#define VIP_HDR_NBAUDS_PER_BYTE (8/VIP_HDR_NBITS_PER_BAUD)

/* header part 1 field lengths */
#define VIP_HDR_TRN_CNT       3   /* number of copies of training pattern */
#define VIP_HDR_EXCESS_NBAUDS 16  /* number of additional symbols preceding
                                     the first "true" preamble sequence,
                                     needed for the gain control */
#define VIP_HDR_EXCESS_NBYTES \
    ((VIP_HDR_EXCESS_NBAUDS * VIP_HDR_NBITS_PER_BAUD) / 8)

#define VIP_HDR_TRN_NBAUDS  16 /* number of preamble bauds */
#define VIP_HDR_TRN_NBYTES  ((VIP_HDR_TRN_NBAUDS * VIP_HDR_NBITS_PER_BAUD)/8)
#define VIP_HDR_PRE_NBAUDS  (VIP_HDR_EXCESS_NBAUDS + \
                              (VIP_HDR_TRN_CNT * VIP_HDR_TRN_NBAUDS))
#define VIP_HDR_PRE_NBYTES  ((VIP_HDR_PRE_NBAUDS * VIP_HDR_NBITS_PER_BAUD)/8)
#define VIP_HDR_RATE        2  /* hdr baud rate normalized in units of MHz */

/* header includes everything modulated at 2 bits/baud */
#define VIP_HDR_FC_NBYTES   4 /* frame control */
#define VIP_HDR_DST_NBYTES  6 /* dst mac address */
#define VIP_HDR_SRC_NBYTES  6 /* src mac address */
#define VIP_HDR_TYPE_NBYTES 2 /* ethernet type */

/* total header bytes at the minimum payload encoding */
#define VIP_HDR_MINPE_NBYTES (VIP_HDR_FC_NBYTES + VIP_HDR_DST_NBYTES + \
    VIP_HDR_SRC_NBYTES + VIP_HDR_TYPE_NBYTES)

/* expressed in bauds */
#define VIP_HDR_FC_NBAUDS   ((VIP_HDR_FC_NBYTES * 8) / VIP_HDR_NBITS_PER_BAUD)
#define VIP_HDR_DST_NBAUDS  ((VIP_HDR_DST_NBYTES * 8) / VIP_HDR_NBITS_PER_BAUD)
#define VIP_HDR_SRC_NBAUDS  ((VIP_HDR_SRC_NBYTES * 8) / VIP_HDR_NBITS_PER_BAUD)
#define VIP_HDR_TYPE_NBAUDS ((VIP_HDR_TYPE_NBYTES * 8) / VIP_HDR_NBITS_PER_BAUD)

/* break down frame control bits for frame type 0 */
#define VIP_HDR_FT_NBITS    8 /* frame type == 0 */
#define VIP_HDR_SI_NBITS    4 /* frame type 0: scrambler init */
#define VIP_HDR_PRI_NBITS   3 /* frame type 0: priority */
#define VIP_HDR_ALT_NBITS   1 /* frame type 0: alternate encoding */
#define VIP_HDR_TXD_NBITS   8 /* frame type 0: transmit descriptor */
#define VIP_HDR_CHK_NBITS   8 /* frame type 0: CRC8 header check */

/* byte offsets to fields from start of header */
#define VIP_HDR_FC_OFF      (VIP_HDR_PRE_NBYTES + 0)
#define VIP_HDR_FT_OFF      (VIP_HDR_PRE_NBYTES + 0)
#define VIP_HDR_SI_OFF      (VIP_HDR_PRE_NBYTES + 1)
#define VIP_HDR_PRI_OFF     (VIP_HDR_PRE_NBYTES + 1)
#define VIP_HDR_ALT_OFF     (VIP_HDR_PRE_NBYTES + 1)
#define VIP_HDR_TXD_OFF     (VIP_HDR_PRE_NBYTES + 2)
#define VIP_HDR_CHK_OFF     (VIP_HDR_PRE_NBYTES + 3)
#define VIP_HDR_DST_OFF     (VIP_HDR_FC_OFF  + VIP_HDR_FC_NBYTES)   
#define VIP_HDR_SRC_OFF     (VIP_HDR_DST_OFF + VIP_HDR_DST_NBYTES)
#define VIP_HDR_TYPE_OFF    (VIP_HDR_SRC_OFF + VIP_HDR_SRC_NBYTES)

#define VIP_HDR_PE_OFF      (VIP_HDR_PRE_NBYTES + 2)

/* values for fixed fields */
#if !defined(CAP)
#define VIP_HDR_TRN  0xfc483084
#define VIP_HDR_TRNR 0x3084fc48
#else /* CAP */
#define VIP_HDR_TRN  0x3084fc48
#endif /* !CAP */

/* Guard is 1st byte of src addr, 0x00 for Epigram MAC addrs */
#define VIP_HDR_GRD 0x00

/* ranges for variable fields */
#define VIP_HDR_MIN_SI    0
#define VIP_HDR_MAX_SI    ((1 << VIP_HDR_SI_NBITS) - 1)
#define VIP_HDR_MIN_PRI   0
#define VIP_HDR_MAX_PRI   ((1 << VIP_HDR_PRI_NBITS) - 1)
#define VIP_HDR_MIN_ALT   0
#define VIP_HDR_MAX_ALT   0

/* for frame type 0 TXD is essentially TXC */
#define VIP_HDR_MIN_TXC   VIP_DATA_MIN_NBITS_PER_BAUD
#define VIP_HDR_MAX_TXC   VIP_DATA_MAX_NBITS_PER_BAUD
#define VIP_HDR_MIN_TXD   (VIP_HDR_MIN_TXC - 1)
#define VIP_HDR_MAX_TXD   (VIP_HDR_MAX_TXC - 1)

/* 
	spectral mask values 
	1	== 4-10MHz transmit band (RE10)
	2	== 4-21MHz transmit band (RE100)
*/
#define VIP_HDR_MIN_SPECTRAL_MASK 1
#define VIP_HDR_MAX_SPECTRAL_MASK 2
#define VIP_HDR_SPECTRAL_MASK1 1
#define VIP_HDR_SPECTRAL_MASK2 2

/* get/set macros */
#define VIP_HDR_GET_PFC(phdr) \
    ((uint8 *)(phdr) + VIP_HDR_FC_OFF)

#define VIP_HDR_GET_FT(phdr) \
     (((uint8 *)(phdr))[VIP_HDR_FT_OFF])

#define VIP_HDR_GET_SI(phdr) \
     (((uint8 *)(phdr))[VIP_HDR_SI_OFF] & 0xf)

#define VIP_HDR_GET_PRI(phdr) \
     ((((uint8 *)(phdr))[VIP_HDR_PRI_OFF] >> 4) & 0x7)

#define VIP_HDR_GET_ALT(phdr) \
     ((((uint8 *)(phdr))[VIP_HDR_ALT_OFF] >> 7) & 0x1)

#define VIP_HDR_GET_TXC(phdr) \
     ((((uint8 *)(phdr))[VIP_HDR_TXD_OFF] & 0x7) + 1)

#define VIP_HDR_GET_PAYLOAD_RATE(phdr) \
     (1 << (((((uint8 *)(phdr))[VIP_HDR_TXD_OFF] & 0x18) >> 3) + 1))

#define VIP_HDR_GET_RE20_PAYLOAD_RATE(phdr) \
	(1 << (((((uint8 *)(phdr))[VIP_HDR_TXD_OFF] & 0x8) >> 3) + 1))

#define VIP_HDR_GET_SPECTRAL_MASK(phdr) \
	 (((((uint8 *)(phdr))[VIP_HDR_TXD_OFF] & 0x60) >> 5) + 1)

#define VIP_HDR_GET_PE(phdr) \
     (((uint8 *)(phdr))[VIP_HDR_PE_OFF] & 0xf)

#define VIP_HDR_VALIDATE_RE20_PE(phdr) \
	 ((((uint8 *)(phdr))[VIP_HDR_PE_OFF] & 0xf0) == 0 )

#define VIP_HDR_GET_CHK(phdr) \
     (((uint8 *)(phdr))[VIP_HDR_CHK_OFF])

#define VIP_HDR_GET_PDST(phdr) \
    ((uint8 *)(phdr) + VIP_HDR_DST_OFF)

#define VIP_HDR_GET_PSRC(phdr) \
    ((uint8 *)(phdr) + VIP_HDR_SRC_OFF)

#define VIP_HDR_GET_TYPE(phdr) \
     ((uint16)((((uint8 *)(phdr))[VIP_HDR_TYPE_OFF + 0] << 8) | \
               (((uint8 *)(phdr))[VIP_HDR_TYPE_OFF + 1] << 0)))

#define VIP_HDR_SET_PRE(phdr, trn)                   \
     /* excess preamble bytes first */                \
     (((uint8 *)(phdr))[0]) = (((trn) >> 24) & 0xff); \
     (((uint8 *)(phdr))[1]) = (((trn) >> 16) & 0xff); \
     (((uint8 *)(phdr))[2]) = (((trn) >> 8) & 0xff);  \
     (((uint8 *)(phdr))[3]) = ((trn) & 0xff);         \
     /* remaining preamble bytes */                   \
     (((uint8 *)(phdr))[0 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 24) & 0xff); \
     (((uint8 *)(phdr))[1 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 16) & 0xff); \
     (((uint8 *)(phdr))[2 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 8) & 0xff);  \
     (((uint8 *)(phdr))[3 + VIP_HDR_EXCESS_NBYTES]) = ((trn) & 0xff);         \
     (((uint8 *)(phdr))[4 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 24) & 0xff); \
     (((uint8 *)(phdr))[5 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 16) & 0xff); \
     (((uint8 *)(phdr))[6 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 8) & 0xff);  \
     (((uint8 *)(phdr))[7 + VIP_HDR_EXCESS_NBYTES]) = ((trn) & 0xff);         \
     (((uint8 *)(phdr))[8 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 24) & 0xff); \
     (((uint8 *)(phdr))[9 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 16) & 0xff); \
     (((uint8 *)(phdr))[10 + VIP_HDR_EXCESS_NBYTES]) = (((trn) >> 8) & 0xff); \
     (((uint8 *)(phdr))[11 + VIP_HDR_EXCESS_NBYTES]) = ((trn) & 0xff)

#define VIP_HDR_SET_FT(phdr, ft) \
    (((uint8 *)(phdr))[VIP_HDR_FT_OFF]) = (ft)

#define VIP_HDR_SET_SI(phdr, si) \
     (((uint8 *)(phdr))[VIP_HDR_SI_OFF]) &= ~0x0f; \
     (((uint8 *)(phdr))[VIP_HDR_SI_OFF]) |= ((si) & 0x0f)

#define VIP_HDR_SET_PRI(phdr, pri) \
     (((uint8 *)(phdr))[VIP_HDR_PRI_OFF]) &= ~0x70; \
     (((uint8 *)(phdr))[VIP_HDR_PRI_OFF]) |= (((pri) & 0x07) << 4)

#define VIP_HDR_SET_ALT(phdr, alt) \
     (((uint8 *)(phdr))[VIP_HDR_ALT_OFF]) &= ~0x80; \
     (((uint8 *)(phdr))[VIP_HDR_ALT_OFF]) |= (((alt) & 0x01) << 7)

#define VIP_HDR_SET_TXC(phdr, txc) \
    (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) &= ~0x07; \
    (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= ((txc) - 1)

#define VIP_HDR_SET_PAYLOAD_RATE(phdr, payload_rate) \
    (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) &= ~0x18; \
    if (payload_rate == 4 ) { \
        (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= (1 << 3); \
    } else if(payload_rate == 8 ) { \
        (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= (2 << 3); \
    } else if(payload_rate == 16) {\
        (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= (3 << 3); \
    }

#define VIP_HDR_SET_SPECTRAL_MASK(phdr, spectral_mask) \
	(((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) &= ~0x60; \
    (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= ( ((spectral_mask) - 1) << 5 )

#define VIP_HDR_SET_CODING(phdr, coding) \
	(((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) &= ~0x80; \
    (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= ( (coding) << 7 )

#define VIP_HDR_SET_PE(phdr, txc, payload_rate) \
    (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) = ((uint8) 0x00); \
    (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= ((txc) - 1); \
    if (payload_rate == 4 ) { \
        (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= (1 << 3); \
    } else if(payload_rate == 8 ) { \
        (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= (2 << 3); \
    } else if(payload_rate == 16) {\
        (((uint8 *)(phdr))[VIP_HDR_TXD_OFF]) |= (3 << 3); \
    }

#define VIP_HDR_SET_CHK(phdr, chk) \
    (((uint8 *)(phdr))[VIP_HDR_CHK_OFF]) = (chk)

#define VIP_HDR_SET_DST(phdr, pdst) \
    (((uint8 *)(phdr))[VIP_HDR_DST_OFF + 0]) = (((uint8 *)(pdst))[0]); \
    (((uint8 *)(phdr))[VIP_HDR_DST_OFF + 1]) = (((uint8 *)(pdst))[1]); \
    (((uint8 *)(phdr))[VIP_HDR_DST_OFF + 2]) = (((uint8 *)(pdst))[2]); \
    (((uint8 *)(phdr))[VIP_HDR_DST_OFF + 3]) = (((uint8 *)(pdst))[3]); \
    (((uint8 *)(phdr))[VIP_HDR_DST_OFF + 4]) = (((uint8 *)(pdst))[4]); \
    (((uint8 *)(phdr))[VIP_HDR_DST_OFF + 5]) = (((uint8 *)(pdst))[5])

#define VIP_HDR_SET_SRC(phdr, psrc) \
    (((uint8 *)(phdr))[VIP_HDR_SRC_OFF + 0]) = (((uint8 *)(psrc))[0]); \
    (((uint8 *)(phdr))[VIP_HDR_SRC_OFF + 1]) = (((uint8 *)(psrc))[1]); \
    (((uint8 *)(phdr))[VIP_HDR_SRC_OFF + 2]) = (((uint8 *)(psrc))[2]); \
    (((uint8 *)(phdr))[VIP_HDR_SRC_OFF + 3]) = (((uint8 *)(psrc))[3]); \
    (((uint8 *)(phdr))[VIP_HDR_SRC_OFF + 4]) = (((uint8 *)(psrc))[4]); \
    (((uint8 *)(phdr))[VIP_HDR_SRC_OFF + 5]) = (((uint8 *)(psrc))[5])

#define VIP_HDR_SET_TYPE(phdr, type) \
    (((uint8 *)(phdr))[VIP_HDR_TYPE_OFF + 0]) = (((type) >> 8) & 0xff); \
    (((uint8 *)(phdr))[VIP_HDR_TYPE_OFF + 1]) = (((type) >> 0) & 0xff)

/* get parts of the PE field given just the PE */

#define VIP_PE_TO_TXC(pe)  (((pe) & 0x07) + 1)

#define VIP_PE_TO_PAYLOAD_RATE_BITS(pe) (((pe) & 0x18) >> 3)
#define VIP_PE_TO_PAYLOAD_RATE(pe) ( \
    (VIP_PE_TO_PAYLOAD_RATE_BITS(pe) == 0) ? VIP_DATA_PAYLOAD_RATE_2MBAUD : \
    (VIP_PE_TO_PAYLOAD_RATE_BITS(pe) == 1) ? VIP_DATA_PAYLOAD_RATE_4MBAUD : \
    (VIP_PE_TO_PAYLOAD_RATE_BITS(pe) == 2) ? VIP_DATA_PAYLOAD_RATE_8MBAUD : \
    (VIP_PE_TO_PAYLOAD_RATE_BITS(pe) == 3) ? VIP_DATA_PAYLOAD_RATE_16MBAUD : \
    VIP_DATA_PAYLOAD_RATE_2MBAUD )

#define VIP_PE_TO_SPECTRAL_MASK(pe) ((((pe) & 0x60) >> 5) + 1)

/* build a PE given the parts of the PE */

#define VIP_PE_SUB(txc, spectral_mask) \
    (((txc)-1) | (((spectral_mask)-1) << 5))

#define VIP_PE(txc, payload_rate, spectral_mask) ( \
    (payload_rate == VIP_DATA_PAYLOAD_RATE_2MBAUD)  ? (VIP_PE_SUB(txc, spectral_mask)) : \
    (payload_rate == VIP_DATA_PAYLOAD_RATE_4MBAUD)  ? (VIP_PE_SUB(txc, spectral_mask) | (1 << 3)) : \
    (payload_rate == VIP_DATA_PAYLOAD_RATE_8MBAUD)  ? (VIP_PE_SUB(txc, spectral_mask) | (2 << 3)) : \
    (payload_rate == VIP_DATA_PAYLOAD_RATE_16MBAUD) ? (VIP_PE_SUB(txc, spectral_mask) | (3 << 3)) : \
    (VIP_PE_SUB(txc, spectral_mask)) )

/* total number of header bits */
#define VIP_HDR_NBITS \
    (VIP_HDR_PRE_NBAUDS * VIP_HDR_NBITS_PER_BAUD + \
     VIP_HDR_FT_NBITS  + VIP_HDR_SI_NBITS + \
     VIP_HDR_PRI_NBITS + VIP_HDR_ALT_NBITS + \
     VIP_HDR_TXD_NBITS + VIP_HDR_CHK_NBITS + \
     VIP_HDR_SRC_NBAUDS * VIP_HDR_NBITS_PER_BAUD + \
     VIP_HDR_DST_NBAUDS * VIP_HDR_NBITS_PER_BAUD + \
     VIP_HDR_TYPE_NBAUDS * VIP_HDR_NBITS_PER_BAUD)

/* total number of header bauds, bytes, and samples */
#define VIP_HDR_NBAUDS (VIP_HDR_NBITS / VIP_HDR_NBITS_PER_BAUD)
#define VIP_HDR_NBYTES (VIP_HDR_NBITS / 8)

#if (VIP_HDR_NBAUDS != VIP_HDR_NBYTES * 4)
#error "inconsistent VIP_HDR_NBAUDS, VIP_HDR_NBYTES"
#endif

#define VIP_HDR_NSAMPS(nsamps_per_baud) \
    (VIP_HDR_NBAUDS * (nsamps_per_baud))

#if defined(USE_EPITYPE)
#define VIP_DATA_MIN_TU 60
#else
/* minimum length RE frame has no ethernet payload */
#define VIP_DATA_MIN_TU 0
#endif /* USE_EPITYPE */


/* FCS followed by additional CRC16 */
#define VIP_DATA_FCS_FIELD_SIZE   4
#define VIP_DATA_CRC16_FIELD_SIZE 2
#define VIP_DATA_CRC_FIELD_SIZE \
    (VIP_DATA_FCS_FIELD_SIZE + VIP_DATA_CRC16_FIELD_SIZE)

/* supported payload encoding rates (normalized in units of MHz) */
#define VIP_DATA_PAYLOAD_RATE_2MBAUD 2
#define VIP_DATA_PAYLOAD_RATE_4MBAUD 4
#define VIP_DATA_PAYLOAD_RATE_8MBAUD 8
#define VIP_DATA_PAYLOAD_RATE_16MBAUD 16
#define VIP_RE10_DATA_NUM_PAYLOAD_RATES 2
#define VIP_DATA_NUM_PAYLOAD_RATES 4

/* 
 * Size of Ethernet MTU + Ethernet header, so we can encapsulate a complete
 * Ethernet frame, plus ARQ header.
 */

#define VIP_DATA_MTU                1518

/* min/max total data length */
#define VIP_DATA_MIN_NBYTES \
     (VIP_DATA_MIN_TU + VIP_DATA_CRC_FIELD_SIZE)
#define VIP_DATA_MAX_NBYTES \
     (VIP_DATA_MTU + VIP_DATA_CRC_FIELD_SIZE)

/* min/max data constellations */
#define VIP_DATA_MAX_NBITS_PER_BAUD 8
#define VIP_DATA_MIN_NBITS_PER_BAUD 2
#define VIP_DATA_NUM_NBITS_PER_BAUD 7

/* end-of-frame marker definitions                      */
/* VIP_EOF is the bit sequence corresponding to the     */
/* end-of-frame symbol sequence; end-of-frame symbols   */
/* taken from VIP_DATA_MIN_NBITS_PER_BAUD constellation */
#if !defined(CAP)
#define VIP_EOF        0xfc
#else /* CAP */
#define VIP_EOF        0x30
#endif /* !CAP */
#define VIP_EOF_NBAUDS 4
#define VIP_EOF_NBYTES (VIP_EOF_NBAUDS * VIP_DATA_MIN_NBITS_PER_BAUD / 8)

/* macro to set end-of-frame marker bit sequence */
#define VIP_SET_EOF(pbuf, bitseq) \
    (((uint8*)(pbuf))[0]) = (bitseq)

/* the average decision error (fixed point) is appended to each packet */
/* FIXME: for now this looks like it's replacing the crc field */
#define VIP_DATA_AVG_ERR_FIELD_SIZE VIP_DATA_CRC_FIELD_SIZE /* sizeof(int32) */
#define VIP_DATA_AVG_ERR_NFRAC_BITS 18
#define VIP_DATA_SET_AVG_ERR(pbuf, avg_err) \
     ((int32 *)pbuf)[0] = avg_err
#define VIP_DATA_GET_AVG_ERR(phdr) \
     (((int32 *)((uint8*)(phdr) + VIP_HDR_NBYTES + \
                 VIP_HDR_GET_LEN(phdr) - \
                 VIP_DATA_CRC_FIELD_SIZE))[0])

/* min/max data lengths, expressed in bauds */
#define VIP_DATA_MIN_NBAUDS \
    ((VIP_DATA_MIN_NBYTES * 8 + VIP_DATA_MAX_NBITS_PER_BAUD - 1) / \
     VIP_DATA_MAX_NBITS_PER_BAUD)

#define VIP_DATA_MAX_NBAUDS \
    ((VIP_DATA_MAX_NBYTES * 8 + VIP_DATA_MIN_NBITS_PER_BAUD - 1) / \
     VIP_DATA_MIN_NBITS_PER_BAUD)

#define VIP_DATA_MIN_NSAMPS(nsamps_per_baud) \
    (VIP_DATA_MIN_NBAUDS * (nsamps_per_baud))
#define VIP_DATA_MAX_NSAMPS(nsamps_per_baud) \
    (VIP_DATA_MAX_NBAUDS * (nsamps_per_baud))

/* 
 * macros to compute nsamps/byte:
 *     nbytes = f(nsamps, nbits_per_baud, nsamps_per_baud) and
 *     nsamps = f(nbytes, nbits_per_baud, nsamps_per_baud). 
 * Results are rounded up.
 */
#define VIP_DATA_NBYTES(nsamps, nbits_per_baud, nsamps_per_baud) \
    ((((((nsamps) + (nsamps_per_baud) - 1) / (nsamps_per_baud)) * \
       (nbits_per_baud)) + 7) / 8)

#define VIP_DATA_NSAMPS(nbytes, nbits_per_baud, nsamps_per_baud) \
    ((((nbytes) * 8 + (nbits_per_baud) - 1) / (nbits_per_baud)) * \
     (nsamps_per_baud))

/* min/max ratio of samples to bytes */
#define VIP_DATA_NSAMPS_PER_BYTE(nbits_per_baud, nsamps_per_baud) \
    (((8 + (nbits_per_baud) - 1) / (nbits_per_baud)) * (nsamps_per_baud))

#define VIP_DATA_MIN_NSAMPS_PER_BYTE(nsamps_per_baud) \
     VIP_DATA_NSAMPS_PER_BYTE(VIP_DATA_MAX_NBITS_PER_BAUD, nsamps_per_baud)

#define VIP_DATA_MAX_NSAMPS_PER_BYTE(nsamps_per_baud) \
     VIP_DATA_NSAMPS_PER_BYTE(VIP_DATA_MIN_NBITS_PER_BAUD, nsamps_per_baud)

/* combined header and data length in bauds */
#define VIP_MIN_NBAUDS (VIP_HDR_NBAUDS + VIP_DATA_MIN_NBAUDS + VIP_EOF_NBAUDS)
#define VIP_MAX_NBAUDS (VIP_HDR_NBAUDS + VIP_DATA_MAX_NBAUDS + VIP_EOF_NBAUDS)

/* combined header and data length in bytes */
#define VIP_MIN_NBYTES (VIP_HDR_NBYTES + VIP_DATA_MIN_NBYTES)
#define VIP_MAX_NBYTES (VIP_HDR_NBYTES + VIP_DATA_MAX_NBYTES)

/* combined header and data max length in samples */
#define VIP_MAX_NSAMPS(nsamps_per_baud) \
    (VIP_HDR_NSAMPS(nsamps_per_baud) + \
     VIP_DATA_MAX_NSAMPS(nsamps_per_baud) + \
     VIP_EOF_NBAUDS * (nsamps_per_baud))

/* FIXME: need to get valid values when we switch to the latest new header */
#if defined(CN_MULTI)

#if !defined(USE_EPITYPE)
#define VIP_HDR_GET_EPISUBTYPE(ppacket) (0)

#define EPI_GET_PETHERTYPE(phdr) \
	((epi_ethertype_hdr_t *)(phdr + VIP_HDR_NBYTES))
#define EPI_GET_PEPI_T8_HDR(phdr) \
	((epi_t8_hdr_t *)EPI_GET_PETHERTYPE(phdr))
#define EPI_GET_PEPI_T16_HDR(phdr) \
	((epi_t16_hdr_t *)EPI_GET_PETHERTYPE(phdr))

#define EPI_GET_ETHERTYPE(phdr) \
	(EPI_GET_PETHERTYPE(phdr))->ethertype

#define EPI_GET_SUBTYPE8(phdr) \
	(EPI_GET_PEPI_T8_HDR(phdr))->subtype8

#define EPI_GET_SUBTYPE8_LENGTH(phdr) \
	(EPI_GET_PEPI_T8_HDR(phdr))->length

#define EPI_GET_SUBTYPE16(phdr) \
	(EPI_GET_PEPI_T16_HDR(phdr))->subtype16
#endif /* !USE_EPITYPE */

/* for now, use the rxc from the vip header */
#define VIP_HDR_GET_RN_RXD(ppacket) \
	VIP_HDR_GET_RXC(ppacket)
/* for now, use the DA from the ethernet header */
#define VIP_HDR_GET_RN_PDA(ppacket) VIP_HDR_GET_PDST(ppacket)

/* FIXME: need to set to valid value */
#define EPI_SUBTYPE_RATE_NEGOT	0

#endif

#endif /* _VIP_H_ */
