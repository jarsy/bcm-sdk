/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibVlanMem.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_VLAN_MEM_H
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_H

#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_SIZE_IN_BYTES 32
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_SIZE 32
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_PATCHES_BITS "0:0"
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_EDGES_BITS "0:0"
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_NUMFREE_BITS "0:0"
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_M_NUMUSED_BITS "0:0"


/**
 * @file sbZfSbQe2000ElibCIT.h
 *
 * <pre>
 *
 * ==============================================================
 * ==  sbZfSbQe2000ElibCIT.h - Class Instruction Table ZFrame  ==
 * ==============================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibVlanMem.hx,v 1.4 Broadcom SDK $
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
 *     sbZfSbQe2000ElibCIT.h
 *
 * ABSTRACT:
 *
 *     Class Instruction Table ZFrame Definition.
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
 *     30-November-2004
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibVlanMem {
  int32 m_Patches;
  int32 m_Edges;
  int32 m_NumFree;
  int32 m_NumUsed;
} sbZfSbQe2000ElibVlanMem_t;

uint32
sbZfSbQe2000ElibVlanMem_Pack(sbZfSbQe2000ElibVlanMem_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibVlanMem_Unpack(sbZfSbQe2000ElibVlanMem_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibVlanMem_InitInstance(sbZfSbQe2000ElibVlanMem_t *pFrame);

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_PATCHES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_EDGES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_NUMFREE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_SET_NUMUSED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_PATCHES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_EDGES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_NUMFREE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_SBQE2000ELIBVLANMEM_GET_NUMUSED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
