/*
 * $Id: Oam.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     Oam.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_Oam_H
#define _SOC_EA_Oam_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Ethernet.h>

#if !defined(OAM_TLV_MAX_LEN)
#define OAM_TLV_MAX_LEN     256
#endif

/* pack structures for ARM */
#if defined(UNIX) || defined(VXWORKS) || defined(LINUX)
#pragma pack (1)
#endif

#if !defined(OAM_VERSION)
#define OAM_VERSION 320
#endif

/* An IEEE Organization Unique Identifier */
typedef struct {
    uint8 byte[3];
} PACK IeeeOui;


typedef enum {
    LacpSlowProtocolSubtype = 0x01, /* Link Agregation Control Protocol */
    LampSlowProtocolSubtype = 0x02, /* Link Agregation Marker Protocol */
    OamSlowProtocolSubtype  = 0x03
} PACK SlowProtocolSubtype;


/* standard OAM opcodes */
typedef enum {
    OamOpInfo,
    OamOpEventNotification,
    OamOpVarRequest,
    OamOpVarResponse,
    OamOpLoopback,
    OamOpVendorOui          = 0xfe
} PACK OamOpcode;

/* common structure for all OAM PDUs */
typedef struct {
    uint8   subtype;    /* should be 0x03 for OAM */
    uint16  flags;
    uint8   opcode;
} PACK OamMsg;

typedef struct {
    OamMsg  OamHead;    /* Excluding SA/DA/EtherType, last field is "OpCode" */
    uint8   OamData[1];
} PACK OamPdu;

typedef struct {
    EthernetFrame EthHeader;
    OamPdu Data;
} PACK OamFrame;


#define OamMsgFlagLinkFault         0x0001
#define OamMsgFlagDyingGasp         0x0002
#define OamMsgFlagCriticalEvent     0x0004

#define OamMsgFlagLocalDiscMsk      0x0018
#define OamMsgFlagLocalDiscSft      3
#define OamMsgFlagRemoteDiscMsk     0x0060
#define OamMsgFlagRemoteDiscSft     5

/* values for Local/RemoteDisc fields */
#define OamMsgFlagDiscFailed        0
#define OamMsgFlagDiscUnsatisfied   1
#define OamMsgFlagDiscComplete      2
#define OamMsgFlagDiscInvalid       3


#define MaxLoidLen                  24
#define MaxAuthPassLen              12

/*
 * Info PDUs contain one or two TLVs following the common msg header
 */
typedef enum {
    OamTlvNull          = 0x00,
    OamTlvLocalInfo     = 0x01,
    OamTlvRemoteInfo    = 0x02,
    OamTlvOrgSpecific   = 0xfe
} PACK OamTlvType;


typedef struct {
    uint8   flags;
} PACK OamInfoState;


/* IEEE 802.3ah ID information */
typedef struct {
    IeeeOui vendor;
    uint16  product;     /*< format up to the vendor */
    uint16  version;     /*< format up to the vendor */
} PACK OamId;


typedef struct {
    uint8           type;
    uint8           length;
    uint8           version;
    uint16          revision;
    OamInfoState    state;
    uint8           configuration;
    uint16          pduConfiguration;
    OamId           id;
} PACK OamInfoTlv;


#define OamInfoVersion              1

/* OAM configuration bits */
#define OamInfoConfigModeActive     0x0001
#define OamInfoConfigUnidir         0x0002
#define OamInfoConfigLoopCapable    0x0004
#define OamInfoConfigLoopEvents     0x0008
#define OamInfoConfigVarResp        0x0010

/* OAMPDU configuration bits */
#define OamInfoPduConfigSizeMsk     0x07ff
#define OamInfoPduConfigSizeSft     0

/* identifies which type of TK-extended TLV this is */
typedef enum {
    TkInfoTlvLink,          /* EPON laser parameters */
    TkInfoTlvKey,           /* Key exchange */
    TkInfoTlvAlarmRequest,  /* Alarm audit request */
    TkInfoTlvTeknovus,      /* Identify OLT/ONU as Teknovus */
    TkInfoTlvNumTlvs
} PACK OamInfoTkTlvType;

/* Teknovus-specific TLV header */
typedef struct {
    uint8   type;           /* FE for org-specific extension */
    uint8   length;         /* includes type/len bytes */
    IeeeOui oui;            /* 00-0D-B6 for Teknovus */
    uint8   tkType;
} PACK OamInfoTkTlvHeader;

/* TkInfoTlvLink: EPON overhead data */
typedef struct {
    uint16  burstCap;
    uint16  minOam;
    uint16  maxOam;
    uint16  laserOn;
    uint16  laserOff;
} PACK OamInfoTkLink;

/* TkInfoTlvKey: key exchange during discovery */
/* followed by keyLength bytes of key data, MSB first */
typedef struct {
    uint8   keyNumber;
    uint8   keyLength;
} PACK OamInfoTkKey;

/* TkInfoTlvAlarmRequest: request alarm message */
/* no parameters */

/* OAM Info msg */
typedef struct {
    OamMsg  common;
} PACK OamInfoMsg;


/*
 * Event Notification
 * events have a number of event TLVs following the message header
 */

typedef struct {
    OamMsg  common;
    uint16  seqNum;
} PACK OamEventNotificationMsg;

/* Event Notification TLVs */

typedef enum {
    OamEventEndOfTlvMarker      = 0,
    OamEventErrSymbolPeriod     = 1,
    OamEventErrFrameCount       = 2,
    OamEventErrFramePeriod      = 3,
    OamEventErrFrameSecSummary  = 4,
    OamEventErrVendor           = 0xfe,
    OamEventErrVendorOld        = 0xff
} PACK OamEventType;

/* All TLVs start like this */
typedef struct {
    uint8   type;
    uint8   length;
} PACK OamEventTlv;

/*  OAM errored symbol period */
typedef struct {
    OamEventTlv tlv;
    uint16      timestamp;      /* 100 ms intervals */
    uint64      window;         /* time, in number of symbols (1s) */
    uint64      threshold;      /* min errors to cause event (1) */
    uint64      errors;         /* errors this period */
    uint64      runningTotal;   /* all errors since reset */
    uint32      eventCount;     /* number of these events generated */
} PACK OamEventErrSymPeriod;

/*  OAM errored frame  */
typedef struct {
    OamEventTlv tlv;
    uint16      timestamp;      /* 100 ms intervals */
    uint16      window;         /* time, in 100ms intervals (1s) */
    uint32      threshold;      /* min errors to cause event (1) */
    uint32      errors;         /* errors this period */
    uint64      runningTotal;   /* all errors since reset */
    uint32      eventCount;     /* number of these events generated */
} PACK OamEventErrFrame;

/*  OAM errored frame period */
typedef struct {
    OamEventTlv tlv;
    uint16      timestamp;      /* 100 ms intervals */
    uint32      window;         /* time, in min size frame times (1s) */
    uint32      threshold;      /* min errors to cause event (1) */
    uint32      errors;         /* errors this period */
    uint64      runningTotal;   /* all errors since reset */
    uint32      eventCount;     /* number of these events generated */
} PACK OamEventErrFrPeriod;

/*  OAM errored frame seconds summary */
typedef struct {
    OamEventTlv tlv;
    uint16      timestamp;      /* 100 ms intervals */
    uint16      window;         /* time, in min size frame times (60s) */
    uint16      threshold;      /* min errors to cause event(1) */
    uint16      errors;         /* errors this period */
    uint32      runningTotal;   /* all errors since reset */
    uint32      eventCount;     /* number of these events generated */
} PACK OamEventErrFrSecondsSum;

/*  Vendor-extended TLV */
typedef struct {
    OamEventTlv tlv;
    IeeeOui     oui;
} PACK OamEventExt;


/*
 * Variable Request/Response    
 */

/*  Allowed values for "branch" */
typedef enum {
    OamBranchTermination    = 0x00,
    OamBranchObject         = 0x03,
    OamBranchPackage        = 0x04,
    OamBranchNameBinding    = 0x06,
    OamBranchAttribute      = 0x07,
    OamBranchAction         = 0x09,
    OamBranchMax            = 0xff
} PACK OamVarBranch;

/*  Leaf values for branch == OamBranchObject */
typedef enum {
    OamObjLeafMacEntity     = 0x01,
    OamObjLeafPhyEntity     = 0x02,
    OamObjLeafMacCtrl       = 0x08,
    OamObjForce16           = 0x7fff
} PACK OamObjLeaf;

/*  Leaf values for branch == OamBranchPackage   */
typedef enum {
    OamPkgLeafIllegal               = 0x00,
    OamPkgLeafMacMandatory          = 0x01,
    OamPkgLeafMacRecommended        = 0x02,
    OamPkgLeafMacOptional           = 0x03,
    OamPkgLeafMacArray              = 0x04,
    OamPkgLeafMacExcessiveDeferral  = 0x05,
    OamPkgLeafPhyRecommended        = 0x06,
    OamPkgLeafPhyMultiplePhy        = 0x07,
    OamPkgLeafPhy100MpbsMonitor     = 0x08,
    OamPkgLeafRepeaterPerfMonitor   = 0x09,
    OamPkgLeafPortPerfMonitor       = 0x0a,
    OamPkgLeafPortAddrTrack         = 0x0b,
    OamPkgLeafMauControl            = 0x0d,
    OamPkgLeafMediaLowTracking      = 0x0e,
    OamPkgLeafBroadbandMau          = 0x0f,
    OamPkgLeafMacControlRecommended = 0x11,
    OamPkgForce16                   = 0x7fff
} PACK OamPkgLeaf;


