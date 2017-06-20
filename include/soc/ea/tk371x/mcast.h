/*
 * $Id: mcast.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     mcast.h
 * Purpose:
 *
 */

#ifndef _SOC_EA_MCAST_H
#define _SOC_EA_MCAST_H

#include <soc/ea/tk371x/CtcMcastApi.h>

typedef CtcOamMcastVlan _soc_ea_ctc_oam_mcast_vlan_t;

#define _soc_ea_ctc_multicast_vlan_add		CtcExtOamAddMulticastVlan
#define _soc_ea_ctc_multicast_vlan_del		CtcExtOamDelMulticastVlan
#define _soc_ea_ctc_multicast_vlan_clear	CtcExtOamClearMulticastVlan
#define _soc_ea_ctc_multicast_vlan_list		CtcExtOamListMulticastVlan
#define _soc_ea_ctc_multicast_tag_stripe_set	CtcExtOamSetMulticastTagstripe
#define _soc_ea_ctc_multicast_taq_stripe_get	CtcExtOamGetMulticastTagstripe
#define _soc_ea_ctc_multicast_switch_set	CtcExtOamSetMulticastSwitch
#define _soc_ea_ctc_multicast_switch_get	CtcExtOamGetMulticastSwitch
#define _soc_ea_ctc_multicast_control_set	CtcExtOamSetMulticastControl
#define _soc_ea_ctc_multicast_control_list	CtcExtOamListMulticastControl
#define _soc_ea_ctc_port_multicast_control_list \
											CtcExtOamListPortMulticastControl
#define _soc_ea_ctc_multicast_max_group_num_set	\
											CtcExtOamSetMulticastMaxGroupNum
#define _soc_ea_ctc_multicast_max_group_num_get \
											CtcExtOamGetMulticastMaxGroupNum
#define _soc_ea_ctc_afast_leave_ability_get	CtcExtOamGetaFastLeaveAblity
#define _soc_ea_ctc_afast_leave_admin_state_do \
											CtcExtOamaFastLeaveAdminState
#define _soc_ea_ctc_afast_leave_admin_control CtcExtOamacFastLeaveAdminControl

#endif /* _SOC_EA_MCAST_H */
