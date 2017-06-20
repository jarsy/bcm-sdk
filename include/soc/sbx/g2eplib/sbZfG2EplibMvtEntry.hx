/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfG2EplibMvtEntry.hx,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_G2_EPLIB_MVTENTRY_H
#define SB_ZF_G2_EPLIB_MVTENTRY_H

#define SB_ZF_G2_EPLIB_MVTENTRY_SIZE_IN_BYTES 1
#define SB_ZF_G2_EPLIB_MVTENTRY_SIZE 1
#define SB_ZF_G2_EPLIB_MVTENTRY_ULLPORTMASK_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_NTYPE_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_ULMVTDA_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_ULMVTDB_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_BSOURCEKNOCKOUT_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_BENABLECHAINING_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_ULNEXTMCGROUP_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_SET_VLAN(_VLAN_, pZf) (pZf)->ulMvtdA = ((_VLAN_) & 0xfff);  (pZf)->ulMvtdB &= 0x1 
#define SB_ZF_G2_EPLIB_MVTENTRY_GET_VLAN(_VLAN_, pZf) _VLAN_ = ((pZf)->ulMvtdA) & 0xfff);
#define SB_ZF_G2_EPLIB_MVTENTRY_SET_OIX(_IDX_, pZf)  (pZf)->ulMvtdA = ((_IDX_) & 0xfff); (pZf)->ulMvtdA |= ((_IDX_) & 0x18000) >> 3; (pZf)->ulMvtdB =((_IDX_) >> 12) & 0x7 
#define SB_ZF_G2_EPLIB_MVTENTRY_GET_OIX(_IDX_, pZf)  _IDX_ = ((pZf)->ulMvtdA & 0xfff); _IDX_ |= ((pZf)->ulMvtdA << 3) & 0x18000; _IDX_ |= ((pZf)->ulMvtdB & 0x7) <<  12
#define SB_ZF_G2_EPLIB_MVTENTRY_SET_IPMC(_IPMC_, pZf)  ( (pZf)->ulMvtdB = (_IPMC_) & 0x1 )
#define SB_ZF_G2_EPLIB_MVTENTRY_GET_IPMC(_IPMC_, pZf)  _IPMC_ =  (pZf)->ulMvtdB & 0x1



/** @brief  EP MVT Entry

  EP MVT Entry 
*/

typedef struct _sbZfG2EplibMvtEntry {
  uint64 ullPortMask;
  uint32 nType;
  uint32 ulMvtdA;
  uint32 ulMvtdB;
  uint8 bSourceKnockout;
  uint8 bEnableChaining;
  uint32 ulNextMcGroup;
} sbZfG2EplibMvtEntry_t;

uint32
sbZfG2EplibMvtEntry_Pack(sbZfG2EplibMvtEntry_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex);
void
sbZfG2EplibMvtEntry_Unpack(sbZfG2EplibMvtEntry_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex);
void
sbZfG2EplibMvtEntry_InitInstance(sbZfG2EplibMvtEntry_t *pFrame);

#define SB_ZF_G2EPLIBMVTENTRY_SET_PORTMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_MVTDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_MVTDB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_SOURCEKNOCKOUT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_ENABLECHAINING(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_NEXTMCGROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_PORTMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_MVTDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_MVTDB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_SOURCEKNOCKOUT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_ENABLECHAINING(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_NEXTMCGROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