/*  Leaf values for branch == OamBranchNameBinding */
typedef enum {
    OamNameMacName          = 1,
    OamNameMacMonitor       = 2,
    OamNameQueueName        = 2,    /* duplicate value OK */
    OamNamePhyName          = 3,
    OamNamePhyMonitor       = 4,
    OamNameAllQueues        = 4,    /* reused value */
    OamNameMacCtrlSystem    = 18,
    OamNameMacCtrlMonitor   = 15,
    OamNameMauRptName       = 9,
    OamNameMauDteName       = 10,
    OamNameRsrcMac          = 12,
    OamNameRsrcRepeater     = 13,
    OamNameRsrcMau          = 14,
    OamNameForce16          = 0x7fff
} PACK OamNameLeaf;

/*  Leaf values for branch == OamBranchAttribute */
typedef enum {
    OamAttrLeafIllegal              = 0x00,

    /* MAC attributes */
    OamAttrMacId                    = 1,
    OamAttrMacFramesTxOk            = 2,
    OamAttrMacSingleCollFrames      = 3,
    OamAttrMacMultipleCollFrames    = 4,
    OamAttrMacFramesRxOk            = 5,
    OamAttrMacFcsErr                = 6,
    OamAttrMacAlignErr              = 7,
    OamAttrMacOctetsTxOk            = 8,
    OamAttrMacFramesDeferred        = 9,
    OamAttrMacLateCollisions        = 10,
    OamAttrMacExcessiveCollisions   = 11,
    OamAttrMacFramesLostMacTxErr    = 12,
    OamAttrMacCarrierSenseErr       = 13,
    OamAttrMacOctetsRxOk            = 14,
    OamAttrMacFramesLostMacRxErr    = 15,
    OamAttrMacPromiscuousStatus     = 16,
    OamAttrMacMcastAddrList         = 17,
    OamAttrMacMcastFramesTxOk       = 18,
    OamAttrMacBcastFramesTxOk       = 19,
    OamAttrMacFrExcessiveDeferral   = 20,
    OamAttrMacMcastFramesRxOk       = 21,
    OamAttrMacBcastFramesRxOk       = 22,
    OamAttrMacInRangeLenErr         = 23,
    OamAttrMacOutOfRangeLenErr      = 24,
    OamAttrMacFrameTooLong          = 25,
    OamAttrMacEnableStatus          = 26,
    OamAttrMacTxEnable              = 27,
    OamAttrMacMcastRxStatus         = 28,
    OamAttrMacAddr                  = 29,
    OamAttrMacCollisonFrames        = 30,

    /* RSTP related definitions */
    /* WARNING: The leaf values overlap with previously defined values and need */
    /* to be interpreted based on the context. In the case of RSTP, it is the */
    /* bridge/bridge-port context */
    OamAttrBridgeAddress            = 0x00,
    OamAttrBridgeNumPorts           = 0x02,
    OamAttrBridgePortAddresses      = 0x03,
    OamAttrRstpBridgeIdentifier     = 0x05,
    OamAttrRstpTimeSinceTopologyChange  = 0x06,
    OamAttrRstpTopologyChangeCount  = 0x07,
    OamAttrRstpTopologyChange       = 0x08,
    OamAttrRstpDesignatedRoot       = 0x09,
    OamAttrRstpRootPathCost         = 0x0A,
    OamAttrRstpRootPort             = 0x0B,
    OamAttrRstpMaxAge               = 0x0C,
    OamAttrRstpHelloTime            = 0x0D,
    OamAttrRstpForwardDelay         = 0x0E,
    OamAttrRstpBridgeMaxAge         = 0x0F,
    OamAttrRstpBridgeHelloTime      = 0x10,
    OamAttrRstpBridgeForwardDelay   = 0x11,
    OamAttrRstpHoldTime             = 0x12,
    OamAttrRstpBridgePriority       = 0x13,

    OamAttrRstpPortIdentifier       = 0x22,
    OamAttrRstpPortPriority         = 0x23,
    OamAttrRstpPathCost             = 0x24,
    OamAttrRstpPortDesignatedRoot   = 0x25,
    OamAttrRstpDesignatedCost       = 0x26,
    OamAttrRstpDesignatedBridge     = 0x27,
    OamAttrRstpDesignatedPort       = 0x28,
    OamAttrRstpPortState            = 0x33,

    /* PHY */
    OamAttrPhyId                    = 31,
    OamAttrPhyType                  = 32,
    OamAttrPhyTypeList              = 33,
    OamAttrPhySqeTestErr            = 34,
    OamAttrPhySymbolErrDuringCarrier= 35,
    OamAttrPhyMiiDetect             = 36,
    OamAttrPhyAdminState            = 37,

    /* MAU attributes */
    OamAttrMauId                    = 68,
    OamAttrMauType                  = 69,
    OamAttrMauTypeList              = 70,
    OamAttrMauMediaAvail            = 71,
    OamAttrLoseMediaCtr             = 72,
    OamAttrJabber                   = 73,
    OamAttrMauAdminState            = 74,
    OamAttrMauBandSplitType         = 75,
    OamAttrMauBandSplitFreq         = 76,
    OamAttrMauFalseCarriers         = 77,

    /* Auto-Neg attributes */
    OamAttrAutoNegId                = 78,
    OamAttrAutoNegAdminState        = 79,
    OamAttrAutoNegRemoteSig         = 80,
    OamAttrAutoNegAutoCfg           = 81,
    OamAttrAutoNegLocalTech         = 82,
    OamAttrAutoNegAdTech            = 83,
    OamAttrAutoNegRxTech            = 84,
    OamAttrAutoNegLocalSelectAble   = 85,
    OamAttrAutoNegAdSelectAble      = 86,
    OamAttrAutoNegRxSelectAble      = 87,

    /* MAC */
    OamAttrMacCapabilities          = 89,
    OamAttrMacDuplexStatus          = 90,

    /* MAU */
    OamAttrMauIdleErrorCount        = 91,

    /* DTE MAC Control */
    OamAttrMacCtrlId                = 92,
    OamAttrMacCtrlFuncsSupported    = 93,
    OamAttrMacCtrlFramesTx          = 94,
    OamAttrMacCtrlFramesRx          = 95,
    OamAttrMacCtrlUnsupportedOpRx   = 96,
    OamAttrMacCtrlPauseDelay        = 97,
    OamAttrMacCtrlPauseTx           = 98,
    OamAttrMacCtrlPauseRx           = 99,

    /* Aggregate */
    /* 101 - 132 */

    /* aggregation port attributes */
    /* 133 - 156 */

    /* agg port stats */
    /* 157 - 165   */

    /* agg port debug info */
    /* 166 - 178 */

    /* 802.3ah, new stuff, no official registration arcs yet */

    /* OAM */
    OamAttrOamId                    = 200,
    OamAttrOamAdminState            = 201,
    OamAttrOamMode                  = 202,
    OamAttrOamRemoteMacAddr         = 203,
    OamAttrOamRemoteConfig          = 204,
    OamAttrOamRemotePduConfig       = 205,
    OamAttrOamLocalFlags            = 206,
    OamAttrOamRemoteFlags           = 207,
    OamAttrOamRemoteState           = 208,
    OamAttrOamRemoteVendorOui       = 209,
    OamAttrOamRemoteVendorDevice    = 210,
    OamAttrOamRemoteVendorVersion   = 211,
    OamAttrOamPduTx                 = 212,
    OamAttrOamPduRx                 = 213,
    OamAttrOamUnsupportedOpcodes    = 214,

    OamAttrOamInfoTx                = 215,
    OamAttrOamInfoRx                = 216,
    OamAttrOamEventTx               = 217,
    OamAttrOamUniqueEventRx         = 218,
    OamAttrOamDupEventRx            = 219,
    OamAttrOamLoopTx                = 220,
    OamAttrOamLoopRx                = 221,
    OamAttrOamVarReqTx              = 222,
    OamAttrOamVarReqRx              = 223,
    OamAttrOamVarRespTx             = 224,
    OamAttrOamVarRespRx             = 225,
    OamAttrOamOrgSpecificTx         = 226,
    OamAttrOamOrgSpecificRx         = 227,

    OamAttrOamLocalErrSymPeriodWin      = 228,
    OamAttrOamLocalErrSymPeriodThresh   = 229,
    OamAttrOamLocalErrSymPeriodEvent    = 230,

    OamAttrOamLocalErrFrameSecsWin      = 231,
    OamAttrOamLocalErrFrameSecsThresh   = 232,
    OamAttrOamLocalErrFrameSecsEvent    = 233,

    OamAttrOamLocalErrFramePeriodWin    = 234,
    OamAttrOamLocalErrFramePeriodThresh = 235,
    OamAttrOamLocalErrFramePeriodEvent  = 236,

    OamAttrOamLocalErrFrSecSumWin       = 237,
    OamAttrOamLocalErrFrSecSumThresh    = 238,
    OamAttrOamLocalErrFrSecSumEvent     = 239,

    OamAttrOamRemoteErrFrameSecsWin     = 240,
    OamAttrOamRemoteErrFrameSecsThresh  = 241,
    OamAttrOamRemoteErrFrameSecsEvent   = 242,

    OamAttrOamRemoteErrFramePeriodWin   = 243,
    OamAttrOamRemoteErrFramePeriodThresh= 244,
    OamAttrOamRemoteErrFramePeriodEvent = 245,

    OamAttrOamFramesLostOamErr          = 246,

    /* OMP Muxing */
    /* no attrs yet defined */

    /* OMP Emulation */
    OamAttrOamEmulId                = 247,
    OamAttrOamEmulSpdErr            = 248,
    OamAttrOamEmulCrc8Err           = 249,
    OamAttrOamEmulBadLlidErr        = 250,

    OamAttrMpcpMACCtrlFramesTx      = 280,
    OamAttrMpcpMACCtrlFramesRx      = 281,
    OamAttrMpcpDiscoveryWindowTx    = 288,
    OamAttrMpcpDiscoveryTimeout     = 290,
    OamAttrFecCorrectedBlocks       = 292,
    OamAttrFecUncorrectableBlocks   = 293,
    OamAttrFecAbility               = 313,
    OamAttrFecMode                  = 314,
    OamAttrMpcpTxGate               = 315,
    OamAttrMpcpTxRegAck             = 316,
    OamAttrMpcpTxRegister           = 317,
    OamAttrMpcpTxRegRequest         = 318,
    OamAttrMpcpTxReport             = 319,
    OamAttrMpcpRxGate               = 320,
    OamAttrMpcpRxRegAck             = 321,
    OamAttrMpcpRxRegister           = 322,
    OamAttrMpcpRxRegRequest         = 323,
    OamAttrMpcpRxReport             = 324,
    OamAttrForce16                  = 0x7fff
} PACK OamAttrLeaf;


