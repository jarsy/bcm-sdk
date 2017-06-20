/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpVlanIndRecord.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPVLANINDRECORD_H
#define SB_ZF_ZFKAEPVLANINDRECORD_H

#define SB_ZF_ZFKAEPVLANINDRECORD_SIZE_IN_BYTES 2
#define SB_ZF_ZFKAEPVLANINDRECORD_SIZE 2
#define SB_ZF_ZFKAEPVLANINDRECORD_M_NPTR_BITS "15:2"
#define SB_ZF_ZFKAEPVLANINDRECORD_M_NCMAP_BITS "1:0"
#define SB_ZF_ZFKAEPVLANINDRECORD_CONTROL_OP_DISCARD_NOTIFY (0x0)
#define SB_ZF_ZFKAEPVLANINDRECORD_CONTROL_OP_DISCARD_SILENT (0x1)
#define SB_ZF_ZFKAEPVLANINDRECORD_CONTROL_OP_FORWARD_UNTAGGED (0x2)
#define SB_ZF_ZFKAEPVLANINDRECORD_CONTROL_OP_FORWARD_TAGGED (0x3)
#define SB_ZF_ZFKAEPVLANINDRECORD_CONTROL_OP_LAST           (0x3)


typedef struct _sbZfKaEpVlanIndRecord {
  uint32 m_nPtr;
  uint32 m_nCMap;
} sbZfKaEpVlanIndRecord_t;

uint32
sbZfKaEpVlanIndRecord_Pack(sbZfKaEpVlanIndRecord_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex);
void
sbZfKaEpVlanIndRecord_Unpack(sbZfKaEpVlanIndRecord_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex);
void
sbZfKaEpVlanIndRecord_InitInstance(sbZfKaEpVlanIndRecord_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPVLANINDRECORD_SET_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_SET_CMAP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEPVLANINDRECORD_SET_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_SET_CMAP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPVLANINDRECORD_SET_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_SET_CMAP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEPVLANINDRECORD_SET_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_SET_CMAP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPVLANINDRECORD_GET_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[2] << 6; \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_GET_CMAP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x03; \
          } while(0)

#else
#define SB_ZF_KAEPVLANINDRECORD_GET_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[1] << 6; \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_GET_CMAP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x03; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPVLANINDRECORD_GET_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[2] << 6; \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_GET_CMAP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x03; \
          } while(0)

#else
#define SB_ZF_KAEPVLANINDRECORD_GET_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[1] << 6; \
          } while(0)

#define SB_ZF_KAEPVLANINDRECORD_GET_CMAP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x03; \
          } while(0)

#endif
#endif
