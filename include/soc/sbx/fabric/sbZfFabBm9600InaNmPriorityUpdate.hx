/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600InaNmPriorityUpdate.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_INANMPRIUPDATE_H
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_H

#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_SIZE_IN_BYTES 7
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_SIZE 7
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UINA_BITS "53:47"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_USYSTEMPORT_BITS "46:35"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UESET_BITS "34:25"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UPORTSETADDRESS_BITS "24:17"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UPORTSETOFFSET_BITS "16:13"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BNOCRITICALUPDATE_BITS "12:12"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BCRITICALUPDATE_BITS "11:11"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BMULTICAST_BITS "10:10"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UPRIORITY_BITS "9:6"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BMAXPRIORITY_BITS "5:5"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UNEXTPRIORITY_BITS "4:1"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BNEXTMAXPRIORITY_BITS "0:0"


typedef struct _sbZfFabBm9600InaNmPriorityUpdate {
  uint32 m_uIna;
  uint32 m_uSystemPort;
  uint32 m_uEset;
  uint32 m_uPortSetAddress;
  uint32 m_uPortSetOffset;
  uint8 m_bNoCriticalUpdate;
  uint8 m_bCriticalUpdate;
  uint8 m_bMulticast;
  uint32 m_uPriority;
  uint8 m_bMaxPriority;
  uint32 m_uNextPriority;
  uint8 m_bNextMaxPriority;
} sbZfFabBm9600InaNmPriorityUpdate_t;

uint32
sbZfFabBm9600InaNmPriorityUpdate_Pack(sbZfFabBm9600InaNmPriorityUpdate_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaNmPriorityUpdate_Unpack(sbZfFabBm9600InaNmPriorityUpdate_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaNmPriorityUpdate_InitInstance(sbZfFabBm9600InaNmPriorityUpdate_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
