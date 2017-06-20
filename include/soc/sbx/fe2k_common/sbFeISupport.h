/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */

#ifndef _SB_FE_I_SUPPORT_H_
#define _SB_FE_I_SUPPORT_H_

/******************************************************************************
 *
 * $Id: sbFeISupport.h,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sbFeISupport.h
 *
 *****************************************************************************/

#include <soc/sbx/sbTypes.h>
typedef sbStatus_t (*sbMalloc_f_t)
     (void *chipUserToken, sbFeMallocType_t type, uint32 size,
      void **memoryP, sbDmaMemoryHandle_t *dmaHandleP);

typedef sbStatus_t (*sbFree_f_t)
     (void *chipUserToken, sbFeMallocType_t type, uint32 size,
      void *memoryP, sbDmaMemoryHandle_t dmaHandleP);

typedef sbSyncToken_t (*sbIsrSync_f_t)(void *chipUserToken);

typedef void (*sbIsrUnsync_f_t)(void *chipUserToken,
                                       sbSyncToken_t syncToken);

typedef enum sbFeHashGeneration_e {
  SB_FE_HASH_OVER_L2  = (1 << 0), 
  SB_FE_HASH_OVER_L3  = (1 << 1),
  SB_FE_HASH_OVER_L4  = (1 << 2),
  SB_FE_HASH_OVER_VID = (1 << 3)
}sbFeHashGeneration_t;

typedef struct sbCommonConfigParams_s {
  uint32 maximumContiguousDmaMalloc;
  sbMalloc_f_t sbmalloc;
  sbFree_f_t sbfree;
  void *clientData;
  sbIsrSync_f_t isrSync;
  sbIsrUnsync_f_t isrUnsync;
  sbFeAsyncCallback_f_t asyncCallback;
  uint32 uHashConfig;
} sbCommonConfigParams_t, *sbCommonConfigParams_p_t;

typedef struct sbIngressConfigParams_s {
  uint32 ipv4McMaxIps;  
  uint32 ipv4McSlabSize;
  uint32 ipv4McMaxDirty;
  uint32 payloadMax;
  uint32 l2MaxMacs;
  uint32 l2slabSize;
  uint32 clsMaxL2Keys;
  uint32 clsMaxRules;
  uint32 svidCompSlabSize;
  uint32 agingMaxDma;
  uint32 ipv4MaxRoutes;
  uint32 ipv4SlabSize;
  uint32 aceRuleDbs;
  uint32 aceRuleSetsPerDb;
  uint32 aceRulesPerChunk;
  uint32 aceMaxMemChunks;
  uint32 ipv6EMKeys;
  uint32 ipv6MaxRoutes;
  uint32 ipv6SlabSize;
  uint32 ipv6McMaxIps;
  uint32 ipv6McSlabSize;
  uint32 ipv4McGSlabSize;
  uint32 ipv4McGMaxDirty;
  /* (*,g) related */
  uint32 ipv4mcgSlabSize;
  uint32 ipv4mcgMaxDirty;
  /* (s,g) related */
  uint32 ipv4mcsgSlabSize;
  uint32 ipv4mcsgMaxDirty;
} sbIngressConfigParams_t, *sbIngressConfigParams_p_t;

