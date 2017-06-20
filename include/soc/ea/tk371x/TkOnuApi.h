/*
 * $Id: TkOnuApi.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOnuApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkOnuApi_H
#define _SOC_EA_TkOnuApi_H

#ifdef __cplusplus
extern "C"   {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/Oam.h>


typedef struct {
    uint8   RegState;       /* 1 - unregistered, 2 - registering, 3 - registered*/
    uint16  PhyLLID;
} PACK OamEponLinkInfo;

typedef struct {
    uint8   EponLosState;   /* 0 - normal, 1 - Los*/
    OamEponLinkInfo LinkInfo[SDK_MAX_NUM_OF_LINK];
} PACK OamEponPortInfo;

typedef struct {
    uint8   PonBaseMac[6];  /* ONU base MAC address*/
    uint8   UserBaseMac[6]; /* user port base MAC address*/
} PACK OamEponBaseMac;

typedef enum {
    EponNormalState = 0,
    EponInLosState  = 1,
} EponLosState;

typedef enum {
    LlidUnregistered,
    LlidWaitingForReg,
    LlidInService,
    LlidWaitingForGates,
    LlidStoppedForDebug = 0x7f
} LlidState;

typedef struct {
    uint8   TKOUI[3];
    uint16  TKOAMVer;
    uint8   CTCOUI[3];
    uint16  CTCOAMVer;
} PACK OamVersion;

/******************************************************************************
 * vlan Tk Link To LLID mapping conde
 * vlan field depdent on the match mode
 * link 
 * queue
 *****************************************************************************/
typedef struct {
    uint16  vlan;       /* the effect info is depedent on the match mode */
    uint8   link;
    uint8   queue;
} TkVlanToLLIDMappingCond;

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
    uint8   matchMode;
    uint8   flags;
    uint8   defaultLink;
    uint8   defaultQueue;
    uint8   numOfVlanEntry;
    TkVlanToLLIDMappingCond cond[63];
} TkVlanToLLIDMapping;

typedef enum {
    Vlan2LLIDMatchOnDA          = 0x00,
    Vlan2LLIDMatchOnVID         = 0x01,
    Vlan2LLIDMatchOnCOS         = 0x02,
    Vlan2LLIDMatchOnVIDAndCOS   = 0x03,
} TkMatchMode;

typedef enum {
    DefaultAction           = 0x0,
    DropUnTaggedOrUnknown   = 0x1,
    StripVlanTag            = 0x2
} TkVlanAction;

typedef enum {
    EponTxLaserNormal       = 0,
    EponTxLaserAlwaysOn     = 1,
    EponTxLaserDisable      = 2,
    EponTxLaserStatusNum
} EponTxLaserStatus;

/*
 * Below struture only used in 10G MXU SDK
 */
typedef struct {
    uint8   oamLinkEstablished; /* 1 - established, 0 - not*/
    uint8   authorizationState; /* 1 - authorizated, 0 - not*/
    uint8   loopBack;           /* 1 - loopback, 0 - normal*/
    uint16  linkLlid;           /* the LLID for the link*/
} TkLinkRegInfo;

typedef struct {
    Bool                connection; /* True : connect, False : disconnect*/
    OamRxOpticalState   rxOptState; /* the state of Rx Optical*/
    uint8               linkNum;
    uint8               Olt_MAC_addr[6];
    TkLinkRegInfo       linkInfo[SDK_MAX_NUM_OF_LINK];
} PACK TkEponStatus;

typedef struct {
    uint8   failsafeCount;
    uint8   failSafeList[CNT_OF_FAIL_SAFE];
} TkFailSafeState;

/* send TK extension OAM OamExtOpInfo and Get response from the ONU */
int32   TkExtOamOnuInfo (uint8 pathId, uint8 LinkId, OamTkInfo * pRxBuf);

/* send TK extension OAM message Get ONU firmware version from the ONU */
int32   TkExtOamGetFirmwareVer (uint8 pathId, uint8 LinkId,
                uint16 * FirmwareVer);

/* send TK extension OAM message Get vendor ID from the ONU */
int32   TkExtOamGetVendorID (uint8 pathId, uint8 LinkId, uint8 * pVendorID);

/* send TK extension OAM message Get JEDEC ID from the ONU */
int32   TkExtOamGetJedecID (uint8 pathId, uint8 LinkId, uint16 * pJedecID);

/* send TK extension OAM message Get Chip ID from the ONU */
int32   TkExtOamGetChipID (uint8 pathId, uint8 LinkId, uint16 * pChipID);

