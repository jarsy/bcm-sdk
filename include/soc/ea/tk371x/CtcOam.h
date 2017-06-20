/*
 * $Id: CtcOam.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcOam.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_CtcOam_H
#define _SOC_EA_CtcOam_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/Oam.h>


#if defined(UNIX) || defined(VXWORKS) || defined(LINUX)
#pragma pack (1)
#endif

/*
 * OamCtcOpcode - OAM opcodes for CTC extended OAM
 *
 * The following codes define the extended OAM opcodes as defined in the China
 * Telecom EPON Technical Specification and Requirement version 1.0 Final.
 */
typedef enum {
    OamCtcReserved = 0x00,
    OamCtcExtVarRequest = 0x01,
    OamCtcExtVarResponse = 0x02,
    OamCtcSetRequest = 0x03,
    OamCtcSetResponse = 0x04,
    OamCtcAuthentication = 0x05,
    OamCtcFileUpgrade = 0x06,
    OamCtcChurning = 0x09,
    OamCtcDba = 0x0A
} PACK OamCtcOpcode;



/*
 * OamCtcBranch - OAM TLV Branch Identifiers
 * 
 *  
 */
typedef enum {
    OamCtcBranchTermination = 0x00, /* Not in specification */
    OamCtcBranchAttribute = OamBranchAttribute, /* IEEE 802.3 Clause 30 */
    OamCtcBranchAction = OamBranchAction,   /* IEEE 802.3 Clause 30 */
    OamCtcBranchObjInst = 0x36, /* OAM attr/act context */
    OamCtcBranchObjInst21 = 0x37,
    OamCtcBranchExtAttribute = 0xC7,    /* CTC extended attr */
    OamCtcBranchExtAction = 0xC9,   /* CTC extended action */
    OamCtcBranchMax = OamCtcBranchExtAction + 1
} PACK OamCtcBranch;



/*
 *  OamCtcContext - OAM TLV Action/Attribute Context
 *  
 *  
 */
typedef enum {
    OamCtcContextOnu = 0x0000,
    OamCtcContextPort = 0x0001,
    OamCtcContextCard = 0x0002,
    OamCtcContextLLID = 0x0003,
    OamCtcContextPON_IF = 0x0004,
    OamCtcContextToU16 = 0x7fff
} PACK OamCtcContext;

typedef enum {
    OamIeeePhyAdminState = 0x0025,
    OamIeeeAutoNegAdminState = 0x004F,
    OamIeeeAutoNegLocalTechAble = 0x0052,
    OamIeeeAutoNegAdvTechAble = 0x0053,
    OamIeeeFecAble = 0x0139,
    OamIeeeFecMode = 0x013A,
    OamIeeeAttrU16 = 0x7FFF
} PACK OamIeeeAttr;

typedef enum {
    OamCtcObjONU = 0x0000,
    OamCtcObjPort = 0x0001,
    OamCtcObjCard = 0x0002,
    OamCtcObjLLID = 0x0003,
    OamCtcObjPonIF = 0x0004,
    OamCtcObjEnd = 0x7FFF
} PACK OamCtcObjInstant;

typedef enum {
    OamCtcPortOnu = 0x0000,
	OamCtcPortEther = 0x0001,
    OamCtcPortVoip = 0x0002,
	OamCtcPortADSL2 = 0x0003,
	OamCtcPortVDSL2 = 0x0004,
	OamCtcPortE1 = 0x0005,
}PACK OamCtcObjUniPort;

typedef struct {
    uint32 uniPortType:8;
    uint32 frameNum:2;
    uint32 slotNum:6;
    uint32 uniPortNum:16;
} PACK OamCtcObjPortInstant;

/*
 *  OamCtcAttr - OAM TLV Leaf Attribute Identifiers
 *  
 *  
 */
