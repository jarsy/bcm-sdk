/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwRepErrInfo.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_H
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_H

#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_SIZE 4
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_M_NCLIENT_BITS "24:19"
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_M_NTABLEID_BITS "18:15"
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_M_NOFFSET_BITS "14:0"


typedef struct _sbZfFabBm3200BwRepErrInfo {
  int32 m_nClient;
  int32 m_nTableId;
  int32 m_nOffset;
} sbZfFabBm3200BwRepErrInfo_t;

uint32
sbZfFabBm3200BwRepErrInfo_Pack(sbZfFabBm3200BwRepErrInfo_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwRepErrInfo_Unpack(sbZfFabBm3200BwRepErrInfo_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwRepErrInfo_InitInstance(sbZfFabBm3200BwRepErrInfo_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (int32) ((pFromData)[1] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[3] ; \
           (nToData) |= (int32) ((pFromData)[2] & 0x7f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (int32) ((pFromData)[2] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[0] ; \
           (nToData) |= (int32) ((pFromData)[1] & 0x7f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (int32) ((pFromData)[1] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[3] ; \
           (nToData) |= (int32) ((pFromData)[2] & 0x7f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (int32) ((pFromData)[2] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[0] ; \
           (nToData) |= (int32) ((pFromData)[1] & 0x7f) << 8; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwRepErrInfo.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_CONSOLE_H
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_CONSOLE_H



void
sbZfFabBm3200BwRepErrInfo_Print(sbZfFabBm3200BwRepErrInfo_t *pFromStruct);
int
sbZfFabBm3200BwRepErrInfo_SPrint(sbZfFabBm3200BwRepErrInfo_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwRepErrInfo_Validate(sbZfFabBm3200BwRepErrInfo_t *pZf);
int
sbZfFabBm3200BwRepErrInfo_SetField(sbZfFabBm3200BwRepErrInfo_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

