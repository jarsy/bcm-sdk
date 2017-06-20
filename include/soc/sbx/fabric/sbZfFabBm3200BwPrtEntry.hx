/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwPrtEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_PRT_ENTRY_H
#define SB_ZF_FAB_BM3200_PRT_ENTRY_H

#define SB_ZF_FAB_BM3200_PRT_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_FAB_BM3200_PRT_ENTRY_SIZE 8
#define SB_ZF_FAB_BM3200_PRT_ENTRY_M_NSPGROUPS_BITS "57:53"
#define SB_ZF_FAB_BM3200_PRT_ENTRY_M_NGROUPS_BITS "52:48"
#define SB_ZF_FAB_BM3200_PRT_ENTRY_M_NGROUP_BITS "47:32"
#define SB_ZF_FAB_BM3200_PRT_ENTRY_M_NLINERATE_BITS "21:0"


typedef struct _sbZfFabBm3200BwPrtEntry {
  uint32 m_nSpGroups;
  uint32 m_nGroups;
  uint32 m_nGroup;
  uint32 m_nLineRate;
} sbZfFabBm3200BwPrtEntry_t;

uint32
sbZfFabBm3200BwPrtEntry_Pack(sbZfFabBm3200BwPrtEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwPrtEntry_Unpack(sbZfFabBm3200BwPrtEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwPrtEntry_InitInstance(sbZfFabBm3200BwPrtEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPRTENTRY_SET_SPGROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPRTENTRY_SET_SPGROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPRTENTRY_SET_SPGROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPRTENTRY_SET_SPGROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPRTENTRY_GET_SPGROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPRTENTRY_GET_SPGROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPRTENTRY_GET_SPGROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPRTENTRY_GET_SPGROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPRTENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwPrtEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_PRT_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_PRT_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwPrtEntry_Print(sbZfFabBm3200BwPrtEntry_t *pFromStruct);
int
sbZfFabBm3200BwPrtEntry_SPrint(sbZfFabBm3200BwPrtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwPrtEntry_Validate(sbZfFabBm3200BwPrtEntry_t *pZf);
int
sbZfFabBm3200BwPrtEntry_SetField(sbZfFabBm3200BwPrtEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_PRT_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

