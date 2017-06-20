/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_H
#define SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_H

#define SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_SIZE_IN_BYTES 8
#define SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_SIZE 8
#define SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_UPROFILEID_BITS "63:53"
#define SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_URSVD_BITS "52:32"
#define SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_USEQUENCENUM_BITS "31:0"



/** @brief  MMU OAM Sequence Generator Raw Format 

  It's for internal use only
  Given that the actual layout per Sequence Generator
  as per the group configuration & profile configuration.
*/

typedef struct _sbZfFe2000PmOamSeqGenRawFormat {
/** @brief <p> Profile Id</p> */

  uint32 uProfileId;
/** @brief <p> time + sequenceNum</p> */

  uint32 uRsvd;
  uint32 uSequenceNum;
} sbZfFe2000PmOamSeqGenRawFormat_t;

uint32
sbZfFe2000PmOamSeqGenRawFormat_Pack(sbZfFe2000PmOamSeqGenRawFormat_t *pFrom,
                                    uint8 *pToData,
                                    uint32 nMaxToDataIndex);
void
sbZfFe2000PmOamSeqGenRawFormat_Unpack(sbZfFe2000PmOamSeqGenRawFormat_t *pToStruct,
                                      uint8 *pFromData,
                                      uint32 nMaxToDataIndex);
void
sbZfFe2000PmOamSeqGenRawFormat_InitInstance(sbZfFe2000PmOamSeqGenRawFormat_t *pFrame);

#define SB_ZF_FE2000PMOAMSEQGENRAWFORMAT_SET_PROFID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMOAMSEQGENRAWFORMAT_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x1f) | (((nFromData) >> 16) & 0x1f); \
          } while(0)

#define SB_ZF_FE2000PMOAMSEQGENRAWFORMAT_SET_SEQUENCENUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMOAMSEQGENRAWFORMAT_GET_PROFID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[7] << 3; \
          } while(0)

#define SB_ZF_FE2000PMOAMSEQGENRAWFORMAT_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x1f) << 16; \
          } while(0)

#define SB_ZF_FE2000PMOAMSEQGENRAWFORMAT_GET_SEQUENCENUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