typedef struct sbIngressUcodeParams_s {
  uint8 * ucodePackage;
  uint32 payloadStartSrc;
  uint32 payloadSizeSrc;
  uint32 payloadIpmcGStartSrc;
  uint32 payloadIpmcGSizeSrc;
  uint32 payloadl2StartSrc;
  uint32 payloadl2SizeSrc;
  uint32 payloadl2StartDst;
  uint32 payloadl2SizeDst;
  uint32 payloadipv6StartSrc;
  uint32 payloadipv6SizeSrc;
  uint32 payloadipv6StartDst;
  uint32 payloadipv6SizeDst;
  uint32 l2Table1Bits;
  uint32 l2SrcTable1Start;
  uint32 l2SrcTable1Bank;
  uint32 l2DstTable1Start;
  uint32 l2DstTable1Bank;
  uint32 l2MacTable1Start;
  uint32 l2MacTable1Bank;
  uint32 l2MacPayloadStart;
  uint32 l2MacPayloadSize;
  uint32 l2MacPayloadBank1;
  uint32 l2MacPayloadBank2;
  uint32 l2MacPayloadBank3;
  uint32 sVid2EtcLeftBase;
  uint32 sVid2EtcRightBase;
  uint32 sVid2EtcPaylBase;
  uint32 sVid2EtcPaylBaseRight;
  uint32 sVid2EtcSeedLoc;
  uint32 sVid2EtcLeftBank;
  uint32 sVid2EtcRightBank;
  uint32 sVid2EtcPaylBankLeft;
  uint32 sVid2EtcPaylBankRight;
  uint32 sVid2EtcTopOff;
  uint32 sVid2EtcTableBits;
  uint32 smacAgeBankBase;
  uint32 ipv4McTable1Bits;
  uint32 ipv4McTable1Base;
  /* For (*,G) Lookup. */
  uint32 ipv4McGTable1Bits;
  uint32 ipv4McGTable1Base;
  uint32 ipv4DstTable1Start;
  uint32 ipv4DstTable1Size;
  uint32 ipv4DstTable1Bank;

  uint32 ipv4SrcTable1Start;
  uint32 ipv4SrcTable1Size;
  uint32 ipv4SrcTable1Bank;
  uint32 ipv4SrcTable2Start;
  uint32 ipv4SrcTable2Size;
  uint32 ipv4SrcTable2BankA;
  uint32 ipv4SrcTable2BankB;
  uint32 ipv4SrcTable2StartB;
  uint32 ipv4SrcTable2SizeB;

  uint32 ipv4DstTable2BankA;
  uint32 ipv4DstTable2Start;
  uint32 ipv4DstTable2Size;
  uint32 ipv4DstTable2BankB;
  uint32 ipv4DstTable2StartB;
  uint32 ipv4DstTable2SizeB;
  uint32 ipv4McTbl1Start;
  uint32 ipv4McTbl1Size;
  /* For (*,G) Lookup. */
  uint32 ipv4McGTbl1Start;
  uint32 ipv4McGTbl1Size;
  uint32 ipv6SaEmTable1Bank;
  uint32 ipv6DaEmTable1Bank;
  uint32 ipv6SaEmTable1Base;
  uint32 ipv6DaEmTable1Base;
  uint32 ipv6SaEmTable1Bits;
  uint32 ipv6DaEmTable1Bits;
  uint32 ipv6TableSize1;
  uint32 ipv6SrcTableInSramA;
  uint32 ipv6SrcTableStart1;
  uint32 ipv6SrcTableSize1;
  uint32 ipv6DstTableStart1;
  uint32 ipv6DstTableSize1;
  uint32 ipv6SrcTableBank2;
  uint32 ipv6SrcTableStart2;
  uint32 ipv6SrcTableSize2;
  uint32 ipv6DstTableBank2;
  uint32 ipv6DstTableStart2;
  uint32 ipv6DstTableSize2;
  uint32 ipv6McTable1Bits;
  uint32 ipv6McTable1Base;
  uint32 payloadipv6McStart;
  uint32 payloadipv6McSize;
  /* Similar to already existing fields - deprecate the previous ones for lack of clear naming */
  /* (s,g) lookup */
  uint32 ipv4mcsgTable1Bits;
  uint32 ipv4mcsgTable1Base;
  uint32 ipv4mcsgTable1Bank;
  uint32 ipv4mcsgTable1Start;
  uint32 payloadipv4mcsgStart;
  uint32 payloadipv4mcsgSize;
  uint32 payloadipv4mcsgBank;
  uint32 payloadipv4mcsgBank2;
  uint32 payloadipv4mcsgBank3;
  /* (*,g) lookup */
  uint32 ipv4mcgTable1Bits;
  uint32 ipv4mcgTable1Base;
  uint32 ipv4mcgTable1Bank;
  uint32 ipv4mcgTable1Start;
  uint32 payloadipv4mcgStart;
  uint32 payloadipv4mcgSize;
  uint32 payloadipv4mcgBank;
  uint32 payloadipv4mcgBank2;
} sbIngressUcodeParams_t, *sbIngressUcodeParams_p_t;

/* SWS and PPE ucode Port Configuration Information */
#define SBX_SWS_QUEUES    128
typedef struct sbFeSwsConfig_s {
  uint32  qid2PortId[SBX_SWS_QUEUES];
  uint32  port2QidId[SBX_SWS_QUEUES/2];
}sbFeSwsConfig_t;

/**
 * Type codes for the asynchronous event callback
 */
