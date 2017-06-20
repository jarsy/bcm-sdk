/*
 * $Id: igmp.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     igmp.h
 * Purpose:
 *
 */
#ifndef _BCM_EA_IGMP_H
#define _BCM_EA_IGMP_H

#include <soc/ea/tk371x/oam.h>
#include <soc/ea/tk371x/TkIgmpApi.h>

typedef OamIgmpConfig 		_soc_ea_oam_igmp_config_t;
typedef OamIgmpGroupInfo	_soc_ea_oam_igmp_group_info_t;
typedef OamIgmpGroupConfig 	_soc_ea_oam_igmp_group_config_t;
typedef IgmpVlanCfg			_soc_ea_igmp_vlan_cfg_t;
typedef IgmpVlanRecord 		_soc_ea_igmp_vlan_record_t;

#define _soc_ea_igmp_config_set		TkExtOamSetIgmpConfig
#define _soc_ea_igmp_config_get		TkExtOamGetIgmpConfig
#define _soc_ea_igmp_group_info		TkExtOamGetIgmpGroupInfo
#define _soc_ea_igmp_group_del_set	TkExtOamSetDelIgmpGroup
#define _soc_ea_igmp_group_add_set	TkExtOamSetAddIgmpGroup
#define _soc_ea_igmp_vlan_set		TkExtOamSetIgmpVlan
#define _soc_ea_igmp_vlan_get		TkExtOamGetIgmpVlan

#endif /* _BCM_EA_HAL_IGMP_H */
