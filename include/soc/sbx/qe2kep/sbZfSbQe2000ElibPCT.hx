/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibPCT.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_SIZE_IN_BYTES 128
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_SIZE 128
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS15_BITS "1023:995"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS15_BITS "994:960"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS14_BITS "959:931"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS14_BITS "930:896"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS13_BITS "895:867"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS13_BITS "866:832"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS12_BITS "831:803"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS12_BITS "802:768"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS11_BITS "767:739"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS11_BITS "738:704"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS10_BITS "703:675"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS10_BITS "674:640"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS9_BITS "639:611"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS9_BITS "610:576"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS8_BITS "575:547"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS8_BITS "546:512"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS7_BITS "511:483"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS7_BITS "482:448"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS6_BITS "447:419"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS6_BITS "418:384"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS5_BITS "383:355"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS5_BITS "354:320"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS4_BITS "319:291"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS4_BITS "290:256"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS3_BITS "255:227"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS3_BITS "226:192"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS2_BITS "191:163"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS2_BITS "162:128"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS1_BITS "127:99"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS1_BITS "98:64"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_PKTCLASS0_BITS "63:35"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_M_BYTECLASS0_BITS "34:0"
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_OFFSET         0x0E0 /**< Offset into the classifier memory for the PCT */


/**
 * @file sbZfSbQe2000ElibPCT.h
 *
 * <pre>
 *
 * =================================================================
 * ==  sbZfSbQe2000ElibPCT.h - Port Counter Table ZFrame  ==
 * =================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibPCT.hx,v 1.4 Broadcom SDK $
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
 *     sbZfSbQe2000ElibPCT.zf
 *
 * ABSTRACT:
 *
 *     Port Counter Table ZFrame Definition
 *     An entry in the port count table is 16 counts
 *     That is, a per-class count for one port.
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
typedef struct _sbZfSbQe2000ElibPCT {
  uint64 m_PktClass15;
  uint64 m_ByteClass15;
  uint64 m_PktClass14;
  uint64 m_ByteClass14;
  uint64 m_PktClass13;
  uint64 m_ByteClass13;
  uint64 m_PktClass12;
  uint64 m_ByteClass12;
  uint64 m_PktClass11;
  uint64 m_ByteClass11;
  uint64 m_PktClass10;
  uint64 m_ByteClass10;
  uint64 m_PktClass9;
  uint64 m_ByteClass9;
  uint64 m_PktClass8;
  uint64 m_ByteClass8;
  uint64 m_PktClass7;
  uint64 m_ByteClass7;
  uint64 m_PktClass6;
  uint64 m_ByteClass6;
  uint64 m_PktClass5;
  uint64 m_ByteClass5;
  uint64 m_PktClass4;
  uint64 m_ByteClass4;
  uint64 m_PktClass3;
  uint64 m_ByteClass3;
  uint64 m_PktClass2;
  uint64 m_ByteClass2;
  uint64 m_PktClass1;
  uint64 m_ByteClass1;
  uint64 m_PktClass0;
  uint64 m_ByteClass0;
} sbZfSbQe2000ElibPCT_t;

uint32
sbZfSbQe2000ElibPCT_Pack(sbZfSbQe2000ElibPCT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibPCT_Unpack(sbZfSbQe2000ElibPCT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibPCT_InitInstance(sbZfSbQe2000ElibPCT_t *pFrame);

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[124] = ((pToData)[124] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[125] = ((pToData)[125] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[126] = ((pToData)[126] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[127] = ((pToData)[127] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[120] = ((nFromData)) & 0xFF; \
           (pToData)[121] = ((pToData)[121] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[122] = ((pToData)[122] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[123] = ((pToData)[123] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[124] = ((pToData)[124] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[116] = ((pToData)[116] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[117] = ((pToData)[117] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[118] = ((pToData)[118] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[119] = ((pToData)[119] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[112] = ((nFromData)) & 0xFF; \
           (pToData)[113] = ((pToData)[113] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[114] = ((pToData)[114] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[115] = ((pToData)[115] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[116] = ((pToData)[116] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[108] = ((pToData)[108] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[109] = ((pToData)[109] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[110] = ((pToData)[110] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[111] = ((pToData)[111] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[104] = ((nFromData)) & 0xFF; \
           (pToData)[105] = ((pToData)[105] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[106] = ((pToData)[106] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[107] = ((pToData)[107] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[108] = ((pToData)[108] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[100] = ((pToData)[100] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[101] = ((pToData)[101] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[102] = ((pToData)[102] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[103] = ((pToData)[103] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[96] = ((nFromData)) & 0xFF; \
           (pToData)[97] = ((pToData)[97] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[98] = ((pToData)[98] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[99] = ((pToData)[99] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[100] = ((pToData)[100] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[92] = ((pToData)[92] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[93] = ((pToData)[93] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[94] = ((pToData)[94] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[95] = ((pToData)[95] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[88] = ((nFromData)) & 0xFF; \
           (pToData)[89] = ((pToData)[89] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[90] = ((pToData)[90] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[91] = ((pToData)[91] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[92] = ((pToData)[92] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[84] = ((pToData)[84] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[85] = ((pToData)[85] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[86] = ((pToData)[86] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[87] = ((pToData)[87] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[80] = ((nFromData)) & 0xFF; \
           (pToData)[81] = ((pToData)[81] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[82] = ((pToData)[82] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[83] = ((pToData)[83] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[84] = ((pToData)[84] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT09(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[76] = ((pToData)[76] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[77] = ((pToData)[77] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[78] = ((pToData)[78] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[79] = ((pToData)[79] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT09(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[72] = ((nFromData)) & 0xFF; \
           (pToData)[73] = ((pToData)[73] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[74] = ((pToData)[74] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[75] = ((pToData)[75] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[76] = ((pToData)[76] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT08(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[68] = ((pToData)[68] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[69] = ((pToData)[69] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[70] = ((pToData)[70] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[71] = ((pToData)[71] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT08(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[64] = ((nFromData)) & 0xFF; \
           (pToData)[65] = ((pToData)[65] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[66] = ((pToData)[66] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[67] = ((pToData)[67] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[68] = ((pToData)[68] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT07(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[60] = ((pToData)[60] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[61] = ((pToData)[61] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[62] = ((pToData)[62] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[63] = ((pToData)[63] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT07(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[56] = ((nFromData)) & 0xFF; \
           (pToData)[57] = ((pToData)[57] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[58] = ((pToData)[58] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[59] = ((pToData)[59] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[60] = ((pToData)[60] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT06(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[52] = ((pToData)[52] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[53] = ((pToData)[53] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[54] = ((pToData)[54] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[55] = ((pToData)[55] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT06(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[48] = ((nFromData)) & 0xFF; \
           (pToData)[49] = ((pToData)[49] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[50] = ((pToData)[50] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[51] = ((pToData)[51] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[52] = ((pToData)[52] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT05(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[44] = ((pToData)[44] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT05(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[40] = ((nFromData)) & 0xFF; \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[44] = ((pToData)[44] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT04(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[36] = ((pToData)[36] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT04(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[32] = ((nFromData)) & 0xFF; \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT03(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((pToData)[28] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT03(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT02(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((pToData)[20] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT02(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT01(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT01(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_PKTCNT00(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_SET_BYTECNT00(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[124]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[125]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[126]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[127]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[120]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[121]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[122]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[123]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[124]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[116]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[117]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[118]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[119]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[112]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[113]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[114]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[115]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[116]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[108]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[109]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[110]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[111]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[104]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[105]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[106]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[107]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[108]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[100]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[101]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[102]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[103]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[96]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[97]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[98]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[99]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[100]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[92]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[93]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[94]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[95]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[88]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[89]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[90]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[91]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[92]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[84]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[85]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[86]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[87]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[80]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[81]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[82]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[83]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[84]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT09(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[76]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[77]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[78]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[79]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT09(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[72]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[73]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[74]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[75]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[76]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT08(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[68]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[69]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[70]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[71]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT08(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[64]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[65]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[66]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[67]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[68]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT07(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[60]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[61]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[62]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[63]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT07(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[56]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[57]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[58]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[59]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[60]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT06(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[52]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[53]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[54]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[55]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT06(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[48]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[49]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[50]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[51]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[52]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT05(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[44]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[47]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT05(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[40]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[44]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT04(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[36]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT04(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[32]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[33]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[34]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[35]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT03(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[28]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[29]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[30]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[31]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT03(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[24]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[25]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[26]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[28]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT02(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[20]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT02(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[16]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT01(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[12]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT01(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[8]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_PKTCNT00(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[4]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCT_GET_BYTECNT00(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[3]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#endif