typedef enum sbG2FeAsyncType_e {
  SB_FE_ASYNC_INIT_DONE,     
  SB_FE_L2_ASYNC_COMMIT_DONE,
  SB_FE_L2_ASYNC_SMAC_UPDATE_DONE,
  SB_FE_L2_ASYNC_DMAC_UPDATE_DONE,
  SB_FE_L2_ASYNC_SMAC_GET_DONE,  
  SB_FE_L2_ASYNC_DMAC_GET_DONE, 
  SB_FE_ASYNC_SVID_COMMIT_DONE,
  SB_FE_ASYNC_SMAC_AGE_SET_DONE,
  SB_FE_ASYNC_SMAC_AGING_DONE,
  SB_FE_ASYNC_OLD_SMAC,
  SB_FE_ASYNC_IPV4MC_COMMIT_DONE,
  SB_FE_ASYNC_IPV4MC_GET_DONE,
  SB_FE_ASYNC_DMAC_UPDATE_DONE,
  SB_FE_ASYNC_SMAC_UPDATE_DONE,
  SB_FE_ASYNC_IPV4_COMMIT_DONE,
  SB_FE_ASYNC_IPV6EM_COMMIT_DONE,
  SB_FE_ASYNC_IPV6SAEM_UPDATE_DONE,
  SB_FE_ASYNC_IPV6DAEM_UPDATE_DONE,
  SB_FE_ASYNC_IPV6_COMMIT_DONE,
  SB_FE_ASYNC_IPV6MC_COMMIT_DONE,
  SB_FE_ASYNC_IPV6MC_UPDATE_DONE,
  SB_FE_ASYNC_IPV6MC_GET_DONE,
  SB_FE_ASYNC_EGR_LAG_DONE,
  SB_FE_ASYNC_EGR_SHIM_HEADER_EXPROC_DONE,
  SB_FE_ASYNC_EGR_SHIM_HEADER_MC_DONE,
  SB_FE_ASYNC_EGR_SHIM_HEADER_UC_DONE,
  SB_FE_ASYNC_ETE_RAW_DONE,
  SB_FE_ASYNC_ETE_L2_DONE,
  SB_FE_ASYNC_ETE_MPLS_DONE,
  SB_FE_ASYNC_ETE_IPV4_DONE,
  SB_FE_ASYNC_ETE_IPV4ENCAP_DONE,
  SB_FE_ASYNC_ETE_IPV6_DONE,
  SB_FE_ASYNC_ETE_L2ENCAP_DONE,
  SB_FE_ASYNC_ETE_MIM_DONE,
  SB_FE_ASYNC_EGR_REMARK_DONE,
  SB_FE_ASYNC_FTE_DONE,
  SB_FE_ASYNC_ING_LAG_DONE,
  SB_FE_ASYNC_PORT_SID_DROP_DONE,
  SB_FE_ASYNC_ISID2ETC_DONE,
  SB_FE_ASYNC_L2PROTO2PORT_DONE,
  SB_FE_ASYNC_LABEL2ETC_DONE,
  SB_FE_ASYNC_LABELVID2ETC_DONE,
  SB_FE_ASYNC_OUT_HDRINDEX2ETC_DONE,
  SB_FE_ASYNC_EGR_VLANPORT2ETC_DONE,
  SB_FE_ASYNC_PORT2ETC_DONE,
  SB_FE_ASYNC_PORT2SID_DONE,
  SB_FE_ASYNC_PORTPROTO2VLAN_DONE,
  SB_FE_ASYNC_PSTACKEDVID2ETC_DONE,
  SB_FE_ASYNC_PSTACKEDVIDSEED_DONE,
  SB_FE_ASYNC_PVID2ETC_DONE,
  SB_FE_ASYNC_MCPORT2ETC_DONE,
  SB_FE_ASYNC_PROT_DONE,
  SB_FE_ASYNC_EGRPVID2ETC_DONE,
  SB_FE_ASYNC_QIDRR_DONE,
  SB_FE_ASYNC_QOSPROFILE_DONE,
  SB_FE_ASYNC_QOS_MAP_DONE,
  SB_FE_ASYNC_REPSETENTRY_DONE,
  SB_FE_ASYNC_SMAC_IDX2SMAC_DONE,
  SB_FE_ASYNC_TUNNEL2ETC_DONE,
  SB_FE_ASYNC_VLAN2ETC_DONE,
  SB_FE_ASYNC_ROUTED_VLAN2ETC_DONE,
  SB_FE_ASYNC_RP2ETC_DONE,
  SB_FE_ASYNC_RULETABLE_DONE,
  SB_FE_ASYNC_EGRRULES_DONE,
  SB_FE_ASYNC_VRF2ETC_DONE,
  SB_FE_ASYNC_EXCEPTION_DONE,
  SB_FE_ASYNC_ING_PROT_DONE,
  SB_FE_ASYNC_L1SA_DONE,
  SB_FE_ASYNC_L1DA_DONE,
  SB_FE_ASYNC_TPID_DONE,
  SB_FE_ASYNC_CLS_COMMIT_DONE,
  SB_FE_ASYNC_OAMTIMERCALENDAR_DONE,
  SB_FE_ASYNC_OAMINSTANCE2ETC_DONE,
  SB_FE_ASYNC_OAMPORTMDLEVEL2ETC_DONE,
  SB_FE_ASYNC_OAMPORT2ETC_DONE,
  SB_FE_ASYNC_OAMENDPOINTRECORD_DONE,
  SB_FE_ASYNC_OAMENDPOINT2PEER_DONE,
  SB_FE_ASYNC_POLICER_DONE,
  SB_FE_ASYNC_INGMIRROR_DONE,
  SB_FE_ASYNC_EGRMIRROR_DONE,
  SB_FE_ASYNC_PORTL2CP2ETC_DONE,
  SB_FE_ASYNC_DMAC_IDX2DMAC_DONE,
  SB_FE_ASYNC_L2MAC_UPDATE_DONE,
  SB_FE_ASYNC_L2MAC_GET_DONE,
  SB_FE_ASYNC_L2MAC_COMMIT_DONE,
  SB_FE_ASYNC_FABRICCOSREMAP_DONE,
  SB_FE_ASYNC_TYPES_MAX
} sbG2FeAsyncType_t;


#endif
