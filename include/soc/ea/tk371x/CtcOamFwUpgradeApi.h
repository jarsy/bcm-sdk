/*
 * $Id: CtcOamFwUpgradeApi.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     CtcOamFwUpgradeApi.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_CtcOamFwUpgradeApi_H
#define _SOC_EA_CtcOamFwUpgradeApi_H

#ifdef __cplusplus
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>


#define CTC_MAX_FILE_SIZE           1400
#define CTC_MAX_END_CHECK_CNT       6
#define CTC_END_CHECK_INTERVAL      30

extern CtcOamFileSession            ctcServerSession;

/*
 *  Function:
 *      CtcExtFirmwareUpgradeFile
 * Purpose:
 *      Upgrade a firmware to ONU using the CTC OAM 
 * Parameters:
 *      pathid    - which chipset you want to upgrade.
 *      Len    - The file length
 *      pLoadBuf    - The file data.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      
 */
int32   CtcExtFirmwareUpgradeFile (uint8 pathId, uint32 Len, uint8 * pLoadBuf);

/*
 *  Function:
 *      CtcExtFirmwareUpgradeActivateImg
 * Purpose:
 *      Try to run the firmware which just upgraded to the EPON chipset
 * Parameters:
 *      pathid    - Which chipset you want to upgrade.
 *      activeFlag    - The active flag input. fixed as 0, and reserved for future definiation.
 *      respCode    - The return code. 0: Activate image success, 1: Paramerter error of the 
 *                         OAM message, 2: the EPON MAC not supporting the image activate OAM
 *                         ,3: Activate fail.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      Actiavate the firmware in the backup section and have the EPON MAC boot with this 
 *      firmware . "Back rolling" is required during the process of the CTC image upgrade. 
 *      The new firmware is upgraded to the backup section. The old firmware is still in the 
 *      active sectino. This API allows the EPON MAC to start with the upgraded firmware
 *      but still keeps the upgraded firmware in the backup section. The EPON MAC still boots
 *      with the old firmware later. This API is used in the APP control mode.
 *
 *      Activeflag is fixed to 0 to denote the activate operation. Other values will cause the 
 *      error respCode(1)
 */
int32   CtcExtFirmwareUpgradeActivateImg (uint8 pathId,
                uint8 activeFlag, uint8 * respCode);

/*
 *  Function:
 *      CtcExtFirmwareUpgradeCheckImg
 * Purpose:
 *      Request ONU to check the firmware using the CTC OAM
 * Parameters:
 *      pathid    - Which chipset you want to upgrade.
 *      fileSize    - The file size upgrade just now.
 *      respCode    - The check response code. 0:" check sucdess" of the download S/W and
 *                         it has been written to the flash, 1 the EPON MAC is writting the firmware
 *                         to the flash, 2 "CRC32 check error" of the upgrade firmware, 3 "parameter
 *                         error" of the OAM message, 4 The EPON MAC not supporting the image 
 *                         check OAM.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32   CtcExtFirmwareUpgradeCheckImg (uint8 pathId, uint32 fileSize,
                uint8 * respCode);

/*
 *  Function:
 *      CtcExtFirmwareUpgradeCommitImg
 * Purpose:
 *      Request ONU to commit the firmware using the CTC OAM
 * Parameters:
 *      pathid    - Which chipset you want to upgrade.
 *      commitFlag    - The commit flag.Fixed 0 and reserved for future extension
 *      respCode    - The check response code. 0:success, 1 for parameter error. 2 for not
 *                         support this OAM. 3 for commit fail
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32   CtcExtFirmwareUpgradeCommitImg(uint8 pathId,
                               uint8 commitFlag, uint8 * respCode);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_CtcOamFwUpgradeApi_H */
