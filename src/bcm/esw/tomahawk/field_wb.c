/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        field.c
 * Purpose:     Field Processor module routines specific to BCM56960
 *
 */
#include <bcm/error.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/tomahawk.h>
#include <soc/drv.h>
#include <soc/scache.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/tomahawk.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/trx.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#ifdef BCM_TOMAHAWK_SUPPORT

_field_dynamic_map_t dynamic_map;
extern uint8 em_action_profile_bitmap[17];
extern uint8 em_th2_action_profile_bitmap[23];
extern uint8 em_td3_action_profile_bitmap[37];
extern uint8 ifp_td3_action_profile_bitmap[44];

int
_field_wb_mirror_override_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_nat_override_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_sflow_set_recover(int unit,_field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_cut_thru_override_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_urpf_override_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_ttl_override_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_lb_controls_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_drop_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_change_cpu_cos_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_nat_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_mirror_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_copy_to_cpu_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_l3swl2_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_emredirect_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_counter_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_meter_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_green_to_pid_set_recover(int unit, _field_entry_t *f_ent,
                         uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_change_ecn_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_pkt_pri_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_dscp_tos_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_drop_precedence_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_cos_or_int_pri_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_int_cn_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_input_priority_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *act_data, _field_action_bmp_t *act_bmp);
int
_field_wb_instrumentation_set_recover(int unit, _field_entry_t *f_ent,
                                   uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_timestamp_set_recover(int unit, _field_entry_t *f_ent,
                                uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_dlb_hgt_lag_set_recover(int unit, _field_entry_t *f_ent,
                            uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_dlb_ecmp_set_recover(int unit, _field_entry_t *f_ent,
                             uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_dlb_alternate_path_control_set_recover(int unit, _field_entry_t *f_ent,
                                      uint32 *ebuf, _field_action_bmp_t *act_bmp);
int
_field_wb_protection_switching_set_recover(int unit, _field_entry_t *f_ent,
                             uint32 *ebuf, _field_action_bmp_t *act_bmp);

void* _field_wb_action_recovery_func_ptr[_FieldActionSetCount] = {
    NULL, /* _FieldActionSetNone */
    _field_wb_protection_switching_set_recover,
    /* _FieldActionProtectionSwitchingDropOverrideSet */
    _field_wb_dlb_alternate_path_control_set_recover,
    /* _FieldActionIfpDlbAlternatePathControlSet */
    _field_wb_dlb_ecmp_set_recover, /* _FieldActionEcmpDlbActionSet */
    _field_wb_dlb_hgt_lag_set_recover, /* _FieldActionHgtLagDlbActionSet */
    NULL, /* _FieldActionEcmp2RhActionSet */
    NULL, /* _FieldActionEcmp1RhActionSet */
    NULL, /* _FieldActionLagRhActionSet */
    NULL, /* _FieldActionHgtRhActionSet*/
    NULL, /* _FieldActionLoopbackProfileSet */
    NULL, /*  _FieldActionExtractionCtrlId */
    NULL, /*  _FieldActionOpaque4Set */
    NULL, /*  _FieldActionOpaque3Set */
    NULL, /*  _FieldActionOpaque2Set */
    NULL, /*  _FieldActionOpaque1Set */
    NULL, /*_FieldActionTxTimestampInsertionSet */
    NULL, /*_FieldActionRxTimestampInsertionSet */
    NULL, /*_FieldActionIgnoreFCOEZoneCheckSet */
    _field_wb_green_to_pid_set_recover,
    /*  _FieldActionGreenToPidSet */
    _field_wb_mirror_override_set_recover,
    /*  _FieldActionMirrorOverrideSet */
    _field_wb_nat_override_set_recover,
    /*  _FieldActionNatOverrideSet */
    _field_wb_sflow_set_recover,
    /*  _FieldActionSflowSet */
    _field_wb_cut_thru_override_set_recover,
    /*  _FieldActionCutThrOverrideSet */
    _field_wb_urpf_override_set_recover,
    /* _FieldActionUrpfOverrideSet */
    _field_wb_ttl_override_set_recover,
    /* _FieldActionTtlOverrideSet */
    _field_wb_lb_controls_set_recover,
    /* _FieldActionLbControlSet */
    _field_wb_drop_set_recover,
    /* _FieldActionDropSet */
    _field_wb_change_cpu_cos_set_recover,
    /* _FieldActionChangeCpuCosSet */
    _field_wb_mirror_set_recover,
    /* _FieldActionMirrorSet */
    _field_wb_nat_set_recover,
    /* _FieldActionNatSet */
    _field_wb_copy_to_cpu_recover,
    /* _FieldActionCopyToCpuSet */
    _field_wb_l3swl2_set_recover,
    /* _FieldActionL3SwChangeL2Set */
    _field_wb_emredirect_set_recover,
    /*  IFP  _FieldActionRedirectSet */
    _field_wb_counter_set_recover,
    /* _FieldActionCounterSet */
    _field_wb_change_ecn_set_recover,
    /* _FieldActionChangeEcnSet */
    _field_wb_pkt_pri_set_recover,
    /* _FieldActionChangePktPriSet */
    _field_wb_dscp_tos_set_recover,
    /* _FieldActionChangeDscpTosSet */
    _field_wb_drop_precedence_set_recover,
    /*_FieldActionChangeDropPrecendenceSet */
    _field_wb_cos_or_int_pri_set_recover,
    /* _FieldActionChangeCosOrIntPriSet */
    _field_wb_int_cn_set_recover,
    /* _FieldActionChangeIntCNSet */
    _field_wb_input_priority_set_recover,
    /* _FieldActionChangeInputPrioritySet */
    _field_wb_instrumentation_set_recover,
    /* _FieldActionInstrumentationSet */
    NULL, /* _FieldActionEditCtrlIdSet */
    NULL, /* _FieldActionFcoeVsanSet */
    _field_wb_meter_set_recover,
        /*_FieldActionMeterSet */
    _field_wb_timestamp_set_recover
    /*_FieldActionTimestampInsertionSet*/
};
int
_field_wb_action_profile_parse(int unit, _field_entry_t *f_ent, uint32 *ebuf,
                              uint32 *act_data,  _field_action_bmp_t *act_bmp);
char *elem_name[] = {
    "_bcmFieldInternalType",
    "_bcmFieldInternalExtractor",
    "_bcmFieldInternalGroup",
    "_bcmFieldInternalGroupId",
    "_bcmFieldInternalGroupPri",
    "_bcmFieldInternalGroupQset",
    "_bcmFieldInternalQsetW",
    "_bcmFieldInternalQsetUdfMap",
    "_bcmFieldInternalGroupFlags",
    "_bcmFieldInternalGroupPbmp",
    "_bcmFieldInternalGroupSlice",
    "_bcmFieldInternalGroupQual",
    "_bcmFieldInternalQualQid",
    "_bcmFieldInternalQualOffset",
    "_bcmFieldInternalQualSize",
    "_bcmFieldInternalGroupStage",
    "_bcmFieldInternalGroupEntry",
    "_bcmFieldInternalGroupBlockCount",
    "_bcmFieldInternalGroupGroupStatus",
    "_bcmFieldInternalGroupGroupAset",
    "_bcmFieldInternalGroupCounterBmp",
    "_bcmFieldInternalGroupInstance",
    "_bcmFieldInternalGroupExtCodes",
    "_bcmFieldInternalGroupLtSlice",
    "_bcmFieldInternalGroupLtConfig",
    "_bcmFieldInternalGroupLtEntry",
    "_bcmFieldInternalGroupLtEntrySize",
    "_bcmFieldInternalGroupLtEntryStatus",
    "_bcmFieldInternalLtStatusEntriesFree",
    "_bcmFieldInternalLtStatusEntriesTotal",
    "_bcmFieldInternalLtStatusEntriesCnt",
    "_bcmFieldInternalGroupQsetSize",
    "_bcmFieldInternalGroupHintId",
    "_bcmFieldInternalGroupMaxSize",
    "_bcmFieldInternalEndStructGroup",
    "_bcmFieldInternalQualOffsetField",
    "_bcmFieldInternalQualOffsetNumOffset",
    "_bcmFieldInternalQualOffsetOffsetArr",
    "_bcmFieldInternalQualOffsetWidth",
    "_bcmFieldInternalQualOffsetSec",
    "_bcmFieldInternalQualOffsetBitPos",
    "_bcmFieldInternalQualOffsetQualWidth",
    "_bcmFieldInternalGroupPartCount",
    "_bcmFieldInternalEntryEid",
    "_bcmFieldInternalEntryPrio",
    "_bcmFieldInternalEntrySliceId",
    "_bcmFieldInternalEntryFlagsPart1",
    "_bcmFieldInternalEntryFlagsPart2",
    "_bcmFieldInternalEntryFlagsPart3",
    "_bcmFieldInternalEntryPbmp",
    "_bcmFieldInternalEntryAction",
    "_bcmFieldInternalEntrySlice",
    "_bcmFieldInternalEntryGroup",
    "_bcmFieldInternalEntryStat",
    "_bcmFieldInternalEntryPolicer",
    "_bcmFieldInternalEntryIngMtp",
    "_bcmFieldInternalEntryEgrMtp",
    "_bcmFieldInternalEntryDvp",
    "_bcmFieldInternalEntryCopy",
    "_bcmFieldInternalEntryCopyTypePointer",
    "_bcmFieldInternalPbmpData",
    "_bcmFieldInternalPbmpMask",
    "_bcmFieldInternalEntryActionsPbmp",
    "_bcmFieldInternalActionParam",
    "_bcmFieldInternalGroupClassAct",
    "_bcmFieldInternalActionHwIdx",
    "_bcmFieldInternalActionFlags",
    "_bcmFieldInternalStatFlags",
    "_bcmFieldInternalPolicerFlags",
    "_bcmFieldInternalExtl1e32Sel",
    "_bcmFieldInternalExtl1e16Sel",
    "_bcmFieldInternalExtl1e8Sel",
    "_bcmFieldInternalExtl1e4Sel",
    "_bcmFieldInternalExtl1e2Sel",
    "_bcmFieldInternalExtl2e16Sel",
    "_bcmFieldInternalExtl3e1Sel",
    "_bcmFieldInternalExtl3e2Sel",
    "_bcmFieldInternalExtl3e4Sel",
    "_bcmFieldInternalExtPmuxSel",
    "_bcmFieldInternalExtIntraSlice",
    "_bcmFieldInternalExtSecondary",
    "_bcmFieldInternalExtIpbmpRes",
    "_bcmFieldInternalExtNorml3l4",
    "_bcmFieldInternalExtNormmac",
    "_bcmFieldInternalExtAuxTagaSel",
    "_bcmFieldInternalExtAuxTagbSel",
    "_bcmFieldInternalExtAuxTagcSel",
    "_bcmFieldInternalExtAuxTagdSel",
    "_bcmFieldInternalExtTcpFnSel",
    "_bcmFieldInternalExtTosFnSel",
    "_bcmFieldInternalExtTtlFnSel",
    "_bcmFieldInternalExtClassIdaSel",
    "_bcmFieldInternalExtClassIdbSel",
    "_bcmFieldInternalExtClassIdcSel",
    "_bcmFieldInternalExtClassIddSel",
    "_bcmFieldInternalExtSrcContaSel",
    "_bcmFieldInternalExtSrcContbSel",
    "_bcmFieldInternalExtSrcDestCont0Sel",
    "_bcmFieldInternalExtSrcDestCont1Sel",
    "_bcmFieldInternalExtKeygenIndex",
    "_bcmFieldInternalLtEntrySlice",
    "_bcmFieldInternalLtEntryIndex",
    "_bcmFieldInternalGroupCount",
    "_bcmFieldInternalSliceCount",
    "_bcmFieldInternalLtSliceCount",
    "_bcmFieldInternalEndStructGroupQual",
    "_bcmFieldInternalEndStructQualOffset",
    "_bcmFieldInternalEndStructEntryDetails",
    "_bcmFieldInternalEndStructEntry",
    "_bcmFieldInternalEntryPbmpData",
    "_bcmFieldInternalEntryPbmpMask",
    "_bcmFieldInternalEndStructAction",
    "_bcmFieldInternalGlobalEntryPolicer",
    "_bcmFieldInternalGlobalEntryPolicerPid",
    "_bcmFieldInternalGlobalEntryPolicerFlags",
    "_bcmFieldInternalEndGroupExtractor",
    "_bcmFieldInternalControl",
    "_bcmFieldInternalControlFlags",
    "_bcmFieldInternalStage",
    "_bcmFieldInternalControlGroups",
    "_bcmFieldInternalControlStages",
    "_bcmFieldInternalControlPolicerHash",
    "_bcmFieldInternalControlPolicerCount",
    "_bcmFieldInternalControlStathash",
    "_bcmFieldInternalControlStatCount",
    "_bcmFieldInternalControlHintBmp",
    "_bcmFieldInternalControlHintHash",
    "_bcmFieldInternalControlLastAllocatedLtId",
    "_bcmFieldInternalEndStructControl",
    "_bcmFieldInternalPolicerPid",
    "_bcmFieldInternalPolicerSwRefCount",
    "_bcmFieldInternalPolicerHwRefCount",
    "_bcmFieldInternalPolicerLevel",
    "_bcmFieldInternalPolicerPoolIndex",
    "_bcmFieldInternalPolicerHwIndex",
    "_bcmFieldInternalPolicerHwFlags",
    "_bcmFieldInternalPolicerStageId",
    "_bcmFieldInternalEndStructPolicer",
    "_bcmFieldInternalStatsId",
    "_bcmFieldInternalStatSwRefCount",
    "_bcmFieldInternalStatHwRefCount",
    "_bcmFieldInternalStatOffsetMode",
    "_bcmFieldInternalStatPoolIndex",
    "_bcmFieldInternalStatHwIndex",
    "_bcmFieldInternalStatHwFlags",
    "_bcmFieldInternalEndStructStage",
    "_bcmFieldInternalStatnStat",
    "_bcmFieldInternalStatArr",
    "_bcmFieldInternalStatHwStat",
    "_bcmFieldInternalStatHwMode",
    "_bcmFieldInternalStatHwEntryCount",
    "_bcmFieldInternalStatGid",
    "_bcmFieldInternalStatStageId",
    "_bcmFieldInternalStatStatValues",
    "_bcmFieldInternalStatFlexMode",
    "_bcmFieldInternalEndStructStat",
    "_bcmFieldInternalHintHintid",
    "_bcmFieldInternalHintHints",
    "_bcmFieldInternalHintsHinttype",
    "_bcmFieldInternalHintsQual",
    "_bcmFieldInternalHintsMaxValues",
    "_bcmFieldInternalHintsStartbit",
    "_bcmFieldInternalHintsEndbit",
    "_bcmFieldInternalHintsFlags",
    "_bcmFieldInternalHintsMaxGrpSize",
    "_bcmFieldInternalEndStructHints",
    "_bcmFieldInternalHintGrpRefCount",
    "_bcmFieldInternalHintCount",
    "_bcmFieldInternalEndStructHint",
    "_bcmFieldInternalPolicerCfgFlags",
    "_bcmFieldInternalPolicerCfgMode",
    "_bcmFieldInternalPolicerCfgCkbitsSec",
    "_bcmFieldInternalPolicerCfgMaxCkbitsSec",
    "_bcmFieldInternalPolicerCfgCkbitsBurst",
    "_bcmFieldInternalPolicerCfgPkbitsSec",
    "_bcmFieldInternalPolicerCfgMaxPkbitsSec",
    "_bcmFieldInternalPolicerCfgPkbitsBurst ",
    "_bcmFieldInternalPolicerCfgKbitsCurrent",
    "_bcmFieldInternalPolicerCfgActionId",
    "_bcmFieldInternalPolicerCfgSharingMode",
    "_bcmFieldInternalPolicerCfgEntropyId",
    "_bcmFieldInternalPolicerCfgPoolId",
    "_bcmFieldInternalControlEndStructUdf",
    "_bcmFieldInternalHintHintsHintType",
    "_bcmFieldInternalHintHintsQual",
    "_bcmFieldInternalHintHintsMaxValues",
    "_bcmFieldInternalHintHintsStartBit",
    "_bcmFieldInternalHintHintsEndBit",
    "_bcmFieldInternalHintHintsFlags",
    "_bcmFieldInternalHintHintsMaxGrpSize",
    "_bcmFieldInternalEndStructHintHints",
    "_bcmFieldInternalStageStageid",
    "_bcmFieldInternalStageFlags",
    "_bcmFieldInternalStageTcamSz",
    "_bcmFieldInternalStageTcamSlices",
    "_bcmFieldInternalStageNumInstances",
    "_bcmFieldInternalStageNumPipes",
    "_bcmFieldInternalStageRanges",
    "_bcmFieldInternalStageRangeId",
    "_bcmFieldInternalStageNumMeterPools",
    "_bcmFieldInternalStageMeterPool",
    "_bcmFieldInternalStageNumCntrPools",
    "_bcmFieldInternalStageCntrPools",
    "_bcmFieldInternalStageLtTcamSz",
    "_bcmFieldInternalStageNumLogicalTables",
    "_bcmFieldInternalStageLtInfo",
    "_bcmFieldInternalStageExtLevels",
    "_bcmFieldInternalStageOperMode",
    "_bcmFieldInternalRangeFlags",
    "_bcmFieldInternalRangeRid",
    "_bcmFieldInternalRangeMin",
    "_bcmFieldInternalRangeMax",
    "_bcmFieldInternalRangeHwIndex",
    "_bcmFieldInternalRangeStyle",
    "_bcmFieldInternalMeterLevel",
    "_bcmFieldInternalMeterSliceId",
    "_bcmFieldInternalMeterSize",
    "_bcmFieldInternalMeterPoolSize",
    "_bcmFieldInternalMeterFreeMeters",
    "_bcmFieldInternalMeterNumMeterPairs",
    "_bcmFieldInternalMeterBmp",
    "_bcmFieldInternalCntrSliceId",
    "_bcmFieldInternalCntrSize",
    "_bcmFieldInternalCntrFreeCntrs",
    "_bcmFieldInternalCntrBmp",
    "_bcmFieldInternalLtConfigValid",
    "_bcmFieldInternalLtConfigLtId",
    "_bcmFieldInternalLtConfigLtPartPri",
    "_bcmFieldInternalLtConfigLtPartMap",
    "_bcmFieldInternalLtConfigLtActionPri",
    "_bcmFieldInternalLtConfigPri",
    "_bcmFieldInternalLtConfigFlags",
    "_bcmFieldInternalLtConfigEntry",
    "_bcmFieldInternalEndStructRanges",
    "_bcmFieldInternalEndStructMeter",
    "_bcmFieldInternalEndStructCntr",
    "_bcmFieldInternalEndStructLtConfig",
    "_bcmFieldInternalSlice",
    "_bcmFieldInternalSliceStartTcamIdx",
    "_bcmFieldInternalSliceNumber",
    "_bcmFieldInternalSliceEntryCount",
    "_bcmFieldInternalSliceFreeCount",
    "_bcmFieldInternalSliceCountersCount",
    "_bcmFieldInternalSliceMetersCount",
    "_bcmFieldInternalSliceInstalledEntriesCount",
    "_bcmFieldInternalSliceCounterBmp",
    "_bcmFieldInternalSliceMeterBmp",
    "_bcmFieldInternalSliceStageId",
    "_bcmFieldInternalSlicePortPbmp",
    "_bcmFieldInternalSliceEntriesInfo",
    "_bcmFieldInternalSliceNextSlice",
    "_bcmFieldInternalSlicePrevSlice",
    "_bcmFieldInternalSliceFlags",
    "_bcmFieldInternalSliceGroupFlags",
    "_bcmFieldInternalSliceLtMap",
    "_bcmFieldInternalEndStructSlice",
    "_bcmFieldInternalEndStructExtractor",
    "_bcmFieldInternalLtSliceSliceNum",
    "_bcmFieldInternalLtSliceStartTcamIdx",
    "_bcmFieldInternalLtSliceEntryCount",
    "_bcmFieldInternalLtSliceFreeCount",
    "_bcmFieldInternalLtSliceStageid",
    "_bcmFieldInternalLtSliceEntryinfo",
    "_bcmFieldInternalLtSliceNextSlice",
    "_bcmFieldInternalLtSlicePrevSlice",
    "_bcmFieldInternalLtSliceFlags",
    "_bcmFieldInternalLtSliceGroupFlags",
    "_bcmFieldInternalDataControlStart",
    "_bcmFieldInternalDataControlUsageBmp",
    "_bcmFieldInternalDataControlDataQualStruct",
    "_bcmFieldInternalDataControlDataQualQid",
    "_bcmFieldInternalDataControlDataQualUdfSpec",
    "_bcmFieldInternalDataControlDataQualOffsetBase",
    "_bcmFieldInternalDataControlDataQualOffset",
    "_bcmFieldInternalDataControlDataQualByteOffset",
    "_bcmFieldInternalDataControlDataQualHwBmp",
    "_bcmFieldInternalDataControlDataQualFlags",
    "_bcmFieldInternalDataControlDataQualElemCount",
    "_bcmFieldInternalDataControlDataQualLength",
    "_bcmFieldInternalDataControlEndStructDataQual",
    "_bcmFieldInternalDataControlEthertypeStruct",
    "_bcmFieldInternalDataControlEthertypeRefCount",
    "_bcmFieldInternalDataControlEthertypeL2",
    "_bcmFieldInternalDataControlEthertypeVlanTag",
    "_bcmFieldInternalDataControlEthertypePortEt",
    "_bcmFieldInternalDataControlEthertypeRelOffset",
    "_bcmFieldInternalDataControlProtStart",
    "_bcmFieldInternalDataControlProtIp4RefCount",
    "_bcmFieldInternalDataControlProtIp6RefCount",
    "_bcmFieldInternalDataControlProtFlags",
    "_bcmFieldInternalDataControlProtIp",
    "_bcmFieldInternalDataControlProtL2",
    "_bcmFieldInternalDataControlProtVlanTag",
    "_bcmFieldInternalDataControlProtRelOffset",
    "_bcmFieldInternalDataControlTcamStruct",
    "_bcmFieldInternalDataControlTcamRefCount",
    "_bcmFieldInternalDataControlTcamPriority",
    "_bcmFieldInternalDataControlElemSize",
    "_bcmFieldInternalDataControlNumElem",
    "_bcmFieldInternalEndStructDataControl",
    "_bcmFieldInternalControlUdfValid",
    "_bcmFieldInternalControlUdfUseCount",
    "_bcmFieldInternalControlUdfNum",
    "_bcmFieldInternalControlUdfUserNum",
    "_bcmFieldInternalControlUdfDetails",
    "_bcmFieldInternalDataControlEndStructEtype",
    "_bcmFieldInternalDataControlEndStructProt",
    "_bcmFieldInternalDataControlEndStructTcam",
    "_bcmFieldInternalEntryStatSid",
    "_bcmFieldInternalEntryStatExtendedSid",
    "_bcmFieldInternalEntryStatFlags",
    "_bcmFieldInternalEntryStatAction",
    "_bcmFieldInternalEndStructLtEntryDetails",
    "_bcmFieldInternalSliceLtPartitionPri",
    "_bcmFieldInternalEntryPolicerPid",
    "_bcmFieldInternalEntryPolicerFlags",
    "_bcmFieldInternalEndStructEntPolicer",
    "_bcmFieldInternalEndStructIFP",
    "_bcmFieldInternalStageClass",
    "_bcmFieldInternalClassFlags",
    "_bcmFieldInternalClassEntUsed",
    "_bcmFieldInternalClassBmp",
    "_bcmFieldInternalEndStructClass",
    "_bcmFieldInternalClassOperMode",
    "_bcmFieldInternalEntryTcamIptype",
    "_bcmFieldInternalEndStageClass",
    "_bcmFieldInternalEntryPbmpFullData",
    "_bcmFieldInternalEntryPbmpFullMask",
    "_bcmFieldInternalStageLtActionPri",
    "_bcmFieldInternalStagePreselQset",
    "_bcmFieldInternalControlPreselInfo",
    "_bcmFieldInternalGroupPreselQual",
    "_bcmFieldInternalPreselId",
    "_bcmFieldInternalPreselFlags",
    "_bcmFieldInternalPreselPri",
    "_bcmFieldInternalPreselHwIndex",
    "_bcmFieldInternalPreselSliceIndex",
    "_bcmFieldInternalPreselTcamIndex",
    "_bcmFieldInternalPreselClassAction",
    "_bcmFieldInternalPreselLtDataSize",
    "_bcmFieldInternalPreselQset",
    "_bcmFieldInternalEndStructPreselInfo",
    "_bcmFieldInternalPreselInfoLimit",
    "_bcmFieldInternalPreselLastAllocatedId",
    "_bcmFieldInternalPreselSet",
    "_bcmFieldInternalPreselOperationalSet",
    "_bcmFieldInternalPreselKeySize",
    "_bcmFieldInternalPreselEntrySlice",
    "_bcmFieldInternalPreselEntryGroup",
    "_bcmFieldInternalEMOperMode",
    "_bcmFieldInternalEndStageEM",
    "_bcmFieldInternalEMEntryarr",
    "_bcmFieldInternalEMGroupMode",
    "_bcmFieldInternalEMGroupAset",
    "_bcmFieldInternalPreselPbmp",
    "_bcmFieldInternalQualOffsetQualWidth16",
    "_bcmFieldInternalStatBmp",
    "_bcmFieldInternalQsetQual",
    "_bcmFieldInternalQsetUdf",
    "_bcmFieldInternalQualifyCount",
    "_bcmFieldInternalHintHintsPri",
    "_bcmFieldInternalEMGroupAset2",
    "_bcmFieldInternalEntrySvpType",
    "_bcmFieldInternalActionParam1",
    "_bcmFieldInternalGroupActionProfIdx",
    "_bcmfieldinternalElementCount",
};

/*
 *  Elements mapped to their attributes. Attributes cannot be modified.
 *  If there is a change in attributes for any of the elements, new
 *  element type _bcm_field_internal_element_t has to be created and
 *  then needs to be added here with updated attributes
 */
static const _field_type_map_t static_type_map[] = {
    { _bcmFieldInternalType,                4,  0x0},
    { _bcmFieldInternalExtractor,           4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroup,               0,  _FP_WB_TLV_NO_VALUE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroupId,             4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupPri,            4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupQset,           4, _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalQsetW,               4,  _FP_WB_TLV_LEN_PRESENT
                                                | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalQsetUdfMap,          4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroupFlags,          2,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupPbmp,           1,  _FP_WB_TLV_CLASS
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroupSlice,          4,  0x0},
    { _bcmFieldInternalGroupQual,           4, _FP_WB_TLV_NO_VALUE
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalQualQid,             4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalQualOffset,          4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalQualSize,            4,  0x0},
    { _bcmFieldInternalGroupStage,          4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupEntry,          4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupBlockCount,     2,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupGroupStatus,    4,  _FP_WB_TLV_CLASS
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroupGroupAset,      4,  _FP_WB_TLV_LEN_PRESENT
                                                | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupCounterBmp,     4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroupInstance,       4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupExtCodes,       4,  _FP_WB_TLV_NO_VALUE
                                                | _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroupLtSlice,        4,  0x0},
    { _bcmFieldInternalGroupLtConfig,       4,  0x0},
    { _bcmFieldInternalGroupLtEntry,        4,  0x0},
    { _bcmFieldInternalGroupLtEntrySize,    2,  0x0},
    { _bcmFieldInternalGroupLtEntryStatus,  4,  _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalLtStatusEntriesFree, 4,  0x0},
    { _bcmFieldInternalLtStatusEntriesTotal,4,  0x0},
    { _bcmFieldInternalLtStatusEntriesCnt,  4,  0x0},
    { _bcmFieldInternalGroupQsetSize,       4,  0x0},
    { _bcmFieldInternalGroupHintId,         4,  0x0},
    { _bcmFieldInternalGroupMaxSize,        4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEndStructGroup,      4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalQualOffsetField,     4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalQualOffsetNumOffset, 1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalQualOffsetOffsetArr, 2,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalQualOffsetWidth,     1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalQualOffsetSec,       1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalQualOffsetBitPos,    1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalQualOffsetQualWidth, 1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalGroupPartCount,      4,  0x0},
    { _bcmFieldInternalEntryEid,            4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryPrio,           4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntrySliceId,        4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryFlagsPart1,     4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryFlagsPart2,     4,  0x0},
    { _bcmFieldInternalEntryFlagsPart3,     4,  0x0},
    { _bcmFieldInternalEntryPbmp,           4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryAction,         4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEntrySlice,          4,  0x0},
    { _bcmFieldInternalEntryGroup,          4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryStat,           4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEntryPolicer,        4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEntryIngMtp,         1,  0x0},
    { _bcmFieldInternalEntryEgrMtp,         1,  0x0},
    { _bcmFieldInternalEntryDvp,            1,  0x0},
    { _bcmFieldInternalEntryCopy,           1,  _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalEntryCopyTypePointer,0,  0x0},
    { _bcmFieldInternalPbmpData,            4,  0x0},
    { _bcmFieldInternalPbmpMask,            4,  0x0},
    { _bcmFieldInternalEntryActionsPbmp,    4,  _FP_WB_TLV_LEN_PRESENT
                                                | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalActionParam,         4,  _FP_WB_TLV_LEN_PRESENT
                                                | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupClassAct,       1,  0x0},
    { _bcmFieldInternalActionHwIdx,         4,  _FP_WB_TLV_LEN_PRESENT
                                                | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalActionFlags,         1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatFlags,           1,  0x0},
    { _bcmFieldInternalPolicerFlags,        1,  0x0},
    { _bcmFieldInternalExtl1e32Sel,         1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl1e16Sel,         1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl1e8Sel,          1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl1e4Sel,          1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl1e2Sel,          1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl2e16Sel,         1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl3e1Sel,          1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl3e2Sel,          1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtl3e4Sel,          1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtPmuxSel,          1,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalExtIntraSlice,       1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtSecondary,        1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtIpbmpRes,         1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtNorml3l4,         1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtNormmac,          1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtAuxTagaSel,       1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtAuxTagbSel,       1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtAuxTagcSel,       1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtAuxTagdSel,       1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtTcpFnSel,         1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtTosFnSel,         1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtTtlFnSel,         1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtClassIdaSel,      1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtClassIdbSel,      1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtClassIdcSel,      1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtClassIddSel,      1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtSrcContaSel,      1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtSrcContbSel,      1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtSrcDestCont0Sel,  1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtSrcDestCont1Sel,  1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalExtKeygenIndex,      4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtEntrySlice,        4,  0x0},
    { _bcmFieldInternalLtEntryIndex,        4,  0x0},
    { _bcmFieldInternalGroupCount,          4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalSliceCount,          4,  0x0},
    { _bcmFieldInternalLtSliceCount,        4,  0x0},
    { _bcmFieldInternalEndStructGroupQual,  4,  0x0},
    { _bcmFieldInternalEndStructQualOffset, 4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructEntryDetails,4, _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalEndStructEntry,      4,  _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryPbmpData,       4,  0x0},
    { _bcmFieldInternalEntryPbmpMask,       4,  0x0},
    { _bcmFieldInternalEndStructAction,     4,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalGlobalEntryPolicer,  4,  0x0},
    { _bcmFieldInternalGlobalEntryPolicerPid,4, 0x0},
    { _bcmFieldInternalGlobalEntryPolicerFlags,1,0x0},
    { _bcmFieldInternalEndGroupExtractor,   4,  0x0},
    { _bcmFieldInternalControl,             4,  _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalControlFlags,        1,  0x0},
    { _bcmFieldInternalStage,               4,  _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalControlGroups,       4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalControlStages,       4,  0x0},
    { _bcmFieldInternalControlPolicerHash,  4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalControlPolicerCount, 4,  0x0},
    { _bcmFieldInternalControlStathash,     4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalControlStatCount,    4,  0x0},
    { _bcmFieldInternalControlHintBmp,      1,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalControlHintHash,     4,  _FP_WB_TLV_NO_VALUE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalControlLastAllocatedLtId,4,0x0},
    { _bcmFieldInternalEndStructControl,    4,  0x0},
    { _bcmFieldInternalPolicerPid,          4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerSwRefCount,   2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerHwRefCount,   2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerLevel,        1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerPoolIndex,    1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerHwIndex,      4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerHwFlags,      4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerStageId,      4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructPolicer,    4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatsId,             4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatSwRefCount,      2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatHwRefCount,      2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatOffsetMode,      1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatPoolIndex,       1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatHwIndex,         4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatHwFlags,         4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructStage,      4,  0x0},
    { _bcmFieldInternalStatnStat,           1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatArr,             4,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalStatHwStat,          4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatHwMode,          2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatHwEntryCount,    1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatGid,             4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatStageId,         4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatStatValues,      8,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalStatFlexMode,        4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructStat,       4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalHintHintid,          4,  0x0},
    { _bcmFieldInternalHintHints,           0,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalHintsHinttype,       4,  0x0},
    { _bcmFieldInternalHintsQual,           4,  0x0},
    { _bcmFieldInternalHintsMaxValues,      4,  0x0},
    { _bcmFieldInternalHintsStartbit,       4,  0x0},
    { _bcmFieldInternalHintsEndbit,         4,  0x0},
    { _bcmFieldInternalHintsFlags,          4,  0x0},
    { _bcmFieldInternalHintsMaxGrpSize,     4,  0x0},
    { _bcmFieldInternalEndStructHints,      4,  0x0},
    { _bcmFieldInternalHintGrpRefCount,     2,  0x0},
    { _bcmFieldInternalHintCount,           2,  0x0},
    { _bcmFieldInternalEndStructHint,       4,  0x0},
    { _bcmFieldInternalPolicerCfgFlags,     4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgMode,      4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgCkbitsSec, 4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgMaxCkbitsSec,4,_FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgCkbitsBurst,4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgPkbitsSec, 4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgMaxPkbitsSec,4,_FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgPkbitsBurst,4,_FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgKbitsCurrent,4,_FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgActionId,  4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgSharingMode,4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgEntropyId, 4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPolicerCfgPoolId,    4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalControlEndStructUdf, 4,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalHintHintsHintType,   4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalHintHintsQual,       4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalHintHintsMaxValues,  4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalHintHintsStartBit,   4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalHintHintsEndBit,     4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalHintHintsFlags,      4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalHintHintsMaxGrpSize, 4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructHintHints,  4,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalStageStageid,        4,  0x0},
    { _bcmFieldInternalStageFlags,          4,  0x0},
    { _bcmFieldInternalStageTcamSz,         4,  0x0},
    { _bcmFieldInternalStageTcamSlices,     4,  0x0},
    { _bcmFieldInternalStageNumInstances,   4,  0x0},
    { _bcmFieldInternalStageNumPipes,       4,  0x0},
    { _bcmFieldInternalStageRanges,         4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalStageRangeId,        4,  0x0},
    { _bcmFieldInternalStageNumMeterPools,  4,  0x0},
    { _bcmFieldInternalStageMeterPool,      4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalStageNumCntrPools,   4,  0x0},
    { _bcmFieldInternalStageCntrPools,      4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalStageLtTcamSz,       4,  0x0},
    { _bcmFieldInternalStageNumLogicalTables,4, 0x0},
    { _bcmFieldInternalStageLtInfo,         4,  _FP_WB_TLV_LEN_ENCODED
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalStageExtLevels,      4,  0x0},
    { _bcmFieldInternalStageOperMode,       4,  0x0},
    { _bcmFieldInternalRangeFlags,          4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalRangeRid,            4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalRangeMin,            4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalRangeMax,            4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalRangeHwIndex,        4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalRangeStyle,          1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalMeterLevel,          1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalMeterSliceId,        4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalMeterSize,           2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalMeterPoolSize,       2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalMeterFreeMeters,     2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalMeterNumMeterPairs,  2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalMeterBmp,            4,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalCntrSliceId,         1,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalCntrSize,            2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalCntrFreeCntrs,       2,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalCntrBmp,             4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtConfigValid,       4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtConfigLtId,        4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtConfigLtPartPri,   4,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalLtConfigLtPartMap,   4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtConfigLtActionPri, 4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtConfigPri,         4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtConfigFlags,       4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalLtConfigEntry,       4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructRanges,     4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructMeter,      4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructCntr,       4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructLtConfig,   4,  _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalSlice,               0,  _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalSliceStartTcamIdx,   4,  0x0},
    { _bcmFieldInternalSliceNumber,         1,  0x0 },
    { _bcmFieldInternalSliceEntryCount,     4,  0x0},
    { _bcmFieldInternalSliceFreeCount,      4,  0x0},
    { _bcmFieldInternalSliceCountersCount,  4,  0x0},
    { _bcmFieldInternalSliceMetersCount,    4,  0x0},
    { _bcmFieldInternalSliceInstalledEntriesCount, 4, 0x0},
    { _bcmFieldInternalSliceCounterBmp,     4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalSliceMeterBmp,       4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalSliceStageId,        4,  0x0},
    { _bcmFieldInternalSlicePortPbmp,       1,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalSliceEntriesInfo,    4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalSliceNextSlice,      4,  0x0},
    { _bcmFieldInternalSlicePrevSlice,      4,  0x0},
    { _bcmFieldInternalSliceFlags,          1,  0x0},
    { _bcmFieldInternalSliceGroupFlags,     1,  0x0},
    { _bcmFieldInternalSliceLtMap,          4,  0x0},
    { _bcmFieldInternalEndStructSlice,      4,  0x0},
    { _bcmFieldInternalEndStructExtractor,  4,  _FP_WB_TLV_NO_TYPE
                                                | _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalLtSliceSliceNum,     1,  0x0},
    { _bcmFieldInternalLtSliceStartTcamIdx, 4,  0x0},
    { _bcmFieldInternalLtSliceEntryCount,   4,  0x0},
    { _bcmFieldInternalLtSliceFreeCount,    4,  0x0},
    { _bcmFieldInternalLtSliceStageid,      4,  0x0},
    { _bcmFieldInternalLtSliceEntryinfo,    4,  0x0},
    { _bcmFieldInternalLtSliceNextSlice,    4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalLtSlicePrevSlice,    4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalLtSliceFlags,        2,  0x0},
    { _bcmFieldInternalLtSliceGroupFlags,   2,  0x0},
    { _bcmFieldInternalDataControlStart,    0,  _FP_WB_TLV_NO_VALUE
                                                | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlUsageBmp,         4, 0x0},
    { _bcmFieldInternalDataControlDataQualStruct,   0, _FP_WB_TLV_NO_VALUE
                                                      | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlDataQualQid,      4, 0x0},
    { _bcmFieldInternalDataControlDataQualUdfSpec,  4, _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlDataQualOffsetBase,4,0x0},
    { _bcmFieldInternalDataControlDataQualOffset,   4, 0x0},
    { _bcmFieldInternalDataControlDataQualByteOffset,1,0x0},
    { _bcmFieldInternalDataControlDataQualHwBmp,    4, 0x0},
    { _bcmFieldInternalDataControlDataQualFlags,    4, 0x0},
    { _bcmFieldInternalDataControlDataQualElemCount,1, 0x0},
    { _bcmFieldInternalDataControlDataQualLength,   4, 0x0},
    { _bcmFieldInternalDataControlEndStructDataQual,4, _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlEthertypeStruct,  0, _FP_WB_TLV_LEN_ENCODED
                                                      | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlEthertypeRefCount,4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlEthertypeL2,      2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlEthertypeVlanTag, 2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlEthertypePortEt,  2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlEthertypeRelOffset,4,_FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlProtStart,        0, _FP_WB_TLV_LEN_ENCODED
                                                      | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlProtIp4RefCount,  4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlProtIp6RefCount,  4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlProtFlags,        4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlProtIp,           1, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlProtL2,           2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlProtVlanTag,      2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlProtRelOffset,    4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlTcamStruct,       4, _FP_WB_TLV_LEN_ENCODED
                                                       |_FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlTcamRefCount,     1,   _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlTcamPriority,     1,   _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalDataControlElemSize,         4, 0x0},
    { _bcmFieldInternalDataControlNumElem,          4, 0x0},
    { _bcmFieldInternalEndStructDataControl,        4, 0x0},
    { _bcmFieldInternalControlUdfValid,             1, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalControlUdfUseCount,          4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalControlUdfNum,               4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalControlUdfUserNum,           1, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalControlUdfDetails,           0, _FP_WB_TLV_LEN_ENCODED
                                                      | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlEndStructEtype,   4, _FP_WB_TLV_NO_TYPE
                                                      | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlEndStructProt,    4, _FP_WB_TLV_NO_TYPE
                                                      | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalDataControlEndStructTcam,    4, _FP_WB_TLV_NO_TYPE
                                                      | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEntryStatSid,        4, 0},
    { _bcmFieldInternalEntryStatExtendedSid,4, 0},
    { _bcmFieldInternalEntryStatFlags,      2, 0},
    { _bcmFieldInternalEntryStatAction,     4, 0},
    { _bcmFieldInternalEndStructLtEntryDetails, 4, _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalSliceLtPartitionPri, 1, 0x0},
    { _bcmFieldInternalEntryPolicerPid,     4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEntryPolicerFlags,   1, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEndStructEntPolicer, 0, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalEndStructIFP,        0, 0x0},
    { _bcmFieldInternalStageClass,          0, _FP_WB_TLV_LEN_ENCODED
                                               | _FP_WB_TLV_LEN_PRESENT
                                               | _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalClassFlags,          4, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalClassEntUsed,        4, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalClassBmp,            4, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_LEN_PRESENT
                                               | _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEndStructClass,      4, _FP_WB_TLV_NO_TYPE
                                               |_FP_WB_TLV_NO_VALUE
                                               | _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalClassOperMode,       4, _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryTcamIptype,     1, _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEndStageClass,       4, _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalEntryPbmpFullData,   4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEntryPbmpFullMask,   4,  _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalStageLtActionPri,    4, 0x0},
    { _bcmFieldInternalStagePreselQset,     4, _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalControlPreselInfo,   0, _FP_WB_TLV_LEN_ENCODED
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalGroupPreselQual,     0, _FP_WB_TLV_NO_VALUE
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalPreselId,            4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselFlags,         4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselPri,           4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselHwIndex,       4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselSliceIndex,    4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselTcamIndex,     4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselClassAction,   4, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalPreselLtDataSize,    2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselQset,          4, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEndStructPreselInfo, 4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselInfoLimit,     4, 0x0},
    { _bcmFieldInternalPreselLastAllocatedId, 4, 0x0},
    { _bcmFieldInternalPreselSet,           4, _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalPreselOperationalSet,4, _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalPreselKeySize,       2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalPreselEntrySlice,    1, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalPreselEntryGroup,    4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEMOperMode,          4, _FP_WB_TLV_NO_INGRESS},
    { _bcmFieldInternalEndStageEM,          4, _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_NO_VALUE},
    { _bcmFieldInternalEMEntryarr,          4, _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEMGroupMode,         4, _FP_WB_TLV_NO_INGRESS},
    { _bcmFieldInternalEMGroupAset,         1, _FP_WB_TLV_NO_INGRESS},
    { _bcmFieldInternalPreselPbmp,          4, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalQualOffsetQualWidth16, 2, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalStatBmp,             4, _FP_WB_TLV_NO_TYPE
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalQsetQual,            4, _FP_WB_TLV_LEN_PRESENT
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalQsetUdf,             4, _FP_WB_TLV_LEN_PRESENT
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalQualifyCount,        4, _FP_WB_TLV_CLASS},
    { _bcmFieldInternalHintHintsPri,        4, _FP_WB_TLV_NO_TYPE},
    { _bcmFieldInternalEMGroupAset2,        4, _FP_WB_TLV_NO_INGRESS
                                               | _FP_WB_TLV_LEN_PRESENT},
    { _bcmFieldInternalEntrySvpType,        1, 0x0},
    { _bcmFieldInternalActionParam1,        4, _FP_WB_TLV_LEN_PRESENT
                                               | _FP_WB_TLV_CLASS},
    { _bcmFieldInternalGroupActionProfIdx,  1, _FP_WB_TLV_LEN_PRESENT},
};

/* Static map used for Downgrades */
_field_type_map_t *recovery_type_map[BCM_MAX_NUM_UNITS];

uint8 downgrade[BCM_MAX_NUM_UNITS] = {FALSE};

#define BCM_IF_ERROR_CLEANUP(rv)            \
    if (rv < 0) {                           \
        goto cleanup;                       \
    }

#define _DYNAMIC_MAP_INSERT( _ptr_, _node_, _tail_)     \
    do {                                                \
        if (_tail_ == NULL) {                            \
            _ptr_ = _node_;                             \
            _tail_ = _node_;                            \
        } else {                                        \
            _tail_->next = _node_;                      \
            _tail_ = _tail_->next;                      \
        }                                               \
    } while(0);

#define _DYNAMIC_MAP_DELETE_ALL(_ptr_, _entry_type_) \
    do {                                             \
        _entry_type_ *node = NULL;                   \
        while (_ptr_ != NULL) {                      \
            node = _ptr_;                            \
            _ptr_ = _ptr_->next;                     \
            sal_free(node);                          \
            node = NULL;                             \
        }                                            \
    } while(0);                                      \

void dynamic_map_free(_field_dynamic_map_t dynamic_map) {
    _DYNAMIC_MAP_DELETE_ALL(dynamic_map.entry_map, _field_dynamic_map_entry_t);
    _DYNAMIC_MAP_DELETE_ALL(dynamic_map.lt_entry_map,
                            _field_dynamic_map_lt_entry_t);
}

_bcm_field_mirror_actions_recover_t *em_mirror_actions = NULL;
/*
 * Function:
 *      _tlv_print_array
 * Purpose:
 *      print the array of values
 * Parameters:
 *      unit   - (IN) StrataSwitch unit #.
 *      value  - (IN) Pointer to the array of values
 *      length - (IN) length of the array.
 *   Returns:
 *      BCM_E_XXX
 */
int
_tlv_print_array(int unit, void *value, int length, int chunk_sz)
{
    int i         = 0;     /* local variable for loop */
    uint8 *val8   = 0;     /* variable to hold 8 bit values */
    uint16 *val16 = 0;     /* variable to hold 16 bit values */
    uint32 *val32 = 0;     /* variable to hold 32 bit values */
    long long unsigned int *val64 = 0;
                           /* variable to hold 64 bit values */

    switch (chunk_sz) {
        case 2:
             val16 = (uint16 *)value;
             for (i = 0; i < length/chunk_sz; i++) {
                  LOG_DEBUG(BSL_LS_BCM_FP,
                   (BSL_META_U(unit, "TLV Value array: %x\n"), val16[i]));
             }
             break;
        case 4:
             val32 = (uint32 *)value;
              for (i = 0; i < length/chunk_sz; i++) {
                  LOG_DEBUG(BSL_LS_BCM_FP,
                   (BSL_META_U(unit, "TLV Value array: %x\n"), val32[i]));
              }
              break;
        case 8:
             val64 = (long long unsigned int *)value;
              for (i = 0; i < length/chunk_sz; i++) {
                  LOG_DEBUG(BSL_LS_BCM_FP,
                   (BSL_META_U(unit, "TLV Value array: %llx\n"), val64[i]));
              }
              break;
        default:
                val8 = (uint8 *)value;
                for (i = 0; i < length; i++) {
                    LOG_DEBUG(BSL_LS_BCM_FP,
                       (BSL_META_U(unit, "TLV Value array: %02x\n"), val8[i]));
                }


    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      tlv_write
 * Purpose:
 *      Element stored in scache =_internal_element_t | ((array/variable) << 28)
 *      if (_FP_WB_TLV_LEN_PRESENT) -> write length to scache
 *      if !(_FP_WB_TLV_NO_VALUE)
 *          -> if variable -> get size from static map and write
 *          -> if array -> length times (size of each component from static map)
 *      if length was encoded, types will be added later as variables with
 *      _bcmfieldinternaltype as element type where we write only a uint32(which
 *      is considered as value for the main type. If more types are added, it
 *      will be a array
 *      Will add an example here later about encoded.
 * Parameters:
 *      unit - (IN) StrataSwitch unit #.
 *      tlv  - (IN) input structure containing type to be written,
                    its length & value
 *      ptr  - (IN) pointer to scache; obtained from
                    field_control->scache_ptr[PART];
 *      pos  - (IN/OUT) Current offset from the base of scache part in use.
 *   Returns:
 *      BCM_E_XXX
 */

int
tlv_write(int unit, _field_tlv_t *tlv, uint8 *ptr, uint32 *position)
{
    uint32 pos, data               = 0;  /* local variables for
                                              * scache_pos and data
                                              */
    _bcm_field_internal_element_t elem;  /* local variable for elem type */
    int write_size = 0;                  /* size used for value field */

    if (tlv == NULL) {
        return BCM_E_PARAM;
    }
    pos = *position;

    elem = tlv->type;
    /* Write Type */
    if (!(static_type_map[elem].flags & _FP_WB_TLV_NO_TYPE))
    {   data = (tlv->basic_type <<  _FP_WB_TLV_BASIC_TYPE_SHIFT);
        data |= elem;
        LOG_DEBUG(BSL_LS_BCM_FP, (BSL_META_U(unit, "TLV Write Enum: %s\n"),
                                             elem_name[elem]));
        if (wb_write_disable != 1) {
            sal_memcpy(&ptr[pos], &data, sizeof(uint32));
        }
        pos += sizeof(uint32);
    } else {
        LOG_DEBUG(BSL_LS_BCM_FP, (BSL_META_U(unit, "Writing For Enum: %s\n"),
                  elem_name[tlv->type]));
    }
    /* Write Length */
    if (static_type_map[elem].flags & _FP_WB_TLV_LEN_PRESENT) {
        LOG_DEBUG(BSL_LS_BCM_FP, (BSL_META_U(unit, "TLV Write Length: %x\n"),
                                             tlv->length));
        if (wb_write_disable != 1) {
            sal_memcpy(&ptr[pos], &(tlv->length), sizeof(uint32));
        }
        pos += sizeof(uint32);
    }

    if ((tlv->value != NULL) &&
        !(static_type_map[elem].flags &
            _FP_WB_TLV_NO_VALUE)) {
            if (tlv->basic_type == _bcmFieldInternalVariable) {
                write_size = static_type_map[elem].size;
            } else if (tlv->basic_type == _bcmFieldInternalArray) {
                write_size = ((tlv->length) *
                              (static_type_map[elem].size));
            } else {
               return BCM_E_INTERNAL;
            }
            if (static_type_map[elem].flags & _FP_WB_TLV_LEN_ENCODED) {
                 write_size = (((tlv->length) & _FP_WB_TLV_TYPES_MASK) >>
                                 _FP_WB_TLV_TYPE_SHIFT) *
                                 sizeof(tlv->type);
             }
    }

    /* Write Value */
    if (write_size > 0) {
        BCM_IF_ERROR_RETURN(_tlv_print_array(unit, tlv->value, write_size,
                                    static_type_map[elem].size));
        if (wb_write_disable != 1) {
            sal_memcpy(&ptr[pos], tlv->value, write_size);
        }
        pos += write_size;
    }

    *position = pos;

    return BCM_E_NONE;

}

/*
 * Function:
 *      tlv_read
 * Purpose:
 *      Read a chunck from scache. Chunk size depends on the type read/passed.
 *      Call function needs to free the memory allocated to tlv structure/value
 *      field in tlv structure.
 *      Function flow:
 *          Read type (if input type is not a valid one)
 *          Get corresponding flags, read length if it is present.
 *          If value is present, read the corresponding value/array of values
 *          If not, and length is encoded, read value array which contains the
 *          list of types that are encoded. Later tlv_read has to be called
 *          with corresponding type to get the value of each instance.
 * Parameters:
 *      unit - (IN) StrataSwitch unit #.
 *      tlv  - (OUT) structure containing type, length and value/value array
 *                   read(based on flags from static_type_map)
 *      ptr  - (IN) pointer to scache; obtained from
 *                  field_control->scache_ptr[PART];
 *      pos  - (IN/OUT) Current offset from the base of scache part in use.
 * Returns:
 *      BCM_E_XXXX
 */

int
tlv_read(int unit, _field_tlv_t *tlv, uint8 *ptr, uint32 *position)
{
    uint32 pos, data = 0;     /* local variables for scache_pos and data */
    int read_size    = 0;     /* size used for value field */

    if (tlv == NULL) {
        return BCM_E_PARAM;
    }
    pos = *position;

    /* Read Type */
    if (tlv->type == -1) {
        sal_memcpy(&data, &ptr[pos], sizeof(uint32));
        pos += sizeof(uint32);

        tlv->type = data & _FP_WB_TLV_ELEM_TYPE_MASK;
        tlv->basic_type = ((data & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                           _FP_WB_TLV_BASIC_TYPE_SHIFT);
        LOG_DEBUG(BSL_LS_BCM_FP, (BSL_META_U(unit, "TLV Read Enum: %s\n"),
                                             elem_name[tlv->type]));
    } else {
        LOG_DEBUG(BSL_LS_BCM_FP, (BSL_META_U(unit,
                  "Reading For Enum: %s\n"),
                   elem_name[tlv->type]));
    }

    /* Read Length */
    if (recovery_type_map[unit][tlv->type].flags
        & _FP_WB_TLV_LEN_PRESENT) {
        sal_memcpy(&(tlv->length), &ptr[pos], sizeof(uint32));
        pos += sizeof(uint32);
        LOG_DEBUG(BSL_LS_BCM_FP, (BSL_META_U(unit, "TLV READ Length: %x\n"),
                                             tlv->length));
    }


    if (!(recovery_type_map[unit][tlv->type].flags
          & _FP_WB_TLV_NO_VALUE)) {
        if (tlv->basic_type == _bcmFieldInternalVariable) {
            read_size = recovery_type_map[unit][tlv->type].size;
        } else if (tlv->basic_type == _bcmFieldInternalArray) {
            read_size = ((tlv->length) * recovery_type_map[unit][tlv->type].size);
        } else {
            return BCM_E_INTERNAL;
        }
        if (recovery_type_map[unit][tlv->type].flags
             & _FP_WB_TLV_LEN_ENCODED) {
              data = (((tlv->length) & _FP_WB_TLV_TYPES_MASK) >>
                       _FP_WB_TLV_TYPE_SHIFT);
              read_size = data * sizeof(uint32);
        }
    }

    /* Read Value */
    if (read_size > 0) {
        _FP_XGS3_ALLOC(tlv->value, read_size, "wb read values");
        if (tlv->value == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memcpy(tlv->value, &ptr[pos], read_size);
        pos += read_size;
        BCM_IF_ERROR_RETURN(
             _tlv_print_array(unit, tlv->value, read_size,
                              recovery_type_map[unit][tlv->type].size));
    }
    if (read_size == 0) {
        tlv->value = NULL;
    }
    *position = pos;

    return BCM_E_NONE;
}


/* Function:
 * _field_tlv_create
 *
 * Purpose:
 *   Create memory for tlv and set basic fields
 *
 * Parameters:
 * type                    -(IN) element type
 * basic_type              -(IN) basic type of the element
 * length                  -(IN) length of element in case of arrays
 * tlv                     -(OUT) tlv output
 *
 *  Returns:
 *      BCM_E_XXXX
 */

int
_field_tlv_create(_bcm_field_internal_element_t type,
                  _bcm_field_internal_type_t basic_type,
                  uint32 length, _field_tlv_t **tlv)
{
    _field_tlv_t *tlv_new = NULL;   /* TLV data structure instance */

    if (*tlv != NULL) {
       sal_free(*tlv);
       *tlv = NULL;
    }
    _FP_XGS3_ALLOC(tlv_new,sizeof(_field_tlv_t),"tlv alloc");

    if (tlv_new == NULL) {
        return BCM_E_MEMORY;
    }
    tlv_new->type = type;
    tlv_new->basic_type = basic_type;
    tlv_new->length = length;

    *tlv = tlv_new;
    return BCM_E_NONE;
}

/* Must Point to the current stage which is being synced */
/* Checck _field_tlv_validate_and_write for usage */
_field_stage_t *curr_stage_fc;

/* Function:
 * _field_tlv_validate_and_write
 *
 * Purpose:
 *   Check if the type is valid for the current stage and write it to
 *   scache.
 *
 * Parameters:
 * unit                    -(IN) unit number
 * tlv                     -(IN) tlv structure
 * ptr                     -(IN) pointer to fc->scache_ptr (usually)
 * position                    -(IN) position
 *
 *  Returns:
 *      BCM_E_XXXX
 */
int
_field_tlv_validate_and_write(int unit, _field_tlv_t *tlv, uint8 *ptr,
                              uint32 *position)
{
    if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
        if (static_type_map[tlv->type].flags & _FP_WB_TLV_NO_INGRESS) {
            return BCM_E_NONE;
        }
    }

    if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_CLASS) {
        if (!(static_type_map[tlv->type].flags & _FP_WB_TLV_CLASS)) {
            return BCM_E_NONE;
        }
    }

    if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
        if (static_type_map[tlv->type].flags & _FP_WB_TLV_NO_EM) {
            return BCM_E_NONE;
        }
   }

    return tlv_write(unit, tlv, ptr, position);
}


/* Basic function calls required in all Functions */

#define WB_FIELD_CONTROL_GET(fc, s_ptr, s_pos)                  \
    do {                                                        \
        BCM_IF_ERROR_RETURN(_field_control_get(unit, &fc));     \
        s_ptr = fc->scache_ptr[_FIELD_SCACHE_PART_0];           \
        s_pos = &(fc->scache_pos);                              \
    } while (0);

#define WB_FIELD_CONTROL_GET_WITH_LOCK(fc, s_ptr, s_pos)        \
    do {                                                        \
        FP_LOCK(unit);                                          \
        rv = _field_control_get(unit, &fc);                     \
        if (BCM_FAILURE(rv)) {                                  \
            FP_UNLOCK(unit);                                    \
            return rv;                                          \
        }                                                       \
        s_ptr = fc->scache_ptr[_FIELD_SCACHE_PART_0];           \
        s_pos = &(fc->scache_pos);                              \
    } while (0);

#define TLV_WRITE(unit, tlv, ptr, pos)     \
           BCM_IF_ERROR_RETURN(_field_tlv_validate_and_write(unit,tlv,ptr,pos));

#define TLV_CREATE(_type_, _inttype_, _size_, _value_) \
   BCM_IF_ERROR_RETURN(_field_tlv_create(_type_, _inttype_, _size_, _value_));


/* Function:
 * _field_hash_sync
 *
 * Purpose:
 *    Sync hashes for stat, policer, hint structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_hash_sync(int unit)
{

    _field_tlv_t *tlv       = NULL;       /* tlv data */
    _field_control_t        *fc;          /* Field control structure.       */
    _field_policer_t        *f_pl;        /* Internal policer descriptor.   */
    _field_stat_t           *f_st;        /* Internal Stat descriptor */
    _field_hints_t          *f_ht;        /* Internal Hints Descriptor */
    int idx                 = 0;          /* Iterator */
    int policer_count       = 0;          /* number of policers */
    int stat_count          = 0;          /* number of stats */
    int hints_count         = 0;          /* number of hints */
    uint32 endmarkerpolicer = _FIELD_WB_EM_POLICER;
                                          /* End marker for policer */
    uint32 endmarkerstat    = _FIELD_WB_EM_STAT;
                                          /* End marker for stat */
    uint32 endmarkerhint    = _FIELD_WB_EM_HINT;
                                          /* End marker for hint */
    _field_hint_t           *hint;        /* temporary field for hint */
    int hintidx             = 0;          /* hint id variable */
    _field_stat_bmp_t       stat_bmp;     /* Field Stat Bitmap */
    int type_policer[22] = {              /* types in _field_policer_t */
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerPid),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgFlags),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgMode),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgCkbitsSec),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgMaxCkbitsSec),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgCkbitsBurst),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgPkbitsSec),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgMaxPkbitsSec),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgPkbitsBurst),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgKbitsCurrent),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgActionId),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgSharingMode),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgEntropyId),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerCfgPoolId),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerSwRefCount),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerHwRefCount),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerLevel),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerPoolIndex),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerHwIndex),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerHwFlags),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPolicerStageId),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalEndStructPolicer)
    };

    int type_stat[6] = {                 /* types in _field_stat_t */
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalStatsId),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalStatnStat),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalStatFlexMode),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalStatHwFlags),
    (int)((_bcmFieldInternalArray    << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalStatBmp),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalEndStructStat)
    };
    int type_hint[9] =  {                 /* types in _field_hint_t */
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsHintType),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsQual),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsMaxValues),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsStartBit),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsEndBit),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsFlags),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsMaxGrpSize),
    (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalHintHintsPri),
    (int) (( _bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalEndStructHintHints)
    };
    uint8 *ptr;                          /* Pointer to scache_pos */
    uint32 *position;                    /* Scache position */
    int i = 0;                           /* Iterator */

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    stat_bmp.w = NULL;

    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
        /* Iterate over entries in each bucket. */
        f_pl = fc->policer_hash[idx];
        while (NULL != f_pl) {
        if((f_pl->pid != -1)  && ((f_pl->stage_id == _BCM_FIELD_STAGE_INGRESS)
                    || (f_pl->stage_id == _BCM_FIELD_STAGE_EXACTMATCH))) {
           policer_count ++;
        }
            f_pl = f_pl->next;
        }
    }

    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
        /* Iterate over entries in each bucket. */
        f_st = fc->stat_hash[idx];
        while (NULL != f_st) {
        if((f_st->sid != -1)  && ((f_st->stage_id == _BCM_FIELD_STAGE_INGRESS)
            || (f_st->stage_id == _BCM_FIELD_STAGE_EXACTMATCH))) {
           stat_count ++;
        }
            f_st = f_st->next;
        }
    }

   /* Iterate over all hash buckets. */
    for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
        /* Iterate over entries in each bucket. */
        f_ht = fc->hints_hash[idx];
        while (NULL != f_ht) {
        if(f_ht->hintid != -1) {
           hints_count ++;
        }
            f_ht = f_ht->next;
        }
    }


    /* _field_policer_t */

    /* In the encoded length below , first ten bits is value 22 [no of types],
     * next 22 bits is policer count in hash. Here number of types indicate
     * count of elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalControlPolicerHash, _bcmFieldInternalArray,
               (policer_count | (22 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &(type_policer);
    TLV_WRITE(unit, tlv, ptr, position);


    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
        /* Iterate over entries in each bucket. */
        f_pl = fc->policer_hash[idx];
        while (NULL != f_pl) {
        if((f_pl->pid != -1)  && ((f_pl->stage_id == _BCM_FIELD_STAGE_INGRESS) ||
                    (f_pl->stage_id == _BCM_FIELD_STAGE_EXACTMATCH))) {
            TLV_CREATE(_bcmFieldInternalPolicerPid,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->pid);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgFlags,
                                  _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.flags);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgMode,
                                 _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.mode);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgCkbitsSec,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.ckbits_sec);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgMaxCkbitsSec,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.max_ckbits_sec);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgCkbitsBurst,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.ckbits_burst);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgPkbitsSec,
                         _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.pkbits_sec);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgMaxPkbitsSec,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.max_pkbits_sec);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgPkbitsBurst ,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.pkbits_burst);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgKbitsCurrent,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.kbits_current);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgActionId,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.action_id);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgSharingMode,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.sharing_mode);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgEntropyId,
                                  _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.entropy_id);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerCfgPoolId,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->cfg.pool_id);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerSwRefCount,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->sw_ref_count);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerHwRefCount,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->hw_ref_count);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerLevel,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->level);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerPoolIndex,
                                  _bcmFieldInternalVariable,0,&tlv);
            tlv->value = &(f_pl->pool_index);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerHwIndex,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->hw_index);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerHwFlags,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->hw_flags);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPolicerStageId,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pl->stage_id);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalEndStructPolicer,
                        _bcmFieldInternalVariable, 0, &tlv);
            tlv->value = &endmarkerpolicer;
            TLV_WRITE(unit, tlv, ptr, position);

        }
            f_pl = f_pl->next;
        }
    }


    /* _field_stat_t */

    /* In the encoded length below , first ten bits is value 17 [no of types],
     * next 22 bits is stat count in hash. Here number of types indicate counts
     * of elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalControlStathash, _bcmFieldInternalVariable,
                (stat_count | (6 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &(type_stat);
    TLV_WRITE(unit, tlv, ptr, position);

    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
        /* Iterate over entries in each bucket. */
        f_st = fc->stat_hash[idx];
        while (NULL != f_st) {
        if((f_st->sid != -1)  && ((f_st->stage_id == _BCM_FIELD_STAGE_INGRESS)
                              || (f_st->stage_id == _BCM_FIELD_STAGE_EXACTMATCH))) {

            TLV_CREATE(_bcmFieldInternalStatsId,
                                   _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_st->sid);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalStatnStat,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_st->nstat);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalStatFlexMode,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_st->flex_mode);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalStatHwFlags,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_st->hw_flags);
            TLV_WRITE(unit, tlv, ptr, position);

            _FP_XGS3_ALLOC(stat_bmp.w,
                           SHR_BITALLOCSIZE(_bcmFieldStatCount),
                           "Stat BMP");
            if (stat_bmp.w == NULL) {
                return BCM_E_MEMORY;
            }

            for (i = 0; i < f_st->nstat; i++) {
                 _FP_STAT_BMP_ADD(stat_bmp, f_st->stat_arr[i]);
            }

            TLV_CREATE(_bcmFieldInternalStatBmp,
                       _bcmFieldInternalArray, _SHR_BITDCLSIZE(_bcmFieldStatCount),
                       &tlv);
            tlv->value = stat_bmp.w;
            TLV_WRITE(unit, tlv, ptr, position);

            if (stat_bmp.w !=NULL) {
                _FP_STAT_BMP_FREE(stat_bmp);
            }

            stat_bmp.w = NULL;

            TLV_CREATE(_bcmFieldInternalEndStructStat,
                               _bcmFieldInternalVariable, 0, &tlv);
            tlv->value = &endmarkerstat;
            TLV_WRITE(unit, tlv, ptr, position);

         }
         f_st = f_st->next;
        }
    }

    /* _field_hints_t */

    TLV_CREATE(_bcmFieldInternalControlHintHash,
               _bcmFieldInternalVariable, hints_count, &tlv);
    TLV_WRITE(unit, tlv, ptr, position);
    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _FP_HASH_SZ(fc); idx++) {
        /* Iterate over entries in each bucket. */
        f_ht = fc->hints_hash[idx];
        while (NULL != f_ht) {
        if(f_ht->hintid != -1) {
           TLV_CREATE(_bcmFieldInternalHintHintid,
                         _bcmFieldInternalVariable,0, &tlv);
           tlv->value = &(f_ht->hintid);
           TLV_WRITE(unit, tlv, ptr, position);

          /* _field_hint_t */
          /* In the encoded length below , first ten bits is value 8 [no of
           * types], next 22 bits is hints count in hash. Here number of types
           * indicate counts of elements in the current datastructure.
           */


           TLV_CREATE(_bcmFieldInternalHintHints, _bcmFieldInternalArray,
                      f_ht->hint_count
                      | (8 << _FP_WB_TLV_TYPE_SHIFT), &tlv);
           tlv->value = &(type_hint);
           TLV_WRITE(unit, tlv, ptr, position);
           hint = f_ht->hints;

           for (hintidx = 0; hintidx < f_ht->hint_count; hintidx++) {

                TLV_CREATE(_bcmFieldInternalHintHintsHintType,
                                        _bcmFieldInternalVariable,0,&tlv);
                tlv->value = &(hint->hint->hint_type);
                TLV_WRITE(unit, tlv, ptr, position);

                TLV_CREATE(_bcmFieldInternalHintHintsQual,
                                        _bcmFieldInternalVariable,0, &tlv);
                tlv->value = &(hint->hint->qual);
                TLV_WRITE(unit, tlv, ptr, position);

                TLV_CREATE(_bcmFieldInternalHintHintsMaxValues,
                                        _bcmFieldInternalVariable,0, &tlv);
                tlv->value = &(hint->hint->max_values);
                TLV_WRITE(unit, tlv, ptr, position);

                TLV_CREATE(_bcmFieldInternalHintHintsStartBit,
                                        _bcmFieldInternalVariable,0, &tlv);
                tlv->value = &(hint->hint->start_bit);
                TLV_WRITE(unit, tlv, ptr, position);

                TLV_CREATE(_bcmFieldInternalHintHintsEndBit,
                                _bcmFieldInternalVariable,0, &tlv);
                tlv->value = &(hint->hint->end_bit);
                TLV_WRITE(unit, tlv, ptr, position);

                TLV_CREATE(_bcmFieldInternalHintHintsFlags,
                            _bcmFieldInternalVariable,0, &tlv);
                tlv->value = &(hint->hint->flags);
                TLV_WRITE(unit, tlv, ptr, position);

                TLV_CREATE(_bcmFieldInternalHintHintsMaxGrpSize,
                           _bcmFieldInternalVariable,0, &tlv);
                tlv->value = &(hint->hint->max_group_size);
                TLV_WRITE(unit, tlv, ptr, position);

                TLV_CREATE(_bcmFieldInternalHintHintsPri,
                           _bcmFieldInternalVariable,0, &tlv);
                tlv->value = &(hint->hint->priority);
                TLV_WRITE(unit, tlv, ptr, position);


                TLV_CREATE(_bcmFieldInternalEndStructHintHints,
                           _bcmFieldInternalVariable, 0, &tlv);
                TLV_WRITE(unit, tlv, ptr, position);

                hint = hint->next;
           }


           TLV_CREATE(_bcmFieldInternalHintGrpRefCount,
                                    _bcmFieldInternalVariable,0, &tlv);
           tlv->value = &(f_ht->grp_ref_count);
           TLV_WRITE(unit, tlv, ptr, position);

           TLV_CREATE(_bcmFieldInternalHintCount,
                                        _bcmFieldInternalVariable,0, &tlv);
           tlv->value = &(f_ht->hint_count);
           TLV_WRITE(unit, tlv, ptr, position);

           TLV_CREATE(_bcmFieldInternalEndStructHint,
                                 _bcmFieldInternalVariable, 0, &tlv);
           tlv->value = &endmarkerhint;
           TLV_WRITE(unit, tlv, ptr, position);

        }
        f_ht = f_ht->next;
        }
    }

    sal_free(tlv);
    return BCM_E_NONE;
}


/* Function:
 * _field_presel_entry_sync
 *
 * Purpose:
 *    Sync presel entries
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_presel_entry_sync(int unit)
{

    _field_tlv_t *tlv       = NULL;       /* tlv data */
    _field_control_t        *fc;          /* Field control structure.       */
    _field_presel_entry_t   *f_pr;        /* Internal Presel Descriptor */
    int idx                 = 0;          /* Iterator */
    int presel_ent_count    = 0;          /* number of presel entries */
    uint32 endmarkerpresel  = _FIELD_WB_EM_PRESEL;
                                          /* End marker for Presel */
     int type_presel[12] =  {             /* types in _field_presel_entry_t */
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselId),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselFlags),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselPri),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselSliceIndex),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselEntryGroup),
    (int)((_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselEntrySlice),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselKeySize),
    (int)((_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselQset),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselHwIndex),
    (int)((_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselClassAction),
    (int)((_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalPreselPbmp),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalEndStructPreselInfo)
    };

    uint8 *ptr;                          /* Pointer to scache_pos */
    uint32 *position;                    /* Scache position */
    bcm_field_qualify_t stage_qual= bcmFieldQualifyStage;  /* Stage Qualifier */
    int empty = -1;                      /* Dummy data */

    if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
        stage_qual = bcmFieldQualifyStageIngress;
    } else if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
        stage_qual = bcmFieldQualifyStageIngressExactMatch;
    }

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    /* Iterate over entry array. */
    for (idx = 0; idx < BCM_FIELD_PRESEL_SEL_MAX; idx++) {
        /* Iterate over entries in each bucket. */
        f_pr = fc->presel_db[idx];
        if ((NULL != f_pr) && (f_pr->presel_id != -1)
             && (BCM_FIELD_QSET_TEST(f_pr->p_qset,
                 stage_qual))) {
           presel_ent_count ++;
        }
    }

    /* _field_presel_entry_t */

    /* In the encoded length below , first ten bits is value 12 [no of types],
     * next 22 bits is presel entry count in hash.
     * Here number of types indicate counts
     * of elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalControlPreselInfo, _bcmFieldInternalVariable,
                (presel_ent_count | (12 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &(type_presel);
    TLV_WRITE(unit, tlv, ptr, position);

    /* Iterate over all hash buckets. */
    for (idx = 0; idx < BCM_FIELD_PRESEL_SEL_MAX ; idx++) {
        /* Iterate over entries in each bucket. */
        f_pr = fc->presel_db[idx];
        if ((NULL != f_pr) && (f_pr->presel_id != -1)
             && (BCM_FIELD_QSET_TEST(f_pr->p_qset,
                 stage_qual))) {
            TLV_CREATE(_bcmFieldInternalPreselId,
                       _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pr->presel_id);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPreselFlags,
                       _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pr->flags);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPreselPri,
                       _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pr->priority);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPreselSliceIndex,
                       _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pr->slice_index);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPreselEntryGroup,
                                    _bcmFieldInternalVariable,0, &tlv);
            if (f_pr->group != NULL) {
                tlv->value = &(f_pr->group->gid);
            } else {
                tlv->value = &empty;
            }
            TLV_WRITE(unit, tlv, ptr, position);
            if (f_pr->lt_fs != NULL) {
                TLV_CREATE(_bcmFieldInternalPreselEntrySlice,
                                    _bcmFieldInternalArray,1, &tlv);
                tlv->value = &(f_pr->lt_fs->slice_number);
            } else {
               TLV_CREATE(_bcmFieldInternalPreselEntrySlice,
                                    _bcmFieldInternalArray,0, &tlv);
            }
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPreselKeySize,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pr->lt_tcam.key_size);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPreselQset,
                       _bcmFieldInternalArray,
                       _SHR_BITDCLSIZE(BCM_FIELD_QUALIFY_MAX), &tlv);
            tlv->value = &(f_pr->p_qset.w);
            TLV_WRITE(unit, tlv, ptr, position);
            TLV_CREATE(_bcmFieldInternalPreselHwIndex,
                      _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(f_pr->hw_index);
            TLV_WRITE(unit, tlv, ptr, position);

            if (f_pr->actions != NULL) {
               TLV_CREATE(_bcmFieldInternalPreselClassAction,
                                        _bcmFieldInternalArray,1, &tlv);
               tlv->value = &(f_pr->actions->param[0]);
            } else {
               TLV_CREATE(_bcmFieldInternalPreselClassAction,
                                        _bcmFieldInternalArray,0, &tlv);
            }
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalPreselPbmp,
                       _bcmFieldInternalArray,
                       _SHR_PBMP_WORD_MAX, &tlv);
            tlv->value = &(f_pr->mixed_src_class_pbmp.pbits);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalEndStructPreselInfo,
                               _bcmFieldInternalVariable, 0, &tlv);
            tlv->value = &endmarkerpresel;
            TLV_WRITE(unit, tlv, ptr, position);

        }
    }

    sal_free(tlv);
    return BCM_E_NONE;

}

/* Function:
 * _field_udf_sync
 *
 * Purpose:
 *    Sync _field_udf_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * scache_ptr       - (IN) Pointer to device field control structure
 * ptr              -
 * udf              -
 * Returns:
 *     BCM_E_XXX
 */
int
_field_udf_sync(int unit, uint8 *ptr, uint32 *pos, _field_udf_t *udf)
{
    _field_tlv_t *tlv   = NULL;              /* TLV data structure */
    int endmarker       = _FIELD_WB_EM_UDF;  /* End marker for UDF */

    TLV_CREATE(_bcmFieldInternalControlUdfValid,
                                   _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(udf->valid);
    TLV_WRITE(unit,tlv,ptr,pos);

    TLV_CREATE(_bcmFieldInternalControlUdfUseCount,
                                   _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(udf->use_count);
    TLV_WRITE(unit,tlv,ptr,pos);

    TLV_CREATE(_bcmFieldInternalControlUdfNum,
                                   _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(udf->udf_num);
    TLV_WRITE(unit,tlv,ptr,pos);

    TLV_CREATE(_bcmFieldInternalControlUdfUserNum,
                                   _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(udf->user_num);
    TLV_WRITE(unit,tlv,ptr,pos);

    TLV_CREATE(_bcmFieldInternalControlEndStructUdf,
                                   _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &endmarker;
    TLV_WRITE(unit,tlv,ptr,pos);

    sal_free(tlv);
    return BCM_E_NONE;

}

/* Function:
 * _field_control_sync
 *
 * Purpose:
 *    Sync _field_control_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * fc               - (IN) Pointer to device field control structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_control_sync(int unit)
{

    _field_tlv_t *tlv   = NULL;     /* Tlv data structure */
    uint8 *scache_ptr   = NULL;
                                    /* Scache Pointer */
    _field_group_t *fg;             /* Group information */
    int group_count    = 0;         /* Number of groups */
    int *gid           = NULL;      /* Array of groupids */
    int hintbmp_length = 0;         /* length of hint bmp */
    uint32 endmarker   = _FIELD_WB_EM_CONTROL;
                                    /* End marker for fc */
    int pos            = 0;         /* variable for loops */

    int type_udf[5] = {             /* types in _field_udf_t */
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
            | _bcmFieldInternalControlUdfValid),
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
            | _bcmFieldInternalControlUdfUseCount),
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
            | _bcmFieldInternalControlUdfNum),
     (int)  ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
            | _bcmFieldInternalControlUdfUserNum),
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalControlEndStructUdf),
                      };
    uint32 *position;             /* Scache position */
    _field_control_t *fc;         /* field control Structure */
    int rv  = 0;                  /* Return variable */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);


    TLV_CREATE(_bcmFieldInternalControl,
               _bcmFieldInternalVariable, 0, &tlv);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalControlFlags,
                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(fc->flags);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    fg = fc->groups;
    while (fg != NULL) {
        if (fg->stage_id == _BCM_FIELD_STAGE_INGRESS) {
            group_count ++ ;
        }
        fg = fg->next;
    }

    _FP_XGS3_ALLOC(gid, (sizeof(uint32) * group_count), "alloc for groups");
    if (gid == NULL) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }

    fg = fc->groups;
    while (fg != NULL) {
        if (fg->stage_id == _BCM_FIELD_STAGE_INGRESS) {
            gid[pos++] = fg->gid;
        }
        fg = fg->next;
    }

    /* In the encoded length below , first ten bits is value 5 [no of types],
     * next 22 bits is no of udfs. Here number of types indicate counts of
     * elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalControlUdfDetails, _bcmFieldInternalArray,
               (BCM_FIELD_USER_NUM_UDFS | (5 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &type_udf;
    /* coverity[resource_leak] */
    /* coverity[leaked_storage] */
    TLV_WRITE(unit, tlv, scache_ptr, position);

    for (pos = 0; pos < BCM_FIELD_USER_NUM_UDFS; pos++) {
        rv = _field_udf_sync(unit, scache_ptr,
                             position, &(fc->udf[pos]));
        BCM_IF_ERROR_CLEANUP(rv);
    }


    TLV_CREATE(_bcmFieldInternalControlGroups,
               _bcmFieldInternalArray, group_count, &tlv);
    tlv->value = &gid;
    TLV_WRITE(unit, tlv, scache_ptr, position);


    TLV_CREATE(_bcmFieldInternalControlPolicerCount,
                                          _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(fc->policer_count);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalControlStatCount,
                                          _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(fc->stat_count);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    hintbmp_length = SHR_BITALLOCSIZE(_FP_HINT_ID_MAX);
    TLV_CREATE(_bcmFieldInternalControlHintBmp,
                                          _bcmFieldInternalArray,
                                          hintbmp_length, &tlv);
    tlv->value = fc->hintid_bmp.w;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalControlLastAllocatedLtId,
                           _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(fc->last_allocated_lt_eid);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    rv = _field_hash_sync(unit);
    BCM_IF_ERROR_CLEANUP(rv);


    TLV_CREATE(_bcmFieldInternalPreselInfoLimit,
               _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(fc->presel_info->presel_limit);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalPreselLastAllocatedId,
               _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(fc->presel_info->last_allocated_id);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalPreselSet,
               _bcmFieldInternalArray,
               _SHR_BITDCLSIZE(BCM_FIELD_PRESEL_SEL_MAX),&tlv);
    tlv->value = &(fc->presel_info->presel_set);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalPreselOperationalSet,
               _bcmFieldInternalArray,
               _SHR_BITDCLSIZE(BCM_FIELD_PRESEL_SEL_MAX),&tlv);
    tlv->value = &(fc->presel_info->operational_set);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalEndStructControl,
                             _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &endmarker;
    TLV_WRITE(unit, tlv, scache_ptr, position);

cleanup:
    sal_free(gid);
    sal_free(tlv);
    return rv;


}
/* Function:
 * _field_datacontroldataqualifier_sync
 *
 * Purpose:
 *    Sync _field_data_qualifier_p structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * fdp              - (IN) Pointer to Field Data Qualifier structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_datacontroldataqualifier_sync(int unit, _field_data_qualifier_p fdp)
{

    _field_tlv_t *tlv = NULL;              /* &tlv data */
    uint8 *ptr = NULL;                     /* Scache Pointer */
    _field_control_t *fc = NULL;           /* Field control structure. */
    uint32 *position = NULL;               /* Scache Position */
    int endmarker = _FIELD_WB_EM_DATAQUAL;
                                           /* End Marker */

    WB_FIELD_CONTROL_GET(fc, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlDataQualQid,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->qid);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlDataQualUdfSpec,
                                _bcmFieldInternalArray, 384, &tlv);
    tlv->value = &(fdp->spec);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlDataQualOffsetBase,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->offset_base);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlDataQualOffset,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->offset);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalDataControlDataQualByteOffset,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->byte_offset);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlDataQualHwBmp,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->hw_bmap);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalDataControlDataQualFlags,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->flags);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlDataQualElemCount,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->elem_count);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlDataQualLength,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fdp->length);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalDataControlEndStructDataQual,
                                _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(endmarker);
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);

    return BCM_E_NONE;
}

/* Function:
 * _field_extractor_sync
 *
 * Purpose:
 *    recover _field_slice_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * fc            - (IN) field control structure of the unit.
 * ext_sel       - (IN) address of start of array in slice structure.
 * count         - (IN) Number of extractor structures to be synced.
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_extractor_sync(int unit,
                      _field_ext_sel_t *ext_sel, int count)
{

    _field_tlv_t *tlv = NULL;  /* tlv structure */
    int i = 0;                 /* loop variable */
    int endmarker = _FIELD_WB_EM_EXTRACTOR;
                               /* End marker */
    uint8 *ptr;                /* pointer to scache_ptr (base) */
    uint32 *pos;               /* Pointer to scache_pos (offset) */
    _field_control_t *fc;      /* field Control structure */
    int type_ext_sel[32] = {   /* types in _field_ext_sel_t */
    (int) (_bcmFieldInternalExtl1e32Sel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl1e16Sel
           |(_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl1e8Sel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl1e4Sel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl1e2Sel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl2e16Sel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl3e1Sel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl3e2Sel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtl3e4Sel
           |(_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtPmuxSel
           | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtIntraSlice
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtSecondary
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtIpbmpRes
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtNorml3l4
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtNormmac
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtAuxTagaSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtAuxTagbSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtAuxTagcSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtAuxTagdSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtTcpFnSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtTosFnSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtTtlFnSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtClassIdaSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtClassIdbSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtClassIdcSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtClassIddSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtSrcContaSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtSrcContbSel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtSrcDestCont0Sel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtSrcDestCont1Sel
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalExtKeygenIndex
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalEndStructExtractor
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    };

    WB_FIELD_CONTROL_GET(fc, ptr, pos);


    /* In the encoded length below , first ten bits is value 32 [no of types],
     * next 22 bits is count of extractors. Here number of types indicate counts
     * of elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalExtractor,
               _bcmFieldInternalArray, (count
               | (32 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &type_ext_sel[0];
    TLV_WRITE(unit,tlv,ptr,pos);

    for (i = 0; i < count; i++) {
        TLV_CREATE(_bcmFieldInternalExtl1e32Sel,
                                       _bcmFieldInternalArray, 4, &tlv);
        tlv->value = ext_sel->l1_e32_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl1e16Sel,
                                       _bcmFieldInternalArray, 7, &tlv);
        tlv->value = ext_sel->l1_e16_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl1e8Sel,
                                       _bcmFieldInternalArray, 7, &tlv);
        tlv->value = ext_sel->l1_e8_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl1e4Sel,
                                       _bcmFieldInternalArray, 8, &tlv);
        tlv->value = ext_sel->l1_e4_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl1e2Sel,
                                       _bcmFieldInternalArray, 8, &tlv);
        tlv->value = ext_sel->l1_e2_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl2e16Sel,
                                       _bcmFieldInternalArray, 10, &tlv);
        tlv->value = ext_sel->l2_e16_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl3e1Sel,
                                       _bcmFieldInternalArray, 2, &tlv);
        tlv->value = ext_sel->l3_e1_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl3e2Sel,
                                       _bcmFieldInternalArray, 5, &tlv);
        tlv->value = ext_sel->l3_e2_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtl3e4Sel,
                                       _bcmFieldInternalArray, 21, &tlv);
        tlv->value = ext_sel->l3_e4_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtPmuxSel,
                                       _bcmFieldInternalArray, 15, &tlv);
        tlv->value = ext_sel->pmux_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtIntraSlice,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->intraslice;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtSecondary,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->secondary;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtIpbmpRes,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->ipbm_present;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtNorml3l4,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->normalize_l3_l4_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtNormmac,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->normalize_mac_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtAuxTagaSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->aux_tag_a_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtAuxTagbSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->aux_tag_b_sel;
        TLV_WRITE(unit,tlv,ptr,pos);


        TLV_CREATE(_bcmFieldInternalExtAuxTagcSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->aux_tag_c_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtAuxTagdSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->aux_tag_d_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtTcpFnSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->tcp_fn_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

            TLV_CREATE(_bcmFieldInternalExtTosFnSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->tos_fn_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtTtlFnSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->ttl_fn_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtClassIdaSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->class_id_cont_a_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtClassIdbSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->class_id_cont_b_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtClassIdcSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->class_id_cont_c_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtClassIddSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->class_id_cont_d_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtSrcContaSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->src_cont_a_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtSrcContbSel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->src_cont_b_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtSrcDestCont0Sel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->src_dest_cont_0_sel;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalExtSrcDestCont1Sel,
                                       _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->src_dest_cont_1_sel;
        TLV_WRITE(unit,tlv,ptr,pos);


        TLV_CREATE(_bcmFieldInternalExtKeygenIndex,
                                      _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &ext_sel->keygen_index;
        TLV_WRITE(unit,tlv,ptr,pos);

        TLV_CREATE(_bcmFieldInternalEndStructExtractor,
                                      _bcmFieldInternalVariable, 4, &tlv);
        tlv->value = &endmarker;
        TLV_WRITE(unit,tlv,ptr,pos);


    }
    sal_free(tlv);
    return BCM_E_NONE;

}
/* Function:
 * _field_datacontrolethertype_sync
 *
 * Purpose:
 *    Sync _field_data_ethertype_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * fd               - (IN) Pointer to Field Data Control structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_datacontrolethertype_sync(int unit, _field_data_control_t *fd)
{

    _field_tlv_t *tlv = NULL;              /* Tlv data structure */
    uint8 *ptr = NULL;                     /* Scache Pointer */
    _field_control_t *fc = NULL;           /* Field control structure. */
    uint32 *position = NULL;               /* Scache Position */
    int endmarker = _FIELD_WB_EM_DATAETYPE;
                                           /* End marker */
    int i = 0;                             /* Loop variable */
    int type_etype[6] = {                  /* types in _field_ethertype_t */
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEthertypeRefCount),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEthertypeL2),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEthertypeVlanTag),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEthertypePortEt),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEthertypeRelOffset),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEndStructEtype)
    };

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    /* In the encoded length below , first ten bits is value 6 [no of types],
     * next 22 bits is ethertype count. Here number of types indicate counts of
     * elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalDataControlEthertypeStruct,
               _bcmFieldInternalArray, _FP_DATA_ETHERTYPE_MAX
               | (6 << _FP_WB_TLV_TYPE_SHIFT), &tlv);
    tlv->value = &(type_etype);
    TLV_WRITE(unit, tlv, ptr, position);


    for (i = 0; i < _FP_DATA_ETHERTYPE_MAX; i++) {

    TLV_CREATE(_bcmFieldInternalDataControlEthertypeRefCount,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->etype[i].ref_count);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlEthertypeL2,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->etype[i].l2);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalDataControlEthertypeVlanTag,
                     _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->etype[i].vlan_tag);
    TLV_WRITE(unit, tlv, ptr, position);



    TLV_CREATE(_bcmFieldInternalDataControlEthertypePortEt,
                     _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->etype[i].ethertype);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlEthertypeRelOffset,
                     _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->etype[i].relative_offset);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalDataControlEndStructEtype,
                                _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(endmarker);
    TLV_WRITE(unit, tlv, ptr, position);
    }

    sal_free(tlv);

    return BCM_E_NONE;

}

int
_field_datacontrolprot_sync(int unit, _field_data_control_t *fd)
{

    _field_tlv_t *tlv = NULL;              /* Tlv data structure */
    uint8 *ptr = NULL;                     /* Scache Pointer */
    _field_control_t *fc = NULL;           /* Field control structure. */
    uint32 *position = NULL;               /* Scache Position */
    int endmarker = _FIELD_WB_EM_DATAPROT; /* End marker */
    int i = 0;                             /* Loop local variable */
    int type_prot[8] = {                   /* types in _field_protocol_t */
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlProtIp4RefCount),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlProtIp6RefCount),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlProtFlags),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlProtIp),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlProtL2),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlProtVlanTag),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlProtRelOffset),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEndStructProt)
    };

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    /* In the encoded length below , first ten bits is value 8 [no of types],
     * next 22 bits is protocol count. Here number of types indicate counts of
     * elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalDataControlProtStart,
                                _bcmFieldInternalArray,
                                _FP_DATA_IP_PROTOCOL_MAX
                                | (8 << _FP_WB_TLV_TYPE_SHIFT), &tlv);
    tlv->value = &(type_prot);
    TLV_WRITE(unit, tlv, ptr, position);


    for (i = 0; i < _FP_DATA_IP_PROTOCOL_MAX; i++) {


    TLV_CREATE(_bcmFieldInternalDataControlProtIp4RefCount,
                                  _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->ip[i].ip4_ref_count);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlProtIp6RefCount,
                                  _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->ip[i].ip6_ref_count);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlProtFlags,
                                  _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->ip[i].flags);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlProtIp,
                                  _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->ip[i].ip);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlProtL2,
                                  _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->ip[i].l2);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlProtVlanTag,
                 _bcmFieldInternalVariable, 0,&tlv);
    tlv->value = &(fd->etype[i].vlan_tag);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlProtRelOffset,
                                  _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->etype[i].relative_offset);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalDataControlEndStructProt,
                                _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(endmarker);
    TLV_WRITE(unit, tlv, ptr, position);

    }
    sal_free(tlv);

    return BCM_E_NONE;

}

int
_field_datacontroltcamentry_sync(int unit, _field_data_control_t *fd)
{

    _field_tlv_t *tlv = NULL;              /* Tlv data structure*/
    uint8 *ptr = NULL;                     /* Scache Pointer */
    _field_control_t *fc = NULL;           /* Field control structure. */
    uint32 *position = NULL;               /* Scache Position */
    int endmarker = _FIELD_WB_EM_DATATCAM; /* End marker */
    int tcam_count = 0;                    /* Count of TCAM */
    _field_data_tcam_entry_t *tarr= NULL;  /* Tcam entry pointer */
    int i = 0;                             /* Loop local variable */
    int type_tcam[3] = {                 /* types in _field_data_tcam_entry_t */
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlTcamRefCount),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlTcamPriority),
    (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalDataControlEndStructTcam)
    };

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    tarr = fd->tcam_entry_arr;
    tcam_count = soc_mem_index_count(unit, FP_UDF_TCAMm);

    /* In the encoded length below , first ten bits is value 3 [no of types],
     * next 22 bits is tcam count. Here number of types indicate counts of
     * elements in the current datastructure.
     */


    TLV_CREATE(_bcmFieldInternalDataControlTcamStruct,
               _bcmFieldInternalArray, tcam_count
               | (3 << _FP_WB_TLV_TYPE_SHIFT),
                                &tlv);
    tlv->value = &(type_tcam);
    TLV_WRITE(unit, tlv, ptr, position);


    for (i = 0; i < tcam_count; i++) {

    TLV_CREATE(_bcmFieldInternalDataControlTcamRefCount,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(tarr[i].ref_count);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlTcamPriority,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(tarr[i].priority);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlEndStructTcam,
                                _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(endmarker);
    TLV_WRITE(unit, tlv, ptr, position);

    }

    sal_free(tlv);

    return BCM_E_NONE;

}


/* Function:
 * _field_data_control_sync
 *
 * Purpose:
 *    Sync _field_data_control_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * stage_fc         - (IN) Pointer to device stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_data_control_sync(int unit, _field_stage_t *stage_fc)
{
    int qual_count = 0;                    /* Number of Qualifiers */
    _field_data_qualifier_p temp = NULL;   /* Field data qualifier */
    _field_data_control_t *fd = NULL;      /* Field data control structure */
    _field_tlv_t *tlv = NULL;              /* Tlv data */
    uint8 *ptr = NULL;                     /* Scache Pointer */
    _field_control_t *fc = NULL;           /* Field control structure. */
    uint32 *position = NULL;               /* Scache Position */
    int endmarker = _FIELD_WB_EM_DATACONTROL;
                                           /* End marker for data control */

    WB_FIELD_CONTROL_GET(fc, ptr, position);
    fd = stage_fc->data_ctrl;

    TLV_CREATE(_bcmFieldInternalDataControlStart,
                                _bcmFieldInternalVariable,0, &tlv);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlUsageBmp,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->usage_bmap);
    TLV_WRITE(unit, tlv, ptr, position);

    temp = fd->data_qual;
    while (temp != NULL)
    {
    qual_count++ ;
    temp=temp->next;
    }


    TLV_CREATE(_bcmFieldInternalDataControlDataQualStruct,
                                _bcmFieldInternalVariable, qual_count , &tlv);
    TLV_WRITE(unit, tlv, ptr, position);

    temp = fd->data_qual;
    while (temp != NULL)
    {
    BCM_IF_ERROR_RETURN(_field_datacontroldataqualifier_sync(unit,temp));
    temp=temp->next;
    }
    BCM_IF_ERROR_RETURN(_field_datacontrolethertype_sync(unit,fd));
    BCM_IF_ERROR_RETURN(_field_datacontrolprot_sync(unit,fd));
    BCM_IF_ERROR_RETURN(_field_datacontroltcamentry_sync(unit,fd));

    TLV_CREATE(_bcmFieldInternalDataControlElemSize,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->elem_size);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalDataControlNumElem,
                                _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(fd->num_elems);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEndStructDataControl,
                                _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(endmarker);
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);

    return BCM_E_NONE;

}

/* Function:
 * _field_ltinfo_sync
 *
 * Purpose:
 *    Sync _field_lt_info_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * lt_info          - (IN) Pointer to device stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_ltinfo_sync(int unit, _field_lt_config_t *lt_info)
{
    _field_tlv_t *tlv = NULL;              /* Tlv data */
    uint8 *ptr = NULL;                     /* Scache Pointer */
    _field_control_t *fc = NULL;           /* Field control structure. */
    uint32 *position = NULL;               /* Scache Position */
    uint32 endmarkerltconf = _FIELD_WB_EM_LTCONF;
                                           /* End Marker */

    WB_FIELD_CONTROL_GET(fc, ptr, position);


    TLV_CREATE(_bcmFieldInternalLtConfigValid,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(lt_info->valid);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtConfigLtId,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(lt_info->lt_id);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtConfigLtPartPri,
                            _bcmFieldInternalArray,_FP_MAX_LT_PARTS, &tlv);
    tlv->value = &(lt_info->lt_part_pri);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtConfigLtPartMap,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(lt_info->lt_part_map);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtConfigLtActionPri,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(lt_info->lt_action_pri);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtConfigPri,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(lt_info->priority);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtConfigFlags,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(lt_info->flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtConfigEntry,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(lt_info->lt_entry);
    TLV_WRITE(unit,tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalEndStructLtConfig,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(endmarkerltconf);
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);

    return BCM_E_NONE;
}
/* Function:
 * _field_stage_sync
 *
 * Purpose:
 *    Sync _field_stage_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * stage_fc         - (IN) Pointer to device stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_stage_sync(int unit, _field_stage_t *stage_fc)
{
   _field_tlv_t *tlv = NULL;        /* Tlv data */
    uint8 *ptr;                     /* Scache Pointer */
    uint32 endmarkerstage  = _FIELD_WB_EM_STAGE;
                                    /* endmarker for stage */
    uint32 endmarkerrange  = _FIELD_WB_EM_RANGE;
                                    /* endmarker for range */
    uint32 endmarkermeter  = _FIELD_WB_EM_METER;
                                    /* endmarker for meter */
    uint32 endmarkercntr   = _FIELD_WB_EM_CNTR;
                                    /* endmarker for cntr */
    int  num_pipes = 0;             /* Number of pipes */
    _field_control_t *fc;           /* Field control structure. */
    uint32 *position;               /* Scache Position */
    int range_count = 0;            /* Count of ranges */
    _field_range_t *fr;             /* field Range structure */
    int type_range[7] =  {          /* Types for _field_range_t structure */
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              |_bcmFieldInternalRangeFlags),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              |_bcmFieldInternalRangeRid),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              |_bcmFieldInternalRangeMin),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              |_bcmFieldInternalRangeMax),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              |_bcmFieldInternalRangeHwIndex),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              |_bcmFieldInternalRangeStyle),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              |_bcmFieldInternalEndStructRanges)
     };
    int type_meter[8] = {           /* Types for _field_meterpool_t structure */
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalMeterLevel),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalMeterSliceId),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalMeterSize),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalMeterPoolSize),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalMeterFreeMeters),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalMeterNumMeterPairs),
       (int)((_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalMeterBmp),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalEndStructMeter)
    };
    int type_cntr[5] = {            /* Types for _field_cntrpool_t structure */
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalCntrSliceId),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalCntrSize),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalCntrFreeCntrs),
       (int)((_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalCntrBmp),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalEndStructCntr)
   };

   int type_ltconf[9] = {          /* Types for _field_ltinfo_t structure */
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigValid),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigLtId),
       (int)((_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigLtPartPri),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigLtPartMap),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigLtActionPri),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigPri),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigFlags),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalLtConfigEntry),
       (int)((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
              | _bcmFieldInternalEndStructLtConfig)
    };
    int  i = 0,j = 0;               /* Local loop variable */

    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        num_pipes = 1;
    } else if (stage_fc->oper_mode == bcmFieldGroupOperModePipeLocal) {
        num_pipes = stage_fc->num_pipes; 
    }

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    TLV_CREATE(_bcmFieldInternalStage,
                            _bcmFieldInternalVariable,0, &tlv);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageOperMode,
                                        _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc-> oper_mode);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageStageid,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc-> stage_id);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageFlags,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageTcamSz,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->tcam_sz);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageTcamSlices,
                            _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(stage_fc->tcam_slices);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageNumInstances,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->num_instances);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageNumPipes,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->num_pipes);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageRangeId,
                            _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(stage_fc->range_id);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageNumMeterPools,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->num_meter_pools);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageNumCntrPools,
                            _bcmFieldInternalVariable,0,&tlv);
    tlv->value = &(stage_fc->num_cntr_pools);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageLtTcamSz,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc-> lt_tcam_sz);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageNumLogicalTables,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc-> num_logical_tables);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageExtLevels,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc-> num_ext_levels);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStageLtActionPri,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc-> lt_action_pri);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalStagePreselQset, _bcmFieldInternalArray,
               _SHR_BITDCLSIZE(BCM_FIELD_QUALIFY_MAX), &tlv);
    tlv->value = &(stage_fc->presel_qset.w);
    TLV_WRITE(unit, tlv, ptr, position);

    /* _field_range_t */
    fr = stage_fc->ranges;
    while (fr != NULL) {
    range_count ++;
    fr = fr ->next;
    }

    /* In the encoded length below , first ten bits is value 7 [no of types],
     * next 22 bits is range count. Here number of types indicate counts of
     * elements in the current datastructure.
     */


    TLV_CREATE(_bcmFieldInternalStageRanges,
                           _bcmFieldInternalVariable,range_count
                           | (7 << _FP_WB_TLV_TYPE_SHIFT), &tlv);
    tlv->value = &(type_range);
    TLV_WRITE(unit, tlv, ptr, position);

    fr = stage_fc->ranges;
    while (fr != NULL) {

        TLV_CREATE(_bcmFieldInternalRangeFlags,
                                _bcmFieldInternalVariable,0, &tlv);
        tlv->value = &(fr->flags);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalRangeRid,
                                _bcmFieldInternalVariable,0, &tlv);
        tlv->value = &(fr->rid);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalRangeMin,
                                _bcmFieldInternalVariable,0, &tlv);
        tlv->value = &(fr->min);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalRangeMax,
                                _bcmFieldInternalVariable,0, &tlv);
        tlv->value = &(fr->max);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalRangeHwIndex,
                                _bcmFieldInternalVariable,0, &tlv);
        tlv->value = &(fr->hw_index);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalRangeStyle,
                                _bcmFieldInternalVariable,0, &tlv);
        tlv->value = &(fr->style);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalEndStructRanges,
                                _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(endmarkerrange);
        TLV_WRITE(unit, tlv, ptr, position);

        fr = fr ->next;
    }

    /* _field_meter_pool_t */

    /* In the encoded length below , first ten bits is value 8 [no of types],
     * next 12 bits is num_meter_pools and next 10 bits is num_pipes.Here number
     * of types indicate counts of elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalStageMeterPool, _bcmFieldInternalVariable,
           ((num_pipes) | (stage_fc->num_meter_pools << 10)
           | (8 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &(type_meter);
    TLV_WRITE(unit, tlv, ptr, position);

    for (i = 0; i < num_pipes; i++) {
        for (j = 0; j < stage_fc->num_meter_pools; j++) {

            TLV_CREATE(_bcmFieldInternalMeterLevel,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(stage_fc->meter_pool[i][j]->level);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalMeterSliceId,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(stage_fc->meter_pool[i][j]->slice_id);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalMeterSize,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(stage_fc->meter_pool[i][j]->size);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalMeterPoolSize,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &( stage_fc->meter_pool[i][j]->pool_size);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalMeterFreeMeters,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(stage_fc->meter_pool[i][j]->free_meters);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalMeterNumMeterPairs,
                                    _bcmFieldInternalVariable,0, &tlv);
            tlv->value = &(stage_fc->meter_pool[i][j]->num_meter_pairs);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalMeterBmp, _bcmFieldInternalArray,
                       _SHR_BITDCLSIZE(stage_fc->meter_pool[i][j]->size), &tlv);
            tlv->value = (stage_fc->meter_pool[i][j]->meter_bmp.w);
            TLV_WRITE(unit, tlv, ptr, position);

            TLV_CREATE(_bcmFieldInternalEndStructMeter,
                                    _bcmFieldInternalVariable, 0, &tlv);
            tlv->value = &(endmarkermeter);
            TLV_WRITE(unit, tlv, ptr, position);

        }
    }

    /* _field_cntr_pool_t */

    /* In the encoded length below , first ten bits is value 5 [no of types],
     * next 22 bits is num_cntr_pools. Here number of types indicate counts of
     * elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalStageCntrPools,
               _bcmFieldInternalVariable,
               (stage_fc->num_cntr_pools
               | (5 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &(type_cntr);
    TLV_WRITE(unit, tlv, ptr, position);

    for (i = 0; i < stage_fc->num_cntr_pools;i++) {

    TLV_CREATE(_bcmFieldInternalCntrSliceId,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->cntr_pool[i]->slice_id);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalCntrSize,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->cntr_pool[i]->size);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalCntrFreeCntrs,
                            _bcmFieldInternalVariable,0, &tlv);
    tlv->value = &(stage_fc->cntr_pool[i]->free_cntrs);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalCntrBmp,_bcmFieldInternalArray,
               _SHR_BITDCLSIZE(stage_fc->cntr_pool[i]->size),
               &tlv);
    tlv->value = (stage_fc->cntr_pool[i]->cntr_bmp.w);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEndStructCntr,
                            _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(endmarkercntr);
    TLV_WRITE(unit, tlv, ptr, position);
    }

    /* _field_lt_info_t */
    /* In the encoded length below , first ten bits is value 9 [no of types],
     * next 12 bits is no of logical tables and next 10 bits is num_pipes.
     * Here number of types indicate counts of elements
     * in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalStageLtInfo,
                            _bcmFieldInternalVariable,
                            ((num_pipes) | (_FP_MAX_NUM_LT << 10)
                            | (9 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &(type_ltconf);
    TLV_WRITE(unit, tlv, ptr, position);

    for (i = 0; i < num_pipes;i++) {
        for (j = 0; j < _FP_MAX_NUM_LT;j++) {
            _field_ltinfo_sync(unit, fc->lt_info[i][j]);
        }
    }

    /* data_control_t sync */
    BCM_IF_ERROR_RETURN(_field_data_control_sync(unit, stage_fc));


    TLV_CREATE(_bcmFieldInternalEndStructStage,
                            _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &endmarkerstage;
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);

    return BCM_E_NONE;
}


/* Function:
 * _field_slice_sync
 *
 * Purpose:
 *    Sync _field_slice_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * slice         - (IN) Pointer to device slice structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_slice_sync(int unit, _field_slice_t *slice)
{
    _field_tlv_t *tlv = NULL;    /* Tlv structure */
    _field_control_t *fc = NULL; /* Field control structure */
    uint8 *ptr;                  /* Pointer to scache_pos */
    uint32 *position;            /* Scache position */
    uint32 endmarker = _FIELD_WB_EM_SLICE;
                                 /* End marker */
    WB_FIELD_CONTROL_GET(fc, ptr, position);


    TLV_CREATE(_bcmFieldInternalSliceNumber,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->slice_number);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceStartTcamIdx,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->start_tcam_idx);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceEntryCount,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->entry_count);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceFreeCount,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->free_count);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceCountersCount,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->counters_count);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceMetersCount,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->meters_count);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceInstalledEntriesCount,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->hw_ent_count);
    TLV_WRITE(unit, tlv, ptr, position);

    /* IFP uses global meter pools and counter pools, so no need to
     *  save below fields
     *
     * TLV_CREATE(_bcmFieldInternalSliceCounterBmp,
     *                            _bcmFieldInternalVariable, 0, &tlv);
     *tlv->value = &(slice->counter_bmp);
     *TLV_WRITE(unit, tlv, ptr, position);

     *TLV_CREATE(_bcmFieldInternalSliceMeterBmp,
     *                           _bcmFieldInternalVariable, 0, &tlv);
     *tlv->value = &(slice->meter_bmp);
     *TLV_WRITE(unit, tlv, ptr, position);
     */
    TLV_CREATE(_bcmFieldInternalSliceStageId,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->stage_id);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSlicePortPbmp,
                                 _bcmFieldInternalArray, 32, &tlv);
    tlv->value = &(slice->pbmp);
    TLV_WRITE(unit, tlv, ptr, position);

    if (slice->next != NULL) {
        TLV_CREATE(_bcmFieldInternalSliceNextSlice,
                                 _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(slice->next->slice_number);
       TLV_WRITE(unit, tlv, ptr, position);
    }
    if (slice->prev != NULL) {
        TLV_CREATE(_bcmFieldInternalSlicePrevSlice,
                                 _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(slice->prev->slice_number);
       TLV_WRITE(unit, tlv, ptr, position);
    }

    TLV_CREATE(_bcmFieldInternalSliceGroupFlags,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->group_flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceFlags,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->slice_flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceLtMap,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->lt_map);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalSliceLtPartitionPri,
                                       _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(slice->lt_partition_pri);
    TLV_WRITE(unit, tlv, ptr, position);

    BCM_IF_ERROR_RETURN(_field_extractor_sync(unit, slice->ext_sel,
                                                    _FP_MAX_NUM_LT));

    TLV_CREATE(_bcmFieldInternalEndStructSlice,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &endmarker;
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);
    return BCM_E_NONE;
}


/* Function:
 * _field_lt_slice_sync
 *
 * Purpose:
 *    Sync _field_lt_slice_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * lt_slice      - (IN) Pointer to device lt_slice structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_lt_slice_sync(int unit, _field_lt_slice_t *lt_slice)
{

    _field_tlv_t *tlv = NULL;    /* Tlv structure */
    _field_control_t *fc = NULL; /* field control structure */
    uint8 *ptr;                  /* pointer to scache_pos */
    uint32 *position;            /* Scache position */
    uint32 endmarker = _FIELD_WB_EM_LTSLICE;
                                 /* End marker */

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtSliceSliceNum,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(lt_slice->slice_number);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtSliceStartTcamIdx,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(lt_slice->start_tcam_idx);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtSliceEntryCount,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(lt_slice->entry_count);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtSliceFreeCount,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(lt_slice->free_count);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtSliceStageid,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(lt_slice->stage_id);
    TLV_WRITE(unit, tlv, ptr, position);

    if (lt_slice->next != NULL) {
        TLV_CREATE(_bcmFieldInternalLtSliceNextSlice,
                                 _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(lt_slice->next->slice_number);
        TLV_WRITE(unit, tlv, ptr, position);
    }
    if (lt_slice->prev != NULL) {
        TLV_CREATE(_bcmFieldInternalLtSlicePrevSlice,
                                 _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(lt_slice->prev->slice_number);
        TLV_WRITE(unit, tlv, ptr, position);
    }

    TLV_CREATE(_bcmFieldInternalLtSliceFlags,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(lt_slice->slice_flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalLtSliceGroupFlags,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(lt_slice->group_flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEndStructSlice,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &endmarker;
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);
    return BCM_E_NONE;

}

/* Function:
 * _field_actions_sync
 *
 * Purpose:
 *    Sync _field_action_t structure
 *    Mirror actions need special treatment as they are the only actions which
 *    can be added multiple times.
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * _field_control_t  - (IN) _field_control_t structure pointer
 * _field_action_t   - (IN) Pointer to action structure
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_actions_sync(int unit, _field_action_t *act)
{

    _field_tlv_t *tlv = NULL;            /* pointer to tlv structure */
    _field_control_t *fc = NULL;         /* Pointer to field control structure*/
    uint8 *ptr;                          /* Pointer to scache_pos (offset) in
                                          * field control structure
                                          */
    uint32 *position;                    /* Scache position */
    _field_action_bmp_t action_bmp;      /* bit map of actions used for entry*/
    _field_action_t *f_act = NULL;       /* pointer to action structure */
    int size, param_id, action_id, mirror_id;
                                         /* loop variables */
    int hw_count = 0, count = 0,count1 = 0;
                                         /* count of params and hw index */
    int val_array[100] = { -1 };         /* Array to hold values of
                                          * params for some actions.Count is 100
                                          * since we cannot predict what
                                          * type of actions will be added
                                          */
    int val_array1[100] = { -1 };        /* Array to hold values of
                                          * params for some actions.Count is 100
                                          * since we cannot predict what
                                          * type of actions will be added
                                          */

    int hw_idx_array[100] = { -1 };      /* Array to hold hw_index in actions */
    int mirror_params_array[(BCM_MIRROR_MTP_COUNT * 2)] = { -1 };
                                         /* params of mirror actions */
    int mirror_hw_idx_array[BCM_MIRROR_MTP_COUNT] = { -1 };
                                         /* hw_index for mirror actions */
    _field_action_t *fa = NULL;          /* Pointer to actions structure */

    f_act = act;
    WB_FIELD_CONTROL_GET(fc, ptr, position);
    action_bmp.w = NULL;
    _FP_XGS3_ALLOC(action_bmp.w, SHR_BITALLOCSIZE(bcmFieldActionCount),
                    "Action BMP");
    if (action_bmp.w == NULL) {
        return BCM_E_MEMORY;
    }

    while (act != NULL) {
        _FP_ACTION_BMP_ADD(action_bmp, act->action);
        act = act->next;
    }
    for (action_id = 0; action_id < bcmFieldActionCount; action_id++) {
        if (!_FP_ACTION_BMP_TEST(action_bmp, action_id)) {
                continue;
        }
        act = f_act;
        while (act->action != action_id) {
            act = act->next;
        }
        switch (act->action) {
            case bcmFieldActionCosQNew:
            case bcmFieldActionGpCosQNew:
            case bcmFieldActionYpCosQNew:
            case bcmFieldActionRpCosQNew:
            case bcmFieldActionUcastCosQNew:
            case bcmFieldActionGpUcastCosQNew:
            case bcmFieldActionYpUcastCosQNew:
            case bcmFieldActionRpUcastCosQNew:
            case bcmFieldActionMcastCosQNew:
            case bcmFieldActionGpMcastCosQNew:
            case bcmFieldActionYpMcastCosQNew:
            case bcmFieldActionRpMcastCosQNew:
            case bcmFieldActionEgressClassSelect:
            case bcmFieldActionHiGigClassSelect:
            case bcmFieldActionFabricQueue:
            case bcmFieldActionL3Switch:
            case bcmFieldActionRedirectMcast:
            case bcmFieldActionRedirectEgrNextHop:
            case bcmFieldActionRedirectIpmc:
            case bcmFieldActionRedirectCancel:
            case bcmFieldActionOuterVlanNew:
                for (param_id = 0; param_id < 1; param_id++) {
                     val_array[count++]= act->param[param_id];
                     val_array1[count1++]= act->param[param_id];

                 }
                 break;
            case bcmFieldActionCopyToCpu:
            case bcmFieldActionTimeStampToCpu:
            case bcmFieldActionRpCopyToCpu:
            case bcmFieldActionRpTimeStampToCpu:
            case bcmFieldActionYpCopyToCpu:
            case bcmFieldActionYpTimeStampToCpu:
            case bcmFieldActionGpCopyToCpu:
            case bcmFieldActionGpTimeStampToCpu:
                 for (param_id = 0; param_id < 2; param_id++) {
                     val_array1[count1++]= act->param[param_id];
                 }
                 break;
            case bcmFieldActionDstMacNew:
            case bcmFieldActionSrcMacNew:
            case bcmFieldActionUnmodifiedPacketRedirectPort:
            case bcmFieldActionRedirect:
            case bcmFieldActionEtagNew:
            case bcmFieldActionVnTagNew:
            case bcmFieldActionRedirectTrunk:
                 for (param_id = 0; param_id < 2; param_id++) {
                     val_array[count++]= act->param[param_id];
                     val_array1[count1++]= act->param[param_id];
                 }
                 break;
            case bcmFieldActionRedirectPbmp:
            case bcmFieldActionEgressMask:
            case bcmFieldActionEgressPortsAdd:
            case bcmFieldActionRedirectBcastPbmp:
                 for (param_id = 0; param_id < 4; param_id++) {
                      val_array[count++]= act->param[param_id];
                      val_array1[count1++]= act->param[param_id];
                 }
                 break;
            case bcmFieldActionMirrorIngress:
            case bcmFieldActionMirrorEgress:
                 fa = act;
                 for (mirror_id = 0; mirror_id < (BCM_MIRROR_MTP_COUNT * 2);
                      mirror_id++) {
                      mirror_params_array[mirror_id] = -1;
                 }
                 mirror_id = 0;
                 /* For all remaining mirror actions add parameters now */
                 while (fa != NULL) {
                        if (fa ->action == action_id) {
                            for (param_id = 0; param_id < 2; param_id++) {
                                 mirror_params_array[mirror_id++] =
                                 fa->param[param_id];
                            }
                        }
                        fa = fa->next;
                 }
                 /* Mirror will have BCM_MIRROR_MTP_COUNT * 2 parameters */
                 for (mirror_id = 0;mirror_id < (BCM_MIRROR_MTP_COUNT * 2);
                      mirror_id++) {
                      val_array[count++] = mirror_params_array[mirror_id];
                      val_array1[count1++] = mirror_params_array[mirror_id];

                 }
                 break;
            case bcmFieldActionRedirectVlan:
            case bcmFieldActionVnTagDelete:
            case bcmFieldActionEtagDelete:
            default:
                   break;
        }
        switch (act->action) {
            case bcmFieldActionDstMacNew:
            case bcmFieldActionSrcMacNew:
            case bcmFieldActionOuterVlanNew:
            case bcmFieldActionVnTagNew:
            case bcmFieldActionVnTagDelete:
            case bcmFieldActionEtagNew:
            case bcmFieldActionEtagDelete:
            case bcmFieldActionRedirectPbmp:
            case bcmFieldActionEgressMask:
            case bcmFieldActionEgressPortsAdd:
            case bcmFieldActionRedirectBcastPbmp:
            case bcmFieldActionRedirectEgrNextHop:
            case bcmFieldActionL3Switch:
                 hw_idx_array[hw_count++] = act->hw_index;
                 break;
            case bcmFieldActionMirrorIngress:
            case bcmFieldActionMirrorEgress:
                 fa = act;
                 for (mirror_id = 0; mirror_id < BCM_MIRROR_MTP_COUNT;
                      mirror_id++) {
                      mirror_hw_idx_array[mirror_id]= -1;
                 }
                 mirror_id = 0;
                 /* For all remaining mirror actions add hw_idx now */
                 while (fa != NULL) {
                        if (fa ->action == action_id) {
                            mirror_hw_idx_array[mirror_id++]= fa->hw_index;
                        }
                        fa = fa->next;
                 }
                 /* Mirror will have BCM_MIRROR_MTP_COUNT hw indices */
                 for (mirror_id = 0; mirror_id < BCM_MIRROR_MTP_COUNT;
                     mirror_id++) {
                     hw_idx_array[hw_count++] = mirror_hw_idx_array[mirror_id];
                 }

            default:
                break;
        }


    }
    size = SHR_BITALLOCSIZE(bcmFieldActionCount);
    size = (size-1)/static_type_map[_bcmFieldInternalEntryActionsPbmp].size + 1;
    TLV_CREATE(_bcmFieldInternalEntryActionsPbmp, _bcmFieldInternalArray,
                size, &tlv);
    tlv->value = action_bmp.w;
    TLV_WRITE(unit, tlv, ptr, position);

    if (val_array[0] != -1) {
        TLV_CREATE(_bcmFieldInternalActionParam, _bcmFieldInternalArray,
                    count, &tlv);
        tlv->value = &val_array[0];
        TLV_WRITE(unit, tlv, ptr, position);
    }

    if (val_array1[0] != -1) {
        TLV_CREATE(_bcmFieldInternalActionParam1, _bcmFieldInternalArray,
                    count1, &tlv);
        tlv->value = &val_array1[0];
        TLV_WRITE(unit, tlv, ptr, position);
    }

    if (hw_idx_array[0] != -1) {
        TLV_CREATE(_bcmFieldInternalActionHwIdx, _bcmFieldInternalArray,
                                hw_count, &tlv);
        tlv->value = &hw_idx_array[0];
        TLV_WRITE(unit, tlv, ptr, position);
    }

    if (action_bmp.w !=NULL) {
        _FP_ACTION_BMP_FREE(action_bmp);
    }

    sal_free(tlv);
    return BCM_E_NONE;
}

/* Function:
 * _field_statistic_sync
 *
 * Purpose:
 *    sync _field_entry_stat_t structure
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * fc                - (IN) _field_control_t structure pointer
 * stat              - (IN) Pointer to _field_entry_stat_t structure.
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_statistic_sync(int unit, _field_control_t *fc,
                         _field_entry_stat_t stat) {

    _field_tlv_t *tlv = NULL; /* pointer to tlv structure */
    uint8 *ptr;               /* pointer to scache_pos (offset) */
    uint32 *position;         /* Scache position */

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryStatSid,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(stat.sid);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryStatExtendedSid,
                                          _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(stat.extended_sid);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryStatFlags,
                                           _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(stat.flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryStatAction,
                                         _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(stat.stat_action);
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);
    return BCM_E_NONE;

}
/* Function:
 * _field_lt_entry_sync
 *
 * Purpose:
 *    Sync _field_lt_entry_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * f_lt_ent      - (IN) _field_lt_entry_t structure.
 * parts_count   - (IN) Number of TCAM Parts.
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_lt_entry_info_sync(int unit, _field_lt_entry_t *f_lt_ent,
                          int parts_count)
{
    _field_tlv_t *tlv = NULL;        /* tlv data */
    int j = 0;                       /* j -> loop variable */
    uint32 end_marker = _FIELD_WB_EM_LTENTRY;
                                     /* end marker for the LT entry structure */
    uint8 *ptr;                      /* pointer to scache_pos(offset)      */
    _field_control_t *fc = NULL;     /* pointer to field_control structure */
    int temp;                        /* temporary variable */
    uint32 *position;                /* Scache position */

    int  flag_enums[3] = {  _bcmFieldInternalEntryFlagsPart1,
                            _bcmFieldInternalEntryFlagsPart2,
                            _bcmFieldInternalEntryFlagsPart3
                         };          /* If there are multiple parts in an
                                      * lt_entry, only flags differ
                                      * for each part. Save complete info about
                                      * part1, and only flags info for
                                      * the remaining parts using FlagsPart2 and
                                      * so on.
                                      */


    WB_FIELD_CONTROL_GET(fc, ptr, position);

    for (j = 0; j < parts_count; j++) {
        TLV_CREATE(flag_enums[j], _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &((f_lt_ent+j)->flags);
        TLV_WRITE(unit, tlv, ptr, position);
    }

    TLV_CREATE(_bcmFieldInternalEntryEid,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_lt_ent->eid);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntrySliceId,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_lt_ent->index);
    TLV_WRITE(unit, tlv, ptr,position);

    TLV_CREATE( _bcmFieldInternalEntryPrio,
                                   _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_lt_ent->prio);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalEntryGroup,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_lt_ent->group->gid);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalEntrySlice,
                                  _bcmFieldInternalVariable, 0, &tlv);
    temp  = ((int) (f_lt_ent->lt_fs->slice_number));
    tlv->value = &temp;
    TLV_WRITE(unit, tlv, ptr, position);

    /* Currently LT actions support only one action */
    if (f_lt_ent->actions != NULL) {
        TLV_CREATE(_bcmFieldInternalGroupClassAct,
                                            _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(f_lt_ent->actions->param[0]);
        TLV_WRITE(unit, tlv, ptr, position);
    }

    TLV_CREATE(_bcmFieldInternalEndStructEntry,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &end_marker;
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);
    return BCM_E_NONE;

}

/* Function:
 * _field_entry_info_sync
 *
 * Purpose:
 *    Sync _field_entry_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * f_ent         - (IN) _field_entry_t structure.
 * fc            - (IN) _field_control_t structure.
 * parts_count   - (IN) Number of TCAM Parts.
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_entry_info_sync(int unit, _field_entry_t *f_ent, int parts_count)
{
    _field_tlv_t *tlv = NULL;     /* tlv structure  */
    uint32 end_marker = _FIELD_WB_EM_ENTRY;
                                  /* end marker */
    uint8 *ptr;                   /* pointer to scache_structure */
    _field_control_t *fc = NULL;  /* pointer to field_control_t structure */
    int j = 0;                    /* j - loop variable */
    int temp;                     /* temporary variable */
     int  flag_enums[_FP_TH_MAX_ENTRY_WIDTH] = {
                            _bcmFieldInternalEntryFlagsPart1,
                            _bcmFieldInternalEntryFlagsPart2,
                            _bcmFieldInternalEntryFlagsPart3
                          };
                                 /* If there are multiple parts in an entry,
                                  * only flags differ.
                                  * save complete info about part1,
                                  * and only flags info for the
                                  * remaining parts using FlagsPart2
                                  * and soon.
                                  */
    int type_entrypolicer[_FP_TH_MAX_ENTRY_WIDTH] = {
    (int) (_bcmFieldInternalEntryPolicerPid
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalEntryPolicerFlags
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
    (int) (_bcmFieldInternalEndStructEntPolicer
           | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT))
                                };

                                 /* Holds types for all fields
                                  * in _field_entry_policer_t structure
                                  */
    uint32 *position;            /* Scache position */

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryEid,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->eid);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE( _bcmFieldInternalEntryPrio,
                                   _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->prio);
    TLV_WRITE(unit, tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalEntrySliceId,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->slice_idx);
    TLV_WRITE(unit, tlv, ptr,position);

    for (j = 0; j < parts_count; j++) {
        TLV_CREATE(flag_enums[j], _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &((f_ent+j)->flags);
        TLV_WRITE(unit, tlv, ptr, position);
    }

    TLV_CREATE(_bcmFieldInternalEntryPbmpFullData,
               _bcmFieldInternalArray, _SHR_PBMP_WORD_MAX, &tlv);
    tlv->value = &(f_ent->pbmp.data.pbits);
    TLV_WRITE(unit,tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryPbmpFullMask,
               _bcmFieldInternalArray, _SHR_PBMP_WORD_MAX, &tlv);
    tlv->value = &(f_ent->pbmp.mask.pbits);
    TLV_WRITE(unit,tlv, ptr, position);


    TLV_CREATE(_bcmFieldInternalEntryGroup,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->group->gid);
    TLV_WRITE(unit, tlv, ptr, position);


    if (f_ent->fs != NULL) {
        TLV_CREATE(_bcmFieldInternalEntrySlice,
                                  _bcmFieldInternalVariable, 0, &tlv);
        temp = ((int) (f_ent->fs->slice_number));
        tlv->value = &temp;
        TLV_WRITE(unit, tlv, ptr, position);
    }

    if (f_ent->actions != NULL) {
        BCM_IF_ERROR_RETURN(_field_actions_sync(unit, f_ent->actions));
    }
    _field_statistic_sync(unit, fc, (f_ent->statistic));

    /* In the encoded length below , first ten bits is value 3 [no of types],
     * next 22 bits is policer level count. Here number of types indicate counts
     * of elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalEntryPolicer,
               _bcmFieldInternalVariable, (_FP_POLICER_LEVEL_COUNT
               | (3 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);

    tlv->value = &type_entrypolicer[0];
    TLV_WRITE(unit, tlv, ptr, position);

    for (j = 0; j < _FP_POLICER_LEVEL_COUNT; j++) {
        TLV_CREATE(_bcmFieldInternalEntryPolicerPid,
               _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(f_ent->policer[j].pid);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalEntryPolicerFlags,
                               _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(f_ent->policer[j].flags);
        TLV_WRITE(unit, tlv, ptr, position);

        TLV_CREATE(_bcmFieldInternalEndStructEntPolicer,
                                     _bcmFieldInternalVariable, 0, &tlv);
        TLV_WRITE(unit, tlv, ptr, position);
    }

    TLV_CREATE(_bcmFieldInternalGlobalEntryPolicerPid,
                                          _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->global_meter_policer.pid);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalGlobalEntryPolicerFlags,
                                          _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->global_meter_policer.flags);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryIngMtp,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->ing_mtp_slot_map);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryEgrMtp,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->egr_mtp_slot_map);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryDvp,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->dvp_type);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntrySvpType,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->svp_type);
    TLV_WRITE(unit, tlv, ptr, position);

    TLV_CREATE(_bcmFieldInternalEntryTcamIptype,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(f_ent->tcam.ip_type);
    TLV_WRITE(unit, tlv, ptr, position);


    /* Disabling saveing entry copy for now to reduce scache_size
     *  if (f_ent->ent_copy != NULL) {
     */
    if (0) {
        TLV_CREATE(_bcmFieldInternalEntryCopy,
                                         _bcmFieldInternalVariable, 0, &tlv);
        TLV_WRITE(unit, tlv, ptr, position);

        _field_entry_info_sync(unit, f_ent->ent_copy, parts_count);
    }
    TLV_CREATE(_bcmFieldInternalEndStructEntry,
                                  _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &end_marker;
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);
    return BCM_E_NONE;

}
/* Function:
 * _field_entry_sync
 *
 * Purpose:
 *    call functions for _field_entry_sync and _field_lt_entry_sync.
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_entry_sync(int unit)
{
    _field_tlv_t *tlv = NULL;        /* Pointer to tlv structure */
    _field_control_t *fc = NULL;     /* Pointer to field control structure. */
    _field_group_t *fg = NULL;       /* Pointer to traverse over groups */
    _field_entry_t *f_ent = NULL;    /* to traverse entries in each group */
    _field_lt_entry_t *f_lt_ent = NULL; /*to traverse over lt_entries */
    int parts_count = 0;             /* Number of parts for that group,
                                       interslice double wide can have two and
                                       interslice triple wide can have three */
    int  i = 0;                      /* local variable */
    uint8 *ptr;                      /* pointer to scache position(offset)  */
    uint32 *position;                /* Scache position */

    WB_FIELD_CONTROL_GET(fc, ptr, position);

    fg = fc->groups;
    while (fg != NULL) {
        if (fg->stage_id != _BCM_FIELD_STAGE_INGRESS) {
            fg = fg->next;
            continue;
        }
        BCM_IF_ERROR_RETURN(_bcm_field_entry_tcam_parts_count (unit,
                    fg->stage_id, fg->flags, &parts_count));

        TLV_CREATE(_bcmFieldInternalGroupPartCount,
                                      _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &parts_count;
        TLV_WRITE(unit, tlv, ptr, position);

        /* _field_entry_t */
        TLV_CREATE(_bcmFieldInternalGroupEntry,
                                      _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(fg->group_status.entry_count);
        TLV_WRITE(unit, tlv, ptr, position);

        for (i = 0; i < fg->group_status.entry_count; i++) {
            if (fg->entry_arr[i] == NULL) {
                continue;
            }
            f_ent = fg->entry_arr[i];
            _field_entry_info_sync(unit, f_ent, parts_count);
        }

        /* _field_ltentry_t */
        TLV_CREATE(_bcmFieldInternalGroupLtEntry,
                                         _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(fg->lt_grp_status.entry_count);
        TLV_WRITE(unit, tlv, ptr, position);


        for (i = 0; i < (fg->lt_grp_status.entry_count); i++) {
            if (fg->lt_entry_arr[i] == NULL) {
                continue;
            }
            f_lt_ent = fg->lt_entry_arr[i];
            _field_lt_entry_info_sync(unit, f_lt_ent, parts_count);
        }

        fg = fg->next;
    }

    TLV_CREATE(_bcmFieldInternalEndStructEntryDetails,
                            _bcmFieldInternalVariable, 0, &tlv);
    TLV_WRITE(unit, tlv, ptr, position);

    sal_free(tlv);
    return BCM_E_NONE;

}


/* Function:
 * _field_group_qualifier_sync
 *
 * Purpose:
 *    Sync _bcm_field_group_qual_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * qual_arr      - (IN) Pointer to field group qual structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_group_qualifier_sync(int unit,
                            _bcm_field_group_qual_t *qual_arr)
{
    _field_tlv_t *tlv = NULL;       /* TLV data */
    uint8 *scache_ptr = NULL;       /* Pointer to Scache */
    _bcm_field_qual_offset_t *offset = NULL;
                                    /* Pointer to Qualifier offset */
    _field_control_t *fc;           /* Field control structure. */
    int  i =0;                      /* Loop local variable */
    uint32 endmarker = _FIELD_WB_EM_QUAL;
                                    /* End marker */
    uint32 endmarkeroffset = _FIELD_WB_EM_QUALOFFSET;
                                    /* End marker */
    uint32 *position;               /* pointer to scache position */
    int type_qual[7] =              /* types in _bcm_field_qual_offset_t */
    {
     (int)(_bcmFieldInternalQualOffsetNumOffset
            | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
     (int)(_bcmFieldInternalQualOffsetOffsetArr
            | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
     (int)(_bcmFieldInternalQualOffsetWidth
            | (_bcmFieldInternalArray << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
     (int)(_bcmFieldInternalQualOffsetSec
            | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
     (int)(_bcmFieldInternalQualOffsetBitPos
            | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
     (int)(_bcmFieldInternalQualOffsetQualWidth16
            | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)),
     (int)(_bcmFieldInternalEndStructQualOffset
            | (_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT))

    };

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalQualQid, _bcmFieldInternalArray,
                qual_arr->size, &tlv);
    tlv->value = qual_arr->qid_arr;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalQualSize, _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &(qual_arr->size);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    /* In the encoded length below , first ten bits is value 8 [no of types],
     * next 22 bits is qualifier array size . Here number of types indicate
     * counts of elements in the current datastructure.
     */

    TLV_CREATE(_bcmFieldInternalQualOffset, _bcmFieldInternalArray,
                    (qual_arr->size | (7 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &type_qual;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    offset = qual_arr->offset_arr;
    for (i = 0; i < qual_arr->size; i++) {

        TLV_CREATE(_bcmFieldInternalQualOffsetNumOffset,
                   _bcmFieldInternalVariable,0, &tlv);
        tlv->value = &offset->num_offsets;
        TLV_WRITE(unit, tlv, scache_ptr, position);

        TLV_CREATE(_bcmFieldInternalQualOffsetOffsetArr,
                    _bcmFieldInternalArray, _BCM_FIELD_QUAL_OFFSET_MAX, &tlv);
        tlv->value = &offset->offset;
        TLV_WRITE(unit, tlv, scache_ptr, position);

        TLV_CREATE(_bcmFieldInternalQualOffsetWidth,
                   _bcmFieldInternalArray, _BCM_FIELD_QUAL_OFFSET_MAX, &tlv);
        tlv->value = &offset->width;
        TLV_WRITE(unit, tlv, scache_ptr, position);

        TLV_CREATE(_bcmFieldInternalQualOffsetSec,
                   _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &offset->secondary;
        TLV_WRITE(unit, tlv, scache_ptr, position);

        TLV_CREATE(_bcmFieldInternalQualOffsetBitPos,
                   _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &offset->bit_pos;
        TLV_WRITE(unit, tlv, scache_ptr, position);

        TLV_CREATE(_bcmFieldInternalQualOffsetQualWidth16,
                   _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &offset->qual_width;
        TLV_WRITE(unit, tlv, scache_ptr, position);


        TLV_CREATE(_bcmFieldInternalEndStructQualOffset,
                   _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &endmarkeroffset;
        TLV_WRITE(unit, tlv, scache_ptr, position);

        offset++;
    }

    TLV_CREATE(_bcmFieldInternalEndStructGroupQual,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &endmarker;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    return BCM_E_NONE;
}


/* Function:
 * _field_group_sync
 *
 * Purpose:
 *    Sync _field_group_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * fg         - (IN) Pointer to device group structure
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_group_sync(int unit, _field_group_t *fg)
{
    _field_tlv_t *tlv = NULL;       /* TLV data */
    uint8 *scache_ptr = NULL;       /* Pointer to Scache */
    _field_control_t *fc;           /* Field control structure. */
    uint32 endmarker = _FIELD_WB_EM_GROUP; /* End marker */
    int parts_count = 0;            /* Number of entry parts. */
    int rv = 0;                     /* Return Variable */
    int partidx = 0;                /* Id to field entry part */
    int temp = 0;                   /* temporary variable */
    uint32 *position;               /* Pointer to scache pos */
    uint32 qual_count = bcmFieldQualifyCount;
                                    /* Qualifier Count */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupId,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->gid;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupPri,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->priority;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    /* this could be synced once but it is synced
       per group for easy processing */

    TLV_CREATE(_bcmFieldInternalQualifyCount,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &qual_count;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalQsetW, _bcmFieldInternalArray,
               _SHR_BITDCLSIZE(_bcmFieldQualifyCount), &tlv);
    tlv->value = &fg->qset.w;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalQsetUdfMap, _bcmFieldInternalArray,
               _SHR_BITDCLSIZE(BCM_FIELD_USER_NUM_UDFS), &tlv);
    tlv->value = &fg->qset.udf_map;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupFlags,_bcmFieldInternalVariable,0,&tlv);
    tlv->value = &fg->flags;
    TLV_WRITE(unit, tlv, scache_ptr, position);


    TLV_CREATE(_bcmFieldInternalGroupInstance,_bcmFieldInternalVariable,0,&tlv);
    tlv->value = &fg->instance;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    if (fg->slices != NULL) {
        TLV_CREATE(_bcmFieldInternalGroupSlice, _bcmFieldInternalVariable,0, &tlv);
        temp = (int) fg->slices->slice_number;
        tlv->value = &temp;
        TLV_WRITE(unit, tlv, scache_ptr, position);
    }

    TLV_CREATE(_bcmFieldInternalGroupPbmp, _bcmFieldInternalArray,64, &tlv);
    tlv->value = &fg->pbmp;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    /* Get number of entry parts we have to read. */
    rv = _bcm_field_entry_tcam_parts_count (unit, fg->stage_id,
                                            fg->flags, &parts_count);

    TLV_CREATE(_bcmFieldInternalGroupQual,
               _bcmFieldInternalArray, parts_count, &tlv);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    for (partidx = 0; partidx < parts_count; partidx++) {
        BCM_IF_ERROR_RETURN( _field_group_qualifier_sync(unit,
                     &(fg->qual_arr[_FP_ENTRY_TYPE_DEFAULT][partidx])));
    }

    TLV_CREATE(_bcmFieldInternalGroupPreselQual,
               _bcmFieldInternalArray, parts_count, &tlv);
    TLV_WRITE(unit, tlv, scache_ptr, position);

    for (partidx = 0; partidx < parts_count; partidx++) {
        BCM_IF_ERROR_RETURN( _field_group_qualifier_sync(unit,
                     &(fg->presel_qual_arr[_FP_ENTRY_TYPE_DEFAULT][partidx])));
    }

    TLV_CREATE(_bcmFieldInternalGroupStage, _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->stage_id;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupBlockCount,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->ent_block_count;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupGroupStatus, _bcmFieldInternalArray,
               (sizeof(bcm_field_group_status_t)/sizeof(int)), &tlv);
    tlv->value = &fg->group_status;
    TLV_WRITE(unit, tlv, scache_ptr, position);


    TLV_CREATE(_bcmFieldInternalGroupGroupAset, _bcmFieldInternalArray,
               _SHR_BITDCLSIZE(bcmFieldActionCount), &tlv);
    tlv->value = &fg->aset.w;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupCounterBmp, _bcmFieldInternalArray,
                _FIELD_MAX_COUNTER_POOLS, &tlv);
    tlv->value = &fg->counter_pool_bmp.w;
    TLV_WRITE(unit, tlv, scache_ptr, position);


    if (fg->lt_slices != NULL) {
        TLV_CREATE(_bcmFieldInternalGroupLtSlice, _bcmFieldInternalVariable,0,
                   &tlv);
        temp = (int) fg->lt_slices->slice_number;
        tlv->value = &temp;
        TLV_WRITE(unit, tlv, scache_ptr, position);
    }

    BCM_IF_ERROR_RETURN(_field_extractor_sync(unit,
                        (fg->ext_codes), _FP_MAX_ENTRY_WIDTH));

    /* _field_ltinfo_t -> we save lt_id and link to the _field_lt_info_t
       structure recovered during stage reinit */

    TLV_CREATE(_bcmFieldInternalGroupLtConfig, _bcmFieldInternalVariable,
               0, &tlv);
    tlv->value = &fg->lt_id;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupLtEntrySize,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->lt_ent_blk_cnt;
    TLV_WRITE(unit, tlv, scache_ptr, position);


    TLV_CREATE(_bcmFieldInternalLtStatusEntriesCnt,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->lt_grp_status.entry_count;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalLtStatusEntriesFree,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->lt_grp_status.entries_free;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalLtStatusEntriesTotal,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->lt_grp_status.entries_total;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupHintId,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->hintid;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupMaxSize,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->max_group_size;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalEMGroupMode,
                           _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &fg->em_mode;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    temp = _SHR_BITDCLSIZE(bcmFieldActionCount);
    TLV_CREATE(_bcmFieldInternalEMGroupAset2,
                              _bcmFieldInternalArray, temp, &tlv);
    tlv->value = &fg->aset.w;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalGroupActionProfIdx,
               _bcmFieldInternalArray, 3, &tlv);
    tlv->value = &fg->action_profile_idx;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    TLV_CREATE(_bcmFieldInternalEndStructGroup,
               _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &endmarker;
    TLV_WRITE(unit, tlv, scache_ptr, position);

    return rv;

}
/* Function:
 * _field_class_info_sync
 *
 * Purpose:
 *   sync function for _field_class_info_t structure
 *
 * Parameters:
 * unit      - (IN) unit number
 * class_info_arr  - (IN) pointer to array of pointers to structures
 * count     - (IN) Number of _field_class_info_t structure pointers
 * Returns:
 *      BCM_E_XXX
 */
int
_field_class_info_sync(int unit, _field_class_info_t **class_info_arr,
                       int count) {
    _field_tlv_t *tlv   = NULL;     /* tlv structure */
    _field_control_t *fc = NULL;    /* pointer to field control structure */
    uint32 *pos = NULL;             /* position in  fc */
    uint8 *ptr = NULL;              /* pointer to fc->scache_ptr */
    int i, bmp_length = 0, max_entries; /* loop var, bitmap length, num of entries
                                       to calculate bitmap length */
    int type_class_info[6] = {       /* types in _field_class_info_t */
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalClassFlags),
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalClassEntUsed),
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalClassBmp),
     (int) ((_bcmFieldInternalVariable << _FP_WB_TLV_BASIC_TYPE_SHIFT)
           | _bcmFieldInternalEndStructClass)
    };

    WB_FIELD_CONTROL_GET(fc, ptr, pos);

    if ((class_info_arr != NULL) && (count != 0)) {
        max_entries = (**class_info_arr).total_entries_available;
        bmp_length = SHR_BITALLOCSIZE(max_entries << 1);
    }

    TLV_CREATE(_bcmFieldInternalStageClass, _bcmFieldInternalVariable,
             (count | (4 << _FP_WB_TLV_TYPE_SHIFT)), &tlv);
    tlv->value = &type_class_info;
    TLV_WRITE(unit, tlv, ptr, pos);

    for (i=0; i < count; i++) {
        TLV_CREATE(_bcmFieldInternalClassFlags,
                            _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(class_info_arr[i]->flags);
        TLV_WRITE(unit, tlv, ptr, pos);

        TLV_CREATE(_bcmFieldInternalClassEntUsed,
                            _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &(class_info_arr[i]->total_entries_used);
        TLV_WRITE(unit, tlv, ptr, pos);

        TLV_CREATE(_bcmFieldInternalClassBmp,
                            _bcmFieldInternalVariable, bmp_length, &tlv);
        tlv->value = &(class_info_arr[i]->class_bmp);
        TLV_WRITE(unit, tlv, ptr, pos);

        TLV_CREATE(_bcmFieldInternalEndStructClass,
                            _bcmFieldInternalVariable, 0, &tlv);
        TLV_WRITE(unit, tlv, ptr, pos);

    }

    return BCM_E_NONE;
}
/* Function:
 * _bcm_field_th_stage_class_sync
 *
 * Purpose:
 *   Main sync function for stage class
 *
 * Parameters:
 * uinit      - (IN) unit number
 * fc         - (IN) pointer to field control structure
 * stage_fc   - (IN) pointer to stage class structure
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_field_th_stage_class_sync(int unit, _field_control_t *fc,
                                _field_stage_t *stage_fc)
{
    int pipe, num_pipes = 0;   /* current pipe id, number of pipes */
    _field_tlv_t *tlv = NULL;  /* tlv structure */
    _field_group_t *fg;        /* pointer to group structure */
    _field_entry_t *f_ent = NULL;
                               /* pointer to entry structure */
    uint8 *ptr;                /* pointer to position in field control  */
    uint32 *pos;               /* pointer to fc->scache_pos */
    int group_count = 0, rv = 0, i = 0;
                               /* group coutn, ret val, loop var */
    uint32 end_marker = _FIELD_WB_EM_CLASS;
                               /* end marker */

    WB_FIELD_CONTROL_GET_WITH_LOCK(fc, ptr, pos);
    curr_stage_fc = stage_fc;

    if (stage_fc->oper_mode == 0 ) {
        num_pipes = 1;
    } else {
        num_pipes = stage_fc->num_pipes;
    }

    fg = fc->groups;
    while (fg != NULL) {
        if (fg->stage_id == _BCM_FIELD_STAGE_CLASS) {
            group_count += 1;
        }
        fg = fg->next;
    }

    if (group_count > 0) {

        TLV_CREATE(_bcmFieldInternalClassOperMode,
                                     _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &stage_fc->oper_mode;
        TLV_WRITE(unit, tlv, ptr, pos);

        for (pipe=0; pipe < num_pipes; pipe++) {
            rv  = _field_class_info_sync(unit,
                                         stage_fc->class_info_arr[pipe],
                                         _FP_MAX_NUM_CLASS_TYPES);
            BCM_IF_ERROR_CLEANUP(rv);
        }

        TLV_CREATE(_bcmFieldInternalGroupCount,
                   _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &group_count;
        TLV_WRITE(unit, tlv, ptr, pos);

        /* Iterate over the groups linked-list */
        fg = fc->groups;
        while (fg != NULL) {
            if (fg->stage_id == _BCM_FIELD_STAGE_CLASS) {
                rv = _field_group_sync(unit,fg);
                BCM_IF_ERROR_CLEANUP(rv);
            }
            fg = fg->next;
        }
        fg = fc->groups;

        while (fg != NULL) {
            if (fg->stage_id != _BCM_FIELD_STAGE_CLASS) {
                fg = fg->next;
                continue;
            }

            TLV_CREATE(_bcmFieldInternalGroupEntry,
                       _bcmFieldInternalVariable, 0, &tlv);
            tlv->value = &(fg->group_status.entry_count);
            TLV_WRITE(unit, tlv, ptr, &(fc->scache_pos));

            for (i=0; i < fg->group_status.entry_count; i++) {
                if (fg->entry_arr[i] == NULL) {
                    continue;
                }
                f_ent = fg->entry_arr[i];
                /* parts_count hardcoded to 1,
                 * as compression entries cannot
                   span across multiple TCAMS */
                rv = _field_entry_info_sync(unit, f_ent, 1);
                BCM_IF_ERROR_CLEANUP(rv);
            }

            fg = fg->next;

        }
    }
    TLV_CREATE(_bcmFieldInternalEndStageClass,
                                 _bcmFieldInternalVariable, 0, &tlv);
    tlv->value = &end_marker;
    TLV_WRITE(unit, tlv, ptr, pos);

cleanup:
    sal_free(tlv);
    tlv = NULL;
    FP_UNLOCK(unit);
    return rv;

}
/*
 * Function:
 *      _field_em_group_entries_sync
 * Purpose:
 *      sync necessary information for each entry in a group
 *      save entry id, flags, (part 2 flags in case of double wide),
 *      and hw index in order for each entry. Hw index is obtained by
 *      populating key and searching for the entry in corresponding
 *      memory
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      fg       - (IN) pointer to group structure whose entries
 *                  have to be synced.
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_em_group_entries_sync(int unit, _field_group_t *fg)
{

    _field_tlv_t *tlv = NULL;               /* tlv pointer */
    _field_entry_t *f_ent = NULL, *f_ent_part = NULL;
                                            /* pointers to entry structures */
    uint8 *ptr = NULL;                      /* pointer to scache_ptr */
    uint32 *pos = NULL;                     /* pointer to scache_pos */
    _field_control_t *fc = NULL;            /* pointer to field control struc */
    _field_stage_t *stage_fc = NULL;        /* pointer to stage structure */
    int count = 0;                 /* to hold the current postion in array */
    uint32 *array = NULL;          /* to hold the values that are to be syced */
    int ent_count = 0, hw_index = 0;        /* entry count, hw index */
    int rv = 0, parts_count = 0;            /* rv, number of parts */
    exact_match_2_entry_t ebuf_nar;         /* Narrow Entry Buffer.      */
    exact_match_4_entry_t ebuf_wide;        /* Wide Entry Buffer.        */
    soc_mem_t mem;                          /* Memory Identifier.        */
    uint32 tbuf[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                            /* Temp Buffer One.          */
    uint32 tbuftemp[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                            /* Temp Buffer Two.          */
    uint32 *bufp;                           /* Hardware Buffer Ptr.      */
    int num_attr = 0;               /* Number of attributes  stored per entry */
    static soc_mem_t em_entry_nar_mem[_FP_MAX_NUM_PIPES] = {
        EXACT_MATCH_2_PIPE0m,
        EXACT_MATCH_2_PIPE1m,
        EXACT_MATCH_2_PIPE2m,
        EXACT_MATCH_2_PIPE3m,
    };
    static soc_mem_t em_entry_wide_mem[_FP_MAX_NUM_PIPES] = {
        EXACT_MATCH_4_PIPE0m,
        EXACT_MATCH_4_PIPE1m,
        EXACT_MATCH_4_PIPE2m,
        EXACT_MATCH_4_PIPE3m,
    };

    ent_count = fg->group_status.entry_count;
    WB_FIELD_CONTROL_GET(fc, ptr, pos);
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                     _BCM_FIELD_STAGE_EXACTMATCH, &stage_fc));


    /* Get Hardware Buffer Pointer and memory identifier. */
    if ((fg->em_mode == _FieldExactMatchMode128) ||
        (fg->em_mode == _FieldExactMatchMode160)) {
        bufp = (uint32 *)&ebuf_nar;
        num_attr = 5;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_2m;
        } else {
            mem = em_entry_nar_mem[fg->instance];
        }
    } else {
        bufp = (uint32 *)&ebuf_wide;
        num_attr = 6;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_4m;
        } else {
            mem = em_entry_wide_mem[fg->instance];
        }
    }
    rv = _bcm_field_entry_tcam_parts_count(unit, fg->stage_id,
                                           fg->flags, &parts_count);
    BCM_IF_ERROR_CLEANUP(rv);

    if (fg->em_mode == _FieldExactMatchMode320) {
        _FP_XGS3_ALLOC(array, num_attr * ent_count * sizeof(int),
                                                    "em entires array");
        if (array == NULL) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        while  (count  != (num_attr * ent_count)) {
            f_ent = fg->entry_arr[count/num_attr];
            array[count++] = f_ent->eid;
            array[count++] = f_ent->flags;
            array[count++] = (f_ent + 1)->flags;
            if (f_ent->statistic.flags & _FP_ENTRY_STAT_VALID) {
                array[count++] = f_ent->statistic.sid;
            } else {
                array[count++] = -1;
            }
            if (f_ent->policer[0].flags & _FP_POLICER_INSTALLED) {
                array[count++] = f_ent->policer[0].pid;
            } else {
                array[count++] = -1;
            }
            if (!(f_ent->flags & _FP_ENTRY_INSTALLED)) {
                array[count++] = -1;
                continue;
            }
            if (f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT) {
                array[count++] = -1;
                continue;
            }
            soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
            soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
            soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_2f, fg->em_mode);
            soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_3f, fg->em_mode);
            soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
            soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
            soc_mem_field32_set(unit, mem, bufp, VALID_2f, 1);
            soc_mem_field32_set(unit, mem, bufp, VALID_3f, 1);
            rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 0, 101);
            BCM_IF_ERROR_CLEANUP(rv);
            soc_mem_field_set(unit, mem, bufp, MODE320__KEY_0_ONLYf, tbuf);
            rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 101, 59);
            BCM_IF_ERROR_CLEANUP(rv);

            /* Key From Second Entry Part. */
            f_ent_part = f_ent + 1;
            if (f_ent_part->tcam.key_hw == NULL) {
                BCM_IF_ERROR_CLEANUP(BCM_E_PARAM);
            }
            rv = _bcm_field_th_val_get(f_ent_part->tcam.key_hw, tbuftemp, 0, 42);
            BCM_IF_ERROR_CLEANUP(rv);
            rv = _bcm_field_th_val_set(tbuf, tbuftemp, 59, 42);
            BCM_IF_ERROR_CLEANUP(rv);
            soc_mem_field_set(unit, mem, bufp, MODE320__KEY_1_ONLYf, tbuf);;
            rv = _bcm_field_th_val_get(f_ent_part->tcam.key_hw, tbuf, 42, 101);
            BCM_IF_ERROR_CLEANUP(rv);
            soc_mem_field_set(unit, mem, bufp, MODE320__KEY_2_ONLYf, tbuf);
            rv = _bcm_field_th_val_get(f_ent_part->tcam.key_hw, tbuf, 143, 17);
            BCM_IF_ERROR_CLEANUP(rv);
            soc_mem_field_set(unit, mem, bufp, MODE320__KEY_3_ONLYf, tbuf);

            rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &hw_index,
                                                                bufp, tbuf, 0 );
            BCM_IF_ERROR_CLEANUP(rv);
            array[count++] = hw_index;
        }

        TLV_CREATE(_bcmFieldInternalEMEntryarr, _bcmFieldInternalArray,
                                                  num_attr * ent_count, &tlv);
        tlv->value = array;
        TLV_WRITE(unit, tlv, ptr, pos);

    } else {
        _FP_XGS3_ALLOC(array, num_attr * ent_count * sizeof(int),
                                                            "em entires array");
        if (array == NULL) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        if (fg->em_mode == _FieldExactMatchMode128) {
            while  (count  != (num_attr * ent_count)) {
                f_ent = fg->entry_arr[count/num_attr];
                array[count++] = f_ent->eid;
                array[count++] = f_ent->flags;
                if (f_ent->statistic.flags & _FP_ENTRY_STAT_VALID) {
                    array[count++] = f_ent->statistic.sid;
                } else {
                    array[count++] = -1;
                }
                if (f_ent->policer[0].flags & _FP_POLICER_INSTALLED) {
                    array[count++] = f_ent->policer[0].pid;
                } else {
                    array[count++] = -1;
                }
                if (!(f_ent->flags & _FP_ENTRY_INSTALLED)) {
                    array[count++] = -1;
                    continue;
                }
                if (f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT) {
                    array[count++] = -1;
                    continue;
                }
                soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
                soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
                soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
                soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
                rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 0, 101);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_set(unit, mem, bufp, MODE128__KEY_0_ONLYf, tbuf);

                rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 101, 27);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_set(unit, mem, bufp, MODE128__KEY_1_ONLYf, tbuf);
                rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &hw_index,
                                                                bufp, tbuf, 0 );
                BCM_IF_ERROR_CLEANUP(rv);
                array[count++] = hw_index;
           }
        } else if (fg->em_mode == _FieldExactMatchMode160) {
            while  (count  != (num_attr * ent_count)) {
                f_ent = fg->entry_arr[count/num_attr];
                array[count++] = f_ent->eid;
                array[count++] = f_ent->flags;
                if (f_ent->statistic.flags & _FP_ENTRY_STAT_VALID) {
                    array[count++] = f_ent->statistic.sid;
                } else {
                    array[count++] = -1;
                }
                if (f_ent->policer[0].flags & _FP_POLICER_INSTALLED) {
                    array[count++] = f_ent->policer[0].pid;
                } else {
                    array[count++] = -1;
                }
                if (!(f_ent->flags & _FP_ENTRY_INSTALLED)) {
                    array[count++] = -1;
                    continue;
                }
                if (f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT) {
                    array[count++] = -1;
                    continue;
                }
                soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
                soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
                soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
                soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
                rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 0, 101);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_set(unit, mem, bufp, MODE160__KEY_0_ONLYf, tbuf);
                rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 101, 59);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_set(unit, mem, bufp, MODE160__KEY_1_ONLYf, tbuf);
                rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &hw_index,
                                                              bufp, tbuf, 0 );
                BCM_IF_ERROR_CLEANUP(rv);
                array[count++] = hw_index;
            }
        }
        /* coverity[leaked_storage] */
        TLV_CREATE(_bcmFieldInternalEMEntryarr, _bcmFieldInternalArray,
                                                 num_attr * ent_count, &tlv);
        tlv->value = array;
        TLV_WRITE(unit, tlv, ptr, pos);

    }
cleanup:
    if (array != NULL) {
        sal_free(array);
    }
    return BCM_E_NONE;


}
/*
 * Function:
 *      _bcm_field_th_stage_em_sync
 * Purpose:
 *      Main sync for exact match stage
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      fc       - (IN) _field_control_t structure
 *      stage_fc - (IN) Pointer to exactmatch stage structure
 *   Returns:
 *      BCM_E_XXX
 */
int
_bcm_field_th_stage_em_sync(int unit, _field_control_t *fc,
                             _field_stage_t *stage_fc)
{
    _field_tlv_t *tlv = NULL;     /* pointer to tlv structure */
    uint8 *ptr = NULL;            /* pointer to scache_ptr */
    uint32 *pos = NULL;           /* pointer to sacache_pos */
    int num_pipes, pipe_id = 0;   /* Number of pipes */
    int slice_id, lt_id = 0;      /* slice id, lt slice id */
    _field_slice_t *curr_slice = NULL;
                                  /* Pointer to curr slice, lt slice */
    _field_lt_slice_t *curr_ltslice = NULL;
    int rv = 0, i = 0;            /* return value, loop variable */
    _field_group_t *fg = NULL;
                                  /* Number of groups,
                                     partitions in each group */
    int group_count = 0, parts_count = 0;
    _field_lt_entry_t *f_lt_ent = NULL;
                                 /* pointer to lt entry */

    WB_FIELD_CONTROL_GET_WITH_LOCK(fc, ptr, pos);
    curr_stage_fc = stage_fc;

    if (stage_fc->oper_mode == 0) {
        num_pipes = 1;
    } else {
        num_pipes = stage_fc->num_pipes;
    }

    fg = fc->groups;
    while (fg != NULL) {
        if (fg->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
            group_count ++ ;
        }
        fg = fg->next;
    }

    if (group_count > 0) {

       TLV_CREATE(_bcmFieldInternalEMOperMode,
                 _bcmFieldInternalVariable, 0, &tlv);
       tlv->value = &stage_fc->oper_mode;
       TLV_WRITE(unit, tlv, ptr, pos);

       TLV_CREATE(_bcmFieldInternalSlice,
                  _bcmFieldInternalVariable, 0, &tlv);
       TLV_WRITE(unit, tlv, ptr, pos);

       for (pipe_id = 0; pipe_id < num_pipes; pipe_id++) {
           curr_slice = stage_fc->slices[pipe_id];
           curr_ltslice = stage_fc->lt_slices[pipe_id];
           if (curr_slice != NULL) {
              for (slice_id= 0; slice_id < stage_fc->tcam_slices; slice_id ++) {
                   rv = _field_slice_sync(unit, (curr_slice + slice_id));
                   BCM_IF_ERROR_CLEANUP(rv);

              }
           } else {
               LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "No slices to sync\n")));
           }
           if (curr_ltslice != NULL) {
               for (lt_id = 0; lt_id < stage_fc->tcam_slices; lt_id++) {
                   rv = _field_lt_slice_sync(unit, (curr_ltslice + lt_id));
                   BCM_IF_ERROR_CLEANUP(rv);
               }
           } else {
              LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                       "No LT slices to sync\n")));
           }
       }

       TLV_CREATE(_bcmFieldInternalGroupCount,
                  _bcmFieldInternalVariable, 0, &tlv);
       tlv->value = &group_count;
       TLV_WRITE(unit, tlv, ptr, pos);

       fg = fc->groups;
       while (fg != NULL) {
           if (fg->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
               rv = _field_group_sync(unit,fg);
               if (fg->em_mode == _FieldExactMatchMode320) {
                   parts_count = 2;
               } else {
                   parts_count = 1;
               }
               BCM_IF_ERROR_CLEANUP(rv);
               if (fg->group_status.entry_count > 0) {
                   rv = _field_em_group_entries_sync(unit, fg);
                   BCM_IF_ERROR_CLEANUP(rv);
               }
               for (i = 0; i < (fg->lt_grp_status.entry_count); i++) {
                   if (fg->lt_entry_arr[i] == NULL) {
                       continue;
                   }
                   f_lt_ent = fg->lt_entry_arr[i];
                _field_lt_entry_info_sync(unit, f_lt_ent, parts_count);
               }

           }
           fg = fg->next;
       }

       rv = _field_presel_entry_sync(unit);
       BCM_IF_ERROR_CLEANUP(rv);
    }

    TLV_CREATE(_bcmFieldInternalEndStageEM, _bcmFieldInternalVariable, 0, &tlv);
    TLV_WRITE(unit, tlv, ptr, pos);


cleanup:
    FP_UNLOCK(unit);
    sal_free(tlv);
    return rv;
}
/*
 * Function:
 * _bcm_field_th_stage_ingress_sync
 *
 * Purpose:
 *   Save field module software state to external cache.
 *
 * Parameters:
 * unit             - (IN) BCM device number
 * fc               - (IN) Pointer to device field control structure
 * stage_fc         - (IN) FP stage control info.
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_field_th_stage_ingress_sync(int unit,
                                  _field_stage_t   *stage_fc)
{
    uint32 group_count = 0;                 /* Number of groups */
    uint32 slice_id,pipe_id,lt_id;          /* Iterator varaiables */
    struct _field_slice_s *curr_slice;      /* Slice information */
    struct _field_lt_slice_s *curr_ltslice; /* LT slice information */
    _field_group_t      *fg;                /* Group information */
    _field_tlv_t *tlv = NULL;               /* TLV data */
    uint8 *scache_ptr = NULL;               /* Scache  Pointer */
    int  num_pipes = 0;                     /* Number of pipes */
    int size = 0;                           /* size of scache for IFP */
    int rv = BCM_E_NONE;                    /* return value */
    _field_control_t *fc;                   /* Field Control Structure */
    uint32 *position;                       /* Pointer to scache pos */
    uint32 enum_properties_type = BCM_FIELD_TYPE_ENUM_PROPERTIES;
                                            /* Type for Enum Properties */
    uint32 enum_total_count = _bcmFieldInternalElementCount;
                                            /* Count of Enums */
    uint32 enum_properties[(enum_total_count * 3)];
                                            /* Enum properties */
    uint32 prop_pos = 0;                    /* To index Enum Properties array */
    uint32 enum_count = 0;                  /* Enum index Variable */

    WB_FIELD_CONTROL_GET_WITH_LOCK(fc, scache_ptr, position);
    size = fc->scache_pos;
    curr_stage_fc = stage_fc;
    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        num_pipes = 1;
    } else if (stage_fc->oper_mode == bcmFieldGroupOperModePipeLocal) {
        num_pipes = stage_fc->num_pipes;
    }

    fg = fc->groups;
    while (fg != NULL) {
        if (fg->stage_id == _BCM_FIELD_STAGE_INGRESS) {
            group_count ++ ;
        }
        fg = fg->next;
    }

    /* Sync new enum properties during downgrade */

    if (fc->wb_current_version < BCM_FIELD_WB_DEFAULT_VERSION) {
        sal_memcpy(&scache_ptr[*position], &enum_properties_type,
                   sizeof(uint32));
        *position += sizeof(uint32);
        sal_memcpy(&scache_ptr[*position], &enum_total_count, sizeof(uint32));
        *position += sizeof(uint32);

        prop_pos = 0;
        for (enum_count = 0; enum_count < enum_total_count; enum_count++) {
            enum_properties[prop_pos++] = (uint32)static_type_map[enum_count].element;
            enum_properties[prop_pos++] = static_type_map[enum_count].size;
            enum_properties[prop_pos++] = static_type_map[enum_count].flags;
        }
        sal_memcpy(&scache_ptr[*position], &enum_properties,
                   ((3 * sizeof(uint32)) * enum_total_count));
        *position += ((3 * sizeof(uint32)) * enum_total_count);

    }


    /* Sync _field_control_t structure,
     * since this is common to all stages
     */

    /* _field_control_t */
    LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                        "_bcm_field_th_stage_ingress_sync -"
                                        "Syncing _field_control_t "
                                        "from pos = %d\r\n"),unit,
                                        fc->scache_pos));
    rv = _field_control_sync(unit);
    BCM_IF_ERROR_CLEANUP(rv);

    if (group_count > 0) {
        /*_field_stage_t */
        LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                            "_bcm_field_th_stage_ingress_sync -"
                                            "Syncing _field_stage_t "
                                            "from pos = %d\r\n"),unit,
                                            fc->scache_pos));

        rv = _field_stage_sync(unit,stage_fc);
        BCM_IF_ERROR_CLEANUP(rv);

        TLV_CREATE(_bcmFieldInternalSlice,
                   _bcmFieldInternalVariable, 0, &tlv);
        TLV_WRITE(unit, tlv, scache_ptr, position);

        /* _field_slice_t and _field_lt_slice_t */
        LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                         "_bcm_field_th_stage_ingress_sync -"
                                         "Syncing _field_slice_t,"
                                         "_field_lt_slice_t from pos = %d\r\n"),
                                         unit,fc->scache_pos));

        for (pipe_id = 0; pipe_id < num_pipes; pipe_id++) {
            curr_slice = stage_fc->slices[pipe_id];
            curr_ltslice = stage_fc->lt_slices[pipe_id];
            if (curr_slice != NULL) {
                for (slice_id= 0; slice_id < stage_fc->tcam_slices; slice_id ++) {
                    rv = _field_slice_sync(unit, (curr_slice + slice_id));
                    BCM_IF_ERROR_CLEANUP(rv);

                }
            } else {
                LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                         "No slices to sync\n")));
            }
            if (curr_ltslice != NULL) {
                for (lt_id = 0; lt_id < stage_fc->tcam_slices; lt_id++) {
                    rv = _field_lt_slice_sync(unit, (curr_ltslice + lt_id));
                    BCM_IF_ERROR_CLEANUP(rv);
                }
            } else {
               LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "No LT slices to sync\n")));
            }
        }

        /* _field_group_t */
        LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                            "_bcm_field_th_stage_ingress_sync -"
                                            "Syncing _field_group_t "
                                            "from pos = %d\r\n"),unit,
                                            fc->scache_pos));

        TLV_CREATE(_bcmFieldInternalGroupCount,
                   _bcmFieldInternalVariable, 0, &tlv);
        tlv->value = &group_count;
        TLV_WRITE(unit, tlv, scache_ptr, position);

        /* Iterate over the groups linked-list */
        fg = fc->groups;
        while (fg != NULL) {
            if (fg->stage_id == _BCM_FIELD_STAGE_INGRESS) {
                rv = _field_group_sync(unit,fg);
                BCM_IF_ERROR_CLEANUP(rv);
            }
            fg = fg->next;
        }

        /* _field_entry_t and _field_lt_entry_t */
        LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                         "_bcm_field_th_stage_ingress_sync -"
                                         "Syncing _field_entry_t,"
                                         "_field_lt_entry_t from pos = %d\r\n"),
                                         unit,fc->scache_pos));

        rv = _field_entry_sync(unit);
        BCM_IF_ERROR_CLEANUP(rv);

        /* _field_presel_entry_t */
        LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                            "_bcm_field_th_stage_ingress_sync -"
                                            "Syncing _field_presel_entry_t,"
                                            "from pos = %d\r\n"),
                                            unit,fc->scache_pos));

        rv = _field_presel_entry_sync(unit);
        BCM_IF_ERROR_CLEANUP(rv);


        LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                            "_bcm_field_th_stage_ingress_sync -"
                                            "End of structures sync "
                                            "at pos = %d\r\n"),unit,
                                            fc->scache_pos));
    }

    TLV_CREATE(_bcmFieldInternalEndStructIFP,
               _bcmFieldInternalVariable, 0, &tlv);
    TLV_WRITE(unit, tlv, scache_ptr, position);


    size = (fc->scache_pos - size);
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "Final scache_size = %d\n"),
                                          size));

cleanup:
    if (tlv != NULL) {
        sal_free(tlv);
        tlv = NULL;
    }
    FP_UNLOCK(unit);

    return rv;
}
/* Function:
 * _field_wb_stage_class_size
 *
 * Purpose:
 *   Calculate size required for IFP Compression section
 *
 * Parameters:
 * uinit      - (IN) unit number
 * size       - (out) Returns the size needed
 *
 * Returns:
 *      BCM_E_XXX
 */
int
_field_wb_stage_class_size(int unit, int *size)
{
    int num_groups = 0;
    int num_entries = 0;
    int size_classinfo = 0;
    int sum = 0;

    num_groups = _FieldClassCount;
    num_entries = 16 + 256 + 256 + 256 + 256 + 16 + 16 + 128 + 128;
    size_classinfo = 256;

    sum = size_classinfo + num_groups * 386 + num_entries * 133;
    /* To account for (type and values )of group and entry count */
    sum  += (8 + num_groups * 8);
    /* Account for 4 pipes */
    *size = _FP_MAX_NUM_PIPES *sum;
    return BCM_E_NONE;
}

#define _FP_ENTRY_COUNT 512
#define _FP_SLICE_COUNT 14
#define _FP_EM_ENTRY_COUNT 256000
#define _FP_STAT_COUNT (34 * 1024 * _FP_MAX_NUM_PIPES)
#define _FP_METER_COUNT 6144
#define BITS_PER_BYTE 8

/* Function:
 * _field_wb_size_calc
 *
 * Purpose:
 *   Calculate size required for IFP WB section
 *
 * Parameters:
 * total_size       - (OUT) Total size required for allocation
 * fc               - (IN) Pointer to device field control structure
 *
 * Returns:
 *      BCM_E_XXX
 */

int
_field_wb_size_calc(int unit, uint32* total_size, _field_control_t  *fc)
{

    int basic_size_slice,basic_size_fc,basic_size_stage,basic_size_ltslice = 0;
                                        /* Basic sizes of structures */
    int basic_size_group,basic_size_entry,basic_size_ltentry = 0;
    int basic_size_datafc,basic_size_presel = 0;
                                        /* Basic sizes of structures */
    int size_slice,size_fc,size_stage,size_ltslice,size_presel = 0;
                                        /* Total sizes of structures */
    int size_group,size_entry,size_ltentry,size_datafc = 0;
                                        /* Total sizes of structures */
    int type_slice,type_fc,type_stage,type_ltslice,type_presel = 0;
                                        /* Total size of types of structures */
    int type_group,type_entry,type_ltentry,type_datafc = 0;
                                        /* Total size of types of structures */

    int length_slice,length_fc,length_stage,length_ltslice,length_presel = 0;
                                        /* Total size of length of structures */
    int length_group,length_entry,length_ltentry,length_datafc = 0;
                                        /* Total size of length of structures */
    int total_type = 0, total_length = 0;
    int compression_size = 0;
    int exactmatch_size = 0;
    int enum_size = 0;

    basic_size_slice = 43;
    basic_size_stage = 596 + 64;       /* added 64 for presel */
    basic_size_ltslice = 32;
    basic_size_group = 106;
    basic_size_entry = 47 + 48 + 12 + (20 * 12);
                                       /* 20 actions needs software cache */
    basic_size_ltentry = 28;
    basic_size_datafc = 799756;
    basic_size_fc = 21 + 264;          /* added 264 for presel */
    basic_size_presel = 36;

    size_slice = basic_size_slice + (_FP_ENTRY_COUNT * 4)
                 + ((_FP_ENTRY_COUNT * 5)/BITS_PER_BYTE)
                 + (BCM_PBMP_PORT_MAX/BITS_PER_BYTE) + (111 * _FP_MAX_NUM_LT) ;

    size_stage = basic_size_stage + ( _FP_MAX_NUM_PIPES *_FIELD_MAX_METER_POOLS
                 * (13 + ((_FP_ENTRY_COUNT * 2)/BITS_PER_BYTE)))
                 + (_FIELD_MAX_CNTR_POOLS * (5 + ((_FP_ENTRY_COUNT
                 * 3)/BITS_PER_BYTE)))
                 + (_FP_MAX_NUM_PIPES * _FP_SLICE_COUNT * 4)
                 + (_FP_MAX_NUM_PIPES *_FP_MAX_NUM_LT * 76);

    size_ltslice = basic_size_ltslice + (_FP_ENTRY_COUNT * 4);

    size_group = basic_size_group + ((BCM_FIELD_QUALIFY_MAX/BITS_PER_BYTE)
                 + (BCM_FIELD_USER_NUM_UDFS/BITS_PER_BYTE))
                 + (BCM_PBMP_PORT_MAX/BITS_PER_BYTE)
                 + (_FP_MAX_ENTRY_TYPES * _FP_MAX_ENTRY_WIDTH * 10304)
                 + (_FP_ENTRY_COUNT * 4) + (bcmFieldActionCount/BITS_PER_BYTE)
                 + (4 *_FIELD_MAX_COUNTER_POOLS) + (111 * _FP_MAX_ENTRY_WIDTH)
                 + (_FP_ENTRY_COUNT * 4) + (_FP_SLICE_COUNT * 80) ;

    size_entry = (basic_size_entry + ((BCM_PBMP_PORT_MAX/BITS_PER_BYTE) * 2)
                  + (5 * _FP_POLICER_LEVEL_COUNT));

    size_ltentry = basic_size_ltentry;

    size_datafc = basic_size_datafc + (_FP_DATA_ETHERTYPE_MAX * 16)
                  + (_FP_DATA_IP_PROTOCOL_MAX * 21);

    size_fc = basic_size_fc + (10 * BCM_FIELD_USER_NUM_UDFS)
              + (_FP_MAX_NUM_LT * 4)
              + ((_FP_METER_COUNT * 97) + (20 * _FP_STAT_COUNT))
                 /* policer and stats hash  - 34k stats per pipe*/
              + (8 + (40 * 1));
                 /* Size for one hint */

    size_presel = basic_size_presel + (BCM_FIELD_QUALIFY_MAX/BITS_PER_BYTE)
                  + (_SHR_PBMP_WORD_MAX * 4);

    /* Currently Tomahawk has one hint added.
     * Total size required for hints
     * as per max limit calculation is (1024 * (8 + (40 * 256)))
     * i.e 1024 hint structures and 256 hints per structure
     */

    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "slice size = %d\n"),
               (_FP_SLICE_COUNT *size_slice)));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "lt slice size = %d\n"),
               (_FP_SLICE_COUNT * size_ltslice)));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "stage size = %d\n"),
               size_stage));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "group size = %d\n"),
               (_FP_MAX_NUM_LT * _FP_MAX_NUM_PIPES * size_group)));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "entry size = %d\n"),
               (_FP_SLICE_COUNT * _FP_ENTRY_COUNT * _FP_MAX_NUM_PIPES
               * size_entry)));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "ltentry size = %d\n"),
               (_FP_SLICE_COUNT * _FP_MAX_NUM_LT * _FP_MAX_NUM_PIPES
               * size_ltentry)));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "fc size = %d\n"),size_fc));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "datafc size = %d\n"),
               size_datafc));
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "presel size = %d\n"),
               BCM_FIELD_PRESEL_SEL_MAX * size_presel));

    *total_size = size_fc + size_datafc + size_stage + (_FP_SLICE_COUNT
                  * (size_slice + size_ltslice))
                  + (_FP_MAX_NUM_LT * _FP_MAX_NUM_PIPES * size_group)
                  + (_FP_SLICE_COUNT * _FP_MAX_NUM_LT *
                     _FP_MAX_NUM_PIPES * size_ltentry)
                  + (_FP_SLICE_COUNT * _FP_ENTRY_COUNT *
                     _FP_MAX_NUM_PIPES * size_entry)
                  + (BCM_FIELD_PRESEL_SEL_MAX * size_presel);

     LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "total value = %d\n"),
                 *total_size));


     type_fc = 64 + (13 * 256) + 16 + 4; /* added 16 for presel */
     type_datafc = 23 + (10 *512);
     type_stage = 48 + 8;            /* added 8 for presel */
     type_slice = 50;
     type_ltslice = 10;
     type_group = 22 + 9 + (3 *12) + 4;
     type_ltentry = 7;
     type_entry = 25;
     type_presel = 44;
     total_type = type_fc + type_datafc + type_stage + (_FP_SLICE_COUNT
                       * (type_slice + type_ltslice))
                       + (_FP_MAX_NUM_LT * _FP_MAX_NUM_PIPES * type_group)
                       + (_FP_SLICE_COUNT * _FP_MAX_NUM_LT *
                          _FP_MAX_NUM_PIPES * type_ltentry)
                       + (_FP_SLICE_COUNT * _FP_ENTRY_COUNT *
                          _FP_MAX_NUM_PIPES * type_entry)
                       + (type_presel);

     total_type = total_type * 4;

     LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "total type = %d\n"),
                                         total_type));


     length_slice = 32 *10;
     length_fc = 8;                /* added 8 for presel */
     length_stage = 4;             /* added 4 for presel */
     length_datafc= length_stage = length_ltslice = length_entry= 0;
     length_ltentry = 2;
     length_group = 10;
     length_presel = 4;

     total_length = length_fc + length_datafc + length_stage + (_FP_SLICE_COUNT
                       * (length_slice + length_ltslice))
                       + (_FP_MAX_NUM_LT * _FP_MAX_NUM_PIPES * length_group)
                       + (_FP_SLICE_COUNT * _FP_MAX_NUM_LT *
                          _FP_MAX_NUM_PIPES * length_ltentry)
                       + (_FP_SLICE_COUNT * _FP_ENTRY_COUNT *
                          _FP_MAX_NUM_PIPES * length_entry)
                       + length_presel;

     total_length = total_length * 4;

     LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "total length = %d\n"),
                 total_length));

     if (soc_feature(unit, soc_feature_field_compression)) {
         _field_wb_stage_class_size(unit, &compression_size);
     }

     if (soc_feature(unit, soc_feature_field_exact_match_support)) {
         /* Exactmatch has 64k entries per pipe
          * and each entry has at max 5 attr
          */
         exactmatch_size = size_stage + sizeof(int) * 5 * _FP_EM_ENTRY_COUNT;
     }

     enum_size = (_bcmFieldInternalElementCount * 4 * 3);
     /* 4 bytes each for 3 properties of enum */

     *total_size = *total_size + total_type + total_length + compression_size
                                            + enum_size + exactmatch_size;

     LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit, "total value = %d\n"),
                 *total_size));


    return BCM_E_NONE;

}

/* Function:
 * _bcm_field_scache_th_pointer_realloc
 *
 * Purpose:
 *   Initialize scache pointers for warmboot
 *
 * Parameters:
 * unit             - (IN) BCM device number
 * fc               - (IN) Pointer to device field control structure
 *
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_field_scache_th_pointer_realloc(int unit, _field_control_t  *fc)
{
    int               stable_size;        /* Total scache size */
    int               rv;                 /* Return variable */
    uint32            alloc_get;          /* Allocated size */
    soc_scache_handle_t handle;           /* scache handle */

    if ((NULL != fc->scache_ptr[_FIELD_SCACHE_PART_0]) ||
        (NULL != fc->scache_ptr[_FIELD_SCACHE_PART_1])) {

        SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
        SOC_SCACHE_HANDLE_SET(handle, unit, BCM_MODULE_FIELD,
                              _FIELD_SCACHE_PART_0);
        if ((stable_size > 0) && !SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit)) {

          /* Partition 0 size validation */

            /* Get the pointer for the Level 2 cache */
            rv = soc_scache_ptr_get(unit, handle,
                                    &(fc->scache_ptr[_FIELD_SCACHE_PART_0]),
                                    &alloc_get);
            if (BCM_FAILURE(rv)) {
                return rv;
            } else if (alloc_get != (fc->scache_size[_FIELD_SCACHE_PART_0]
                                     + SOC_WB_SCACHE_CONTROL_SIZE)) {
                /* Allocated size is less than expected size */
                  SOC_IF_ERROR_RETURN
                   (soc_scache_realloc(unit, handle,
                   (fc->scache_size[_FIELD_SCACHE_PART_0] - (alloc_get
                                   - SOC_WB_SCACHE_CONTROL_SIZE))));
                  rv = soc_scache_ptr_get(unit, handle,
                                  &(fc->scache_ptr[_FIELD_SCACHE_PART_0]),
                                  &alloc_get);
                  if (BCM_FAILURE(rv)) {
                      return rv;
                  } else if (alloc_get !=
                             (fc->scache_size[_FIELD_SCACHE_PART_0] +
                              SOC_WB_SCACHE_CONTROL_SIZE)) {
                      /* Expected size doesn't match retrieved size */
                      return BCM_E_INTERNAL;
                  } else if (NULL == fc->scache_ptr[_FIELD_SCACHE_PART_0]) {
                      return BCM_E_MEMORY;
                  }
            } else if (NULL == fc->scache_ptr[_FIELD_SCACHE_PART_0]) {
                return BCM_E_MEMORY;
            }

        }
   }
   return  BCM_E_NONE;
}

/* Function:
 * _bcm_field_scache_th_pointer_init
 *
 * Purpose:
 *   Initialize scache pointers for warmboot
 *
 * Parameters:
 * unit             - (IN) BCM device number
 * fc               - (IN) Pointer to device field control structure
 *
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_field_scache_th_pointer_init(int unit, _field_control_t  *fc)
{
    int               stable_size;        /* Total scache size */
    int               rv;                 /* Return variable */
    uint32            alloc_get;          /* Allocated size */
    soc_scache_handle_t handle;           /* scache handle */
    uint32              ifp_calculated_size;
                                          /* total scache size for IFP */
    uint32            scache_size = 0;    /* total scache size */

    if ((NULL == fc->scache_ptr[_FIELD_SCACHE_PART_0]) ||
        (NULL == fc->scache_ptr[_FIELD_SCACHE_PART_1])) {

        SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
        SOC_SCACHE_HANDLE_SET(handle, unit, BCM_MODULE_FIELD,
                              _FIELD_SCACHE_PART_0);
        if ((stable_size > 0) && !SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit)) {

             scache_size = soc_property_get(unit,
                                             spn_FIELD_SCACHE_SIZE,0);
             if (scache_size == 0) {
                 BCM_IF_ERROR_RETURN(_field_wb_size_calc(unit,
                                                     &ifp_calculated_size,fc));
                 /* value should be a multiple of 1024 */
                 ifp_calculated_size = (((ifp_calculated_size + 1023) / 1024)
                                          * 1024);
                 scache_size = ((294 * 1024) + (ifp_calculated_size));
             }

             fc->scache_size[_FIELD_SCACHE_PART_0] = scache_size;

             fc->scache_size[_FIELD_SCACHE_PART_1] = 20 * 1024;

            /* Partition 0 size validation */

            /* Get the pointer for the Level 2 cache */
            rv = soc_scache_ptr_get(unit, handle,
                                    &(fc->scache_ptr[_FIELD_SCACHE_PART_0]),
                                    &alloc_get);
            if (!SOC_WARM_BOOT(unit) && (SOC_E_NOT_FOUND == rv)) {
                /* Not yet allocated in Cold Boot */
                SOC_IF_ERROR_RETURN
                    (soc_scache_alloc(unit, handle,
                                      (fc->scache_size[_FIELD_SCACHE_PART_0]
                                      + SOC_WB_SCACHE_CONTROL_SIZE)));
                rv = soc_scache_ptr_get(unit, handle,
                                        &(fc->scache_ptr[_FIELD_SCACHE_PART_0]),
                                        &alloc_get);
            }

            /* Fetch WB Recovered Version */
            if (SOC_WARM_BOOT(unit)) {
                sal_memcpy(&(fc->wb_recovered_version),
                           fc->scache_ptr[_FIELD_SCACHE_PART_0],
                           sizeof(fc->wb_recovered_version));
            }


            if (BCM_FAILURE(rv)) {
                return rv;
            } else if (alloc_get != (fc->scache_size[_FIELD_SCACHE_PART_0]
                                     + SOC_WB_SCACHE_CONTROL_SIZE)) {
                if (!SOC_WARM_BOOT(unit)) {
                    /* Allocated size is greater than expected size */
                    return BCM_E_INTERNAL;
                }
            } else if (NULL == fc->scache_ptr[_FIELD_SCACHE_PART_0]) {
                return BCM_E_MEMORY;
            }

            /* Partition 1 size validation*/

            SOC_SCACHE_HANDLE_SET(handle, unit, BCM_MODULE_FIELD, 1);
            SOC_SCACHE_MODULE_MAX_PARTITIONS_SET(unit, BCM_MODULE_FIELD, 1);

            /* Get the pointer for the Level 2 cache */
            rv = soc_scache_ptr_get(unit, handle,
                                    &(fc->scache_ptr[_FIELD_SCACHE_PART_1]),
                                    &alloc_get);
            if (!SOC_WARM_BOOT(unit) && (SOC_E_NOT_FOUND == rv)) {
                /* Not yet allocated in Cold Boot */
                SOC_IF_ERROR_RETURN
                    (soc_scache_alloc(unit, handle,
                                      (fc->scache_size[_FIELD_SCACHE_PART_1]
                                      + SOC_WB_SCACHE_CONTROL_SIZE)));
                rv = soc_scache_ptr_get(unit, handle,
                                        &(fc->scache_ptr[_FIELD_SCACHE_PART_1]),
                                        &alloc_get);
            }

            if (BCM_FAILURE(rv)) {
                return rv;
            } else if (alloc_get != (fc->scache_size[_FIELD_SCACHE_PART_1]
                                    + SOC_WB_SCACHE_CONTROL_SIZE)) {
                /* Expected size doesn't match retrieved size */
                return BCM_E_INTERNAL;
            } else if (NULL == fc->scache_ptr[_FIELD_SCACHE_PART_1]) {
                return BCM_E_MEMORY;
            }

        }
   }
   return  BCM_E_NONE;
}

#define TLV_INIT(tlv)                    \
    do {                                 \
        tlv.type = -1;                   \
        tlv.length = 0;                  \
        if (NULL != tlv.value) {         \
            sal_free(tlv.value);         \
        }                                \
        tlv.value = NULL;                \
    }  while(0);

/* Function:
 * _field_policer_recover
 *
 * Purpose:
 *    recover _field_policer_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * Returns:
 *     BCM_E_XXX
 */

int
_field_policer_recover(int unit, _field_tlv_t *tlv)
{

    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint32 *type;                         /* Elem type */
    uint8 *scache_ptr;                    /* Scache pointer */
    uint32 *position;                     /* Scache position */
    _field_policer_t    *f_pl = NULL;     /* Field policer structure */
    _field_control_t        *fc;          /* Field control structure.  */
    int rv = 0;

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {

        _FP_XGS3_ALLOC(f_pl, sizeof (_field_policer_t), "Field policer");
        if (NULL == f_pl) {
           return (BCM_E_MEMORY);
        }
        type_pos = 0;

        while  ((tlv2.type != _bcmFieldInternalEndStructPolicer)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            rv = tlv_read(unit, &tlv2, scache_ptr, position);
            BCM_IF_ERROR_CLEANUP(rv);

            switch (tlv2.type) {

                case _bcmFieldInternalPolicerPid:
                     f_pl->pid = *(bcm_policer_t *)tlv2.value;
                     break;
                case _bcmFieldInternalPolicerCfgFlags:
                     f_pl->cfg.flags= *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgMode:
                     f_pl->cfg.mode = *(bcm_policer_mode_t *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgCkbitsSec:
                     f_pl->cfg.ckbits_sec = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgMaxCkbitsSec:
                     f_pl->cfg.max_ckbits_sec = *(uint32 *)tlv2.value;
                     break;


                case _bcmFieldInternalPolicerCfgCkbitsBurst:
                     f_pl->cfg.ckbits_burst = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgPkbitsSec:
                     f_pl->cfg.pkbits_sec = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgMaxPkbitsSec:
                     f_pl->cfg.max_pkbits_sec = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgPkbitsBurst :
                     f_pl->cfg.pkbits_burst = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgKbitsCurrent:
                     f_pl->cfg.kbits_current = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgActionId:
                     f_pl->cfg.action_id = *(uint32 *)tlv2.value;
                     break;


                case _bcmFieldInternalPolicerCfgSharingMode:
                     f_pl->cfg.action_id = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgEntropyId:
                     f_pl->cfg.entropy_id = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerCfgPoolId:
                     f_pl->cfg.pool_id = *(bcm_policer_pool_t *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerSwRefCount:
                     f_pl->sw_ref_count = *(uint16 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerHwRefCount:
                     f_pl->hw_ref_count = *(uint16 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerLevel:
                     f_pl->level = *(uint8 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerPoolIndex:
                     f_pl->pool_index = *(int8 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerHwIndex:
                     f_pl->hw_index = *(int *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerHwFlags:
                     f_pl->hw_flags = *(uint32 *)tlv2.value;
                     break;

                case _bcmFieldInternalPolicerStageId:
                     f_pl->stage_id = *(_field_stage_id_t *)tlv2.value;
                     break;

                case _bcmFieldInternalEndStructPolicer:
                     if (*(uint32 *)tlv2.value != _FIELD_WB_EM_POLICER) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                       "END MARKER CHECK FAILED : POLICER\n")));
                        rv = BCM_E_INTERNAL;
                        goto cleanup;
                     }
                     break;
                default :
                     break;
            }
            type_pos++;
        }

        TLV_INIT(tlv2);

        _FP_HASH_INSERT(fc->policer_hash, f_pl,
                 ((f_pl->pid) & _FP_HASH_INDEX_MASK(fc)));
        f_pl = NULL;
    }

    return BCM_E_NONE;

cleanup:
    TLV_INIT(tlv2);
    if (f_pl != NULL) {
        sal_free(f_pl);
        f_pl = NULL;
    }
    return rv;


}


/* Function:
 *
 * _field_presel_entry_recover
 *
 * Purpose:
 *    recover _field_stat_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * new_fg        - (IN) Array of groups in order of gids.
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_presel_entry_recover(int unit, _field_tlv_t *tlv,
                            _field_group_t **new_fg)
{

    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2 ;                   /* TLV structure */
    uint32 *type;                         /* Elem type */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_presel_entry_t *f_pr = NULL;   /* Field Presel Structure */
    _field_control_t    *fc;              /* Field control structure.   */
    int rv = 0;                           /* Return variable */
    _field_action_t *action = NULL;       /* Field Action Structure */
    int entry_idx = -1;                   /* Field Presel Tcam Index */
    soc_mem_t *fp_presel_mem = NULL;      /* Presel TCAM memeory */
    char *fp_presel_buf[_FP_MAX_NUM_PIPES] =  {0};
                                          /* Buffer to read the
                                           * IFP_LOGICAL_TABLE_SELECT_DATA_ONLY
                                           * table
                                           */
    int index_min = -1, index_max = -1;   /* Min and max index Tcam memory */
    int pipe = 0, num_pipes = 0;          /* Pipe variable and count of pipes */
    uint32 *e_buf = NULL;                 /* Buffer for corresponding entry */
    int temp = 0;                         /* Temporary Variable */
    int parts_count = 0 ;                 /* Number of parts of Entry */
    _field_group_t *fg = NULL;            /* Field Group Structure */
    _field_presel_entry_t *f_presel_p;    /* Field Presel Structure */
    _field_lt_slice_t *f_lt;     /* Field Presel Structure */
    int pri_tcam_idx = 0;                 /* Primary Tcam Index */
    int part_index = 0;                   /* Entry Part Index */
    int slice_num = 0;                    /* Slice Number */
    int id =0,idx =0;                     /* Loop Variables */
    uint32 flags = 0;                        /* Entry Part Flags */
    soc_profile_mem_t *keygen_profile = NULL;
    soc_mem_t  ifp_tcam_mems[_FP_MAX_NUM_PIPES + 1] = {
                                      IFP_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                                      IFP_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                                      IFP_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                                      IFP_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                                      IFP_LOGICAL_TABLE_SELECT_TCAM_ONLYm
                                                  };
                                          /* Memory for LT IFP Tcam */
    soc_mem_t  em_tcam_mems[_FP_MAX_NUM_PIPES + 1] = {
                              EXACT_MATCH_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                              EXACT_MATCH_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                              EXACT_MATCH_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                              EXACT_MATCH_LOGICAL_TABLE_SELECT_TCAM_ONLY_PIPE0m,
                              EXACT_MATCH_LOGICAL_TABLE_SELECT_TCAM_ONLYm
                                                  };
                                          /* Memory for LT EM Tcam */

    tlv2.value = NULL;
    TLV_INIT(tlv2);
    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);

    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    if (curr_stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        num_pipes = 1;
        if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
            fp_presel_mem = &ifp_tcam_mems[_FP_MAX_NUM_PIPES];
        } else if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
            fp_presel_mem = &em_tcam_mems[_FP_MAX_NUM_PIPES];
         }
    } else {
        num_pipes = curr_stage_fc->num_pipes;
        if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
            fp_presel_mem = &ifp_tcam_mems[0];
        } else if (curr_stage_fc->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
            fp_presel_mem = &em_tcam_mems[0];
        }
    }

    for (pipe = 0; pipe < num_pipes; pipe++) {
        /*  coverity[var_deref_op] */
        fp_presel_buf[pipe] = soc_cm_salloc(unit,
                           SOC_MEM_TABLE_BYTES(unit, fp_presel_mem[pipe]),
                           "FP POLICY TABLE buffer");
        if (NULL == fp_presel_buf[pipe]) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        index_min = soc_mem_index_min(unit, fp_presel_mem[pipe]);
        index_max = soc_mem_index_max(unit, fp_presel_mem[pipe]);
        rv = soc_mem_read_range(unit, fp_presel_mem[pipe], MEM_BLOCK_ALL,
                                index_min, index_max, fp_presel_buf[pipe]);
        BCM_IF_ERROR_CLEANUP(rv);

    }

    for (i = 0; i < num_instances; i++) {

        f_pr = NULL;
        _FP_XGS3_ALLOC(f_pr, sizeof (_field_presel_entry_t),
                       "Field Presel entity");
        if (NULL == f_pr) {
            return (BCM_E_MEMORY);
        }
        type_pos = 0;

        while  ((tlv2.type != _bcmFieldInternalEndStructPreselInfo)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            /* coverity[no_write_call : FALSE] */
            rv = tlv_read(unit, &tlv2, scache_ptr, position);
            BCM_IF_ERROR_CLEANUP(rv);

            switch (tlv2.type) {
                case _bcmFieldInternalPreselId:
                    f_pr->presel_id = *(uint32 *)tlv2.value;
                    break;
                case _bcmFieldInternalPreselFlags:
                    f_pr->flags = *(uint32 *)tlv2.value;
                    break;
                case _bcmFieldInternalPreselPri:
                    f_pr->priority = *(int *)tlv2.value;
                    break;
                case _bcmFieldInternalPreselHwIndex:
                    f_pr->hw_index = *(int *)tlv2.value;

                    break;
                case _bcmFieldInternalPreselSliceIndex:
                    f_pr->slice_index = *(int *)tlv2.value;
                    break;
                case _bcmFieldInternalPreselEntrySlice:
                     if (tlv2.length == 1) {
                         temp = *(uint8 *)tlv2.value;
                         if (f_pr->group != NULL) {
                             f_pr->lt_fs =
                             &curr_stage_fc->lt_slices[f_pr->group->instance][temp];
                         }
                     }
                     break;
                case _bcmFieldInternalPreselEntryGroup:
                     temp = *(int *)tlv2.value;
                     if (temp != -1) {
                         f_pr->group = (new_fg[temp]);
                     }
                     break;
                case _bcmFieldInternalPreselKeySize:
                     if (f_pr->group != NULL) {
                     pipe = f_pr->group->instance;
                     rv = _bcm_field_presel_entry_tcam_idx_get(unit,
                                                               f_pr,
                                                               f_pr->lt_fs,
                                                               &entry_idx);
                     BCM_IF_ERROR_CLEANUP(rv);
                     /* Get memory for tcam index  */
                     e_buf = soc_mem_table_idx_to_pointer(
                                          unit,
                                          fp_presel_mem[pipe],
                                          uint32 *,
                                          fp_presel_buf[pipe],
                                          entry_idx);

                     /* Allocate memory for TCAM Key and Mask. */
                     rv = _bcm_field_th_presel_tcam_key_mask_get(unit,
                                                          curr_stage_fc,
                                                          f_pr->group->instance,
                                                          f_pr,
                                                          &f_pr->lt_tcam);
                     BCM_IF_ERROR_CLEANUP(rv);
                     soc_mem_field_get(unit,fp_presel_mem[pipe],
                                            e_buf, KEYf, f_pr->lt_tcam.key);
                     soc_mem_field_get(unit,fp_presel_mem[pipe],
                                            e_buf, MASKf, f_pr->lt_tcam.mask);

                    f_pr->lt_tcam.key_size = *(uint16 *)tlv2.value;
                    }
                    break;
                case _bcmFieldInternalPreselQset:
                    if (downgrade[unit] == TRUE) {
                       sal_memcpy(f_pr->p_qset.w, tlv2.value,
                          ((recovery_type_map[unit][tlv2.type].size) *
                            _SHR_BITDCLSIZE(BCM_FIELD_QUALIFY_MAX)));
                    } else {
                       sal_memcpy(f_pr->p_qset.w, tlv2.value,
                       ((recovery_type_map[unit][tlv2.type].size) * tlv2.length));
                    }
                    break;
                case _bcmFieldInternalPreselClassAction:
                    if (tlv2.length == 1) {
                        action = NULL;
                        _FP_XGS3_ALLOC(action, sizeof(_field_action_t),
                                "actions");
                        if (action == NULL) {
                            rv = BCM_E_MEMORY;
                            goto cleanup;
                        }
                        action->action = bcmFieldActionGroupClassSet;
                        action->param[0] = *(uint32 *)tlv2.value;
                        f_pr->actions = action;
                    }
                    break;
                case _bcmFieldInternalEndStructPreselInfo:
                    if (*(uint32 *)tlv2.value != _FIELD_WB_EM_PRESEL) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : PRESEL\n")));
                        rv = BCM_E_INTERNAL;
                        goto cleanup;
                    }
                    break;
                case _bcmFieldInternalPreselPbmp:
                    sal_memcpy(f_pr->mixed_src_class_pbmp.pbits, tlv2.value,
                       ((recovery_type_map[unit][tlv2.type].size) * tlv2.length));
                    break;
                default:
                    break;
            }
            type_pos++;
        }

        if (f_pr->group !=NULL)
        {
            fg = f_pr->group;
            /* Get number of entry parts based on group flags. */
            BCM_IF_ERROR_RETURN(_bcm_field_th_entry_tcam_parts_count(unit,
                                                                 fg->flags,
                                                                 &parts_count));
            /* Get entry TCAM index given slice number
             * and entry offset in slice.
             */
            rv = _bcm_field_th_lt_slice_offset_to_tcam_index(unit,
                                       curr_stage_fc,
                                       fg->instance, f_pr->lt_fs->slice_number,
                                       f_pr->hw_index, &pri_tcam_idx);
            BCM_IF_ERROR_CLEANUP(rv);

            /* Adjust the presel entry as per the priority. */
            for (id = 0; id < _FP_PRESEL_ENTRIES_MAX_PER_GROUP; id++) {
               if (fg->presel_ent_arr[id] == NULL) {
                  fg->presel_ent_arr[id] = f_pr;
                  break;
               } else {
                  if (fg->presel_ent_arr[id]->priority < f_pr->priority) {
                     /* Move down all the entries */
                     for (idx = _FP_PRESEL_ENTRIES_MAX_PER_GROUP-1; idx > id;
                          idx--) {
                        fg->presel_ent_arr[idx] = fg->presel_ent_arr[idx - 1];
                     }
                     fg->presel_ent_arr[id] = f_pr;
                     break;
                  }
               }
            }

            if (parts_count > 1) {
               _field_presel_entry_t *p_ent_alloc;

               f_presel_p = f_pr;
               /* Allocate memory for presel entries for
                * double/triple wide modes.
                */
               for (idx = 1; idx < parts_count; idx++) {
                  p_ent_alloc = NULL;
                  _FP_XGS3_ALLOC(p_ent_alloc, sizeof(_field_presel_entry_t),
                              "Presel Entry Alloc.");
                  if (NULL == p_ent_alloc) {
                      LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                      "Error: allocation failure for Presel[%d] "
                      "Parts entry[%d]\n"),
                         f_pr->presel_id, idx));
                      return (BCM_E_MEMORY);
                  }
                  /* Copy the base presel entry contents to its
                   * allocated part entry
                   */
                  sal_memcpy(p_ent_alloc, f_presel_p,
                             sizeof(_field_presel_entry_t));
                  f_presel_p->next = p_ent_alloc;
                  f_presel_p = p_ent_alloc;
               }
            }

            f_presel_p = f_pr;
            keygen_profile =
                        &curr_stage_fc->keygen_profile[fg->instance].profile;

            /* Fill entry primitives. */
            for (idx = 0; (idx < parts_count && f_presel_p != NULL); idx++) {

                /* Allocate the Preselector Data Memory and get data*/
                BCM_IF_ERROR_RETURN (_bcm_field_th_presel_mem_data_get(
                                                       unit, curr_stage_fc,
                                                       fg->instance,
                                                       f_presel_p,
                                                       &f_presel_p->lt_data));
                flags = 0;
                /* Get entry part flags. */
                rv = _bcm_field_th_tcam_part_to_entry_flags(unit, idx,
                                                            fg->flags,
                                                            &(flags));

                /* Reset primary flags for other parts of entry */
                if (idx > 0) {
                f_presel_p->flags &= ~(_FP_ENTRY_PRIMARY);
                }

                f_presel_p->flags = (f_presel_p->flags | flags);

                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                /* Update keygen profile reference
                 * count for each part
                 */

                rv  = soc_profile_mem_reference(unit, keygen_profile,
                                fg->ext_codes[idx].keygen_index, 1);

                if (BCM_FAILURE(rv)) {
                    goto cleanup;
                }

                 /* Given primary entry TCAM index calculate
                 * entry part tcam index.
                 */
                if (0 != idx) {
                    rv = _bcm_field_th_lt_entry_part_tcam_idx_get(unit, fg,
                                                              pri_tcam_idx,
                                                              idx, &part_index);
                    if (BCM_FAILURE(rv)) {
                        goto cleanup;
                    }

                    /*
                     * Given entry part TCAM index, determine the slice number
                     * and slice offset index.
                     */
                    rv = _bcm_field_th_lt_tcam_idx_to_slice_offset(unit,
                            curr_stage_fc,
                            fg->instance, part_index, &slice_num,
                            (int *)&f_pr->hw_index);
                    if (BCM_FAILURE(rv)) {
                        goto cleanup;
                    }
                    f_presel_p->lt_fs = curr_stage_fc->lt_slices[fg->instance]
                                        + slice_num;
                }

		    		/* Assign entry to a slice. */
                f_presel_p->lt_fs->p_entries[f_presel_p->hw_index] = f_presel_p;
                f_lt = f_presel_p->lt_fs;
                while (f_lt !=NULL) {
                      f_lt ->p_entries[f_presel_p->hw_index]
                      = f_presel_p;
                      f_lt = f_lt->next;
                }

                /* Next Entry Part */
                f_presel_p = f_presel_p->next;
            }
        }

        fc->presel_db[f_pr->presel_id] = f_pr;
        TLV_INIT(tlv2);

    }
    return BCM_E_NONE;

cleanup:
    TLV_INIT(tlv2);
    for (pipe = 0; pipe < num_pipes; pipe++) {
         if (fp_presel_buf[pipe] != NULL) {
             soc_cm_sfree(unit, fp_presel_buf[pipe]);
          }
    }
    if (f_pr != NULL) {
        if (f_pr->actions != NULL) {
            sal_free(f_pr->actions);
        }
        sal_free(f_pr);
        f_pr = NULL;
    }
    return rv;
}

/* Function:
 * _field_stat_recover
 *
 * Purpose:
 *    recover _field_stat_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * Returns:
 *     BCM_E_XXX
 */

int
_field_stat_recover(int unit, _field_tlv_t *tlv)
{

    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2 ;                   /* TLV structure */
    uint32 *type;                         /* Elem type */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_stat_t       *f_st = NULL;     /* Field Stat Structure */
    _field_control_t    *fc;              /* Field control structure.   */
    int rv = 0;                           /* Return variable */
    _field_stat_bmp_t   stat_bmp;         /* Stat BMP structure */
    uint8  idx;                           /* Index to carry loop count. */
    int  pos = 0;                         /* Iterator */

    tlv2.value = NULL;
    stat_bmp.w = NULL;
    TLV_INIT(tlv2);
    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);

    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);
    for (i = 0; i < num_instances; i++) {

        _FP_XGS3_ALLOC(f_st, sizeof (_field_stat_t), "Field stat entity");
        if (NULL == f_st) {
            return (BCM_E_MEMORY);
        }
        type_pos = 0;

        while  ((tlv2.type != _bcmFieldInternalEndStructStat)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            rv = tlv_read(unit, &tlv2, scache_ptr, position);
            BCM_IF_ERROR_CLEANUP(rv);

            switch (tlv2.type) {
                case _bcmFieldInternalStatsId:
                    f_st->sid = *(uint32 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatSwRefCount:
                    f_st->sw_ref_count = *(uint16 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatHwRefCount:
                    f_st->hw_ref_count = *(uint16 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatOffsetMode:
                    f_st->offset_mode = *(int8 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatPoolIndex:
                    f_st->pool_index = *(int8 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatHwIndex :
                    f_st->hw_index = *(int *)tlv2.value;
                    break;
                case _bcmFieldInternalStatHwFlags:
                    f_st->hw_flags = *(uint32 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatnStat:
                    f_st->nstat = *(uint8 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatArr:
                    _FP_XGS3_ALLOC(f_st->stat_arr,
                                  (tlv2.length * (recovery_type_map[unit][tlv2.type].size)),
                                   "Field stat array");
                    if (NULL == f_st->stat_arr) {
                        rv = (BCM_E_MEMORY);
                        goto cleanup;
                    }
                    sal_memcpy(f_st->stat_arr, tlv2.value,
                             ((recovery_type_map[unit][tlv2.type].size) * tlv2.length));
                    break;
                case _bcmFieldInternalStatBmp:
                    _FP_XGS3_ALLOC(stat_bmp.w,
                                  (tlv2.length * (recovery_type_map[unit][tlv2.type].size)),
                                   "Field stat array");
                    if (NULL == stat_bmp.w) {
                        rv = (BCM_E_MEMORY);
                        goto cleanup;
                    }
                    sal_memcpy(stat_bmp.w, tlv2.value,
                             ((recovery_type_map[unit][tlv2.type].size)
                             * tlv2.length));

                    _FP_XGS3_ALLOC(f_st->stat_arr, (f_st->nstat *
                                   sizeof (bcm_field_stat_t)),
                                   "Field stat array");
                    if (NULL == f_st->stat_arr) {
                        return (BCM_E_MEMORY);
                    }

                    pos = 0;
                    for (idx = 0; idx < ((tlv2.length)
                                      * (recovery_type_map[unit][tlv2.type].size)
                                      * 8); idx++) {
                         if (_FP_STAT_BMP_TEST(stat_bmp, idx)) {
                            f_st->stat_arr[pos] = idx;
                            pos++;
                         }
                    }

                    if (stat_bmp.w !=NULL) {
                        _FP_STAT_BMP_FREE(stat_bmp);
                    }
                    stat_bmp.w = NULL;
                    break;
                case _bcmFieldInternalStatHwStat:
                    f_st->hw_stat = *(uint32 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatHwMode:
                    f_st->hw_mode = *(uint16 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatHwEntryCount:
                    f_st->hw_entry_count = *(uint8 *)tlv2.value;
                    break;
                case _bcmFieldInternalStatGid:
                    f_st->gid = *(bcm_field_group_t *)tlv2.value;
                    break;
                case _bcmFieldInternalStatStageId:
                    f_st->stage_id = *(_field_stage_id_t *)tlv2.value;
                    break;
                case _bcmFieldInternalStatStatValues:
                    _FP_XGS3_ALLOC(f_st->stat_values,
                                  (tlv2.length * sizeof (uint64)),
                                    "Field stat values array");
                     if (NULL == f_st->stat_values) {
                         rv = (BCM_E_MEMORY);
                         goto cleanup;
                     }
                    sal_memcpy(f_st->stat_values, tlv2.value,
                       ((recovery_type_map[unit][tlv2.type].size) * tlv2.length));
                    break;
                case _bcmFieldInternalStatFlexMode:
                    f_st->flex_mode = *(uint32 *)tlv2.value;
                    break;
                case _bcmFieldInternalEndStructStat:
                    if (*(uint32 *)tlv2.value != _FIELD_WB_EM_STAT) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : STAT\n")));
                        rv = BCM_E_INTERNAL;
                        goto cleanup;
                    }
                    break;
                default:
                    break;
            }
            type_pos++;
        }
        TLV_INIT(tlv2);

        /* Allocate memory for stat_values */
        if (NULL == f_st->stat_values) {
            _FP_XGS3_ALLOC(f_st->stat_values,(f_st->nstat * sizeof (uint64)),
                           "Field stat values array");
            if (NULL == f_st->stat_values) {
                rv = (BCM_E_MEMORY);
                goto cleanup;
            }
         }

        /* Initialize sw ref count */
        f_st->sw_ref_count = 1;
        f_st->pool_index = _FP_INVALID_INDEX;
        f_st->hw_index = _FP_INVALID_INDEX;
        f_st->hw_mode = _FP_INVALID_INDEX;
        f_st->hw_entry_count = _FP_INVALID_INDEX;
        f_st->hw_stat = _FP_INVALID_INDEX;
        f_st->hw_ref_count =  0;
        f_st->gid  = _FP_INVALID_INDEX;

        _FP_HASH_INSERT(fc->stat_hash, f_st,
                        (f_st->sid & _FP_HASH_INDEX_MASK(fc)));
        f_st = NULL;
    }

    return BCM_E_NONE;

cleanup:
    TLV_INIT(tlv2);
    if (f_st != NULL) {
        if (f_st->stat_arr != NULL) {
            sal_free(f_st->stat_arr);
            f_st->stat_arr = NULL;
        }
        sal_free(f_st);
        f_st = NULL;
    }
    return rv;
}

/* Function:
 * _field_hint_count_array_recover
 *
 * Purpose:
 *    recover _bcm_field_hint_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * f_ht             - (OUT) Field hints are stored
 * Returns:
 *     BCM_E_XXX
 */

int
_field_hint_count_array_recover(int unit, _field_tlv_t *tlv,
                         _field_hints_t *f_ht)
{
    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint32 *type;                         /* Elem type */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_hint_t     *hint_entry = NULL; /* Field hint structure */
    _field_control_t  *fc;                /* Field control structure.       */
    int rv = 0;                           /* Return variable */

    tlv2.value = NULL;
    TLV_INIT(tlv2);
    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);

    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {
         _FP_XGS3_ALLOC (hint_entry, sizeof (_field_hint_t),
                         "Field Hint Entry Structure");
         if (hint_entry == NULL) {
             rv = BCM_E_MEMORY;
             goto cleanup;
         }
         _FP_XGS3_ALLOC (hint_entry->hint, sizeof (bcm_field_hint_t),
                         "Field Hint Entry");
         if (hint_entry->hint == NULL) {
             rv = BCM_E_MEMORY;
             goto cleanup;
         }

         hint_entry->next = NULL;
         type_pos = 0;

         while ((tlv2.type != _bcmFieldInternalEndStructHintHints)
                 && (type_pos != num_types)) {
             TLV_INIT(tlv2);
             tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
             tlv2.basic_type =((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
             rv = tlv_read(unit, &tlv2, scache_ptr, position);
             BCM_IF_ERROR_CLEANUP(rv);
             switch (tlv2.type) {
                case _bcmFieldInternalHintHintsHintType:
                     hint_entry->hint->hint_type =
                                *(bcm_field_hint_type_t *)tlv2.value;
                     break;
                case _bcmFieldInternalHintHintsQual:
                     hint_entry->hint->qual =
                             *(bcm_field_qualify_t *)tlv2.value;
                     break;
                case _bcmFieldInternalHintHintsMaxValues:
                     hint_entry->hint->max_values = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalHintHintsStartBit:
                     hint_entry->hint->start_bit = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalHintHintsEndBit:
                     hint_entry->hint->end_bit = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalHintHintsFlags:
                     hint_entry->hint->flags = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalHintHintsMaxGrpSize:
                     hint_entry->hint->max_group_size = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalHintHintsPri:
                     hint_entry->hint->priority = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalEndStructHintHints:
                     break;
                default:
                    break;
            }
            type_pos++;
        }
        _FP_HINTS_LINKLIST_INSERT(&(f_ht->hints),hint_entry);
        hint_entry = NULL;

    }
    TLV_INIT(tlv2);
    return BCM_E_NONE;
cleanup:
    TLV_INIT(tlv2);
    if (hint_entry != NULL) {
        if (hint_entry->hint != NULL) {
            sal_free(hint_entry->hint);
            hint_entry->hint = NULL;
        }
        sal_free(hint_entry);
        hint_entry = NULL;
    }
    return rv;

}

/* Function:
 * _field_hint_recover
 *
 * Purpose:
 *    recover _field_hint_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * Returns:
 *     BCM_E_XXX
 */

int
_field_hint_recover(int unit, _field_tlv_t *tlv)
{
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_hints_t   *f_ht = NULL;        /* field Hints structure */
    _field_control_t *fc;                 /* Field control structure */
    int rv = 0;                           /* Return variable */

    tlv2.value = NULL;
    TLV_INIT(tlv2);
    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    num_instances = (int) tlv->length;

    for (i = 0; i < num_instances; i++) {
    _FP_XGS3_ALLOC(f_ht, _FP_HASH_SZ(fc) * \
                   sizeof(_field_hints_t), "Hints hash");
    if (f_ht == NULL) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }

    while  (tlv2.type != _bcmFieldInternalEndStructHint) {
            TLV_INIT(tlv2);
            /* coverity[no_write_call : FALSE] */
            rv = tlv_read(unit, &tlv2, scache_ptr, position);
            BCM_IF_ERROR_CLEANUP(rv);
            switch (tlv2.type) {
                case _bcmFieldInternalHintHintid:
                    f_ht->hintid = *(bcm_field_hintid_t *)tlv2.value;
                    break;
                case _bcmFieldInternalHintHints:
                    _field_hint_count_array_recover(unit, &tlv2, f_ht);
                    break;
                case _bcmFieldInternalHintGrpRefCount:
                    f_ht->grp_ref_count = *(uint16 *)tlv2.value;
                    break;
                case _bcmFieldInternalHintCount:
                    f_ht->hint_count = *(uint16 *)tlv2.value;
                    break;
                case _bcmFieldInternalEndStructHint:
                    if (*(uint32 *)tlv2.value != _FIELD_WB_EM_HINT) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : HINT\n")));
                        rv = BCM_E_INTERNAL;
                        goto cleanup;
                    }
                    break;
                default:
                    break;
            }
        }
        TLV_INIT(tlv2);

        _FP_HASH_INSERT(fc->hints_hash, f_ht,
                       (f_ht->hintid & _FP_HASH_INDEX_MASK(fc)));

    }
    return BCM_E_NONE;
cleanup:
    TLV_INIT(tlv2);
    if (f_ht != NULL) {
        sal_free(f_ht);
        f_ht = NULL;
    }
    return rv;

}
/* Function:
 * _field_udf_recover
 *
 * Purpose:
 *    recover _field_udf_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * fc               - (IN) Pointer to device field control structure
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_udf_recover(int unit, uint8 *ptr, uint32 *pos, _field_udf_t *udf,
                    _field_tlv_t *tlv)
{
    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint32 *type;                         /* Elem type */

    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {
        type_pos = 0;

        while  ((tlv2.type != _bcmFieldInternalControlEndStructUdf)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, ptr, pos));
            switch (tlv2.type) {
                case _bcmFieldInternalControlUdfValid:
                     udf->valid = *(uint8 *)tlv2.value;
                     break;
                case _bcmFieldInternalControlUdfUseCount:
                     udf->use_count = *(int *)tlv2.value;
                     break;
                case _bcmFieldInternalControlUdfNum:
                     udf->udf_num = *(bcm_field_qualify_t *)tlv2.value;
                     break;
                case _bcmFieldInternalControlUdfUserNum:
                     udf->user_num = *(uint8 *)tlv2.value;
                     break;
                case _bcmFieldInternalControlEndStructUdf:
                     TLV_INIT(tlv2);
                     break;
                default:
                    break;
            }
            type_pos++;
        }
        udf++;
    }
    TLV_INIT(tlv2);
    return BCM_E_NONE;

}

/* Function:
 * _field_control_recover
 *
 * Purpose:
 *    recover _field_control_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * fc               - (IN) Pointer to device field control structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_control_recover(int unit, _field_control_t *fc)
{

    _field_tlv_t tlv;              /* Field TLV structure */
    uint8 *scache_ptr = fc->scache_ptr[_FIELD_SCACHE_PART_0];
                                   /* Pointer to Scache */
    uint32 *position = &(fc->scache_pos);
                                   /* Position in Scache */
    tlv.value = NULL;
    TLV_INIT(tlv);

    while (tlv.type != _bcmFieldInternalEndStructControl) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv, scache_ptr, position));

        switch (tlv.type)
        {
            case _bcmFieldInternalControlFlags:
                 fc->flags = *(uint8 *)tlv.value;
                 break;
            case _bcmFieldInternalControlUdfDetails:
                  BCM_IF_ERROR_RETURN(_field_udf_recover(unit, scache_ptr,
                                           position, fc->udf, &tlv));
                  break;
            case _bcmFieldInternalControlGroups:
                 break;
            case _bcmFieldInternalControlStages:
                 break;
            case _bcmFieldInternalControlPolicerCount:
                 fc->policer_count = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalControlStatCount:
                 fc->stat_count = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalControlHintBmp:
                 sal_memcpy(fc->hintid_bmp.w, tlv.value,
                      ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalControlLastAllocatedLtId:
                 fc->last_allocated_lt_eid = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalControlPolicerHash:
                 BCM_IF_ERROR_RETURN(_field_policer_recover(unit, &tlv));
                 break;
            case _bcmFieldInternalControlStathash:
                 BCM_IF_ERROR_RETURN(_field_stat_recover(unit, &tlv));
                 break;
            case _bcmFieldInternalControlHintHash:
                 BCM_IF_ERROR_RETURN(_field_hint_recover(unit, &tlv));
                 break;
            case _bcmFieldInternalPreselInfoLimit:
                 fc->presel_info->presel_limit = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalPreselLastAllocatedId:
                 fc->presel_info->last_allocated_id = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalPreselSet:
                 sal_memcpy(fc->presel_info->presel_set.w, tlv.value,
                      ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalPreselOperationalSet:
                 sal_memcpy(fc->presel_info->operational_set.w, tlv.value,
                      ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalEndStructControl:
                 if (*(uint32 *)tlv.value != _FIELD_WB_EM_CONTROL) {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                       "END MARKER CHECK FAILED : CONTROL\n")));
                     TLV_INIT(tlv);
                     return BCM_E_INTERNAL;
                 }
                 break;
            default:
                 break;
        }
    }
    TLV_INIT(tlv);
    return BCM_E_NONE;
}



/* Function:
 * _field_meterpool_recover
 *
 * Purpose:
 *    recover _field_meter_pool_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * stage_fc         - (OUT) Pointer to field stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_meterpool_recover(int unit, _field_tlv_t *tlv, _field_stage_t *stage_fc)
{
    int i = 0;                            /* local variable for loops */
    int j = 0;
    uint32 *type;                         /* Elem type */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    int type_pos = 0;                     /* position of type */
    int num_i_instances = 0;              /* num_instances in 1st dimension */
    int num_j_instances = 0;              /* num_instances in 2nd dimension */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_i_instances = (tlv->length) & 0x000003ff;
    num_j_instances = ((tlv->length) & 0x003ffc00) >> 10;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_i_instances; i++) {
        for (j = 0; j < num_j_instances; j++) {
             type_pos = 0;

             while ((tlv2.type != _bcmFieldInternalEndStructMeter)
                     && (type_pos != num_types)) {
                TLV_INIT(tlv2);
                tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
                tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                                  _FP_WB_TLV_BASIC_TYPE_SHIFT);
                BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));
                switch (tlv2.type) {
                    case _bcmFieldInternalMeterLevel:
                         stage_fc->meter_pool[i][j]->level = *(int8 *)tlv2.value;
                         break;
                    case _bcmFieldInternalMeterSliceId:
                         stage_fc->meter_pool[i][j]->slice_id = *(int *)tlv2.value;
                         break;
                    case _bcmFieldInternalMeterSize:
                         stage_fc->meter_pool[i][j]->size = *(uint16 *)tlv2.value;
                         break;
                    case _bcmFieldInternalMeterPoolSize:
                         stage_fc->meter_pool[i][j]->pool_size =
                                                            *(uint16 *)tlv2.value;
                         break;
                    case _bcmFieldInternalMeterFreeMeters:
                         stage_fc->meter_pool[i][j]->free_meters =
                                                    *(uint16 *)tlv2.value;
                         break;
                    case _bcmFieldInternalMeterNumMeterPairs:
                         stage_fc->meter_pool[i][j]->num_meter_pairs =
                                                            *(uint16 *)tlv2.value;
                         break;
                    case _bcmFieldInternalMeterBmp:
                         sal_memcpy(stage_fc->meter_pool[i][j]->meter_bmp.w,
                                 tlv2.value, ((tlv2.length) *
                                     (recovery_type_map[unit][tlv2.type].size)));
                         break;
                    case _bcmFieldInternalEndStructMeter:
                         if (*(uint32 *)tlv2.value !=  _FIELD_WB_EM_METER) {
                             LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                    "END MARKER CHECK FAILED : METER(stage)\n")));
                             TLV_INIT(tlv2);
                             return BCM_E_INTERNAL;
                         }
                         break;
                    default:
                         break;
                }
             type_pos++;

             }
        TLV_INIT(tlv2);
        }
    }

    return BCM_E_NONE;
}
/* Function:
 * _field_extractor_recover
 *
 * Purpose:
 *    recover _field_slice_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * ptr           - (IN) Pointer to scache in field control
 * pos           - (IN) Pointer to position in field control
 * tlv           - (IN) tlv structure containing encoded types and number.
 * ext_sel       - (IN) address of start of array in slice structure.
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_extractor_recover(int unit, uint8 *ptr, uint32 *pos,
                   _field_tlv_t *tlv,  _field_ext_sel_t *ext_sel)
{

    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint32 *type;                         /* Elem type */

    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {
        type_pos = 0;
        TLV_INIT(tlv2);
        while  ((tlv2.type != _bcmFieldInternalEndStructExtractor)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, ptr, pos));
            switch (tlv2.type) {
                case _bcmFieldInternalExtl1e32Sel:
                     sal_memcpy(ext_sel->l1_e32_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl1e16Sel:
                     sal_memcpy(ext_sel->l1_e16_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl1e8Sel:
                     sal_memcpy(ext_sel->l1_e8_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl1e4Sel:
                     sal_memcpy(ext_sel->l1_e4_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl1e2Sel:
                     sal_memcpy(ext_sel->l1_e2_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl2e16Sel:
                     sal_memcpy(ext_sel->l2_e16_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl3e1Sel:
                     sal_memcpy(ext_sel->l3_e1_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl3e2Sel:
                     sal_memcpy(ext_sel->l3_e2_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtl3e4Sel:
                     sal_memcpy(ext_sel->l3_e4_sel, tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalExtPmuxSel:
                    sal_memcpy(ext_sel->pmux_sel, tlv2.value, tlv2.length);
                    break;
                case _bcmFieldInternalExtIntraSlice:
                     ext_sel->intraslice = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtSecondary:
                     ext_sel->secondary = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtIpbmpRes:
                     ext_sel->ipbm_present = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtNorml3l4:
                     ext_sel->normalize_l3_l4_sel= *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtNormmac:
                     ext_sel->normalize_mac_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtAuxTagaSel:
                     ext_sel->aux_tag_a_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtAuxTagbSel:
                    ext_sel->aux_tag_b_sel = *(int8 *)tlv2.value;
                    break;

                case _bcmFieldInternalExtAuxTagcSel:
                    ext_sel->aux_tag_c_sel = *(int8 *)tlv2.value;
                    break;
                case _bcmFieldInternalExtAuxTagdSel:
                    ext_sel->aux_tag_d_sel = *(int8 *)tlv2.value;
                    break;
                case _bcmFieldInternalExtTcpFnSel:
                    ext_sel->tcp_fn_sel = *(int8 *)tlv2.value;
                    break;
                case _bcmFieldInternalExtTosFnSel:
                     ext_sel->tos_fn_sel = *(int8 *)tlv2.value;
                    break;
                case _bcmFieldInternalExtTtlFnSel:
                     ext_sel->ttl_fn_sel = *(int8 *)tlv2.value;
                    break;
                case _bcmFieldInternalExtClassIdaSel:
                     ext_sel->class_id_cont_a_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtClassIdbSel:
                     ext_sel->class_id_cont_b_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtClassIdcSel:
                     ext_sel->class_id_cont_c_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtClassIddSel:
                     ext_sel->class_id_cont_d_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtSrcContaSel:
                     ext_sel->src_cont_a_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtSrcContbSel:
                     ext_sel->src_cont_b_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtSrcDestCont0Sel:
                     ext_sel->src_dest_cont_0_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtSrcDestCont1Sel:
                     ext_sel->src_dest_cont_1_sel = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalExtKeygenIndex:
                    ext_sel->keygen_index = *(uint32 *)tlv2.value;
                case _bcmFieldInternalEndStructExtractor:
                    break;
                default:
                    break;
            }
            type_pos++;
        }
        ext_sel++;
    }
    TLV_INIT(tlv2);
    return BCM_E_NONE;

}
#define SLICE_STRUCT(_stage_, _ptr_, _pipenum_, _sliceid_)      \
        (_stage_->_ptr_[_pipenum_][_sliceid_])                  \

/* Function:
 * _field_cntrpool_recover
 *
 * Purpose:
 *    recover _field_cntr_pool_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * stage_fc         - (OUT) Pointer to field stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_cntrpool_recover(int unit, _field_tlv_t *tlv, _field_stage_t *stage_fc)
{
    int i = 0;                            /* local variable for loops */
    uint32 *type;                         /* Elem type */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    int type_pos = 0;                     /* position of type */
    int num_i_instances = 0;              /* number of instances */
    _field_cntr_pool_t *f_cp = NULL;      /* Field counter pool structure */


    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_i_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_i_instances; i++) {
         f_cp = stage_fc->cntr_pool[i];
         type_pos = 0;
         while ((tlv2.type != _bcmFieldInternalEndStructCntr)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));
            switch (tlv2.type) {
                case _bcmFieldInternalCntrSliceId:
                     f_cp->slice_id = *(int8 *)tlv2.value;
                     break;
                case _bcmFieldInternalCntrSize:
                     f_cp->size = *(uint16 *)tlv2.value;
                     break;
                case _bcmFieldInternalCntrFreeCntrs:
                     f_cp->free_cntrs = *(uint16 *)tlv2.value;
                     break;
                case _bcmFieldInternalCntrBmp:
                     sal_memcpy(f_cp->cntr_bmp.w, tlv2.value,
                               ((tlv2.length)
                                 * (recovery_type_map[unit][tlv2.type].size)));

                     break;
                case _bcmFieldInternalEndStructCntr:
                     if (*(uint32 *)tlv2.value != _FIELD_WB_EM_CNTR) {
                         LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                       "END MARKER CHECK FAILED : COUNTER\n")));
                         TLV_INIT(tlv2);
                         return BCM_E_INTERNAL;
                     }
                     break;
                default:
                     break;
            }
        type_pos++;

        }
    TLV_INIT(tlv2);
    }

    return BCM_E_NONE;
}

/* Function:
 * _field_groupqualoffset_recover
 *
 * Purpose:
 *    recover _bcm_field_qual_offset_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * offset_arr       - (OUT) Pointer to qualifier offset array structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_groupqualoffset_recover(int unit, _field_tlv_t *tlv,
                               _bcm_field_qual_offset_t *offset_arr)
{
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    uint32 *type;                         /* Elem type */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    int type_pos = 0;                     /* Position of type */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {
      type_pos = 0;
      offset_arr[i].field = KEYf;
      while ((tlv2.type != _bcmFieldInternalEndStructQualOffset)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN (tlv_read(unit, &tlv2, scache_ptr, position));
            switch (tlv2.type) {
                 case _bcmFieldInternalQualOffsetField:
                      break;
                 case _bcmFieldInternalQualOffsetNumOffset:
                      offset_arr[i].num_offsets = * (uint8 *)tlv2.value;
                      break;
                 case _bcmFieldInternalQualOffsetOffsetArr:
                      sal_memcpy(offset_arr[i].offset, tlv2.value,
                      ((recovery_type_map[unit][tlv2.type].size) * tlv2.length));
                      break;
                 case _bcmFieldInternalQualOffsetWidth:
                      sal_memcpy(offset_arr[i].width, tlv2.value,
                      ((recovery_type_map[unit][tlv2.type].size) * tlv2.length));
                      break;
                 case _bcmFieldInternalQualOffsetSec:
                      offset_arr[i].secondary = * (uint8 *)tlv2.value;
                      break;
                 case _bcmFieldInternalQualOffsetBitPos:
                      offset_arr[i].bit_pos = * (uint8 *)tlv2.value;
                      break;
                 case _bcmFieldInternalQualOffsetQualWidth:
                      offset_arr[i].qual_width = * (uint8 *)tlv2.value;
                      break;
                 case _bcmFieldInternalQualOffsetQualWidth16:
                      offset_arr[i].qual_width = * (uint16 *)tlv2.value;
                      break;
                 case _bcmFieldInternalEndStructQualOffset:
                      if (*(uint32 *)tlv2.value
                          != _FIELD_WB_EM_QUALOFFSET) {
                          LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                    "END MARKER CHECK FAILED : "
                                    "QUALOFFSET\n")));
                          TLV_INIT(tlv2);
                          return BCM_E_INTERNAL;
                      }
                      break;
                 default:
                      break;
            }
        type_pos++;

        }
        TLV_INIT(tlv2);

    }
    return BCM_E_NONE;

}


/* Function:
 * _field_groupqual_recover
 *
 * Purpose:
 *    recover _bcm_field_group_qual_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * fg               - (OUT) Pointer to field group structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_groupqual_recover(int unit, _field_tlv_t *tlv, _field_group_t *fg,
                         _bcm_field_group_qual_t qual_arr[_FP_MAX_ENTRY_TYPES]
                         [_FP_MAX_ENTRY_WIDTH], uint32 qual_count)
{
    int j = 0;
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scache pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    int num_instances = 0;                /* Number of instances */
    int rv = 0;                           /* Return Variable */
     uint8 entry_type;                    /* Index to tcam entries of
                                              multiple types            */
    _bcm_field_group_qual_t *q_arr;       /* Qualifiers array.        */
    int idx = 0;                          /* Loop index Variable */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    num_instances = (tlv->length);

    entry_type = _FP_ENTRY_TYPE_DEFAULT;

    for (j = 0; j < num_instances; j++) {

         while ((tlv2.type != _bcmFieldInternalEndStructGroupQual)) {
            TLV_INIT(tlv2);
            q_arr = NULL;
            /* coverity[no_write_call : FALSE] */
            rv = tlv_read(unit, &tlv2, scache_ptr, position);
            BCM_IF_ERROR_CLEANUP(rv);

            switch (tlv2.type) {
                 case _bcmFieldInternalQualQid:
                      _FP_XGS3_ALLOC(qual_arr[entry_type][j].qid_arr,
                                     ((recovery_type_map[unit][tlv2.type].size)
                                     * tlv2.length) , "FP Qual Array");
                      if (qual_arr[entry_type][j].qid_arr == NULL) {
                          rv = BCM_E_MEMORY;
                          goto cleanup;
                      }
                      sal_memcpy(qual_arr[entry_type][j].qid_arr,
                                 tlv2.value,
                                 ((recovery_type_map[unit][tlv2.type].size)
                                  * tlv2.length));
                      break;
                 case _bcmFieldInternalQualSize:
                      qual_arr[entry_type][j].size = * (uint16*)tlv2.value;
                      break;
                 case _bcmFieldInternalQualOffset:
                       _FP_XGS3_ALLOC(qual_arr[entry_type][j].offset_arr,
                                      (sizeof(_bcm_field_qual_offset_t)
                                        *(fg->qual_arr[entry_type][j].size)),
                                     "FP Qual Array");

                      if (qual_arr[entry_type][j].offset_arr == NULL) {
                          rv = BCM_E_MEMORY;
                          goto cleanup;
                      }
                      BCM_IF_ERROR_RETURN(
                           _field_groupqualoffset_recover(unit,
                                   &tlv2,
                                   qual_arr[entry_type][j].offset_arr));
                      break;
                 case _bcmFieldInternalEndStructGroupQual:
                      if (*(uint32 *)tlv2.value != _FIELD_WB_EM_QUAL) {
                          LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : QUAL\n")));
                          rv = BCM_E_INTERNAL;
                          goto cleanup;
                      }
                      break;
                 default:
                      break;
            }
        }
        if (qual_count != 0) {
            q_arr = &(qual_arr[entry_type][j]);
            for (idx = 0; idx < q_arr->size; idx++) {
                if (q_arr->qid_arr[idx] >= qual_count) {
                   q_arr->qid_arr[idx] = (q_arr->qid_arr[idx] - qual_count)
                                         + bcmFieldQualifyCount;
                 }
            }
        }
        TLV_INIT(tlv2);
    }

    return BCM_E_NONE;

cleanup:
    TLV_INIT(tlv2);
    if (qual_arr[entry_type][j].qid_arr != NULL) {
        sal_free(qual_arr[entry_type][j].qid_arr);
        qual_arr[entry_type][j].qid_arr = NULL;
    }
    if (qual_arr[entry_type][j].offset_arr != NULL) {
        sal_free(qual_arr[entry_type][j].offset_arr);
        qual_arr[entry_type][j].offset_arr = NULL;
    }

    return rv;

}

/* Function:
 * _field_ltinfo_recover
 *
 * Purpose:
 *    recover _field_lt_config_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * stage_fc         - (OUT) Pointer to field stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_ltinfo_recover(int unit, _field_tlv_t *tlv)
{
    int i = 0;                            /* local variable for loops */
    int j = 0;
    uint32 *type;                         /* Elem type */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    int type_pos = 0;                     /* position of type */
    int num_i_instances = 0;              /* num_instances in 1st dimension */
    int num_j_instances = 0;              /* num_instances in 2nd dimension */
    _field_lt_config_t *f_cfg = NULL;     /* Field config for LT */


    tlv2.value = NULL;
    TLV_INIT(tlv2);
    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    type = (uint32 *)tlv->value;
    num_i_instances = (tlv->length) & 0x000003ff;
    num_j_instances = ((tlv->length) & 0x003ffc00) >> 10;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_i_instances; i++) {
    for (j = 0; j < num_j_instances; j++) {

    f_cfg = fc->lt_info[i][j];
    type_pos = 0;

         while ((tlv2.type != _bcmFieldInternalEndStructLtConfig)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));
            switch (tlv2.type) {
                 case _bcmFieldInternalLtConfigValid:
                      f_cfg->valid = *(int *) tlv2.value;
                      break;
                 case _bcmFieldInternalLtConfigLtId:
                      f_cfg->lt_id = *(int *) tlv2.value;
                      break;
                 case _bcmFieldInternalLtConfigLtPartPri:
                      sal_memcpy(f_cfg->lt_part_pri, tlv2.value,
                                ((tlv2.length)
                                 * (recovery_type_map[unit][tlv2.type].size)));
                      break;
                 case _bcmFieldInternalLtConfigLtPartMap:
                      f_cfg->lt_part_map = *(uint32 *)tlv2.value;
                      break;
                 case _bcmFieldInternalLtConfigLtActionPri:
                      f_cfg->lt_action_pri = *(int *) tlv2.value;
                      break;
                 case _bcmFieldInternalLtConfigPri:
                      f_cfg->priority = *(int *) tlv2.value;
                      break;
                 case _bcmFieldInternalLtConfigFlags:
                      f_cfg->flags = *(int *) tlv2.value;
                      break;
                 case _bcmFieldInternalLtConfigEntry:
                      f_cfg->lt_entry = *(bcm_field_entry_t *) tlv2.value;
                      break;
                 case _bcmFieldInternalEndStructLtConfig:
                      if (*(uint32 *)tlv2.value != _FIELD_WB_EM_LTCONF) {
                          LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : LTCONF\n")));
                          TLV_INIT(tlv2);
                          return BCM_E_INTERNAL;
                      }
                      break;
                 default:
                      break;
            }
        type_pos++;

        }
    TLV_INIT(tlv2);
    }
    }

    return BCM_E_NONE;
}

/* Function:
 * _field_range_recover
 *
 * Purpose:
 *    recover _field_range_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * stage_fc         - (OUT) Pointer to field stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_range_recover(int unit, _field_tlv_t *tlv, _field_stage_t *stage_fc)
{
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    uint32 *type;                         /* Elem type */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    _field_range_t *new_fr = NULL;        /* New field range structure */
    _field_range_t *prev_fr = NULL;       /* Prev field range structure */
    int type_pos = 0;                     /* Positon of type */
    int rv = 0;                           /* Return variable */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {
      _FP_XGS3_ALLOC(new_fr, sizeof(_field_range_t), "FP Range");
      if (NULL == new_fr) {
          rv = BCM_E_MEMORY;
          goto cleanup;
      }
      type_pos = 0;
      while ((tlv2.type != _bcmFieldInternalEndStructRanges)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            rv = tlv_read(unit, &tlv2, scache_ptr, position);
            BCM_IF_ERROR_CLEANUP(rv);
            switch (tlv2.type) {
                case _bcmFieldInternalRangeFlags:
                     new_fr->flags = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalRangeRid:
                     new_fr->rid = *(bcm_field_range_t *)tlv2.value;
                     break;
                case _bcmFieldInternalRangeMin:
                     new_fr->min = *(bcm_l4_port_t *)tlv2.value;
                     break;
                case _bcmFieldInternalRangeMax:
                     new_fr->max = *(bcm_l4_port_t *)tlv2.value;
                     break;
                case _bcmFieldInternalRangeHwIndex:
                     new_fr->hw_index = *(int *)tlv2.value;
                     break;
                case _bcmFieldInternalRangeStyle:
                     new_fr->style = *(uint8 *)tlv2.value;
                     break;
                case _bcmFieldInternalEndStructRanges:
                     if (*(uint32 *)tlv2.value != _FIELD_WB_EM_RANGE) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : RANGE\n")));
                        rv = BCM_E_INTERNAL;
                        goto cleanup;
                     }
                     break;
                default:
                     break;
            }
        type_pos++;

        }
        new_fr->next = prev_fr;
        prev_fr = new_fr;
        new_fr = NULL;
        TLV_INIT(tlv2);

    }

cleanup:
    TLV_INIT(tlv2);
    stage_fc->ranges = prev_fr;

    if (new_fr != NULL) {
        sal_free(new_fr);
        new_fr = NULL;
    }
    return rv;
}

/* Function:
 * _field_datacontroldataqualifier_recover
 *
 * Purpose:
 *    recover _field_data_qualifier_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * fd               - (IN) Field data control structure
 * Returns:
 *     BCM_E_XXX
 */

int
_field_datacontroldataqualifier_recover(int unit, _field_tlv_t *tlv,
                                        _field_data_control_t *fd)
{
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    _field_data_qualifier_p new_f_dq = NULL;   /* New qualifier */
    _field_data_qualifier_p prev_f_dq = NULL;  /* Prev qualifier */
    int rv = 0;                           /* Return variable */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    num_instances = (tlv->length);

    for (i = 0; i < num_instances; i++) {

      _FP_XGS3_ALLOC(new_f_dq, sizeof(_field_data_qualifier_t),
                   "FP data qualifier");
      if (NULL == new_f_dq) {
          rv = BCM_E_MEMORY;
          goto cleanup;
      }

      _FP_XGS3_ALLOC(new_f_dq->spec, sizeof(bcm_field_udf_spec_t),
                   " FP data qualifier spec ");
      if (NULL == new_f_dq->spec) {
          rv = BCM_E_MEMORY;
          goto cleanup;
      }


      while (tlv2.type != _bcmFieldInternalDataControlEndStructDataQual) {
            TLV_INIT(tlv2);
            /* coverity[no_write_call : FALSE] */
            rv = tlv_read(unit, &tlv2, scache_ptr, position);
            if (rv < 0 ) {
                goto cleanup;
            }

            switch (tlv2.type) {
                case _bcmFieldInternalDataControlDataQualQid:
                     new_f_dq->qid = *(int *)tlv2.value;
                     break;
                case _bcmFieldInternalDataControlDataQualUdfSpec:
                     sal_memcpy(new_f_dq->spec->offset,tlv2.value,tlv2.length);
                     break;
                case _bcmFieldInternalDataControlDataQualOffsetBase:
                     new_f_dq->offset_base = * (bcm_field_data_offset_base_t *)
                                                tlv2.value;
                     break;
                case _bcmFieldInternalDataControlDataQualOffset:
                     new_f_dq->offset = * (int *)tlv2.value;
                     break;
                case _bcmFieldInternalDataControlDataQualByteOffset:
                     new_f_dq->byte_offset = * (uint8 *)tlv2.value;
                     break;
                case _bcmFieldInternalDataControlDataQualHwBmp:
                     new_f_dq->hw_bmap = * (uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalDataControlDataQualFlags:
                     new_f_dq->flags = * (uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalDataControlDataQualElemCount:
                     new_f_dq->elem_count = * (uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalDataControlDataQualLength:
                     new_f_dq->length = * (int *)tlv2.value;
                     break;
                case _bcmFieldInternalDataControlEndStructDataQual:
                     if (*(int *)tlv2.value != _FIELD_WB_EM_DATAQUAL) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                     "END MARKER CHECK FAILED : DATAQUAL\n")));
                        rv = BCM_E_INTERNAL;
                        goto  cleanup;
                     }
                     break;
                default:
                     break;
            }
        }
        new_f_dq->next = prev_f_dq;
        prev_f_dq = new_f_dq;
        new_f_dq = NULL;
        TLV_INIT(tlv2);

    }

cleanup:
    TLV_INIT(tlv2);
    fd->data_qual = prev_f_dq;

    if (new_f_dq != NULL) {
        if (new_f_dq->spec != NULL) {
            sal_free(new_f_dq->spec);
            new_f_dq->spec = NULL;
        }
        sal_free(new_f_dq);
        new_f_dq = NULL;
    }
    return rv;

}

/* Function:
 * _field_datacontrolprot_recover
 *
 * Purpose:
 *    recover _field_data_protocol_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * fd               - (IN) Field data control structure
 * Returns:
 *     BCM_E_XXX
 */

int
_field_datacontrolprot_recover(int unit, _field_tlv_t *tlv,
                               _field_data_control_t *fd)
{
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    uint32 *type;                         /* Elem type */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    int type_pos = 0;                     /* Positon of type */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {
      type_pos = 0;
      while ((tlv2.type != _bcmFieldInternalDataControlEndStructProt)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));
            switch (tlv2.type) {
               case _bcmFieldInternalDataControlProtIp4RefCount:
                    fd->ip[i].ip4_ref_count = *(int *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlProtIp6RefCount:
                    fd->ip[i].ip6_ref_count = *(int *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlProtFlags:
                    fd->ip[i].flags = *(uint32 *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlProtIp:
                    fd->ip[i].ip = *(uint8 *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlProtL2:
                    fd->ip[i].l2 = *(uint16 *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlProtVlanTag:
                    fd->ip[i].vlan_tag = *(uint16 *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlProtRelOffset:
                    fd->ip[i].relative_offset = *(int *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlEndStructProt:
                    if (*(uint32 *)tlv2.value != _FIELD_WB_EM_DATAPROT) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                    "END MARKER CHECK FAILED : DATAPROT\n")));
                        TLV_INIT(tlv2);
                        return BCM_E_INTERNAL;
                    }
                    break;
               default:
                    break;
            }
        type_pos++;

        }
        TLV_INIT(tlv2);
    }
    return BCM_E_NONE;
}


/* Function:
 * _field_datacontrolethertype_recover
 *
 * Purpose:
 *    recover _field_data_ethertype_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * fd               - (IN) Field data control structure
 * Returns:
 *     BCM_E_XXX
 */

int
_field_datacontrolethertype_recover(int unit, _field_tlv_t *tlv,
                                    _field_data_control_t *fd)
{
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    uint32 *type;                         /* Elem type */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    int type_pos = 0;                     /* Positon of type */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {
      type_pos = 0;
      while ((tlv2.type != _bcmFieldInternalDataControlEndStructEtype)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));
            switch (tlv2.type) {
              case _bcmFieldInternalDataControlEthertypeRefCount:
                    fd->etype[i].ref_count = * (int *)tlv2.value;
                    break;

               case _bcmFieldInternalDataControlEthertypeL2:
                    fd->etype[i].l2 = * (uint16 *)tlv2.value;
                    break;

               case _bcmFieldInternalDataControlEthertypeVlanTag:
                    fd->etype[i].vlan_tag = * (uint16 *)tlv2.value;
                    break;

               case _bcmFieldInternalDataControlEthertypePortEt:
                    fd->etype[i].ethertype = * (bcm_port_ethertype_t *)
                                                tlv2.value;
                    break;

               case _bcmFieldInternalDataControlEthertypeRelOffset:
                    fd->etype[i].relative_offset = * (int *)tlv2.value;
                    break;

               case _bcmFieldInternalDataControlEndStructEtype:
                    if (*(uint32 *)tlv2.value != _FIELD_WB_EM_DATAETYPE) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                    "END MARKER CHECK FAILED : DATATYPE\n")));
                        TLV_INIT(tlv2);
                        return BCM_E_INTERNAL;
                    }
                    break;
               default:
                    break;
            }
        type_pos++;

        }
        TLV_INIT(tlv2);
    }
    return BCM_E_NONE;
}

/* Function:
 * _field_datacontroltcamentry_recover
 *
 * Purpose:
 *    recover _field_data_tcam_entry_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * fd               - (IN) Field data control structure
 * Returns:
 *     BCM_E_XXX
 */

int
_field_datacontroltcamentry_recover(int unit, _field_tlv_t *tlv,
                                        _field_data_control_t *fd)
{
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    uint32 *type;                         /* Elem type */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.*/
    _field_data_tcam_entry_t *fent = NULL; /* Field tcam entry */
    int type_pos = 0;                     /* Positon of type */
    int num_types = 0;                    /* Number of types */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    fent = fd->tcam_entry_arr;
    if (fent == NULL) {
        /* data control not initilaized */
        return BCM_E_INIT;
    }

    for (i = 0; i < num_instances; i++) {
      type_pos = 0;
      while ((tlv2.type != _bcmFieldInternalDataControlEndStructTcam)
               && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);

            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));
            switch (tlv2.type) {
               case _bcmFieldInternalDataControlTcamRefCount:
                    fent->ref_count = * (uint8 *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlTcamPriority:
                    fent->priority = * (uint8 *)tlv2.value;
                    break;
               case _bcmFieldInternalDataControlEndStructTcam:
                    if (*(uint32 *)tlv2.value != _FIELD_WB_EM_DATATCAM) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                    "END MARKER CHECK FAILED : DATATCAM\n")));
                        TLV_INIT(tlv2);
                        return BCM_E_INTERNAL;
                    }
                    break;
               default:
                    break;
            }
        type_pos++;
        }
        fent++;
        TLV_INIT(tlv2);
    }
    return BCM_E_NONE;
}


/* Function:
 * _field_data_control_recover
 *
 * Purpose:
 *    recover _field_data_control_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * tlv              - (IN) TLV structure for field
 * stage_fc         - (OUT) Pointer to field stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_data_control_recover(int unit, _field_tlv_t *tlv,
                            _field_stage_t *stage_fc)
{
    _field_tlv_t tlv2;             /* Field TLV structure */
    uint8 *scache_ptr = NULL;        /* Pointer to Scache */
    uint32 *position = NULL;         /* Position in Scache */
    _field_control_t  *fc;           /* Field control structure.*/
    _field_data_control_t *fd = NULL;/* Field Data control structure */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;              /* Initialize value */
    TLV_INIT(tlv2);
    fd = stage_fc->data_ctrl;


    while (tlv2.type != _bcmFieldInternalEndStructDataControl) {
        TLV_INIT(tlv2);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));

        switch (tlv2.type)
        {
             case _bcmFieldInternalDataControlUsageBmp:
                  fd->usage_bmap = * (uint32 *)tlv2.value;
                  break;
             case _bcmFieldInternalDataControlDataQualStruct:
                  BCM_IF_ERROR_RETURN(
                       _field_datacontroldataqualifier_recover(unit,
                                                               &tlv2,
                                                               fd));
                  break;
             case _bcmFieldInternalDataControlElemSize:
                  fd->elem_size = *(int *)tlv2.value;
                  break;
             case _bcmFieldInternalDataControlNumElem:
                  fd->num_elems = *(int *)tlv2.value;
                  break;
             case _bcmFieldInternalDataControlEthertypeStruct:
                  BCM_IF_ERROR_RETURN(_field_datacontrolethertype_recover(unit,
                                                                 &tlv2,
                                                                 fd));
                  break;
             case _bcmFieldInternalDataControlProtStart:
                  BCM_IF_ERROR_RETURN(_field_datacontrolprot_recover(unit,
                                                                 &tlv2,
                                                                 fd));
                  break;
             case _bcmFieldInternalDataControlTcamStruct:
                  BCM_IF_ERROR_RETURN(_field_datacontroltcamentry_recover(unit,
                                                                 &tlv2,
                                                                 fd));

                  break;
             case _bcmFieldInternalEndStructDataControl:
                  if (*(uint32 *)tlv2.value != _FIELD_WB_EM_DATACONTROL) {
                      LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                  "END MARKER CHECK FAILED : DATACONTROL\n")));
                      TLV_INIT(tlv2);
                      return BCM_E_INTERNAL;
                  }
                  break;
             default:
                  break;
        }
    }
    TLV_INIT(tlv2);

    return BCM_E_NONE;
}


/* Function:
 * _field_stage_recover
 *
 * Purpose:
 *    recover _field_stage_t structure
 *
 * Paramaters:
 * unit             - (IN) BCM device number
 * stage_fc         - (IN) Pointer to device stage structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_stage_recover(int unit, _field_stage_t **stage_fc)
{
    _field_tlv_t tlv;              /* Field TLV structure */
    uint8 *scache_ptr = NULL;      /* Pointer to Scache */
    uint32 *position = NULL;       /* Position in Scache */
    _field_control_t  *fc;         /* Field control structure.*/

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv.value = NULL;              /* Initialize value */
    TLV_INIT(tlv);


    while (tlv.type != _bcmFieldInternalEndStructStage) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv, scache_ptr, position));

        switch (tlv.type)
        {
            case _bcmFieldInternalStageStageid:
                 (*stage_fc)->stage_id = *(_field_stage_id_t *)tlv.value;
                 break;
            case _bcmFieldInternalStageFlags:
                 (*stage_fc)->flags = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageTcamSz:
                 (*stage_fc)->tcam_sz = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageTcamSlices:
                 (*stage_fc)->tcam_slices = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageNumInstances:
                 (*stage_fc)->num_instances = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageNumPipes:
                 (*stage_fc)->num_pipes = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageRangeId:
                 (*stage_fc)->range_id = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalStageNumMeterPools:
                 (*stage_fc)->num_meter_pools = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageNumCntrPools:
                 (*stage_fc)->num_cntr_pools = *(unsigned *)tlv.value;
                 break;
            case _bcmFieldInternalStageLtTcamSz:
                 (*stage_fc)->lt_tcam_sz = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageNumLogicalTables:
                 (*stage_fc)->num_logical_tables = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageExtLevels:
                 (*stage_fc)->num_ext_levels = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStageOperMode:
                 if ((*(bcm_field_group_oper_mode_t *)tlv.value)
                                        == bcmFieldGroupOperModePipeLocal) {
                     BCM_IF_ERROR_RETURN(_bcm_field_wb_group_oper_mode_set(unit,
                                         bcmFieldQualifyStageIngress,
                                         stage_fc,
                                         bcmFieldGroupOperModePipeLocal));
                 }
                 break;
            case _bcmFieldInternalStageRanges:
                 BCM_IF_ERROR_RETURN(_field_range_recover(unit,
                                                          &tlv, (*stage_fc)));
                 break;
            case _bcmFieldInternalStageMeterPool:
                 BCM_IF_ERROR_RETURN(_field_meterpool_recover(unit,
                                                              &tlv, (*stage_fc)));
                 break;
            case _bcmFieldInternalStageCntrPools:
                 BCM_IF_ERROR_RETURN(_field_cntrpool_recover(unit,
                                                             &tlv, (*stage_fc)));
                 break;
            case _bcmFieldInternalStageLtInfo:
                 BCM_IF_ERROR_RETURN(_field_ltinfo_recover(unit,
                                                      &tlv));
                 break;
            case _bcmFieldInternalDataControlStart:
                 BCM_IF_ERROR_RETURN(_field_data_control_recover(unit,
                                                                 &tlv,
                                                                 (*stage_fc)));
                 break;
            case _bcmFieldInternalStageLtActionPri:
                 (*stage_fc)->lt_action_pri = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalStagePreselQset:
                 if (downgrade[unit] ==TRUE) {
                     sal_memcpy(&((*stage_fc)->presel_qset.w), tlv.value,
                         (recovery_type_map[unit][tlv.type].size *
                          _SHR_BITDCLSIZE(BCM_FIELD_QUALIFY_MAX)));
                 } else {
                     sal_memcpy(&((*stage_fc)->presel_qset.w), tlv.value,
                         (recovery_type_map[unit][tlv.type].size * tlv.length));
                 }
                 break;
            case _bcmFieldInternalEndStructStage:
                 if (*(uint32 *)tlv.value != _FIELD_WB_EM_STAGE) {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : STAGE\n")));
                     TLV_INIT(tlv);
                     return BCM_E_INTERNAL;
                 }
                 break;
            default:
                 break;
        }
    }
    TLV_INIT(tlv);

    return BCM_E_NONE;
}

/* Function:
 * _field_slice_recover
 *
 * Purpose:
 *    recover _field_slice_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * pipe_id       - (IN) Pipe to which this slice belongs
 * slice         - (IN) Pointer to device slice structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_slice_recover(int unit, int pipe_id, _field_slice_t *slice)
{
    _field_tlv_t tlv;                /* tlv structure */
    _field_control_t *fc = NULL;     /* Field control structure */
    _field_stage_t *stage_fc = NULL; /* Field stage structure */
    uint8 *scache_ptr;               /* Pointer to scache_ptr */
    uint8 temp_id;                   /* temp variable to store next
                                      * or prev slice id's
                                      */
    uint32 *position;                /* pointer to scache_pos */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS,
                                                 &stage_fc));
    tlv.value = NULL;
    TLV_INIT(tlv);

    while (tlv.type != _bcmFieldInternalEndStructSlice) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv, scache_ptr, position));

        switch (tlv.type) {

            case _bcmFieldInternalSliceNumber:
                  slice->slice_number = *(uint8 *)tlv.value;
                  break;
            case _bcmFieldInternalSliceStartTcamIdx:
                  slice->start_tcam_idx = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalSliceEntryCount:
                  slice->entry_count = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalSliceFreeCount:
                  slice->free_count = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalSliceCountersCount:
                  slice->counters_count = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalSliceMetersCount:
                  slice->meters_count = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalSliceInstalledEntriesCount:
                  /* slice->hw_ent_count = *(int *)tlv.value; */
                  slice->hw_ent_count = 0;
                  /* Since calculcation was wrong in older releases,
                   * resetting this field to 0 and then doing manual
                   * calculation during entry recover
                   */
                  break;
            case _bcmFieldInternalSliceCounterBmp:
                  sal_memcpy(&(slice->counter_bmp), tlv.value,
                              tlv.length);
                  break;
            case _bcmFieldInternalSliceMeterBmp:
                  sal_memcpy(&(slice->meter_bmp), tlv.value,
                          tlv.length);
                  break;
            case _bcmFieldInternalSliceStageId:
                  slice->stage_id = *(_field_stage_id_t *)tlv.value;
                  break;
            case _bcmFieldInternalSlicePortPbmp:
                  sal_memcpy(&(slice->pbmp), tlv.value,
                                (recovery_type_map[unit][tlv.type].size * tlv.length));

                  break;
            case _bcmFieldInternalSliceNextSlice:
                  temp_id = *(uint8 *)tlv.value;
                  if (slice->slice_number > temp_id) {
                      slice->next =
                          &SLICE_STRUCT(stage_fc, slices, pipe_id, temp_id);
                      SLICE_STRUCT(stage_fc, slices, pipe_id, temp_id).prev
                                                                    = slice;
                  }
                  break;

            case _bcmFieldInternalSlicePrevSlice:
                  temp_id = *(uint8 *)tlv.value;
                  if (slice->slice_number > temp_id) {
                      slice->prev =
                              &SLICE_STRUCT(stage_fc, slices, pipe_id, temp_id);
                      SLICE_STRUCT(stage_fc,slices,
                              pipe_id, temp_id).next = slice;
                  }
                  break;

            case _bcmFieldInternalSliceFlags:
                  slice->slice_flags = *(uint8 *)tlv.value;
                  break;
            case _bcmFieldInternalSliceGroupFlags:
                  slice->group_flags = *(uint8 *)tlv.value;
                  break;
            case _bcmFieldInternalSliceLtMap:
                  slice->lt_map = *(uint32 *)tlv.value;
                  break;
            case _bcmFieldInternalSliceLtPartitionPri:
                  slice->lt_partition_pri = *(uint8 *)tlv.value;
                  break;
            case _bcmFieldInternalExtractor:
                 BCM_IF_ERROR_RETURN(_field_extractor_recover(unit, scache_ptr,
                                     position, &tlv, slice->ext_sel));
                 break;
            case _bcmFieldInternalEndStructSlice:
                 if (*(uint32 *)tlv.value != _FIELD_WB_EM_SLICE) {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                        "END MARKER CHECK FAILED : SLICE\n")));
                     TLV_INIT(tlv);
                     return BCM_E_INTERNAL;
                 }
                 break;
            default:
                 break;
        }
    }
    TLV_INIT(tlv);
    return BCM_E_NONE;
}


/* Function:
 * _field_lt_slice_recover
 *
 * Purpose:
 *    recover _field_lt_slice_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * pipe_id       - (IN) Pipe to which this lt_slice belongs.
 * lt_slice      - (IN) Pointer to device lt_slice structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_lt_slice_recover(int unit, int pipe_id, _field_lt_slice_t *lt_slice)
{
    _field_tlv_t tlv;                /* tlv structure */
    _field_control_t *fc = NULL;     /* field control structure */
    _field_stage_t *stage_fc = NULL; /* field stage structure */
    uint8 *scache_ptr;               /*pointer to scache_ptr */
    uint8 temp_id;                   /* temp var to store next
                                      * and prev slice id's
                                      */
    uint32 *position;                /* pointer to scache_pos */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                        _BCM_FIELD_STAGE_INGRESS, &stage_fc));
    tlv.value = NULL;
    TLV_INIT(tlv);

    while (tlv.type != _bcmFieldInternalEndStructSlice) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv, scache_ptr, position));

        switch (tlv.type) {

            case _bcmFieldInternalLtSliceSliceNum:
                  lt_slice->slice_number = *(uint8 *)tlv.value;
                  break;
            case _bcmFieldInternalLtSliceStartTcamIdx:
                  lt_slice->start_tcam_idx = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalLtSliceEntryCount:
                  lt_slice->entry_count = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalLtSliceFreeCount:
                  lt_slice->free_count = *(int *)tlv.value;
                  break;
            case _bcmFieldInternalLtSliceStageid:
                  lt_slice->stage_id = *(_field_stage_id_t *)tlv.value;
                  break;
            case _bcmFieldInternalLtSliceNextSlice:
                  temp_id = *(uint8 *)tlv.value;
                  if (lt_slice->slice_number > temp_id) {
                      lt_slice->next =
                          &SLICE_STRUCT(stage_fc, lt_slices, pipe_id, temp_id);
                      SLICE_STRUCT(stage_fc,lt_slices, pipe_id, temp_id).prev
                                                                    = lt_slice;
                  }
                  break;
            case _bcmFieldInternalLtSlicePrevSlice:
                  temp_id = *(uint8 *)tlv.value;
                  if (lt_slice->slice_number > temp_id) {
                      lt_slice->prev =
                          &SLICE_STRUCT(stage_fc, lt_slices, pipe_id, temp_id);
                      SLICE_STRUCT(stage_fc,lt_slices, pipe_id,temp_id).next
                                                                = lt_slice;
                  }
                  break;
            case _bcmFieldInternalLtSliceFlags:
                  lt_slice->slice_flags = *(uint16 *)tlv.value;
                  break;
            case _bcmFieldInternalLtSliceGroupFlags:
                  lt_slice->group_flags = *(uint16 *)tlv.value;
                  break;
            case _bcmFieldInternalEndStructSlice:
                  if (*(uint32 *)tlv.value != _FIELD_WB_EM_LTSLICE) {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                     "END MARKER CHECK FAILED : LT SLICE\n")));
                     TLV_INIT(tlv);
                     return BCM_E_INTERNAL;
                  }
                  break;
            default:
                  break;
        }
    }
    TLV_INIT(tlv);
    return BCM_E_NONE;

}
#define _FIELD_ENTRY_WB_ACTION_INSERT(_ptr_, action)    \
    do {                                                \
        if (action != NULL) {                           \
            action->next = _ptr_;                       \
            _ptr_ = action;                             \
        }                                               \
        action = NULL;                                  \
    } while (0);                                        \

/* Function:
 * _field_action_profile_set1_recover
 *
 * Purpose:
 *    recover profile set 1 actions
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * f_ent             - (IN) Field Entry Structure
 * e_buf             - (IN) FP Policy Entry data
 * act               - (OUT) Pointer to action structure
 *
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_action_profile_set1_recover(int unit, _field_entry_t *f_ent,
                                   uint32* e_buf, _field_action_t *act)
{
    _bcm_field_action_offset_t a_offset; /* Action offsets */

    if (NULL == f_ent || NULL == act || NULL == e_buf) {
        LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META("Invalid Parameters\n")));
        return BCM_E_PARAM;
    }
    sal_memset(&a_offset, 0 , sizeof(a_offset));
    ACTION_GET(unit, f_ent, e_buf, act->action, 0, &a_offset);

    switch (act ->action) {

           case bcmFieldActionGpPrioIntCopy:
           case bcmFieldActionYpPrioIntCopy:
           case bcmFieldActionRpPrioIntCopy:
           case bcmFieldActionPrioIntCopy:
           case bcmFieldActionPrioIntTos:
           case bcmFieldActionGpPrioIntTos:
           case bcmFieldActionYpPrioIntTos:
           case bcmFieldActionRpPrioIntTos:
           case bcmFieldActionPrioIntCancel:
           case bcmFieldActionGpPrioIntCancel:
           case bcmFieldActionYpPrioIntCancel:
           case bcmFieldActionRpPrioIntCancel:
                /* No Params */
                break;

           case bcmFieldActionGpPrioIntNew:
           case bcmFieldActionYpPrioIntNew:
           case bcmFieldActionRpPrioIntNew:
           case bcmFieldActionGpDropPrecedence:
           case bcmFieldActionYpDropPrecedence:
           case bcmFieldActionRpDropPrecedence:
           case bcmFieldActionGpIntCongestionNotificationNew:
           case bcmFieldActionYpIntCongestionNotificationNew:
           case bcmFieldActionRpIntCongestionNotificationNew:
           case bcmFieldActionPfcClassNew:
           case bcmFieldActionUntaggedPacketPriorityNew:
                act->param[0] = a_offset.value[0];
                break;
           case bcmFieldActionGpCosMapNew:
           case bcmFieldActionYpCosMapNew:
           case bcmFieldActionRpCosMapNew:
                _BCM_COSQ_CLASSIFIER_FIELD_SET(act->param[0], a_offset.value[0]);
                break;
           case bcmFieldActionPrioIntNew:
                ACTION_GET(unit, f_ent, e_buf, bcmFieldActionGpPrioIntNew,
                           0, &a_offset);
                act->param[0] = a_offset.value[0];
                break;

           case bcmFieldActionDropPrecedence:
                ACTION_GET(unit, f_ent, e_buf, bcmFieldActionGpDropPrecedence,
                           0, &a_offset);
                act->param[0] = a_offset.value[0];
                break;
           case bcmFieldActionCosMapNew:
                ACTION_GET(unit, f_ent, e_buf, bcmFieldActionGpCosMapNew,
                           0, &a_offset);
                _BCM_COSQ_CLASSIFIER_FIELD_SET(act->param[0], a_offset.value[0]);
                break;
           default:
                return BCM_E_INTERNAL;


    }

    return BCM_E_NONE;
}


/* Function:
 * _field_action_profile_set2_recover
 *
 * Purpose:
 *    recover profile set 2 actions
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * f_ent             - (IN) Field Entry Structure
 * e_buf             - (IN) FP Policy Entry data
 * act               - (OUT) Pointer to action structure
 *
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_action_profile_set2_recover(int unit, _field_entry_t *f_ent,
                                   uint32* e_buf, _field_action_t *act)
{
    _bcm_field_action_offset_t a_offset;  /* Action offsets */

    if (NULL == f_ent || NULL == act || NULL == e_buf) {
        LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META("Invalid Parameters\n")));
        return BCM_E_PARAM;
    }
    sal_memset(&a_offset, 0 , sizeof(a_offset));
    ACTION_GET(unit, f_ent, e_buf, act->action, 0, &a_offset);

    switch (act ->action) {

           case bcmFieldActionGpEcnNew:
           case bcmFieldActionYpEcnNew:
           case bcmFieldActionRpEcnNew:
           case bcmFieldActionGpPrioPktNew:
           case bcmFieldActionYpPrioPktNew:
           case bcmFieldActionRpPrioPktNew:
           case bcmFieldActionRpDscpNew:
           case bcmFieldActionYpDscpNew:
           case bcmFieldActionGpDscpNew:
           case bcmFieldActionGpTosPrecedenceNew:
                act->param[0] = a_offset.value[0];
                break;

           case bcmFieldActionEcnNew:
                ACTION_GET(unit, f_ent, e_buf, bcmFieldActionGpEcnNew,
                           0, &a_offset);
                act->param[0] = a_offset.value[0];
                break;
           case bcmFieldActionPrioPktNew:
                ACTION_GET(unit, f_ent, e_buf, bcmFieldActionGpPrioPktNew,
                           0, &a_offset);
                act->param[0] = a_offset.value[0];
                break;
           case bcmFieldActionDscpNew:
                ACTION_GET(unit, f_ent, e_buf, bcmFieldActionGpDscpNew,
                           0, &a_offset);
                act->param[0] = a_offset.value[0];
                break;

           case bcmFieldActionGpPrioPktCopy:
           case bcmFieldActionGpPrioPktTos:
           case bcmFieldActionGpPrioPktCancel:
           case bcmFieldActionYpPrioPktCopy:
           case bcmFieldActionYpPrioPktTos:
           case bcmFieldActionYpPrioPktCancel:
           case bcmFieldActionRpPrioPktCopy:
           case bcmFieldActionRpPrioPktTos:
           case bcmFieldActionRpPrioPktCancel:
           case bcmFieldActionGpDscpCancel:
           case bcmFieldActionYpDscpCancel:
           case bcmFieldActionRpDscpCancel:
           case bcmFieldActionPrioPktCopy:
           case bcmFieldActionPrioPktTos:
           case bcmFieldActionDscpCancel:
           case bcmFieldActionPrioPktCancel:
           case bcmFieldActionGpTosPrecedenceCopy:
           case bcmFieldActionDot1pPreserve:
           case bcmFieldActionGpDot1pPreserve:
           case bcmFieldActionYpDot1pPreserve:
           case bcmFieldActionRpDot1pPreserve:
           case bcmFieldActionDscpPreserve:
           case bcmFieldActionGpDscpPreserve:
           case bcmFieldActionYpDscpPreserve:
           case bcmFieldActionRpDscpPreserve:
                /* No Params */
                break;
           default:
                return BCM_E_INTERNAL;

    }

    return BCM_E_NONE;

}


/* Function:
 * _field_action_l3swl2_recover
 *
 * Purpose:
 *    recover L3_SWITCH_L2 set actions
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * f_ent             - (IN) Field Entry Structure
 * e_buf             - (IN) FP Policy Entry data
 * act               - (OUT) Pointer to action structure
 *
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_action_l3swl2_recover(int unit, _field_entry_t *f_ent,
                                   uint32* e_buf, _field_action_t *act)
{
    _bcm_field_action_offset_t a_offset; /* action offsets */

    if (NULL == f_ent || NULL == act || NULL == e_buf) {
        LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META("Invalid Parameters\n")));
        return BCM_E_PARAM;
    }
    sal_memset(&a_offset, 0 , sizeof(a_offset));
    ACTION_GET(unit, f_ent, e_buf, act->action, 0, &a_offset);

    switch (act ->action) {

         case bcmFieldActionNewClassId:
         case bcmFieldActionAddClassTag:
         case bcmFieldActionMultipathHash:
         case bcmFieldActionBFDSessionIdNew:
              act->param[0] = a_offset.value[0];
              break;
#ifdef INCLUDE_L3
         case bcmFieldActionChangeL2Fields:
              act->param[0] = BCM_XGS3_EGRESS_IDX_MIN + a_offset.value[0];
              break;
#endif
         case bcmFieldActionL3SwitchCancel:
         case bcmFieldActionChangeL2FieldsCancel:
              /* No Params */
              break;
         default:
              return BCM_E_INTERNAL;

    }

    return BCM_E_NONE;

}

/* Function:
 * _field_action_misc_recover
 *
 * Purpose:
 *    recover misc actions
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * f_ent             - (IN) Field Entry Structure
 * e_buf             - (IN) FP Policy Entry data
 * act               - (OUT) Pointer to action structure
 *
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_action_misc_recover(int unit, _field_entry_t *f_ent,
                                   uint32* e_buf, _field_action_t *act)
{
    _bcm_field_action_offset_t a_offset;

    if (NULL == f_ent || NULL == act || NULL == e_buf) {
        LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META("Invalid Parameters\n")));
        return BCM_E_PARAM;
    }
    sal_memset(&a_offset, 0 , sizeof(a_offset));
    ACTION_GET(unit, f_ent, e_buf, act->action, 0, &a_offset);

    switch (act ->action) {
            case bcmFieldActionHgTrunkRandomRoundRobinHashCancel:
            case bcmFieldActionTrunkRandomRoundRobinHashCancel:
            case bcmFieldActionEcmpRandomRoundRobinHashCancel:
            case bcmFieldActionNatCancel:
            case bcmFieldActionDoNotCutThrough:
            case bcmFieldActionDoNotCheckUrpf:
            case bcmFieldActionDoNotChangeTtl:
            case bcmFieldActionMirrorOverride:
            case bcmFieldActionIngSampleEnable:
            case bcmFieldActionCopyToCpuCancel:
            case bcmFieldActionSwitchToCpuCancel:
            case bcmFieldActionSwitchToCpuReinstate:
            case bcmFieldActionDrop:
            case bcmFieldActionDropCancel:
            case bcmFieldActionGpCopyToCpuCancel:
            case bcmFieldActionGpSwitchToCpuCancel:
            case bcmFieldActionGpSwitchToCpuReinstate:
            case bcmFieldActionYpCopyToCpuCancel:
            case bcmFieldActionYpSwitchToCpuCancel:
            case bcmFieldActionYpSwitchToCpuReinstate:
            case bcmFieldActionRpCopyToCpuCancel:
            case bcmFieldActionRpSwitchToCpuCancel:
            case bcmFieldActionRpSwitchToCpuReinstate:
            case bcmFieldActionGpDrop:
            case bcmFieldActionGpDropCancel:
            case bcmFieldActionYpDrop:
            case bcmFieldActionYpDropCancel:
            case bcmFieldActionRpDrop:
            case bcmFieldActionRpDropCancel:
            case bcmFieldActionPacketTraceEnable:
            case bcmFieldActionNat:
#if defined(BCM_TOMAHAWK2_SUPPORT)
            case bcmFieldActionEgressTimeStampInsert:
            case bcmFieldActionIngressTimeStampInsert:
            case bcmFieldActionIngressTimeStampInsertCancel:
            case bcmFieldActionEgressTimeStampInsertCancel:
            case bcmFieldActionDynamicTrunkEnable:
            case bcmFieldActionDynamicTrunkCancel:
            case bcmFieldActionDynamicHgTrunkEnable:
            case bcmFieldActionDynamicHgTrunkCancel:
#if defined(INCLUDE_L3)
            case bcmFieldActionDynamicEcmpEnable:
            case bcmFieldActionDynamicEcmpCancel:
            case bcmFieldActionRecoverableDropCancel:
#endif /*INCLUDE_L3*/
#endif /*BCM_TOMAHAWK2_SUPPORT*/
                 /* No params */
                 break;
            case bcmFieldActionColorIndependent:
            case bcmFieldActionServicePoolIdNew:
            case bcmFieldActionCosQCpuNew:
                 act->param[0] = a_offset.value[0];
                 break;
            case bcmFieldActionServicePoolIdPrecedenceNew:
                 act->param[0] = a_offset.value[0];
                 /* SPAP is 2b'00 for BCM_FIELD_COLOR_GREEN
                            2b'01 for BCM_FIELD_COLOR_RED
                            2b'11 for BCM_FIELD_COLOR_YELLOW
                */
                 act->param[1] = ((0 == a_offset.value[1]) ? BCM_FIELD_COLOR_GREEN : \
                                 ((3 == a_offset.value[1])? BCM_FIELD_COLOR_YELLOW : BCM_FIELD_COLOR_RED));
                 break;
#ifdef INCLUDE_L3
            case bcmFieldActionNatEgressOverride:
                 act->param[0] =
                     ((a_offset.value[0] << 1) | (a_offset.value[1]));
                 break;
#if defined(BCM_TOMAHAWK2_SUPPORT)
            case bcmFieldActionDgm:
                break;
            case bcmFieldActionDgmThreshold:
            case bcmFieldActionDgmBias:
            case bcmFieldActionDgmCost:
                act->param[0] = a_offset.value[0];
                break;
#endif /*BCM_TOMAHAWK2_SUPPORT*/
#endif /*INCLUDE_L3*/

            default:
                 return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;

}
/* Function:
 * _field_action_eh_recover
 *
 * Purpose:
 *    recover bcmFieldActionFabricEHAddOrUpdate action.
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * f_ent             - (IN) Field Entry Structure
 * e_buf             - (IN) FP Policy Entry data
 * act               - (OUT) Pointer to action structure
 *
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_action_eh_recover(int unit, _field_entry_t *f_ent,
                         uint32* e_buf, _field_action_t *act)
{
    uint32 eh_mask = 0;   /* HiGiG2 extended header mask */
    int hw_index = 0; /* h/w index to EH_MASK_PROFILE table entry. */
    soc_profile_mem_t *eh_mask_profile = NULL; /* Profile pointer for EH_MASK_PROFILE table. */
    eh_mask_profile_entry_t *eh_mask_entry = NULL; /* pointer to EH_MASK_PROFILE table entry. */
    _field_stage_t *stage_fc = NULL; /* Field Stage Control Structure. */
    _bcm_field_action_offset_t a_offset; /* Field action offset information. */
    int rv = 0;

    /* NULL Parameters Check */
    if (NULL == f_ent || NULL == act || NULL == e_buf) {
        LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META("Invalid Parameters\n")));
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                        _BCM_FIELD_STAGE_INGRESS, &stage_fc));
    /* Fetch the action offset information. */
    sal_memset(&a_offset, 0 , sizeof(a_offset));
    ACTION_GET(unit, f_ent, e_buf, act->action, 0, &a_offset);
    switch (act ->action) {
        case bcmFieldActionFabricEHAddOrUpdate:
            _FP_XGS3_ALLOC(eh_mask_entry, sizeof(eh_mask_profile_entry_t),
                         "wb HiGiG2 extended header mask profile  entry");
            if (eh_mask_entry == NULL) {
                return BCM_E_MEMORY;
            }
            eh_mask_profile = &stage_fc->eh_mask_profile;
            if ((a_offset.value[3] == 6)) {
                /* First parameter is New extended header. */
                act->param[0] = a_offset.value[0];
                /* Second parameter is extended header mask. */
                /* coverity[check_return : FALSE] */
                hw_index = a_offset.value[1];
                rv = soc_mem_read(unit, EH_MASK_PROFILEm,
                             MEM_BLOCK_ANY, hw_index, eh_mask_entry);
                if (!SOC_SUCCESS(rv)) {
                    sal_free(eh_mask_entry);
                    return rv;
                }
                soc_mem_field_get(unit, EH_MASK_PROFILEm,
                                (uint32 *)eh_mask_entry, EH_MASKf, &eh_mask);
                act->param[1] = eh_mask;

                /* Increment the reference count for profile entry. */
                SOC_PROFILE_MEM_REFERENCE(unit, eh_mask_profile, hw_index, 1);
                SOC_PROFILE_MEM_ENTRIES_PER_SET(unit, eh_mask_profile,
                                                          hw_index, 1);
            }
            sal_free(eh_mask_entry);
            break;
        default:
            return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}
/* Function:
 * _field_action_sofware_recover
 *
 * Purpose:
 *    recover cached actions
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * f_ent             - (IN) Field Entry Structure
 * e_buf             - (IN) FP Policy Entry data
 * act               - (OUT) Pointer to action structure
 * val_array         - (OUT) Action value array
 * pos               - (OUT) reference to position.
 * Returns:
 *     BCM_E_XXX
 */
int
_field_action_software_recover(int unit, _field_entry_t *f_ent, uint32 *e_buf,
                               _field_action_t *act, int *val_array, int *pos,
                               uint8 action_param_type)
{
    int param_id = 0;                   /* Loop variable */
    int position = 0;                   /* pointer to position in the
                                         * array of values
                                         */
    _field_action_t *new_action = NULL; /* Used to hold multiple
                                         * actions structures in case
                                         * an action is repeated
                                         */
    _field_action_t *fa = act;          /* pointer to array of actions */

    position = *pos;
    switch (fa->action) {
            case bcmFieldActionCosQNew:
            case bcmFieldActionGpCosQNew:
            case bcmFieldActionYpCosQNew:
            case bcmFieldActionRpCosQNew:
            case bcmFieldActionUcastCosQNew:
            case bcmFieldActionGpUcastCosQNew:
            case bcmFieldActionYpUcastCosQNew:
            case bcmFieldActionRpUcastCosQNew:
            case bcmFieldActionMcastCosQNew:
            case bcmFieldActionGpMcastCosQNew:
            case bcmFieldActionYpMcastCosQNew:
            case bcmFieldActionRpMcastCosQNew:
            case bcmFieldActionEgressClassSelect:
            case bcmFieldActionHiGigClassSelect:
            case bcmFieldActionFabricQueue:
            case bcmFieldActionL3Switch:
            case bcmFieldActionRedirectMcast:
            case bcmFieldActionRedirectEgrNextHop:
            case bcmFieldActionRedirectIpmc:
            case bcmFieldActionRedirectCancel:
            case bcmFieldActionOuterVlanNew:
               for (param_id = 0; param_id < 1; param_id++) {
                    act->param[param_id] = val_array[position++];
               }
               break;
            case bcmFieldActionCopyToCpu:
            case bcmFieldActionTimeStampToCpu:
            case bcmFieldActionRpCopyToCpu:
            case bcmFieldActionRpTimeStampToCpu:
            case bcmFieldActionYpCopyToCpu:
            case bcmFieldActionYpTimeStampToCpu:
            case bcmFieldActionGpCopyToCpu:
            case bcmFieldActionGpTimeStampToCpu:
                 if (action_param_type == 1) {
                     for (param_id = 0; param_id < 2; param_id++) {
                          act->param[param_id] = val_array[position++];
                     }
                 }
                 break;
            case bcmFieldActionDstMacNew:
            case bcmFieldActionSrcMacNew:
            case bcmFieldActionUnmodifiedPacketRedirectPort:
            case bcmFieldActionRedirect:
            case bcmFieldActionEtagNew:
            case bcmFieldActionVnTagNew:
            case bcmFieldActionRedirectTrunk:
                 for (param_id = 0; param_id < 2; param_id++) {
                      act->param[param_id] = val_array[position++];
                 }
                 break;
          case bcmFieldActionRedirectPbmp:
          case bcmFieldActionEgressMask:
          case bcmFieldActionEgressPortsAdd:
          case bcmFieldActionRedirectBcastPbmp:
               for (param_id = 0; param_id < 4; param_id++) {
                    act->param[param_id] = val_array[position++];
               }
               break;
            case bcmFieldActionMirrorIngress:
            case bcmFieldActionMirrorEgress:
                 fa = act;
                 /* fetch all mirror params, if param is != -1,
                  * action is valid, hence alloc new action
                  */
                 for (param_id = 0; param_id < BCM_MIRROR_MTP_COUNT;
                      param_id++) {
                      if ((val_array[position] != -1)
                           && (val_array[position + 1] != -1)) {
                           if (param_id != 0) {
                              new_action = NULL;
                              _FP_XGS3_ALLOC(new_action,
                                      sizeof(_field_action_t), "actions");
                              if (new_action == NULL) {
                                  return BCM_E_MEMORY;
                              }
                              act->next = new_action;
                              act = act->next;
                           }
                           act->action = fa->action;
                           act->param[0] = val_array[position];
                           act->param[1] = val_array[position + 1];
                           if ((SOC_INFO(unit).th_ctc_replace_enabled) &&
                               (fa->action == bcmFieldActionMirrorEgress)) {
                               _bcm_field_action_offset_t a_offset;

                               sal_memset(&a_offset, 0 , sizeof(a_offset));
                               ACTION_GET(unit, f_ent, e_buf, bcmFieldActionFabricQueue, 0, &a_offset);
                               if (a_offset.value[1]) {
                                  act->param[2] = 1;
                                  act->param[3] = a_offset.value[1];
                                  act->flags |= _BCM_FIELD_ACTION_CTC_MANIPULATE_FLAG;
                               }
                           }
                           act->hw_index = -1;
                           act->old_index = -1;
                           act->flags |= _FP_ACTION_VALID ;

                      }
                      position = position + 2;
                 }
                 break;

            case bcmFieldActionRedirectVlan:
            case bcmFieldActionVnTagDelete:
            case bcmFieldActionEtagDelete:

      default:
             break;
    }
    *pos = position;
    return BCM_E_NONE;
}
/* Function:
 * _field_compression_actions_recover
 *
 * Purpose:
 *    recover parameters for compression actions
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * f_ent             - (IN) Field Entry Structure
 * fa               - (OUT) Pointer to action structure
 *
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_compression_actions_recover(int unit, _field_entry_t  *f_ent,
                        _field_action_t *fa) {
    uint32 cdata;               /* compression entry data */
    uint32 *cdata_ptr;          /* pointer to the same data */
    _field_class_type_t ctype; /* type of compression */
    soc_reg_t reg;              /* SOC Register */
    soc_mem_t mem;              /* SOC Memory */
    int rv = BCM_E_NONE;        /* return value */
    uint64 value;               /* to store 64 bit class_ids */
    int tcamsz = 0;
    /* Retreive Class Type from entry qset */
    rv = _bcm_field_th_class_type_qset_get(unit,
                                      &f_ent->group->qset, &ctype);
    if (BCM_FAILURE(rv)) {
        sal_free(fa);
        return (rv);
    }

    rv = _bcm_field_th_class_size_get(unit, ctype, &tcamsz);

    /* Assign class data buffer */
    if (f_ent->tcam.key == NULL) {
        if ((f_ent->flags & _FP_ENTRY_INSTALLED) &&
                (f_ent->slice_idx != -1)) {
            /* Allocate memory for class data */
            _FP_XGS3_ALLOC(f_ent->tcam.key,
                           tcamsz, "field class data buf");
            f_ent->tcam.key_size = tcamsz;
            if (f_ent->tcam.key == NULL) {
                sal_free(fa);
                return (BCM_E_MEMORY);
            }
            /* Entry is in hardware , read it from hardware */
            rv = _bcm_field_th_class_entry_hwread(unit, f_ent,
                                                  ctype, f_ent->tcam.key);
            if (BCM_FAILURE(rv)) {
                sal_free(f_ent->tcam.key);
                sal_free(fa);
                return (rv);
            }
        } else {
            sal_free(fa);
            return (BCM_E_INTERNAL);
        }
    }
    cdata = *(f_ent->tcam.key);
    cdata_ptr = f_ent->tcam.key;
    switch(ctype) {
        case _FieldClassEtherType:
            reg = ETHERTYPE_MAPr;
            fa->param[0] = soc_reg_field_get(unit, reg, cdata, C_ETHERTYPEf);
            break;
        case _FieldClassTtl:
            mem = TTL_FNm;
            if (fa->action == bcmFieldActionClassZero) {
                soc_mem_field_get(unit, mem, cdata_ptr,
                                          FN0f, &fa->param[0]);
            } else {
                soc_mem_field_get(unit, mem, cdata_ptr,
                                            FN1f, &fa->param[0]);
            }
            break;
        case _FieldClassToS:
            mem = TOS_FNm;
            if (fa->action == bcmFieldActionClassZero) {
                soc_mem_field_get(unit, mem, cdata_ptr,
                                            FN0f, &fa->param[0]);
            } else {
                soc_mem_field_get(unit, mem, cdata_ptr,
                                            FN1f, &fa->param[0]);
            }
            break;
        case _FieldClassIpProto:
            mem = IP_PROTO_MAPm;
            soc_mem_field_get(unit, mem, cdata_ptr,
                                        C_IP_PROTOCOLf, &fa->param[0]);
            break;
        case _FieldClassL4SrcPort:
            reg = L4_SRC_PORT_MAPr;
            fa->param[0] = soc_reg_field_get(unit, reg, cdata, C_L4_SRC_PORTf);
            break;
        case _FieldClassL4DstPort:
            reg = L4_DST_PORT_MAPr;
            fa->param[0] = soc_reg_field_get(unit, reg, cdata, C_L4_DST_PORTf);
            break;
        case _FieldClassTcp:
            mem = TCP_FNm;
            if (fa->action == bcmFieldActionClassZero) {
                soc_mem_field_get(unit, mem, cdata_ptr,
                                         FN0f, &fa->param[0]);
            } else {
                soc_mem_field_get(unit, mem, cdata_ptr,
                                            FN1f, &fa->param[0]);
            }
            break;
        case _FieldClassSrcCompression:
            mem = SRC_COMPRESSIONm;
            soc_mem_field64_get(unit, mem, cdata_ptr,
                                        C_SRCf, &value);
            COMPILER_64_TO_32_LO(fa->param[0], value);
            COMPILER_64_TO_32_HI(fa->param[1], value);
            break;
        case _FieldClassDstCompression:
            mem = DST_COMPRESSIONm;
            soc_mem_field64_get(unit, mem, cdata_ptr,
                                        C_DSTf, &value);
            COMPILER_64_TO_32_LO(fa->param[0], value);
            COMPILER_64_TO_32_HI(fa->param[1], value);
            break;
        default:
            rv = BCM_E_PARAM;
    }

    return rv;
}

/* Function:
 * _field_action_recover
 *
 * Purpose:
 *    recover _field_action_t structure
 *
 * Paramaters:
 * unit              - (IN) BCM device number
 * entry             - (IN) Field Entry Structure
 * act_bmp           - (IN) Action Bitmap
 * e_buf             - (IN) Policy memory data
 * val_array         - (IN) Array of parameters
 * hw_idx_array      - (IN) Array of hardware indices
 * action_count      - (OUT) Number of Action
 *
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_actions_recover(int unit, uint32 *e_buf,
                       _field_entry_t *entry,
                       _field_wb_entry_t *wb_entry)
{
    bcm_field_action_t action_id;         /* action id - used in loop */
    _field_action_t *curr_action = NULL;  /* current action */
    _field_action_t *fa = NULL;           /* pointer to actions array */
    int rv = 0, mirror_id = 0;            /* return value, loop variable */
    soc_profile_mem_t *redirect_profile;  /* pointer to redirect profile mem */
    int pos = 0;                          /* postion in the array of values
                                           * -val_array
                                           */
    int hw_count = 0;                     /* position in the array
                                           * of hw_indices
                                           */
    /* Validate input params , hw_index and val_array can be null in case there 
     * are actions that doesnt have params or hw_index
     */
    if (entry == NULL) {
         rv = BCM_E_INTERNAL;
         BCM_IF_ERROR_CLEANUP(rv);
    }

    if (wb_entry->act_bmp.w == NULL) {
        return BCM_E_NONE;
    }

    for (action_id = 0; action_id < wb_entry->action_count; action_id++) {
        if (_FP_ACTION_BMP_TEST(wb_entry->act_bmp, action_id)) {
            _FP_XGS3_ALLOC(curr_action, sizeof(_field_action_t), "actions");
            if (curr_action == NULL) {
                rv = BCM_E_MEMORY;
                goto cleanup;
            }
            curr_action->action = action_id;
            curr_action->hw_index = -1;
            curr_action->old_index = -1;
            curr_action->flags = _FP_ACTION_VALID;
            switch (action_id) {
                case bcmFieldActionGpPrioIntCopy:
                case bcmFieldActionGpPrioIntNew:
                case bcmFieldActionGpPrioIntTos:
                case bcmFieldActionGpPrioIntCancel:
                case bcmFieldActionYpPrioIntCopy:
                case bcmFieldActionYpPrioIntNew:
                case bcmFieldActionYpPrioIntTos:
                case bcmFieldActionYpPrioIntCancel:
                case bcmFieldActionRpPrioIntCopy:
                case bcmFieldActionRpPrioIntNew:
                case bcmFieldActionRpPrioIntTos:
                case bcmFieldActionRpPrioIntCancel:
                case bcmFieldActionGpDropPrecedence:
                case bcmFieldActionYpDropPrecedence:
                case bcmFieldActionRpDropPrecedence:
                case bcmFieldActionPrioIntCopy:
                case bcmFieldActionPrioIntNew:
                case bcmFieldActionPrioIntTos:
                case bcmFieldActionPrioIntCancel:
                case bcmFieldActionDropPrecedence:
                case bcmFieldActionGpIntCongestionNotificationNew:
                case bcmFieldActionYpIntCongestionNotificationNew:
                case bcmFieldActionRpIntCongestionNotificationNew:
                case bcmFieldActionPfcClassNew:
                case bcmFieldActionCosMapNew:
                case bcmFieldActionGpCosMapNew:
                case bcmFieldActionYpCosMapNew:
                case bcmFieldActionRpCosMapNew:
                case bcmFieldActionUntaggedPacketPriorityNew:
                rv = _field_action_profile_set1_recover(unit, entry,
                                                   e_buf, curr_action);
                BCM_IF_ERROR_CLEANUP(rv);
                break;
                case bcmFieldActionGpEcnNew:
                case bcmFieldActionYpEcnNew:
                case bcmFieldActionRpEcnNew:
                case bcmFieldActionGpPrioPktCopy:
                case bcmFieldActionGpPrioPktNew:
                case bcmFieldActionGpPrioPktTos:
                case bcmFieldActionGpPrioPktCancel:
                case bcmFieldActionYpPrioPktCopy:
                case bcmFieldActionYpPrioPktNew:
                case bcmFieldActionYpPrioPktTos:
                case bcmFieldActionYpPrioPktCancel:
                case bcmFieldActionRpPrioPktCopy:
                case bcmFieldActionRpPrioPktNew:
                case bcmFieldActionRpPrioPktTos:
                case bcmFieldActionRpPrioPktCancel:
                case bcmFieldActionGpTosPrecedenceNew:
                case bcmFieldActionGpTosPrecedenceCopy:
                case bcmFieldActionGpDscpNew:
                case bcmFieldActionGpDscpCancel:
                case bcmFieldActionYpDscpNew:
                case bcmFieldActionYpDscpCancel:
                case bcmFieldActionRpDscpNew:
                case bcmFieldActionRpDscpCancel:
                case bcmFieldActionEcnNew:
                case bcmFieldActionPrioPktCopy:
                case bcmFieldActionPrioPktNew:
                case bcmFieldActionPrioPktTos:
                case bcmFieldActionPrioPktCancel:
                case bcmFieldActionDscpNew:
                case bcmFieldActionDscpCancel:
                case bcmFieldActionDot1pPreserve:
                case bcmFieldActionGpDot1pPreserve:
                case bcmFieldActionYpDot1pPreserve:
                case bcmFieldActionRpDot1pPreserve:
                case bcmFieldActionDscpPreserve:
                case bcmFieldActionGpDscpPreserve:
                case bcmFieldActionYpDscpPreserve:
                case bcmFieldActionRpDscpPreserve:
                rv = _field_action_profile_set2_recover(unit, entry,
                                                  e_buf, curr_action);
                BCM_IF_ERROR_CLEANUP(rv);
                break;
                case bcmFieldActionNewClassId:
                case bcmFieldActionAddClassTag:
                case bcmFieldActionMultipathHash:
                case bcmFieldActionL3SwitchCancel:
#ifdef INCLUDE_L3
                case bcmFieldActionChangeL2Fields:
#endif
                case bcmFieldActionChangeL2FieldsCancel:
                case bcmFieldActionBFDSessionIdNew:
                rv = _field_action_l3swl2_recover(unit, entry,
                                            e_buf, curr_action);
                BCM_IF_ERROR_CLEANUP(rv);
                break;
                case bcmFieldActionHgTrunkRandomRoundRobinHashCancel:
                case bcmFieldActionTrunkRandomRoundRobinHashCancel:
                case bcmFieldActionEcmpRandomRoundRobinHashCancel:
                case bcmFieldActionNatCancel:
                case bcmFieldActionDoNotCutThrough:
                case bcmFieldActionDoNotCheckUrpf:
                case bcmFieldActionDoNotChangeTtl:
                case bcmFieldActionMirrorOverride:
                case bcmFieldActionIngSampleEnable:
                case bcmFieldActionCopyToCpuCancel:
                case bcmFieldActionSwitchToCpuCancel:
                case bcmFieldActionSwitchToCpuReinstate:
                case bcmFieldActionDrop:
                case bcmFieldActionDropCancel:
                case bcmFieldActionGpCopyToCpuCancel:
                case bcmFieldActionGpSwitchToCpuCancel:
                case bcmFieldActionGpSwitchToCpuReinstate:
                case bcmFieldActionYpCopyToCpuCancel:
                case bcmFieldActionYpSwitchToCpuCancel:
                case bcmFieldActionYpSwitchToCpuReinstate:
                case bcmFieldActionRpCopyToCpuCancel:
                case bcmFieldActionRpSwitchToCpuCancel:
                case bcmFieldActionRpSwitchToCpuReinstate:
                case bcmFieldActionGpDrop:
                case bcmFieldActionGpDropCancel:
                case bcmFieldActionYpDrop:
                case bcmFieldActionYpDropCancel:
                case bcmFieldActionRpDrop:
                case bcmFieldActionRpDropCancel:
                case bcmFieldActionNat:
#ifdef INCLUDE_L3
                case bcmFieldActionNatEgressOverride:
#endif
                case bcmFieldActionColorIndependent:
                case bcmFieldActionServicePoolIdNew:
                case bcmFieldActionServicePoolIdPrecedenceNew:
                case bcmFieldActionCosQCpuNew:
                case bcmFieldActionPacketTraceEnable:
#if defined(BCM_TOMAHAWK2_SUPPORT)
                case bcmFieldActionEgressTimeStampInsert:
                case bcmFieldActionIngressTimeStampInsert:
                case bcmFieldActionIngressTimeStampInsertCancel:
                case bcmFieldActionEgressTimeStampInsertCancel:
                case bcmFieldActionDynamicTrunkEnable:
                case bcmFieldActionDynamicTrunkCancel:
                case bcmFieldActionDynamicHgTrunkEnable:
                case bcmFieldActionDynamicHgTrunkCancel:
#if defined(INCLUDE_L3)
                case bcmFieldActionDynamicEcmpEnable:
                case bcmFieldActionDynamicEcmpCancel:
                case bcmFieldActionDgm:
                case bcmFieldActionDgmThreshold:
                case bcmFieldActionDgmBias:
                case bcmFieldActionDgmCost:
                case bcmFieldActionRecoverableDropCancel:
#endif /*INCLUDE_L3*/
#endif /*BCM_TOMAHAWK2_SUPPORT*/
                rv = _field_action_misc_recover(unit, entry,
                                           e_buf, curr_action);
                BCM_IF_ERROR_CLEANUP(rv);
                break;
                case bcmFieldActionCopyToCpu:
                case bcmFieldActionTimeStampToCpu:
                case bcmFieldActionRpCopyToCpu:
                case bcmFieldActionRpTimeStampToCpu:
                case bcmFieldActionYpCopyToCpu:
                case bcmFieldActionYpTimeStampToCpu:
                case bcmFieldActionGpCopyToCpu:
                case bcmFieldActionGpTimeStampToCpu:
                case bcmFieldActionCosQNew:
                case bcmFieldActionGpCosQNew:
                case bcmFieldActionYpCosQNew:
                case bcmFieldActionRpCosQNew:
                case bcmFieldActionUcastCosQNew:
                case bcmFieldActionGpUcastCosQNew:
                case bcmFieldActionYpUcastCosQNew:
                case bcmFieldActionRpUcastCosQNew:
                case bcmFieldActionMcastCosQNew:
                case bcmFieldActionGpMcastCosQNew:
                case bcmFieldActionYpMcastCosQNew:
                case bcmFieldActionRpMcastCosQNew:
                case bcmFieldActionEgressClassSelect:
                case bcmFieldActionHiGigClassSelect:
                case bcmFieldActionFabricQueue:
#ifdef INCLUDE_L3
                case bcmFieldActionL3Switch:
#endif
                case bcmFieldActionRedirect:
                case bcmFieldActionRedirectTrunk:
                case bcmFieldActionUnmodifiedPacketRedirectPort:
                case bcmFieldActionRedirectMcast:
#ifdef INCLUDE_L3
                case bcmFieldActionRedirectEgrNextHop:
                case bcmFieldActionRedirectIpmc:
#endif
                case bcmFieldActionRedirectCancel:
                case bcmFieldActionRedirectPbmp:
                case bcmFieldActionRedirectVlan:
                case bcmFieldActionRedirectBcastPbmp:
                case bcmFieldActionEgressMask:
                case bcmFieldActionEgressPortsAdd:
                case bcmFieldActionMirrorIngress:
                case bcmFieldActionMirrorEgress:
                case bcmFieldActionDstMacNew:
                case bcmFieldActionSrcMacNew:
                case bcmFieldActionOuterVlanNew:
                case bcmFieldActionVnTagNew:
                case bcmFieldActionVnTagDelete:
                case bcmFieldActionEtagNew:
                case bcmFieldActionEtagDelete:
                    rv = _field_action_software_recover(unit, entry, e_buf,
                                         curr_action, wb_entry->val_array, &pos,
                                         wb_entry->action_param_type);
                    BCM_IF_ERROR_CLEANUP(rv);
                    break;
                case bcmFieldActionFabricEHAddOrUpdate:
                    rv = _field_action_eh_recover(unit, entry,
                                             e_buf, curr_action);
                    BCM_IF_ERROR_CLEANUP(rv);
                    break;
                case bcmFieldActionClassOne:
                case bcmFieldActionClassZero:
                rv = _field_compression_actions_recover(unit, entry,
                                                    curr_action);
                break;
                default:
                     rv = BCM_E_INTERNAL;
                     goto cleanup;
             }

            switch (curr_action->action) {
                case bcmFieldActionDstMacNew:
                case bcmFieldActionSrcMacNew:
                case bcmFieldActionOuterVlanNew:
                case bcmFieldActionVnTagNew:
                case bcmFieldActionVnTagDelete:
                case bcmFieldActionEtagNew:
                case bcmFieldActionEtagDelete:
                case bcmFieldActionRedirectPbmp:
                case bcmFieldActionEgressMask:
                case bcmFieldActionEgressPortsAdd:
                case bcmFieldActionRedirectBcastPbmp:
                case bcmFieldActionRedirectEgrNextHop:
                case bcmFieldActionL3Switch:
                     curr_action->hw_index = wb_entry->hw_idx_array[hw_count++];
                     break;
                case bcmFieldActionMirrorIngress:
                case bcmFieldActionMirrorEgress:
                     fa = curr_action;
                     for (mirror_id = 0; mirror_id < BCM_MIRROR_MTP_COUNT;
                          mirror_id++) {
                          if (wb_entry->hw_idx_array[hw_count] != -1) {
                              fa->hw_index = wb_entry->hw_idx_array[hw_count];
                              if (fa ->next != NULL) {
                                  fa = fa->next;
                              }
                           }
                           hw_count++;
                     }
                     break;
                 default :
                    break;
            }


            /* Update profile memeory for redirect action's reference count*/
            if ((curr_action->action == bcmFieldActionRedirectBcastPbmp) ||
                (curr_action->action == bcmFieldActionRedirectPbmp) ||
                (curr_action->action == bcmFieldActionEgressMask) ||
                (curr_action->action == bcmFieldActionEgressPortsAdd)) {
                 BCM_IF_ERROR_RETURN(_field_trx_redirect_profile_get(unit,
                                                           &redirect_profile));
                 SOC_PROFILE_MEM_REFERENCE(unit, redirect_profile,
                                            curr_action->hw_index, 1);
                 SOC_PROFILE_MEM_ENTRIES_PER_SET(unit, redirect_profile,
                                                 curr_action->hw_index, 1);
            }
            if ((curr_action->action == bcmFieldActionMirrorIngress) ||
                (curr_action->action == bcmFieldActionMirrorEgress)) {
                 fa ->next = entry->actions;
            } else {
                 curr_action->next = entry->actions;
            }
            entry->actions = curr_action;
            curr_action = NULL;

        }
    }

cleanup:
    if (NULL != wb_entry->val_array) {
        sal_free(wb_entry->val_array);
    }
    if (NULL != wb_entry->hw_idx_array) {
        sal_free(wb_entry->hw_idx_array);
    }
    if (NULL != curr_action) {
        sal_free(curr_action);
    }
    return rv;
}

/* Function:
 * _field_lt_entry_info_recover
 *
 * Purpose:
 *    recover _field_lt_entry_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * f_lt_ent      - (IN/OUT) _field_lt_entry_t structure with memory allocated.
 * fc            - (IN) _field_control_t structure.
 * flags         - (IN/OUT) Array of uint32, to fill in the flags for each part.
 * Returns:
 *     BCM_E_XXX
 */
int
_field_lt_entry_info_recover(int unit, _field_lt_entry_t *f_lt_ent,
                             uint32 flags[], _field_group_t **new_fg)
{
    _field_tlv_t tlv;               /* tlv structure */
    uint8 *scache_ptr;              /* pointer to scache_ptr (base) */
    uint32 *position;               /* pointer to scache_pos (offset) */
    _field_control_t *fc = NULL;    /* pointer to field control structure */
    _field_stage_t *stage_fc = NULL;/* Field stage Structure */
    int temp;                       /* Temporary variable */
    int rv = 0;                     /* return Variable */
    _field_action_t *action = NULL; /* Lt entry action */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    stage_fc = curr_stage_fc;
    tlv.value = NULL;
    TLV_INIT(tlv);

    while (tlv.type != _bcmFieldInternalEndStructEntry) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv, scache_ptr, position));
        switch (tlv.type) {

            case _bcmFieldInternalEntryFlagsPart1:
                 f_lt_ent->flags = *(uint32 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryFlagsPart2:
                 flags[1] = *(uint32 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryFlagsPart3:
                 flags[2] = *(uint32 *)tlv.value;
                 break;


            case _bcmFieldInternalEntryEid:
                 f_lt_ent->eid = *(bcm_field_entry_t *)tlv.value;
                 break;


            case _bcmFieldInternalEntrySliceId:
                 f_lt_ent->index = *(uint32 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryPrio:
                 f_lt_ent->prio = *(int *)tlv.value;
                 break;


            case _bcmFieldInternalEntrySlice:
                 temp = *(int *)tlv.value;
                 f_lt_ent->lt_fs =
                     &stage_fc->lt_slices[f_lt_ent->group->instance][temp];
                 break;

            case _bcmFieldInternalGroupClassAct:
                 _FP_XGS3_ALLOC(action, sizeof(_field_action_t), "actions");
                 if (action == NULL) {
                     rv = BCM_E_MEMORY;
                     goto cleanup;
                 }
                 action->action = bcmFieldActionGroupClassSet;
                 action->param[0] = *(uint32 *)tlv.value;
                 f_lt_ent->actions = action;
                 break;

            case _bcmFieldInternalEntryGroup:
                 temp = *(int *)tlv.value;
                 if (new_fg != NULL) {
                    f_lt_ent->group = (new_fg[temp]);
                 }
                 break;

            case _bcmFieldInternalEndStructEntry:
                 if (*(uint32 *)tlv.value != _FIELD_WB_EM_LTENTRY) {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                      "END MARKER CHECK FAILED : LTENTRY\n")));
                     rv = BCM_E_INTERNAL;
                     goto cleanup;
                 }
                 break;

            default:
                 break;
        }
    }

cleanup:
    TLV_INIT(tlv);
    return rv;
}

/* Function:
 * _field_entry_policer_recover
 *
 * Purpose:
 *    recover field policers in  _field_entry_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * tlv           - (IN) tlv structure containing encoded types
 * f_ent         - (IN/OUT) _field_entry_t structure with memory allocated.
 * Returns:
 *     BCM_E_XXX
 */
int
_field_entry_policer_recover(int unit, _field_tlv_t *tlv, _field_entry_t *f_ent)
{

    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint32 *type;                         /* Elem type */
    uint8 *scache_ptr;                    /* Scahce pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t  *fc = NULL;         /* Field control structure.  */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    for (i = 0; i < num_instances; i++) {

        type_pos = 0;
        while  ((tlv2.type != _bcmFieldInternalEndStructEntPolicer)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));

            switch (tlv2.type) {

            case _bcmFieldInternalEntryPolicerPid:
                 f_ent->policer[i].pid = *(bcm_policer_t *)tlv2.value;
                 break;
            case _bcmFieldInternalEntryPolicerFlags:
                 f_ent->policer[i].flags = *(uint8 *)tlv2.value;
                 break;
            case _bcmFieldInternalEndStructEntPolicer:
                 TLV_INIT(tlv2);
                 break;
            default:
                 break;
            }
            type_pos++;
        }
    }
    return BCM_E_NONE;
}

/* Function:
 * _field_entry_info_recover
 *
 * Purpose:
 *    recover _field_entry_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * f_ent         - (IN/OUT) _field_entry_t structure with memory allocated.
 * flags         - (IN/OUT) Array of uint32, to fill in the flags for each part.
 * parts_count   - (IN) number of parts of Field Entry.
 * new_fg        - (IN) Array of Groups
 * act_bmp       - (OUT) Pbmp of Actions
 * val_array     - (OUT) Array of parameters
 * hw_idx_array  - (OUT) Array of hardware indices
 * action_count  - (OUT) Number of Action
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_entry_info_recover(int unit, _field_entry_t *f_ent,
                          int parts_count, _field_group_t **new_fg,
                          _field_wb_entry_t *wb_entry)

{
    _field_tlv_t tlv;                  /* tlv structure */
    uint8 *scache_ptr;                 /* pointer to scache_ptr (base) */
    uint32 *position;                  /* Pointer to scache_pos (offset) */
    /* uint32 flag_cpy[_FP_TH_MAX_ENTRY_WIDTH] = {0}; */
                                       /* array to flags of each entry part,
                                        * entries parts of the same
                                        * entry differ only in flags
                                        */
    _field_control_t *fc = NULL;       /* pointer to field control structure */
    _field_stage_t *stage_fc = NULL;   /* Field stage structure */
    int temp = 0;                      /* Temporary variable */
    int rv = 0;                        /* Return Variable */

    tlv.value = NULL;
    TLV_INIT(tlv);

    wb_entry->act_bmp.w = NULL;

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    stage_fc = curr_stage_fc;

    while (tlv.type != _bcmFieldInternalEndStructEntry) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        rv = tlv_read(unit, &tlv, scache_ptr, position);
        BCM_IF_ERROR_CLEANUP(rv);
        switch (tlv.type)
        {

            case _bcmFieldInternalEntryEid:
                 f_ent->eid = *(bcm_field_entry_t *)tlv.value;
                 break;
            case _bcmFieldInternalEntryPrio:
                 f_ent->prio = *(int *)tlv.value;
                 break;

            case _bcmFieldInternalEntrySliceId:
                 f_ent->slice_idx = *(uint32 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryFlagsPart1:
                 f_ent->flags = *(uint32 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryFlagsPart2:
                 wb_entry->flags[1] = *(uint32 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryFlagsPart3:
                 wb_entry->flags[2] = *(uint32 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryPbmpData:
                 f_ent->pbmp.data = *(bcm_pbmp_t *)tlv.value;
                 break;

            case _bcmFieldInternalEntryPbmpMask:
                 f_ent->pbmp.mask = *(bcm_pbmp_t *)tlv.value;
                 break;

            case _bcmFieldInternalEntryPbmpFullData:
                 sal_memcpy(f_ent->pbmp.data.pbits, tlv.value,
                            (tlv.length * recovery_type_map[unit][tlv.type].size));
                 break;

            case _bcmFieldInternalEntryPbmpFullMask:
                 sal_memcpy(f_ent->pbmp.mask.pbits, tlv.value,
                            (tlv.length * recovery_type_map[unit][tlv.type].size));
                 break;

            case _bcmFieldInternalEntrySlice:
                 temp = *(int *)tlv.value;
                 f_ent->fs = &stage_fc->slices[f_ent->group->instance][temp];
                 break;

            case _bcmFieldInternalEntryGroup:
                 temp = *(int *)tlv.value;
                 f_ent->group = (new_fg[temp]);
                 break;

            case _bcmFieldInternalEntryActionsPbmp:
                 _FP_XGS3_ALLOC(wb_entry->act_bmp.w,
                     (tlv.length * recovery_type_map[unit][tlv.type].size),
                     "action_bmp");
                 if (wb_entry->act_bmp.w == NULL) {
                     rv = BCM_E_MEMORY;
                     goto cleanup;
                 }
                 wb_entry->action_count = (tlv.length
                                * recovery_type_map[unit][tlv.type].size * 8);
                 sal_memcpy(wb_entry->act_bmp.w, tlv.value,
                      (tlv.length * recovery_type_map[unit][tlv.type].size));
                 break;

            case _bcmFieldInternalActionParam:
                 wb_entry->val_array = (int *)tlv.value;
                 wb_entry->action_param_type = 0;
                 tlv.value = NULL;
                 break;

            case _bcmFieldInternalActionParam1:
                 wb_entry->val_array = (int *)tlv.value;
                 wb_entry->action_param_type = 1;
                 tlv.value = NULL;
                 break;

            case _bcmFieldInternalActionHwIdx:
                 wb_entry->hw_idx_array = (int *)tlv.value;
                 tlv.value = NULL;
                 break;

            case _bcmFieldInternalEntryStatSid:
                 f_ent->statistic.sid = *(int *)tlv.value;
                 break;

            case _bcmFieldInternalEntryStatExtendedSid:
                 f_ent->statistic.extended_sid = *(int *)(tlv.value);
                 break;
            case _bcmFieldInternalEntryStatFlags:
                 f_ent->statistic.flags = *(uint16 *)tlv.value;
                 break;
            case _bcmFieldInternalEntryStatAction:
                 f_ent->statistic.stat_action =
                            *(bcm_field_stat_action_t *)tlv.value;
                 break;

            case _bcmFieldInternalEntryPolicer:
                  _field_entry_policer_recover(unit, &tlv, f_ent);
                 break;

            case _bcmFieldInternalGlobalEntryPolicerPid:
                 f_ent->global_meter_policer.pid = *(bcm_policer_t *) tlv.value;
                 break;

            case _bcmFieldInternalGlobalEntryPolicerFlags:
                 f_ent->global_meter_policer.flags = *(uint8 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryIngMtp:
                 f_ent->ing_mtp_slot_map = *(uint8 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryEgrMtp:
                 f_ent->egr_mtp_slot_map = *(uint8 *)tlv.value;
                 break;

            case _bcmFieldInternalEntryDvp:
                 f_ent->dvp_type = *(uint8 *)tlv.value;
                 break;
            case _bcmFieldInternalEntrySvpType:
                 f_ent->svp_type = *(uint8 *)tlv.value;
                 break;
            case _bcmFieldInternalEntryTcamIptype:
                 f_ent->tcam.ip_type = *(uint8 *)tlv.value;
                 break;
            case _bcmFieldInternalEntryCopy:
                 /*
                 _FP_XGS3_ALLOC(f_ent->ent_copy,
                 parts_count* sizeof(_field_entry_t), "field entry copy");
                 _field_entry_info_recover(unit, f_ent->ent_copy, flag_cpy,
                                           parts_count, new_fg, policy_mem,
                                           policy_buf);
                 if (parts_count > 1) {
                    for (j=1; j < parts_count; j++) {
                        sal_memcpy((f_ent->ent_copy)+j, (f_ent->ent_copy),
                                                      sizeof(_field_entry_t));
                        ((f_ent->ent_copy)+j)->flags = flag_cpy[j];
                    }
                 }
                 */
                 break;

            case _bcmFieldInternalEndStructEntry:
                 if (*(uint32 *)tlv.value != _FIELD_WB_EM_ENTRY) {
                        LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : ENTRY\n")));
                     rv = BCM_E_INTERNAL;
                     goto cleanup;
                 }
                 break;

            default:
                 break;
        }
    }
    TLV_INIT(tlv);
    return BCM_E_NONE;
cleanup:
    TLV_INIT(tlv);
    if (f_ent->actions != NULL) {
        sal_free(f_ent->actions);
        f_ent->actions = NULL;
    }
    if (f_ent->ent_copy != NULL) {
       sal_free(f_ent->ent_copy);
       f_ent->ent_copy = NULL;
    }
    return rv;
}

/* Function:
 * _field_entry_recover_policer_actual_hw_rates
 *
 * Purpose:
 *  Recover actual hardware rates for policers already
 *  installed in coldboot.
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * f_ent         - (IN) Field entry structure
 *
 * Returns:
 *  BCM_E_XXX
 */

int _field_entry_recover_policer_actual_hw_rates(int unit, _field_entry_t *f_ent)
{
    _field_policer_t        *f_pl;          /*Internal policer descriptor.  */
    _field_entry_policer_t  *f_ent_pl;      /*Field entry policer structure.*/
    _field_stage_t          *stage_fc;      /*Field stage control structure.*/
    soc_mem_t               policer_mem = INVALIDm; /*Meter table name.     */
    int                     idx;            /*Policers levels iterator.     */
    int                     rv = BCM_E_NONE;/*Operation return status.      */

    for (idx = 0; idx < _FP_POLICER_LEVEL_COUNT; idx++) {
        f_ent_pl = f_ent->policer + idx;

        rv = _bcm_field_policer_get(unit, f_ent_pl->pid, &f_pl);

        if (BCM_SUCCESS(rv)) {
            if(!(f_ent->policer[idx].flags & _FP_POLICER_INSTALLED))  {
                continue;
            }
            rv = _field_stage_control_get (unit, f_ent->fs->stage_id, &stage_fc);
            BCM_IF_ERROR_RETURN(rv);

            /* Resolve meter table name. */
            rv = _bcm_field_th_policer_mem_get(unit, stage_fc, f_ent->group->instance,
                    &policer_mem);
            BCM_IF_ERROR_RETURN(rv);

            rv = _field_entry_policer_update_actual_hw_rates(unit, policer_mem,
                    f_ent->group->instance,
                    f_ent, f_pl);
            BCM_IF_ERROR_RETURN(rv);
        }
    }
    return BCM_E_NONE;
}

/* Function:
 * _field_entry_recover
 *
 * Purpose:
 *    recover _field_entry_t and _field_lt_entry_t structures
 *    For each group,
 *       Get partscount and number of entries, call entry recover for each entry
 *       Using the same parts count, and get number of lt_entries, call
 *       lt_entry_recover for each lt_entry.
 *       Proceed to next group.
 *    when _bcmFieldInternalEndStructEntryDetails is reached, that means all
 *    entries in all groups are recovered.
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_field_entry_recover(int unit, _field_group_t **new_fg)
{
    _field_tlv_t tlv;                     /* tlv structure */
    uint8 *scache_ptr;                    /* pointer to scache_pointer (base) */
    uint32 *position;                     /* pointer to scache_pos */
    uint32 flags[_FP_TH_MAX_ENTRY_WIDTH]   = {0};
                                          /* Flags array to store flags
                                           * for each entry part
                                           */
    _field_control_t *fc = NULL;          /* pointer to field control structure */
    _field_entry_t *f_ent = NULL;         /* pointer to entry structure */
    _field_lt_entry_t *f_lt_ent = NULL;   /* pointer to lt_entry structure */
    _field_dynamic_map_entry_t *dyf_ent = NULL;
                                          /* pointer to entry in dynamic map */
    _field_dynamic_map_entry_t *dyf_ent_tail = NULL;
                                          /* Tail of dynamic entry map LL */
    _field_dynamic_map_lt_entry_t *dyf_lt_ent= NULL;
                                          /* Dynamic  LT entry map structure */
    _field_dynamic_map_lt_entry_t *dyf_lt_ent_tail = NULL;
                                          /* Tail of Dynamic LT entry map LL */

    int i,j, parts_count = 0;             /* i,j - loop variables
                                           * parts_count ->
                                           * Number of parts in entry
                                           */
    int entry_count = 0, rv = 0;          /* entry_count -> number of entries
                                           * rv -> return value
                                           */
    _field_stage_t *stage_fc = NULL;      /* Field Stage structure */
    int pri_tcam_idx = -1;                /* Primary tcam index */
    int policy_tcam_idx = -1;             /* Policy memory index */
    int slice_number = 0;                 /* Slice where entry belongs */
    int part_index =  0;                  /* To loop through parts */
    soc_mem_t *fp_policy_mem;             /* IFP Policy memeory */
    char *fp_policy_buf[_FP_MAX_NUM_PIPES] =  {0};
                                          /* Buffer to read the FP_POLICY table */
    soc_mem_t *lt_policy_mem;             /* LT Policy memory */
    char *lt_policy_buf[_FP_MAX_NUM_PIPES] = {0};
                                          /* Buffer to read the FP_POLICY table */
    int index_min = -1, index_max = -1;   /* Min and max index in Policy memory */
    int pipe = 0, num_pipes = 0;          /*  Pipe variable and count of pipes */
    uint32 *e_buf = NULL;                 /* Buffer holding corresponding entry */
    _field_stat_t *f_st = NULL;           /* Field Stat structure */
    soc_mem_t  policy_tcam_mems[_FP_MAX_NUM_PIPES + 1] = {
                                                        IFP_POLICY_TABLE_PIPE0m,
                                                        IFP_POLICY_TABLE_PIPE1m,
                                                        IFP_POLICY_TABLE_PIPE2m,
                                                        IFP_POLICY_TABLE_PIPE3m,
                                                        IFP_POLICY_TABLEm
                                                      };
                                          /* IFP policy memory for every Pipe */
    soc_mem_t lt_policy_mems[_FP_MAX_NUM_PIPES + 1] =
                                 {
                                    IFP_LOGICAL_TABLE_SELECT_DATA_ONLY_PIPE0m,
                                    IFP_LOGICAL_TABLE_SELECT_DATA_ONLY_PIPE1m,
                                    IFP_LOGICAL_TABLE_SELECT_DATA_ONLY_PIPE2m,
                                    IFP_LOGICAL_TABLE_SELECT_DATA_ONLY_PIPE3m,
                                    IFP_LOGICAL_TABLE_SELECT_DATA_ONLYm
                                 };
                                          /* LT policy memory for every Pipe */
    bcm_stat_group_mode_t   stat_mode;    /* Stat type bcmStatGroupModeXXX. */
    bcm_stat_object_t       stat_obj;     /* Stat object type. */
    bcm_stat_flex_mode_t    offset_mode;  /* Counter mode.     */
    int fp_stat_mode_max;                 /* Max stat modes. */
    uint8  idx;                           /* Index to carry loop count. */
    uint32 pool_num;                      /* Flex Stat Hw Pool No. */
    uint32 base_index;                    /* Flex Stat counter base index. */
    uint32 num_hw_cntrs;                  /* Number of counters allocated. */
    uint32 req_bmap = 0;                  /* Requested statistics bitmap.  */
    uint32 hw_bmap;                       /* HW supported statistics bitmap. */
    _field_wb_entry_t wb_entry;    /* Set of WB entry info */

    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                         _BCM_FIELD_STAGE_INGRESS, &stage_fc));
    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv.value = NULL;
    TLV_INIT(tlv);

    wb_entry.action_count = -1;
    wb_entry.action_param_type = -1;
    wb_entry.hw_idx_array = NULL;
    wb_entry.val_array = NULL;

    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        num_pipes = 1;
        fp_policy_mem = &policy_tcam_mems[_FP_MAX_NUM_PIPES];
        lt_policy_mem = &lt_policy_mems[_FP_MAX_NUM_PIPES];
    } else {
        num_pipes = stage_fc->num_pipes;
        fp_policy_mem = &policy_tcam_mems[0];
        lt_policy_mem = &lt_policy_mems[0];
    }

    for (pipe = 0; pipe < num_pipes; pipe++) {

        fp_policy_buf[pipe] = soc_cm_salloc(unit,
                           SOC_MEM_TABLE_BYTES(unit, fp_policy_mem[pipe]),
                           "FP POLICY TABLE buffer");
        lt_policy_buf[pipe] = soc_cm_salloc(unit,
                           SOC_MEM_TABLE_BYTES(unit, lt_policy_mem[pipe]),
                           "FP POLICY TABLE buffer");
        if ((NULL == fp_policy_buf[pipe]) || (NULL == lt_policy_buf[pipe])) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        index_min = soc_mem_index_min(unit, fp_policy_mem[pipe]);
        index_max = soc_mem_index_max(unit, fp_policy_mem[pipe]);
        rv = soc_mem_read_range(unit, fp_policy_mem[pipe], MEM_BLOCK_ALL,
                                   index_min, index_max, fp_policy_buf[pipe]);
        BCM_IF_ERROR_CLEANUP(rv);

        index_min = soc_mem_index_min(unit, lt_policy_mem[pipe]);
        index_max = soc_mem_index_max(unit, lt_policy_mem[pipe]);
        rv = soc_mem_read_range(unit, lt_policy_mem[pipe], MEM_BLOCK_ALL,
                                 index_min, index_max, lt_policy_buf[pipe]);
        BCM_IF_ERROR_CLEANUP(rv);
    }

    while (tlv.type != _bcmFieldInternalEndStructEntryDetails) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        rv = tlv_read(unit, &tlv, scache_ptr, position);
        BCM_IF_ERROR_CLEANUP(rv);
        switch (tlv.type) {
            case _bcmFieldInternalGroupPartCount:
                 parts_count = *(int *)tlv.value;
                 break;

            case _bcmFieldInternalGroupEntry:
                 entry_count = *(int *)tlv.value;
                 for (i = 0; i < entry_count; i++) {
                    _FP_XGS3_ALLOC(f_ent, parts_count* sizeof(_field_entry_t),
                                                                "field entry");
                    if (f_ent == NULL) {
                        rv = BCM_E_MEMORY;
                        goto cleanup;
                    }

                    _FP_XGS3_ALLOC(dyf_ent, sizeof(_field_dynamic_map_entry_t),
                                    "temp ent");
                    sal_memset(wb_entry.flags, 0, sizeof(wb_entry.flags));
                    if (dyf_ent == NULL) {
                        rv = BCM_E_MEMORY;
                        goto cleanup;
                    }

                    /* Recover entry */
                    rv = _field_entry_info_recover(unit, f_ent,
                                                   parts_count, new_fg,
                                                   &wb_entry);
                    BCM_IF_ERROR_CLEANUP(rv);

                    /* Increment Stat reference counts */
                    f_st = NULL;
                    rv = _bcm_field_stat_get(unit, f_ent->statistic.sid,
                                             &f_st);
                    if ((BCM_SUCCESS(rv)) && (f_st->flex_mode != 0)) {
                        offset_mode = pool_num = base_index = num_hw_cntrs = -1;

                        /* Get flex counter hardware details stored in flex stat ID.*/
                        _bcm_esw_stat_get_counter_id_info(unit, f_st->flex_mode,
                                                          &stat_mode,
                                                          &stat_obj,
                                                          &offset_mode,
                                                          &pool_num,
                                                          &base_index);

                        if ((bcmStatObjectIngFieldStageIngress == stat_obj) ||
                            (bcmStatObjectIngExactMatch == stat_obj)){
                           /* Get application requested bitmap. */
                           rv = (_bcm_field_stat_array_to_bmap(unit, f_st,
                                                               &req_bmap));
                           BCM_IF_ERROR_CLEANUP(rv);

                           fp_stat_mode_max = th_ingress_cntr_hw_mode_tbl_size;

                           for (idx = 0; idx < fp_stat_mode_max; idx++) {
                              hw_bmap = th_ingress_cntr_hw_mode_tbl[idx].hw_bmap;
                              num_hw_cntrs = th_ingress_cntr_hw_mode_tbl[idx].hw_entry_count;

                              if (0 == ((req_bmap | hw_bmap) & ~(hw_bmap))) {
                                 /*
                                  * For maximizing utilization of hardware counters, this should
                                  * select the hw_mode both supports the requested stats
                                  * AND minimizes the number of required hardware counters.
                                  */
                                  break;
                              }
                           }
                        } else {
                           num_hw_cntrs = 1;
                        }

                        /* Store flex stat hardware details in stat data structure. */
                        f_st->pool_index = pool_num;
                        f_st->hw_index = base_index;
                        f_st->hw_mode = offset_mode;
                        f_st->hw_entry_count = num_hw_cntrs;
                        f_st->hw_stat = req_bmap;
                        f_st->sw_ref_count =  f_st->sw_ref_count + 1;

                        if(_FP_ENTRY_INSTALLED & f_ent->flags) {
                           f_st->hw_ref_count =  f_st->hw_ref_count + 1;
                        }

                        f_st->gid  = f_ent->group->gid;
                    }

                    /* Get tcam index for entry */
                    pipe = f_ent->group->instance;
                    rv = _bcm_field_slice_offset_to_tcam_idx (unit, stage_fc,
                                                        pipe,
                                                        f_ent->fs->slice_number,
                                                        f_ent->slice_idx,
                                                        &pri_tcam_idx);
                    policy_tcam_idx = pri_tcam_idx;

                    if ((!(f_ent->group->flags
                         & _FP_GROUP_SPAN_SINGLE_SLICE)
                         || (f_ent->group->flags
                         & _FP_GROUP_INTRASLICE_DOUBLEWIDE))) {
                         policy_tcam_idx = pri_tcam_idx
                                     + (f_ent->fs->slice_number * 256);
                    }

                    BCM_IF_ERROR_CLEANUP(rv);

                    if (soc_feature(unit,
                                    soc_feature_fp_nw_tcam_prio_order_war)) {
                       rv = soc_mem_field_nw_tcam_prio_order_index_get(unit,
                                                        fp_policy_mem[pipe],
                                                          &policy_tcam_idx);
                       BCM_IF_ERROR_CLEANUP(rv);
                    }

                    /* Get memory for tcam index  */
                    e_buf = soc_mem_table_idx_to_pointer(
                                         unit,
                                         fp_policy_mem[pipe],
                                         uint32 *,
                                         fp_policy_buf[pipe],
                                         policy_tcam_idx);

                    if (f_ent->flags & _FP_ENTRY_INSTALLED) {
                       /* Actions recover for entry */
                        rv =  _field_actions_recover(unit, e_buf,
                                                     f_ent, &wb_entry);
                        BCM_IF_ERROR_CLEANUP(rv);
                        /* Update Slice hw entry count */
                        f_ent->fs->hw_ent_count +=1;

                    } else {
                        if (NULL != wb_entry.val_array) {
                            sal_free(wb_entry.val_array);
                        }
                        if (NULL != wb_entry.hw_idx_array) {
                            sal_free(wb_entry.hw_idx_array);
                        }
                    }
                    /* We free memory in actions recover for installed actions*/
                    wb_entry.val_array = NULL;
                    wb_entry.hw_idx_array = NULL;

                    rv = _field_entry_recover_policer_actual_hw_rates(unit, f_ent);
                    BCM_IF_ERROR_CLEANUP(rv);

                    /* Recover remaining entry parts */
                    if (parts_count > 1) {
                        for (j=1; j < parts_count; j++) {
                            sal_memcpy(f_ent+j, f_ent, sizeof(_field_entry_t));
                            (f_ent+j)->flags = wb_entry.flags[j];
                            (f_ent+j)->actions = NULL;
                             /* Given primary entry tcam index calculate entry
                                                            part tcam index. */
                            rv = _bcm_field_entry_part_tcam_idx_get(unit, f_ent,
                                  pri_tcam_idx,  j, &part_index);
                            BCM_IF_ERROR_CLEANUP(rv);
                            rv = _bcm_field_th_tcam_idx_to_slice_offset(unit,
                                        stage_fc, f_ent, part_index,
                                                  &slice_number,
                                                  (int *)&f_ent[j].slice_idx);
                            BCM_IF_ERROR_CLEANUP(rv);
                            f_ent[j].fs =
                                    stage_fc->slices[f_ent->group->instance]
                                                            + slice_number;

                            /* Update Slice hw entry count */
                            if (f_ent->flags & _FP_ENTRY_INSTALLED) {
                                f_ent[j].fs->hw_ent_count +=1;
                            }

                        }

                    }

                    /* Add to dynamic map */
                    dyf_ent->entry = f_ent;
                    dyf_ent->entry_id = f_ent->eid;
                    _DYNAMIC_MAP_INSERT(dynamic_map.entry_map, dyf_ent,
                                        dyf_ent_tail);
                    dyf_ent = NULL;
                    f_ent = NULL;
                 }
                 break;

            case _bcmFieldInternalGroupLtEntry:
                 entry_count = *(int *)tlv.value;
                 for (i = 0; i < entry_count; i++) {
                    _FP_XGS3_ALLOC(f_lt_ent,
                       parts_count*sizeof(_field_lt_entry_t), "field entry");
                    if (f_lt_ent == NULL) {
                        rv = BCM_E_MEMORY;
                        goto cleanup;
                    }
                    _FP_XGS3_ALLOC(dyf_lt_ent,
                        sizeof(_field_dynamic_map_entry_t), "temp ent");
                    if (dyf_lt_ent == NULL) {
                        rv = BCM_E_MEMORY;
                        goto cleanup;
                    }
                    sal_memset(flags, 0, sizeof(flags));

                    /* Recover entry */
                    rv =_field_lt_entry_info_recover(unit, f_lt_ent,
                                                     flags, new_fg);

                    /* Get tcam index for entry */
                    rv = _bcm_field_th_lt_slice_offset_to_tcam_index(unit,
                              stage_fc, f_lt_ent->group->instance,
                              f_lt_ent->lt_fs->slice_number,
                                  f_lt_ent->index, &pri_tcam_idx);
                    BCM_IF_ERROR_CLEANUP(rv);

                    /* Recover remaining entry parts */
                    if (parts_count > 1) {
                        for (j = 1; j < parts_count; j++) {
                            sal_memcpy(f_lt_ent+j, f_lt_ent,
                                        sizeof(_field_lt_entry_t));
                            (f_lt_ent+j)->flags = flags[j];

                            rv = _bcm_field_th_lt_entry_part_tcam_idx_get(unit,
                                            f_lt_ent->group, pri_tcam_idx,
                                                          j, &part_index);
                            BCM_IF_ERROR_CLEANUP(rv);
                            rv = _bcm_field_th_lt_tcam_idx_to_slice_offset(unit,
                                       stage_fc, f_lt_ent->group->instance,
                                         part_index,
                                         &slice_number,(int *)&f_lt_ent->index);
                            BCM_IF_ERROR_CLEANUP(rv);

                            f_lt_ent[j].lt_fs =
                                 stage_fc->lt_slices[f_lt_ent->group->instance]
                                                    + slice_number;

                        }
                    }

                    /* Recover tcam details */
                    for (j = 0; j < parts_count; j++) {
                         rv = _bcm_field_th_lt_tcam_entry_get(unit,
                                                              f_lt_ent->group,
                                                              f_lt_ent+j);
                         BCM_IF_ERROR_CLEANUP(rv);
                    }

                    /* Add to dynamic map */
                    dyf_lt_ent->lt_entry = f_lt_ent;
                    dyf_lt_ent->lt_entry_id = f_lt_ent->eid;
                    _DYNAMIC_MAP_INSERT(dynamic_map.lt_entry_map, dyf_lt_ent,
                                            dyf_lt_ent_tail);
                    dyf_lt_ent = NULL;
                    f_lt_ent = NULL;
                 }
                 break;

            case _bcmFieldInternalEndStructEntryDetails:
                 break;
            default:
                 break;
        }
    }

    TLV_INIT(tlv);
    return BCM_E_NONE;

cleanup:
    TLV_INIT(tlv);
    if (f_lt_ent != NULL) {
        sal_free(f_lt_ent);
        f_lt_ent = NULL;
    }
    if (dyf_lt_ent != NULL) {
        sal_free(dyf_lt_ent);
        dyf_lt_ent = NULL;
    }
    if (f_ent != NULL) {
        sal_free(f_ent);
        f_ent = NULL;
    }
    if (dyf_ent != NULL) {
        sal_free(dyf_ent);
        dyf_ent = NULL;
    }
    for (pipe = 0; pipe < num_pipes; pipe++) {
         if (fp_policy_buf[pipe] != NULL) {
             soc_cm_sfree(unit, fp_policy_buf[pipe]);
          }
    }
    for (pipe = 0; pipe < num_pipes; pipe++) {
         if (lt_policy_buf[pipe] != NULL) {
             soc_cm_sfree(unit, lt_policy_buf[pipe]);
          }
    }

    return rv;

}


/* Function:
 * _field_group_recover
 *
 * Purpose:
 *    recover _field_group_t structure
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * fg            - (IN) Pointer to device group structure
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_group_recover(int unit, _field_group_t *fg)
{
    _field_tlv_t tlv;                 /* Field TLV structure */
    uint32 *position = NULL;          /* Position in Scache */
    uint8 *scache_ptr = NULL;         /* Pointer to Scache */
    _field_control_t  *fc;            /* Field control structure.*/
    int mem_sz = 0;                   /* memeory to be allocated to entry_arr */
    _field_stage_t *stage_fc = NULL;  /* Field Stage structure */
    soc_profile_mem_t *keygen_profile;/* Key gen program profile memory */
    int part_count = 0;               /* Parts for Field entry in group */
    int i = 0;                        /* Local variable */
    SHR_BITDCL udf_map[_SHR_BITDCLSIZE(BCM_FIELD_USER_NUM_UDFS)] = {0};
                                      /* Map of UDF qualifiers */
    uint32 qual_count = -1;           /* Qualifier Count */
    int total_qualify_count = -1;  /* Qualifier Count including UDF EM */
    int idx = 0;                      /* Loop index variable*/
    bcm_field_qset_t qset_copy;       /* Qset For Group */
    SHR_BITDCL *group_qset = NULL;    /* Group Qset */
    int rv = 0;                       /* Return Variable */
    soc_profile_mem_t *action_profile;/* action profile memory */

    stage_fc = curr_stage_fc;
    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv.value = NULL;
    TLV_INIT(tlv);

    for (idx=0; idx < MAX_ACT_PROF_ENTRIES_PER_GROUP; idx++) {
        fg->action_profile_idx[idx] =  -1;
    }

    while (tlv.type != _bcmFieldInternalEndStructGroup) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_CLEANUP(tlv_read(unit, &tlv, scache_ptr, position));

        switch (tlv.type)
        {
            case _bcmFieldInternalGroupId:
                 fg->gid = *(bcm_field_group_t *)tlv.value;
                 break;
            case _bcmFieldInternalGroupPri:
                 fg->priority = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalQualifyCount:
                 qual_count = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalQsetW:
                 if (downgrade[unit] == TRUE) {
                     _FP_XGS3_ALLOC(group_qset,
                     (tlv.length * (recovery_type_map[unit][tlv.type].size)),
                      "Qset for Group");
                 } else {
                     _FP_XGS3_ALLOC(group_qset,
                     (_SHR_BITDCLSIZE(_bcmFieldQualifyCount)
                      * (recovery_type_map[unit][tlv.type].size)),
                      "Qset for Group");
                 }
                 if (group_qset == NULL) {
                     rv = BCM_E_MEMORY;
                     goto cleanup;
                 }
                 sal_memcpy(group_qset, tlv.value,
                 ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalQsetQual:
                 if (downgrade[unit] == TRUE) {
                     _FP_XGS3_ALLOC(group_qset,
                     (tlv.length * (recovery_type_map[unit][tlv.type].size)),
                      "Qset for Group");
                 } else {
                     _FP_XGS3_ALLOC(group_qset,
                     (_SHR_BITDCLSIZE(_bcmFieldQualifyCount)
                      * (recovery_type_map[unit][tlv.type].size)),
                      "Qset for Group");
                 }
                 if (group_qset == NULL) {
                     rv = BCM_E_MEMORY;
                     goto cleanup;
                 }
                 sal_memcpy(group_qset, tlv.value,
                 ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalQsetUdf:
                 sal_memcpy(udf_map, tlv.value,
                      ((recovery_type_map[unit][tlv.type].size) * tlv.length));

                 for (idx = 0; idx < ((recovery_type_map[unit][tlv.type].size)
                      * tlv.length); idx++) {
                      if (SHR_BITGET(udf_map, idx)) {
                          BCM_FIELD_QSET_ADD_INTERNAL(fg->qset,
                              bcmFieldQualifyCount + idx);
                       }
                 }
                 break;
            case _bcmFieldInternalQsetUdfMap:
                 sal_memcpy(fg->qset.udf_map, tlv.value,
                 ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalGroupFlags:
                 fg->flags = *(uint16 *)tlv.value;
                 break;
            case _bcmFieldInternalGroupPbmp:
                 sal_memcpy(&fg->pbmp, tlv.value,
                             ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalGroupSlice:
                 fg->slices =
                     &(stage_fc->slices[fg->instance][*(int *)tlv.value]);
                 break;
            case _bcmFieldInternalGroupQual:
                 BCM_IF_ERROR_CLEANUP(_field_groupqual_recover(unit,
                                                              &tlv, fg,
                                                              (fg->qual_arr),
                                                              qual_count));
                 break;
            case _bcmFieldInternalGroupPreselQual:
                 BCM_IF_ERROR_CLEANUP(_field_groupqual_recover(unit,
                                                        &tlv, fg,
                                                        (fg->presel_qual_arr),
                                                        qual_count));
                 break;
            case _bcmFieldInternalGroupStage:
                 fg->stage_id = *(_field_stage_id_t *)tlv.value;
                 break;
            case _bcmFieldInternalGroupEntry:
                 break;
            case _bcmFieldInternalGroupBlockCount:
                 fg->ent_block_count = *(uint16 *)tlv.value;
                 break;
            case _bcmFieldInternalGroupGroupStatus:
                 sal_memcpy(&(fg->group_status), tlv.value,
                      ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalGroupGroupAset:
                 if (downgrade[unit] == TRUE) {
                     sal_memcpy(&(fg->aset.w), tlv.value,
                       ((recovery_type_map[unit][tlv.type].size) *
                         _SHR_BITDCLSIZE(bcmFieldActionCount)));
                 } else {
                     sal_memcpy(&(fg->aset.w), tlv.value,
                      ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 }
                 break;
            case _bcmFieldInternalGroupCounterBmp:
                 sal_memcpy(fg->counter_pool_bmp.w, tlv.value,
                      ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalGroupInstance:
                 fg->instance = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalExtractor:
                 BCM_IF_ERROR_CLEANUP(_field_extractor_recover(unit,
                                    scache_ptr, position, &tlv, fg->ext_codes));
                 break;
            case _bcmFieldInternalGroupLtSlice:
                 fg->lt_slices =
                       &(stage_fc->lt_slices[fg->instance][*(int *)tlv.value]);
                 break;
            case _bcmFieldInternalGroupLtConfig:
                 fg->lt_id = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalGroupLtEntry:
                 /* backlink  function will handle */
                 break;
            case _bcmFieldInternalGroupLtEntrySize:
                 fg->lt_ent_blk_cnt = *(uint16 *)tlv.value;
                 break;
            case _bcmFieldInternalLtStatusEntriesFree:
                 fg->lt_grp_status.entries_free = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalLtStatusEntriesTotal:
                 fg->lt_grp_status.entries_total = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalLtStatusEntriesCnt:
                 fg->lt_grp_status.entry_count = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalGroupQsetSize:
                 fg->qset_size = *(int *)tlv.value;
                 break;
            case _bcmFieldInternalGroupHintId:
                 fg->hintid = *(bcm_field_hintid_t *)tlv.value;
                 break;
            case _bcmFieldInternalGroupMaxSize:
                 fg->max_group_size = *(uint32 *)tlv.value;
                 break;
            case _bcmFieldInternalEMGroupMode:
                 fg->em_mode = *(_field_em_mode_t *)tlv.value;
                 break;
            case _bcmFieldInternalEMGroupAset:
                 /*length value was not saved in this case. Hence added a new
                   enum */
                 if (downgrade[unit] == TRUE) {
                     sal_memcpy(&(fg->aset.w), tlv.value,
                        ((recovery_type_map[unit][tlv.type].size) *
                          _SHR_BITDCLSIZE(bcmFieldActionCount)));
                 } else {
                     sal_memcpy(&(fg->aset.w), tlv.value,
                       ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 }
                 break;
            case _bcmFieldInternalEMGroupAset2:
                 if (downgrade[unit] == TRUE) {
                     sal_memcpy(&(fg->aset.w), tlv.value,
                        ((recovery_type_map[unit][tlv.type].size) *
                          _SHR_BITDCLSIZE(bcmFieldActionCount)));
                 } else {
                     sal_memcpy(&(fg->aset.w), tlv.value,
                       ((recovery_type_map[unit][tlv.type].size) * tlv.length));
                 }
                 break;
            case _bcmFieldInternalGroupActionProfIdx:
                 sal_memcpy(fg->action_profile_idx, tlv.value,
                            ((static_type_map[tlv.type].size) * tlv.length));
                 break;
            case _bcmFieldInternalEndStructGroup:
                 if (*(uint32 *)tlv.value != _FIELD_WB_EM_GROUP) {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                        "END MARKER CHECK FAILED : GROUP\n")));
                     TLV_INIT(tlv);
                     rv = BCM_E_INTERNAL;
                     goto cleanup;
                 }
                 break;

            default:
                 break;
        }
    }

    fg->action_res_id = -1;

    /* Key gen program profile memory is set with correct reference count */
    keygen_profile = &stage_fc->keygen_profile[fg->instance].profile;
    /* Action profile memory is set with correct reference count */
    action_profile = &stage_fc->action_profile[fg->instance];
    if (fg->flags & _FP_GROUP_SPAN_TRIPLE_SLICE) {
        part_count = 3;
    } else if (fg->flags & _FP_GROUP_SPAN_DOUBLE_SLICE) {
        part_count = 2;
    } else {
        part_count = 1;
    }

    /* Allocate memory for entries in group */
    mem_sz = _FP_GROUP_ENTRY_ARR_BLOCK * (fg->ent_block_count) *
                        sizeof(_field_entry_t *);
    _FP_XGS3_ALLOC(fg->entry_arr, mem_sz, "entry block for fp group");
    if (fg->entry_arr == NULL) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }

    /* Clear copy of qset. */
    BCM_FIELD_QSET_INIT(qset_copy);

    /* Update Qualifier Ids for qualifiers after bcmFieldqualifyCount */

    if (qual_count != 0) {
        /* Downgrade case */
        if (qual_count > bcmFieldQualifyCount) {
            total_qualify_count = ((_bcmFieldQualifyCount) +
                                  (qual_count - bcmFieldQualifyCount));
        } else {
            total_qualify_count = ((_bcmFieldQualifyCount) -
                                  (bcmFieldQualifyCount - qual_count));
        }

        /* Cleanup new qualifiers which are not present in downgrade */
        if (downgrade[unit] == TRUE) {
            for (idx = bcmFieldQualifyCount; idx < qual_count; idx ++) {
                 SHR_BITCLR(group_qset, idx);
            }
        }

        for (idx = qual_count; idx < total_qualify_count; idx ++) {
            if (SHR_BITGET((group_qset), (idx))) {
                 SHR_BITCLR(group_qset, idx);
                 SHR_BITSET(qset_copy.w, ( (idx - qual_count)
                            + bcmFieldQualifyCount));
            }
        }

        for (idx = bcmFieldQualifyCount; idx < _bcmFieldQualifyCount;
             idx++) {
            if (BCM_FIELD_QSET_TEST(qset_copy, idx)) {
                SHR_BITSET(group_qset, idx);
            }
        }

        for (idx = 0; idx < _bcmFieldQualifyCount; idx++) {
            if (SHR_BITGET((group_qset), (idx))) {
                SHR_BITSET(fg->qset.w, idx);
            }
        }

        BCM_FIELD_QSET_INIT(qset_copy);
    }
 

    if (stage_fc->stage_id == _BCM_FIELD_STAGE_CLASS) {
        rv = BCM_E_NONE;
        goto cleanup;
    }

    for (i = 0; i < part_count; i++) {
         soc_profile_mem_reference(unit, keygen_profile,
                                   fg->ext_codes[i].keygen_index, 1);
    }
    if (stage_fc->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        for (i =0; i < MAX_ACT_PROF_ENTRIES_PER_GROUP; i++) {
            if (-1 != fg->action_profile_idx[i]) {
                SOC_PROFILE_MEM_REFERENCE(unit, action_profile,
                                         fg->action_profile_idx[i], 1);
                SOC_PROFILE_MEM_ENTRIES_PER_SET(unit, action_profile,
                                         fg->action_profile_idx[i], 1);
            }
        }
    }
    mem_sz = _FP_GROUP_ENTRY_ARR_BLOCK * (fg->lt_ent_blk_cnt + 1)
                    * sizeof(_field_lt_entry_t *);
    _FP_XGS3_ALLOC(fg->lt_entry_arr, mem_sz, "entry block for fp group");
    if (fg->lt_entry_arr == NULL) {
        rv = BCM_E_MEMORY;
    }

cleanup:
    TLV_INIT(tlv);
    if (group_qset != NULL) {
       sal_free(group_qset);
    }
    return rv;

}

/* Function:
 * _field_backlink_data
 *
 * Purpose:
 *    backlink entries and lt entries from slice and group structures.
 *
 * Paramaters:
 * unit          - (IN) BCM device number
 * stage_fc      - (IN) Pointer to stage structure.
 * group_count   - (IN) Number of groups present
 *
 * Description:
 *  Entries are linked with their groups and slices at the time of
 *  entry_info_recover, and are stored temporarily in dynamic_map
 *  structure. In this function, these entries are picked up and are
 *  added into group->entry_arr and slice->entries.
 *
 * Returns:
 *     BCM_E_XXX
 */

int
_field_backlink_data(int unit, _field_stage_t *stage_fc, int max_grp_id)
{
    _field_dynamic_map_entry_t *dyf_ent= NULL;
                                  /* pointer to entry in dynamic map */
    _field_dynamic_map_lt_entry_t *dyf_lt_ent = NULL;
                                  /* pointer to LT entry in dynamic map */
    _field_group_t *fg = NULL;    /* Field group structure */
    _field_control_t *fc = NULL;  /* pointer to field control structure */
    _field_entry_t *f_ent = NULL; /* Field Entry structure */
    _field_lt_entry_t *f_lt_ent = NULL;
                                  /* Field LT entry structure */
    int *group_pos = NULL;        /* Group Position */
    int parts_count = 0, part = 0;/* Parts in Field entry */
    int rv = BCM_E_NONE;          /* Return  Variable */
    bcm_field_group_t current_gid = -1;
                                  /* Group id where entry belongs */

    BCM_IF_ERROR_RETURN(_field_control_get(unit, &fc));

    dyf_ent = dynamic_map.entry_map;
    if (dyf_ent != NULL) {
        current_gid = dyf_ent->entry->group->gid;
    }
    fg = fc->groups;

    /* Group id's range from 0 to max_grp_id */
    _FP_XGS3_ALLOC(group_pos, (max_grp_id + 1) * sizeof(int),
                   "group count array");
    if (group_pos == NULL) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }

    while (dyf_ent != NULL)
    {
        /* Make sure that the current entry belongs to current group */
        if (fg->gid != current_gid) {
            fg = fc->groups;

            /* Find group where entry belongs */
            while (fg != NULL) {
                fg = fg->next;
                if (fg->gid == current_gid) {
                    break;
                }
            }
            if (fg == NULL) {
                 LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                 "entry backlink failed\n")));
                rv = BCM_E_INTERNAL;
                goto cleanup;
            }
        }

        BCM_IF_ERROR_CLEANUP(_bcm_field_entry_tcam_parts_count (unit,
                            fg->stage_id, fg->flags, &parts_count));
        /* Link group to entry */
        fg->entry_arr[group_pos[current_gid]] = dyf_ent->entry;
        group_pos[current_gid]++;
        f_ent = dyf_ent->entry;
        for (part = 0; part < parts_count; part ++)
        {
            f_ent[part].fs->entries[f_ent[part].slice_idx] = f_ent + part;
        }

        dyf_ent = dyf_ent->next;
        if (dyf_ent != NULL) {
            current_gid = dyf_ent->entry->group->gid;
            parts_count = 0;
        }
    }

    dyf_lt_ent = dynamic_map.lt_entry_map;
    if (dyf_lt_ent != NULL) {
        current_gid = dyf_lt_ent->lt_entry->group->gid;
    }
    fg = fc->groups;
    sal_memset(group_pos, 0,  (max_grp_id  + 1) * sizeof(int));

    while (dyf_lt_ent != NULL) {

        /* Make sure that the current entry belongs to current group */
        if (fg->gid != current_gid) {
            fg = fc->groups;

            /* Find group where entry belongs */
            while (fg != NULL) {
                fg = fg->next;
                if (fg->gid == current_gid) {
                    break;
                }
            }
            if (fg == NULL) {
                 LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                                 "lt entry backlink failed\n")));
                rv = BCM_E_INTERNAL;
                goto cleanup;
            }
        }
        BCM_IF_ERROR_RETURN(_bcm_field_entry_tcam_parts_count (unit,
                            fg->stage_id, fg->flags, &parts_count));

        /* Link group to LT entry */
        fg->lt_entry_arr[group_pos[current_gid]] = dyf_lt_ent->lt_entry;
        group_pos[current_gid]++;
        f_lt_ent = dyf_lt_ent->lt_entry;
        for (part = 0; part < parts_count; part ++)
        {
             f_lt_ent[part].lt_fs->entries[f_lt_ent[part].index] = f_lt_ent;
        }

        dyf_lt_ent = dyf_lt_ent->next;
        if (dyf_lt_ent != NULL) {
            current_gid = dyf_lt_ent->lt_entry->group->gid;
            parts_count = 0;
        }
    }

cleanup:
    if (group_pos != NULL) {
        sal_free(group_pos);
        group_pos = NULL;
    }
    return rv;

}
/* Function:
 * _field_class_info_recover
 *
 * Purpose:
 *   Recover function for _field_class_info_t structure
 *
 * Parameters:
 * unit      - (IN) unit number
 * class_info_arr  - (IN) pointer to array of pointers to structures
 * tlv     - (IN) tlv structure holding the number of types and count of array
 * Returns:
 *      BCM_E_XXX
 */
int
_field_class_info_recover(int unit, _field_class_info_t **class_info_arr,
                          _field_tlv_t *tlv)
{
    int type_pos = 0;                     /* position of type */
    int i = 0;                            /* local variable for loops */
    int num_instances = 0;                /* number of instances */
    int num_types = 0;                    /* Number of types */
    _field_tlv_t tlv2;                    /* TLV structure */
    uint32 *type;                         /* Elem type */
    uint8 *scache_ptr;                    /* Scache pointer */
    uint32 *position;                     /* Scache position */
    _field_control_t        *fc;          /* Field control structure.  */

    WB_FIELD_CONTROL_GET(fc, scache_ptr, position);
    tlv2.value = NULL;
    TLV_INIT(tlv2);
    type = (uint32 *)tlv->value;
    num_instances = (tlv->length) & _FP_WB_TLV_LENGTH_MASK;
    num_types     = ((tlv->length & _FP_WB_TLV_TYPES_MASK) >>
                    _FP_WB_TLV_TYPE_SHIFT);

    if (class_info_arr == NULL) {
        return BCM_E_INTERNAL;
    }

    for (i=0; i < num_instances; i++) {
        type_pos = 0;
        while  ((tlv2.type != _bcmFieldInternalEndStructClass)
                 && (type_pos != num_types)) {
            TLV_INIT(tlv2);
            tlv2.type = (type[type_pos] & _FP_WB_TLV_ELEM_TYPE_MASK);
            tlv2.basic_type = ((type[type_pos] & _FP_WB_TLV_BASIC_TYPE_MASK) >>
                              _FP_WB_TLV_BASIC_TYPE_SHIFT);
            BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv2, scache_ptr, position));

            switch(tlv2.type) {
                case _bcmFieldInternalClassFlags:
                     class_info_arr[i]->flags = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalClassEntUsed:
                     class_info_arr[i]->total_entries_used
                                              = *(uint32 *)tlv2.value;
                     break;
                case _bcmFieldInternalClassBmp:
                     sal_memcpy(class_info_arr[i]->class_bmp.w,
                                              tlv2.value, tlv2.length);
                     break;
                case _bcmFieldInternalEndStructClass:
                     break;
                default:
                     break;
             }
             type_pos++;
        }
        TLV_INIT(tlv2);
    }
    return BCM_E_NONE;
}
int
_bcm_field_th_stage_class_reinit(int unit, _field_control_t *fc,
        _field_stage_t *stage_fc)
{
    _field_tlv_t tlv;              /* Field TLV structure */
    _field_entry_t *f_ent = NULL;  /* pointer to entry structure */
    uint8 *ptr = NULL;             /* pointer to scache_ptr in fc */
    uint32 *pos = NULL;            /* pointer to position in fc */
    int *group_pos = NULL;         /* Pointer to hold array of groups */
     /* Used  to build a linked list of groups in stage */
    _field_group_t *curr_fg = NULL, *prev_fg = NULL;
    /* pointer to array of group pointers */
    _field_group_t **new_fg = NULL, *fg = NULL;
    int group_count = 0, rv = 0;   /* Number of groups, return value */
    int idx = 0, entry_count;      /* loop variable, no. of entries in group */
    int pipe = 0;                  /* loop var,perpipe mode or global */
    int group_id = 0, max_grp_id = 0; /*  maximum id of group in this stage */
    _field_class_type_t ctype;
    int tcamsz = 0;
    _field_wb_entry_t wb_entry;    /* Set of WB entry info */


    tlv.value = NULL;
    TLV_INIT(tlv);
    if ((fc == NULL) || (stage_fc == NULL)) {
        return BCM_E_PARAM;
    }

    wb_entry.action_count = -1;
    wb_entry.action_param_type = -1;
    wb_entry.hw_idx_array = NULL;
    wb_entry.val_array = NULL;

    WB_FIELD_CONTROL_GET_WITH_LOCK(fc, ptr, pos);
    BCM_IF_ERROR_CLEANUP(_field_scache_stage_hdr_chk(fc,
                        _FIELD_CLASS_DATA_START));
    while (tlv.type != _bcmFieldInternalEndStageClass) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_CLEANUP(tlv_read(unit, &tlv, ptr, pos));

        switch (tlv.type) {

        case _bcmFieldInternalClassOperMode:
             if (*(bcm_field_group_oper_mode_t *)tlv.value) {
                 BCM_IF_ERROR_CLEANUP(_bcm_field_wb_group_oper_mode_set(unit,
                                    bcmFieldQualifyStageClass,
                                    &stage_fc,
                                    bcmFieldGroupOperModePipeLocal));
                 stage_fc = fc->stages;
                 while (stage_fc->stage_id != _BCM_FIELD_STAGE_CLASS) {
                    stage_fc = stage_fc->next;
                 }
             }
             curr_stage_fc = stage_fc;
             break;
        case _bcmFieldInternalStageClass:
             BCM_IF_ERROR_CLEANUP(
                    _field_class_info_recover(unit,
                        stage_fc->class_info_arr[pipe], &tlv));
             pipe++;
             break;
        case _bcmFieldInternalGroupCount:
             group_count = *(int *)tlv.value;
             for (group_id =0 ;group_id < group_count; group_id ++) {
                 curr_fg = NULL;
                 _FP_XGS3_ALLOC(curr_fg, sizeof(_field_group_t),
                                "current field grp");
                 if (curr_fg == NULL) {
                     rv = BCM_E_MEMORY;
                     goto cleanup;
                 }
                 rv = _field_group_recover(unit, curr_fg);
                 BCM_IF_ERROR_CLEANUP(rv);

                 if (fc->groups == NULL) {
                     prev_fg = curr_fg;
                     fc->groups = curr_fg;
                     max_grp_id = curr_fg->gid;
                     continue;
                 }

                 if (prev_fg == NULL) {
                     prev_fg = fc->groups;
                     while (prev_fg->next != NULL) {
                         prev_fg = prev_fg->next;
                     }
                     prev_fg->next = curr_fg;
                     prev_fg = prev_fg->next;
                     max_grp_id = curr_fg->gid;
                 } else {
                     prev_fg->next = curr_fg;
                     prev_fg = curr_fg;
                     max_grp_id =
                         ((max_grp_id > (curr_fg->gid))
                          ? max_grp_id:(curr_fg->gid));
                 }
             }
             _FP_XGS3_ALLOC(new_fg, (max_grp_id+1) *sizeof(_field_group_t *),
                            "group alloc");
             if (new_fg == NULL) {
                 rv = BCM_E_MEMORY;
                 goto cleanup;
             }

             _FP_XGS3_ALLOC(group_pos, (max_grp_id + 1) * sizeof(int),
                            "grp pos");
             if (group_pos == NULL) {
                 rv = BCM_E_MEMORY;
                 goto cleanup;
             }

             curr_fg = fc->groups;
             while (curr_fg != NULL) {
                 if (curr_fg->stage_id == _BCM_FIELD_STAGE_CLASS) {
                    new_fg[(curr_fg->gid)] = curr_fg;
                 }
                 curr_fg = curr_fg->next;
             }

             break;
        case _bcmFieldInternalGroupEntry:
             entry_count = *(int *)tlv.value;
             for (idx = 0; idx < entry_count; idx++) {
                _FP_XGS3_ALLOC(f_ent, sizeof(_field_entry_t), "field entry");
                /* coverity[forward_null : FALSE] */
                /* coverity[var_deref_model] */
                if (f_ent == NULL) {
                    rv = BCM_E_MEMORY;
                    goto cleanup;
                }
                sal_memset(wb_entry.flags, 0, sizeof(wb_entry.flags));

                _field_entry_info_recover(unit, f_ent,
                                          1, new_fg,
                                          &wb_entry);
               /* Actions recover for entry */
               if (f_ent->flags & _FP_ENTRY_INSTALLED) {
                    rv = _field_actions_recover(unit, NULL,
                                                  f_ent, &wb_entry);
               } else {
                    rv = _bcm_field_th_class_type_qset_get(unit,
                                                 &f_ent->group->qset, &ctype);
                    BCM_IF_ERROR_CLEANUP(rv);
                    rv = _bcm_field_th_class_size_get(unit, ctype, &tcamsz);
                    _FP_XGS3_ALLOC(f_ent->tcam.key,
                                   tcamsz, "field class data buf");
                    if (f_ent->tcam.key == NULL) {
                        rv = BCM_E_MEMORY;
                        goto cleanup;
                    }
                    f_ent->tcam.key_size = tcamsz;

               }
               BCM_IF_ERROR_CLEANUP(rv);


                group_id = f_ent->group->gid;
                fg = new_fg[group_id];
                fg->entry_arr[group_pos[group_id]] = f_ent;
                group_pos[group_id]++;
                f_ent = NULL;

             }
             break;


        case _bcmFieldInternalEndStageClass:
             if (*(uint32 *)tlv.value != _FIELD_WB_EM_CLASS) {
                 rv = BCM_E_INTERNAL;
                 goto cleanup;
             }
             break;

        default:
             break;
        }
    }

    BCM_IF_ERROR_CLEANUP(_field_scache_stage_hdr_chk(fc, _FIELD_CLASS_DATA_END));
cleanup:
    TLV_INIT(tlv);

    if (curr_fg != NULL) {
        sal_free(curr_fg);
        curr_fg = NULL;
    }
    if (new_fg != NULL) {
        sal_free(new_fg);
        new_fg = NULL;
    }
    if (group_pos != NULL) {
        sal_free(group_pos);
        group_pos = NULL;
    }
    if (f_ent != NULL) {
        sal_free(f_ent);
        f_ent = NULL;
    }
    FP_UNLOCK(unit);
    return rv;
}
/*
 * Function:
 *      _field_qos_actions_recover
 * Purpose:
 *      Recover actions that belong to IFP_PROFILE_SET1 and SET2.
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      ebuf  - (IN) qos profile table entry
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_qos_actions_recover(int unit, _field_entry_t *f_ent, uint32 *ebuf)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL;
    bcm_field_action_t action_id;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    int supported = 0;

    for (action_id = 0; action_id < bcmFieldActionCount; action_id++) {
        valid  = 0;
        _bcm_field_th_action_support_check(unit, f_ent, action_id, &supported);
        if (!supported) {
            continue;
        }
        ACTION_GET(unit, f_ent, ebuf, action_id, 0, &a_offset);
        switch (action_id)  {

           case bcmFieldActionGpPrioIntCopy:
           case bcmFieldActionYpPrioIntCopy:
           case bcmFieldActionRpPrioIntCopy:
                if (a_offset.value[0] == 4) {
                    valid = 1;
                }
                break;
 /*        case bcmFieldActionPrioIntCopy:
           case bcmFieldActionPrioIntTos: */
           case bcmFieldActionGpPrioIntTos:
           case bcmFieldActionYpPrioIntTos:
           case bcmFieldActionRpPrioIntTos:
                if (a_offset.value[0] == 6) {
                    valid = 1;
                }
                break;
/*         case bcmFieldActionPrioIntCancel: */
           case bcmFieldActionGpPrioIntCancel:
           case bcmFieldActionYpPrioIntCancel:
           case bcmFieldActionRpPrioIntCancel:
                if (a_offset.value[0] == 7) {
                    valid = 1;
                }
                break;
           case bcmFieldActionGpPrioIntNew:
           case bcmFieldActionYpPrioIntNew:
           case bcmFieldActionRpPrioIntNew:
                if (a_offset.value[1] == 5) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionGpDropPrecedence:
           case bcmFieldActionYpDropPrecedence:
           case bcmFieldActionRpDropPrecedence:
                if (a_offset.value[0]  != 0) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionGpEcnNew:
           case bcmFieldActionYpEcnNew:
           case bcmFieldActionRpEcnNew:
                if (a_offset.value[1] == 1) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionGpPrioPktNew:
           case bcmFieldActionYpPrioPktNew:
           case bcmFieldActionRpPrioPktNew:
                if (a_offset.value[1] == 5) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionRpDscpNew:
           case bcmFieldActionYpDscpNew:
           case bcmFieldActionGpTosPrecedenceNew:
                if (a_offset.value[1] == 1) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionGpDscpNew:
                if (a_offset.value[1] == 3) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionGpPrioPktCopy:
           case bcmFieldActionYpPrioPktCopy:
           case bcmFieldActionRpPrioPktCopy:
                if (a_offset.value[0] == 4) {
                    valid = 1;
                }
                break;
           case bcmFieldActionGpPrioPktTos:
           case bcmFieldActionYpPrioPktTos:
           case bcmFieldActionRpPrioPktTos:
                if (a_offset.value[0] == 6) {
                    valid =  1;
               }
               break;
           case bcmFieldActionGpPrioPktCancel:
           case bcmFieldActionYpPrioPktCancel:
           case bcmFieldActionRpPrioPktCancel:
                if (a_offset.value[0] == 7) {
                    valid = 1;
                }
                break;
           case bcmFieldActionGpDscpCancel:
           case bcmFieldActionYpDscpCancel:
           case bcmFieldActionRpDscpCancel:
                if (a_offset.value[0] == 4) {
                    valid = 1;
                }
                break;
           case bcmFieldActionGpTosPrecedenceCopy:
                if (a_offset.value[0] == 2) {
                    valid = 1;
                }
                break;
           case bcmFieldActionGpDot1pPreserve:
           case bcmFieldActionYpDot1pPreserve:
           case bcmFieldActionRpDot1pPreserve:
               if (a_offset.value[0] == 3) {
                   valid = 1;
               }
               break;
           case bcmFieldActionGpDscpPreserve:
               if (a_offset.value[0] == 5) {
                   valid = 1;
               }
               break;
           case bcmFieldActionYpDscpPreserve:
           case bcmFieldActionRpDscpPreserve:
               if (a_offset.value[0] == 3) {
                   valid = 1;
               }
               break;
           case bcmFieldActionGpIntCongestionNotificationNew:
           case bcmFieldActionYpIntCongestionNotificationNew:
           case bcmFieldActionRpIntCongestionNotificationNew:
               if (a_offset.value[1] == 1) {
                   valid = 1;
               }
               break;
           case bcmFieldActionPfcClassNew:
               if (a_offset.value[1] == 1) {
                   valid = 1;
               }
               break;
           case bcmFieldActionGpCosMapNew:
           case bcmFieldActionYpCosMapNew:
           case bcmFieldActionRpCosMapNew:
               if (a_offset.value[1] == 2) {
                   valid = 1;
               }
               break;
           case bcmFieldActionGpCosQNew:
           case bcmFieldActionYpCosQNew:
           case bcmFieldActionRpCosQNew:
                if (a_offset.value[1] == 1) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionGpUcastCosQNew:
           case bcmFieldActionYpUcastCosQNew:
           case bcmFieldActionRpUcastCosQNew:
                if (a_offset.value[1] == 8) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionGpMcastCosQNew:
           case bcmFieldActionYpMcastCosQNew:
           case bcmFieldActionRpMcastCosQNew:
                if (a_offset.value[1] == 9) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           case bcmFieldActionUntaggedPacketPriorityNew:
                if (a_offset.value[1] == 1) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
           default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_id;
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            f_act->next = f_ent->actions;
            f_ent->actions = f_act;
        }
    }
    return BCM_E_NONE;
}
static char *qos_profile_buf[_FP_MAX_NUM_PIPES] = {0};
int
_field_wb_qos_buffer_free(int unit) {
    _field_stage_t *stage_fc = NULL;
    int pipe = 0, num_pipes = 0;

    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                _BCM_FIELD_STAGE_EXACTMATCH, &stage_fc));

    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        num_pipes = 1;
    } else {
        num_pipes = stage_fc->num_pipes;
    }
    for (pipe = 0; pipe < num_pipes; pipe++) {
        if (qos_profile_buf[pipe]) {
            soc_cm_sfree(unit, qos_profile_buf[pipe]);
            qos_profile_buf[pipe] = NULL;
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_qos_actions_recover
 * Purpose:
 *      Recover all QOS Related actions for a particular entry.
 *      This profile may be pointed to, by many entries. so, we
 *      DMA the entire table and store the pointers in statically
 *      declared buffer. Corresponding free function is called
 *      at the end of em reinit.
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this qos profile.
 *      qos_idx  - (IN) qos index (obtained from QOS_PROFILE_IDf from
 *                      exact match tables)
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_qos_actions_recover(int unit, _field_entry_t *f_ent, int qos_idx)
{
    soc_mem_t qos_prof_mems[_FP_MAX_NUM_PIPES+1] =
            { EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE0m,
              EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE1m,
              EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE2m,
              EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE3m,
              EXACT_MATCH_QOS_ACTIONS_PROFILEm
            };
    static  soc_mem_t *mem;
    int pipe = 0, num_pipes = 0;
    _field_stage_t *stage_fc = NULL;
    uint32 *e_buf = NULL;
    int index_min = 0, index_max = 0;
    int rv = 0;
    soc_profile_mem_t *qos_act_prof = NULL;
    int inst;


    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                               _BCM_FIELD_STAGE_EXACTMATCH, &stage_fc));
    inst = f_ent->group->instance;
    qos_act_prof =  &stage_fc->qos_action_profile[inst];


    if (qos_profile_buf[0] == NULL) {

        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            num_pipes = 1;
            mem = &qos_prof_mems[_FP_MAX_NUM_PIPES];
        } else {
            num_pipes = stage_fc->num_pipes;
            mem = &qos_prof_mems[0];
        }
        for (pipe = 0; pipe < num_pipes; pipe++) {
            qos_profile_buf[pipe] = soc_cm_salloc(unit,
                             SOC_MEM_TABLE_BYTES(unit, qos_prof_mems[pipe]),
                                          "QOS Profile table buffer");

            index_min = soc_mem_index_min(unit, mem[pipe]);
            index_max = soc_mem_index_max(unit, mem[pipe]);
            rv = soc_mem_read_range(unit, mem[pipe], MEM_BLOCK_ALL,
                                  index_min, index_max, qos_profile_buf[pipe]);
            BCM_IF_ERROR_RETURN(rv);

        }
    }
    pipe = f_ent->group->instance;

    e_buf = soc_mem_table_idx_to_pointer(unit, qos_prof_mems[pipe],
                                  uint32 *, qos_profile_buf[pipe], qos_idx);
    _field_qos_actions_recover(unit, f_ent, e_buf);
    SOC_PROFILE_MEM_REFERENCE(unit, qos_act_prof, qos_idx, 1);
    SOC_PROFILE_MEM_ENTRIES_PER_SET(unit, qos_act_prof, qos_idx, 1);

    return BCM_E_NONE;


}
static char *act_profile_buf[_FP_MAX_NUM_PIPES] = {0};
int
_field_wb_em_act_buffer_free(int unit) {
    _field_stage_t *stage_fc = NULL;
    int pipe = 0, num_pipes = 0;

    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                _BCM_FIELD_STAGE_EXACTMATCH, &stage_fc));

    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        num_pipes = 1;
    } else {
        num_pipes = stage_fc->num_pipes;
    }
    for (pipe = 0; pipe < num_pipes; pipe++) {
        if (act_profile_buf[pipe]) {
            soc_cm_sfree(unit, act_profile_buf[pipe]);
            act_profile_buf[pipe] = NULL;
        }
    }
    return BCM_E_NONE;
}
#define ACTION_DATA_SHIFT(act_data, shift)                      \
    umask = ((1 << shift) - 1);                                 \
    act_data[0] = (act_data[0] >> shift);                       \
    act_data[0] |= ((act_data[1] & umask) << (32-shift));       \
    act_data[1] = (act_data[1] >> shift);                       \
/*
 * Function:
 *      _field_wb_meter_set_recover
 * Purpose:
 *      Recover actions that belong to meter set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_meter_set_recover(int unit, _field_entry_t *f_ent,
                               uint32 *act_data, _field_action_bmp_t *act_bmp) {
    /* check here */
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_green_to_pid_set_recover
 * Purpose:
 *      Recover actions that belong to green to pid set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_green_to_pid_set_recover(int unit, _field_entry_t *f_ent,
                                      uint32 *act_data, _field_action_bmp_t *act_bmp) {
     /* Setting this action only adjusts flags in f_ent, as entry flags are
         recovered from software nothing needs to be done */

    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_mirror_override_set_recover
 * Purpose:
 *      Recover actions that belong to meter override set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_mirror_override_set_recover(int unit, _field_entry_t *f_ent,
                                         uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionMirrorOverride,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }


    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionMirrorOverride:
                valid = 1;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;

}
/*
 * Function:
 *      _field_wb_nat_override_set_recover
 * Purpose:
 *      Recover actions that belong to nat override set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_nat_override_set_recover(int unit, _field_entry_t *f_ent,
                                      uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionNatCancel,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionNatCancel:
                if (a_offset.value[0] == 1) {
                    valid = 1;
                }
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_sflow_set_recover
 * Purpose:
 *      Recover actions that belong to sflow set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_sflow_set_recover(int unit,_field_entry_t *f_ent, uint32 *ebuf,
                                  _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionIngSampleEnable,
                                      };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionIngSampleEnable:
                if (a_offset.value[0] == 1) {
                    valid = 1;
                }
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_cut_thru_override_set_recover
 * Purpose:
 *      Recover actions that belong to cut thru override set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_cut_thru_override_set_recover(int unit, _field_entry_t *f_ent,
                               uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionDoNotCutThrough,
                                      };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionDoNotCutThrough:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_urpf_override_set_recover
 * Purpose:
 *      Recover actions that belong to urpf override set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_urpf_override_set_recover(int unit, _field_entry_t *f_ent,
                                uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionDoNotCheckUrpf,
                                      };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionDoNotCheckUrpf:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_ttl_override_set_recover
 * Purpose:
 *      Recover actions that belong to ttl override set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_ttl_override_set_recover(int unit, _field_entry_t *f_ent,
                             uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionDoNotChangeTtl,
                                      };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionDoNotChangeTtl:
                if (a_offset.value[0] == 1) {
                    valid = 1;
                }
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_lb_controls_set_recover
 * Purpose:
 *      Recover actions that belong to loopback controls set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_lb_controls_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] =
                    {
                         bcmFieldActionHgTrunkRandomRoundRobinHashCancel,
                         bcmFieldActionTrunkRandomRoundRobinHashCancel,
                         bcmFieldActionEcmpRandomRoundRobinHashCancel
                    };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }


    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionHgTrunkRandomRoundRobinHashCancel:
                if (a_offset.value[0] == 1) {
                    valid = 1;
                }
                break;
            case bcmFieldActionTrunkRandomRoundRobinHashCancel:
                if (a_offset.value[0] == 1) {
                    valid = 1;
                }
                break;
            case bcmFieldActionEcmpRandomRoundRobinHashCancel:
                if (a_offset.value[0] == 1) {
                    valid = 1;
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_drop_set_recover
 * Purpose:
 *      Recover actions that belong to drop set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_drop_set_recover(int unit, _field_entry_t *f_ent, uint32 *ebuf,
                                            _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    _field_stage_t *stage_fc = NULL;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionGpDrop,
                                        bcmFieldActionGpDropCancel,
                                        bcmFieldActionDrop,
                                        bcmFieldActionDropCancel,
                                        bcmFieldActionYpDrop,
                                        bcmFieldActionYpDropCancel,
                                        bcmFieldActionRpDrop,
                                        bcmFieldActionRpDropCancel,
                                      };

    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,f_ent->fs->stage_id, &stage_fc));

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }


    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionGpDrop:
            case bcmFieldActionYpDrop:
            case bcmFieldActionRpDrop:
                if (a_offset.value[0] == 1) {
                    valid = 1;
                }
                break;
            case bcmFieldActionDrop:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS &&
                    a_offset.value[0] == 1) {
                    valid = 1;
                }
                break;
            case bcmFieldActionGpDropCancel:
            case bcmFieldActionYpDropCancel:
            case bcmFieldActionRpDropCancel:
                if (a_offset.value[0] == 2) {
                    valid = 1;
                }
                break;
            case bcmFieldActionDropCancel:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS &&
                    a_offset.value[0] == 2) {
                    valid = 1;
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_change_cpu_cos_set_recover
 * Purpose:
 *      Recover actions that belong to cpu cos set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_change_cpu_cos_set_recover(int unit, _field_entry_t *f_ent,
                                  uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionCosQCpuNew,
                                        bcmFieldActionServicePoolIdNew,
                                        bcmFieldActionServicePoolIdPrecedenceNew
                                      };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionCosQCpuNew:
                if (a_offset.value[1] == 1) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
            case bcmFieldActionServicePoolIdNew:
                if ((a_offset.value[1] == 2) && (a_offset.value[2] == 2)){
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
            case bcmFieldActionServicePoolIdPrecedenceNew:
                if ((a_offset.value[2] == 3) && (a_offset.value[3] == 2)){
                    valid = 1;
                    param[0] = a_offset.value[0];
                    /* SPAP is 2b'00 for BCM_FIELD_COLOR_GREEN
                               2b'01 for BCM_FIELD_COLOR_RED
                               2b'11 for BCM_FIELD_COLOR_YELLOW
                    */
                    param[1] = ((0 == a_offset.value[1]) ? BCM_FIELD_COLOR_GREEN : \
                                 ((3 == a_offset.value[1])? BCM_FIELD_COLOR_YELLOW : BCM_FIELD_COLOR_RED));
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                return BCM_E_MEMORY;
            }
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;

}
/*
 * Function:
 *      _field_wb_nat_set_recover
 * Purpose:
 *      Recover actions that belong to nat set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_nat_set_recover(int unit, _field_entry_t *f_ent,
                           uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL;
    _field_action_t *fa  = NULL;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionNat,
                                        bcmFieldActionNatEgressOverride,
                                      };
    ACTION_GET(unit, f_ent, ebuf, action_arr[0], 0, &a_offset);
    if (a_offset.value[0] == 0) {
        return BCM_E_NONE;
    }
    _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions nat");
    if (f_act == NULL) {
        return BCM_E_MEMORY;
    }
    if (f_ent->actions == NULL) {
        f_ent->actions = f_act;
     } else {
         fa = f_ent->actions;
         while (fa->next != NULL) {
             fa = fa->next;
         }
         fa->next = f_act;
    }
    f_act->action = action_arr[0];
    f_act->param[0] = 0;
    f_act->old_index = -1;
    f_act->hw_index = -1;
    f_act->flags |= _FP_ACTION_VALID;
    fa = NULL;
    _FP_XGS3_ALLOC(fa, sizeof(_field_action_t), "FP em actions nat");
    if (fa == NULL) {
        return BCM_E_MEMORY;
    }
    f_act->next = fa;
    f_act = f_act->next;
    ACTION_GET(unit, f_ent, ebuf, action_arr[1], 0, &a_offset);
    f_act->action = action_arr[1];
    f_act->param[0] = (a_offset.value[0] << 1) | (a_offset.value[1]);
    f_act->old_index = -1;
    f_act->hw_index = -1;
    f_act->flags |= _FP_ACTION_VALID;

    return BCM_E_NONE;
}
int
_field_mirror_actions_recover_callback(int unit,_bcm_mirror_config_p _bcm_mirror_config)
{
    int idx = 0;
    _bcm_field_mirror_actions_recover_t *mirror_cb = NULL;
    _bcm_mirror_dest_config_p  mdest;/* Mirror destination description. */
    _field_action_t *f_act = NULL;

    if (NULL == em_mirror_actions) {
        return BCM_E_NONE;
    }
    mirror_cb = em_mirror_actions;
    while (NULL != mirror_cb) {
        for (idx = 0; idx < _bcm_mirror_config->dest_count; idx++) {
            mdest = _bcm_mirror_config->dest_arr + idx;
            /* Skip unused entries. */
            if (0 == mdest->ref_count) {
                continue;
            }

            if (mdest->mirror_dest.gport == mirror_cb->mtp_gport) {
                /* Matching mirror destination found. */
                for (f_act = mirror_cb->f_ent->actions; NULL!= f_act; f_act = f_act->next) {
                    if ((mirror_cb->action == f_act->action) &&
                            (f_act->param[1] == mirror_cb->mtp_gport)) {
                        f_act->param[1]= mdest->mirror_dest.mirror_dest_id;
                        break;
                    }
                }
            }
        }
        em_mirror_actions = mirror_cb->next;
        sal_free(mirror_cb);
        mirror_cb = em_mirror_actions;
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_mirror_set_recover
 * Purpose:
 *      Recover actions that belong to mirror set
 *      For any entry, 4 Mirror actions can be attached,
 *      Ingress and egress together. Mirror_select register
 *      indicates whether the actions or ingress or egress
 *      corresponding bit  -> 0 - ingress 1 - egress
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_mirror_set_recover(int unit, _field_entry_t *f_ent,
                             uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
   _bcm_field_mirror_actions_recover_t *mirror_recover = NULL;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx = 0;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    uint32 reg_value = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionMirrorIngress,
                                        bcmFieldActionMirrorIngress,
                                        bcmFieldActionMirrorIngress,
                                        bcmFieldActionMirrorIngress,
                                        bcmFieldActionMirrorEgress,
                                        bcmFieldActionMirrorEgress,
                                        bcmFieldActionMirrorEgress,
                                        bcmFieldActionMirrorEgress
                                      };


    BCM_IF_ERROR_RETURN(READ_MIRROR_SELECTr(unit, &reg_value));
    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    ACTION_GET(unit, f_ent, ebuf, action_arr[0], 0, &a_offset);

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        if (action_idx == 4) {
            ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        }
        switch (action_arr[action_idx]) {
            case bcmFieldActionMirrorIngress:
                if ((a_offset.value[0] & 0x1) && !(reg_value & 0x1)) {
                    hw_index = a_offset.value[1] & 0x3;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                                BCM_MIRROR_PORT_INGRESS, &param[0], &param[1]);
                     a_offset.value[0] &= 0xfffffffe;
                     valid = 1;
                     break;

                }
                if ((a_offset.value[0] & 0x2) && !(reg_value & 0x2)) {
                    hw_index  = (a_offset.value[1] & 0xc) >> 2;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                            BCM_MIRROR_PORT_INGRESS, &param[0], &param[1]);
                    a_offset.value[0] &= 0xfffffffc;
                    valid = 1;
                    break;
                }
                if ((a_offset.value[0] & 0x4) && !(reg_value & 0x4)) {
                    hw_index  = (a_offset.value[1] & 0x30) >> 4;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                                    BCM_MIRROR_PORT_INGRESS, &param[0], &param[1]);
                    a_offset.value[0] &= 0xfffffff8;
                    valid = 1;
                    break;
                }
                if ((a_offset.value[0] & 0x8) && !(reg_value & 0x8)) {
                    hw_index  = (a_offset.value[1] & 0xc0) >> 6;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                                    BCM_MIRROR_PORT_INGRESS, &param[0], &param[1]);
                     a_offset.value[0] &= 0xfffffff0;
                     valid = 1;
                     break;
                }
                break;
            break;
            case bcmFieldActionMirrorEgress:
                if ((a_offset.value[0] & 0x1) && (reg_value & 0x1)){
                    hw_index = a_offset.value[1] & 0x3;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                                        BCM_MIRROR_PORT_EGRESS, &param[0], &param[1]);

                     a_offset.value[0] &= 0xfffffffe;
                     valid = 1;
                     break;

                }
                if ((a_offset.value[0] & 0x2) && (reg_value & 0x2)) {
                    hw_index  = (a_offset.value[1] & 0xc) >> 2;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                                    BCM_MIRROR_PORT_EGRESS, &param[0], &param[1]);
                    a_offset.value[0] &= 0xfffffffc;
                    valid = 1;
                    break;
                }
                if ((a_offset.value[0] & 0x4) && (reg_value & 0x4)) {
                    hw_index  = (a_offset.value[1] & 0x30) >> 4;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                                        BCM_MIRROR_PORT_EGRESS, &param[0], &param[1]);
                    a_offset.value[0] &= 0xfffffff8;
                    valid = 1;
                    break;
                }
                if ((a_offset.value[0] & 0x8) && (reg_value & 0x8)) {
                    hw_index  = (a_offset.value[1] & 0xc0) >> 6;
                    _bcm_esw_mirror_mtp_to_modport(unit, hw_index, TRUE,
                                        BCM_MIRROR_PORT_EGRESS, &param[0], &param[1]);
                     a_offset.value[0] &= 0xfffffff0;
                     valid = 1;
                     break;
                }

            break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
             return BCM_E_MEMORY;
            }
            _FP_XGS3_ALLOC(mirror_recover, sizeof(_bcm_field_mirror_actions_recover_t),
                    "Field Mirror Actions Recover Alloc.");
            if (mirror_recover == NULL) {
                sal_free(f_act);
                return BCM_E_MEMORY;
            }

            mirror_recover->f_ent = f_ent;
            f_act->action = action_arr[action_idx];
            mirror_recover->action = f_act->action;
            mirror_recover->mtp_gport = param[1];
            mirror_recover->next = em_mirror_actions;
            em_mirror_actions = mirror_recover;

            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_copy_to_cpu_recover
 * Purpose:
 *      Recover actions that belong to copy to cpu set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_copy_to_cpu_recover(int unit, _field_entry_t *f_ent,
                              uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    _field_stage_t *stage_fc = NULL;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionCopyToCpu,
                                        bcmFieldActionCopyToCpuCancel,
                                        bcmFieldActionSwitchToCpuCancel,
                                        bcmFieldActionSwitchToCpuReinstate,
                                        bcmFieldActionTimeStampToCpu,
                                        bcmFieldActionGpCopyToCpu,
                                        bcmFieldActionGpCopyToCpuCancel,
                                        bcmFieldActionGpSwitchToCpuCancel,
                                        bcmFieldActionGpSwitchToCpuReinstate,
                                        bcmFieldActionGpTimeStampToCpu,
                                        bcmFieldActionYpCopyToCpu,
                                        bcmFieldActionYpCopyToCpuCancel,
                                        bcmFieldActionYpSwitchToCpuCancel,
                                        bcmFieldActionYpSwitchToCpuReinstate,
                                        bcmFieldActionYpTimeStampToCpu,
                                        bcmFieldActionRpCopyToCpu,
                                        bcmFieldActionRpCopyToCpuCancel,
                                        bcmFieldActionRpSwitchToCpuCancel,
                                        bcmFieldActionRpSwitchToCpuReinstate,
                                        bcmFieldActionRpTimeStampToCpu
                                    };

    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,f_ent->fs->stage_id, &stage_fc));
    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }


    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionCopyToCpu:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS &&
                    a_offset.value[1] == 1) {
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;
            case bcmFieldActionGpCopyToCpu:
            case bcmFieldActionYpCopyToCpu:
            case bcmFieldActionRpCopyToCpu:
                if (a_offset.value[1] == 1) {
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;
            case bcmFieldActionCopyToCpuCancel:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS &&
                    a_offset.value[0] == 2) {
                    valid = 1;
                }
                break;
            case bcmFieldActionGpCopyToCpuCancel:
            case bcmFieldActionYpCopyToCpuCancel:
            case bcmFieldActionRpCopyToCpuCancel:
                if (a_offset.value[0] == 2) {
                    valid = 1;
                }
                break;
            case bcmFieldActionSwitchToCpuCancel:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS &&
                    a_offset.value[0] == 3) {
                    valid = 1;
                }
                break;
            case bcmFieldActionGpSwitchToCpuCancel:
            case bcmFieldActionYpSwitchToCpuCancel:
            case bcmFieldActionRpSwitchToCpuCancel:
                if (a_offset.value[0] == 3)  {
                    valid = 1;
                }
                break;
            case bcmFieldActionSwitchToCpuReinstate:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS &&
                    a_offset.value[0] == 4) {
                    valid = 1;
                }
                break;
            case bcmFieldActionGpSwitchToCpuReinstate:
            case bcmFieldActionYpSwitchToCpuReinstate:
            case bcmFieldActionRpSwitchToCpuReinstate:
                if (a_offset.value[0] == 4) {
                    valid = 1;
                }
                break;
            case bcmFieldActionTimeStampToCpu:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS &&
                    a_offset.value[1] == 5) {
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;
            case bcmFieldActionRpTimeStampToCpu:
            case bcmFieldActionYpTimeStampToCpu:
            case bcmFieldActionGpTimeStampToCpu:
                if (a_offset.value[1] == 5) {
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
             return BCM_E_MEMORY;
            }

            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_l3swl2_set_recover
 * Purpose:
 *      Recover actions that belong to l3swl2 set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_l3swl2_set_recover(int unit, _field_entry_t *f_ent, uint32 *ebuf,
                                        _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    int  action_idx = 0;
    int param[6] = {0};
    int sa_disable = 0, da_disable = 0, vn_action = 0;
    int hw_index = 0, i = 0, valid = 0, ov_new = 0;
    egr_l3_next_hop_entry_t entry;
    bcm_mac_t mac;
    uint32 *temp;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionEgressClassSelect,
                                        bcmFieldActionHiGigClassSelect,
                                        bcmFieldActionNewClassId,
                                        bcmFieldActionDstMacNew,
                                        bcmFieldActionSrcMacNew,
                                        bcmFieldActionOuterVlanNew,
                                        bcmFieldActionVnTagNew,
                                        bcmFieldActionVnTagDelete,
                                        bcmFieldActionEtagNew,
                                        bcmFieldActionEtagDelete,
                                        bcmFieldActionFabricQueue,
                                        bcmFieldActionAddClassTag,
                                        bcmFieldActionL3Switch,
                                        bcmFieldActionMultipathHash,
                                        bcmFieldActionL3SwitchCancel,
                                        bcmFieldActionChangeL2FieldsCancel,
                                        bcmFieldActionBFDSessionIdNew,
                                    };


    temp = (uint32 *) &mac;
    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }
    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionEgressClassSelect:
                if ((a_offset.value[1] == 8) && (a_offset.value[0] != 0)) {
                    if (a_offset.value[0] == 0xf) {
                        param[0] = 0xb;
                    } else {
                        param[0] = a_offset.value[0];
                    }
                    valid = 1;
                }
                break;
            case bcmFieldActionHiGigClassSelect:
                if ((a_offset.value[1] == 8) && (a_offset.value[0] != 0)) {
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;
            case bcmFieldActionNewClassId:
                if ((a_offset.value[1] == 8) && (a_offset.offset[0] == 7)){
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;

            case bcmFieldActionSrcMacNew:
                if ((a_offset.offset[1]==19) && (a_offset.width[1]==4)
                    && (a_offset.value[1]==1)) {
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm,
                                     MEM_BLOCK_ANY, a_offset.value[0], &entry));
                    sa_disable = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                        &entry, IFP_ACTIONS__L3_UC_SA_DISABLEf);
                    da_disable = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                        &entry, IFP_ACTIONS__L3_UC_DA_DISABLEf);
                /*    ov_new = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, &entry, IFP_ACTIONS__L3_UC_VLAN_DISABLEf);
                    vn_action = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, &entry, IFP_ACTIONS__VNTAG_ACTIONf); */
                }
                if (sa_disable == 1) {
                    valid = 1;
                    hw_index = a_offset.value[0];
                    soc_mem_field_get(unit, EGR_L3_NEXT_HOPm, (uint32 *)&entry,
                                              IFP_ACTIONS__MAC_ADDRESSf, temp);
                    for (i=0; i < 6; i++) {
                        param[i] = mac[i];
                    }
                }
                break;
            case bcmFieldActionOuterVlanNew:
                if ((a_offset.offset[1]==19) && (a_offset.width[1]==4)
                    && (a_offset.value[1]==1)) {
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm,
                                     MEM_BLOCK_ANY, a_offset.value[0], &entry));
                    ov_new = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, &entry,
                                                 IFP_ACTIONS__L3_UC_VLAN_DISABLEf);
                    if (ov_new == 1) {
                        valid = 1;
                        param[0] = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                               &entry,  IFP_ACTIONS__INTF_NUMf);
                    }
                }
                break;
            case bcmFieldActionDstMacNew:
                if ((a_offset.offset[1]==19) && (a_offset.width[1]==4)
                    && (a_offset.value[1]==1)) {
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm,
                                     MEM_BLOCK_ANY, a_offset.value[0], &entry));
                    sa_disable = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                           &entry, IFP_ACTIONS__L3_UC_SA_DISABLEf);
                    da_disable = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                        &entry, IFP_ACTIONS__L3_UC_DA_DISABLEf);
           /*         ov_new = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, &entry, IFP_ACTIONS__L3_UC_VLAN_DISABLEf);
                    vn_action = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm, &entry, IFP_ACTIONS__VNTAG_ACTIONf); */
                }
                if (da_disable == 1) {
                    valid = 1;
                    hw_index = a_offset.value[0];
                    soc_mem_field_get(unit, EGR_L3_NEXT_HOPm, (uint32 *)&entry,
                                      IFP_ACTIONS__MAC_ADDRESSf, temp);
                    for (i=0; i < 6; i++) {
                        param[i] = mac[i];
                    }
                }
                break;
            case bcmFieldActionVnTagDelete:
                if ((a_offset.offset[1]==19) && (a_offset.width[1]==4)
                    && (a_offset.value[1]==1)) {
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm,
                                    MEM_BLOCK_ANY, a_offset.value[0], &entry));
                    vn_action = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                            &entry, IFP_ACTIONS__VNTAG_ACTIONf);
                    if (vn_action == 0x3) {
                        valid = 1;
                        hw_index = a_offset.value[0];
                    }
                }
                break;
            case bcmFieldActionEtagNew:
                if ((a_offset.offset[1]==19) && (a_offset.width[1]==4)
                    && (a_offset.value[1]==1)) {
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm,
                                MEM_BLOCK_ANY, a_offset.value[0], &entry));
                    vn_action = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                            &entry, IFP_ACTIONS__VNTAG_ACTIONf);
                    if (vn_action == 0x2) {
                        valid = 1;
                        hw_index = a_offset.value[0];
                        param[0] = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                                   &entry, IFP_ACTIONS__VNTAGf);
                    }
                }
                break;
            case bcmFieldActionVnTagNew:
                if ((a_offset.offset[1]==19) && (a_offset.width[1]==4)
                    && (a_offset.value[1]==1)) {
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit, EGR_L3_NEXT_HOPm,
                                MEM_BLOCK_ANY, a_offset.value[0], &entry));
                    vn_action = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                            &entry, IFP_ACTIONS__VNTAG_ACTIONf);
                    if (vn_action == 0x1) {
                        valid = 1;
                        hw_index = a_offset.value[0];
                        param[0] = soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                                &entry, IFP_ACTIONS__VNTAGf);
                    }
                }
                break;
            case bcmFieldActionFabricQueue:
                if (a_offset.value[2] == 3) {
                    param[0] = a_offset.value[0];
                    param[1] = a_offset.value[1];
                }
                break;
            case bcmFieldActionAddClassTag:
                if (a_offset.value[1] == 4) {
                    param[0] = a_offset.value[0];
                }
                break;

            case bcmFieldActionL3Switch:
#ifdef INCLUDE_L3
                if ((a_offset.value[2] == 6) && (a_offset.value[1] == 1)) {
                     param[0] = a_offset.value[0] + BCM_XGS3_MPATH_EGRESS_IDX_MIN;
                     valid = 1;
                }
                if ((a_offset.value[2] == 6) && (a_offset.value[1] == 0)) {
                    param[0] = a_offset.value[1] + BCM_XGS3_EGRESS_IDX_MIN;
                    valid = 1;
                }
#endif
                break;
            case bcmFieldActionMultipathHash:
                if ((a_offset.value[1] == 6) && (a_offset.width[0] == 3)) {
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;
            case bcmFieldActionL3SwitchCancel:
                if (a_offset.value[1] == 7) {
                    valid = 1;
                }
                break;
            case bcmFieldActionChangeL2FieldsCancel:
                if (a_offset.value[1] == 2) {
                    valid = 1;
                }
                break;
            case bcmFieldActionBFDSessionIdNew:
                if (a_offset.value[1] == 9) {
                    param[0] = a_offset.value[0];
                    valid = 1;
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
             return BCM_E_MEMORY;
            }

            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
                param[i] = 0;
            }
            f_act->hw_index = hw_index;
            hw_index = 0;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_emredirect_set_recover
 * Purpose:
 *      Recover actions that belong to redirect set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_emredirect_set_recover(int unit, _field_entry_t *f_ent,
                            uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
   _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    _field_stage_t *stage_fc = NULL;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionRedirectTrunk,
                                        bcmFieldActionRedirect,
                                        bcmFieldActionUnmodifiedPacketRedirectPort,
                                        bcmFieldActionRedirectEgrNextHop,
                                        bcmFieldActionRedirectCancel,
                                        bcmFieldActionRedirectPbmp,
                                        bcmFieldActionRedirectVlan,
                                        bcmFieldActionRedirectBcastPbmp,
                                        bcmFieldActionRedirectMcast,
                                        bcmFieldActionRedirectIpmc,
                                        bcmFieldActionEgressMask,
                                        bcmFieldActionEgressPortsAdd,
                                        bcmFieldActionFabricEHAddOrUpdate
                                      };
    soc_profile_mem_t *redirect_profile;
    ifp_redirection_profile_entry_t *entry = NULL;
    bcm_pbmp_t pbmp;
    int rv = BCM_E_NONE;
#ifdef INCLUDE_L3
    int vp = 0;
#endif

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    entry = sal_alloc(sizeof(ifp_redirection_profile_entry_t), "em wb redirect entry");
    BCM_IF_ERROR_CLEANUP(_field_stage_control_get(unit,f_ent->fs->stage_id, &stage_fc));
    BCM_IF_ERROR_CLEANUP(_field_trx_redirect_profile_get(unit, &redirect_profile));
    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        BCM_IF_ERROR_CLEANUP(_bcm_field_action_val_get(unit, f_ent,
                                ebuf, action_arr[action_idx], 0, &a_offset));
        switch (action_arr[action_idx]) {
            case bcmFieldActionRedirect:
                if ((a_offset.value[3] == 1) && (a_offset.value[2] == 0)
                     && (a_offset.value[1] == 0)) {
                    valid = 1;
                    param[0] = (a_offset.value[0] >> 8) & 0xff;
                    param[1] = (a_offset.value[0]) & 0xff;
                } else if ((a_offset.value[2] == 1) && (a_offset.value[1] == 6)) {
#ifdef INCLUDE_L3
                    valid = 1;
                    vp = a_offset.value[0];
                    if (_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
                        BCM_GPORT_MPLS_PORT_ID_SET(param[1], vp);
                    } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
                        BCM_GPORT_MIM_PORT_ID_SET(param[1], vp);
                    } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeVxlan)) {
                        BCM_GPORT_VXLAN_PORT_ID_SET(param[1], vp);
                    } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
                        BCM_GPORT_TRILL_PORT_ID_SET(param[1], vp);
                    } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeL2Gre)) {
                        BCM_GPORT_L2GRE_PORT_ID_SET(param[1], vp);
                    } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeNiv)) {
                          BCM_GPORT_NIV_PORT_ID_SET(param[1], vp);
                    }
#endif
                }
                break;
            case bcmFieldActionRedirectTrunk:
                if ((a_offset.value[3] == 1) && (a_offset.value[2] == 0)
                    && (a_offset.value[1] == 1)) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
#if defined(BCM_TRIDENT2_SUPPORT) && defined(INCLUDE_L3)
                else {
                    int vp_id_min = -1;
                    BCM_IF_ERROR_CLEANUP(_bcm_field_action_val_get(unit, f_ent,
                                                  ebuf, action_arr[action_idx],
                                   _BCM_FIELD_ACTION_REDIRECT_DVP, &a_offset));
                    if ((a_offset.value[2] == 1) && (a_offset.value[1] == 6)) {
                        BCM_IF_ERROR_CLEANUP(
                               _bcm_esw_trunk_chip_info_vp_resource_get(unit,
                                                                   &vp_id_min,
                                                                   NULL, NULL));
                        valid = 1;
                        param[0] = a_offset.value[0] + vp_id_min;
                    }
                }
#endif
                break;
            case bcmFieldActionUnmodifiedPacketRedirectPort:
                 if ((a_offset.value[3] == 1) && (a_offset.value[2] == 1)
                    && (a_offset.value[1] == 0)) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                 }
                 break;
#ifdef INCLUDE_L3
            case bcmFieldActionRedirectEgrNextHop:
                if ((a_offset.value[2] == 1) && (a_offset.value[1] == 2)) {
                    valid = 1;
                    param[0] = (a_offset.value[0] & 0x7fff)
                                 + BCM_XGS3_EGRESS_IDX_MIN;
                }
                if ((a_offset.value[2] == 1) && (a_offset.value[1] == 3)) {
                    valid = 1;
                    param[0] = (a_offset.value[0] & 0x7ff)
                                + BCM_XGS3_MPATH_EGRESS_IDX_MIN;
                }
                break;
#endif
            case bcmFieldActionRedirectCancel:
                 if (a_offset.value[0] == 2) {
                    valid = 1;
                }
                break;
            case bcmFieldActionRedirectVlan:
                if ((a_offset.value[2] == 3) && (a_offset.value[1] == 1)) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
            case bcmFieldActionRedirectMcast:
                if ((a_offset.value[2] == 3) && (a_offset.value[1] == 2)) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
#ifdef INCLUDE_L3
            case bcmFieldActionRedirectIpmc:
                if ((a_offset.value[2]== 3) && (a_offset.value[1] == 3)) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
#endif
            case bcmFieldActionEgressMask:
                if (a_offset.value[1] == 4) {
                    hw_index = a_offset.value[0];
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit,
                                IFP_REDIRECTION_PROFILEm, MEM_BLOCK_ANY,
                                hw_index, entry));
                    soc_mem_field_get(unit, IFP_REDIRECTION_PROFILEm,
                                     (uint32 *)entry, BITMAPf, pbmp.pbits);
                    for (i=0; i < 4; i++) {
                        param[i] = SOC_PBMP_WORD_GET(pbmp, i);
                    }
                }
                break;
            case bcmFieldActionEgressPortsAdd:
                if (a_offset.value[1] == 5) {
                    hw_index = a_offset.value[0];
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit,
                                        IFP_REDIRECTION_PROFILEm,
                                        MEM_BLOCK_ANY, hw_index, entry));
                    soc_mem_field_get(unit, IFP_REDIRECTION_PROFILEm,
                                        (uint32 *)entry, BITMAPf, pbmp.pbits);
                    for (i=0; i < 4; i++) {
                        param[i] = SOC_PBMP_WORD_GET(pbmp, i);
                    }
                }
                break;
            case bcmFieldActionRedirectBcastPbmp:
                if ((a_offset.value[3] == 3) && (a_offset.value[2] == 1)
                    && (a_offset.value[1] == 1)) {
                    hw_index = a_offset.value[0];
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit,
                                    IFP_REDIRECTION_PROFILEm, MEM_BLOCK_ANY,
                                    hw_index, entry));
                    soc_mem_field_get(unit, IFP_REDIRECTION_PROFILEm,(
                                        uint32 *) entry, BITMAPf,pbmp.pbits);
                    for (i=0; i < 4; i++) {
                        param[i] = SOC_PBMP_WORD_GET(pbmp, i);
                    }
                }
                break;
            case bcmFieldActionRedirectPbmp:
                if ((a_offset.value[2] == 3) && (a_offset.value[1] == 0)) {
                    hw_index = a_offset.value[0];
                    SOC_IF_ERROR_RETURN(soc_mem_read(unit,
                                        IFP_REDIRECTION_PROFILEm,
                                        MEM_BLOCK_ANY, hw_index, entry));
                    soc_mem_field_get(unit, IFP_REDIRECTION_PROFILEm,
                                     (uint32 *)entry, BITMAPf, pbmp.pbits);
                    for (i=0; i < 4; i++) {
                        param[i] = SOC_PBMP_WORD_GET(pbmp, i);
                    }
                }
                break;
             case bcmFieldActionFabricEHAddOrUpdate:
                 if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
                     valid =1;
                 }
                 break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            if (f_act == NULL) {
                rv = BCM_E_MEMORY;
                goto cleanup;
            }

            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
                param[i] = 0;
            }
            f_act->hw_index = hw_index;
            hw_index = 0;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
            if ((f_act->action == bcmFieldActionRedirectBcastPbmp) ||
                (f_act->action == bcmFieldActionRedirectPbmp) ||
                (f_act->action == bcmFieldActionEgressMask) ||
                (f_act->action == bcmFieldActionEgressPortsAdd)) {
                SOC_PROFILE_MEM_REFERENCE(unit, redirect_profile, hw_index, 1);
                SOC_PROFILE_MEM_ENTRIES_PER_SET(unit,
                                                redirect_profile, hw_index, 1);
            }
            if (f_act->action == bcmFieldActionFabricEHAddOrUpdate) {
                rv = _field_action_eh_recover(unit, f_ent,
                                          ebuf, f_act);
                BCM_IF_ERROR_CLEANUP(rv);
            }
        }

    }
cleanup:
    if (entry != NULL) {
        sal_free(entry);
    }
    return rv;
}
/*
 * Function:
 *      _field_wb_counter_set_recover
 * Purpose:
 *      Recover actions that belong to counter set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_counter_set_recover(int unit, _field_entry_t *f_ent,
                    uint32 *ebuf, _field_action_bmp_t *act_bmp) {
    uint32 hw_offset_mode = 0;
    _field_stat_t *f_st = NULL;
    int rv = 0;
    bcm_stat_object_t       stat_obj;     /* Stat object type. */
    bcm_stat_flex_mode_t    offset_mode;  /* Counter mode.     */
    bcm_stat_group_mode_t   stat_mode;    /* Stat type bcmStatGroupModeXXX. */
    int fp_stat_mode_max;                 /* Max stat modes. */
    uint8  idx;                           /* Index to carry loop count. */
    uint32 pool_num;                      /* Flex Stat Hw Pool No. */
    uint32 base_index;                    /* Flex Stat counter base index. */
    uint32 num_hw_cntrs;                  /* Number of counters allocated. */
    uint32 req_bmap = 0;                  /* Requested statistics bitmap.  */
    uint32 hw_bmap;                       /* HW supported statistics bitmap. */

    hw_offset_mode = (*ebuf & 0x1800) >> 11;
    pool_num = (*ebuf & 0x7c0) >> 6;
    base_index = (*ebuf & 0x1ff7000) >> 13;

    _bcm_esw_stat_flex_check_ingress_em_table_entry(unit,
                                        f_ent->group->instance,
                                        hw_offset_mode, pool_num, base_index);

    rv = _bcm_field_stat_get(unit, f_ent->statistic.sid,
                             &f_st);
    if ((BCM_SUCCESS(rv)) && (f_st->flex_mode != 0)) {
        offset_mode = pool_num = base_index = num_hw_cntrs = -1;
         /* Get flex counter hardware details stored in flex stat ID.*/
        _bcm_esw_stat_get_counter_id_info(unit, f_st->flex_mode, &stat_mode,
                                          &stat_obj, &offset_mode, &pool_num,
                                          &base_index);

       if ((bcmStatObjectIngFieldStageIngress == stat_obj) ||
           (bcmStatObjectIngExactMatch == stat_obj)) {
            /* Get application requested bitmap. */
            BCM_IF_ERROR_RETURN(_bcm_field_stat_array_to_bmap(unit, f_st,
                                                              &req_bmap));
            fp_stat_mode_max = th_ingress_cntr_hw_mode_tbl_size;
            for (idx = 0; idx < fp_stat_mode_max; idx++) {
                 hw_bmap = th_ingress_cntr_hw_mode_tbl[idx].hw_bmap;
                 num_hw_cntrs = th_ingress_cntr_hw_mode_tbl[idx].hw_entry_count;
                 if (0 == ((req_bmap | hw_bmap) & ~(hw_bmap))) {
                      /*
                       * For maximizing utilization of hardware counters, this should
                       * select the hw_mode both supports the requested stats
                       * AND minimizes the number of required hardware counters.
                       */
                     break;
                }
            }
       } else {
            num_hw_cntrs = 1;
       }

       /* Store flex stat hardware details in stat data structure. */
       f_st->pool_index = pool_num;
       f_st->hw_index = base_index;
       f_st->hw_mode = offset_mode;
       f_st->hw_entry_count = num_hw_cntrs;
       f_st->hw_stat = req_bmap;
       f_st->sw_ref_count =  f_st->sw_ref_count + 1;
       if(_FP_ENTRY_INSTALLED & f_ent->flags) {
           f_st->hw_ref_count =  f_st->hw_ref_count + 1;
       }
       f_st->gid  = f_ent->group->gid;
       f_st->stage_id = _BCM_FIELD_STAGE_EXACTMATCH;
    }
    return BCM_E_NONE;

}
/*
 * Function:
 *      _field_wb_timestamp_set_recover
 * Purpose:
 *      Recover actions that belong to time stamp set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_timestamp_set_recover(int unit, _field_entry_t *f_ent,
                                uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                    bcmFieldActionIngressTimeStampInsert,
                                    bcmFieldActionIngressTimeStampInsertCancel,
                                    bcmFieldActionEgressTimeStampInsert,
                                    bcmFieldActionEgressTimeStampInsertCancel,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionEgressTimeStampInsert:
            case bcmFieldActionIngressTimeStampInsert:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
                break;
            case bcmFieldActionIngressTimeStampInsertCancel:
            case bcmFieldActionEgressTimeStampInsertCancel:
                if (a_offset.value[0] ==  2) {
                    valid = 1;
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions time stamp");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_instrumentation_set_recover
 * Purpose:
 *      Recover actions that belong to instrumentation set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_instrumentation_set_recover(int unit, _field_entry_t *f_ent,
                                 uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionPacketTraceEnable,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionPacketTraceEnable:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions instrumentation");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_em_dlb_hgt_lag_set_recover
 * Purpose:
 *      Recover actions that belong to dlb_hgt_lag set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_dlb_hgt_lag_set_recover(int unit, _field_entry_t *f_ent,
                            uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionDynamicTrunkEnable,
                                        bcmFieldActionDynamicTrunkCancel,
                                        bcmFieldActionDynamicHgTrunkEnable,
                                        bcmFieldActionDynamicHgTrunkCancel,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionDynamicTrunkCancel:
            case bcmFieldActionDynamicTrunkEnable:
            case bcmFieldActionDynamicHgTrunkCancel:
            case bcmFieldActionDynamicHgTrunkEnable:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions dlb hgt lag");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
#if defined(INCLUDE_L3)
/*
 * Function:
 *      _field_wb_protection_switching_set_recover
 * Purpose:
 *      Recover actions that belong to protection switching set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_protection_switching_set_recover(int unit, _field_entry_t *f_ent,
                             uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionRecoverableDropCancel,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionRecoverableDropCancel:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions protection switching");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_dlb_ecmp_set_recover
 * Purpose:
 *      Recover actions that belong to dlb_ecmp set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_dlb_ecmp_set_recover(int unit, _field_entry_t *f_ent,
                             uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionDynamicEcmpEnable,
                                        bcmFieldActionDynamicEcmpCancel,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionDynamicEcmpEnable:
            case bcmFieldActionDynamicEcmpCancel:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions dlb ecmp");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_dlb_alternate_path_control_set_recover
 * Purpose:
 *      Recover actions that belong to dlb_alternate_path_control set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_dlb_alternate_path_control_set_recover(int unit, _field_entry_t *f_ent,
                                      uint32 *ebuf, _field_action_bmp_t *act_bmp)
{
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
                                        bcmFieldActionDgm,
                                        bcmFieldActionDgmCost,
                                        bcmFieldActionDgmBias,
                                        bcmFieldActionDgmThreshold,
                                      };

    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionDgm:
                if (a_offset.value[0] ==  1) {
                    valid = 1;
                }
                break;
            case bcmFieldActionDgmThreshold:
            case bcmFieldActionDgmBias:
            case bcmFieldActionDgmCost:
                param[0] = a_offset.value[0];
                valid = 1;
                break;
            default:
                break;
        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions dlb dgm");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
#endif /*INCLUDE_L3*/

/*
 * Function:
 *      _field_wb_change_ecn_set_recover
 * Purpose:
 *      Recover actions that belong to ecn set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_change_ecn_set_recover(int unit, _field_entry_t *f_ent,
        uint32 *ebuf, _field_action_bmp_t *act_bmp) {
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    _field_stage_t *stage_fc = NULL;
    bcm_field_action_t action_arr[] = {
        bcmFieldActionRpEcnNew,
        bcmFieldActionYpEcnNew,
        bcmFieldActionGpEcnNew,
        bcmFieldActionEcnNew
    };

    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,f_ent->fs->stage_id, &stage_fc));
    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionRpEcnNew:
            case bcmFieldActionYpEcnNew:
            case bcmFieldActionGpEcnNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            case bcmFieldActionEcnNew:
                if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
                    valid = 1;
                    param[0] = a_offset.value[0];
                }
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }


    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_pkt_pri_set_recover
 * Purpose:
 *      Recover actions that belong to pkt pri set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_pkt_pri_set_recover(int unit, _field_entry_t *f_ent,
        uint32 *ebuf, _field_action_bmp_t *act_bmp) {

    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
        bcmFieldActionPrioPktCopy,
        bcmFieldActionPrioPktNew,
        bcmFieldActionPrioPktCancel,
        bcmFieldActionPrioPktTos,
        bcmFieldActionDot1pPreserve,
        bcmFieldActionGpPrioPktCopy,
        bcmFieldActionGpPrioPktNew,
        bcmFieldActionGpPrioPktCancel,
        bcmFieldActionGpPrioPktTos,
        bcmFieldActionGpDot1pPreserve,
        bcmFieldActionRpPrioPktCopy,
        bcmFieldActionRpPrioPktNew,
        bcmFieldActionRpPrioPktCancel,
        bcmFieldActionRpPrioPktTos,
        bcmFieldActionRpDot1pPreserve,
        bcmFieldActionYpPrioPktCopy,
        bcmFieldActionYpPrioPktNew,
        bcmFieldActionYpPrioPktCancel,
        bcmFieldActionYpPrioPktTos,
        bcmFieldActionYpDot1pPreserve
    };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionPrioPktCopy:
            case bcmFieldActionPrioPktTos:
            case bcmFieldActionPrioPktCancel:
            case bcmFieldActionDot1pPreserve:
            case bcmFieldActionGpPrioPktCopy:
            case bcmFieldActionGpPrioPktTos:
            case bcmFieldActionGpPrioPktCancel:
            case bcmFieldActionYpPrioPktCopy:
            case bcmFieldActionYpPrioPktTos:
            case bcmFieldActionYpPrioPktCancel:
            case bcmFieldActionRpPrioPktCopy:
            case bcmFieldActionRpPrioPktTos:
            case bcmFieldActionRpPrioPktCancel:
            case bcmFieldActionGpDot1pPreserve:
            case bcmFieldActionYpDot1pPreserve:
            case bcmFieldActionRpDot1pPreserve:
                valid = 1;
                break;
            case bcmFieldActionPrioPktNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            case bcmFieldActionGpPrioPktNew:
            case bcmFieldActionRpPrioPktNew:
            case bcmFieldActionYpPrioPktNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }
    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_dscp_tos_set_recover
 * Purpose:
 *      Recover actions that belong to dscp tos set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_dscp_tos_set_recover(int unit, _field_entry_t *f_ent,
        uint32 *ebuf, _field_action_bmp_t *act_bmp) {
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
        bcmFieldActionDscpNew,
        bcmFieldActionDscpCancel,
        bcmFieldActionDscpPreserve,
        bcmFieldActionRpDscpNew,
        bcmFieldActionRpDscpCancel,
        bcmFieldActionRpDscpPreserve,
        bcmFieldActionYpDscpNew,
        bcmFieldActionYpDscpCancel,
        bcmFieldActionYpDscpPreserve,
        bcmFieldActionGpDscpNew,
        bcmFieldActionGpDscpCancel,
        bcmFieldActionGpDscpPreserve,
        bcmFieldActionGpTosPrecedenceNew,
        bcmFieldActionGpTosPrecedenceCopy
    };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionDscpNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            case bcmFieldActionRpDscpNew:
            case bcmFieldActionYpDscpNew:
            case bcmFieldActionGpDscpNew:
            case bcmFieldActionGpTosPrecedenceNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            case bcmFieldActionDscpCancel:
            case bcmFieldActionGpDscpCancel:
            case bcmFieldActionYpDscpCancel:
            case bcmFieldActionRpDscpCancel:
            case bcmFieldActionDscpPreserve:
            case bcmFieldActionGpDscpPreserve:
            case bcmFieldActionYpDscpPreserve:
            case bcmFieldActionRpDscpPreserve:
            case bcmFieldActionGpTosPrecedenceCopy:
                valid = 1;
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }

    return BCM_E_NONE;

}/*
  * Function:
  *      _field_wb_drop_precedence_set_recover
  * Purpose:
  *      Recover actions that belong to drop precedence set
  *
  * Parameters:
  *      unit     - (IN) StrataSwitch unit #.
  *      f_ent    - (IN) pointer to entry structure which refers
  *                      this action profile.
  *      act_data  - (IN) action data in the entry obtained from
  *                      exact match tables
  *   Returns:
  *      BCM_E_XXX
  */
int
_field_wb_drop_precedence_set_recover(int unit, _field_entry_t *f_ent,
        uint32 *ebuf, _field_action_bmp_t *act_bmp) {
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
        bcmFieldActionGpDropPrecedence,
        bcmFieldActionYpDropPrecedence,
        bcmFieldActionRpDropPrecedence,
        bcmFieldActionDropPrecedence
    };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionGpDropPrecedence:
            case bcmFieldActionYpDropPrecedence:
            case bcmFieldActionRpDropPrecedence:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            case bcmFieldActionDropPrecedence:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_cos_or_int_pri_set_recover
 * Purpose:
 *      Recover actions that belong to cos or int priority set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_cos_or_int_pri_set_recover(int unit, _field_entry_t *f_ent,
        uint32 *ebuf, _field_action_bmp_t *act_bmp) {
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
        bcmFieldActionPrioIntNew,
        bcmFieldActionGpPrioIntNew,
        bcmFieldActionYpPrioIntNew,
        bcmFieldActionRpPrioIntNew,
        bcmFieldActionPrioIntCopy,
        bcmFieldActionRpPrioIntCopy,
        bcmFieldActionGpPrioIntCopy,
        bcmFieldActionYpPrioIntCopy,
        bcmFieldActionPrioIntTos,
        bcmFieldActionRpPrioIntTos,
        bcmFieldActionGpPrioIntTos,
        bcmFieldActionYpPrioIntTos,
        bcmFieldActionPrioIntCancel,
        bcmFieldActionRpPrioIntCancel,
        bcmFieldActionGpPrioIntCancel,
        bcmFieldActionYpPrioIntCancel,
        bcmFieldActionCosQNew,
        bcmFieldActionRpCosQNew,
        bcmFieldActionGpCosQNew,
        bcmFieldActionYpCosQNew,
        bcmFieldActionCosMapNew,
        bcmFieldActionRpCosMapNew,
        bcmFieldActionGpCosMapNew,
        bcmFieldActionYpCosMapNew,
        bcmFieldActionUcastCosQNew,
        bcmFieldActionRpUcastCosQNew,
        bcmFieldActionGpUcastCosQNew,
        bcmFieldActionYpUcastCosQNew,
        bcmFieldActionMcastCosQNew,
        bcmFieldActionRpMcastCosQNew,
        bcmFieldActionGpMcastCosQNew,
        bcmFieldActionYpMcastCosQNew
    };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionPrioIntCopy:
            case bcmFieldActionGpPrioIntCopy:
            case bcmFieldActionYpPrioIntCopy:
            case bcmFieldActionRpPrioIntCopy:
            case bcmFieldActionPrioIntTos:
            case bcmFieldActionGpPrioIntTos:
            case bcmFieldActionYpPrioIntTos:
            case bcmFieldActionRpPrioIntTos:
            case bcmFieldActionPrioIntCancel:
            case bcmFieldActionGpPrioIntCancel:
            case bcmFieldActionYpPrioIntCancel:
            case bcmFieldActionRpPrioIntCancel:
            case bcmFieldActionCosQNew:
            case bcmFieldActionGpCosQNew:
            case bcmFieldActionYpCosQNew:
            case bcmFieldActionRpCosQNew:
            case bcmFieldActionUcastCosQNew:
            case bcmFieldActionGpUcastCosQNew:
            case bcmFieldActionYpUcastCosQNew:
            case bcmFieldActionRpUcastCosQNew:
            case bcmFieldActionMcastCosQNew:
            case bcmFieldActionGpMcastCosQNew:
            case bcmFieldActionYpMcastCosQNew:
            case bcmFieldActionRpMcastCosQNew:
                valid = 1;
                break;
            case bcmFieldActionPrioIntNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            case bcmFieldActionGpPrioIntNew:
            case bcmFieldActionYpPrioIntNew:
            case bcmFieldActionRpPrioIntNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            case bcmFieldActionGpCosMapNew:
            case bcmFieldActionYpCosMapNew:
            case bcmFieldActionRpCosMapNew:
            case bcmFieldActionCosMapNew:
                valid = 1;
                _BCM_COSQ_CLASSIFIER_FIELD_SET(param[0], a_offset.value[0]);
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_int_cn_set_recover
 * Purpose:
 *      Recover actions that belong to int cn set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_int_cn_set_recover(int unit, _field_entry_t *f_ent,
        uint32 *ebuf, _field_action_bmp_t *act_bmp) {
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
        bcmFieldActionGpIntCongestionNotificationNew,
        bcmFieldActionYpIntCongestionNotificationNew,
        bcmFieldActionRpIntCongestionNotificationNew
    };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionYpIntCongestionNotificationNew:
            case bcmFieldActionRpIntCongestionNotificationNew:
            case bcmFieldActionGpIntCongestionNotificationNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *     _field_wb_input_priority_set_recover 
 * Purpose:
 *      Recover actions that belong to input priority set
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this action profile.
 *      act_data  - (IN) action data in the entry obtained from
 *                      exact match tables
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_input_priority_set_recover(int unit, _field_entry_t *f_ent,
        uint32 *ebuf, _field_action_bmp_t *act_bmp) {
    _bcm_field_action_offset_t a_offset;
    _field_action_t *f_act = NULL, *prev_act = NULL;
    bcm_field_action_t action_idx;
    int param[6] = {0};
    int hw_index = 0, i = 0, valid = 0;
    bcm_field_action_t action_arr[] = {
        bcmFieldActionUntaggedPacketPriorityNew,
        bcmFieldActionPfcClassNew
    };


    if (f_ent->actions != NULL) {
        prev_act = f_ent->actions;
        while (prev_act->next != NULL) {
            prev_act = prev_act->next;
        }
    }

    for (action_idx = 0; action_idx < (sizeof(action_arr)/sizeof(int)); action_idx++) {
        valid  = 0;
        if (act_bmp != NULL) {
            if (act_bmp->w == NULL) {
                continue;
            } else {
               if (!SHR_BITGET(act_bmp->w, action_arr[action_idx])) {
                    continue;
                }
            }
        }
        ACTION_GET(unit, f_ent, ebuf, action_arr[action_idx], 0, &a_offset);
        switch (action_arr[action_idx]) {
            case bcmFieldActionUntaggedPacketPriorityNew:
            case bcmFieldActionPfcClassNew:
                valid = 1;
                param[0] = a_offset.value[0];
                break;
            default:
                break;

        }
        if (valid) {
            f_act = NULL;
            _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "FP em actions qos");
            f_act->action = action_arr[action_idx];
            for (i=0; i < 6; i++) {
                f_act->param[i] = param[i];
            }
            f_act->hw_index = hw_index;
            f_act->old_index = -1;
            f_act->flags = _FP_ACTION_VALID;
            if (prev_act == NULL) {
                prev_act = f_act;
                f_ent->actions = prev_act;
            } else {
                prev_act->next = f_act;
                prev_act = prev_act->next;
            }
        }
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_wb_em_actions_recover
 * Purpose:
 *      Recover all non-qos actions for a particular entry.
 *      This profile may be pointed to, by many entries. so, we
 *      DMA the entire table and store the pointers in statically
 *      declared buffer. Corresponding free function is called
 *      at the end of em reinit.
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) pointer to entry structure which refers
 *                      this qos profile.
 *      act_idx  - (IN) action profile index (obtained from ACTION_PROFILE_IDf
 *                       from exact match tables)
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_wb_em_actions_recover(int unit, _field_entry_t *f_ent, int act_idx,
                             uint32 *act_data)
{
    soc_mem_t act_prof_mems[_FP_MAX_NUM_PIPES+1] =
            { EXACT_MATCH_ACTION_PROFILE_PIPE0m,
              EXACT_MATCH_ACTION_PROFILE_PIPE1m,
              EXACT_MATCH_ACTION_PROFILE_PIPE2m,
              EXACT_MATCH_ACTION_PROFILE_PIPE3m,
              EXACT_MATCH_ACTION_PROFILEm,
            };
    static  soc_mem_t *mem;
    int pipe = 0, num_pipes = 0;
    _field_stage_t *stage_fc = NULL;
    uint32 *e_buf = NULL;
    int index_min = 0, index_max = 0;
    int rv = 0;


    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                _BCM_FIELD_STAGE_EXACTMATCH, &stage_fc));

    if (act_profile_buf[0] == NULL) {

        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            num_pipes = 1;
            mem = &act_prof_mems[_FP_MAX_NUM_PIPES];
        } else {
            num_pipes = stage_fc->num_pipes;
            mem = &act_prof_mems[0];
        }
        for (pipe = 0; pipe < num_pipes; pipe++) {
            act_profile_buf[pipe] = soc_cm_salloc(unit,
                             SOC_MEM_TABLE_BYTES(unit, act_prof_mems[pipe]),
                                          "act Profile table buffer");

            index_min = soc_mem_index_min(unit, mem[pipe]);
            index_max = soc_mem_index_max(unit, mem[pipe]);
            rv = soc_mem_read_range(unit, mem[pipe], MEM_BLOCK_ALL,
                                  index_min, index_max, act_profile_buf[pipe]);
            BCM_IF_ERROR_RETURN(rv);

        }
    }
    pipe = f_ent->group->instance;

    e_buf = soc_mem_table_idx_to_pointer(unit, act_prof_mems[pipe],
                                  uint32 *, act_profile_buf[pipe], act_idx);
    _field_wb_action_profile_parse(unit, f_ent, e_buf, act_data, NULL);
    return BCM_E_NONE;


}
int
_field_em_class_action_add(int unit, _field_entry_t *f_ent, int class_id) {
    _field_action_t *f_act = NULL;

    _FP_XGS3_ALLOC(f_act, sizeof(_field_action_t), "class id action");
    if (f_act == NULL) {
     return BCM_E_MEMORY;
    }

    f_act->action = bcmFieldActionExactMatchClassId;
    f_act->flags = _FP_ACTION_VALID;
    f_act->param[0] = class_id;
    f_act->next = f_ent->actions;
    f_ent->actions = f_act;

    return BCM_E_NONE;
}
int
_field_wb_em_defentry_recover(int unit, _field_entry_t *f_ent, soc_mem_t mem) {
    int idx = 0;
    uint32 *bufp;
    uint32 act_data[2] = {0};
    int qos_idx  = 0, act_idx = 0, class_id = 0;
    exact_match_default_policy_entry_t ebuf;

    idx = f_ent->group->ext_codes[0].keygen_index;
    bufp = (uint32 *)&ebuf;
    sal_memcpy(bufp, soc_mem_entry_null(unit, mem),
            soc_mem_entry_words(unit, mem) * sizeof(uint32));


    SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY, idx, bufp));


    soc_mem_field_get(unit, mem, bufp, ACTION_DATAf, act_data);
    qos_idx = soc_mem_field32_get(unit, mem, bufp, QOS_PROFILE_IDf);
    act_idx = soc_mem_field32_get(unit, mem, bufp, ACTION_PROFILE_IDf);
    class_id = soc_mem_field32_get(unit, mem, bufp, EXACT_MATCH_CLASS_IDf);

    _field_wb_qos_actions_recover(unit, f_ent, qos_idx);
    _field_wb_em_actions_recover(unit, f_ent, act_idx, act_data);

    if (class_id) {
        _field_em_class_action_add(unit, f_ent, class_id);
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      _field_em_group_entries_recover
 * Purpose:
 *      Recover all entries for a given exactmatch match group
 *      For each entry, entry_id, flags, (flags for 2nd part in
 *      case of double wide group), hw_index are stored in order
 *      Read EXACT_MATCH_2 or EXACT_MATCH_4 based on group mode,
 *      parse the entry, and populate entry structe, link it to
 *      group slices.  IN case of double wide, as there are no real
 *      slices(same memory is used in different views), both are
 *      linked to the same slice. Read action profile and qos_action
 *      profile pointer and call corresponding functions to retrive
 *      action data.
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      fg       - (IN) pointer to group structure whose entries
 *                  have to be recovered.
 *   Returns:
 *      BCM_E_XXX
 */
int
_field_em_group_entries_recover(int unit, _field_group_t *fg)
{
    _field_tlv_t tlv;
    int i = 0;
    _field_control_t *fc = NULL;
    uint8 *ptr = NULL;
    uint32 *pos = NULL;
    uint32 *value = NULL;
    _field_entry_t *f_ent = NULL, *f_ent_part = NULL;
    int hw_index = -1, idx = 0;
    soc_mem_t mem, def_policy_mem;     /* Memory Identifier.        */
    _field_stage_t *stage_fc = NULL;
    uint32 tbuf[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                     /* Temp Buffer One.          */
    uint32 tbuftemp[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                     /* Temp Buffer Two.          */
    uint32 tbuftemp2[5] = {0};       /* Temp Buffer Three.        */
    uint32 *bufp;                    /* Hardware Buffer Ptr.      */
    exact_match_2_entry_t ebuf_nar;  /* Narrow Entry Buffer.      */
    exact_match_4_entry_t ebuf_wide; /* Wide Entry Buffer.        */
    soc_profile_mem_t *action_profile;/* action profile memory */
    int rv = 0, parts_count = 0;
    int qos_idx = 0;
    int act_idx = 0, class_id = 0;
    uint32 act_data[2] = {0};
    char *em_buffer = NULL;
    static soc_mem_t em_entry_nar_mem[_FP_MAX_NUM_PIPES] = {
        EXACT_MATCH_2_PIPE0m,
        EXACT_MATCH_2_PIPE1m,
        EXACT_MATCH_2_PIPE2m,
        EXACT_MATCH_2_PIPE3m,
    };
    static soc_mem_t em_entry_wide_mem[_FP_MAX_NUM_PIPES] = {
        EXACT_MATCH_4_PIPE0m,
        EXACT_MATCH_4_PIPE1m,
        EXACT_MATCH_4_PIPE2m,
        EXACT_MATCH_4_PIPE3m,
    };
     static soc_mem_t em_default_entry_mem[_FP_MAX_NUM_PIPES] = {
        EXACT_MATCH_DEFAULT_POLICY_PIPE0m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE1m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE2m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE3m,
    };
    tlv.value = NULL;
    TLV_INIT(tlv);
    WB_FIELD_CONTROL_GET(fc, ptr, pos);
    if (fg->group_status.entry_count == 0) {
        return BCM_E_NONE;
    }
    /* coverity[no_write_call : FALSE] */
    BCM_IF_ERROR_RETURN(tlv_read(unit, &tlv, ptr, pos));
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                       _BCM_FIELD_STAGE_EXACTMATCH, &stage_fc));

    if (tlv.type != _bcmFieldInternalEMEntryarr) {
        sal_free(tlv.value);
        return BCM_E_INTERNAL;
    }
    /* Get Hardware Buffer Pointer and memory identifier. */
    if ((fg->em_mode == _FieldExactMatchMode128) ||
        (fg->em_mode == _FieldExactMatchMode160)) {
        bufp = (uint32 *)&ebuf_nar;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_2m;
            def_policy_mem = EXACT_MATCH_DEFAULT_POLICYm;
        } else {
            mem = em_entry_nar_mem[fg->instance];
            def_policy_mem = em_default_entry_mem[fg->instance];
        }
    } else {
        bufp = (uint32 *)&ebuf_wide;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_4m;
            def_policy_mem = EXACT_MATCH_DEFAULT_POLICYm;
        } else {
            mem = em_entry_wide_mem[fg->instance];
            def_policy_mem = em_default_entry_mem[fg->instance];
        }
    }
    em_buffer = soc_cm_salloc(unit, SOC_MEM_TABLE_BYTES(unit, mem),
                                         "em table buffer");
    rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ALL,
                            soc_mem_index_min(unit, mem),
                            soc_mem_index_max(unit, mem),
                            em_buffer);
    BCM_IF_ERROR_RETURN(rv);

    rv = _bcm_field_entry_tcam_parts_count(unit, fg->stage_id,
                                           fg->flags, &parts_count);
    BCM_IF_ERROR_RETURN(rv);


    value = tlv.value;
    while (i < tlv.length) {
        if (fg->em_mode == _FieldExactMatchMode320) {
            f_ent = NULL;
            f_ent_part = NULL;
            _FP_XGS3_ALLOC(f_ent, 2 * sizeof(_field_entry_t), "em entry");
            if (f_ent == NULL) {
                rv = BCM_E_MEMORY;
                goto cleanup;
            }


            f_ent_part = f_ent+1;
            f_ent->eid = value[i++];
            f_ent_part->eid = f_ent->eid;
            /* _bcm_field_th_qual_tcam_key_mask_get allocates memory for
               tcam.key and tcam.mask, also populates tcam.key_size.
               This function also checks for group, and if installed flag
               is set, tries to read hw. so flags should be populated later.
               we save tcam.key, tcam.mask, and tcam.key_hw for each em entry.
               tcam.key may change in software but tcam.key_hw changes only
               when entry is written to hardware. */
            f_ent->group = fg;
            f_ent->fs = fg->slices;
            f_ent->slice_idx = -1;
            f_ent_part->group = fg;
            f_ent_part->fs = fg->slices;
            f_ent_part->slice_idx = -1;
            fg->entry_arr[idx++] = f_ent;
            _bcm_field_th_qual_tcam_key_mask_get(unit, f_ent, &f_ent->tcam);
            _bcm_field_th_qual_tcam_key_mask_get(unit, f_ent_part,
                                                          &f_ent_part->tcam);

            f_ent->flags =  value[i++];
            f_ent_part->flags = value[i++];
            _FP_XGS3_ALLOC(f_ent->tcam.key_hw, f_ent->tcam.key_size,
                                                            "em entry key");
            if (f_ent->tcam.key_hw == NULL) {
                rv = BCM_E_MEMORY;
                goto cleanup;
            }

            _FP_XGS3_ALLOC(f_ent_part->tcam.key_hw, f_ent->tcam.key_size,
                                                             "em entry key");
            if (f_ent_part->tcam.key_hw == NULL) {
                rv = BCM_E_MEMORY;
                goto cleanup;
            }

            if (value[i] != -1) {
                f_ent->statistic.sid = value[i];
                f_ent->statistic.flags |= _FP_ENTRY_STAT_VALID;
            }
            i += 1;
            if (value[i] != -1) {
                f_ent->policer[0].pid = value[i];
                f_ent->policer[0].flags |=
                                    _FP_POLICER_INSTALLED | _FP_POLICER_VALID;
            }

            rv = _field_entry_recover_policer_actual_hw_rates(unit, f_ent);
            BCM_IF_ERROR_CLEANUP(rv);

            i += 1;
            hw_index = value[i++];
            if (f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT) {
                _field_wb_em_defentry_recover(unit, f_ent, def_policy_mem);
                continue;
            }
            if (hw_index != -1) {
                bufp = soc_mem_table_idx_to_pointer(unit, mem, uint32 *,
                                    em_buffer, hw_index);
                soc_mem_field_get(unit, mem, bufp, MODE320__KEY_0_ONLYf, tbuf);
                soc_mem_field_get(unit, mem, bufp, MODE320__KEY_1_ONLYf,
                                                                     tbuftemp);
                /* using tbuftemp2 to copy the first 59 bits of tbuftemp to
                   tbuf. directly copying the data from tbuftemp to tbuf is
                   failing as th_val_set function will return E_PARAM error
                   if input buffer has valid bits after the 59th bit in it */
                rv = _bcm_field_qual_partial_data_get(tbuftemp, 0, 59, tbuftemp2);
                BCM_IF_ERROR_CLEANUP(rv);
                rv = _bcm_field_th_val_set(tbuf, tbuftemp2, 101, 59);
                BCM_IF_ERROR_CLEANUP(rv);
                sal_memcpy(f_ent->tcam.key_hw, tbuf, f_ent->tcam.key_size);
                sal_memcpy(f_ent->tcam.key, tbuf, f_ent->tcam.key_size);
                rv = _bcm_field_th_val_get(tbuftemp, f_ent_part->tcam.key_hw, 59, 42);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_get(unit, mem, bufp, MODE320__KEY_2_ONLYf, tbuf);
                rv = _bcm_field_th_val_set(f_ent_part->tcam.key_hw, tbuf, 42, 101);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_get(unit, mem, bufp, MODE320__KEY_3_ONLYf, tbuf);
                rv = _bcm_field_th_val_set(f_ent_part->tcam.key_hw, tbuf, 143, 17);
                BCM_IF_ERROR_CLEANUP(rv);
                sal_memcpy(f_ent_part->tcam.key, f_ent_part->tcam.key_hw,
                                                          f_ent->tcam.key_size);
                soc_mem_field_get(unit, mem, bufp, MODE320__ACTION_DATAf,
                                                          act_data);
                qos_idx = soc_mem_field32_get(unit, mem, bufp,
                                                      MODE320__QOS_PROFILE_IDf);
                act_idx = soc_mem_field32_get(unit, mem, bufp,
                                                   MODE320__ACTION_PROFILE_IDf);
                class_id = soc_mem_field32_get(unit, mem, bufp,
                                                MODE320__EXACT_MATCH_CLASS_IDf);
                fg->action_profile_idx[0] = act_idx;
                if (class_id) {
                    _field_em_class_action_add(unit, f_ent, class_id);
                }
            } else {
                continue;
            }
            /* EM match doesnot support bit masking */
            sal_memset(f_ent->tcam.mask, 0xff, f_ent->tcam.key_size);
            sal_memset(f_ent_part->tcam.mask, 0xff, f_ent->tcam.key_size);
            _field_wb_qos_actions_recover(unit, f_ent, qos_idx);
            _field_wb_em_actions_recover(unit, f_ent, act_idx, act_data);
            continue;
        }
        f_ent = NULL;
        _FP_XGS3_ALLOC(f_ent, sizeof(_field_entry_t), "em entry");
        if (f_ent == NULL) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        f_ent->eid = value[i++];
        /* similar order is followed. Check groupmode320 for details */
        f_ent->group = fg;
        f_ent->fs = fg->slices;
        f_ent->slice_idx = -1;
        fg->entry_arr[idx++] = f_ent;
        _bcm_field_th_qual_tcam_key_mask_get(unit, f_ent, &f_ent->tcam);
        f_ent->flags = value[i++];
        if (value[i] != -1) {
            f_ent->statistic.sid = value[i];
           f_ent->statistic.flags |= _FP_ENTRY_STAT_VALID;
        }
        i += 1;
        if (value[i] != -1) {
            f_ent->policer[0].pid = value[i];
            f_ent->policer[0].flags |=
                                _FP_POLICER_INSTALLED | _FP_POLICER_VALID;
        }

        rv = _field_entry_recover_policer_actual_hw_rates(unit, f_ent);
        BCM_IF_ERROR_CLEANUP(rv);

        i += 1;
        hw_index = value[i++];
        if (f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT) {
            _field_wb_em_defentry_recover(unit, f_ent, def_policy_mem);
            continue;
        }
        if (hw_index != -1) {
                bufp = soc_mem_table_idx_to_pointer(unit, mem, uint32 *,
                                    em_buffer, hw_index);
            if (fg->em_mode == _FieldExactMatchMode128) {
                soc_mem_field_get(unit, mem, bufp, MODE128__KEY_0_ONLYf, tbuf);

                soc_mem_field_get(unit, mem, bufp, MODE128__KEY_1_ONLYf, tbuftemp);
                rv = _bcm_field_th_val_set(tbuf, tbuftemp, 101, 27);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_get(unit, mem, bufp, MODE128__ACTION_DATAf, act_data);
                qos_idx = soc_mem_field32_get(unit, mem, bufp,
                                                      MODE128__QOS_PROFILE_IDf);
                act_idx = soc_mem_field32_get(unit, mem, bufp,
                                                   MODE128__ACTION_PROFILE_IDf);
                class_id = soc_mem_field32_get(unit, mem, bufp,
                                               MODE128__EXACT_MATCH_CLASS_IDf);
                fg->action_profile_idx[0] = act_idx;
            } else if (fg->em_mode == _FieldExactMatchMode160) {
                soc_mem_field_get(unit, mem, bufp, MODE160__KEY_0_ONLYf, tbuf);
                soc_mem_field_get(unit, mem, bufp, MODE160__KEY_1_ONLYf, tbuftemp);
                rv = _bcm_field_th_val_set(tbuf, tbuftemp, 101, 59);
                BCM_IF_ERROR_CLEANUP(rv);
                soc_mem_field_get(unit, mem, bufp, MODE160__ACTION_DATAf,
                                                                      act_data);
                qos_idx = soc_mem_field32_get(unit, mem, bufp,
                                                      MODE160__QOS_PROFILE_IDf);
                act_idx = soc_mem_field32_get(unit, mem, bufp,
                                                  MODE160__ACTION_PROFILE_IDf);
                class_id = soc_mem_field32_get(unit, mem, bufp,
                                                MODE160__EXACT_MATCH_CLASS_IDf);
                fg->action_profile_idx[0] = act_idx;
            }
        } else {
            continue;
        }
        if (class_id) {
            _field_em_class_action_add(unit, f_ent, class_id);
        }
        _FP_XGS3_ALLOC(f_ent->tcam.key_hw,
                       f_ent->tcam.key_size, "em entry key");

        if (f_ent->tcam.key_hw == NULL) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        sal_memcpy(f_ent->tcam.key_hw, tbuf, f_ent->tcam.key_size);
        sal_memcpy(f_ent->tcam.key, tbuf, f_ent->tcam.key_size);
        sal_memset(f_ent->tcam.mask, 0xff, f_ent->tcam.key_size);
        _field_wb_qos_actions_recover(unit, f_ent, qos_idx);
        _field_wb_em_actions_recover(unit, f_ent, act_idx, act_data);


    }

    action_profile = &stage_fc->action_profile[fg->instance];
    for (i =0; i < MAX_ACT_PROF_ENTRIES_PER_GROUP; i++) {
        if (-1 != fg->action_profile_idx[i]) {
            SOC_PROFILE_MEM_REFERENCE(unit, action_profile,
                                         fg->action_profile_idx[i], 1);
            SOC_PROFILE_MEM_ENTRIES_PER_SET(unit, action_profile,
                                         fg->action_profile_idx[i], 1);
        }
    }

    soc_cm_sfree(unit, em_buffer);
    return BCM_E_NONE;

cleanup:
    TLV_INIT(tlv);

    if (f_ent != NULL) {
        if (f_ent->tcam.key_hw != NULL) {
            sal_free(f_ent->tcam.key_hw);
            f_ent->tcam.key_hw = NULL;
        }
        if ((f_ent+1)->tcam.key_hw != NULL) {
            sal_free((f_ent+1)->tcam.key_hw);
            (f_ent+1)->tcam.key_hw = NULL;
        }
        sal_free(f_ent);
        f_ent = NULL;
    }
   return rv;
}

/*
 * Function:
 *      _bcm_field_th_stage_mixed_src_class_mode_recover
 * Purpose:
 *      Recover Mixed Src Class mode for Ingress and exact match stage
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      stage_fc - (IN) pointer to stage structure
 *   Returns:
 *      BCM_E_XXX
 */
int _bcm_field_th_stage_mixed_src_class_mode_recover(int unit,
                                     _field_stage_t *stage_fc)
{
    const soc_reg_t _bcm_field_th_ifp_lt_select_config_reg[] = {
        IFP_LOGICAL_TABLE_SELECT_CONFIG_PIPE0r,
        IFP_LOGICAL_TABLE_SELECT_CONFIG_PIPE1r,
        IFP_LOGICAL_TABLE_SELECT_CONFIG_PIPE2r,
        IFP_LOGICAL_TABLE_SELECT_CONFIG_PIPE3r};
    const soc_reg_t _bcm_field_th_em_lt_select_config_reg[] = {
        EXACT_MATCH_LOGICAL_TABLE_SELECT_CONFIG_PIPE0r,
        EXACT_MATCH_LOGICAL_TABLE_SELECT_CONFIG_PIPE1r,
        EXACT_MATCH_LOGICAL_TABLE_SELECT_CONFIG_PIPE2r,
        EXACT_MATCH_LOGICAL_TABLE_SELECT_CONFIG_PIPE3r};
    soc_reg_t                     lt_select_config;
    uint32                        mixed_src_class_mode;
    uint8                         pipe_idx = 0;

    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
            lt_select_config  = IFP_LOGICAL_TABLE_SELECT_CONFIGr;
        }
        else {
            lt_select_config  = EXACT_MATCH_LOGICAL_TABLE_SELECT_CONFIGr;
        }
        BCM_IF_ERROR_RETURN(soc_reg32_get(unit, lt_select_config,
                REG_PORT_ANY, 0, &mixed_src_class_mode));
        stage_fc->field_src_class_mode[pipe_idx] = soc_reg_field_get(unit,
                lt_select_config,
                mixed_src_class_mode,
                SOURCE_CLASS_MODEf);
    } else {
        for (pipe_idx = 0; pipe_idx < BCM_PIPES_MAX ; pipe_idx++) {
            if (stage_fc->stage_id == _BCM_FIELD_STAGE_INGRESS) {
                lt_select_config  = _bcm_field_th_ifp_lt_select_config_reg[pipe_idx];
            }
            else {
                lt_select_config  = _bcm_field_th_em_lt_select_config_reg[pipe_idx];
            }
            BCM_IF_ERROR_RETURN(soc_reg32_get(unit, lt_select_config,
                    REG_PORT_ANY, 0, &mixed_src_class_mode));
            stage_fc->field_src_class_mode[pipe_idx] = soc_reg_field_get(unit,
                    lt_select_config,
                    mixed_src_class_mode,
                    SOURCE_CLASS_MODEf);
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_field_th_stage_em_reinit
 * Purpose:
 *      Main reinit function for exact match stage
 *
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      fc   - (IN) _field_control_t structure
 *      stage_fc - (IN) pointer to stage structure
 *   Returns:
 *      BCM_E_XXX
 */
int
_bcm_field_th_stage_em_reinit(int unit, _field_control_t *fc,
                                    _field_stage_t *stage_fc)
{
    _field_tlv_t tlv;
    uint8 *ptr = NULL;
    uint32 *pos = NULL;
    int rv  = BCM_E_NONE;
    int pipe_id = 0, num_pipes = 0;
    _field_slice_t *curr_slice = NULL;
    _field_lt_slice_t *curr_ltslice = NULL;
    int slice_id = 0, lt_id = 0, i = 0;
    int group_count = 0, group_id = 0;
    _field_group_t *curr_fg = NULL;
    _field_group_t **new_fg = NULL;
    int max_grp_id = 0;
    _field_lt_entry_t *f_lt_ent = NULL, *f_lt_ent_part = NULL;
    uint32 flags[3];
    int part_index = 0, pri_tcam_idx = 0, slice_number = 0;

    tlv.value = NULL;
    if ((fc == NULL) || (stage_fc == NULL)) {
        return BCM_E_PARAM;
    }
    TLV_INIT(tlv);

    WB_FIELD_CONTROL_GET_WITH_LOCK(fc, ptr, pos);
    BCM_IF_ERROR_CLEANUP(_field_scache_stage_hdr_chk(fc, _FIELD_EM_DATA_START));

    while (tlv.type != _bcmFieldInternalEndStageEM) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_CLEANUP(tlv_read(unit, &tlv, ptr, pos));

        switch (tlv.type) {

         case _bcmFieldInternalEMOperMode:
             if (*(bcm_field_group_oper_mode_t *)tlv.value) {
                 BCM_IF_ERROR_CLEANUP(_bcm_field_wb_group_oper_mode_set(unit,
                                    bcmFieldQualifyStageIngressExactMatch,
                                    &stage_fc,
                                    bcmFieldGroupOperModePipeLocal));
                 BCM_IF_ERROR_CLEANUP(_field_stage_control_get(unit,
                                     _BCM_FIELD_STAGE_EXACTMATCH,
                                     &stage_fc));
                 num_pipes = NUM_PIPE(unit);

             } else {
                 num_pipes = 1;
             }
             curr_stage_fc = stage_fc;
             break;
        case _bcmFieldInternalSlice:
             /* _field_slice_t and _field_lt_slice_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"EM:: FP(unit %d):"
                                                 "_bcm_field_th_stage"
                                                 "_em_reinit -"
                                                 "recovering _field_slice_t,"
                                                 "_field_lt_slice_t "
                                                 "from pos = %d\r\n"),
                                                 unit,fc->scache_pos));

             for (pipe_id = 0; pipe_id < num_pipes; pipe_id++) {
                 curr_slice = stage_fc->slices[pipe_id];
                 curr_ltslice = stage_fc->lt_slices[pipe_id];

                 if (curr_slice != NULL) {
                     for (slice_id= 0; slice_id < stage_fc->tcam_slices;
                         slice_id ++) {

                          rv = _field_slice_recover(unit, pipe_id,
                                                   (curr_slice + slice_id));
                          BCM_IF_ERROR_CLEANUP(rv);
                     }
                 } else {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                               "Memory not allocated for slice\n")));
                 }

                 if (curr_ltslice != NULL) {
                     for (lt_id = 0; lt_id < stage_fc->tcam_slices; lt_id++) {
                          rv = _field_lt_slice_recover(unit, pipe_id,
                                                        (curr_ltslice
                                                        + lt_id));
                          BCM_IF_ERROR_CLEANUP(rv);
                     }
                 } else {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                               "Memory not allocated for LT slice\n")));
                 }

             }
             break;
         case _bcmFieldInternalGroupCount:
             group_count = *(int *)tlv.value;
             for (group_id =0 ;group_id < group_count; group_id ++) {
                 curr_fg = NULL;
                 _FP_XGS3_ALLOC(curr_fg, sizeof(_field_group_t),
                                "current field grp");
                 if (curr_fg== NULL) {
                     rv = BCM_E_MEMORY;
                     goto cleanup;
                 }
                 rv = _field_group_recover(unit, curr_fg);
                 BCM_IF_ERROR_CLEANUP(rv);
                 rv = _field_em_group_entries_recover(unit, curr_fg);
                 BCM_IF_ERROR_CLEANUP(rv);
                 for (i=0; i < curr_fg->lt_grp_status.entry_count; i++) {
                     f_lt_ent = NULL;
                     if (curr_fg->em_mode ==  _FieldExactMatchMode320) {
                         _FP_XGS3_ALLOC(f_lt_ent, 2 *sizeof(_field_lt_entry_t),
                                                  "em ltentry");
                     } else {
                         _FP_XGS3_ALLOC(f_lt_ent, sizeof(_field_lt_entry_t),
                                                        "em ltentry");
                     }

                     if (f_lt_ent == NULL) {
                         rv = BCM_E_MEMORY;
                         goto cleanup;
                     }
                     curr_fg->lt_entry_arr[i] = f_lt_ent;
                     f_lt_ent->group = curr_fg;
                     f_lt_ent_part = f_lt_ent + 1;
                    /* Recover entry */
                    rv =_field_lt_entry_info_recover(unit, f_lt_ent,
                                                flags, NULL);
                    BCM_IF_ERROR_CLEANUP(rv);
                    if (curr_fg->em_mode ==  _FieldExactMatchMode320) {
                        sal_memcpy((f_lt_ent_part), f_lt_ent, sizeof(_field_lt_entry_t));
                        (f_lt_ent_part)->flags = flags[1];
                         rv = _bcm_field_th_lt_slice_offset_to_tcam_index(unit,
                                            stage_fc, f_lt_ent->group->instance,
                                                f_lt_ent->lt_fs->slice_number,
                                               f_lt_ent->index, &pri_tcam_idx);
                            BCM_IF_ERROR_CLEANUP(rv);

                            rv = _bcm_field_th_lt_entry_part_tcam_idx_get(unit,
                                            f_lt_ent->group, pri_tcam_idx,
                                                          1, &part_index);
                            BCM_IF_ERROR_CLEANUP(rv);
                            rv = _bcm_field_th_lt_tcam_idx_to_slice_offset(unit,
                                       stage_fc, f_lt_ent->group->instance,
                                         part_index,
                                         &slice_number,(int *)&f_lt_ent->index);
                            BCM_IF_ERROR_CLEANUP(rv);
                            f_lt_ent_part->lt_fs =
                                 stage_fc->lt_slices[f_lt_ent->group->instance]
                                                    + slice_number;

                        }


                 }
                 curr_fg->next = fc->groups;
                 max_grp_id =
                         ((max_grp_id > (curr_fg->gid))
                          ? max_grp_id:(curr_fg->gid));
                 fc->groups = curr_fg;

             }

             f_lt_ent = NULL;

             /* Group id's range from 0 to max_grp_id */
             _FP_XGS3_ALLOC(new_fg, (max_grp_id+1) *sizeof(_field_group_t *),
                            "group alloc");
             if (new_fg == NULL) {
                 rv = BCM_E_MEMORY;
                 goto cleanup;
             }

             curr_fg = fc->groups;
             while ((curr_fg != NULL)
                     && (curr_fg->stage_id ==_BCM_FIELD_STAGE_EXACTMATCH)) {
                 new_fg[(curr_fg->gid)] = curr_fg;
                 curr_fg = curr_fg->next;
             }
             curr_fg = NULL;
             break;
        case _bcmFieldInternalControlPreselInfo:
             /* _field_presel_entry_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                                 "_bcm_field_th_stage"
                                                 "_em_reinit -"
                                                 "recovering _field_presel_"
                                                 "entry_t from pos = %d\r\n"),
                                                 unit,
                                                 fc->scache_pos));
             /* coverity[var_deref_model] */
             rv = _field_presel_entry_recover(unit, &tlv, new_fg);
             BCM_IF_ERROR_CLEANUP(rv);
             break;

         case _bcmFieldInternalEndStageEM:
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                      "_bcm_field_th_stage_em_reinit -"
                                      "at pos = %d\r\n"),unit, fc->scache_pos));
             break;
         default :
             break;
        }

    }
    _field_wb_qos_buffer_free(unit);
    _field_wb_em_act_buffer_free(unit);
    rv = _field_scache_stage_hdr_chk(fc, _FIELD_EM_DATA_END);
    BCM_IF_ERROR_CLEANUP(rv);
    rv = _bcm_field_th_stage_mixed_src_class_mode_recover(unit, stage_fc);

cleanup:
    TLV_INIT(tlv);

    if (new_fg != NULL) {
        sal_free(new_fg);
    }
    if (curr_fg != NULL) {
       sal_free(curr_fg);
    }
    if (f_lt_ent != NULL) {
        sal_free(f_lt_ent);
    }
    FP_UNLOCK(unit);
    return rv;

}


void
_bcm_field_th_tlv_recovery_map_free(int unit) {
    if (downgrade[unit]  == TRUE) {
        if (recovery_type_map[unit] != NULL) {
            sal_free(recovery_type_map[unit]);
        }
    }
    downgrade[unit] = FALSE;
}

int
_bcm_field_th_tlv_recovery_map_alloc(int unit, uint32 enum_total_count) {
    downgrade[unit] = TRUE;
    _FP_XGS3_ALLOC(recovery_type_map[unit], (sizeof (_field_type_map_t)
                   * enum_total_count),
                   "enum type map");
    if (recovery_type_map[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_field_th_stage_ingress_reinit
 * Description:
 *  Service routine used to retain the software sate of
 *  IFP(Ingress Field Processor) configuration .
 * Parameters:
 *   unit     - (IN) BCM device number.
 *   fc       - (IN) Field control structure.
 *   stage_fc - (IN/OUT) Stage field control structure.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  None.
 */

int
_bcm_field_th_stage_ingress_reinit(int unit,
                                   _field_control_t *fc,
                                   _field_stage_t   *stage_fc)
{
    int    rv = BCM_E_NONE;                 /* Operation return value  */
    uint8  *scache_ptr;                     /* Pointer to 1st scache part*/
    uint32 temp;                            /* Temporary variable   */
    _field_group_t **new_fg,*curr_fg, *prev_fg;
                                            /* For Group LL */
    uint32 slice_id,pipe_id,lt_id,group_id; /* Iterator varaiables */
    struct _field_slice_s *curr_slice;      /* Slice information */
    struct _field_lt_slice_s *curr_ltslice; /* LT slice information */
    _field_tlv_t tlv;                       /* TLV data */
    int  num_pipes = 0;                     /* Number of pipes */
    int group_count = 0, max_grp_id = 0;
    uint32 enum_properties_type = 0;
                                            /* Type for Enum Properties */
    uint32 enum_total_count = 0;
                                            /* Count of Enums */
    uint32 *enum_properties = NULL;
                                            /* Enum properties */
    uint32 prop_pos = 0;                    /* To index Enum Properties array */
    uint32 enum_count = 0;                  /* Enum index Variable */

    tlv.value = NULL;
    new_fg = NULL;
    prev_fg = curr_fg = NULL;

    FP_LOCK(unit);
    TLV_INIT(tlv);
    scache_ptr = fc->scache_ptr[_FIELD_SCACHE_PART_0];

    fc->scache_pos = 0;
    fc->scache_pos += SOC_WB_SCACHE_CONTROL_SIZE;

    sal_memcpy(&enum_properties_type, &scache_ptr[fc->scache_pos],
                sizeof(uint32));
    fc->scache_pos += sizeof(uint32);


    if (enum_properties_type == BCM_FIELD_TYPE_ENUM_PROPERTIES) {
        _field_type_map_t temp_type_map;

        sal_memcpy(&enum_total_count, &scache_ptr[fc->scache_pos], sizeof(uint32));
        fc->scache_pos+= sizeof(uint32);

        _FP_XGS3_ALLOC(enum_properties, (sizeof(uint32) * 3 *
                       enum_total_count),
                       "enum properties");
        if (enum_properties == NULL) {
            rv = BCM_E_MEMORY;
            goto cleanup;
        }

        rv = _bcm_field_th_tlv_recovery_map_alloc(unit, enum_total_count);
        if (BCM_FAILURE(rv)) {
            goto cleanup;
        }

        sal_memcpy(enum_properties, &scache_ptr[fc->scache_pos],
                   ((3 * sizeof(uint32)) * enum_total_count));
        fc->scache_pos += ((3 * sizeof(uint32)) * enum_total_count);

        prop_pos = 0;
        for (enum_count = 0; enum_count < enum_total_count; enum_count++) {
            sal_memset(&temp_type_map, 0x0, sizeof(_field_type_map_t));
            temp_type_map.element = enum_properties[prop_pos++];
            temp_type_map.size = enum_properties[prop_pos++];
            temp_type_map.flags = enum_properties[prop_pos++];
            sal_memcpy(&recovery_type_map[unit][enum_count], &temp_type_map,
                       sizeof(_field_type_map_t));
        }
    } else {
        recovery_type_map[unit] = (_field_type_map_t *)&static_type_map;
        fc->scache_pos -= sizeof(uint32);
    }

    while (tlv.type != _bcmFieldInternalEndStructIFP) {
        TLV_INIT(tlv);
        /* coverity[no_write_call : FALSE] */
        BCM_IF_ERROR_CLEANUP(tlv_read(unit, &tlv, scache_ptr,
                                     &(fc->scache_pos)));


    switch (tlv.type) {
        case _bcmFieldInternalControl:
             /* _field_control_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                                 "_bcm_field_th_stage"
                                                 "_ingress_reinit"
                                                 " recovering _field_control_t "
                                                 " from pos = %d\r\n"),unit,
                                                 fc->scache_pos));
             rv = _field_control_recover(unit,fc);
             BCM_IF_ERROR_CLEANUP(rv);
             break;
        case _bcmFieldInternalStage:
             /*_field_stage_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                                 "_bcm_field_th_stage"
                                                 "_ingress_reinit"
                                                 " recovering _field_stage_t "
                                                 " from pos = %d\r\n"),unit,
                                                 fc->scache_pos));
             rv = _field_stage_recover(unit,&stage_fc);
             BCM_IF_ERROR_CLEANUP(rv);
             if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
                 num_pipes = 1;
             } else if (stage_fc->oper_mode == bcmFieldGroupOperModePipeLocal) {
                 num_pipes = (_FP_MAX_NUM_PIPES);
             }
             curr_stage_fc = stage_fc;
             break;
        case _bcmFieldInternalSlice:
             /* _field_slice_t and _field_lt_slice_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                                 "_bcm_field_th_stage"
                                                 "_ingress_reinit -"
                                                 "recovering _field_slice_t,"
                                                 "_field_lt_slice_t "
                                                 "from pos = %d\r\n"),
                                                 unit,fc->scache_pos));

             for (pipe_id = 0; pipe_id < num_pipes; pipe_id++) {
                 curr_slice = stage_fc->slices[pipe_id];
                 curr_ltslice = stage_fc->lt_slices[pipe_id];

                 if (curr_slice != NULL) {
                     for (slice_id= 0; slice_id < stage_fc->tcam_slices;
                         slice_id ++) {

                          rv = _field_slice_recover(unit, pipe_id,
                                                   (curr_slice + slice_id));
                          BCM_IF_ERROR_CLEANUP(rv);
                     }
                 } else {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                               "Memory not allocated for slice\n")));
                 }




                 if (curr_ltslice != NULL) {
                     for (lt_id = 0; lt_id < stage_fc->tcam_slices; lt_id++) {
                          rv = _field_lt_slice_recover(unit, pipe_id,
                                                        (curr_ltslice
                                                        + lt_id));
                          BCM_IF_ERROR_CLEANUP(rv);
                     }
                 }else {
                     LOG_ERROR(BSL_LS_BCM_FP,(BSL_META_U(unit,
                               "Memory not allocated for LT slice\n")));
                 }

             }
             break;
        case _bcmFieldInternalGroupCount:
             /* _field_group_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                                 "_bcm_field_th_stage"
                                                 "_ingress_reinit -"
                                                 "recovering _field_group_t "
                                                 "from pos = %d\r\n"),unit,
                                                 fc->scache_pos));
             group_count = *(int *)tlv.value;

             /* Iterate over the groups */
             for (group_id =0 ;group_id < group_count; group_id ++) {
                 curr_fg = NULL;
                 _FP_XGS3_ALLOC(curr_fg, sizeof(_field_group_t),
                                "current field grp");
                 if (curr_fg == NULL) {
                     rv = BCM_E_MEMORY;
                     goto cleanup;
                 }

                 rv = _field_group_recover(unit, curr_fg);
                 BCM_IF_ERROR_CLEANUP(rv);

                 if (prev_fg == NULL) {
                     prev_fg = curr_fg;
                     fc->groups = prev_fg;
                     max_grp_id = curr_fg->gid;
                 } else {
                     prev_fg->next = curr_fg;
                     prev_fg = curr_fg;
                     max_grp_id =
                         ((max_grp_id > (curr_fg->gid))
                          ? max_grp_id:(curr_fg->gid));
                 }
             }
             /* Group id's range from 0 to max_grp_id */
             _FP_XGS3_ALLOC(new_fg, (max_grp_id+1) *sizeof(_field_group_t *),
                            "group alloc");
             if (new_fg == NULL) {
                 rv = BCM_E_MEMORY;
                 goto cleanup;
             }

             curr_fg = fc->groups;
             while (curr_fg != NULL) {
                 new_fg[(curr_fg->gid)] = curr_fg;
                 curr_fg = curr_fg->next;
             }
             curr_fg = NULL;
             /* _field_entry_t and _field_lt_entry_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                      "_bcm_field_th_stage_ingress_reinit -"
                                      "recovering _field_entry_t,"
                                      "_field_lt_entry_t from pos = %d\r\n"),
                                                 unit,fc->scache_pos));

             rv = _field_entry_recover(unit, new_fg);
             BCM_IF_ERROR_CLEANUP(rv);
             break;

        case _bcmFieldInternalControlPreselInfo:
             /* _field_presel_entry_t */
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                                 "_bcm_field_th_stage"
                                                 "_ingress_reinit -"
                                                 "recovering _field_presel_"
                                                 "entry_t from pos = %d\r\n"),
                                                 unit,
                                                 fc->scache_pos));
             /* coverity[var_deref_model] */
             rv = _field_presel_entry_recover(unit, &tlv, new_fg);
             BCM_IF_ERROR_CLEANUP(rv);
             break;

        case _bcmFieldInternalEndStructIFP:
             LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                      "_bcm_field_th_stage_ingress_reinit -"
                                      "End of structures recover "
                                      "at pos = %d\r\n"),unit, fc->scache_pos));
             break;
        default:
             break;
        }
    }


    temp = 0;
    temp |= scache_ptr[fc->scache_pos];
    fc->scache_pos++;
    temp |= scache_ptr[fc->scache_pos] << 8;
    fc->scache_pos++;
    temp |= scache_ptr[fc->scache_pos] << 16;
    fc->scache_pos++;
    temp |= scache_ptr[fc->scache_pos] << 24;
    fc->scache_pos++;
    if (temp != _FIELD_IFP_DATA_END) {
        fc->l2warm = 0;
        rv = BCM_E_INTERNAL;
    }
    LOG_DEBUG(BSL_LS_BCM_FP,(BSL_META_U(unit,"FP(unit %d):"
                                        "_bcm_field_th_stage_ingress_reinit -"
                                        "End of IFP Section "
                                        "at pos = %d\r\n"),unit,
                                        fc->scache_pos));
    if ((dynamic_map.entry_map != NULL) ||
       (dynamic_map.lt_entry_map != NULL)) {
        rv =_field_backlink_data(unit, stage_fc, max_grp_id);
    }

    BCM_IF_ERROR_CLEANUP(rv);
    rv = _bcm_field_th_stage_mixed_src_class_mode_recover(unit, stage_fc);
    BCM_IF_ERROR_CLEANUP(rv);

cleanup:
    TLV_INIT(tlv);
    FP_UNLOCK(unit);
    if (new_fg != NULL) {
        sal_free(new_fg);
    }
    if (curr_fg != NULL) {
        sal_free(curr_fg);
    }
    if (enum_properties != NULL) {
        sal_free(enum_properties);
    }

    dynamic_map_free(dynamic_map);

    return rv;
}
/*
 * Function:
 *      _field_wb_action_profile_parse
 * Purpose:
 *      Parse the entry read from action profile table and figure out
 *      which action sets are enabled. Update the f_ent structure
 *      with data in hardware. It is a common function for TH EM,
 *      TD3 IFP and TD3 EM.
 * Parameters:
 *      unit     - (IN) StrataSwitch unit #.
 *      f_ent    - (IN) entry which points to this action profile
 *      ebuf     - (IN) action profile table entry
 *      act_data - (IN) action data part in the entry
 *   Returns:
 *      BCM_E_XXX
 */
    int
_field_wb_action_profile_parse(int unit, _field_entry_t *f_ent,
                                uint32 *action_set_bitmap, uint32 *act_data,
                                _field_action_bmp_t *act_bmp)
{
    int idx;
    _field_stage_t *stage_fc = NULL;
                     /* pointer to stage structure */
    uint32 act_valid =0;
                     /* Action valid flag */
    _bcm_field_action_set_t   *action_set_ptr;
                     /* action set pointer */
    uint8 *hw_bitmap;
                     /* hw bitmap as per Action profile table */
    uint8 hw_bitmap_entries = 0;
                     /* number of entries in hw bitmap */
    uint32 tbuf[2] = {0};
    uint32 offset = 0;

    /* Function pointer to call the recovery function for each
       action set */
    int (*aset_recovery_func_ptr)(int unit, _field_entry_t *f_ent,
            uint32 *act_data, _field_action_bmp_t *act_bmp);

    /* stage control gey */
    BCM_IF_ERROR_RETURN(_field_stage_control_get (unit,
                f_ent->fs->stage_id, &stage_fc));

    /*Action_set bitmap*/
    /*Retrieve action set pointer from stage structure */
    action_set_ptr = stage_fc->action_set_ptr;
    /* depending on the stage, get the hw bitmap and
       number of entries in action profile bitmap */
    if (f_ent->group->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
        /* Assign Exact Match Action Profile Memory */
        if (SOC_IS_TOMAHAWK(unit)) {
            hw_bitmap = em_action_profile_bitmap;
            hw_bitmap_entries =
                sizeof(em_action_profile_bitmap)/sizeof(uint8);
        } else if (SOC_IS_TOMAHAWK2(unit)) {
            hw_bitmap = em_th2_action_profile_bitmap;
            hw_bitmap_entries =
                sizeof(em_th2_action_profile_bitmap)/sizeof(uint8);          
        } else {
            hw_bitmap = em_td3_action_profile_bitmap;
            hw_bitmap_entries =
                sizeof(em_td3_action_profile_bitmap)/sizeof(uint8);
        }
    } else {
        hw_bitmap = ifp_td3_action_profile_bitmap;
        hw_bitmap_entries =
            sizeof(ifp_td3_action_profile_bitmap)/sizeof(uint8);
        if (!(f_ent->group->flags & _FP_GROUP_SPAN_SINGLE_SLICE)
             || (f_ent->group->flags & _FP_GROUP_INTRASLICE_DOUBLEWIDE)) {
            offset = _FP_ASET_DATA_SIZE_FOR_POLICY_WIDE;
        } else {
            offset = _FP_ASET_DATA_SIZE_FOR_POLICY_NARROW;
        }
    }
    /* Starting from bit0 to bitn (n = number of entries in bitmap)
       1) check whether the bit is 1.
       2) if the bit is 1, get the corresponing Action set from hw_bitmap array.
       3) get the size of the action set say width from action set pointer.
       4) get the action set data, size equal to width from the policy data.
       5) if the Action set data is non Zero value, call the corresponding
          action set's recovery function.
       6) shift the policy data index to width.
       7) repeat steps from 1 to 6 for all entries in hw bitmap.
    */
#if defined(BCM_TRIDENT3_SUPPORT)
    if(soc_feature(unit, soc_feature_ifp_action_profiling)) {
        for (idx=hw_bitmap_entries-1; idx >=0; idx--) {
            BCM_IF_ERROR_RETURN(_bcm_field_th_val_get(action_set_bitmap,
                                                  &act_valid, idx, 1));
            if (act_valid == 1) {
                /*idx Bit is set in the action profile */
                tbuf[0] = 0;
                tbuf[1] = 0;
                _bcm_field_th_val_get(act_data, tbuf,
                                 offset - action_set_ptr[hw_bitmap[idx]].size,
                                 action_set_ptr[hw_bitmap[idx]].size);
                if ((tbuf[0] !=0 || tbuf[1] !=0) &&
                    (_field_wb_action_recovery_func_ptr[hw_bitmap[idx]] != NULL)) {
                    aset_recovery_func_ptr =
                             (_field_wb_action_recovery_func_ptr[hw_bitmap[idx]]);
                    BCM_IF_ERROR_RETURN((*aset_recovery_func_ptr)
                                       (unit, f_ent, tbuf, act_bmp));
                }
	        offset -=  action_set_ptr[hw_bitmap[idx]].size;
            }
        }
    } else 
#endif
    {
        for (idx=0; idx < hw_bitmap_entries;idx++)
        {
            BCM_IF_ERROR_RETURN(_bcm_field_th_val_get(action_set_bitmap,
                                                  &act_valid, idx, 1));
            if (act_valid == 1) {
                /*idx Bit is set in the action profile */
                tbuf[0] = 0;
                tbuf[1] = 0;
                BCM_IF_ERROR_RETURN(_bcm_field_th_val_get(act_data, tbuf,
                                 offset,
                                 action_set_ptr[hw_bitmap[idx]].size));
                if ((tbuf[0] !=0 || tbuf[1] !=0) &&
                    (_field_wb_action_recovery_func_ptr[hw_bitmap[idx]] != NULL)) {
                    aset_recovery_func_ptr =
                             (_field_wb_action_recovery_func_ptr[hw_bitmap[idx]]);
                    BCM_IF_ERROR_RETURN((*aset_recovery_func_ptr)
                                       (unit, f_ent, tbuf, act_bmp));
                }
	        offset +=  action_set_ptr[hw_bitmap[idx]].size;
            }
        }
    }
    return BCM_E_NONE;
}

#endif /* BCM_TOMAHAWK_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT */
