/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpInstruction.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPINSTRUCTION_H
#define SB_ZF_ZFKAEPINSTRUCTION_H

#define SB_ZF_ZFKAEPINSTRUCTION_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEPINSTRUCTION_SIZE 4
#define SB_ZF_ZFKAEPINSTRUCTION_M_NVALID_BITS "31:31"
#define SB_ZF_ZFKAEPINSTRUCTION_M_NOPCODE_BITS "30:25"
#define SB_ZF_ZFKAEPINSTRUCTION_M_NOPAVARIABLE_BITS "24:24"
#define SB_ZF_ZFKAEPINSTRUCTION_M_NOPERANDA_BITS "23:18"
#define SB_ZF_ZFKAEPINSTRUCTION_M_NOPBVARIABLE_BITS "17:17"
#define SB_ZF_ZFKAEPINSTRUCTION_M_NOPERANDB_BITS "16:11"
#define SB_ZF_ZFKAEPINSTRUCTION_M_NOPCVARIABLE_BITS "10:10"
#define SB_ZF_ZFKAEPINSTRUCTION_M_NOPERANDC_BITS "9:0"
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_NOP             0
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VENCAP          1
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_DELETE          2
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_UNDELETE        3
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_MEENCAP0        4
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_MEENCAP1        5
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_MEENCAPV4       6
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_MEENCAPV6       7
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_ADD             8
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_SUB             9
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_SKIPEQ          10
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_SKIPNE          11
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_SHIFTR          12
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_SHIFTL          13
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_UEENCAP0        16
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_UEENCAP1        17
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_UEENCAPV4       18
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_UEENCAPV6       19
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_V6EENCAP        20
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_MENCAP          21
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_PREPEND         22
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_COUNT           23
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_DROP            24
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_REWRITE         26
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_SKIPIN          27
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_SKIPEX          28
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_LDV1 0x5
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_LDV2 0x6
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_LDVB 0x7
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_GROUP        0
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_VECTOR       1
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_MVTD         2
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_MVTD_VOP     3
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_MVTD_VID     4
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_ECN          5
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_T            6
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_LENGTH       7
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_63_60        8
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_59           9
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_LEN_ADJ      10
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_MC_GROUP     11
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_TCI_DMAC     11
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_MC           12
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_CRC          13
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_SRC          14
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_MVTDB        15
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_SW_OP        16
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_SID          17
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_17_16        18
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_IS           19
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_NDX          20
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_NDX_A        21
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_NDX_B        22
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_TYPE         23
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_CCOS         24
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_CDP          25
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_S            26
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_TTL          27
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_STTL         28
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_DQUEUE       29
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_DPORT        30
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_DCOS         31
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_DDP          32
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_CCOS_CDP     33
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_CLASS        34
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_PORT         35
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_MIRROR       36
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_PCDE         37
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_DROP         38 /*SET-ONLY on REWRITE or ARITH*/
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_RH_MTCI         39
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_V0           41
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_V1           42
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_V2           43
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_HW_V3           44
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_PL_LENGTH       58
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_PL_COS          59
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_PL_TTL          60
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_PL_ID           61
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_PL_TYPE         62
#define SB_ZF_ZFKAEPINSTRUCTION_KA_EP_VAR_PL_DA           63
#define SB_ZF_ZFKAEPINSTRUCTION_EP_TYPE_VLAN 0
#define SB_ZF_ZFKAEPINSTRUCTION_EP_TYPE_IPV4 1
#define SB_ZF_ZFKAEPINSTRUCTION_EP_TYPE_IPV6 2
#define SB_ZF_ZFKAEPINSTRUCTION_EP_TYPE_MPLS 3
#define SB_ZF_ZFKAEPINSTRUCTION_USE_SPRINT


typedef struct _sbZfKaEpInstruction {
  int32 m_nValid;
  int32 m_nOpcode;
  int32 m_nOpAVariable;
  int32 m_nOperandA;
  int32 m_nOpBVariable;
  int32 m_nOperandB;
  int32 m_nOpCVariable;
  int32 m_nOperandC;
} sbZfKaEpInstruction_t;

uint32
sbZfKaEpInstruction_Pack(sbZfKaEpInstruction_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex);
void
sbZfKaEpInstruction_Unpack(sbZfKaEpInstruction_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex);
void
sbZfKaEpInstruction_InitInstance(sbZfKaEpInstruction_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPINSTRUCTION_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPAV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPBV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEPINSTRUCTION_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPAV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPBV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPINSTRUCTION_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPAV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPBV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEPINSTRUCTION_SET_VALID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPAV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPBV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPCV(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_SET_OPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPINSTRUCTION_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPAV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPBV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[1] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[3] ; \
           (nToData) |= (int32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAEPINSTRUCTION_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPAV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPBV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[2] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[0] ; \
           (nToData) |= (int32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPINSTRUCTION_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPAV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPBV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[1] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[3] ; \
           (nToData) |= (int32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAEPINSTRUCTION_GET_VALID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPAV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPBV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (int32) ((pFromData)[2] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPCV(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPINSTRUCTION_GET_OPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[0] ; \
           (nToData) |= (int32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
