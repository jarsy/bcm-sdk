/*
 * $Id: bm9600_init.h,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef BM9600_INIT_H
#define BM9600_INIT_H

#include "sbTypes.h"
#include "bm9600.h"

#define BOOL int

#define PL_MAX(a, b)                       ((a) > (b) ? (a) : (b))
#define PL_MIN(a, b)                       ((a) < (b) ? (a) : (b))



/*
                                                                        
 ALL CONSTRAINT DEFINES SHOULD BE ADDED TO bm9600Properties.h
                                                                        
*/
#include "bm9600_properties.h"

/*  Each Hypercore supports a Quad of SI ports */
#define PL_SI_PER_HC (4)
/*  Hypercore Lane Mode values */
#define PL_HC_LANE_MODE_HALF_SPEED 0
#define PL_HC_LANE_MODE_FULL_SPEED 1

/* Loopback register offset */
#define HW_BM9600_HYPERCORE_XGXSBLK1_LANECTRL2_OFFSET             0x08017
#define HW_BM9600_HYPERCORE_SERDESDIGITAL_MISC_CONTROL2_OFFSET    0x08309
#define HW_BM9600_HYPERCORE_AER_LANE_SELECT                       0x0ffde

/*  INA defines */
#define PL_INA_DEFAULT_HOLD_PRI (0xC)
#define PL_INA_DEFAULT_MAX_PRI (0xF)
#define INA_RAND_NUM_TABLE_NUM_ENTRIES  55
#define NM_RAND_NUM_TABLE_NUM_ENTRIES   55
#define NM_INGRESS_RANKER_TABLE_NUM_ENTRIES  73
#define NM_EGRESS_RANKER_TABLE_NUM_ENTRIES  73

#define HW_BM9600_MAX_SERIALIZERS           (96)
#define HW_BM9600_MAX_PLANES                (2)

#define HW_BM9600_STATUS_OK                   (0)
#define HW_BM9600_STATUS_BAD_ARG              (1)
#define HW_BM9600_STATUS_TIMEOUT              (2)
#define HW_BM9600_STATUS_FATAL_ERROR          (3)

/* These defines are the timeouts for acks on all BM9600 init code */
/* these can be updated to increase timeout.                       */
#define HW_BM9600_TIMEOUT_GENERAL           100000 /* usec */
#define HW_BM9600_POLL_GENERAL              1000   /* minimum polls */


typedef enum _bm9600ChanelType {
  DATA    = 0,
  CONTROL = 1
} bm9600ChanelType_t;

typedef enum _bm9600NodeType {
  QE2000    = 0,
  QE4000    = 1
} bm9600NodeType_t;

typedef enum _bm9600XcfgRemapType {
  BM9600_XCFG_REMAP_INIT0    = 0,
  BM9600_XCFG_REMAP_INIT1    = 1
} bm9600XcfgRemapType_t;

typedef struct _bm9600InitParamsSi {
  BOOL bBringUp;
  BOOL bIsEnabled;
  UINT uJitTolerance;
  BOOL bEvenChannelOn;
  BOOL bOddChannelOn;
  UINT uSerdesAbility;
  bm9600ChanelType_t eChannelType;
  bm9600NodeType_t eNodeType;
} bm9600InitParamsSi_t;

typedef struct _bm9600InitParamsAi {
  BOOL bBringUp;
  BOOL bIsEnabled;
  BOOL bExpandedQe2kEsetSpace;
} bm9600InitParamsAi_t;

typedef struct _bm9600InitParamsFo {
  BOOL bBringUp;
  UINT uLinkEnable;
  UINT uDefaultBmId;
  UINT uForceNullGrant;
  UINT uNumNodes;
  UINT uUseGlobalLinkEnable;
  UINT uMaxDisLinks;
  UINT uLinkDisTimerEnable;
  UINT uAutoLinkDisEnable;
  UINT uLinkDisTimeoutPeriod;
  UINT uFailoverTimerEnable;
  UINT uAutoFailoverEnable;
  UINT uFailoverTimeoutPeriod;
  UINT uLocalBmId;
  UINT uAcDegTimeslotSize;   /* set but not implemented */
  UINT uNumNullGrants;
  UINT uAcTsToGrantOffset;
  UINT uAcTsToNmTsOffset;
  UINT uAcTimeslotSize[BM9600_NUM_TIMESLOTSIZE];
  UINT uForceLinkSelect[BM9600_FO_NUM_FORCE_LINK_SELECT];
  UINT uForceLinkActive[BM9600_FO_NUM_FORCE_LINK_ACTIVE];
} bm9600InitParamsFo_t;

