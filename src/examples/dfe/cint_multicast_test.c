/*
 * $Id: cint_multicast_test.c,v 1.3 Broadcom SDK $ 
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Multicast test application
 * Cint shows two examples of multicast applications: direct and indirect
 * Direct: set multicast table.
 * Indirect: set multicast table and create a static topology. An example of such multicast mode will be
 * to work with module ids that higher than 127.
 * 
 * Focal funtions:
 * o    multicast_test__fabric_direct_mode_set()
 * o    multicast_test__fabric_indirect_mode_set()
 */


/* 
 * Create multicast id, and set fabric to it.
 */
int multicast_test__fabric_set(int fe_unit, int fabric_multicast_id, bcm_module_t *modid_array, int nof_modules)
{
    int rv = BCM_E_NONE;

    rv = bcm_multicast_create(fe_unit, BCM_MULTICAST_WITH_ID, &fabric_multicast_id);
    if(rv != 0) {
        printf("Error, in bcm_multicast_create, rv=%d, \n", rv);
        return rv;
    }

    rv = bcm_fabric_multicast_set(fe_unit, fabric_multicast_id, 0, nof_modules, modid_array);
    if(rv != 0) {
        printf("Error, in bcm_fabric_multicast_set, rv=%d, \n", rv);
        return rv;
    }

    return rv;
}

/* 
 * Create multicast id, and set egress of unit to it.
 */ 
int multicast_test__egress_set(int unit, int mc_id, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    bcm_gport_t local_gports;
    bcm_if_t encap=1;

    rv = bcm_multicast_create(unit, BCM_MULTICAST_EGRESS_GROUP | BCM_MULTICAST_WITH_ID, &mc_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_create (%d)\n", unit);
        return rv;
    }
    BCM_GPORT_LOCAL_SET(&(local_gports), port);
    rv = bcm_multicast_egress_set(unit, mc_id, 1, &local_gports, &encap);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_multicast_egress_set (%d)\n", unit);
        return rv;
    }

    return rv;
}

/* 
 * Set multicast direct application. 
 * Given multicast id, create and replicate multicast id over all requested devices. 
 */
int multicast_test__fabric_direct_mode_set(int fe_unit, int fabric_multicast_id, bcm_module_t* modid_array, int nof_modules)
{
    int rv = BCM_E_NONE;
    int tmp,local_modid;

    /* map local modid with modid */
    for (local_modid = 0; local_modid < nof_modules; local_modid++)
    {
        tmp = local_modid;
        BCM_FABRIC_LOCAL_MODID_SET(tmp);
        
        rv = bcm_fabric_modid_local_mapping_set(fe_unit, tmp, modid_array[local_modid]);
        if(rv != 0) {
            printf("Error, in bcm_fabric_modid_local_mapping_set, rv=%d, \n", rv);
            return rv;
        }
    }

    rv = multicast_test__fabric_set(fe_unit, fabric_multicast_id, modid_array, nof_modules);
    if(rv != 0) {
        printf("Error, in multicast_test__fabric_set, rv=%d, \n", rv);
        return rv;
    }

    return rv;
}
