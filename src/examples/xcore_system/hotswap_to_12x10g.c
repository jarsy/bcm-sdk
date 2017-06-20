/*
 * $Id: hotswap_to_12x10g.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 * Caladan3 script to switch to a 12x10g tdm.
 */
/* This script was developed and tested using the cint interpreter */

/* Queue related defines */
int INTERNAL_LINE_SQ_START    = 48;
int INTERNAL_LINE_SQ_END      = 50;
int INTERNAL_FABRIC_SQ_START  = 112;
int INTERNAL_FABRIC_SQ_END    = 114;
int MIN_SQ                    = 0;
int MAX_SQ                    = 127;
int MIN_LINE_SQ               = 0;
int MAX_LINE_SQ               = 63;
int MIN_FABRIC_SQ             = 64;
int MAX_FABRIC_SQ             = 127;
int MIN_DQ                    = 128;
int MAX_DQ                    = 255;
int MIN_LINE_DQ               = 128;
int MAX_LINE_DQ               = 191;
int MIN_FABRIC_DQ             = 192;
int MAX_FABRIC_DQ             = 255;

/*
 *  This functions scans the set of units to find which
 * unit is the C3.
 */
int get_c3_unit(void)
{
    int                 i;
    bcm_info_t          unit_info;
    int                 rc;


    /* Find C3 unit */
    for (i = 0; bcm_unit_valid(i); i++) {
        rc = bcm_info_get(i, &unit_info);
        if (BCM_FAILURE(rc)) {
            printf("bcm_info_get failed for unit %d, error: %s\n", i, bcm_errmsg(rc));
            return rc;
        }
        printf("Unit: %d, device: 0x%x, revision: %d\n", i, unit_info.device, unit_info.revision);
        if (unit_info.device == 0x0038) {
            return i;
        }
    }

    return  BCM_E_NOT_FOUND;

}
/* Returns TRUE if the source queue is internal CPU, etc... */ 
int is_internal_queue(int qid)
{
    if ((qid >= INTERNAL_LINE_SQ_START) && (qid <= INTERNAL_LINE_SQ_END)) {
        return TRUE;
    }
    if ((qid >= INTERNAL_FABRIC_SQ_START) && (qid <= INTERNAL_FABRIC_SQ_END)) {
        return TRUE;
    }
    return FALSE;
}

