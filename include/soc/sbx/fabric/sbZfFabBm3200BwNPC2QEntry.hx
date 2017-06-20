/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwNPC2QEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_NPC2Q_ENTRY_H
#define SB_ZF_FAB_BM3200_NPC2Q_ENTRY_H

#define SB_ZF_FAB_BM3200_NPC2Q_ENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM3200_NPC2Q_ENTRY_SIZE 2
#define SB_ZF_FAB_BM3200_NPC2Q_ENTRY_M_NBASEGROUP_BITS "15:5"
#define SB_ZF_FAB_BM3200_NPC2Q_ENTRY_M_NGROUPS_BITS "4:0"


typedef struct _sbZfFabBm3200BwNPC2QEntry {
  uint32 m_nBaseGroup;
  uint32 m_nGroups;
} sbZfFabBm3200BwNPC2QEntry_t;

uint32
sbZfFabBm3200BwNPC2QEntry_Pack(sbZfFabBm3200BwNPC2QEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwNPC2QEntry_Unpack(sbZfFabBm3200BwNPC2QEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwNPC2QEntry_InitInstance(sbZfFabBm3200BwNPC2QEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_BASEGROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_BASEGROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_BASEGROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_BASEGROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_BASEGROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_BASEGROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_BASEGROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_BASEGROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWNPC2QENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwNPC2QEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_NPC2Q_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_NPC2Q_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwNPC2QEntry_Print(sbZfFabBm3200BwNPC2QEntry_t *pFromStruct);
int
sbZfFabBm3200BwNPC2QEntry_SPrint(sbZfFabBm3200BwNPC2QEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwNPC2QEntry_Validate(sbZfFabBm3200BwNPC2QEntry_t *pZf);
int
sbZfFabBm3200BwNPC2QEntry_SetField(sbZfFabBm3200BwNPC2QEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_NPC2Q_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

