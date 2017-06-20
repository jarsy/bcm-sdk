/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwDstEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_DST_ENTRY_H
#define SB_ZF_FAB_BM3200_DST_ENTRY_H

#define SB_ZF_FAB_BM3200_DST_ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM3200_DST_ENTRY_SIZE 4
#define SB_ZF_FAB_BM3200_DST_ENTRY_M_NDELTA_BITS "31:22"
#define SB_ZF_FAB_BM3200_DST_ENTRY_M_NDEMAND_BITS "21:0"


typedef struct _sbZfFabBm3200BwDstEntry {
  uint32 m_nDelta;
  uint32 m_nDemand;
} sbZfFabBm3200BwDstEntry_t;

uint32
sbZfFabBm3200BwDstEntry_Pack(sbZfFabBm3200BwDstEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwDstEntry_Unpack(sbZfFabBm3200BwDstEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwDstEntry_InitInstance(sbZfFabBm3200BwDstEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWDSTENTRY_SET_DELTA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_SET_DEMAND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWDSTENTRY_SET_DELTA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_SET_DEMAND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWDSTENTRY_SET_DELTA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_SET_DEMAND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWDSTENTRY_SET_DELTA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_SET_DEMAND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWDSTENTRY_GET_DELTA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[0] << 2; \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_GET_DEMAND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWDSTENTRY_GET_DELTA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[3] << 2; \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_GET_DEMAND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWDSTENTRY_GET_DELTA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[0] << 2; \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_GET_DEMAND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWDSTENTRY_GET_DELTA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[3] << 2; \
          } while(0)

#define SB_ZF_FABBM3200BWDSTENTRY_GET_DEMAND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwDstEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_DST_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_DST_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwDstEntry_Print(sbZfFabBm3200BwDstEntry_t *pFromStruct);
int
sbZfFabBm3200BwDstEntry_SPrint(sbZfFabBm3200BwDstEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwDstEntry_Validate(sbZfFabBm3200BwDstEntry_t *pZf);
int
sbZfFabBm3200BwDstEntry_SetField(sbZfFabBm3200BwDstEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_DST_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

