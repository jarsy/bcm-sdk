/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEbMvtAddress.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEBMVTADDRESS_H
#define SB_ZF_ZFKAEBMVTADDRESS_H

#define SB_ZF_ZFKAEBMVTADDRESS_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEBMVTADDRESS_SIZE 4
#define SB_ZF_ZFKAEBMVTADDRESS_M_NRESERVED_BITS "31:22"
#define SB_ZF_ZFKAEBMVTADDRESS_M_NEGRESS_BITS "21:16"
#define SB_ZF_ZFKAEBMVTADDRESS_M_NOFFSET_BITS "15:0"


typedef struct _sbZfKaEbMvtAddress {
  uint32 m_nReserved;
  uint32 m_nEgress;
  uint32 m_nOffset;
} sbZfKaEbMvtAddress_t;

uint32
sbZfKaEbMvtAddress_Pack(sbZfKaEbMvtAddress_t *pFrom,
                        uint8 *pToData,
                        uint32 nMaxToDataIndex);
void
sbZfKaEbMvtAddress_Unpack(sbZfKaEbMvtAddress_t *pToStruct,
                          uint8 *pFromData,
                          uint32 nMaxToDataIndex);
void
sbZfKaEbMvtAddress_InitInstance(sbZfKaEbMvtAddress_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEBMVTADDRESS_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_EGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAEBMVTADDRESS_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_EGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEBMVTADDRESS_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_EGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAEBMVTADDRESS_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_EGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEBMVTADDRESS_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[0] << 2; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_EGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KAEBMVTADDRESS_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[3] << 2; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_EGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEBMVTADDRESS_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[0] << 2; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_EGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KAEBMVTADDRESS_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[3] << 2; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_EGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x3f; \
          } while(0)

#define SB_ZF_KAEBMVTADDRESS_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
