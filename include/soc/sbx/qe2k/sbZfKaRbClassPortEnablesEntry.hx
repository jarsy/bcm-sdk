/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassPortEnablesEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSPORTENABLESENTRY_H
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_H

#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE 4
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NRESERVE_BITS "31:23"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEUSER1_BITS "22:22"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEUSER0_BITS "21:21"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEVLANPRI_BITS "20:20"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEHIPRIVLAN_BITS "19:19"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEDMACMATCH_BITS "18:18"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSELAYER4_BITS "17:17"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NFLOWHASHENABLE_BITS "16:16"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEHASHCOS_BITS "15:0"


typedef struct _sbZfKaRbClassPortEnablesEntry {
  uint32 m_nReserve;
  uint8 m_nUseUser1;
  uint8 m_nUseUser0;
  uint8 m_nUseVlanPri;
  uint8 m_nUseHiPriVlan;
  uint8 m_nUseDmacMatch;
  uint8 m_nUseLayer4;
  uint8 m_nFlowHashEnable;
  uint32 m_nUseHashCos;
} sbZfKaRbClassPortEnablesEntry_t;

uint32
sbZfKaRbClassPortEnablesEntry_Pack(sbZfKaRbClassPortEnablesEntry_t *pFrom,
                                   uint8 *pToData,
                                   uint32 nMaxToDataIndex);
void
sbZfKaRbClassPortEnablesEntry_Unpack(sbZfKaRbClassPortEnablesEntry_t *pToStruct,
                                     uint8 *pFromData,
                                     uint32 nMaxToDataIndex);
void
sbZfKaRbClassPortEnablesEntry_InitInstance(sbZfKaRbClassPortEnablesEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[0] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[3] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[0] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[3] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
