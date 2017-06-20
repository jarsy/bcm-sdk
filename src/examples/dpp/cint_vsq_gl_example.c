/*~~~~~~~~~~~~~~~~~~~~~~~~~~VSQ Applications~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* $Id: cint_vsq_gl_example.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$ 
 *  
 * File: cint_vsq_gl_example.c 
 * Purpose: Example of usage of the Global VSQ
 *  
 * Global VSQ holds counters on available resources per resource type.
 * This is useful to trigger DP-dependent drop decisions and flow control.
 * The application example shows the sequence of enabling such a trigger.
 
 * The process of the Global VSQ application:
 *  1.  Create a GL-VSQ.
 *  2.  Set a specific drop trigger type and value (user provide parameters).
 * Enternace point: vsq_global_example
 *  
 * In order to test such application, user should run:
 *  1.  Traffic that will go to the same drop precedence (as configured).
 *  2.  See that the drop percentage is as execpted. (see example in the enternace point).
 *
 *
 * Explanation regrading the SET and CLEAR threshold:
 * --------------------------------------------------
 * The goal is to stabilize the traffic load using drop thresholds.
 * For example:
 *  In Buffer Descriptors the threshold represent used BD's, so we want the SET above the CLEAR.
 *  In Buffer Descriptors Buffers the threshold represent free BDB's, so we want the CLEAR above the SET.
 *     
 * The idea in this example is to create 2 threshold:
 *  1.  Once we are over the SET threshold, we start dropping packets.
 *  2.  As a result of the dropped packets, our traffic load is lowered.
 *  3.  Once we are below the CLEAR threshold, we stop dropping packets.
 *  4.  As a result, our traffic load is getting higher, until we reach step 1 again.
 *   
 * To see the results (when using the example below with BD), run the following scenario:
 *  1.  Run traffic from a port to itself, and set shaper upon it.
 *  2.  Set the above thresholds
 *  3.  You should see that VSQ QSZ counter is between SET and CLEAR *  thresholds (+-2).
 *
 */

/*
bcm_cosq_threshold_type_t :
    0  - bcmCosqThresholdBytes
    1  - bcmCosqThresholdPacketDescriptors
    2  - bcmCosqThresholdPackets
    3  - bcmCosqThresholdDataBuffers
    4  - bcmCosqThresholdAvailablePacketDescriptors
    5  - bcmCosqThresholdAvailableDataBuffers
    6  - bcmCosqThresholdBufferDescriptorBuffers
    7  - bcmCosqThresholdBufferDescriptors
    8  - bcmCosqThresholdDbuffs
    9  - bcmCosqThresholdFullDbuffs
    10 - bcmCosqThresholdMiniDbuffs    
*/

cint_reset();

struct vsq_global_info_e {
    bcm_cosq_threshold_type_t threshold_type;
    int threshold_value_set;
    int threshold_value_clear;
    int dp;
};

vsq_global_info_e vsq_global_info;

void vsq_global_init(int unit,int threshold_type, int threshold_value_set, int threshold_value_clear, int dp) {
    vsq_global_info.threshold_type = threshold_type;
    vsq_global_info.threshold_value_set = threshold_value_set;
    vsq_global_info.threshold_value_clear = threshold_value_clear;
    vsq_global_info.dp = dp;
}


/* The Enterance point 
 * Following configuration examples: 
 * vsq_global_init(0,7,1024,1000,1);
 * vsq_global_example(0); 
 * Run Traffic: Set shaper and run 10Gbps. 
 * Expected: VSQ QSZ counter will be between 1000-1024 +-2
 */
int vsq_global_example(int unit)
{
    bcm_gport_t vsq;
    bcm_cosq_vsq_info_t vsq_info;
    bcm_cosq_threshold_t threshold;
    bcm_error_t rc = BCM_E_NONE;

    /* Create Vsq global and optain its gport. */
    vsq_info.flags = BCM_COSQ_VSQ_GL;
    rc = bcm_cosq_gport_vsq_create(unit,&vsq_info,&vsq);
    print vsq;
    if (rc != BCM_E_NONE) {
        printf("Error, in bcm_cosq_gport_vsq_create\n");
        return rc;
    }
    
    /* set threshold settings */
    threshold.flags = BCM_COSQ_THRESHOLD_INGRESS | BCM_COSQ_THRESHOLD_PER_DP | BCM_COSQ_THRESHOLD_DROP | BCM_COSQ_THRESHOLD_SET;
    threshold.dp = vsq_global_info.dp;
    threshold.type = vsq_global_info.threshold_type;
    threshold.value = vsq_global_info.threshold_value_set;
    if (threshold.type == bcmCosqThresholdDbuffs) 
    {
        threshold.flags |= BCM_COSQ_THRESHOLD_UNICAST;
    }
    if (threshold.type == bcmCosqThresholdFullDbuffs || threshold.type == bcmCosqThresholdMiniDbuffs)
    {
        threshold.flags |= BCM_COSQ_THRESHOLD_MULTICAST;
    }

    rc = bcm_cosq_gport_threshold_set(unit,vsq,0,&threshold);
    if (rc != BCM_E_NONE) {
        printf("Error, in bcm_cosq_gport_threshold_set vsq 0x%x\n",vsq);
        return rc;
    }

    threshold.flags = BCM_COSQ_THRESHOLD_INGRESS | BCM_COSQ_THRESHOLD_PER_DP | BCM_COSQ_THRESHOLD_DROP | BCM_COSQ_THRESHOLD_CLEAR;    
    threshold.value = vsq_global_info.threshold_value_clear;
    rc = bcm_cosq_gport_threshold_set(unit,vsq,0,&threshold);
    if (rc != BCM_E_NONE) {
        printf("Error, in bcm_cosq_gport_threshold_set vsq 0x%x\n",vsq);
        return rc;
    }

    return rc;
}

/* VSQ Example: */
int vsq_example (int unit,bcm_gport_t* vsq_gport) {
    bcm_error_t rv = BCM_E_NONE;    
    
    /* Create VSQ Global */    
    bcm_cosq_vsq_info_t vsq_info;
    vsq_info.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,vsq_info,vsq_gport);
    if (rv != BCM_E_NONE) {
        return rv;
    }
    
    return rv;
}


/* Run automatic main */
int i;
for (i = 0; i < 4; ++i) {
    vsq_global_init(0,7,1024,1000,i);
    vsq_global_example(0);
}

    



    

