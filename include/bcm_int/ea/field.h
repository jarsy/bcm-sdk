/*
 * $Id: field.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains filed process module definitions internal to
 * the BCM library.
 */

#ifndef _BCM_INT_EA_FIELD_H
#define _BCM_INT_EA_FIELD_H
#include <soc/ea/tk371x/TkRuleApi.h>
#include <bcm/types.h>
#include <bcm/field.h>

#define _TK371X_FP_DEBUG	0

#define _BCM_EA_FIELD_MATCH_NEVER_MATCH 0
#define _BCM_EA_FIELD_MATCH_EQUAL		1
#define _BCM_EA_FIELD_MATCH_NOTEQUAL	2
#define _BCM_EA_FIELD_MATCH_LESSTHEN	3
#define _BCM_EA_FIELD_MATCH_MORETHEN	4
#define _BCM_EA_FIELD_MATCH_EXISTS		5
#define _BCM_EA_FIELD_MATCH_NOTEXISTS	6
#define _BCM_EA_FIELD_MATCH_ALWAYSMATCH	7
#define _BCM_EA_FIELD_MATCH_ERROR		-1


extern int _bcm_ea_field_rule_to_TkRuleConditionList(
		bcm_field_entry_t entry, TkRuleConditionList *cond);
extern int
_bcm_ea_field_operator_cond(uint8 *data, uint8 *mask, int len);
extern int
_bcm_ea_field_operator_cond_mask_get(uint8 match, uint8 *data, uint8 *mask, int len);

#endif