/* Enumerated types for attributes */

typedef enum {
    OamFalse,
    OamTrue
} PACK OamTrueFalse;

typedef enum {
    OamStateDisabled = 1,
    OamStateEnabled = 2
} PACK OamEnableStatus;

typedef enum {
    OamMacDuplexHalf = 1,
    OamMacDuplexFull,
    OamMacDuplexUnknown
} PACK OamMacDuplexStatus;

typedef enum {
    OamPhyTypeOther = 1,
    OamPhyTypeUnknown = 2,
    OamPhyTypeNone = 3,
    OamPhyType10 = 7,
    OamPhyType100T4 = 23,
    OamPhyType100X = 24,
    OamPhyType100T2 = 32,
    OamPhyType1000X = 36,
    OamPhyType1000T = 40
} PACK OamPhyType;

typedef enum {
    OamMauMediaAvailOther = 1,
    OamMauMediaAvailUnknown = 2,
    OamMauMediaAvailAvailable = 3,
    OamMauMediaAvailNotAvailable = 4,
    OamMauMediaAvailRemoteFault = 5,
    OamMauMediaAvailInvSignal = 6,
    OamMauMediaAvailRemoteJabber = 7,
    OamMauMediaAvailRemoteLinkLoss = 8,
    OamMauMediaAvailRemoteTest = 9,
    OamMauMediaAvailOffline = 10,
    OamMauMediaAvailAutoNegError = 11
} PACK OamMauMediaAvail;

typedef enum {
    OamAutoCapGlobal = 0,
    OamAutoCapOther = 1,
    OamAutoCapUnknown = 2,
    OamAutoCap10T = 14,
    OamAutoCap10TFD = 142,
    OamAutoCap100T4 = 23,
    OamAutoCap100TX = 25,
    OamAutoCap100TXFD = 252,
    OamAutoCapFdxPause = 312,
    OamAutoCapFdxAPause = 313,
    OamAutoCapFdxSPause = 314,
    OamAutoCapFdxBPause = 315,
    OamAutoCap100T2 = 32,
    OamAutoCap100T2FD = 322,
    OamAutoCap1000X = 36,
    OamAutoCap1000XFD = 362,
    OamAutoCap1000T = 40,
    OamAutoCap1000TFD = 402,
    OamAutoCapRemFault1 = 37,
    OamAutoCapRemFault2 = 372,
    OamAutoCapIsoEthernet = 8029
} PACK OamAutoNegCapability;

typedef enum {
    OamPortHalfDuplex = 0x0001,
    OamPortFullDuplex = 0x0002,
    OamPort10Speed = 0x0004,
    OamPort100Speed = 0x0008,
    OamPort1GSpeed = 0x0010,
    OamPort10GSpeed = 0x0020,
    OamPortFlowControl = 0x0040,
    OamPortAutoMDIMDIX = 0x0080,
    OamPort2GSpeed = 0x8000,
    OamPortMaxCap = 0xFFFF
} PACK OamPortCapability;

typedef enum {
    OamAutoAdminDisabled = 1,
    OamAutoAdminEnabled
} PACK OamAutoNegAdminState;

typedef enum {
    OamAutoConfigOther = 1,
    OamAutoConfigConfiguring,
    OamAutoConfigComplete,
    OamAutoConfigDisabled,
    OamAutoConfigParallelFail
} PACK OamAutoNegAutoConfig;

typedef enum {
    OamAutoSelectOther = 1,
    OamAutoSelectEthernet,  /* 802.3 */
    OamAutoSelectIsoEthernet    /* 802.9 */
} PACK OamAutoSelector;

typedef enum {
    OamAutoRemoteDetected = 1,
    OamAutoRemoteNotDetected
} PACK OamAutoRemoteSig;

typedef enum {
    OamMacCtrlFuncPause = 312
} PACK OamMacCtrlFuncs;


/* Actions */

typedef enum {
    OamActMacInit = 1,
    OamActMacAddGroupAddr = 2,
    OamActMacDelGroupAddr = 3,
    OamActMacSelfTest = 4,

    OamActPhyAdminControl = 5,

    OamActRptReset = 6,
    OamActRptInServiceTest = 7,

    OamActPortAdminCtrl = 8,

    OamActMauReset = 9,
    OamActMauAdminCtrl = 10,

    OamActAutoRenegotiate = 11,
    OamActAutoAdminCtrl = 12,
    OamActForce16 = 0x7fff
} PACK OamActionLeaf;


/*  Variable Descriptor, as found in OAM Var Request msgs */
typedef struct {
    uint8 branch;
    uint16 leaf;
} PACK OamVarDesc;

typedef struct {
    OamVarBranch Branch;
    uint16 Leaf;
    uint8 Width;
    uint8 *pValue;
} PACK tGenOamVar;


/*  Variable Container error codes, which occur in place of the length field */
typedef enum {
    OamVarErrReserved = 0x80,
    OamVarErrNoError = 0x80,
    OamVarErrTooLong = 0x81,

    /* Teknovus extended error codes */
    OamVarErrAttrToBeContinued = 0x85,
    OamVarErrActBadParameters = 0x86,
    OamVarErrActNoResources = 0x87,

    /* standard attribute error codes */
    OamVarErrAttrUndetermined = 0xa0,
    OamVarErrAttrUnsupported = 0xa1,
    OamVarErrAttrMayBeCorrupted = 0xa2,
    OamVarErrAttrHwFailure = 0xa3,
    OamVarErrAttrOverflow = 0xa4,

    /* standard object error codes */
    OamVarErrObjEnd = 0xc0,
    OamVarErrObjUndetermined = 0xc1,
    OamVarErrObjUnsupported = 0xc2,
    OamVarErrObjMayBeCorrupted = 0xc3,
    OamVarErrObjHwFailure = 0xc4,

    /* standard package error codes */
    OamVarErrPkgEnd = 0xe0,
    OamVarErrPkgUndetermined = 0xe1,
    OamVarErrPkgUnsupported = 0xe2,
    OamVarErrPkgMayBeCorrupted = 0xe3,
    OamVarErrPkgHwFailure = 0xe4,

    OamVarErrUnknow = 0xf0, /* Added for ONU SDK by Qiang Jiang */
    SdkNoResource = 0xf1

} PACK OamVarErrorCode;

/*  Variable Container, as found in OAM Var Response msgs */
typedef struct {
    uint8 branch;
    uint16 leaf;
    uint8 length;
    uint8 value[1];     /* actually length bytes long */
} PACK OamVarContainer;


/*  Var Request */
typedef struct {
    OamMsg common;
} PACK OamVarReqMsg;

/*  Var Response */
typedef struct {
    OamMsg common;
} PACK OamVarRespMsg;


/*
 * Loopback
 */

typedef enum {
    OamLoopCmdEnable = 1,
    OamLoopCmdDisable = 2
} PACK OamLoopCmd;

typedef struct {
    OamMsg common;
    uint8 cmd;
} PACK OamLoopbackMsg;


/*
 * Spanning Tree
 */

typedef enum {
    OamRstpBridgeModeDisabled = 0,
    OamRstpBridgeModePassThrough = 1,
    OamRstpBridgeModeNormal = 2
} PACK OamRstpBridgeMode;

typedef enum {
    OamRstpPortDisabled = 1,
    OamRstpPortBlocking = 2,
    OamRstpPortListening = 3,
    OamRstpPortLearning = 4,
    OamRstpPortForwarding = 5,
    OamRstpPortBroken = 6
} PACK OamRstpPortState;


/*
 *  Vendor Extension
 */

/* Vendor-extended message types (opcodes) */

/*  OUI-type vendor extension header */
typedef struct {
    OamMsg common;
    IeeeOui oui;
    /* the remainer of the vendor extension PDU is unspecified, up */
    /* to the vendor with the given OUI */
} PACK OamOuiVendorExt;

/*  Teknovus OUI extended opcodes */
typedef enum {
    OamExtOpInfo,
    OamExtOpVarRequest,
    OamExtOpVarResponse,
    OamExtOpVarSet,
    OamExtOpVarSetResponse,

    /* multicast */
    OamExtOpMcastRequest,
    OamExtOpMcastReg,
    OamExtOpMcastRegResponse,

    /* encryption */
    OamExtOpKeyExchange,

    /* file transfer */
    OamExtOpFileRdReq,
    OamExtOpFileWrReq,
    OamExtOpFileData,
    OamExtOpFileAck,

    OamExtOpI2cRdReq,
    OamExtOpI2cRdResp,
    OamExtOpI2cWrReq,
    OamExtOpI2cWrResp,

    OamExtOpUnackedAction,

    /* Loop Detect */
    OamExtOpLoopDetect,

    OamExtNumOpcodes
} PACK OamExtOpcode;

/*  Teknovus extended message common format */
typedef struct {
    uint8 opcode;
} PACK OamTkExt;

typedef enum {
    SimpleReportMode = 0,
    TeknovusReportMode = 1 << 0,
    CtcReportMode = 1 << 1,
    NttReportMode = 1 << 2,
    ManualReportMode = 1 << 3
} PACK EponReportModes;


