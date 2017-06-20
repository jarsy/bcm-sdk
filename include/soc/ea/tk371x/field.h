/*
 * $Id: field.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     field.h
 * Purpose:
 *
 */
#ifndef _SOC_EA_FIELD_H
#define _SOC_EA_FIELD_H

#include <soc/ea/tk371x/TkRuleApi.h>

typedef TkLinkConfigInfo 	_soc_ea_tk_link_config_info_t;
typedef TkPortConfigInfo 	_soc_ea_tk_port_config_info_t;
typedef TkQueueConfigInfo 	_soc_ea_tk_queue_config_info_t;
typedef TkRuleCondition		_soc_ea_tk_rule_condition_t;
typedef TkRuleConditionList	_soc_ea_tk_rule_condition_list_t;
typedef TkRuleNameQueue		_soc_ea_tk_rule_name_queue_t;
typedef TkRulePara			_soc_ea_tk_rule_para_t;

#define _soc_ea_filter_rules_by_port_get	\
									TkExtOamGetFilterRulesByPort
#define _soc_ea_all_filter_rules_by_port_clear \
									TkExtOamClearAllFilterRulesByPort
#define _soc_ea_all_user_rules_by_port_clear	\
									TkExtOamClearAllUserRulesByPort
#define _soc_ea_one_rule_by_port_add	\
									TkExtOamAddOneRuleByPort
#define _soc_ea_one_rule_by_port_delete	\
									TkExtOamDelOneRuleByPort
#define _soc_ea_all_classify_rules_by_port_clear	\
									TkExtOamClearAllClassifyRulesByPort
#define _soc_ea_rule_act_discard	TkRuleActDiscard
#define _soc_ea_rule_ds_arp_with_specified_sender_ip_addr_discard \
							TkRuleDiscardDsArpWithSpecifiedSenderIpAddr

int
_soc_ea_queue_configuration_get(int unit, TkQueueConfigInfo * q_cfg);

int
_soc_ea_queue_configuration_set(int unit, TkQueueConfigInfo *q_cfg);

#endif /* _SOC_EA_FIELD_H */