typedef enum {
    /*
     * ONU Object Class Attributes, Leaf 0x0001-0x000F
     */
    OamCtcAttrOnuSn = 0x0001,
    OamCtcAttrFirmwareVer = 0x0002,
    OamCtcAttrChipsetId = 0x0003,
    OamCtcAttrOnuCap = 0x0004,
    OamCtcAttrOptTranDiag = 0x0005,
    OamCtcAttrServiceSLA = 0x0006,
    OamCtcAttrOnuCap2 = 0x0007,
    OamCtcAttrHoldoverConfig = 0x0008,
    OamCtcAttrMxUMnGlobalPara = 0x0009,
    OamCtcAttrMxUMngSnmpPara = 0x000A,
    OamCtcAttrPonIfAdmin = 0x000B,

    /*
     * Port Object Class Attributes, Leaf 0x0011-0x001F
     */
    OamCtcAttrEthLinkState = 0x0011,
    OamCtcAttrEthPortPause = 0x0012,
    OamCtcAttrEthPortPolice = 0x0013,
    OamCtcAttrVoipPort = 0x0014,
    OamCtcAttrE1Port = 0x0015,
    OamCtcAttrEthPortDsRateLimit = 0x0016,
    OamCtcAttrPortLoopDetect = 0x0017,
    /*
     * VLAN Object Class Attributes, Leaf 0x0021-0x002F
     */
    OamCtcAttrVlan = 0x0021,

    /*
     * QoS Object Class Attributes, Leaf 0x0031-0x003F
     */
    OamCtcAttrClassMarking = 0x0031,

    /*
     * Mcast Object Class Attribute, Leaf 0x0041-0x004F
     */
    OamCtcAttrMcastVlan = 0x0041,
    OamCtcAttrMcastStrip = 0x0042,
    OamCtcAttrMcastSwitch = 0x0043,
    OamCtcAttrMcastCtrl = 0x0044,
    OamCtcAttrGroupMax = 0x0045,

    /*
     * Fast Leave Class Attribute, Leaf 0x0046-0x0047
     */
    OamCtcAttrFastLeaveAbility = 0x0046,
    OamCtcAttrFastLeaveState = 0x0047,

    /*
     * LLID leaf base
     */
    OamCtcAttrLLIDQueueConfig = 0x0051,
    /*
     * Alarm Class Attribute, Leaf 0x0046-0x0047
     */
    OamCtcAttrAlarmAdminState = 0x0081,
    OamCtcAttrAlarmThreshold = 0x0082,

    /*
     * VoIP Class Attribute, Leaf 0x0061 - 0x006D
     */
    OamCtcAttrIadInfo = 0x0061,
    OamCtcAttrVoIPFirst = OamCtcAttrIadInfo,
    OamCtcAttrGlobalParaConfig = 0x0062,
    OamCtcAttrH248ParaConfig = 0x0063,
    OamCtcAttrH248UserTidInfo = 0x0064,
    OamCtcAttrH248RtpTidConfig = 0x0065,
    OamCtcAttrH248RtpTidInfo = 0x0066,
    OamCtcAttrSipParaConfig = 0x0067,
    OamCtcAttrSipUserParaConfig = 0x0068,
    OamCtcAttrFaxModemConfig = 0x0069,
    OamCtcAttrH248IadOperationStatus = 0x006A,
    OamCtcAttrPotsStatus = 0x006B,
    OamCtcAttrIadOperation = 0x006C,
    OamCtcAttrSipDigitMap = 0x006D,
    OamCtcAttrVoIPLast = 0x007F,
    /*
     * ONUTxPowerSupplyControl, Leaf 0x00A1
     */
    OamCtcAttrOnuTxPowerSupplyControl = 0x00A1,

    OamCtcAttrU16 = 0x7FFF
} OamCtcAttr;

typedef enum {
    OamIeeePhyAdminCtl = 0x0005,
    OamIeeeAutoNegRestart = 0x000B,
    OamIeeeAutoNegAdminCtl = 0X000C,
    OamIeeeActU16 = 0x7FFF
} PACK OamIeeeAct;

/*
 * OamCtcAct - OAM TLV Leaf Action Identifiers
 *  
 *  
 */
typedef enum {
    OamCtcActReset = 0x0001,

    /*
     * New Attribute in CTC2.0, Leaf 0x0048
     */
    OamCtcActFastLeaveAdminCtl = 0x0048,
    OamCtcActMultLLIDAdminCtl = 0x0202,
    OamCtcActResetCard = 0x0401,

    OamCtcActU16 = 0x7FFF
} PACK OamCtcAct;



/*
 * CTC OAM TLV Auxillary Type Definitions
 */

#define OamCtcEponPortNum           0x00
#define OamCtcEthPortStart          0x01
#define OamCtcEthPortEnd            0x4F
#define OamCtcPotsPortStart         0x50
#define OamCtcPotsPortEnd           0x8F
#define OamCtcE1PortStart           0x90
#define OamCtcE1PortEnd             0x9F
#define OamCtcAllEthPorts           0xFF

