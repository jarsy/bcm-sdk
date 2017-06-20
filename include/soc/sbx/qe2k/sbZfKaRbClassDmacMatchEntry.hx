/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassDmacMatchEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSDMACMATCHENTRY_H
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_H

#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE_IN_BYTES 20
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE 20
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACDATALSB_BITS "159:128"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACDATARSV_BITS "127:112"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACDATAMSB_BITS "111:96"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACMASKLSB_BITS "95:64"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACMASKRSV_BITS "63:48"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACMASKMSB_BITS "47:32"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACRESERVE_BITS "31:7"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACENABLE_BITS "6:6"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACDP_BITS "5:4"
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_M_NDMACLSB_BITS "3:0"


typedef struct _sbZfKaRbClassDmacMatchEntry {
  uint32 m_nDmacDataLsb;
  uint32 m_nDmacDataRsv;
  uint32 m_nDmacDataMsb;
  uint32 m_nDmacMaskLsb;
  uint32 m_nDmacMaskRsv;
  uint32 m_nDmacMaskMsb;
  uint32 m_nDmacReserve;
  uint8 m_nDmacEnable;
  uint32 m_nDmacDp;
  uint32 m_nDmacLsb;
} sbZfKaRbClassDmacMatchEntry_t;

uint32
sbZfKaRbClassDmacMatchEntry_Pack(sbZfKaRbClassDmacMatchEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaRbClassDmacMatchEntry_Unpack(sbZfKaRbClassDmacMatchEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfKaRbClassDmacMatchEntry_InitInstance(sbZfKaRbClassDmacMatchEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATALSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATARSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATAMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKRSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACRESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATALSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATARSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATAMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKRSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACRESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATALSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATARSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATAMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKRSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACRESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATALSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATARSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDATAMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKRSV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACMASKMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACRESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_SET_DMACLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATALSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
           (nToData) |= (uint32) (pFromData)[17] << 16; \
           (nToData) |= (uint32) (pFromData)[16] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATARSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[13] ; \
           (nToData) |= (uint32) (pFromData)[12] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATAMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) (pFromData)[14] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKRSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACRESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
           (nToData) |= (uint32) (pFromData)[1] << 9; \
           (nToData) |= (uint32) (pFromData)[0] << 17; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATALSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
           (nToData) |= (uint32) (pFromData)[18] << 16; \
           (nToData) |= (uint32) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATARSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[14] ; \
           (nToData) |= (uint32) (pFromData)[15] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATAMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) (pFromData)[13] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
           (nToData) |= (uint32) (pFromData)[10] << 16; \
           (nToData) |= (uint32) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKRSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACRESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
           (nToData) |= (uint32) (pFromData)[2] << 9; \
           (nToData) |= (uint32) (pFromData)[3] << 17; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATALSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
           (nToData) |= (uint32) (pFromData)[17] << 16; \
           (nToData) |= (uint32) (pFromData)[16] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATARSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[13] ; \
           (nToData) |= (uint32) (pFromData)[12] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATAMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) (pFromData)[14] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKRSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACRESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
           (nToData) |= (uint32) (pFromData)[1] << 9; \
           (nToData) |= (uint32) (pFromData)[0] << 17; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATALSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
           (nToData) |= (uint32) (pFromData)[18] << 16; \
           (nToData) |= (uint32) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATARSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[14] ; \
           (nToData) |= (uint32) (pFromData)[15] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDATAMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) (pFromData)[13] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
           (nToData) |= (uint32) (pFromData)[10] << 16; \
           (nToData) |= (uint32) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKRSV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACMASKMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACRESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
           (nToData) |= (uint32) (pFromData)[2] << 9; \
           (nToData) |= (uint32) (pFromData)[3] << 17; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDMACMATCHENTRY_GET_DMACLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
