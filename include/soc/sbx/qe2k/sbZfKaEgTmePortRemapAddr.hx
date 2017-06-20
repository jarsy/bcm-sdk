/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgTmePortRemapAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEGTMEPORTREMAPADDR_H
#define SB_ZF_ZFKAEGTMEPORTREMAPADDR_H

#define SB_ZF_ZFKAEGTMEPORTREMAPADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEGTMEPORTREMAPADDR_SIZE 4
#define SB_ZF_ZFKAEGTMEPORTREMAPADDR_M_NNODE_BITS "9:5"
#define SB_ZF_ZFKAEGTMEPORTREMAPADDR_M_NPORT_BITS "4:0"


typedef struct _sbZfKaEgTmePortRemapAddr {
  uint32 m_nNode;
  uint32 m_nPort;
} sbZfKaEgTmePortRemapAddr_t;

uint32
sbZfKaEgTmePortRemapAddr_Pack(sbZfKaEgTmePortRemapAddr_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfKaEgTmePortRemapAddr_Unpack(sbZfKaEgTmePortRemapAddr_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfKaEgTmePortRemapAddr_InitInstance(sbZfKaEgTmePortRemapAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#endif