/* Removes all fabric and line side queues */
int delete_queues(int unit, int skip_internal_queues)
{
    int rv = BCM_E_NONE;
    bcm_gport_t ingress_queue, egress_queue;
    bcm_cos_t ingress_int_pri = 0, egress_int_pri=0;
    int sq = 0, dq=0;
    int attach_id = -1;
    int cos_level = 0;
    bcm_gport_t physical_port = 0;
    bcm_port_t port;
    int flags = 0;
    int num_cos_levels;
    
    for (sq=MIN_LINE_SQ; sq<MAX_LINE_SQ; sq++) {


        /* 
         * Skip internal queues 
         */
        if (skip_internal_queues && is_internal_queue(sq)) {
            printf("skipping internal queue %d\n", sq);
            continue;
        }

        /* 
         * Get the port associated with the current line source queue 
         */
        BCM_COSQ_GPORT_SRC_QUEUE_SET(ingress_queue, sq);

        rv = bcm_cosq_gport_get(unit, ingress_queue, &physical_port, &cos_level, &flags);
        
        /*
         * If there is no port associated with the queue, skip it.
         */
        if (rv == BCM_E_RESOURCE) {
            printf("skipping free queue %d\n", sq);
            rv = BCM_E_NONE;
            continue;
        } else if (BCM_FAILURE(rv)) {
            printf("error getting physical_port from sq(%d)\n", sq);
            return rv;
        }

        /*
         * Get the current fabric destination queue associated withe line source queue
         */
        rv = bcm_cosq_gport_queue_attach_get(unit, ingress_queue, ingress_int_pri, 
                                             &egress_queue, &egress_int_pri, 
                                             attach_id);
        if (BCM_FAILURE(rv)) {
            printf("error getting attached port sq(%d)\n", sq);    
            return rv;
        }

        /*
         * Detach the line source queue from the fabric dest queue
         */
        rv = bcm_cosq_gport_queue_detach(unit, ingress_queue, ingress_int_pri, attach_id);

        if (BCM_FAILURE(rv)) {
            printf("error detaching sq(%d)\n", sq);
            return rv;
        }

        /*
         * Delete the line source queue
         */
        rv = bcm_cosq_gport_delete(unit, ingress_queue);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) deleting ingress_queue(0x%08x)\n", rv, ingress_queue);
            return rv;
        }

        /*
         * Delete the fabric destination queue.
         */
        rv = bcm_cosq_gport_delete(unit, egress_queue);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) deleting egress_queue(0x%08x)\n", rv, egress_queue);        
            return rv;
        }
    }

    for (sq=MIN_FABRIC_SQ; sq<MAX_FABRIC_SQ; sq++) {

        /* 
         * Skip internal queues 
         */
        if (skip_internal_queues && is_internal_queue(sq)) {
            printf("skipping internal queue %d\n", sq);
            continue;
        }
        BCM_COSQ_GPORT_SRC_QUEUE_SET(ingress_queue, sq);
        rv = bcm_cosq_gport_get(unit, ingress_queue, &physical_port, &cos_level, &flags);

        /*
         * If there is no port associated with the queue, skip it.
         */
        if (rv == BCM_E_RESOURCE) {
            printf("skipping free queue %d\n", sq);
            rv = BCM_E_NONE;
            continue;
        } else if (BCM_FAILURE(rv)) {
            printf("rv(%d) error getting physical_port from sq(%d)\n", rv, sq);
            return rv;
        }

        /*
         * Get the current line destination queue associated withe fabric source queue
         */
        rv = bcm_cosq_gport_queue_attach_get(unit, ingress_queue, ingress_int_pri, 
                                             &egress_queue, &egress_int_pri, 
                                             attach_id);

        if (BCM_FAILURE(rv)) {
            printf("error getting attached port sq(%d)\n", sq);    
            return rv;
        }


        /*
         * Detach the fabric source queue from the line dest queue
         */
        rv = bcm_cosq_gport_queue_detach(unit, ingress_queue, ingress_int_pri, attach_id);

        if (BCM_FAILURE(rv)) {
            printf("rv(%d) error detaching sq(%d)\n", rv, sq);
            break;
        }

        /*
         * Delete the fabric source queue
         */
        rv = bcm_cosq_gport_delete(unit, ingress_queue);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) deleting ingress_queue(0x%08x)\n", rv, ingress_queue);
            return rv;
        }

        /*
         * Delete the fabric dest queue
         */
        rv = bcm_cosq_gport_delete(unit, egress_queue);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) deleting egress_queue(0x%08x)\n", rv, egress_queue);        
            return rv;
        }
    }
    return (rv);
}

int add_queue_and_attach(int unit, int sq, int dq, int *attach_id,
                         bcm_port_t src_port, bcm_port_t dst_port) {

    bcm_gport_t ingress_queue, egress_queue;
    bcm_cos_t ingress_int_pri = 0, egress_int_pri=0;
    int cos_level = 0;
    bcm_gport_t physical_port;
    int flags = 0;
    int num_cos_levels;
    int rv = BCM_E_NONE;
    
    /* 
     * Associate source queue  with source port
     */
    BCM_COSQ_GPORT_SRC_QUEUE_SET(ingress_queue, sq);
    BCM_GPORT_LOCAL_SET(physical_port, src_port);
    num_cos_levels = 1;
    flags = BCM_COSQ_GPORT_WITH_ID;

    rv = bcm_cosq_gport_add(unit, physical_port, num_cos_levels,
                            flags, &ingress_queue);
    if (rv == BCM_E_NONE) {
        printf("rv(%d) ingress_queue(0x%08x) added\n", rv, ingress_queue);
    } else {
        printf("bcm_cosq_gport_add(port=%d, queue=%d) failed, error %d(%s)\n",
               physical_port, ingress_queue, rv, bcm_errmsg(rv));
        return rv;
    }
    
    /*
     * Associate destination queue with destination port
     */
    BCM_COSQ_GPORT_DST_QUEUE_SET(egress_queue, dq);
    BCM_GPORT_LOCAL_SET(physical_port, dst_port);
    num_cos_levels = 1;
    flags = BCM_COSQ_GPORT_WITH_ID;

    rv = bcm_cosq_gport_add(unit, physical_port, num_cos_levels, 
                            flags, &egress_queue);
    if (rv != BCM_E_NONE) {
        printf("bcm_cosq_gport_add(port=%d, eqress queue=%d) failed, error %d(%s)\n",
               physical_port, egress_queue, rv, bcm_errmsg(rv));
        return rv;
    } else {
        printf("rv(%d) egress_queue(0x%08x) added\n", rv, egress_queue);
    }

    
    /*
     * Attach source queue to destination queue
     */
    flags = 0;
    rv = bcm_cosq_gport_queue_attach(unit, flags, ingress_queue, ingress_int_pri, 
                                     egress_queue, egress_int_pri, attach_id);

    if (rv != BCM_E_NONE) {
        printf("bcm_cosq_gport_queue_attach(ingress=%d, egress=%d) failed, error %d(%s)\n",
               ingress_queue, egress_queue, rv, bcm_errmsg(rv));
        return rv;
    }

    return rv;
}

