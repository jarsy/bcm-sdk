/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgMemFifoControlEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_H
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_H

#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_SIZE 8
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NTAILPAGE_BITS "35:26"
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NTAILOFFSET_BITS "25:21"
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NCURRDEPTH_BITS "20:5"
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NHEADOFFSET_BITS "4:0"


typedef struct _sbZfKaEgMemFifoControlEntry {
  uint32 m_nTailPage;
  uint32 m_nTailOffset;
  uint32 m_nCurrDepth;
  uint32 m_nHeadOffset;
} sbZfKaEgMemFifoControlEntry_t;

uint32
sbZfKaEgMemFifoControlEntry_Pack(sbZfKaEgMemFifoControlEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaEgMemFifoControlEntry_Unpack(sbZfKaEgMemFifoControlEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfKaEgMemFifoControlEntry_InitInstance(sbZfKaEgMemFifoControlEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#endif
