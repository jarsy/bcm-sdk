/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbPoliceCfgCtrlEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBPOLCFGCTRLENTRY_H
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_H

#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE 4
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NRESERVED_BITS "31:27"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NENABLE_BITS "26:26"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NNOTBLIND_BITS "25:25"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NDROPONRED_BITS "24:24"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NENABLEMON_BITS "23:23"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NMONCNTID_BITS "22:20"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NINCRATE_BITS "19:0"


typedef struct _sbZfKaRbPoliceCfgCtrlEntry {
  uint32 m_nReserved;
  uint8 m_nEnable;
  uint8 m_nNotBlind;
  uint8 m_nDropOnRed;
  uint8 m_nEnableMon;
  uint32 m_nMonCntId;
  uint32 m_nIncRate;
} sbZfKaRbPoliceCfgCtrlEntry_t;

uint32
sbZfKaRbPoliceCfgCtrlEntry_Pack(sbZfKaRbPoliceCfgCtrlEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfKaRbPoliceCfgCtrlEntry_Unpack(sbZfKaRbPoliceCfgCtrlEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaRbPoliceCfgCtrlEntry_InitInstance(sbZfKaRbPoliceCfgCtrlEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 16; \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 16; \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 16; \
          } while(0)

#endif
#endif