#define OamCtcAllEthPorts21         0xFFFF

/*
 *  OamCtcPortNum - Port number used in CTC port object TLVs
 *  
 *  There is an offset port number scheme for port messages in CTC OAM as
 *  follows:
 *  
 *      - 0x00 to 0x4F: Ethernet ports, EPON port is port 0
 *      - 0x50 to 0x8F: VoIP ports
 *      - 0x90 to 0x9F: E1 ports
 */
typedef uint8 OamCtcPortNum;



/*
 *  OamCtcLinkState - Ethernet port status
 *  
 *  The following is reported on EthLinkStatus message (leaf 0x11).
 */
typedef enum {
    OamCtcLinkDown = 0x00,
    OamCtcLinkUp = 0x01
} PACK OamCtcLinkState;



/*
 *  OamCtcServices - ONU services supported
 *  
 *  The following is used by the ONU Capabilities message (leaf 0x0004).  These
 *  values are ORed together to determine the possible capabities of an ONU.
 *  The number of ports that a system has for each service is in a separate
 *  portion of the message.  It would seems that a service can be supported even
 *  if there is not a dedicated port on the system to handle the service.
 */
typedef enum {
    OamCtcServNoServices = 0x00,
    OamCtcServSupportGe = 0x01,
    OamCtcServSupportFe = 0x02,
    OamCtcServSupportVoip = 0x04,
    OamCtcServSupportTdm = 0x08
} PACK OamCtcServices;



/*
 *  OamCtcPortLock - CTC OAM port enable/disable type
 *  
 *  This type is passed in the VoIP and E1 port management message to enable or
 *  disable a given port.
 */
typedef enum {
    OamCtcPortLock = 0x00,
    OamCtcPortUnlock = 0x01
} PACK OamCtcPortState;


/*
 *  OamCtcVlanMode - CTC OAM VLAN mode
 *  
 *  These are the various VLAN modes that need to be supported by CTC ONUs.  The
 *  VLAN stacking mode is currently options.
 */
typedef enum {
    OamCtcVlanTransparent = 0x00,
    OamCtcVlanTag = 0x01,
    OamCtcVlanTranslation = 0x02,
    OamCtcVlanN21Translation = 0x03,
    OamCtcVlanTrunk = 0x04,
    OamZteVlanTrunk = 0x13,
    OamZteVlanHybrid = 0x14,
    OamCtcNumVlans
} PACK OamCtcVlanMode;


typedef enum {
    OamCtcFieldDaMac,
    OamCtcFieldSaMac,
    OamCtcFieldVlanPri,
    OamCtcFieldVlanId,
    OamCtcFieldEthertype,
    OamCtcFieldDestIp,
    OamCtcFieldSrcIp,
    OamCtcFieldIpType,
    OamCtcFieldIpTos,
    OamCtcFieldIpPrec,
    OamCtcFieldL4SrcPort,
    OamCtcFieldL4DestPort,
    OamCtcNumFieldSelects,
} PACK OamCtcFieldSelect;

typedef enum {
    OamCtcRuleOpFalse,
    OamCtcRuleOpEqual,
    OamCtcRuleOpNotEq,
    OamCtcRuleOpLtEq,
    OamCtcRuleOpGtEq,
    OamCtcRuleOpExist,
    OamCtcRuleOpNotEx,
    OamCtcRuleOpTrue,
    OamCtcNumRuleOp
} PACK OamCtcRuleOp;

typedef uint8 OamCtcMatchValue[6];

typedef struct {
    uint8 field;
    OamCtcMatchValue matchVal;
    uint8 op;
} PACK OamCtcRuleCondition;

typedef enum {
    OamCtcRuleActionDel = 0x00,
    OamCtcRuleActionAdd = 0x01,
    OamCtcRuleActionClear = 0x02,
    OamCtcRuleActionList = 0x03
} PACK OamCtcRuleAction;

typedef struct {
    uint8 prec;
    uint8 length;
    uint8 queueMapped;
    uint8 pri;
    uint8 numClause;
    OamCtcRuleCondition clause[0];
} PACK OamCtcRule;

typedef struct {
    uint16 Qid;
    uint16 QWrr;
} PACK OamCtcLLIDQAttr;
typedef struct {
    uint8 numOfQ;
    OamCtcLLIDQAttr QAttrList[0];
} PACK OamCtcLLIDQconfig;

