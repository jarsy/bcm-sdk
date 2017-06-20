/**********************************************************************************
**********************************************************************************
*                                                                                *
*  Revision      :  $Id: falcon_api_uc_common.h 1172 2015-10-06 19:34:58Z kirand $  *
*                                                                                *
*  Description   :  Defines and Enumerations required by Falcon ucode            *
*                                                                                *
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$                                                          *
*  No portions of this material may be reproduced in any form without            *
*  the written permission of:                                                    *
*      Broadcom Corporation                                                      *
*      5300 California Avenue                                                    *
*      Irvine, CA  92617                                                         *
*                                                                                *
*  All information contained in this document is Broadcom Corporation            *
*  company private proprietary, and trade secret.                                *
*                                                                                *
**********************************************************************************
**********************************************************************************/

/** @file falcon_api_uc_common.h
 * Defines and Enumerations shared by Falcon IP Specific API and Microcode
 */

#ifndef FALCON_API_UC_COMMON_H
#define FALCON_API_UC_COMMON_H

/* Add Falcon specific items below this */

/** Translate between a VCO frequency in MHz and the vco_rate that is **\
*** found in the Core Config Variable Structure using the formula:    ***
***                                                                   ***
***     vco_rate = (frequency_in_ghz * 16.0) - 224.0                  ***
***                                                                   ***
*** Both functions round to the nearest resulting value.  This        ***
*** provides the highest accuracy possible, and ensures that:         ***
***                                                                   ***
***     vco_rate == MHZ_TO_VCO_RATE(VCO_RATE_TO_MHZ(vco_rate))        ***
***                                                                   ***
*** In the microcode, this should only be called with a numeric       ***
*** literal parameter.                                                ***
\**                                                                   **/
#define MHZ_TO_VCO_RATE(mhz) ((uint8_t)(((((uint16_t)(mhz) * 2) + 62) / 125) - 224))
#define VCO_RATE_TO_MHZ(vco_rate) (((((uint16_t)(vco_rate) + 224) * 125) + 1) >> 1)

/* Please note that when adding entries here you should update the #defines in the falcon_tsc_common.h */

/** OSR_MODES Enum */
enum falcon_tsc_osr_mode_enum {
	FALCON_TSC_OSX1    = 0,
	FALCON_TSC_OSX2    = 1,
	FALCON_TSC_OSX4    = 2,
	FALCON_TSC_OSX8    = 5,
	FALCON_TSC_OSX16P5   = 8,
	FALCON_TSC_OSX16     = 9,
	FALCON_TSC_OSX20P625 = 12,
	FALCON_TSC_OSX32     = 13
};

/** CDR mode Enum **/
enum falcon_tsc_cdr_mode_enum {
	FALCON_TSC_CDR_MODE_OS_ALL_EDGES         = 0,
	FALCON_TSC_CDR_MODE_OS_PATTERN           = 1,
	FALCON_TSC_CDR_MODE_OS_PATTERN_ENHANCED  = 2,
	FALCON_TSC_CDR_MODE_BR_PATTERN           = 3
};

/** Lane User Control Clause93/72 Force Value **/
enum falcon_tsc_cl93n72_frc_val_enum {
	FALCON_TSC_CL93N72_FORCE_OS  = 0,
	FALCON_TSC_CL93N72_FORCE_BR  = 1
};


#endif
