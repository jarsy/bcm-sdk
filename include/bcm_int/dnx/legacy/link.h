/*
 * $Id: link.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        link.h
 * Purpose:     LINK internal definitions to the BCM library.
 */

#ifndef   _BCM_INT_DNX_LINK_H_
#define   _BCM_INT_DNX_LINK_H_

extern int bcm_common_linkscan_mode_set(int,bcm_port_t,int);
extern int bcm_dnx_linkscan_mode_get(int unit, bcm_port_t port, int *mode);

extern int bcm_common_linkscan_trigger_event_set(int unit,
                                                bcm_port_t port,
                                                uint32 flags,
                                                bcm_linkscan_trigger_event_t trigger_event,
                                                int enable);
extern int bcm_common_linkscan_trigger_event_get(int unit,
                                                bcm_port_t port,
                                                uint32 flags,
                                                bcm_linkscan_trigger_event_t trigger_event,
                                                int *enable);

#endif /* _BCM_INT_DNX_LINK_H_ */