/*
 *  OamCtcMcastVlanOp - CTC OAM multicast vlan operations
 *  
 *  A group of VIDs may be added or deleted from a port at one type.  This type
 *  defines the operation of that message.
 */
typedef enum {
    OamCtcMcastVlanDel = 0x00,
    OamCtcMcastVlanAdd = 0x01,
    OamCtcMcastVlanClear = 0x02,
    OamCtcMcastVlanList = 0x03
} PACK OamCtcMcastVlanOp1;

typedef enum {
    OamCtcMcastGroupDel = 0x00,
    OamCtcMcastGroupAdd = 0x01,
    OamCtcMcastGroupClear = 0x02,
    OamCtcMcastGroupList = 0x03
} PACK OamCtcMcastGroupOp;

typedef enum {
    OamCtcMcastIgmpMode = 0x00,
    OamCtcMcastHostMode = 0x01
} PACK OamCtcMcastMode;


#define OamCtcMcastCtrlToBeContinued        0x80


typedef enum {
    McastDaMacOnly = 0x00,
    McastDaMacVid = 0x01,
    McastDaMacSaMac = 0x02,
    McastCtrlNumModes
} PACK McastCtrl;

typedef enum {
    OamCtcMcastCtrlDel = 0x00,
    OamCtcMcastCtrlAdd = 0x01,
    OamCtcMcastCtrlClear = 0x02,
    OamCtcMcastCtrlList = 0x03
} PACK OamCtcMcastCtrlOp;

typedef struct {
    VlanTag userId;
    VlanTag vlanTag;
    MacAddr da;
} PACK McastEntry;




/*
 *  CTC OAM TLV Data Definitions
 */



/*
 *  OamCtcTlvOnuSn - ONU serial number
 *  
 *  Object: ONU
 *  Leaf:   OamCtcAttrOnuSn (0x0001)
 */
typedef struct {
    uint32 vendorId;
    uint32 onuModel;
    MacAddr onuId;
    uint8 hardVersion[8];
    uint8 softVersion[16];
} PACK OamCtcTlvOnuSn;




/*
 *  OamCtcTlvFirmwareVer - ONU firmware version
 *  
 *  Object: ONU
 *  Leaf:   OamCtcAttrFirmwareVer (0x0002)
 */
typedef struct {
    SwVersion version;
} PACK OamCtcTlvFirmwareVer;



/*
 *  OamCtcTlvChipsetId - ONU chipset identification
 *  
 *  Object: ONU
 *  Leaf:   OamCtcAttrChipsetId (0x0003)
 */
typedef struct {
    TkJedecId vendorId;
    TkChipId chipModel;
    TkChipVersion revisionDate;
} PACK OamCtcTlvChipsetId;


/*
 *  OamCtcTlvOnuCap - ONU capabilities
 *  
 *  Object: ONU
 *  Leaf:   OamCtcAttrOnuCap (0x0004)
 */
typedef struct {
    uint8 services;
    uint8 numGePorts;
    uint8 geBitmap[8];
    uint8 numFePorts;
    uint8 feBitmap[8];
    uint8 numPotsPorts;
    uint8 numE1Ports;
    uint8 numUpQueue;
    uint8 upQueueMax;
    uint8 numDnQueue;
    uint8 dnQueueMax;
    Bool batteryBack;
} PACK OamCtcTlvOnuCap;

/*
 *  OamCtcTlvOnuIf - ONU interface type
 *  
 *  Object:     ONU
 *  Leaf:       OamCtcAttrOnuCap (0x0007)
 */

typedef struct {
    uint32 ifType;
    uint16 numPort;
} PACK OamCtcTlvOnuIf;


/*
 *  OamCtcTlvOnuCap - ONU capabilities2
 *  
 *  Object:     ONU
 *  Leaf:       OamCtcAttrOnuCap2 (0x0007)
 */
typedef struct {
    uint32 onuType;     /* this is NOT byte-swapped! */
    uint8 multiLlid;        /* 0x0 = multi; 0x1 = single */
    uint8 protection;
    uint8 numPonIf;
    uint8 numSlot;
    uint8 numIfType;
    OamCtcTlvOnuIf ifs[0];
} PACK OamCtcTlvOnuCap2;

/*
 *  OamCtcTlvEthLinkState - ONU ethernet port status
 *  Object: Port
 *  Leaf:   OamCtcAttrEthLinkState (0x11)
 */
