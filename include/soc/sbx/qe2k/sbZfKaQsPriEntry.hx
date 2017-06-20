/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsPriEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSPRIENTRY_H
#define SB_ZF_ZFKAQSPRIENTRY_H

#define SB_ZF_ZFKAQSPRIENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSPRIENTRY_SIZE 4
#define SB_ZF_ZFKAQSPRIENTRY_M_NRESERVED_BITS "31:8"
#define SB_ZF_ZFKAQSPRIENTRY_M_NCPRI_BITS "7:4"
#define SB_ZF_ZFKAQSPRIENTRY_M_NNPRI_BITS "3:0"


typedef struct _sbZfKaQsPriEntry {
  uint32 m_nReserved;
  uint32 m_nCPri;
  uint32 m_nNPri;
} sbZfKaQsPriEntry_t;

uint32
sbZfKaQsPriEntry_Pack(sbZfKaQsPriEntry_t *pFrom,
                      uint8 *pToData,
                      uint32 nMaxToDataIndex);
void
sbZfKaQsPriEntry_Unpack(sbZfKaQsPriEntry_t *pToStruct,
                        uint8 *pFromData,
                        uint32 nMaxToDataIndex);
void
sbZfKaQsPriEntry_InitInstance(sbZfKaQsPriEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_CPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_NPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSPRIENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_CPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_NPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_CPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_NPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSPRIENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_CPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_SET_NPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[0] << 16; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_CPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_NPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSPRIENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[3] << 16; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_CPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_NPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[0] << 16; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_CPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_NPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSPRIENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[3] << 16; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_CPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRIENTRY_GET_NPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
