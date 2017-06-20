/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200WredDataTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_H
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_H

#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE 4
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_M_NTEMPLATEODD_BITS "31:24"
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_M_NRESERVEDODD_BITS "23:20"
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_M_NGAINODD_BITS "19:16"
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_M_NTEMPLATEEVEN_BITS "15:8"
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_M_NRESERVEDEVEN_BITS "7:4"
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_M_NGAINEVEN_BITS "3:0"


/**

 * Copyright (c) Sandburst, Inc. 2005
 * All Rights Reserved.  Unpublished rights reserved under the copyright
 * laws of the United States.
 *
 * The software contained on this media is proprietary to and embodies the
 * confidential technology of Sandburst, Inc. Possession, use, duplication
 * or dissemination of the software and media is authorized only pursuant
 * to a valid written license from Sandburst, Inc.
 *
 * RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure by the U.S.
 * Government is subject to restrictions as set forth in Subparagraph
 * (c) (1) (ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
 */
typedef struct _sbZfFabBm3200WredDataTableEntry {
  uint32 m_nTemplateOdd;
  uint32 m_nReservedOdd;
  uint32 m_nGainOdd;
  uint32 m_nTemplateEven;
  uint32 m_nReservedEven;
  uint32 m_nGainEven;
} sbZfFabBm3200WredDataTableEntry_t;

uint32
sbZfFabBm3200WredDataTableEntry_Pack(sbZfFabBm3200WredDataTableEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex);
void
sbZfFabBm3200WredDataTableEntry_Unpack(sbZfFabBm3200WredDataTableEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex);
void
sbZfFabBm3200WredDataTableEntry_InitInstance(sbZfFabBm3200WredDataTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_ODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_TEMPLATE_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_RESERVED_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_SET_GAIN_EVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_ODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_TEMPLATE_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_RESERVED_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200WREDDATATABLEENTRY_GET_GAIN_EVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200WredDataTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_CONSOLE_H



void
sbZfFabBm3200WredDataTableEntry_Print(sbZfFabBm3200WredDataTableEntry_t *pFromStruct);
int
sbZfFabBm3200WredDataTableEntry_SPrint(sbZfFabBm3200WredDataTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200WredDataTableEntry_Validate(sbZfFabBm3200WredDataTableEntry_t *pZf);
int
sbZfFabBm3200WredDataTableEntry_SetField(sbZfFabBm3200WredDataTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

