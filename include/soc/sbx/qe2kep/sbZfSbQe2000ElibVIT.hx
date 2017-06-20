/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibVIT.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_SIZE 8
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD3_BITS "63:48"
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD2_BITS "47:32"
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD1_BITS "31:16"
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_M_RECORD0_BITS "15:00"


/**
 * @file sbZfSbQe2000ElibVIT.h
 *
 * <pre>
 *
 * ====================================================================
 * ==  sbZfSbQe2000ElibVIT.h - VLAN Indirection Record Table ZFrame  ==
 * ====================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibVIT.hx,v 1.4 Broadcom SDK $
 *
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
 *
 *
 * MODULE NAME:
 *
 *     sbZfSbQe2000ElibVIT.h
 *
 * ABSTRACT:
 *
 *     VLAN Indirection Record Table ZFrame Definition.
 *
 * LANGUAGE:
 *
 *     ZFrame
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     19-January-2005
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibVIT {
  int32 m_record3;
  int32 m_record2;
  int32 m_record1;
  int32 m_record0;
} sbZfSbQe2000ElibVIT_t;

uint32
sbZfSbQe2000ElibVIT_Pack(sbZfSbQe2000ElibVIT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibVIT_Unpack(sbZfSbQe2000ElibVIT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibVIT_InitInstance(sbZfSbQe2000ElibVIT_t *pFrame);

#define SB_ZF_SBQE2000ELIBVIT_SET_REC3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_SET_REC2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_SET_REC1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_SET_REC0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[6] ; \
           (nToData) |= (int32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[4] ; \
           (nToData) |= (int32) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[2] ; \
           (nToData) |= (int32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVIT_GET_REC0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[0] ; \
           (nToData) |= (int32) (pFromData)[1] << 8; \
          } while(0)

#endif
