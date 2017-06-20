/*
 * $Id: common.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     common.h
 * Purpose:
 *
 */

#ifndef _SOC_EA_COMMON_H
#define _SOC_EA_COMMON_H

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/Ethernet.h>
#include <soc/ea/tk371x/CtcOam.h>

#define _BCM_EA_EPON_MAX_LINK_NUM 	SDK_MAX_NUM_OF_LINK
#define _BCM_EA_FAILSAFE_CNT 		CNT_OF_FAIL_SAFE

typedef EthernetVlanData 	_soc_ea_ethernet_vlan_data_t;
typedef CtcVlanTranslatate 	_soc_ea_vlan_translate_t;
typedef OamCtcDbaQueueSet 	_soc_ea_oam_ctc_dba_queue_set_t;
typedef OamCtcDbaData 		_soc_ea_oam_ctc_dba_data_t;
typedef CtcOamOnuSN			_soc_ea_ctc_oam_onu_sn_t;
typedef CtcOamFirmwareVersion 	_soc_ea_ctc_oam_firmware_version_t;
typedef CtcOamChipsetId		_soc_ea_ctc_oam_chipset_id_t;
typedef McastEntry			_soc_ea_mcast_entry_t;
typedef OamCtcMcastMode		_soc_ea_oam_ctc_mcast_mode_t;
typedef OamCtcMcastGroupOp 	_soc_ea_oam_ctc_mcast_group_op_t;
typedef McastCtrl			_soc_ea_mcast_ctrl_t;

#endif /* _SOC_EA_HAL_COMMON_H */
