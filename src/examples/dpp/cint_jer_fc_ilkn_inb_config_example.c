/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: cint_jer_fc_ilkn_inb_config_example.c
 */
/*
 * Purpose: 
 *     Example of inband fc generation and reception on ILKN.
 *
 */

/*##############################################################*/
/*###################    INB GENERATION     ####################*/
/*##############################################################*/
/* set fc generation(global resource) and fc target(a tx slot on ILKN) */
int cint_jer_test_fc_ilkn_inb_gl_gen_set_example(int unit, int ilkn_port, int inb_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;

    bcm_cosq_fc_endpoint_t fc_trigger; 
    bcm_cosq_fc_endpoint_t fc_handler;

    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;

    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_set_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("Trigger global fc failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }

    /* Configuring inb fc { */
    /* setting port */

    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags = BCM_COSQ_FC_BDB;
            break;
        case global_vsq_src_mini_db: 
            flags = BCM_COSQ_FC_MINI_DB;
            break;
        case global_vsq_src_full_db: 
            flags = BCM_COSQ_FC_FULL_DB;
            break;
        case global_vsq_src_ocb_db: 
            flags = BCM_COSQ_FC_OCB_DB;
            break;
        default: 
            return BCM_E_PARAM;
    }

    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags | BCM_COSQ_FC_MASK_POOL_INDICATION;
    fc_handler.flags = 0;
    /* setting priority */    
    fc_trigger.priority = priority;
    /* setting cosq */ 
    fc_trigger.cosq = 0;
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("INB fc generation configuration failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }
    /* Configuring inb fc } */

    return rv;
}

int cint_jer_test_fc_ilkn_inb_gl_gen_unset_example(int unit, int ilkn_port, int inb_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;

    bcm_cosq_fc_endpoint_t fc_trigger;
    bcm_cosq_fc_endpoint_t fc_handler;

    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;
    
    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_unset_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("Trigger global fc failed(%d) in cint_jer_test_fc_inb_gl_gen_unset_example\n", rv);
        return rv;
    }

    /* Configuring inb fc { */
    /* setting port */

    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags = BCM_COSQ_FC_BDB;
            break;
        case global_vsq_src_mini_db: 
            flags = BCM_COSQ_FC_MINI_DB;
            break;
        case global_vsq_src_full_db: 
            flags = BCM_COSQ_FC_FULL_DB;
            break;
        case global_vsq_src_ocb_db: 
            flags = BCM_COSQ_FC_OCB_DB;
            break;
        default: 
            return BCM_E_PARAM;
    }

    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags | BCM_COSQ_FC_MASK_POOL_INDICATION;
    fc_handler.flags = 0;
    /* setting priority */    
    fc_trigger.priority = priority;
    /* setting cosq */ 
    fc_trigger.cosq = 0;
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("INB fc generation configuration failed(%d) in cint_jer_test_fc_inb_gl_gen_unset_example\n", rv);
        return rv;
    }
    /* Configuring inb fc } */

    return rv;
}

int cint_qax_test_fc_ilkn_inb_gl_gen_set_example(int unit, int ilkn_port, int inb_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;
    bcm_cosq_fc_endpoint_t fc_trigger; 
    bcm_cosq_fc_endpoint_t fc_handler;
    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;

    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_set_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("Trigger global fc failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }

    /* Configuring inb fc { */
    sal_memset(&fc_trigger, 0, sizeof(fc_trigger));
    sal_memset(&fc_handler, 0, sizeof(fc_handler));

    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags &= ~(BCM_COSQ_FC_IS_OCB_ONLY);
            flags |= BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_ocb_db:
        case global_vsq_src_ocb_pdb:
            flags |= BCM_COSQ_FC_IS_OCB_ONLY | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_p0_db: 
        case global_vsq_src_p0_pd:
        case global_vsq_src_p0_byte:
            fc_trigger.cosq = 0;
            break;
        case global_vsq_src_p1_db: 
        case global_vsq_src_p1_pd:
        case global_vsq_src_p1_byte:
            fc_trigger.cosq = 1;
            break;
        case global_vsq_src_hdrm_db: 
        case global_vsq_src_hdrm_pd:
            flags |= BCM_COSQ_FC_HEADROOM | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        default: 
            return BCM_E_PARAM;
    }

    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags;
    fc_handler.flags = 0;
    /* setting priority */    
    fc_trigger.priority = priority;
    /* setting cosq */ 
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("INB fc generation configuration failed(%d) in cint_qax_test_fc_ilkn_inb_gl_gen_set_example\n", rv);
        return rv;
    }
    /* Configuring inb fc } */

    return rv;
}

