/*
 * $Id: port.h,v 1.2.10.11.2.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        link.h
 * Purpose:     LINK internal definitions to the BCM library.
 */

#ifndef   _BCM_INT_DNXF_LINK_H_
#define   _BCM_INT_DNXF_LINK_H_

int  bcm_dnxf_linkscan_enable_set(int unit, int us);
int  bcm_dnxf_linkscan_mode_set(int unit,  bcm_port_t port,  int mode);
int  bcm_dnxf_linkscan_mode_get(int unit,  bcm_port_t port,  int *mode);
int  bcm_dnxf_linkscan_mode_set_pbm(int unit, bcm_pbmp_t pbm, int mode);
int bcm_dnxf_linkscan_detach(int unit);

#endif /*_BCM_INT_DNXF_LINK_H_*/
