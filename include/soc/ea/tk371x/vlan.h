/*
 * $Id: vlan.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     vlan.h
 * Purpose:
 *
 */
#ifndef _BCM_INT_VLAN_H
#define _BCM_INT_VLAN_H

#include <soc/ea/tk371x/CtcVlanApi.h>
#include <soc/ea/tk371x/common.h>

typedef CtcVlanTagInfo _soc_ea_ctc_vlan_tag_info_t;
typedef CtcVlanTranslateInfo _soc_ea_ctc_vlan_translate_info_t;
typedef CtcVlanTrunkInfo _soc_ea_ctc_vlan_trunk_info_t;
typedef CtcVlanEntryInfo _soc_ea_ctc_vlan_entry_info_t;

#define _soc_ea_ctc_vlan_transparent_set 	CtcExtOamSetVlanTransparent
#define _soc_ea_ctc_vlan_tag_set 			CtcExtOamSetVlanTag
#define _soc_ea_ctc_vlan_translation_set 	CtcExtOamSetVlanTranslation
#define _soc_ea_ctc_vlan_trunk_set			CtcExtOamSetVlanTrunk
#define _soc_ea_ctc_vlan_get				CtcExtOamGetVlan

#endif /* _BCM_INT_VLAN_H */
