/*
 * $Id: sbG2FeEgrRouteHeader.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains the Gu2k Egress Route header definition
 */

#ifndef SB_ZF_G2_FE_EGRROUTEHEADER_H
#define SB_ZF_G2_FE_EGRROUTEHEADER_H

#define SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES 12
#define SB_ZF_G2_FE_EGRROUTEHEADER_SIZE 12
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULTTL_BITS "7:0"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULS_BITS "8:8"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULRDP_BITS "10:9"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULRCOS_BITS "13:11"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULRSVD_BITS "17:14"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULSID_BITS "31:18"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULRSVD1_BITS "38:32"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULMC_BITS "39:39"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULOUTUNION_BITS "56:40"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULRSVD_INTEROP_BITS "59:57"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULLENADJINDEX_BITS "63:60"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULFRMLEN_BITS "77:64"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULTEST_BITS "78:78"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULE_BITS "79:79"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULFDP_BITS "81:80"
#define SB_ZF_G2_FE_EGRROUTEHEADER_ULQID_BITS "95:82"



/**
 * sbG2FeEgrRouteHeaderPpeSwop_e
 *
 * This enumeration type encodes the ulPpeSwop values.
 *
 */
typedef enum sbG2FeEgrRouteHeaderPpeSwop_e {
    SB_G2_FE_EGRROUTEHEADER_PPESWOP_BRIDGED, /**< bridged and raw */
    SB_G2_FE_EGRROUTEHEADER_PPESWOP_IPV4,   /**< Ipv4 */
    SB_G2_FE_EGRROUTEHEADER_PPESWOP_IPV6,   /**< Ipv6 */
    SB_G2_FE_EGRROUTEHEADER_PPESWOP_MPLS    /**< Mpls */
} sbG2FeEgrRouteHeaderPpeSwop_t;

#define SB_G2_FE_EGRROUTEHEADER_PPESWOP_RAW SB_G2_FE_EGRROUTEHEADER_PPESWOP_BRIDGED


/** @brief  Egress Route Header

  The Egress Route Header defines the content of the route header prepended
  to packets bound for egress (and NOT to the CPU).
*/

typedef struct  {
/** @brief <p> Time to live value</p> */

  uint32 ulTtl;
/** @brief <p> Mpls header S-bit value</p> */

  uint32 ulS;
/** @brief <p> Remark Drop Precedence Copy</p> */

  uint32 ulRDp;
/** @brief <p> Remark Cos Value</p> */

  uint32 ulRCos;
/** @brief <p> Reserved</p> */

  uint32 ulRsvd;
/** @brief <p> Source Identifier</p> */
/** @brief <p> Used for L2 multicast source knockout</p> */

  uint32 ulSid;
/** @brief <p> Reserved</p> */

  uint32 ulRsvd1;
/** @brief <p> Multicast flag</p> */

  uint32 ulMc;
/** @brief <p> OutUnion</p> */
/** @brief <p> If (Mc==0), 17 bit OutHeaderIndex.</p> */
/** @brief <p> If (Mc==1), McGroup.</p> */

  uint32 ulOutUnion;
/** @brief <p> Reserved or InterOp Bits</p> */

  uint32 ulRsvd_InterOp;
/** @brief <p> Length Adjust</p> */
/** @brief <p> Used by the QE-2000.</p> */

  uint32 ulLenAdjIndex;
/** @brief <p> Frame Length</p> */

  uint32 ulFrmLen;
/** @brief <p> Test Bit</p> */
/** @brief <p> For debug use only</p> */

  uint32 ulTest;
/** @brief <p> ECN (explicit congestion notification)</p> */

  uint32 ulE;
/** @brief <p> Fabric Drop Precedence</p> */

  uint32 ulFDp;
/** @brief <p> Queue ID</p> */

  uint32 ulQid;
} sbZfG2FeEgrRouteHeader_t;

uint32
sbZfG2FeEgrRouteHeader_Pack(sbZfG2FeEgrRouteHeader_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex);
void
sbZfG2FeEgrRouteHeader_Unpack(sbZfG2FeEgrRouteHeader_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex);
void
sbZfG2FeEgrRouteHeader_InitInstance(sbZfG2FeEgrRouteHeader_t *pFrame);

#define SB_ZF_G2FEEGRROUTEHEADER_SET_TTL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_S(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_RDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_RCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_SID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_RSVD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_OUTUNION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_RSVD_INTEROP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_LENADJINDEX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_FRMLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_TEST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_E(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_FDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_SET_QID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_TTL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_S(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x01; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_RDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_RCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x03) << 2; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_SID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[8] << 6; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_RSVD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x7f; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_OUTUNION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x01) << 16; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_RSVD_INTEROP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_LENADJINDEX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_FRMLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_TEST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_E(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_FDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x03; \
          } while(0)

#define SB_ZF_G2FEEGRROUTEHEADER_GET_QID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[0] << 6; \
          } while(0)

#endif
