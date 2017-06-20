/*
 * $Id: brg.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     brg.h
 * Purpose:
 *
 */

#ifndef _SOC_EA_BRG_H
#define _SOC_EA_BRG_H

#include <soc/ea/tk371x/TkBrgApi.h>
#include <soc/ea/tk371x/TkExtSwitchApi.h>
#include <soc/ea/tk371x/Oam.h>

typedef LoopbackState soc_port_loopback_state_t;

typedef TagPortMirrorCfg 	_soc_ea_tag_port_mirror_cfg_t;
typedef FlushMacType		_soc_ea_flush_mac_type_t;
typedef ArlLearnStatus		_soc_ea_arl_learn_status_t;
typedef ForwardingMode		_soc_ea_fowarding_mode_t;
typedef OamAutoNegAdminState _soc_ea_oam_auto_neg_admin_state_t;
typedef OamMacDuplexStatus	_soc_ea_oam_mac_duplex_status_t;


#define _soc_ea_dyna_mac_tab_size_get	TkExtOamGetDynaMacTabSizeNew
#define _soc_ea_dyna_mac_tab_size_set	TkExtOamSetDynaMacTabSizeNew
#define _soc_ea_dyna_mac_tab_age_get	TkExtOamGetDynaMacTabAge
#define _soc_ea_dyna_mac_tab_age_set	TkExtOamSetDynaMacTabAge
#define _soc_ea_dyna_mac_entries_get	TkExtOamGetDynaMacEntries
#define _soc_ea_dyna_mac_table_clr_set	TkExtOamSetClrDynaMacTable
#define _soc_ea_static_mac_entries_get	TkExtOamGetStaticMacEntries
#define _soc_ea_static_mac_entry_add	TkExtOamAddStaticMacEntry
#define _soc_ea_static_mac_entry_del	TkExtOamDelStaticMacEntry
#define	_soc_ea_mac_table_flush			TkExtOamFlushMacTableNew
#define _soc_ea_mac_learning_set		TkExtOamSetMacLearning
#define _soc_ea_forward_mode_set		TkExtOamSetForwardMode
#define _soc_ea_auto_neg_get			TkExtOamGetAutoNeg
#define _soc_ea_auto_neg_set			TkExtOamSetAutoNeg
#define _soc_ea_mtu_set					TkExtOamSetMtu
#define _soc_ea_mtu_get					TkExtOamGetMtu
#define _soc_ea_flood_unknown_set		TkExtOamSetFloodUnknown
#define _soc_ea_flood_unknown_get		TkExtOamGetFloodUnknown
#define _soc_ea_mac_learn_switch_set	TkExtOamSetMacLearnSwitch
#define _soc_ea_mac_learn_switch_get	TkExtOamGetMacLearnSwitch
#define _soc_ea_eth_link_state_get		TkExtOamGetEthLinkState
#define _soc_ea_phy_admin_state_set		TkExtOamSetPhyAdminState
#define _soc_ea_phy_admin_state_get		TkExtOamGetPhyAdminState
#define _soc_ea_all_filter_table_clr	TkExtOamClrAllFilterTbl

#endif /* _SOC_EA_HAL_BRG_H */