int config_12x10g_queues(int unit)
{
    int rv = BCM_E_NONE;
    int attach_id;
    int sq_id, dq_id;
    bcm_port_t src_port, dst_port;
    int num_10g_ports = 12;

    /* 
     * add line to fabric queues 
     */
    dst_port = 53; /* ILKN fabric port */

    for (sq_id=MIN_LINE_SQ, dq_id=MIN_LINE_DQ, src_port=0;
         sq_id < num_10g_ports;
         sq_id++, dq_id++, src_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    /* 
     * add fabric to line queues 
     */
    src_port = 53; /* ILKN fabric port */

    for (sq_id=MIN_FABRIC_SQ, dq_id=MIN_FABRIC_DQ, dst_port=0; 
         sq_id <= MIN_FABRIC_SQ + num_10g_ports; 
         sq_id++, dq_id++, dst_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    return rv;
}

int config_1x100g_queues(int unit)
{
    int rv = BCM_E_NONE;
    int attach_id;
    int sq_id, dq_id;
    bcm_port_t src_port, dst_port;
    int num_100g_ports = 1;

    /* 
     * add line to fabric queues 
     */
    dst_port = 53; /* ILKN fabric port */

    for (sq_id=MIN_LINE_SQ, dq_id=MIN_LINE_DQ, src_port=0;
         sq_id < num_100g_ports;
         sq_id++, dq_id++, src_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    /* 
     * add fabric to line queues 
     */
    src_port = 53; /* ILKN fabric port */

    for (sq_id=MIN_FABRIC_SQ, dq_id=MIN_FABRIC_DQ, dst_port=0; 
         sq_id <= MIN_FABRIC_SQ + num_100g_ports;
         sq_id++, dq_id++, dst_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    return rv;
}

int config_8x10g_2xhg10_queues(int unit)
{
    int rv = BCM_E_NONE;
    int attach_id;
    int sq_id, dq_id;
    bcm_port_t src_port, dst_port;
    int num_10g_ports = 8;
    int num_hg10_ports = 2;
    int num_hg10_channels_per_port = 8;
    int channel;
    /* 
     * add line to fabric queues hg10
     */
    dst_port = 53; /* ILKN fabric port */

    for (sq_id=MIN_LINE_SQ, dq_id=MIN_LINE_DQ, src_port=0;
         sq_id < num_hg10_ports * num_hg10_channels_per_port;
         sq_id++, dq_id++, src_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    /* 
     * add line to fabric queues 10g
     */
    dst_port = 53; /* ILKN fabric port */

    for (;
         sq_id < (num_hg10_ports * num_hg10_channels_per_port) + num_10g_ports;
         sq_id++, dq_id++, src_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    /* 
     * add fabric to line queues hg10
     */
    src_port = 53; /* ILKN fabric port */

    for (sq_id=MIN_FABRIC_SQ, dq_id=MIN_FABRIC_DQ, dst_port=0; 
         sq_id <= MIN_FABRIC_SQ + (num_hg10_ports * num_hg10_channels_per_port); 
         sq_id++, dq_id++, dst_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    /* 
     * add fabric to line queues 10g
     */
    src_port = 53; /* ILKN fabric port */

    for (; 
         sq_id <= MIN_FABRIC_SQ + num_10g_ports + (num_hg10_ports * num_hg10_channels_per_port); 
         sq_id++, dq_id++, dst_port++) {

        rv = add_queue_and_attach(unit, sq_id,  dq_id, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rv)) {
            printf("error rv(%d) adding sq(%d) dq(%d) src_port(%d/%s) dst_port(%d)\n", rv, sq_id, dq_id, 
                   src_port, BCM_PORT_NAME(src_port), dst_port);
        }
    }

    return rv;
}

int config_12x10g(void)
{
    int                         c3_unit;
    int                         rc;
    bcm_pbmp_t                  pbmp;
    bcm_pbmp_t                  detached_ports;
    bcm_port_interface_config_t config;
    int                         port;


    /* Get the C3 unit number */
    c3_unit = get_c3_unit();
    if (c3_unit < 0) {
        printf("config_12x10g: Failed to find C3\n");
        return BCM_E_FAIL;
    }

    /* Delete all queues */
    rc = delete_queues(c3_unit, 1);
    if (BCM_FAILURE(rc)) {
        printf("delete_queues: failed\n");
        return rc;
    }

    /* Delete all existing line ports */
    BCM_PBMP_CLEAR(pbmp);
    for (port = 0; port < 48; port++) {
        BCM_PBMP_PORT_ADD(pbmp, port);
    }

    rc = bcm_port_detach(c3_unit, pbmp, &detached_ports);
    if (BCM_FAILURE(rc)) {
        printf("config_12x10g: bcm_caladan3_port_detach failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    config.flags = 0;
    config.channel = 0;
    config.interface = BCM_PORT_IF_SFI;

    for(port = 0; port < 12; port++) {
        
        /* Create the port */
        config.phy_port = port;
        rc = bcm_port_interface_config_set(c3_unit, config.phy_port, &config);
        if (BCM_FAILURE(rc)) {
            printf("config_12x10g: bcm_port_interface_config_set port %d failed: %s\n",
                config.phy_port, bcm_errmsg(rc));
            return rc;
        }

    }
    /* configure queues */
    rc = config_12x10g_queues(c3_unit);
    if (BCM_FAILURE(rc)) {
        printf("add_queues: failed\n");
        return rc;
    }

    /* Initiate the reconfiguration */
    rc =  bcm_switch_control_set(c3_unit, bcmSwitchControlPortConfigInstall, 1);
    if (BCM_FAILURE(rc)) {
        printf("config_12x10g: bcm_caladan3_port_detach failed: %s\n", bcm_errmsg(rc));
    }

    return BCM_E_NONE;

}


int config_1x100g(void)
{
    int                         c3_unit;
    int                         rc;
    bcm_pbmp_t                  pbmp;
    bcm_pbmp_t                  detached_ports;
    bcm_port_interface_config_t config;
    int                         port;


    /* Get the C3 unit number */
    c3_unit = get_c3_unit();
    if (c3_unit < 0) {
        printf("config_12x10g: Failed to find C3\n");
        return BCM_E_FAIL;
    }

    /* Delete all queues */
    rc = delete_queues(c3_unit, 1);
    if (BCM_FAILURE(rc)) {
        printf("delete_queues: failed\n");
        return rc;
    }

    /* Delete all existing line ports */
    BCM_PBMP_CLEAR(pbmp);
    for (port = 0; port < 48; port++) {
        BCM_PBMP_PORT_ADD(pbmp, port);
    }

    rc = bcm_port_detach(c3_unit, pbmp, &detached_ports);
    if (BCM_FAILURE(rc)) {
        printf("config_12x10g: bcm_caladan3_port_detach failed: %s\n", bcm_errmsg(rc));
        return rc;
    }

    config.flags = 0;
    config.channel = 0;
    config.interface = BCM_PORT_IF_XGMII;

    for(port = 0; port < 1; port++) {
        
        /* Create the port */
        config.phy_port = port;
        rc = bcm_port_interface_config_set(c3_unit, config.phy_port, &config);
        if (BCM_FAILURE(rc)) {
            printf("config_12x10g: bcm_port_interface_config_set port %d failed: %s\n",
                config.phy_port, bcm_errmsg(rc));
            return rc;
        }

    }
    /* configure queues */
    rc = config_1x100g_queues(c3_unit);
    if (BCM_FAILURE(rc)) {
        printf("add_queues: failed\n");
        return rc;
    }

    /* Initiate the reconfiguration */
    rc =  bcm_switch_control_set(c3_unit, bcmSwitchControlPortConfigInstall, 1);
    if (BCM_FAILURE(rc)) {
        printf("config_12x10g: bcm_caladan3_port_detach failed: %s\n", bcm_errmsg(rc));
    }

    return BCM_E_NONE;

}
/* 2xHg10 + 8x10g phy ports */
int PHY_PORT_HG10_START = 0;
int PHY_PORT_10G_START  = 4;

int config_8x10g_2xhg10(void)
{
    int                         c3_unit;
    int                         rc;
    bcm_pbmp_t                  pbmp;
    bcm_pbmp_t                  detached_ports;
    bcm_port_interface_config_t config;
    int                         port;
    int                         channel;
    int                         num_hg10_channels_per_port = 8;
    int                         num_hg10_ports = 2;
    int                         num_10g_ports = 8;
    int                         num_ports;
    int                         phy_port;

    num_ports = (num_hg10_ports * num_hg10_channels_per_port) + num_10g_ports;
    
    /* Get the C3 unit number */
    c3_unit = get_c3_unit();
    if (c3_unit < 0) {
        printf("config_12x10g: Failed to find C3\n");
        return BCM_E_FAIL;
    }
    
    /* Delete all queues */
    rc = delete_queues(c3_unit, 1);
    if (BCM_FAILURE(rc)) {
        printf("delete_queues: failed\n");
        return rc;
    }
    
    /* Delete all existing line ports */
    BCM_PBMP_CLEAR(pbmp);
    for (port = 0; port < 48; port++) {
        BCM_PBMP_PORT_ADD(pbmp, port);
    }
    
    rc = bcm_port_detach(c3_unit, pbmp, &detached_ports);
    if (BCM_FAILURE(rc)) {
        printf("config_12x10g: bcm_caladan3_port_detach failed: %s\n", bcm_errmsg(rc));
        return rc;
    }
    
    /* set up 2 hg10 ports */
    config.flags = BCM_PORT_ENCAP_HIGIG2;
    config.channel = 10000;
    config.interface = BCM_PORT_IF_XGMII;
    phy_port = PHY_PORT_HG10_START;
    
    for(port = 0; port < num_hg10_ports; port++, phy_port++) {
        
        /* Create the port */
        config.phy_port = phy_port;
        for (channel=0; channel<num_hg10_channels_per_port; channel++) {

            rc = bcm_port_interface_config_set(c3_unit, (port * num_hg10_channels_per_port) + channel, &config);
            if (BCM_FAILURE(rc)) {
                printf("config_2xhg10: bcm_port_interface_config_set port %d chan %d failed: %s\n",
                       port, channel, bcm_errmsg(rc));
                return rc;
            }
            
        }
    }
    port = num_hg10_ports * num_hg10_channels_per_port;

    /* set up 8 10g ports */
    config.flags = 0;
    config.channel = 0;
    config.interface = BCM_PORT_IF_SFI;
    phy_port = PHY_PORT_10G_START;
    
    for(; port < num_ports; port++, phy_port++) {
        
        /* Create the port */
        config.phy_port = phy_port;
        rc = bcm_port_interface_config_set(c3_unit, port, &config);
        if (BCM_FAILURE(rc)) {
            printf("config_2xhg10: bcm_port_interface_config_set port %d failed: %s\n",
                   config.phy_port, bcm_errmsg(rc));
            return rc;
        }
        
    }
    /* configure queues */
    rc = config_8x10g_2xhg10_queues(c3_unit);
    if (BCM_FAILURE(rc)) {
        printf("add_queues: failed\n");
        return rc;
    }
    
    /* Initiate the reconfiguration */
    rc =  bcm_switch_control_set(c3_unit, bcmSwitchControlPortConfigInstall, 1);
    if (BCM_FAILURE(rc)) {
        printf("config_12x10g: bcm_caladan3_port_detach failed: %s\n", bcm_errmsg(rc));
    }

    return BCM_E_NONE;
    
}

int config_4x10g_6xhg10(void)
{
    int                         c3_unit;
    int                         rc;
    bcm_pbmp_t                  pbmp;
    bcm_pbmp_t                  detached_ports;
    bcm_port_interface_config_t config;
    int                         port;
    int                         channel;
    int                         num_ports;
    int                         phy_port;
    int                         hg10_channels[6] = {
        4, 12, 8, 8, 8, 4

        // The following doesn't match any TDM. This leaves  sws_dbase[unit]->qm_cfg NULL,
        // which prevents PR ICC TCAM gets configured, because soc_sbx_caladan3_sws_pr_icc_tcam_program
        // checks this.
        // soc_sbx_caladan3_sws_tdm_select[1033] sws_dbase[unit]->qm_cfg = &sws_dbase[unit]->current_sws_cfg->qm_cfg;
        //8, 4, 16, 1, 2, 3
    };
    int                         i;
    char*                       __FUNCTION__ = "config_4x10g_6xhg10";
    int                         fabric_port = 53; /* ILKN fabric port */
    int                         attach_id;
    int                         sq, dq;
    bcm_port_t                  src_port, dst_port;

    sal_config_set("bcm88030_ucode", "g3p1");
    sal_config_set("ucode_num_port_override", "1");

    num_ports = 4;
    for (i=0; i<6; i++) {
        num_ports += hg10_channels[i];
    }

    c3_unit = get_c3_unit();
    if (c3_unit < 0) {
        printf("%s: Failed to find C3\n", __FUNCTION__);
        return BCM_E_FAIL;
    }
    
    /* Delete all queues */
    rc = delete_queues(c3_unit, 0);
    if (BCM_FAILURE(rc)) {
        printf("delete_queues: failed\n");
        return rc;
    }
    
    /* Delete all existing line port:  ucode ports?  */
    BCM_PBMP_CLEAR(pbmp);
    for (port = 0; port < 48; port++) {
        BCM_PBMP_PORT_ADD(pbmp, port);
    }
    rc = bcm_port_detach(c3_unit, pbmp, &detached_ports);
    if (BCM_FAILURE(rc)) {
        printf("%s: bcm_caladan3_port_detach failed: %s\n", __FUNCTION__, bcm_errmsg(rc));
        return rc;
    }

    /* set up 4 10g ports */
    config.flags = 0;
    config.channel = 0;
    config.interface = BCM_PORT_IF_SFI;
    phy_port = 0;
    for(port=0; port < 4; port++) {
        phy_port = port; /* not channelized */
        config.phy_port = phy_port;
        rc = bcm_port_interface_config_set(c3_unit, port, &config);
        if (BCM_FAILURE(rc)) {
            printf("%s: bcm_port_interface_config_set port %d failed: %s\n",
                   __FUNCTION__,
                   config.phy_port, bcm_errmsg(rc));
            return rc;
        }
        /* ingress queues */
        sq = port;
        dq = 128 + port;
        src_port = port;
        dst_port = fabric_port;
        rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rc)) {
            printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                   rc, sq, dq, src_port, dst_port);
            return rc;
        }
        /* egress queues */
        sq = 64 + port;
        dq = 192 + port;
        src_port = fabric_port;
        dst_port = port;
        rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rc)) {
            printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                   rc, sq, dq, src_port, dst_port);
            return rc;
        }        
    }
    
    /* set up 6 hg10 ports */
    config.flags = BCM_PORT_ENCAP_HIGIG2;
    config.channel = 10000;
    config.interface = BCM_PORT_IF_XGMII;
    phy_port = 4;
    for (i=0; i<6; i++) {
        config.phy_port = phy_port;
        for (channel=0; channel<hg10_channels[i]; channel++) {
            rc = bcm_port_interface_config_set(c3_unit, 
                                               port,
                                               &config);
            if (BCM_FAILURE(rc)) {
                printf("%s: bcm_port_interface_config_set port %d: phy_port %d chan %d failed: %s\n",
                       __FUNCTION__,
                       port, phy_port, channel, bcm_errmsg(rc));
                return rc;
            }

            /* ingress queues */
            sq = port;
            dq = 128 + port;
            src_port = port;
            dst_port = fabric_port;
            rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
            if (BCM_FAILURE(rc)) {
                printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                       rc, sq, dq, src_port, dst_port);
                return rc;
            }
            /* egress queues */
            sq = 64 + port;
            dq = 192 + port;
            src_port = fabric_port;
            dst_port = port;
            rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
            if (BCM_FAILURE(rc)) {
                printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                       rc, sq, dq, src_port, dst_port);
                return rc;
            }        
            
            port++;
        }
        phy_port++;
    }
    
    /* Initiate the reconfiguration */
    rc =  bcm_switch_control_set(c3_unit, bcmSwitchControlPortConfigInstall, 1);
    if (BCM_FAILURE(rc)) {
        printf("%s: bcm_caladan3_port_detach failed: %s\n", __FUNCTION__, bcm_errmsg(rc));
        return rc;
    }

    /* test bed */
    bshell(c3_unit, "vlan create 2");
    bshell(c3_unit, "vlan add 2 pbm=xe,hg,cpu");
    bshell(c3_unit, "l2 add mac=0x1 vlanid=2 mod=0 pbm=xe,cpu static=true");
    bshell(c3_unit, "l2 add mac=0xfb0000000001  vlanid=2 mod=0 pbm=hg static=true");
    bshell(c3_unit, "g3p1s v2e vlan=1 dontlearn=1");
    bshell(c3_unit, "g3p1s v2e vlan=2 dontlearn=1");
    bshell(c3_unit, "g3p1s pv2e port=48 vid=2 stpstate=0");
    bshell(c3_unit, "port il0 lb=phy");
    bshell(c3_unit, "g3p1GlobalSet higig_loop_enable 1");
    bshell(c3_unit, "m xmac_pause_ctrl.xe0 tx_pause_en=0");
    bshell(c3_unit, "setr mac_rsv_mask.hg 0xc8");
    bshell(c3_unit, "for i=0,63 'g3p1s lp $i pid=0xfff'");

    return BCM_E_NONE;
}