typedef struct {
    uint8 op;           /* 0xFE -- vendor specific */
    uint8 length;       /* length of TLV; */
    IeeeOui oui;        /* set to teknovus OUI.  */
    uint8 tlvType;      /* Set to TkInfoTlvTeknovus  */
    uint8 version;      /* Set to 0 */
    EponReportModes supportedReportModes;   /* OLT will do these */
    EponReportModes perferredReportModes;   /* perferes these */
} PACK TekOltOamDiscoveryTlv;

typedef struct {
    uint8 op;           /* 0xFE -- vendor specific */
    uint8 length;       /* length of TLV; */
    IeeeOui oui;        /* set to teknovus OUI.  */
    uint8 tlvType;      /* Set to TkInfoTlvTeknovus  */
    uint8 version;
    EponReportModes currentReportMode;  /* mode ONU is currently reporting  */
} PACK TekOnuOamDiscoveryTlv;

typedef struct {
    uint8 op;           /* should be 0xFE */
    uint8 length;
    IeeeOui oui;
} PACK VendorSpecificInfoOamHeader;

/*
 * Extended Info
 */

typedef struct {
    OamTkExt ext;
    uint16 firmwareVersion; /*  TK firmware version */
    OamId oamId;        /*  repeat of std info */
    uint8 extendedId[64];   /*  as supplied by vendor */
    uint8 baseMac[6];       /*  base MAC address for this ONU */

    uint8 numLinks;     /*  max number of links supported */
    uint8 numPorts;     /*  including the EPON port */

    /* upstream queue info                              */
    uint8 numUpQueues;      /*  number of queues assignable */
    uint8 maxUpQueuesPerLink;   /*  maximum queues per link */
    uint8 upQueueIncrement; /*  Kbytes per increment */

    /* downstream queue info                    */
    uint8 numDnQueues;      /*  number of queues assignable */
    uint8 maxDnQueuesPerLink;   /*  maximum queues per link */
    uint8 dnQueueIncrement; /*  Kbytes per increment */

    /* buffer sizes                                             */
    uint16 upBuffer;        /*  Kbytes upstream buffer avail */
    uint16 dnBuffer;        /*  Kbytes downstream buffer avail */

    uint16 jedecId;     /*  Teknovus JEDEC Manufacturer ID */
    uint16 chipId;      /*  Chip ID */
    uint32 chipVersion;     /*  Chip Version */

} PACK OamTkInfo;

/*
 * Extended Var Request, Response, Set
*/

/* extended var requests,responses have the same format as standard
 * var req,resp beyond the header; they consist of a series of 
 * var branch+leaf TLV structures.  They differ in that the values
 * can also be one of these extended values, in addition to the standard
 * ones. 
 */

typedef enum {
    OamExtBranchFirst = 0x80
} PACK OamExtBranch;

typedef enum {
    OamExtObjFirst = 0x80,
    OamExtObjForce16 = 0x7fff
} PACK OamExtObjLeaf;

typedef enum {
    OamExtPkgFirst = 0x80,
    OamExtPkgId = 0x80,     /* extended ID information */
    OamExtPkgSizeBins = 0x81,
    OamExtPkgEponDelayStats = 0x82,
    OamExtPkgForce16 = 0x7fff
} PACK OamExtPkgLeaf;


/*
 Extended Name Bindings
*/


typedef enum {
    OamExtNameFirst = 0x80,
    OamExtNameLink = 0x80,
    OamExtNameQueue = 0x81,
    OamExtNameBridge = 0x82,
    OamExtNameBridgePort = 0x83, 
    OamExtNameForce16 = 0x7fff
} PACK OamExtNameLeaf;

/*  value used to name a port */
typedef struct {
    uint16 port;
} PACK OamNamePort;

/*  value used to name a link: port.link */
typedef struct {
    uint16 port;
    uint16 link;
} PACK OamNameLink;

/*  value used to name a queue: port.link.queue */
typedef struct {
    uint16 port;
    uint16 link;
    uint16 queue;
} PACK OamNameQueue;

/*  value used to name a bridge */
typedef struct {
    uint16 bridge;
} PACK OamNameBridge;

/*  value used to name a bridge port */
typedef struct {
    uint16 port;
} PACK OamNameBridgePort;


/*  value used to name a queue: port_link.queue */
typedef struct {
    uint8   port_link;
    uint8   queue;
} PACK OamNewNameQueue;

typedef union {
    uint8           portId;
    uint8           linkId;
    OamNameQueue    queueId;
} PACK OamObjIndex;

typedef union {
    OamNewNameQueue ndest;
    uint16 vid_cos;
} PACK OamNewParam;

typedef uint8 OamRuleAttr;
#define OAMRULE_NONVOLATILE 0
#define OAMRULE_VOLATILE    1

typedef struct {
    /* Currently only Volatile Rule bit 0 (not saved in NVS) */
    OamRuleAttr ruleflag;
    OamNewParam nparam;
    uint8 onureserve;       /* Reserved for onu internal use. */
} PACK OamNameParam;

typedef union {
    OamNameQueue dest;
    OamNameParam new;
} PACK OamRuleParam;

/*
 *Extended Attributes
 */
typedef enum {
    OamExtAttrFirst = 0x80,

    /* additional info   */
    OamExtAttrFirmwareVer = 0x80,
    OamExtAttrExtendedId = 0x81,

    /* bridging attributes */
    OamExtAttrDynLearnTblSize = 0x82,
    OamExtAttrDynLearnAgeLimit = 0x83,

    /* additional link attributes */
    OamExtAttrRxUnicastFrames = 0x84,
    OamExtAttrTxUnicastFrames = 0x85,
    OamExtAttrRxFrameTooShort = 0x86,

    /* Frame size bins, as per RFC 1757 */
    OamExtAttrRxFrame64 = 0x87,
    OamExtAttrRxFrame65_127 = 0x88,
    OamExtAttrRxFrame128_255 = 0x89,
    OamExtAttrRxFrame256_511 = 0x8A,
    OamExtAttrRxFrame512_1023 = 0x8B,
    OamExtAttrRxFrame1024_1518 = 0x8C,
    OamExtAttrRxFrame1519Plus = 0x8D,

    OamExtAttrTxFrame64 = 0x8E,
    OamExtAttrTxFrame65_127 = 0x8F,
    OamExtAttrTxFrame128_255 = 0x90,
    OamExtAttrTxFrame256_511 = 0x91,
    OamExtAttrTxFrame512_1023 = 0x92,
    OamExtAttrTxFrame1024_1518 = 0x93,
    OamExtAttrTxFrame1519Plus = 0x94,

    /* addtional per-LLID statistics */
    OamExtAttrTxDelayThreshold = 0x95,
    OamExtAttrTxDelay = 0x96,
    OamExtAttrTxFramesDropped = 0x97,
    OamExtAttrTxBytesDropped = 0x98,    /* dropped in queue, that is  */
    OamExtAttrTxBytesDelayed = 0x99,
    OamExtAttrTxBytesUnused = 0x9A, /* granted but not sent */

    OamExtAttrRxDelayThreshold = 0x9B,
    OamExtAttrRxDelay = 0x9C,
    OamExtAttrRxFramesDropped = 0x9D,
    OamExtAttrRxBytesDropped = 0x9E,    /* dropped in queue, that is */
    OamExtAttrRxBytesDelayed = 0x9F,

    /* Statistics thresholds (see also 0xD9) */
    OamExtAttrPortStatThreshold = 0xA0,
    OamExtAttrLinkStatThreshold = 0xA1,

    /* Encryption */
    OamExtAttrEncryptKeyExpiryTime = 0xA2,

    /* Additional port attributes */
    OamExtAttrLenErrorDiscard = 0xA3,

    OamExtAttrDynMacTbl = 0xA4,
    OamExtAttrStaticMacTbl = 0xA5,

    OamExtAttrUpFilterTbl = 0xA6,
    OamExtAttrDnFilterTbl = 0xA7,

    /* Threshold levels for report messsages */
    OamExtAttrReportThresholds = 0xA8,

    /* broadcast frames per second allowed through a port */
    OamExtAttrBcastRateLimit = 0xA9,

    /* Path configuration for this LLID */
    OamExtAttrLinkConfig = 0xAA,

    /* Path and queue configuration for the entire ONU */
    OamExtAttrOnuConfig = 0xAB,

    /* VLAN tag Ethertype (in addition to 8100) */
    OamExtAttrVlanEthertype = 0xAC,

    /* Output shaping (3711 only) */
    OamExtAttrOutputShaping = 0xAD,

    /* UNI Port Capability (3711 only) */
    OamExtAttrPortCapability = 0xAE,

    /* Control VLAN Tag (3711 only) use for delete/copy */
    OamExtAttrCtlVlanID = 0xAF,

    /* MDI extended setting */
    OamExtAttrMdiCrossover = 0xB0,

    OamNewExtAttrDnFilterTbl = 0xB1,

    OamNewExtAttrUpFilterTbl = 0xB2,

    OamExtAttrJedecId = 0xB3,
    OamExtAttrChipId = 0xB4,
    OamExtAttrChipVersion = 0xB5,

    /* OLT MPCP clock value to generate 1 second phase locked pulse at ONU */
    OamExtAttrMpcpClock = 0xB6,
    OamExtAttrMpcpClockCompensate = 0xB7,

    OamExtAttrEgressShaping = 0xB8,
    OamExtAttrIngressPolicing = 0xB9,
    OamExtAttrCosTranslation = 0xBA,
    OamExtAttrPriEnqueuing = 0xBB,

    OamExtAttrPortVlanPolicy = 0xBC,
    OamExtAttrPortVlanMembership = 0xBD,

    /* ARP replication destination bitmap */
    OamExtAttrArpReplicateDest = 0xBE,

    OamExtAttrLacpDest = 0xBF,

    /* RSTP Attributes */
    OamExtAttrRstpPortMode = 0xC0,
    OamExtAttrRstpPortOperEdge = 0xC1,
    OamExtAttrRstpBridgeOptions = 0xC2,
    OamExtAttrRstpBridgeMode = 0xC3,

    OamExtAttrOnuIgmpVlan = 0xC4,

    /* ASCII coded Time of Day string */
    OamExtAttrTimeOfDay = 0xC5,

    /* Learning Mode Configuration */
    OamExtAttrDynLearningMode = 0xD0,
    OamExtAttrMinMacLimit = 0xD1,
    OamExtAttrOnuAggregateLimit = 0xD2,

    OamExtAttrNvsScratchpad = 0xD3,

    /* More learning mode */
    OamExtAttrFloodUnknown = 0xD4,
    OamExtAttrLocalSwitching = 0xD5,
    OamExtAttrDownBurstToll = 0xD6,

    /* FEC attribute */
    OamExtAttrFECMode = 0xD7,
    OamExtAttrSequenceNumber = 0xD8,

    /* Statistics thresholds (see also 0xA0) */
    OamExtAttrQueueStatThreshold = 0xD9,
    /* Even mode learning mode */
    OamExtAttrLearnModeRuleUpdate = 0xDA,

    /* Power Monitoring statistics */
    OamExtAttrPowerMonTemperature = 0xDB,
    OamExtAttrPowerMonVcc = 0xDC,
    OamExtAttrPowerMonTxBias = 0xDD,
    OamExtAttrPowerMonTxPower = 0xDE,
    OamExtAttrPowerMonRxPower = 0xDF,

    OamExtAttrLinkState = 0xE3,

    /* EPON port admin state */
    OamExtAttrONUStatus = 0xE5,

    OamExtAttrVlanDest = 0xF0,

    OamExtAttrOnuTemps = 0xF1,

    /*IPN Specific OAM*/
    OamExtAttrXcvrSignalDetect = 0xF2,

    OamExtAttrCrossbarConfig = 0xF3,

    OamExtAttrXcvrExtBurstActivity = 0xF4,

    OamExtAttrControlPort = 0xF5,

    OamExtAttrOnuFailsafe = 0xF6,


    /**********************************************************
     * Extended by Teknovus China ONU team
     **********************************************************/
    OamExtAttrAddiFirst = 0x1080,

    OamExtAttrCtcAuthLoid = 0x1081,
    OamExtAttrCtcAuthPassword = 0x1082,

    OamExtAttrLinkInfo = 0X10EA,
    OamExtAttrSwitchPortMir = 0x10EB,
    OamExtAttrOnuSn = 0x10EC,
    OamExtAttrEponPortAdmin = 0x10ED,
    OamExtAttrEponRegState = 0x10EE,

    OamExtAttrMacLearning = 0x10EF,
    OamExtAttrMacFlush = 0x10F0,
    OamExtAttrMtuSize = 0x10F1,
    OamExtAttrMacLearningSwitch = 0x10F2,
    OamExtAttrBatteryBackupCap = 0x10F3,
    OamExtAttrCtcOamVerion = 0x10F4,
    OamExtAttrLaserAlwaysOn = 0x10FF,

    /**********************************************************
     * for CTC LOID auth result from external CPU
     **********************************************************/    
    OamExtAttrCTCLoidAuthResult = 0x12EC,
    
    OamExtAttrForce16 = 0x7fff
} PACK OamExtAttrLeaf;