typedef struct _bm9600WredCurveTableEntry {
  UINT uMinDp[BM9600_BW_WRED_DP_NUM];
  UINT uMaxDp[BM9600_BW_WRED_DP_NUM];
  UINT uEcnDp[BM9600_BW_WRED_DP_NUM];
  UINT uScaleDp[BM9600_BW_WRED_DP_NUM];
  UINT uSlopeDp[BM9600_BW_WRED_DP_NUM];
} bm9600WredCurveTableEntry_t;

typedef struct _bm9600InitParamsBw {
  BOOL bBringUp;
  UINT uMaxResponseLatencyInTimeSlots;
  UINT uRxSotToBaaOffset;
  BOOL bTagCheckEnable;
  UINT uActiveBagNum;
  UINT uActiveVoqNum;
  BOOL bBandwidthAllocationEnable;
  BOOL bWredEnable;
  BOOL bAllocationScaleEnable;
  BOOL bIgnoreTimeout;
  UINT uGroupsPerPort;
  UINT uFetchSwap;
  UINT uDistribSwap;
  UINT uSegmentSize;
  UINT uEpochLength;
  UINT auVoqNumInBag[BM9600_BW_MAX_BAG_NUM];
  bm9600WredCurveTableEntry_t azCurveTable[BM9600_BW_WRED_CURVE_TABLE_SIZE];
} bm9600InitParamsBw_t;

typedef struct _bm9600InitParamsIna {
  BOOL bBringUp;
  BOOL bIsEnabled;
  BOOL bCritUpdNcudEnable;
  BOOL bRandGenEnable;
  UINT uEsetLimit;
  BOOL bFilterPriUpdates;
  UINT uHoldPri;
  UINT uMaxPriPri;
  UINT uStrobeDelay;
} bm9600InitParamsIna_t;

typedef struct _bm9600InitParamsNm {
  BOOL bBringUp;
  BOOL bInitRankerTables;
  BOOL bRandGenEnable;
  BOOL bEnableAllEgress;
  UINT uTsToEsetShiftOffset;
  UINT uNmFullShiftOffset;
  UINT uGrantToFullShiftOffset;
  UINT uNmDualGrantConfig0;
  UINT uNmDualGrantConfig1;
  UINT uNmDualGrantConfig2;
  UINT pIngressRankerTable[NM_INGRESS_RANKER_TABLE_NUM_ENTRIES];  /* fill these in */
  UINT pEgressRankerTable[NM_EGRESS_RANKER_TABLE_NUM_ENTRIES];
  UINT pRandTable[NM_RAND_NUM_TABLE_NUM_ENTRIES];
} bm9600InitParamsNm_t;

typedef struct _bm9600InitParamsPi {
  BOOL bBringUp;
} bm9600InitParamsPi_t;

typedef enum _bm9600XcfgMode {
  XCFG_SE_MODE                  = 0,
  XCFG_LCM_MODE_FIXED_ADDRESS_A = 1,
  XCFG_LCM_MODE_FIXED_ADDRESS_B = 2,
  XCFG_LCM_MODE_EXT_A_B_PIN     = 3
} bm9600XcfgMode_t;

typedef struct _bm9600InitParamsXb {
  BOOL bBringUp;
  UINT uInitMode;
  UINT uPassThru; /* number of 16b words */
  bm9600XcfgMode_t eXcfgMode;
  BOOL bBypass;
  UINT uTsJitterTolerance;
  BOOL bCheckGlobalAlignment;
  BOOL bEnableSotPolicing;
  BOOL bEnableXcfgReplace;
} bm9600InitParamsXb_t;

typedef struct _bm9600InitParamsSerX {
  UINT uPeriod;
}bm9600InitParamsSerX_t;