int cint_qax_test_fc_ilkn_inb_gl_gen_unset_example(int unit, int ilkn_port, int inb_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;

    bcm_cosq_fc_endpoint_t fc_trigger; 
    bcm_cosq_fc_endpoint_t fc_handler;

    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;

    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_unset_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("Trigger global fc failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }

    /* Configuring inb fc { */
   sal_memset(&fc_trigger, 0, sizeof(fc_trigger));
   sal_memset(&fc_handler, 0, sizeof(fc_handler));
   
    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags &= ~(BCM_COSQ_FC_IS_OCB_ONLY);
            flags |= BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_ocb_db:
        case global_vsq_src_ocb_pdb:
            flags |= BCM_COSQ_FC_IS_OCB_ONLY | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_p0_db: 
        case global_vsq_src_p0_pd:
        case global_vsq_src_p0_byte:
            fc_trigger.cosq = 0;
            break;
        case global_vsq_src_p1_db: 
        case global_vsq_src_p1_pd:
        case global_vsq_src_p1_byte:
            fc_trigger.cosq = 1;
            break;
        case global_vsq_src_hdrm_db: 
        case global_vsq_src_hdrm_pd:
            flags |= BCM_COSQ_FC_HEADROOM | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        default: 
            return BCM_E_PARAM;
    }

    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags;
    fc_handler.flags = 0;
    /* setting priority */    
    fc_trigger.priority = priority;
	
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("INB fc generation configuration failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }
    /* Configuring inb fc } */

    return rv;
}
/* set fc generation(global resource) and fc target(a tx slot on ILKN MUB) */
int cint_jer_test_fc_mub_gl_gen_set_example(int unit, int ilkn_port, int mub_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;

    bcm_cosq_fc_endpoint_t fc_trigger; 
    bcm_cosq_fc_endpoint_t fc_handler;

    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;
    
    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_set_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in cint_arad_test_fc_oob_vsq_gl_gen_set_example\n", rv);
        return rv;
    }
    
    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags = BCM_COSQ_FC_BDB;
            break;
        case global_vsq_src_mini_db: 
            flags = BCM_COSQ_FC_MINI_DB;
            break;
        case global_vsq_src_full_db: 
            flags = BCM_COSQ_FC_FULL_DB;
            break;
        case global_vsq_src_ocb_db: 
            flags = BCM_COSQ_FC_OCB_DB;
            break;
        default: 
            return BCM_E_PARAM;
            break;
    }

    /* Configuring mub fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags | BCM_COSQ_FC_MASK_POOL_INDICATION;
    fc_handler.flags = BCM_COSQ_FC_MUB;
    /* setting priority */    
    fc_trigger.priority = priority;
    /* setting cosq */ 
    fc_trigger.cosq = 0;
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = mub_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("MUB fc generation configuration failed(%d) in cint_arad_test_fc_oob_vsq_gl_gen_set_example\n", rv);
        return rv;
    }
    /* Configuring mub fc } */

    return rv;
}    

