/*
 * $Id: CtcMiscApi.h,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcMiscApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_CtcMiscApi_H
#define _SOC_EA_CtcMiscApi_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/CtcOam.h>


#define RE_ENABLE_LASER_TX_POWER    0
#define POWER_DOWN_LASER_TX_POWER   65535

typedef struct {
    int8    CtcInfoFromONUPONChipsSetInit;
    uint16  CtcONUSNLen;
    uint8   CtcONUSN[64];
    uint16  CtcFirmwareVerLen;
    uint8   CtcFirmwareVer[64];
    uint16  CtcChipsetIdLen;
    uint8   CtcChipsetId[64];
    uint16  CtcOnuCap1Len;
    uint8   CtcOnuCap1[64];
    uint16  CtcOnuCap2Len;
    uint8   CtcOnuCap2[64];
} CtcInfoFromONUPONChipsSet;

typedef struct {
    uint8   Select;
    uint8   MatchVal[6];
    uint8   ValidOperator;
} CtcExtRuleCondtion;

typedef struct {
    uint8   Priority;
    uint8   QueueMapped;
    uint8   EthernetPri;
    uint8   NumOfEntry;
    CtcExtRuleCondtion cond[16];
} CtcExtRule;

typedef struct {
    uint16  Qid;
    uint16  QWrr;
} CtcExtQConfig;

typedef struct {
    uint32  Action;
    uint8   ONUID[6];
    uint32  OpticalTransmitterID;
} CtcExtONUTxPowerSupplyControl;

#define  HOLDOVERDISACTIVATED  0x01     /* default, holdover disactivated */
#define  HOLDOVERACTIVATED	   0x02     /* holdover activated */

typedef struct {
    uint32 state; /* the holdover flag */
    uint32 time;  /* the holdover time */
}PACK TkCtcHoldover;

typedef struct {
    uint32 raiseThreshold;
    uint32 clearThreshold;
} PACK CtcOamAlarmThreshold;

#define CTCALMSTATEDEACTIVED 0X01
#define CTCALMSTATEACTIVED   0x02

typedef struct{
    uint16 alarmId;
    uint32 alarmState;
}PACK CtcOamAlarmState;

void    TKCTCClearONUPONChipsSetInfo (void);

void    TKCTCExtOamFillMacForSN (uint8 * mac);

int32   TKCTCExtOamGetInfoFromONUPONChipsets (uint8 pathId, uint8 linkId,
                CtcInfoFromONUPONChipsSet * pCont);

int32   TKCTCExtOamGetDbaCfg (uint8 pathId, uint8 linkId, OamCtcDbaData * dba);

int32   TKCTCExtOamSetDbaCfg(uint8 pathId,uint8 linkId,OamCtcDbaData * dba); 

int32   TKCTCExtOamGetFecAbility (uint8 pathId, uint8 LinkId,
                uint32 * fecAbility);

int32   TKCTCExtOamNoneObjGetRaw (uint8 pathId, uint8 LinkId, uint8 branch,
                uint16 leaf, uint8 * pBuff, int32 * retLen);

int32   TKCTCExtOamNoneObjSetRaw (uint8 pathId, uint8 LinkId, uint8 branch,
                uint16 leaf, uint8 * pBuff, uint8 retLen, uint8 * reCode);

int32   TKCTCExtOamObjGetRaw (uint8 pathId, uint8 LinkId, uint8 objBranch,
                uint16 objLeaf, uint32 objIndex, uint8 branch,
                uint16 leaf, uint8 * pBuff, int32 * retLen);

int32   TKCTCExtOamObjSetRaw (uint8 pathId, uint8 LinkId, uint8 objBranch,
                uint16 objLeaf, uint32 objIndex, uint8 branch,
                uint16 leaf, uint8 * pBuff, uint8 retLen, uint8 * reCode);

int32   TKCTCExtOamGetONUSN (uint8 pathId, uint8 LinkId,
                CtcOamOnuSN * pCtcOamOnuSN);

int32   TKCTCExtOamGetFirmwareVersion (uint8 pathId, uint8 LinkId,
                CtcOamFirmwareVersion * pCtcOamFirmwareVersion, int32 * retLen);

int32   TKCTCExtOamGetChipsetId (uint8 pathId, uint8 LinkId,
                CtcOamChipsetId * pCtcOamChipsetId);

int32   TKCTCExtOamGetONUCap (uint8 pathId, uint8 LinkId, uint8 * pCap,
                int32 * retLen);

int32   TKCTCExtOamGetONUCap2 (uint8 pathId, uint8 LinkId, uint8 * pCap,
                int32 * retLen);

int32   TKCTCExtOamGetHoldOverConfig (uint8 pathId, uint8 LinkId, uint8 * pCap,
                int32 * retLen);

int32   TKCTCExtOamSetClsMarking (uint8 pathId, uint8 linkId, uint32 portNo,
                uint8 action, uint8 ruleCnt, CtcExtRule * pCtcExtRule);

int32   TKCTCExtOamSetLLIDQueueConfig (uint8 pathId, uint8 linkId, uint32 LLID,
                uint8 numOfQ, CtcExtQConfig * pCtcExtQConfig);

int32   TKCTCExtOamSetLaserTxPowerAdminCtl (uint8 pathId, uint8 linkId,
                CtcExtONUTxPowerSupplyControl * pCtcExtONUTxPowerSupplyControl);

int32   CtcExtOamSetMulLlidCtrl (uint8 pathId, uint32 llidNum);

int32   CtcExtOamSetHoldover (uint8 pathId, uint8 linkId, uint32 state, 
                uint32 time);

int32   CtcExtOamGetHoldover(uint8 pathId, uint8 linkId, uint32 *state, 
                uint32 *time);
	
int32   CtcExtOamGetAlarmState(uint8 pathId, uint8 linkId, uint16 port, 
                uint16 alarmid, uint8 *state);

int32   CtcExtOamSetAlarmState(uint8 pathId,uint8 linkId,  uint16 port,  
                uint16 alarmid, uint8 state);

int32   CtcExtOamGetAlarmThreshold(uint8 pathId,uint8 linkId, uint16 port, 
                uint16 alarmid, CtcOamAlarmThreshold *threshold);
	
int32   CtcExtOamSetAlarmThreshold(uint8 pathId,uint8 linkId, uint16 port,  
                uint16 alarmid, CtcOamAlarmThreshold threshold);	

int32   CtcExtOamGetOptTransDiag(uint8 path_id, uint8 link_id, 
                CtcOamTlvPowerMonDiag *info);

int32   CtcExtOamSetPonIfAdmin(uint8 path_id, uint8 link_id, uint8 optical_no);

int32   CtcExtOamGetPonIfAdmin(uint8 path_id, uint8 link_id, uint8 *optical_no);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_CtcMiscApi_H */
