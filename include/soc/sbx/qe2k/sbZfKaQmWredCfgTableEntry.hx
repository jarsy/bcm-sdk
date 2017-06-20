/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmWredCfgTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMWREDCFGTABLEENTRY_H
#define SB_ZF_ZFKAQMWREDCFGTABLEENTRY_H

#define SB_ZF_ZFKAQMWREDCFGTABLEENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQMWREDCFGTABLEENTRY_SIZE 4
#define SB_ZF_ZFKAQMWREDCFGTABLEENTRY_M_NTEMPLATE_BITS "7:4"
#define SB_ZF_ZFKAQMWREDCFGTABLEENTRY_M_NGAIN_BITS "3:0"


typedef struct _sbZfKaQmWredCfgTableEntry {
  uint32 m_nTemplate;
  uint32 m_nGain;
} sbZfKaQmWredCfgTableEntry_t;

uint32
sbZfKaQmWredCfgTableEntry_Pack(sbZfKaQmWredCfgTableEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQmWredCfgTableEntry_Unpack(sbZfKaQmWredCfgTableEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaQmWredCfgTableEntry_InitInstance(sbZfKaQmWredCfgTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCFGTABLEENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