typedef enum {
    OamExtActFirst = 0x80,

    OamExtActClearDynLearnTbl = 0x80,

    /* static bridging entries */
    OamExtActAddStaticEntry = 0x81,
    OamExtActDelStaticEntry = 0x82,

    /* user-defined packet filtering rules */
    OamExtActClearUpFilterTbl = 0x83,
    OamExtActClearDnFilterTbl = 0x84,

    /* rules */
    OamExtActAddRule = 0x85,
    OamExtActDelRule = 0x86,

    /* reset the ONU */
    OamExtActResetOnu = 0x87,

    /* clear all stats to 0 */
    OamExtActClearStats = 0x88,

    /* read/write GPIO register */
    OamExtActGetGpioConfig = 0x89,
    OamExtActSetGpioConfig = 0x8A,

    OamExtActGetGpioValue = 0x8B,
    OamExtActSetGpioValue = 0x8C,

    /* MDIO register control */
    OamExtActGetMdio = 0x8D,
    OamExtActSetMdio = 0x8E,

    /* Extended loopback */
    OamExtActLoopbackEnable = 0x8F,
    OamExtActLoopbackDisable = 0x90,

    /* Classification rules */
    OamExtActClrUpClass = 0x91, /* clears current in-context link */
    OamExtActClrDnClass = 0x92, /* clears current in-context port */

    /* Queue configuration */
    OamExtActSetQueueConfig = 0x93,
    OamExtActGetQueueConfig = 0x94,

    /* Erase Non-volatile Store */
    OamExtActEraseNvs = 0x95,

    /* IGMP configuration */
    OamExtActSetIgmpConfig = 0x96,
    OamExtActGetIgmpConfig = 0x97,

    /* IGMP Group Settings and information */
    OamExtActGetIgmpGroupInfo = 0x98,
    OamExtActDelIgmpGroup = 0x99,
    OamExtActAddIgmpGroup = 0x9A,

    /* Queue output rate control -- 3711 only */
    OamExtActSetQueueRateControl = 0x9B,
    OamExtActGetQueueRateControl = 0x9C,

    /* Port output rate control -- 3711 only */
    OamExtActSetPortRateControl = 0x9D,
    OamExtActGetPortRateControl = 0x9E,

    /* Phy addressable MDIO register control */
    OamExtActSetPhyAddrMdio = 0x9F,
    OamExtActGetPhyAddrMdio = 0xA0,

    OamExtActNewAddRule = 0xA1,

    OamExtActOnuEnableUserTraffic = 0xA2,
    OamExtActOnuDisableUserTraffic = 0xA3,

    OamExtActSetDnBcastQueue = 0xA4,
    OamExtActGetDnBcastQueue = 0xA5,

    OamExtActSetOamRate = 0xA6,
    OamExtActGetOamRate = 0xA7,

    OamExtActSetLUEField = 0xA8,
    OamExtActGetLUEField = 0xA9,

    OamExtActGetLoadInfo = 0xAA,
    OamExtActNewDelRule = 0xAB,

    /* Classification rules */
    OamExtActClrUpUserRules = 0xAC,
    OamExtActClrDnUserRules = 0xAD,

    OamExtActLaserPowerOffTime = 0xB1,

    /*
     * Extended by Teknovus China ONU team
     */
    OamExtActAddiFirst = 0x1080,
    OamExtActClrMacFilterTbl = 0x1081,
    OamExtActPonLosInfo = 0x10FF,
    OamExtActBaseMac = 0x7f04,
    OamExtActForce16 = 0x7fff
} PACK OamExtActionLeaf;


/*  value of the "stat threshold" attribute */
typedef struct {
    uint16 stat;        /* < stat affected by this threshold */
    uint32 rising;      /* < rising threshold that sets alarm */
    uint32 falling;     /* < falling threshold that clears alarm */
} PACK OamExtStatThreshold;

/*  value of OutputShaping attribute */
typedef struct {
    uint8 outputBurstSize;  /* < units of 256 bytes; 0 to disable shaping */
    uint16 rate;        /* < units of 1/(2^15) Gbps (~30,518 bps) */
    uint16 queueMap;        /* < 1s indicate queues involved in shaping */
} PACK OamExtOutputShaping;

typedef struct {
    uint16 burstSize;       /* < units of 256 bytes; 0 to disable shaping */
    uint32 rate;        /* < units of 1 Kbps  */
    uint8 numQs;
    OamNameQueue queue[1];
} PACK OamExtPortOutputShaping;

typedef struct {
    OamNameQueue queue;
    uint16 burstSize;       /* < units of 256 bytes; 0 to disable shaping */
    uint32 rate;        /* < units of 1 Kbps  */
} PACK OamExtPerQueueShaping;

typedef struct {
    uint8 numQs;
    OamExtPerQueueShaping queueRate[1];
} PACK OamExtQueueOutputShaping;

/*  value of the "OamExtAttrVlanEthertype"  attribute */
/*  For both TK3701 and TK3711 but dnEtype/upEtype only affects TK3711 */
typedef struct {
    uint16 ethertype;       /* < User-defined Ethertype for VLAN */
    uint8 upEtype;      /* < 0=0x8100, 1=user Ethertype when adding VLAN in upstream */
    uint8 dnEtype;      /* < 0=0x8100, 1=user Ethertype when adding VLAN in downstream */
} PACK OamExtVlanEthertype;


/* Ext FEC mode  */
typedef struct {
    uint8 rxFEC;        /* < non-zero if ONU rx/downstream FEC is enabled */
    uint8 txFEC;        /* < non-zero if ONU tx/upstream FEC is enabled */
} PACK OamExtFECMode;


/*  value of the attribute OamExtAttrMpcpClock */
typedef struct {
    uint32 val;
} PACK OamExtMpcpClock;


/*  value of the attribute OamExtAttrTimeOfDay */
typedef struct {
    char tod[2];
} PACK OamExtTimeOfDay;


typedef enum {
    EponAdminEnable = 0,
    EponAdminDisable = 1
} PACK OamEponAdmin;


/*  Defines to control UNI Port VLAN configuration */
typedef enum {
    VlanPolicyDisabled = 0,
    VlanPolicyFallback = 1,
    VlanPolicyCheck = 2,
    VlanPolicySecure = 3,

    VlanPolicyMaxPolicy = 4
} PACK OamUniVlanPolicy;


/*  Defines to control per VLAN ingress/egress tagging behavior */
typedef enum {
    VlanUnmodified = 0,
    VlanAddDefTagIfNone = 1,
    VlanRemoveTag = 2,
    VlanOverwriteTag = 3,

    VlanTagBehaveMaxBehave = 4,
    VlanStackDefaultTag = 4 /* FOR FUTURE USE - DO NOT USE AT THIS TIME */
} PACK OamVlanTagBehave;

