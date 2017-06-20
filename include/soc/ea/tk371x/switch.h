/*
 * $Id: switch.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     hal_swtich.h
 * Purpose:
 *
 */
#ifndef _SOC_EA_HAL_SWITCH_H
#define _SOC_EA_HAL_SWITCH_H

#include <soc/ea/tk371x/TkExtSwitchApi.h>

typedef OamUniVlanPolicy		_soc_ea_oam_uni_vlan_policy_t;
typedef OamVlanTagBehave		_soc_ea_oam_vlan_tag_behave_t;
typedef OamVlanPolicyCfg		_soc_ea_oam_vlan_policy_cfg_t;
typedef OamVlanMemberCfg		_soc_ea_oam_vlan_memeber_cfg_t;
typedef OamVlanCfgTlv			_soc_ea_oam_vlan_cfg_tlv_t;

#define _soc_ea_port_mirror_set			TkExtOamSetPortMirror
#define _soc_ea_port_mirror_get			TkExtOamGetPortMirror
#define _soc_ea_local_switch_get		TkExtOamGetLocalSwitch
#define _soc_ea_local_switch_set		TkExtOamSetLocalSwitch
#define _soc_ea_port_vlan_policy_set	TkExtOamSetPortVlanPolicy
#define _soc_ea_port_vlan_policy_get	TkExtOamGetPortVlanPolicy
#define _soc_ea_port_vlan_member_ship_set \
										TkExtOamSetPortVlanMemberShip
#define _soc_ea_port_vlan_member_ship_get \
										TkExtOamGetPortVlanMemberShip


#endif /* _SOC_EA_HAL_SWITCH_H */
