/* 
 * $Id: switch.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        switch.h
 * Purpose:     Switch internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_SWITCH_H_
#define _BCM_INT_SBX_FE2000_SWITCH_H_

int
_bcm_fe2000_switch_control_init(int unit);

/* Default priority for all exceptions - 
 * Higher cos, lower priority in the QE 
 */
#define _BCM_SWITCH_EXC_DEFAULT_COS  0

#endif /* _BCM_INT_SBX_FE2000_SWITCH_H_ */