typedef struct {
    uint16 options;     /* reserved - NOT IN USE AT THE MOMENT */

    OamUniVlanPolicy ingressPolicy;
    OamUniVlanPolicy egressPolicy;
    uint16 defaultVlan;
} PACK OamVlanPolicyCfg;

typedef struct {
    uint16 ingressVlan;
    uint16 egressVlan;
    OamVlanTagBehave ingressTagging;
    OamVlanTagBehave egressTagging;

    uint8 flags;        /* reserved - NOT IN USE AT THE MOMENT */
} PACK OamVlanMemberCfg;


/* Shaping/Policing related stuff */
typedef enum {
    OamTrafficBitBroadcast = 0x1,
    OamTrafficBitMulticast = 0x2,
    OamTrafficBitReservedGroup = 0x4,
    OamTrafficBitUnicast = 0x8,
    OamTrafficBitFloodUnicast = 0x10
} PACK OamTrafficBits;

typedef enum {
    OamSchedWeigtedFair,
    OamSchedStricPri
} PACK OamSchedAlgorithm;

typedef enum {
    OamRateKbps = 0,        /* < Kilo bits per second */
    OamRateFps = 1      /* < frames per second */
} PACK OamRateUnits;

typedef struct {
    uint8 queueBits;        /* < Queues to which shaping applies */
    uint32 maxRate;     /* < Rate in terms of OamRateUnits */
    uint16 burstSize;       /* < KBytes */
    uint8 schedAlgo;        /* < Scheduling algorithm */
    uint8 numWeights;       /* < Number of priority levels */
    uint8 weight[1];        /* < Weights per priority (high first) */
} PACK OamUniShaperData;

typedef struct {
    uint8 trafficBits;
    OamRateUnits unit;
    uint8 numShapers;
    OamUniShaperData shaper[1];
} PACK OamEgressShaping;

typedef struct {
    uint8 trafficBits;      /* < what kind of traffic to police */
    uint32 maxRate;     /* < Rate in terms of OamRateUnits */
} PACK OamOnuPolicerData;

typedef struct {
    OamRateUnits unit;
    uint8 numRateLevels;
    OamOnuPolicerData policer[1];
} PACK OamIngressPolicing;


typedef enum {
    OamPriCos = 0,
    OamPriTos = 1,
    OamPriDiffServTc = 2
} PACK OamPriType;

typedef struct {
    uint8 lo;           /* < lower value of range */
    uint8 hi;           /* < higher value of range */
} PACK OamPriRange;

typedef struct {
    uint8 inputType;
    OamPriRange inPri[8];   /* < input priority range for each COS output  */
} PACK OamPriCosTable;

typedef struct {
    uint8 inputPriOptions;  /* < reserved */
    uint8 numTables;        /* < number of priority mapping tables */
    OamPriCosTable mapTable;
} PACK OamCosTranslation;


typedef struct {
    uint8 inPri;
    uint8 outQueue;
} PACK OamPriToQueue;

typedef struct {
    uint8 inPri;        /* < input priority type */
    uint8 numMaps;      /* < Number of values to map */
    OamPriToQueue map[1];
} PACK OamPriEnqueueTable;

typedef struct {
    uint8 portDfltPri;
    uint8 inpPriOptions;
    uint8 numPriTables;
    OamPriEnqueueTable enqTable[1];
} PACK OamPriTrafficEnqueuing;


/*
* Extended Actions
*/

/* clear MAC table - no payload required */

/* Get MAC table */
typedef struct {
    uint8 seqNum;       /* sequence number of this frame */
    uint16 numEntries;      /* number of MAC addresses (6 bytes) to follow */
} PACK OamMacTable;

/* Add/Delete filter */

typedef enum {
    OamRuleDirDownstream = 0,
    OamRuleDirUpstream = 1
} PACK OamRuleDirection;

typedef enum {
    OamFieldDa,
    OamFieldSa,
    OamFieldLlidIndex,
    OamFieldEthertype,
    OamFieldVlan,

    OamFieldPriority,
    OamFieldIpProtocol,
    OamFieldUserDefined
} PACK OamRuleFields;

/*  possible actions for filters. */
typedef enum {
    OamRuleActDiscard,      /* Old format */
    OamNewRuleActNoOp = 0,
    OamRuleActForward,      /* Old format */
    OamNewRuleActReserve1 = 1,
    OamNewRuleActForward,
    OamNewRuleActSetVlanAdd,
    OamNewRuleActSetVlanDel,
    OamNewRuleActSetVlanAddVid,
    OamNewRuleActSetVlanCos,
    OamNewRuleActSetVlanAddDel,
    OamNewRuleActSetVlanAddDelVid,
    OamNewRuleActClrVlanAdd,
    OamNewRuleActClrVlanDel,
    OamNewRuleActClrVlanAddDel,
    OamNewRuleActCpVlanCos,
    OamNewRuleActCpVlanVid,
    OamNewRuleActDiscard,
    OamNewRuleActReserve2,
    OamNewRuleActNoOpClrDisc,
    OamNewRuleActReserve3,
    OamNewRuleActForwardClrDisc,
    OamNewRuleActSetVlanAddClrDisc,
    OamNewRuleActSetVlanDelClrDisc,
    OamNewRuleActSetVlanAddVidClrDisc,
    OamNewRuleActSetVlanCosClrDisc,
    OamNewRuleActSetVlanAddDelClrDisc,
    OamNewRuleActSetVlanAddDelVidClrDisc,
    OamNewRuleActClrVlanAddClrDisc,
    OamNewRuleActClrVlanDelClrDisc,
    OamNewRuleActClrVlanAddDelClrDisc,
    OamNewRuleActCpVlanCosClrDisc,
    OamNewRuleActCpVlanVidClrDisc,
    OamNewRuleActDiscardClrDisc,
    OamNewRuleActReserve4,
    OamNumberRuleActions
} PACK OamRuleAction;

#define OAMRULE_CLRDISCARD  0x10

/*  possible operators for filter rules */
typedef enum {
    OamRuleOpNeverMatch,
    OamRuleOpEq,
    OamRuleOpNotEq,
    OamRuleOpLessEq,
    OamRuleOpGreaterEq,
    OamRuleOpExist,
    OamRuleOpNotExist,
    OamRuleOpAlwaysMatch,

    OamRuleOpNumOps
} PACK OamRuleOp;

/*  describes a single filter rule condition */
typedef struct {
    uint8 field;
    uint8 value[8];
    uint8 operator;
} PACK OamNewRuleCondition;

/* multiple conditions in a single action are considered to be logically
 * ANDed together.  ORs can be accomplished by adding multiple filter rules.
 * specification for a single rule */
typedef struct {
    uint8 dir;
    OamRuleParam param;
    uint8 pri;
    uint8 action;
    uint8 numConds;
    OamNewRuleCondition cond[1];
} PACK OamNewRuleSpec;

/*  describes a single filter rule condition */
typedef struct {
    uint8 field;
    uint8 value[6];
    uint8 operator;
} PACK OamRuleCondition;

/* multiple conditions in a single action are considered to be logically */
/* ANDed together.  ORs can be accomplished by adding multiple filter rules. */
/*  specification for a single rule */
typedef struct {
    uint8 dir;
    OamNameQueue dest;
    uint8 pri;
    uint8 action;
    uint8 numConds;
    OamRuleCondition cond[1];
} PACK OamRuleSpec;

/*  Layers available to lookup engine */
typedef enum {
    OamFieldLayerNull,
    OamFieldLayerL2,        /* < Preamble and Ethernet DA, SA */
    OamFieldLayerL2TypeLen, /* < Ethernet type/length field */
    OamFieldLayerMpls,      /* < MPLS tags */
    OamFieldLayerIpv4,      /* < IP version 4 */
    OamFieldLayerL4Ip,      /* < Layer 4 on IP, e.g. TCP / UDP */
    OamFieldLayerIpv6,      /* < IP version 6 */

    OamFieldNumLayers
} PACK OamFieldLayer;

/*  define a lookup engine field pointer */
typedef struct {
    uint8 layer;        /* < layer containing field */
    uint8 dwordOffset;      /* < 32-bit words from start of layer */
    uint8 bitOffset;        /* < first bit within dword */
    uint8 bitWidth;     /* < width of field in bits */
    uint8 inUse;        /* < true if rules use this field */
} PACK OamFieldDef;


/* GPIO configuration structure */
typedef struct {
    uint32 autoReport;      /* < 1 if autonomous message generated on changes */
} PACK OamGpioConfig;

/* GPIO control structure */
/* Write specifies mask and value.  Write response specified actual mask  */
/* used and values after write.  Read request specifies mask; read response  */
/* specifies mask and value. */
typedef struct {
    uint32 mask;        /* < 1s indicate bits to affect */
    uint32 value;       /* < valid where mask is 1 */
} PACK OamGpioValue;


/* MDIO control */
/* the MDIO device is specified by the port of the context in effect */
/* when this action is processed */
typedef struct {
    uint16 reg;         /* < register (0..31) */
    uint16 mask;        /* < 1 indicates bit in value to affect */
    uint16 value;       /* < value to read; value read in read response */
} PACK OamMdioValue;

/*  PHY addressable MDIO control  */
typedef struct {
    uint8 phy;          /* < PHY address */
    OamMdioValue mdio;
} PACK OamPhyAddrMdioValue;

/* Extended loopback */
/* TK ONUs support UNI PHY, UNI MAC, and 802.3ah EPON link loopbacks */
typedef enum {
    OamLoopLocUniPhy,
    OamLoopLocUniMac,
    OamLoopLoc8023ah,
    OamLoopLocNowhere,      /* used in ONU to shutoff traffic without loopback */
    OamLoopLocNone,
    OamLoopLocNumLocs
} PACK OamLoopbackLoc;