int config_8x10g_4xhg10(void)
{
    int                         c3_unit;
    int                         rc;
    bcm_pbmp_t                  pbmp;
    bcm_pbmp_t                  detached_ports;
    bcm_port_interface_config_t config;
    int                         port;
    int                         channel;
    int                         num_ports;
    int                         phy_port;
    int                         hg10_channels[4] = {
        10, 10, 10, 10
    };
    int                         i;
    char*                       __FUNCTION__ = "config_8x10g_4xhg10";
    int                         fabric_port = 53; /* ILKN fabric port */
    int                         attach_id;
    int                         sq, dq;
    bcm_port_t                  src_port, dst_port;

    sal_config_set("bcm88030_ucode", "g3p1");
    sal_config_set("ucode_num_port_override", "1");

    num_ports = 8;
    for (i=0; i<4; i++) {
        num_ports += hg10_channels[i];
    }

    c3_unit = get_c3_unit();
    if (c3_unit < 0) {
        printf("%s: Failed to find C3\n", __FUNCTION__);
        return BCM_E_FAIL;
    }
    
    /* Delete all queues */
    rc = delete_queues(c3_unit, 0);
    if (BCM_FAILURE(rc)) {
        printf("delete_queues: failed\n");
        return rc;
    }
    
    /* Delete all existing line port:  ucode ports?  */
    BCM_PBMP_CLEAR(pbmp);
    for (port = 0; port < 60; port++) {
        BCM_PBMP_PORT_ADD(pbmp, port);
    }
    rc = bcm_port_detach(c3_unit, pbmp, &detached_ports);
    if (BCM_FAILURE(rc)) {
        printf("%s: bcm_caladan3_port_detach failed: %s\n", __FUNCTION__, bcm_errmsg(rc));
        return rc;
    }

    /* set up 8 10g ports */
    config.flags = 0;
    config.channel = 0;
    config.interface = BCM_PORT_IF_SFI;
    phy_port = 0;
    for(port=0; port < 8; port++) {
        phy_port = port; /* not channelized */
        config.phy_port = phy_port;
        rc = bcm_port_interface_config_set(c3_unit, port, &config);
        if (BCM_FAILURE(rc)) {
            printf("%s: bcm_port_interface_config_set port %d failed: %s\n",
                   __FUNCTION__,
                   config.phy_port, bcm_errmsg(rc));
            return rc;
        }
        /* ingress queues */
        sq = port;
        dq = 128 + port;
        src_port = port;
        dst_port = fabric_port;
        rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rc)) {
            printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                   rc, sq, dq, src_port, dst_port);
            return rc;
        }
        /* egress queues */
        sq = 64 + port;
        dq = 192 + port;
        src_port = fabric_port;
        dst_port = port;
        rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
        if (BCM_FAILURE(rc)) {
            printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                   rc, sq, dq, src_port, dst_port);
            return rc;
        }        
    }
    
    /* set up 4 hg10 ports */
    config.flags = BCM_PORT_ENCAP_HIGIG2;
    config.channel = 10000;
    config.interface = BCM_PORT_IF_XGMII;
    phy_port = 8;
    for (i=0; i<4; i++) {
        config.phy_port = phy_port;
        for (channel=0; channel<hg10_channels[i]; channel++) {
            rc = bcm_port_interface_config_set(c3_unit, 
                                               port,
                                               &config);
            if (BCM_FAILURE(rc)) {
                printf("%s: bcm_port_interface_config_set port %d: phy_port %d chan %d failed: %s\n",
                       __FUNCTION__,
                       port, phy_port, channel, bcm_errmsg(rc));
                return rc;
            }

            /* ingress queues */
            sq = (port>= 48)? port + 3 : port;
            dq = sq + 128;
            src_port = port;
            dst_port = fabric_port;
            rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
            if (BCM_FAILURE(rc)) {
                printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                       rc, sq, dq, src_port, dst_port);
                return rc;
            }
            /* egress queues */
            sq = (port >= 48)? (64 + port + 3) : (64+port);
            dq = sq + 128;
            src_port = fabric_port;
            dst_port = port;
            rc = add_queue_and_attach(c3_unit, sq,  dq, &attach_id, src_port, dst_port);
            if (BCM_FAILURE(rc)) {
                printf("error %d adding sq %d dq %d src_port %d dst_port %d\n", 
                       rc, sq, dq, src_port, dst_port);
                return rc;
            }        
            
            port++;
        }
        phy_port++;
    }
    
    /* Initiate the reconfiguration */
    rc =  bcm_switch_control_set(c3_unit, bcmSwitchControlPortConfigInstall, 1);
    if (BCM_FAILURE(rc)) {
        printf("%s: bcm_caladan3_port_detach failed: %s\n", __FUNCTION__, bcm_errmsg(rc));
        return rc;
    }

    /* test bed */
    bshell(c3_unit, "vlan create 2");
    bshell(c3_unit, "vlan add 2 pbm=xe,hg,cpu");
    bshell(c3_unit, "l2 add mac=0x1 vlanid=2 mod=0 pbm=xe,cpu static=true");
    bshell(c3_unit, "l2 add mac=0xfb0000000001  vlanid=2 mod=0 pbm=hg static=true");
    bshell(c3_unit, "g3p1s v2e vlan=1 dontlearn=1");
    bshell(c3_unit, "g3p1s v2e vlan=2 dontlearn=1");
    bshell(c3_unit, "g3p1s pv2e port=48 vid=2 stpstate=0");
    bshell(c3_unit, "port il0 lb=phy");
    bshell(c3_unit, "g3p1GlobalSet higig_loop_enable 1");
    bshell(c3_unit, "m xmac_pause_ctrl.xe0 tx_pause_en=0");
    bshell(c3_unit, "setr mac_rsv_mask.hg 0xc8");
    bshell(c3_unit, "for i=0,63 'g3p1s lp $i pid=0xfff'");

    return BCM_E_NONE;
}


/* config_12x10g(); */
/* config_1x100g(); */
/*config_8x10g_2xhg10();*/
/*config_4x10g_6xhg10();*/
config_8x10g_4xhg10();
