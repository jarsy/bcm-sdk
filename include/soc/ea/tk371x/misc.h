/*
 * $Id: misc.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     misc.h
 * Purpose:
 *
 */

#ifndef _SOC_EA_MISC_H
#define _SOC_EA_MISC_H

#include <soc/ea/tk371x/CtcMiscApi.h>

typedef CtcInfoFromONUPONChipsSet
								_soc_ea_ctc_info_from_onu_pon_chip_set_t;
typedef CtcExtRuleCondtion		_soc_ea_ctc_ext_rule_condtion_t;
typedef CtcExtRule				_soc_ea_ctc_ext_rule_t;
typedef CtcExtQConfig			_soc_ea_ctc_ext_qconfig_t;
typedef CtcExtONUTxPowerSupplyControl
								_soc_ea_ctc_ext_onu_tx_power_supply_control_t;

#define _soc_ea_ctc_onu_chipset_info_clear	TKCTCClearONUPONChipsSetInfo
#define	_soc_ea_ctc_mac_for_sn_fill			TKCTCExtOamFillMacForSN
#define _soc_ea_ctc_info_from_onu_pon_chipsets_get \
										TKCTCExtOamGetInfoFromONUPONChipsets
#define _soc_ea_ctc_dba_cfg_get			TKCTCExtOamGetDbaCfg
#define _soc_ea_ctc_dba_cfg_set         TKCTCExtOamSetDbaCfg
#define _soc_ea_ctc_fec_ability_get		TKCTCExtOamGetFecAbility
#define _soc_ea_ctc_oam_none_obj_raw_get	TKCTCExtOamNoneObjGetRaw
#define _soc_ea_ctc_oam_none_obj_raw_set	TKCTCExtOamNoneObjSetRaw
#define _soc_ea_ctc_oam_obj_raw_get			TKCTCExtOamObjGetRaw
#define _soc_ea_ctc_oam_obj_raw_set			TKCTCExtOamObjSetRaw
#define _soc_ea_ctc_onu_sn_get				TKCTCExtOamGetONUSN
#define _soc_ea_ctc_firmware_version_get	TKCTCExtOamGetFirmwareVersion
#define _soc_ea_ctc_chipset_id_get			TKCTCExtOamGetChipsetId
#define _soc_ea_ctc_onu_cap_get				TKCTCExtOamGetONUCap
#define _soc_ea_ctc_onu_cap2_get			TKCTCExtOamGetONUCap2
#define _soc_ea_ctc_holdover_config_get		TKCTCExtOamGetHoldOverConfig
#define _soc_ea_ctc_cls_marking_set			TKCTCExtOamSetClsMarking
#define _soc_ea_ctc_llid_queue_config_set	TKCTCExtOamSetLLIDQueueConfig
#define _soc_ea_ctc_laser_tx_power_admin_ctl_set	\
										TKCTCExtOamSetLaserTxPowerAdminCtl
#define _soc_ea_ctc_mllid_set           CtcExtOamSetMulLlidCtrl

#endif /* _SOC_EA_MISC_H */
