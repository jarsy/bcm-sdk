/*
 * $Id: cint_e2e_interdigitated.c $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This cint configures simple scheduling scheme using interdigitated mode. 
 * Interdigitated mode is configured by the flag BCM_COSQ_GPORT_NON_CONTIGUOUS_VOQ_CONNECTOR.
 *
 * Test Scenario:
 * 1) SoC property:
 *      diag_cosq_disable=1
 * 2) Configure 8 streams with different TCs.
 * 3) Build simple interdigitated scheduling scheme.
 * 4) Configure snake from the port to itself, and expect all traffic to get back.
 */

/*
 * Params:
 *  unit - 
 *  dest_port   - destination port to build the scheduling scheme.
 *  q_num       - VOQ number for the destination port.
 *  e2e_bw      - bandwidth of the port (in kbps).
 */
int cint_e2e_interdigitated_start_run(int unit, bcm_port_t dest_port, int q_num, int e2e_bw) {
    int rv = BCM_E_NONE; 
    int cos, num_cos = 8;
    
    bcm_gport_t queue;
    bcm_gport_t sys_port;
    bcm_cosq_ingress_queue_bundle_gport_config_t conf;

    bcm_gport_t cl_element;
    bcm_gport_t voq_con;
    bcm_cosq_voq_connector_gport_t config;
    
    /***********************************/
    /*          VOQ                    */
    /***********************************/

    /* Create E2E port */
    bcm_gport_t e2e_port;
    BCM_COSQ_GPORT_E2E_PORT_SET(e2e_port, dest_port);
    
    /* Set E2E BW */
    rv = bcm_cosq_gport_bandwidth_set(unit, e2e_port, 0, 0, e2e_bw, 0);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_gport_bandwidth_set failed, bw = %d\n", e2e_bw);
        return rv;
    }
    
    /* Create dest gport */
    bcm_gport_t dest_gport;
    BCM_GPORT_SYSTEM_PORT_ID_SET(sys_port, dest_port);
    rv = bcm_stk_sysport_gport_get(unit, sys_port, &dest_gport); /*sysport to modport */
    if (BCM_FAILURE(rv)) {
        printf("bcm_stk_sysport_gport_get failed, sys_port = %d\n", sys_port);
        return rv;
    }
    
    /* Create interdigitated queue */
    BCM_GPORT_UNICAST_QUEUE_GROUP_SET(queue, q_num);
    conf.flags = BCM_COSQ_GPORT_UCAST_QUEUE_GROUP | BCM_COSQ_GPORT_NON_CONTIGUOUS_VOQ_CONNECTOR | BCM_COSQ_GPORT_WITH_ID;
    conf.port = dest_gport;
    conf.numq = num_cos;
    for (cos = 0; cos < num_cos; ++cos) {
        conf.queue_atrributes[cos].delay_tolerance_level = BCM_COSQ_DELAY_TOLERANCE_10G_SLOW_ENABLED;
        conf.queue_atrributes[cos].rate_class = 0;
    }
    rv = bcm_cosq_ingress_queue_bundle_gport_add(unit, &conf, &queue);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_ingress_queue_bundle_gport_add failed, VOQ = 0x%x", queue);
        return rv;
    }
    
    /***********************************/
    /*          VOQ CONNECTOR          */
    /***********************************/
    
    /* Create scheduler and connect it to e2e_port */
    rv = bcm_cosq_gport_add(unit, e2e_port, 1, BCM_COSQ_GPORT_SCHEDULER | BCM_COSQ_GPORT_SCHEDULER_CLASS_MODE1_4SP, &cl_element);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_gport_add failed\n");
        return rv;
    }
    rv = bcm_cosq_gport_sched_set(unit, cl_element, 0, BCM_COSQ_SP1, 0);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_gport_sched_set failed, cl_element = 0x%x\n", cl_element);
        return rv;
    }
    rv = bcm_cosq_gport_attach(unit, e2e_port, cl_element, 0);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_gport_attach failed, cl_element = 0x%x\n", cl_element);
        return rv;
    }
    rv = bcm_cosq_gport_bandwidth_set(unit, cl_element, 0, 0, e2e_bw, 0);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_gport_bandwidth_set failed, cl_element = 0x%x, e2e_bw = %d\n", cl_element, e2e_bw);
        return rv;
    }
    rv = bcm_cosq_control_set(unit, cl_element, 0, bcmCosqControlBandwidthBurstMax, 3000);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_control_set bcmCosqControlBandwidthBurstMax failed, cl_element = 0x%x\n", cl_element, e2e_bw);
        return rv;
    }
    
    /* Get modid to use in VOQ connector creation */
    int modid;
    rv = bcm_stk_modid_get(unit, &modid);
    if (BCM_FAILURE(rv)) {
        printf("bcm_stk_modid_get failed, unit = %d\n", unit);
        return rv;
    }
   
    /* Create VOQ connector */
    config.port = e2e_port;
    config.flags = BCM_COSQ_GPORT_NON_CONTIGUOUS_VOQ_CONNECTOR;
    config.numq = num_cos;
    config.remote_modid = modid;
    config.nof_remote_cores = 1;
    rv = bcm_cosq_voq_connector_gport_add(unit, config, &voq_con);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_voq_connector_gport_add failed, unit = %d\n", unit);
        return rv;
    }
    
    /* Attach VOQ connector to sched scheme*/
    for(cos = 0; cos < num_cos; cos++) {
        rv = bcm_cosq_gport_sched_set(unit, voq_con, cos, BCM_COSQ_SP0, 0);
        if (BCM_FAILURE(rv)) {
            printf("bcm_cosq_gport_sched_set failed, voq_connector = 0x%x, cosq = %d\n", voq_con, cos);
            return rv;
        }
        rv = bcm_cosq_gport_attach(unit, cl_element, voq_con, cos);
        if (BCM_FAILURE(rv)) {
            printf("bcm_cosq_gport_attach failed, cl_element = 0x%x, voq_connector = 0x%x, cosq = %d\n", cl_element, voq_con, cos);
            return rv;
        }
    }
    
    /* Connect between VOQs and VOQ-Connectors */
    bcm_cosq_gport_connection_t connection;
    connection.flags = BCM_COSQ_GPORT_CONNECTION_INGRESS;
    connection.remote_modid = modid;
    connection.voq = queue;
    connection.voq_connector = voq_con;
    rv = bcm_cosq_gport_connection_set(unit, &connection);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_gport_connection_set BCM_COSQ_GPORT_CONNECTION_INGRESS failed, unit = %d\n", unit);
        return rv;
    }
    
    connection.flags = BCM_COSQ_GPORT_CONNECTION_EGRESS;
    rv = bcm_cosq_gport_connection_set(unit, &connection);
    if (BCM_FAILURE(rv)) {
        printf("bcm_cosq_gport_connection_set BCM_COSQ_GPORT_CONNECTION_EGRESS failed, unit = %d\n", unit);
        return rv;
    }

    /***********************************/
    /*          SET COMPENSATION       */
    /***********************************/

    for (cos = 0; cos < num_cos; ++cos) {
        rv = bcm_cosq_control_set(unit, queue, cos, bcmCosqControlPacketLengthAdjust, 2);
        if (BCM_FAILURE(rv)) {
            printf("bcm_cosq_control_set with bcmCosqControlPacketLengthAdjust failed, unit = %d, queue_gport = 0x%x, cos = %d\n", unit, queue, cos);
            return rv;
        }
    }

    printf("cint_e2e_interdigitated_start_run: PASS\n\n");
    return rv;
}

