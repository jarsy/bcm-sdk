/*
 * $Id: upgrade.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     upgrade.h
 * Purpose:
 *
 */
#ifndef _SOC_EA_UPGRADE_H
#define _SOC_EA_UPGRADE_H

#include <soc/ea/tk371x/FileTransCtc.h>
#include <soc/ea/tk371x/CtcOamFwUpgradeApi.h>
#include <soc/ea/tk371x/TkOamFwUpgradeApi.h>

#define _soc_ea_ctc_fw_upgrade_file_do		CtcExtFirmwareUpgradeFile
#define _soc_ea_ctc_fw_upgrade_img_active 	CtcExtFirmwareUpgradeActivateImg
#define _soc_ea_ctc_fw_upgrade_img_check	CtcExtFirmwareUpgradeCheckImg
#define _soc_ea_ctc_fw_upgrade_img_commit	CtcExtFirmwareUpgradeCommitImg

#define _soc_ea_sdk_fw_upgrade_do	\
										TkExtFirmwareUpgrade
#endif /* _SOC_EA_UPGRADE_H */
