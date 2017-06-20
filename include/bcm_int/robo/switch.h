/*
 * $Id: switch.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains internal definitions for switch module to the BCM library.
 */

#ifndef _BCM_INT_SWITCH_H
#define _BCM_INT_SWITCH_H

#define _BCM_SWITCH_DOS_COMMON_GROUP 0x1
#define _BCM_SWITCH_DOS_TCP_GROUP    0x2

/* TB's new design on IGMP/MLD snnoping mode setting definition.
 * The difference from previous ROBO device for IGMP and MLD are :
 *  - IGMP and MLD can only be set together on TB.
 *  - IGMP/MLD snooping can be disable on TB.
 *
 *  Assign the symbol definition as general logical type for further usage.
 */
#define  _BCM_IGMPMLD_MODE_NONE          0
#define  _BCM_IGMPMLD_MODE_CPU_TRAP      1
#define  _BCM_IGMPMLD_MODE_CPU_SNOOP     2
#define  _BCM_IGMPMLD_MODE_MAXID        _BCM_IGMPMLD_MODE_CPU_SNOOP


#define _BCM_SWITCH_IGMP_MLD_TYPES(type)    \
            (((type) == bcmSwitchIgmpPktToCpu) || \
            ((type) == bcmSwitchIgmpPktDrop) || \
            ((type) == bcmSwitchIgmpQueryToCpu) || \
            ((type) == bcmSwitchIgmpQueryDrop) || \
            ((type) == bcmSwitchIgmpReportLeaveToCpu) || \
            ((type) == bcmSwitchIgmpReportLeaveDrop) || \
            ((type) == bcmSwitchIgmpUnknownToCpu) || \
            ((type) == bcmSwitchIgmpUnknownDrop) || \
            ((type) == bcmSwitchMldPktToCpu) || \
            ((type) == bcmSwitchMldPktDrop) || \
            ((type) == bcmSwitchMldQueryToCpu) || \
            ((type) == bcmSwitchMldQueryDrop) || \
            ((type) == bcmSwitchMldReportDoneToCpu) || \
            ((type) == bcmSwitchMldReportDoneDrop) || \
            ((type) == bcmSwitchIgmpQueryFlood) || \
            ((type) == bcmSwitchIgmpReportLeaveFlood) || \
            ((type) == bcmSwitchIgmpUnknownFlood) || \
            ((type) == bcmSwitchMldQueryFlood) || \
            ((type) == bcmSwitchMldReportDoneFlood))

extern int _bcm_robo_switch_init(int unit);

#endif  /* !_BCM_INT_SWITCH_H */

