/*
 * $Id: port.h,v 1.2.10.11.2.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        link.h
 * Purpose:     LINK internal definitions to the BCM library.
 */

#ifndef   _BCM_INT_DNXF_SWITCH_H_
#define   _BCM_INT_DNXF_SWITCH_H_
#include <sal/types.h>
#include <bcm/types.h>
#include <bcm/switch.h>

extern int bcm_dnxf_switch_temperature_monitor_get(int,int,bcm_switch_temperature_monitor_t *,int *);

#endif /*_BCM_INT_DNXF_SWITCH_H_*/