typedef struct {
    uint8 state;
} PACK OamCtcTlvEthLinkState;



/*
 *  OamCtcTlvEthPortPause - ONU port flow control configuration
 *  
 *  Object: Port
 *  Leaf:   OamCtcAttrEthPortPause (0x0012)
 */
typedef struct {
    Bool enabled;
} PACK OamCtcTlvEthPortPause;



/*
 *  OamCtcTlvEthPortPolice - ONU port rate policying configuration
 *  
 *  Object: Port
 *  Leaf:   OamCtcAttrEthPortPolice (0x0013)
 */
typedef struct {
    Bool enabled;
    uint8 cir[3];
    uint8 cbs[3];
    uint8 ebs[3];
} PACK OamCtcTlvEthPortPolice;



/*
 *  OamCtcTlvVoipPort - ONU VoIP port configuration
 *  
 *  Object: Port
 *  Leaf:   OamCtcAttrVoipPort (0x0014)
 */
typedef struct {
    uint8 lock;
} PACK OamCtcTlvVoipPort;



/*
 *  OamCtcTlvE1Port - ONU E1 port configuration
 *  
 *  Object: Port
 *  Leaf:   OamCtcAttrE1Port (0x0015)
 */
typedef struct {
    uint8 lock;
} PACK OamCtcTlvE1Port;

/*
 *  OamCtcTlvEthPortDsRateLimiting - ONU port downstream rate limiting configuration
 *  
 *  Object:     Port
 *  Leaf:       OamCtcAttrEthPortDsRateLimiting (0x0016)
 */
typedef struct {
    Bool enabled;
    uint8 cir[3];
    uint8 pir[3];
} OamCtcTlvEthPortDsRateLimiting;


/*
 *  OamCtcTlvVlan - ONU VLAN configuration
 *  
 *  Object: VLAN
 *  Leaf:   OamCtcAttrVlan (0x0021)
 */

typedef struct {
    uint8 mode;          /* OamCtcVlanMode*/
    uint8 pad[0];
} PACK OamCtcTlvVlan;

typedef struct {
    uint32 vlanContent[0];
} PACK OamCtcVlanContent;

typedef struct {
    uint32 defaultVlan;
    uint16 cntOfN21Entry;
    uint8 pad[0];
} PACK OamCtcVlanN21VlanAggrMode;

typedef struct {
    uint16 cntOfAggrVlanInEntry;
    uint32 vlanContent[0];
} PACK OamCtcVlanN21VlanEntry;

typedef enum {
    OamCtcClassRuleDel = 0x00,
    OamCtcClassRuleAdd = 0x01,
    OamCtcClassRuleClear = 0x02,
    OamCtcClassRuleList = 0x03
} PACK OamCtcClassRuleOp;

typedef struct {
    uint8 length;
    uint8 mode;         /* OamCtcVlanMode*/
    uint8 pad[0];
} PACK OamCtcVlanInfo;

/*
 *  OamCtcTlvClassMarking - ONU traffic classification configuration
 *  
 *  Object: QoS
 *  Leaf:   OamCtcAttrClassMarking (0x0031)
 */
typedef struct {
    uint8 action;       /* OamCtcClassRuleOp*/
    uint8 numRules;
    OamCtcRule rule[0];
} PACK OamCtcTlvClassMarking;

typedef struct {
    uint8 action;
    uint8 numPrec;
    uint8 precToDelete[1];
} PACK OamCtcTlvClassMarkingDelete;



/*
 *  OamCtcTlvMcastVlan - Add/delete ONU multicast VLAN
 *  
 *  Object: Multicast
 *  Leaf:   OamCtcAttrMcastVlan (0x0041)
 */

typedef struct {
    uint8 operation;
    VlanTag vid[0];
} PACK OamCtcTlvMcastVlan;

typedef struct {
    uint8 length;
    uint8 operation;
    uint16 vidList[0];
} PACK OamCtcMcastVlanInfo;

/*
 *  OamCtcTlvMcastVlanStrip - Strip tags from multicast frames
 *  
 *  Object: Multicast
 *  Leaf:   OamCtcAttrMcastVlanStrip (0x0042)
 */
typedef enum {
    OamCtcUnstrip = 0x00,
    OamCtcStrip = 0x01,
    OamCtcSwitch = 0x02
} PACK OamCtcMcastVlanOp;

typedef struct {
    uint16 mVlan;
    uint16 uVlan;
} PACK OamCtcTlvMcastVlanStripSwitchVlan;

