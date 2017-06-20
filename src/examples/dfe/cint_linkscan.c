/* 
 * $Id: cint_linkscan.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        cint_linkscan.c
 * Purpose:     Example to enable linkscan on a given set of ports.
 *
 */



/*
 * Callback routine called during a port state change event.
 */
void
linkscan_cb(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    printf("Link change event on unit %d, port %d, status: %d \n", unit, port, info->linkstatus);

    /* Application to process the link change event .... */
    
    return;
}

/*
 * This routine starts the linkscan thread with
 * the specified time interval.
 */
int
linkscan_start(int unit, int usecs)
{
    int  result;

    /* Start linkscan */
    result = bcm_linkscan_enable_set(unit, usecs);
    if (BCM_FAILURE(result)) {
        printf("Error, bcm_linkscan_enable_set unit %d, usecs %d\n",
               unit, usecs);
        return result;
    }

    /* Register callback routine */
    result = bcm_linkscan_register(unit, linkscan_cb);
    if (BCM_FAILURE(result)) {
        printf("Error, bcm_linkscan_register unit %d\n", unit);
        return result;
    }

    return result;
}

/*
 * Main routine to start SW linkscan on a given set of ports.
 * Linkscan thread is started.
 * Callback routine is registered, called at each port link change event. 
 *  ##input:
 *    modes:
 *        1. BCM_LINKSCAN_MODE_SW
 *        2. BCM_LINKSCAN_MODE_HW                                                                     .
 *  port1 , port2, port3 to enable linkscan in the above mode.                                                                .
 */
int
linkscan_enable(int unit, int mode, bcm_port_t port1, bcm_port_t port2, bcm_port_t port3)
{
    int         result;
    bcm_port_t  port;
    bcm_pbmp_t  pbmp;
    

    /* Start linkscan at 250000 usecs */
    result = linkscan_start(unit, 250000);
    if (BCM_FAILURE(result)) {
        return result;
    }

    /* Linkscan can be set per port or per-bitmap */
    /* Set SW linkscan on port1 */
    result = bcm_linkscan_mode_set(unit, port1, mode);
    if (BCM_FAILURE(result)) {
        printf("Error, bcm_linkscan_mode_set unit %d, port %d\n", unit, port1);
        return result;
    }

    /* Set SW linkscan on given port bitmap (port2, port3) */
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_SET(pbmp, port2);
    BCM_PBMP_PORT_ADD(pbmp, port3);
    result = bcm_linkscan_mode_set_pbm(unit, pbmp, mode);
    if (BCM_FAILURE(result)) {
        printf("Error, bcm_linkscan_mode_set_pbm unit %d\n", unit);
    }

    return result;
}

/*
 * Main routine to disable linkscan.
 * Linkscan thread is stopped.
 * Callback routine is unregistered.
 * Port linkscan is removed (set to NONE). 
 * port1 , port2, port3 to diable linkscan in the above mode.                                      .
 */
int
linkscan_disable(int unit, bcm_port_t port1, bcm_port_t port2, bcm_port_t port3)
{
    int         result;
    bcm_pbmp_t  pbmp;

    /* Stop linkscan thread */
    result = bcm_linkscan_enable_set(unit, 0);
    if (BCM_FAILURE(result)) {
        printf("Error, bcm_linkscan_enable_set unit %d, usecs 0\n", unit);
    }

    /* Unregister callback routine */
    result = bcm_linkscan_unregister(unit, linkscan_cb);
    if (BCM_FAILURE(result)) {
        printf("Error, bcm_linkscan_unregister unit %d\n", unit);
        return result;
    }

    /* Set linkscan mode to NONE on ports */
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_SET(pbmp, port1);
    BCM_PBMP_PORT_ADD(pbmp, port2);
    BCM_PBMP_PORT_ADD(pbmp, port3);
    result = bcm_linkscan_mode_set_pbm(unit, pbmp, BCM_LINKSCAN_MODE_NONE);
    if (BCM_FAILURE(result)) {
        printf("Error, bcm_linkscan_mode_set_pbm unit %d\n", unit);
    }

    return result;
}




/*  Checks if link is up
 */

int cint_check_link_up(int unit ,bcm_port_t port) {
    bcm_fabric_link_connectivity_t link_partner_info;
    int remote_port;
    int rv;
    

    rv = bcm_fabric_link_connectivity_status_single_get(unit, port, link_partner_info);
    if (BCM_FAILURE(rv)) {
        printf("Error, bcm_fabric_link_connectivity_status_single_get unit %d port %d\n", unit, port);
    }

    remote_port = link_partner_info.link_id;

    if (link_partner_info.device_type == bcmFabricDeviceTypeFAP) {
        remote_port = -1;
        printf("Error, unexpected connectivity to FAP, unit %d port %d\n", unit, port);
    }
    if (remote_port == BCM_FABRIC_LINK_NO_CONNECTIVITY) {
        remote_port = -1;
    }

    printf("%d\n",remote_port);
    return BCM_E_NONE;

   

}

