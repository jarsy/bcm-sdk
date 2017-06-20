/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600InaHi1Selected_1Entry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_H
#define SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_H

#define SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_SIZE 4
#define SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_M_UPRI_BITS "27:24"
#define SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_M_UPORTSETADDR_BITS "23:16"
#define SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_M_UOFFSET_BITS "15:12"
#define SB_ZF_FAB_BM9600_INAHI1SELECTED_1ENTRY_M_USYSPORT_BITS "11:0"


typedef struct _sbZfFabBm9600InaHi1Selected_1Entry {
  uint32 m_uPri;
  uint32 m_uPortsetAddr;
  uint32 m_uOffset;
  uint32 m_uSysport;
} sbZfFabBm9600InaHi1Selected_1Entry_t;

uint32
sbZfFabBm9600InaHi1Selected_1Entry_Pack(sbZfFabBm9600InaHi1Selected_1Entry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaHi1Selected_1Entry_Unpack(sbZfFabBm9600InaHi1Selected_1Entry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaHi1Selected_1Entry_InitInstance(sbZfFabBm9600InaHi1Selected_1Entry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAHI1SELECTED_1ENTRY_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#endif
