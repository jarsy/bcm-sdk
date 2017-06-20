/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwBwpEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_BWP_ENTRY_H
#define SB_ZF_FAB_BM3200_BWP_ENTRY_H

#define SB_ZF_FAB_BM3200_BWP_ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM3200_BWP_ENTRY_SIZE 4
#define SB_ZF_FAB_BM3200_BWP_ENTRY_M_NGAMMA_BITS "31:24"
#define SB_ZF_FAB_BM3200_BWP_ENTRY_M_NSIGMA_BITS "23:0"


typedef struct _sbZfFabBm3200BwBwpEntry {
  uint32 m_nGamma;
  uint32 m_nSigma;
} sbZfFabBm3200BwBwpEntry_t;

uint32
sbZfFabBm3200BwBwpEntry_Pack(sbZfFabBm3200BwBwpEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwBwpEntry_Unpack(sbZfFabBm3200BwBwpEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwBwpEntry_InitInstance(sbZfFabBm3200BwBwpEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWBWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWBWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWBWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWBWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWBWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWBWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWBWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWBWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_FABBM3200BWBWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwBwpEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_BWP_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_BWP_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwBwpEntry_Print(sbZfFabBm3200BwBwpEntry_t *pFromStruct);
int
sbZfFabBm3200BwBwpEntry_SPrint(sbZfFabBm3200BwBwpEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwBwpEntry_Validate(sbZfFabBm3200BwBwpEntry_t *pZf);
int
sbZfFabBm3200BwBwpEntry_SetField(sbZfFabBm3200BwBwpEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_BWP_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