typedef struct {
    uint8 num;
    OamCtcTlvMcastVlanStripSwitchVlan vlan[0];
} PACK OamCtcTlvMcastVlanStripSwitch;

typedef struct {
    uint8 tagStripped;
    OamCtcTlvMcastVlanStripSwitch swVlan[0];
} PACK OamCtcTlvMcastVlanStrip;



/*
 *  OamCtcTlvMcastSwitch - IGMP snooping or host controled IGMP
 *  
 *  Object: Multicast
 *  Leaf:   OamCtcAttrMcastSwitch (0x0043)
 */
typedef struct {
    uint8 mode;
} PACK OamCtcTlvMcastSwitch;



/*
 *  OamCtcTlvMcastCtrl - ONU multicast control
 *  
 *  Object: Multicast
 *  Leaf:   OamCtcAttrMcastCtrl (0x0044)
 */

typedef struct {
    uint8 action;
    uint8 ctrlType;
    uint8 numEntries;
    McastEntry entry[0];
} PACK OamCtcTlvMcastCtrl;



/*
 *  OamCtcTlvGroupMax - Multicast group maximums
 *  
 *  Object: Multicast
 *  Leaf:   OamCtcTlvGroupMax (0x0043)
 */
typedef struct {
    uint8 maxGroup;
} PACK OamCtcTlvGroupMax;


/*
 *  OamCtcExt - CTC extension OAMPDU header
 *  This record comes at the begining of an China Telecom extended OAMPDU, just
 *  after the CTC OUI.  It contains the OAM opcode.
 */
 
typedef struct {
    uint8 opcode;
} PACK OamCtcExt;


/*
 *   OamCtcOamVersion - OAM version type
 *  The following type represents the OAM version number used in the CTC
 *  extended OAM discovery sequence.  It defines the version of the CTC OAM as
 *  well as the version number of any other extended OAM as well.
 */
typedef uint8 OamCtcOamVersion;



/*
 *  OamCtcVersionInfoState - Version negotiation state
 *  This type defines the states that an OLT or ONU may be in when performing
 *  CTC OAM version negotiation.
 */
typedef enum {
    CtcVersionUnknown,
    CtcVersionNegotiating,
    CtcVersionKnown
} PACK OamCtcVersionInfoState;



/*
 *  OamCtcInfoTlvExtData - Extended OAM version support
 *  This structure contains a single entry for extended OUI information durring
 *  China Telecom OAM discovery.  This structure will be found at the end of an
 *  CTC Organazation Specific Information TVL.  Inclusion of extended OUI
 *  information is optional.
 */
typedef struct {
    IeeeOui oui;        /*< Vendor OUI */
    OamCtcOamVersion version;   /*< Vendor OAM version number */
} PACK OamCtcInfoTlvExtData;



/*
 *  OamCtcEncryptKey - CTC encryption key
 *  The following record defines the China Telecom triple churning encryption
 *  key.  The byte layout is as follows:
 *
 *  - Byte 0: X1..X8
 *  - Byte 1: P1..P8
 *  - Byte 2: P9..P16
 */
typedef union {
    struct {
    uint8 xPart;
    uint16 pPart;
    } parts;
    uint8 byte[3];
} PACK OamCtcEncryptKey;



/*
 *  OamChurningOpcode - CTC churning PDU opcode
 * 
 *  The following defines the opcodes that are found in China Telecom Churning
 *  PDUs.  Possilble operations are new_key_request (ChurningKeyRequest) which
 *  the OLT sends the ONU to force the ONU to generate a new encryption key and
 *  new_churning_key (ChurningNewKey) which the ONU send to the OLT to inform
 *  the OLT of the new key.
 */
typedef enum {
    ChurningKeyRequest,
    ChurningNewKey
} PACK OamCtcChurningOpcode;



/*
 *  OamChurningPdu - CTC churning PDU
 *
 *  The following message is a China Telecom Churning OAMPDU (0x09).  This PDU
 *  functions for both opcodes although the key is not used in a key request
 *  message.
 */
typedef struct {
    uint8 ext;
    uint8 opcode;
    uint8 keyIndex;
    OamCtcEncryptKey key;
} PACK OamCtcChurningPdu;


/*
 *  OamCtcDbaOpcode - CTC DBA PDU opcode
 *
 *  The following defines all of the various operation that may be found in a
 *  China Telecom extended Dynamic Bandwidth Allocation PDU.
 */