/* send TK extension OAM message Get Chip revision from the ONU */
int32   TkExtOamGetChipRev (uint8 pathId, uint8 LinkId, uint32 * pChipRev);

/* send TK extension OAM message Set reset ONU to the ONU */
int32   TkExtOamSetResetOnu (uint8 pathId, uint8 LinkId);

/* send TK extension OAM message Set restore ONU to the ONU */
int32   TkExtOamSetEraseNvs (uint8 pathId, uint8 LinkId);

/* send TK extension OAM message Set GPIO to the ONU */
int32   TkExtOamSetGpio (uint8 pathId, uint8 LinkId, uint32 GpioMask,
                uint32 GpioVal);

/* Get GPIO value from the ONU through sending TK extension OAM message
 * gpio means all pins' status
 */
int32   TkExtOamGetGpio (uint8 pathId, uint8 LinkId, uint32 * gpio);

int32   TkExtOamSetGpioConfig(uint8 pathId, uint8 LinkId, uint32 mask);

int32   TkExtOamGetGpioConfig(uint8 pathId, uint8 LinkId, uint32 *mask);


/* send TK extension OAM message Get load info from the ONU */
int32   TkExtOamGetLoadInfo (uint8 pathId, uint8 LinkId,
                OamLoadInfo * pLoadInfo);

/* send TK extension OAM message Get FEC mode from the ONU */
int32   TkExtOamGetFECMode (uint8 pathId, uint8 LinkId,
                OamExtFECMode * pFECMode);

/* send TK extension OAM message Set igmp VLAN to the ONU */
int32   TkExtOamSetFECMode (uint8 pathId, uint8 LinkId,
                OamExtFECMode * pFECMode);

/* send TK extension OAM message EPON port enable or disable to the ONU */
int32   TkExtOamSetEponAdmin (uint8 pathId, uint8 LinkId, uint8 EponCtl);
int32   TkExtOamGetEponAdmin (uint8 pathId, uint8 LinkId, uint8 * EponCtl);
int32   TkExtOamGetOnuLlid (uint8 pathId, uint8 linkId, uint16 * llid);
int32   TkExtOamSetEponBaseMac (uint8 pathId, OamEponBaseMac * baseMacAddr);
int32   TkExtOamGetEponPortInfo (uint8 pathId, uint8 linkId,
                OamEponPortInfo * EponPortInfo);
int32   TkExtOamSetBatteryBackup (uint8 pathId, uint8 linkId, uint8 state);
int32   TkExtOamGetBatteryBackup (uint8 pathId, uint8 linkId, uint8 * state);
int32   TkExtOamGetCtcOamVersion (uint8 pathId, OamVersion * version);
int32   TkExtOamSetLaserOn (uint8 pathId, EponTxLaserStatus state);
int32   TkExtOamGetLaserOn (uint8 pathId, EponTxLaserStatus * state);
int32   TkExtOamGetPonLosInfo (uint8 pathId, uint8 * state);
int32   TkExtOamGetPonRegStatus (uint8 pathId, uint8 linkId,
                TkEponRegState * state);
int32   TkExtOamGetEncryptKeyExpiryTime (uint8 pathId, uint8 linkId,
                uint16 * time);
int32   TkExtOamSetEncryptKeyExpiryTime(uint8 pathId, uint8 linkId,
                uint16 time);
int32   TkExtOamSetVlanToLLIDMapping (uint8 pathId, uint8 linkId, uint32 portId,
                TkVlanToLLIDMapping * vlanMappingInfo);
int32   TkExtOamGetVlanToLLIDMapping (uint8 pathId, uint8 linkId, uint32 portId,
                TkVlanToLLIDMapping * vlanMappingInfo);
int32   TkExtOamSetVlan2LLIDMappingStripTag (uint8 pathId, uint8 linkId,
                uint8 strip);
int32   TkExtOamGetMLLIDLinkStatus (uint8 pathId, uint8 linkId, 
                TkEponStatus * mLLIDLinkStatus);
int32   TkExtOamSetLaserTxPwrOffTime (uint8 pathId, uint16 powerOffTime);
int32   TkExtOamSetFailSafeState (uint8 pathId,
                TkFailSafeState * pTkFailSafeState);
int32   TkExtOamGetFailSafeState (uint8 pathId,
                TkFailSafeState * pTkFailSafeState);
int32   TkExtOamSetCTCLoidAuthIfFail(uint8 pathId, uint8 linkId, uint8 auth);
int32   TkExtOamGetCTCLoidAuthIfFail(uint8 pathId, uint8 linkId, uint8 *auth);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkOnuApi_H */