int cint_jer_test_fc_mub_gl_gen_unset_example(int unit, int ilkn_port, int mub_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;

    bcm_cosq_fc_endpoint_t fc_trigger; 
    bcm_cosq_fc_endpoint_t fc_handler;

    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;
    
    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_unset_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in cint_arad_test_fc_oob_vsq_gl_gen_set_example\n", rv);
        return rv;
    }

    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags = BCM_COSQ_FC_BDB;
            break;
        case global_vsq_src_mini_db: 
            flags = BCM_COSQ_FC_MINI_DB;
            break;
        case global_vsq_src_full_db: 
            flags = BCM_COSQ_FC_FULL_DB;
            break;
        case global_vsq_src_ocb_db: 
            flags = BCM_COSQ_FC_OCB_DB;
            break;
        default: 
            return BCM_E_PARAM;
            break;
    }

    /* Configuring mub fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags | BCM_COSQ_FC_MASK_POOL_INDICATION;
    fc_handler.flags = BCM_COSQ_FC_MUB;
    /* setting priority */    
    fc_trigger.priority = priority;
    /* setting cosq */ 
    fc_trigger.cosq = 0;
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = mub_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("MUB fc generation configuration failed(%d) in cint_arad_test_fc_oob_vsq_gl_gen_set_example\n", rv);
        return rv;
    }
    /* Configuring mub fc } */

    return rv;
}    

int cint_qax_test_fc_ilkn_mub_gl_gen_set_example(int unit, int ilkn_port, int mub_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;
    bcm_cosq_fc_endpoint_t fc_trigger; 
    bcm_cosq_fc_endpoint_t fc_handler;
    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;

    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_set_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("Trigger global fc failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }

    /* Configuring inb fc { */
    sal_memset(&fc_trigger, 0, sizeof(fc_trigger));
    sal_memset(&fc_handler, 0, sizeof(fc_handler));

    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags &= ~(BCM_COSQ_FC_IS_OCB_ONLY);
            flags |= BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_ocb_db:
        case global_vsq_src_ocb_pdb:
            flags |= BCM_COSQ_FC_IS_OCB_ONLY | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_p0_db: 
        case global_vsq_src_p0_pd:
        case global_vsq_src_p0_byte:
            fc_trigger.cosq = 0;
            break;
        case global_vsq_src_p1_db: 
        case global_vsq_src_p1_pd:
        case global_vsq_src_p1_byte:
            fc_trigger.cosq = 1;
            break;
        case global_vsq_src_hdrm_db: 
        case global_vsq_src_hdrm_pd:
            flags |= BCM_COSQ_FC_HEADROOM | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        default: 
            return BCM_E_PARAM;
    }

    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags;
    fc_handler.flags = BCM_COSQ_FC_MUB;
    /* setting priority */    
    fc_trigger.priority = priority;
    /* setting cosq */ 
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = mub_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("INB fc generation configuration failed(%d) in cint_qax_test_fc_ilkn_inb_gl_gen_set_example\n", rv);
        return rv;
    }
    /* Configuring inb fc } */

    return rv;
}

