/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200PriMemEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_PRI_ENTRY_H
#define SB_ZF_FAB_BM3200_PRI_ENTRY_H

#define SB_ZF_FAB_BM3200_PRI_ENTRY_SIZE_IN_BYTES 19
#define SB_ZF_FAB_BM3200_PRI_ENTRY_SIZE 19
#define SB_ZF_FAB_BM3200_PRI_ENTRY_M_NPRIX_BITS "147:128"
#define SB_ZF_FAB_BM3200_PRI_ENTRY_M_NPRID_BITS "127:96"
#define SB_ZF_FAB_BM3200_PRI_ENTRY_M_NPRIC_BITS "95:64"
#define SB_ZF_FAB_BM3200_PRI_ENTRY_M_NPRIB_BITS "63:32"
#define SB_ZF_FAB_BM3200_PRI_ENTRY_M_NPRIA_BITS "31:0"


typedef struct _sbZfFabBm3200PriMemEntry {
  uint32 m_nPrix;
  uint32 m_nPrid;
  uint32 m_nPric;
  uint32 m_nPrib;
  uint32 m_nPria;
} sbZfFabBm3200PriMemEntry_t;

uint32
sbZfFabBm3200PriMemEntry_Pack(sbZfFabBm3200PriMemEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfFabBm3200PriMemEntry_Unpack(sbZfFabBm3200PriMemEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfFabBm3200PriMemEntry_InitInstance(sbZfFabBm3200PriMemEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_SET_PRIA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
           (nToData) |= (uint32) ((pFromData)[17] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) (pFromData)[14] << 8; \
           (nToData) |= (uint32) (pFromData)[13] << 16; \
           (nToData) |= (uint32) (pFromData)[12] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
           (nToData) |= (uint32) ((pFromData)[18] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) (pFromData)[13] << 8; \
           (nToData) |= (uint32) (pFromData)[14] << 16; \
           (nToData) |= (uint32) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
           (nToData) |= (uint32) (pFromData)[10] << 16; \
           (nToData) |= (uint32) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
           (nToData) |= (uint32) (pFromData)[6] << 16; \
           (nToData) |= (uint32) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
           (nToData) |= (uint32) ((pFromData)[17] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) (pFromData)[14] << 8; \
           (nToData) |= (uint32) (pFromData)[13] << 16; \
           (nToData) |= (uint32) (pFromData)[12] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
           (nToData) |= (uint32) ((pFromData)[18] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) (pFromData)[13] << 8; \
           (nToData) |= (uint32) (pFromData)[14] << 16; \
           (nToData) |= (uint32) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
           (nToData) |= (uint32) (pFromData)[10] << 16; \
           (nToData) |= (uint32) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
           (nToData) |= (uint32) (pFromData)[6] << 16; \
           (nToData) |= (uint32) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_FABBM3200PRIMEMENTRY_GET_PRIA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200PriMemEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_PRI_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_PRI_ENTRY_CONSOLE_H



void
sbZfFabBm3200PriMemEntry_Print(sbZfFabBm3200PriMemEntry_t *pFromStruct);
int
sbZfFabBm3200PriMemEntry_SPrint(sbZfFabBm3200PriMemEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200PriMemEntry_Validate(sbZfFabBm3200PriMemEntry_t *pZf);
int
sbZfFabBm3200PriMemEntry_SetField(sbZfFabBm3200PriMemEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_PRI_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