/* Loopback action parameters */
typedef struct {
    uint8 loc;
} PACK OamExtLoopback;


/* IGMP Snooping per-port configuration */
typedef struct {
    uint8 numGroups;        /* < # IGMP groups (0 = Snooping disabled) */
    uint8 dnQueue;      /* < Queue for downstream classification */
} PACK OamIgmpPortConfig;

/* IGMP Snooping overall configuration */
typedef struct {
    uint8 robustnessCount;  /* < # Leaves before a group is dropped */
    uint8 lmqCount;     /* < Last member query count */
    uint8 numPorts;     /* < Number of UNI ports to configure */
    OamIgmpPortConfig port[1];  /* < Port specific parameters */
    /* IgmpGrpFwdQualifier     grpFwdQual;  < Group Forwarding Qualifier */
    /* IgmpSnoopOption         snoopOpt;  < IGMP Snooping Options */
} PACK OamIgmpConfig;


/* Structure used to convey information about an IP MC group */
/* and also the ports involved */
typedef struct {
    uint32 ipAddr;      /* Group Address */
    uint8 portMap;      /* this is a bitmap indicating which of the */
    /* 8 ports are involved in the IGMP group */
} PACK OamIgmpGroupInfo;


/* Structure used to convey information about multiple IP MC groups
   and also the ports involved */
typedef struct {
    uint8 numGroups;        /* < Number of IGMP groups */
    OamIgmpGroupInfo group[1];  /* < Group specific information */
} PACK OamIgmpGroupConfig;

/* Structure used to set the logical queue number for multicast/broadcast
   Llids */
typedef struct {
    uint8 numPorts;     /* < Number of UNI Ports */
    uint8 queue[1];     /* < Logical Queueu number per port */
} PACK OamDnBcastQueue;

/* Structure used to set the upstream min (not implemented) and max OAM rate
   in units of 100ms */
typedef struct {
    uint8 maxRate;
    uint8 minRate;      /* < Not implemented */
} PACK OamUpOamRate;


typedef struct {
    uint8 refCnt;
    uint8 layerSelect;
    uint8 dwordOffset;
    uint8 lsb;
    uint8 bitWidth;
} PACK OamFieldDescriptor;


typedef struct {
    uint8 numField;     /* < Number of Field Select */
    OamFieldDescriptor field[1];    /* < Field Entries */
} PACK OamLUEField;


typedef struct {
    uint16 bootVer;     /* < Boot version */
    uint32 bootCrc;     /* < Boot crc 32 */
    uint16 persVer;     /* < Personality version */
    uint32 persCrc;     /* < Personality crc 32 */
    uint16 app0Ver;     /* < App0 version */
    uint32 app0Crc;     /* < App0 crc 32 */
    uint16 app1Ver;     /* < App1 version */
    uint32 app1Crc;     /* < App1 crc 32 */
    uint16 diagVer;     /* < Diagnostic version */
    uint32 diagCrc;     /* < Diagnostic crc 32 */
} PACK OamLoadInfo;


/*
 * ONU Configuration 
 *
 * The ONU configuration message body is (all TKU8):
 *     num up links
 *          num up queues+
 *              queue size+
 *     num dn ports
 *          num dn queues+
 *              queue size+
 *     num dn multicast queues
 *          queue size+
 *
*/


/*
 * Extended TLVs for standard Info
*/


/* Vendor-extended TLVs in standard Info message */
typedef enum {
    OamAlmNone = 0x00,

    OamAlmCodeLinkFault = 0x10,
    OamAlmCodeLos = 0x11,
    OamAlmCodeTxFail = 0x12,    /* OLT only */
    OamAlmCodeTransmitDegrade = 0x13,   /* OLT only */
    OamAlmCodeQueueOv = 0x14,
    OamAlmCodeLearnTblOv = 0x15,
    OamAlmCodeFlowCtrlTimeout = 0x16,
    OamAlmCodeReportFail = 0x17,    /* OLT only */
    OamAlmCodeGateTimeout = 0x18,   /* ONU only */
    OamAlmCodeOamTimeout = 0x19,
    OamAlmCodeKeyExchange = 0x1A,
    OamAlmCodeAutoNegFailure = 0x1B,    /* OLT only */
    OamAlmCodeGpioLinkFault = 0x1C,

    OamAlmCodeLoopback = 0x20,
    OamAlmCodePortDisabled = 0x21,

    OamAlmCodeDyingGasp = 0x40,
    OamAlmCodePower = 0x41,
    OamAlmCodeGpioDying = 0x42,

    OamAlmCodeCriticalEvent = 0x60,
    OamAlmCodeReserved61 = 0x61,
    OamAlmCodeGpioCritical = 0x62,

    OamAlmCodeSystem = 0x80,
    OamAlmCodeTemperature = 0x81,
    OamAlmCodeGpio = 0x82,
    OamAlmCodeAuthUnavail = 0x83,
    OamAlmCodeAuthenticated = 0x84,
    OamAlmCodeMpcpNack = 0x85,
    OamAlmCodeStatAlarm = 0x86,
    OamAlmCodeFlashBusy = 0x87,

/*    OamAlmCodeEponPortConn      = 0x88,*/
   
    OamAlmCodeReset = 0x88,
    OamAlmCodeOamDisc = 0x89,
    OamAlmCodePonConnect = 0x8A,
    OamAlmCodeEponPortConn = 0x8A,
    OamAlmCodeCtcAlarm = 0x90,
    OamAlmCodeCTCDiscComplete = 0x91,
    
    /* Alarm code defined by Teknovus China Team
        * Starting from 0xC0
        */
    OamAlmCodeOnuReady          = 0xB0,
    OamAlmCodeOnuPonDisable     = 0xB1,
    OamAlmCodeCtcDiscover       = 0xB2,
    OamAlmCodeCount
} PACK OamTkAlarmCode;

typedef enum {
    /*********************************************************
     *ONU Object Class Attributes, AlarmId 0x0001-0x00FF     *
     *********************************************************/
    OamCtcAttrEquipmentAlarm = 0x0001,
    OamCtcAttrPowerAlarm = 0x0002,
    OamCtcAttrBatteryMissing = 0x0003,
    OamCtcAttrBatteryFailure = 0x0004,
    OamCtcAttrBatteryVoltLow = 0x0005,
    OamCtcAttrPhysicalInstusionAlarm = 0x0006,
    OamCtcAttrOnuSelfTestFailure = 0x0007,
    OamCtcAttrOnuTempHighAlarm = 0x0009,
    OamCtcAttrOnuTempLowAlarm = 0x000A,
    OamCtcAttrIadConnectionFailure = 0x000B,
    OamCtcAttrPonIfSwitch = 0x000C,

    /********************************************************
     *PON IF Object Class Attributes, AlarmId 0x0101-0x01FF * 
     ********************************************************/
    OamCtcAttrPowerMonRxPowerAlmHigh = 0x0101,
    OamCtcAttrPowerMonRxPowerAlmLow = 0x0102,
    /*======== TxPower ==========*/ 
    OamCtcAttrPowerMonTxPowerAlmHigh = 0x0103,
    OamCtcAttrPowerMonTxPowerAlmLow = 0x0104,
    /*========= TxBias ==========*/
    OamCtcAttrPowerMonTxBiasAlmHigh = 0x0105,
    OamCtcAttrPowerMonTxBiasAlmLow = 0x0106,
    /*=========== Vcc ==========*/
    OamCtcAttrPowerMonVccAlmHigh = 0x0107,
    OamCtcAttrPowerMonVccAlmLow = 0x0108,
    /*======= Temperature ========*/
    OamCtcAttrPowerMonTempAlmHigh = 0x0109,
    OamCtcAttrPowerMonTempAlmLow = 0x010A,
    /********** Warn Section************/
    /*======== RxPower ==========*/         
    OamCtcAttrPowerMonRxPowerWarnHigh = 0x010B,
    OamCtcAttrPowerMonRxPowerWarnLow = 0x010C,
    /*======== TxPower ==========*/ 
    OamCtcAttrPowerMonTxPowerWarnHigh = 0x010D,
    OamCtcAttrPowerMonTxPowerWarnLow = 0x010E,
    /*========= TxBias ==========*/ 
    OamCtcAttrPowerMonTxBiasWarnHigh = 0x010F,
    OamCtcAttrPowerMonTxBiasWarnLow = 0x0110,
    /*========== Vcc ===========*/          
    OamCtcAttrPowerMonVccWarnHigh = 0x0111,
    OamCtcAttrPowerMonVccWarnLow = 0x0112,
    /*======= Temperature ========*/        
    OamCtcAttrPowerMonTempWarnHigh = 0x0113,
    OamCtcAttrPowerMonTempWarnLow = 0x0114,

    /*******************************************************************
       * Card Object Class Attributes, AlarmId 0x0201-0x02FF
       *******************************************************************/
    OamCtcAttrCardAlarm = 0x0201,
    OamCtcAttrSelfTestFailure = 0x0202,

    /*
     * Port Object Class Attributes, AlarmId 0x0301-0x05FF
     */
    /* Eth Section***********/
    OamCtcAttrEthPortAutoNegFailure = 0x0301,
    OamCtcAttrEthPortLos = 0x0302,
    OamCtcAttrEthPortFailure = 0x0303,
    OamCtcAttrEthPortLoopback = 0x0304,
    OamCtcAttrEthPortCongestion = 0x0305,
    /********** Pots Section***********/
    OamCtcAttrPotsPortFailure = 0x0401,
    /*********** E1 Section************/
    OamCtcAttrE1PortFailure = 0x0501,
    OamCtcAttrE1TimingUnlock = 0x0502,
    OamCtcAttrE1Los = 0x0503,
    OamCtcAttrAlarmCount = 0x7fff
} PACK OamCtcAlarmId;