typedef enum {
    DbaGetRequest,
    DbaGetResponse,
    DbaSetRequest,
    DbaSetResponse
} PACK OamCtcDbaOpcode;



/*
 *  OamCtcQueueSetCount - Number of queue sets
 *
 *  The 3714 hardware supports two queue sets as defined by EponBurstCap and
 *  EponBurstCap2.  Setting or requesting any queue sets beyond two will lead
 *  to very undesirable behavior.
 */
extern
uint8 OamCtcQueueSetCount;

/*
 * OamCtcDbaQueueSet - CTC DBA queue set information
 *
 *  The following record contains the infomation to set the queue thresholds
 *  for a set of queues.  A threshold only applies to those queues who have
 *  their bit set in the report bit map.
 */
typedef struct {
    uint8 report;
    uint16 threshold[8];
} PACK OamCtcDbaQueueSet;

/*
 *  OamCtcDbaData - CTC DBA PDU
 *  
 */
typedef struct {
    uint8 num;          /* number of queue sets (2-4).*/
    OamCtcDbaQueueSet set[4];
} PACK OamCtcDbaData;

typedef struct {
    uint8 report;
    uint16 threshold[1];
} PACK OamCtcDbaQueueSetPack;

typedef struct {
    uint8 num;
    OamCtcDbaQueueSetPack set[1];
} PACK OamCtcDbaDataPack;

/*
 *  OamCtcDbaPdu - CTC DBA PDU
 * 
 *  The following defines the structure of a China Telecom Dynamic Bandwidth
 *  Allocation PDU (0x0A).  The opcode determise whether the message is a
 *  get/set request or a get/set response.  The indexes field is the number of
 *  queue sets in the message.
 */
typedef struct {
    uint8 ext;
    uint8 opcode;
    uint8 indexes;
    OamCtcDbaQueueSet set[1];
} PACK OamCtcDbaPdu;



/*
 *  OamCtcDbaPduSetResponse - Response structure to a CTC DBA set message
 * 
 *  Here is the platypus in the system.  The set message response is the same
 *  as the rest of the message with exception of a random ACK field in the 
 *  middle.  The ACK is set to TRUE on the response if everything went OK.
 */
typedef struct {
    uint8 ext;
    uint8 opcode;
    Bool ack;
    uint8 indexes;
} PACK OamCtcDbaPduSetResponse;


typedef struct {
    uint8 indexes;
    OamCtcDbaQueueSet set[1];
} PACK CtcDbaSets;


/*
 *  OamCtcInfoTlvHeader - Header for CTC information TVL
 *
 *  This structure is the header that will be found on China Telecom
 *  Organazation Specific Information TVL messages.  This message should be
 *  exchanged by the OLT and ONU during OAM discovery.
 */
typedef struct {
    OamTlvType type;        /* < OAM opcode (0xFE) */
    uint8 length;       /* < TLV size including this header */
    IeeeOui oui;        /* < CTC OUI */
    Bool support;       /* < Is CTC OAM supported */
    OamCtcOamVersion version;   /* < CTC OAM version */
    OamCtcInfoTlvExtData ext[1];    /* < Other vendor extended data */
} PACK OamCtcInfoTlvHeader;

typedef struct {
    EthernetVlanData fromVid;
    EthernetVlanData toVid;
} PACK CtcVlanTranslatate;

typedef struct {
    uint8 CtcOamVlanMode;
    uint8 CtcOamVlanData[0];
} PACK CtcOamVlanEntry;

typedef struct {
    EthernetVlanData tag[0];
} PACK CtcOamVlanTag;

typedef struct {
    EthernetVlanData defaultVlan;
    CtcVlanTranslatate vlanTranslateArry[0];
} PACK CtcOamVlanTranslate;

typedef struct {
    EthernetVlanData defaultVlan;
    EthernetVlanData vlanTrunkArry[0];
} PACK CtcOamVlanTrunk;

typedef struct {
    uint32 mngIpAddr;
    uint32 mngIpMask;
    uint32 mngGw;
    uint16 mngDataCvlan;
    uint16 mngDataSvlan;
    uint8 mngDataPri;
} PACK CtcUMnGlobalParameter;

typedef struct {
    uint8 snmpVer;
    uint32 trapHostIpAddr;
    uint16 trapPort;
    uint16 snmpPort;
    int8 securityName[32];
    int8 communityForRead[32];
    int8 communityForWrite[32];
} PACK CtcSNMPParameter;

