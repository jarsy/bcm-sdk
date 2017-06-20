/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibPriTableAddr.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_H
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_H

#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES 4
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE 4
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NPORT_BITS "11:6"
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NCOS_BITS "5:3"
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NDP_BITS "2:1"
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_M_NECN_BITS "0:0"


/**
 * @file sbZfSbQe2000ElibPriTableAddr.h
 *
 * <pre>
 *
 * =================================================================================
 * ==  sbZfSbQe2000ElibPriTableAddr.h - PRI Rewrite Table Address ZFrame  ==
 * =================================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibPriTableAddr.hx,v 1.4 Broadcom SDK $
 *
 * Copyright (c) Sandburst, Inc. 2004
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
 *     sbZfSbQe2000ElibPriTableAddr.h
 *
 * ABSTRACT:
 *
 *     PRI Rewrite Table Address ZFrame Definition.
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
 *     07-December-2004
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibPriTableAddr {
  uint32 m_nPort;
  uint32 m_nCos;
  uint32 m_nDp;
  uint32 m_nEcn;
} sbZfSbQe2000ElibPriTableAddr_t;

uint32
sbZfSbQe2000ElibPriTableAddr_Pack(sbZfSbQe2000ElibPriTableAddr_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibPriTableAddr_Unpack(sbZfSbQe2000ElibPriTableAddr_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibPriTableAddr_InitInstance(sbZfSbQe2000ElibPriTableAddr_t *pFrame);

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPRITABLEADDR_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#endif