typedef enum {
    OamAlarmClear,
    OamAlarmRaised
} PACK OamTkAlarmState;

typedef enum {
    OamAlarmContextOnu,
    OamAlarmContextPort,
    OamAlarmContextLink,
    OamAlarmContextQueue
} PACK OamTkAlarmContext;

/*  Teknovus extended alarm report */
typedef struct {
    OamEventExt ext;
    uint8 alm;          /* < which alarm */
    uint8 state;        /* < raised or cleared */
    OamNameQueue which;     /* < affected entity */
    uint8 context;      /* < type of affected entity */
} PACK OamEventTkAlarm;

/*  Teknovus extended statistics alarm report */
typedef struct {
    OamEventExt ext;
    uint8 alm;
    uint8 state;        /* < raised or cleared */
    OamNameQueue which;     /* < affected entity */
    OamTkAlarmContext context;  /* < type of affected entity */
    uint16 stat;        /* < which stat crossed threshold */
} PACK OamEventTkStatAlarm;

/*  Teknovus extended GPIO alarm report */
typedef struct {
    OamEventExt ext;
    uint8 alm;
    uint8 state;        /* < raised or cleared */
    OamNameQueue which;     /* < affected entity */
    OamTkAlarmContext context;  /* < type of affected entity */
    uint32 changes;     /* < new alarms/clears that caused msg */
    uint32 currentState;     /* < current state of GPIO */
} PACK OamEventTkGpioAlarm;


/*
* Key Exchange 
*/

typedef struct {
    OamTkExt ext;
    uint8 keyNumber;        /*  key 0 or key 1 */
    uint8 keyLength;        /*  number of bytes of key to follow */
} PACK OamTkKeyExchange;


/*
*  File Transfer
*/

/* Read / Write Request */
typedef enum {
    OamTkFileBoot,
    OamTkFileApp,
    OamTkFilePersonality,
    OamTkFileDiag,

    OamTkFileNumTypes
} PACK OamTkFileType;

typedef struct {
    OamTkExt ext;
    uint8 file;
} PACK OamTkFileRequest;

/* Data blocks are a sequence number followed by a number of bytes of data */
typedef struct {
    OamTkExt ext;
    uint16 blockNum;        /*  sequence number for this block */
    uint16 size;        /*  number of bytes of data to follow this block */
} PACK OamTkFileData;

typedef enum {
    OamTkFileErrOk,

    OamTkFileErrUndefined,
    OamTkFileErrNotFound,
    OamTkFileErrNoAccess,
    OamTkFileErrFull,
    OamTkFileErrIllegalOp,
    OamTkFileErrUnknownId,

    OamTkFileErrBadBlock,
    OamTkFileErrTimeout,
    OamTkFileErrBusy
} PACK OamTkFileErr;

/* acknowledge receipt of block blockNum */
typedef struct {
    OamTkExt ext;
    uint16 blockNum;        /* sequence number for this block */
    uint8 err;
} PACK OamTkFileAck;


/*
* I2C commands
*/

typedef enum {
    OamTkI2cErrOk,

    OamTkI2cErrFailed
} PACK OamTkI2cErr;

typedef struct {
    OamTkExt ext;
    uint8 i2cDevId;     /* < Address of the I2C device */
    uint16 readLen;     /* < # bytes to read  */

    uint8 addrLen;      /* < # bytes of addr[] */
    uint8 addr[1];      /* < address within the I2C device to read from */
} PACK OamTkI2cRdReq;


typedef struct {
    OamTkExt ext;
    uint8 err;
    uint16 len;         /* < # bytes in rdData[] */
    uint8 rdData[1];        /* < bytes read from the I2c device  */
} PACK OamTkI2cRdResp;


typedef struct {
    OamTkExt ext;
    uint8 i2cDevId;     /* < Address of the I2C device */
    uint16 dataLen;     /* < # bytes of wrData[] */
    uint8 wrData[1];        /* < data to write to the device */
} PACK OamTkI2cWrReq;

typedef struct {
    OamTkExt ext;
    OamTkI2cErr err;
} PACK OamTkI2cWrResp;


typedef struct {
    uint16 Ver;         /* < Boot version */
    uint32 Crc;         /* < Boot crc 32 */
} PACK LoadInfo;

typedef struct {
    uint16 eponVid;     /* EPON VLAN ID */
    uint16 userVid;     /* User VLAN ID */
    uint8 maxGroups;        /* Max allowed IGMP groups for this VLAN */
} PACK IgmpVlanCfg;

typedef struct {
    Bool ignoreUnmanaged;   /* Action for unmanaged groups */
    uint8 numVlans;     /* Number of IGMP VLANs */
    IgmpVlanCfg vlanCfg[1]; /* IGMP port settings */
} PACK IgmpVlanRecord;

/*
 * ClockTransport - ONU configuration
*/
typedef enum {
    TkOnuKeyAll = 0,
    TkOnuKeyInit,
    TkOnuKeyTodEnable,
    TkOnuKey1ppsEnable,
    TkOnuKeyAdjust,
    TkOnuKeyPonRange,
    TkOnuKeyFakeMpcpJump,
    TkOnuNumKeys
} PACK TkOamClkTransKey;


typedef struct {
    uint8 key;
    uint8 reinit;       /* Non 0 : Reinitialize ONU clk transport */
    uint8 tod;          /* Non 0 : Enable tod output */
    uint8 pulse;        /* Non 0 : Enable 1pps output */
    int32 adj;          /* Per-onu 1pps offset adjustment */
    uint32 rtt;         /* Round trip time */
    int32 rsvd;         /* Reserved */
} PACK TkOamOnuClkTransConfig;


/*
* Chip version and ID enumerations
*/

#ifndef TeknovusChipTypes

#define TeknovusChipTypes

typedef enum {
    Tk3701ChipType = 0x3701,
    Tk3711ChipType = 0x3711,
    Tk3713ChipType = 0x3713,
    Tk3721ChipType = 0x3721
} PACK TkChipTypes;

#endif /* TeknovusChipTypes */


typedef uint16 SwVersion;

typedef uint16 TkChipId;
typedef uint16 TkJedecId;
typedef uint32 TkChipVersion;
/* from SysInfo.h */


typedef struct {
    uint8 EponMac[6];       /* EPON MAC address */
    uint32 onuModel;        /* ONU model */
    uint16 chipModel;       /* chip ID */

    uint16 app0Ver;     /* App0 version */
    uint16 app1Ver;     /* App1 version */
    uint16 bootVer;     /* Boot version */
    uint16 persVer;     /* Personality version */
} PACK ONU_CFG_INFO;


typedef struct {
    uint16 threshold[8];
} PACK DbaQueueSets;

typedef struct {
    uint8 indexes;
    DbaQueueSets QueueSets[1];
} PACK CTC_DBA_CFG;


typedef struct {
    uint8 report;
    uint16 threshold[1];
} PACK CtcDbaQueueHdr;

typedef struct {
    uint8 connection;           /* 1 - connect, 0 - disconnect*/
    uint8 Oam_link_established; /* 1 - established, 0 - not*/
    uint8 authorization_state;  /* 1 - authorizated, 0 - not*/
    uint8 Pon_Loopback;         /* 1 - loopback, 0 - normal*/
    uint8 Olt_MAC_addr[6];
    uint16 ONU_LLID;
} PACK TkEponRegState;

/******************************************************************************
 * vlan Tk vlan To LLID mapping conde
 * vlan field depdent on the match mode
 * link 
 * queue
 *****************************************************************************/
typedef struct {
    uint16 vlan;        /*the effect information is depedent on the matchmode */
    uint8 link;
    uint8 queue;
} PACK TkOamVlanToLLIDMappingCond;

/******************************************************************************
 * Vlan To LLID mapping
 *      matchMode: 0: DA,1: VID, 2: COS, 3: VID+COS
 * Flags:
 *      0x01: drop untagged or unknown
 *      0x02: strip vlan tag
 * Default destination:
 *      default link
 *      default queue
 * NumOfVlanEntry
 * Entry:
 *      VLAN tag(VID+COS)
 *      Link
 *      queue
 ******************************************************************************/
typedef struct {
    uint8 matchMode;
    uint8 flags;
    uint8 defaultLink;
    uint8 defaultQueue;
    uint8 numOfVlanEntry;
    TkOamVlanToLLIDMappingCond cond[1];
} PACK TkOamVlanToLLIDMapping;

/*
 * Below strure is only used in 10G MXU SDK
 */
typedef struct {
    uint8 oamLinkEstablished;   /* 1 - established, 0 - not*/
    uint8 authorizationState;   /* 1 - authorizated, 0 - not*/
    uint8 loopBack;     /* 1 - loopback, 0 - normal*/
    uint16 linkLlid;    /* the LLID for the link*/
} PACK OamLinkRegInfo;


typedef enum {
    OamRxOpticalLos = 0,
    OamRxOpticalUnLock = 1,
    OamRxOpticalLock = 2,
    OamRxOpticalStateNum
} PACK OamRxOpticalState;


typedef struct {
    Bool connection;        /* True : connect, False : disconnect*/
    OamRxOpticalState rxOptState;   /* the state of Rx Optical*/
    uint8 linkNum;
    uint8 Olt_MAC_addr[6];
    OamLinkRegInfo linkInfo[1];
} PACK OamExtEponStatus;

typedef struct {
    uint16 laserTxPwrDisableTime;
} PACK OamLaserTxPwrCtl;

typedef struct {
    uint8 failsafeCount;    /* number of failsafes being returned.*/
    uint8 failsafes[1];     /* The failsafes being returned.*/
} PACK OamExtFailsafes;



#if defined(UNIX) || defined(VXWORKS) || defined(LINUX)
/* Restore packing to previous setting */
#pragma pack ()
#endif

#if defined(__cplusplus)
}
#endif

#endif /* Oam.h */