int cint_qax_test_fc_ilkn_mub_gl_gen_unset_example(int unit, int ilkn_port, int mub_calendar_idx, int global_vsq_src, int priority, int shaped_port)
{
    int rv = BCM_E_NONE;
    int flags = 0;

    bcm_cosq_fc_endpoint_t fc_trigger; 
    bcm_cosq_fc_endpoint_t fc_handler;

    bcm_cosq_vsq_info_t vsq_inf;
    bcm_gport_t vsq_gport;

    /*Creating vsq*/
    vsq_inf.flags = BCM_COSQ_VSQ_GL;
    rv = bcm_cosq_gport_vsq_create(unit,&vsq_inf,&vsq_gport);
    if (rv != BCM_E_NONE) {
        printf("creating vsq failed(%d) in bcm_cosq_gport_vsq_create\n", rv);
        return rv;
    }

    /* Trigger global fc : Setting FC threshold and Shaping local port*/
    rv = cint_jer_test_fc_gl_trigger_unset_example(unit, global_vsq_src, priority, shaped_port);
    if (rv != BCM_E_NONE) {
        printf("Trigger global fc failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }

    /* Configuring inb fc { */
   sal_memset(&fc_trigger, 0, sizeof(fc_trigger));
   sal_memset(&fc_handler, 0, sizeof(fc_handler));
   
    switch (global_vsq_src) {
        case global_vsq_src_bdb:
            flags &= ~(BCM_COSQ_FC_IS_OCB_ONLY);
            flags |= BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_ocb_db:
        case global_vsq_src_ocb_pdb:
            flags |= BCM_COSQ_FC_IS_OCB_ONLY | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        case global_vsq_src_p0_db: 
        case global_vsq_src_p0_pd:
        case global_vsq_src_p0_byte:
            fc_trigger.cosq = 0;
            break;
        case global_vsq_src_p1_db: 
        case global_vsq_src_p1_pd:
        case global_vsq_src_p1_byte:
            fc_trigger.cosq = 1;
            break;
        case global_vsq_src_hdrm_db: 
        case global_vsq_src_hdrm_pd:
            flags |= BCM_COSQ_FC_HEADROOM | BCM_COSQ_FC_MASK_POOL_INDICATION;
            break;
        default: 
            return BCM_E_PARAM;
    }

    BCM_GPORT_LOCAL_SET(fc_handler.port, ilkn_port);
    fc_trigger.port = vsq_gport;
    /* setting flags */    
    fc_trigger.flags = flags;
    fc_handler.flags = BCM_COSQ_FC_MUB;
    /* setting priority */    
    fc_trigger.priority = priority;
	
    fc_handler.cosq = -1; /* cosq can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_handler.calender_index = mub_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlGeneration, &fc_trigger, &fc_handler);
    if (rv != BCM_E_NONE) {
        printf("INB fc generation configuration failed(%d) in cint_jer_test_fc_inb_gl_gen_set_example\n", rv);
        return rv;
    }
    /* Configuring inb fc } */

    return rv;
}
/*##############################################################*/
/*##################     INB RECEPTION     #####################*/
/*##############################################################*/

/* set fc reception(an rx slot on ILKN) and fc target(a port traffic) */
int cint_jer_test_fc_ilkn_inb_port_rec_set_example(int unit, int target_port, int ilkn_port, int inb_calendar_idx)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config inb fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_target.flags = fc_reception_port.flags = 0;
    /* setting cosq */    
    fc_target.cosq = -1;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("INB fc reception configuration failed(%d) in cint_jer_test_fc_inb_pfc_rec_set_example\n", rv);
    }
    /* Config inb fc } */

    return rv;
}

int cint_jer_test_fc_ilkn_inb_port_rec_unset_example(int unit, int target_port, int ilkn_port, int inb_calendar_idx)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config inb fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_target.flags = fc_reception_port.flags = 0;
    /* setting cosq */    
    fc_target.cosq = -1;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("INB fc reception configuration failed(%d) in cint_jer_test_fc_inb_pfc_rec_unset_example\n", rv);
    }
    /* Config inb fc } */

    return rv;
}

/* set fc reception(an rx slot on ILKN MUB) and fc target(a port traffic) */
int cint_jer_test_fc_mub_port_rec_set_example(int unit, int target_port, int ilkn_port, int mub_calendar_idx)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config mub fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_reception_port.flags = BCM_COSQ_FC_MUB;
    fc_target.flags = 0;
    /* setting cosq */    
    fc_target.cosq = -1;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = mub_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("MUB fc reception configuration failed(%d) in cint_jer_test_fc_mub_port_rec_set_example\n", rv);
    }
    /* Config mub fc } */

    return rv;
}

int cint_jer_test_fc_mub_port_rec_unset_example(int unit,int target_port, int ilkn_port, int mub_calendar_idx)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config mub fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port,ilkn_port);
    /* setting flags */    
	  fc_reception_port.flags = BCM_COSQ_FC_MUB;
    fc_target.flags = 0;
    /* setting cosq */    
    fc_target.cosq = -1;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index=mub_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("MUB fc reception configuration failed in cint_jer_test_fc_mub_port_rec_unset_example\n");
    }
    /* Config mub fc } */
    
    return rv;
}

/* set fc reception(an rx slot on ILKN) and fc target(a port priority traffic) */
int cint_jer_test_fc_ilkn_inb_pfc_rec_set_example(int unit, int target_port, int ilkn_port, int inb_calendar_idx, int target_cosq)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config inb fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_target.flags = fc_reception_port.flags = 0;
    /* setting cosq */    
    fc_target.cosq = target_cosq;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("INB fc reception configuration failed(%d) in cint_jer_test_fc_inb_pfc_rec_set_example\n", rv);
    }
    /* Config inb fc } */

    return rv;
}