typedef struct _bm9600InitParams {
  BOOL bCleanUp;

  BOOL bZvCore;
  BOOL bAllowBroadcast;
  BOOL bEnableEcc;
  BOOL bInaSysportMap;
  UINT m_nBaseAddr;

  BOOL bIsLcm;

  UINT uNumLogicalXbars;
/* settings for WRED */
  UINT uDemandClks;
  UINT uBwModeLatency;
  UINT uTimeslotSizeNs;
  UINT uNumEsetEntries;
  UINT uSerdesSpeed;    /* global setting for all SI */
  BOOL bSerdesEncoding; /* global setting for all SI */
  UINT uSpMode;

  /*  Used to initialize serdes for Si links */
  bm9600InitParamsSerX_t serx[BM9600_NUM_LINKS];

  bm9600InitParamsSi_t si[BM9600_NUM_LINKS];
  bm9600InitParamsAi_t ai[BM9600_NUM_LINKS];
  bm9600InitParamsIna_t ina[BM9600_NUM_LINKS];

  bm9600InitParamsFo_t fo;
  bm9600InitParamsBw_t bw;

  bm9600InitParamsNm_t nm;
  bm9600InitParamsPi_t pi;
  bm9600InitParamsXb_t xb;

  uint32   uLcmXcfg[HW_BM9600_MAX_PLANES][HW_BM9600_MAX_SERIALIZERS];
  uint32   uLcmPlaneValid[HW_BM9600_MAX_PLANES];
  uint32   uSiLsThreshold;
  uint32   uSiLsWindow;
} bm9600InitParams_t;

#define FO_INIT_TIMEOUT (100)
#define BW_INIT_TIMEOUT (100)
#define XB_INIT_TIMEOUT (100)

typedef struct _qeLogical2NodeAndPort {
  UINT uOffset;
  UINT uPort[BM9600_NUM_LINKS];
  UINT uNode[BM9600_NUM_LINKS];
  bm9600NodeType_t zNodeType[BM9600_NUM_LINKS];
} qeLogical2NodeAndPort_t;

int hwBm9600EsetSet(int nUnit, uint32 uEset, uint64 uLowNodesMask, uint32 uHiNodesMask, uint32 uMcFullEvalMin, uint32 uEsetFullStatusMode);
int hwBm9600EsetGet(int nUnit, uint32 uEset, uint64 *pLowNodesMask, uint32 *pHiNodesMask, uint32 *pMcFullEvalMin, uint32 *pEsetFullStatusMode);


void GetDefaultBmInitParams(int unit, bm9600InitParams_t *pInitParams);
void GetDefaultSeInitParams(int unit, bm9600InitParams_t *pInitParams);

uint32 hwBm9600Init(bm9600InitParams_t *pInitParams);

#if 0
  void BringOutOfReset(const bm9600InitParams_t *pInitParams);
  void BringUpStep0(const bm9600InitParams_t *pInitParams);
  void BringUpStep1(const bm9600InitParams_t *pInitParams);
  void CleanUp(const bm9600InitParams_t *pInitParams);

  BOOL m_bInaEnabled[BM9600_NUM_LINKS];

  void CleanUpSi(INT nSi, const bm9600InitParams_t *pInitParams);
  void CleanUpAi(INT nAi, const bm9600InitParams_t *pInitParams);
  void CleanUpIna(INT nIna, const bm9600InitParams_t *pInitParams);
  void CleanUpFo(const bm9600InitParams_t *pInitParams);
  void CleanUpBw(const bm9600InitParams_t *pInitParams);
  void CleanUpNm(const bm9600InitParams_t *pInitParams);
  void CleanUpPi(const bm9600InitParams_t *pInitParams);
  void CleanUpXb(const bm9600InitParams_t *pInitParams);

  void VerifyInterrupt(UINT uError,         UINT uUnit0Error= 0, UINT uUnit1Error= 0, UINT uUnit2Error= 0 ,
		       UINT uUnit3Error= 0, UINT uUnit4Error= 0, UINT uUnit5Error= 0,
		       UINT uUnit6Error= 0, UINT uUnit7Error= 0, UINT uUnit8Error= 0,
		       UINT uUnit9Error= 0);
  UINT CheckInterruptReg(INT nOffset, UINT uValue, BOOL bCheck, BOOL bClear, BOOL bReturnMasked=TRUE);
  void TraceErrorRegs(CZString sBlockName, BOOL bClear, BOOL bUnMask=FALSE);
  void TraceInterruptReg(INT nOffset, UINT uValue, BOOL bClear);
  UINT GetTimeSlotSizeInClocks();
/* UINT GetTimeSlotSizeInClocks(UINT uNodeNum); */
  void ClearAllInterrupts();

  INT  CreateXbarRemapEntry( UINT uQeLogicalPort, UINT uSeLogicalPort, UINT uBmeInaId, bm9600NodeType_t zNodeType);


#endif

#endif /* BM9600_INIT_H */