typedef enum {
    OamCtcAuthserved = 0x00,
    OamCtcAuthRequest = 0x01,
    OamCtcAuthResponse = 0x02,
    OamCtcAuthsuccess = 0x03,
    OamCtcAuthFailure = 0x04,
} PACK OamCtcAuthCode;

typedef struct {
    uint8 authType;     
} PACK CtcAuthRequest;

typedef struct {
    char loid[24];
    char password[12];
} PACK AuthResLoidPassword;

typedef struct {
    uint8 desiredAuthType;
} PACK AuthResFailure;

typedef struct {
    uint8 authType;
    uint8 rescont[0];
} PACK CtcAuthResponse;

typedef struct {
    uint8 failureType;
} PACK CtcAuthFailure;

typedef struct {
    uint8 authCode;
    uint16 len;
    uint8 authCont[0];
} PACK CtcAuthPara;

typedef struct {
    uint16 transceiverTemperature;
    uint16 supplyVoltage;
    uint16 txBiasCurrent;
    uint16 txPower;
    uint16 rxPower;
} PACK CtcOamOpticalTransceiverDiagnosis;

typedef enum {
    HoldoverDisactived = 0x01,
    HoldoverActived = 0x02,
} PACK CtcOamHoldoverState;

typedef struct {
    CtcOamHoldoverState state;  
    uint32 time;
} PACK CtcOamHoldover;

typedef struct {
    uint8 state;
} PACK CtcOamEthLinkState;

typedef struct {
    uint8 state;
} PACK CtcOamEthPortPause;

typedef enum {
    CtcNonFastLeaveInIgmpSnooping = 0x00000000,
    CtcFastLeaveInIgmpSnooping = 0x00000001,
    CtcNonFastLeaveInHostControl = 0x00000010,
    CtcFastLeaveInHostControl = 0x00000011,
    CtcFastLeaveEndMode = 0x7FFFFFFF
} PACK CtcOamFastLeaveAbilityMode;

typedef struct {
    uint32 num;
    CtcOamFastLeaveAbilityMode mode[0];
} PACK CtcOamFastLeaveAbility;

typedef enum {
    FastLeaveDisactived = 0x01,
    FastLeaveActived = 0x02,
    FastLeaveStateEnd = 0x7f,
} PACK CtcOamFastLeaveAdminStateValue;

typedef struct {
    uint32 state;
} PACK CtcOamFastLeaveAdminState;

typedef struct {
    uint8 portNo;
} PACK CtcOamPonIfAdmin;

typedef struct {
    uint8 portType;
    uint8 frameIndex;
    uint8 slotIndex;
    uint16 portIndex;
} PACK CtcPortInst;

typedef struct {
    uint8 vendorId[4];
    uint8 ONUMode[4];
    uint8 ONUID[6];
    uint8 hardwareVersion[8];
    uint8 softwareVersion[16];
} PACK CtcOamOnuSN;

typedef struct {
    uint8 firmwarVersion[255];
} PACK CtcOamFirmwareVersion;

typedef struct {
    uint8 vendorId[2];
    uint8 chipModel[2];
    uint8 revision;
    uint8 IC_VersionDate[3];
} PACK CtcOamChipsetId;

/*
 * OamCtcTlvTxPowerCtrl - Tx Power Supply Control
 *  
 *  Object:     ONU
 *  Leaf:       OamCtcAttrOnuTxPowerSupplyControl (0x00A1)
 */
typedef struct {
    uint32 action;
    MacAddr onuId;
    uint32 optId;
} PACK CtcOamTlvTxPowerCtrl;

typedef struct{
    uint16 temp;
    uint16 vcc;
    uint16 txBias;
    uint16 txPower;
    uint16 rxPower;
} PACK CtcOamTlvPowerMonDiag;

/*
 *  CtcInfoTvlHdrSize - CTC Information TVL header size
 *  This constant is the size of the China Telecom Organazation Specific
 *  Information TVL without any extended vendor OAM data.
 */
#define CtcInfoTvlHdrSize   \
        (sizeof(OamCtcInfoTlvHeader) - sizeof(OamCtcInfoTlvExtData))

#if defined(UNIX) || defined(VXWORKS) || defined(LINUX)
#pragma pack()
#endif

#if defined(__cplusplus)
}
#endif


#endif /* _SOC_EA_CtcOam_H */