int cint_jer_test_fc_ilkn_inb_pfc_rec_unset_example(int unit, int target_port, int ilkn_port, int inb_calendar_idx, int target_cosq)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config inb fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_target.flags = fc_reception_port.flags = 0;
    /* setting cosq */    
    fc_target.cosq = target_cosq;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("INB fc reception configuration failed(%d) in cint_jer_test_fc_inb_pfc_rec_unset_example\n", rv);
    }
    /* Config inb fc } */

    return rv;
}

/* set fc reception(an rx slot on ILKN MUB) and fc target(a port priority traffic) */
int cint_jer_test_fc_mub_pfc_rec_set_example(int unit, int target_port, int ilkn_port, int mub_calendar_idx, int target_cosq)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config mub fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_reception_port.flags = BCM_COSQ_FC_MUB;
    fc_target.flags = 0;
    /* setting cosq */    
    fc_target.cosq = target_cosq;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = mub_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("MUB fc reception configuration failed(%d) in cint_jer_test_fc_mub_pfc_rec_set_example\n", rv);
    }
    /* Config mub fc } */

    return rv;
}

int cint_jer_test_fc_mub_pfc_rec_unset_example(int unit,int target_port, int ilkn_port, int mub_calendar_idx, int target_cosq)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config mub fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port,ilkn_port);
    /* setting flags */    
	  fc_reception_port.flags = BCM_COSQ_FC_MUB;
    fc_target.flags = 0;
    /* setting cosq */    
    fc_target.cosq = target_cosq;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index=mub_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("MUB fc reception configuration failed in cint_jer_test_fc_mub_pfc_rec_unset_example\n");
    }
    /* Config mub fc } */
    
    return rv;
}

/* set fc reception(an rx slot on ILKN) and fc target(traffic specified by PFC bitmap) */
int cint_jer_test_fc_ilkn_inb_pfc_bmp_rec_set_example(int unit, int target_port, int ilkn_port, int inb_calendar_idx, int target_cosq)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config inb fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_target.flags = BCM_COSQ_FC_PORT_OVER_PFC;
	  fc_reception_port.flags = 0;
    /* setting cosq */    
    fc_target.cosq = target_cosq;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_add(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("INB fc reception configuration failed(%d) in cint_jer_test_fc_inb_pfc_rec_set_example\n", rv);
    }
    /* Config inb fc } */

    return rv;
}

int cint_jer_test_fc_ilkn_inb_pfc_bmp_rec_unset_example(int unit, int target_port, int ilkn_port, int inb_calendar_idx, int target_cosq)
{
    int rv = BCM_E_NONE;
    bcm_cosq_fc_endpoint_t fc_reception_port; 
    bcm_cosq_fc_endpoint_t fc_target;

    /* Config inb fc { */
    /* setting port */
    BCM_GPORT_LOCAL_SET(fc_target.port,target_port) /*Can be local, or modport etc..*/;
    BCM_GPORT_LOCAL_SET(fc_reception_port.port, ilkn_port);
    /* setting flags */
	  fc_target.flags = BCM_COSQ_FC_PORT_OVER_PFC;
	  fc_reception_port.flags = 0;
    /* setting cosq */    
    fc_target.cosq = target_cosq;
    fc_reception_port.cosq = -1; /* cosq for fc_reception can be 0 or -1, but -1 is recommended */
    /* setting calender index */
    fc_reception_port.calender_index = inb_calendar_idx;

    rv = bcm_cosq_fc_path_delete(unit, bcmCosqFlowControlReception, &fc_reception_port, &fc_target);
    if (rv != BCM_E_NONE) {
        printf("INB fc reception configuration failed(%d) in cint_jer_test_fc_inb_pfc_rec_unset_example\n", rv);
    }
    /* Config inb fc } */

    return rv;
}
