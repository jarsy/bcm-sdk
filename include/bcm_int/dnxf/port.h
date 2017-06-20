/*
 * $Id: port.h,v 1.2.10.11.2.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        port.h
 * Purpose:     PORT internal definitions to the BCM library.
 */

#ifndef   _BCM_INT_DNXF_PORT_H_
#define   _BCM_INT_DNXF_PORT_H_


/* Attributes taht can be controlled on BCM88750*/
#define _BCM_DNXF_PORT_ATTRS      \
   (BCM_PORT_ATTR_ENABLE_MASK      | \
    BCM_PORT_ATTR_SPEED_MASK       | \
    BCM_PORT_ATTR_LINKSCAN_MASK    | \
    BCM_PORT_ATTR_LOOPBACK_MASK)

int _bcm_dnxf_port_deinit(int unit);

#endif /*_BCM_INT_DNXF_PORT_H_*/
