/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Multicast Options~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
 * $Id: cint_multicast_applications.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * File: cint_mutlicast_applications.c
 * Purpose: Example of multicast application for TM users.
 *
 * The Configuration enables creating two types of multicast groups:
 *      o   Port level
 *      o   Device level
 * 
 * In port-level scheduled MC, the configuration results in a fully scheduled MC scheme.
 * Its drawback is that it consumes the most resources
 * (that is, ingress memory MC linked-lists, VoQs, fabric utilization, and scheduling elements at the egress).
 * See cint_multicast_application_port_lvl_example.c for application-based example.
 * 
 * In the Device-level scheduled MC, configuration results in a partially scheduled MC scheme.
 * It consumes relatively few resources. It is a viable compromise between port-level scheduled MC
 * and Fabric MC. See cint_multicast_application_device_lvl_example.c for application-based example.
 * 
 * The MC configurations above require opening MC groups in the ingress and egress FAPs.
 * The following MC configuration types can be specified:
 *      o    Ingress Port-Level Scheduled: Sets the ingress FAP configuration for Port-Level Scheduled MC.
 *      o    Ingress Device-Level Scheduled: Sets the ingress FAP configuration for Device-Level Scheduled MC. 
 *
 * The Configuration assumes only one Soc_petra-B device and ERP port (for device level) is enabled.
 */

 
/*
 * General frame to create multicast group
 */
int create_multicast_group(int unit,int is_ingress,int is_egress,bcm_multicast_t * multicast_group) {
    bcm_error_t rv;
    uint32 flags;
    flags = BCM_MULTICAST_WITH_ID;

    if (is_ingress) {
        flags |= BCM_MULTICAST_INGRESS_GROUP;
    }
    if (is_egress) {
        flags |= BCM_MULTICAST_EGRESS_GROUP;
    }

    
    rv = bcm_multicast_create(unit,flags,multicast_group);      
    return rv;
}

/*
 * This application creates Ingress multicast with a set of {system ports or VODs + cuds),
 * The configuration is a fully scheduled MC Scheme. 
 * MC traffic is replicated at the ingress FAP to each destination member port. 
 * After the replication, each copy is placed in a VoQ destined to the target member port. 
 * Each copy is dequeued from the VoQ upon reception of a credit from the target port. 
 * destinations parameter can be gport destination: system port, VOQ, FMQ
 */
int port_level_scheduled_multicast(int unit,int multicast_id,bcm_gport_t* destinations,int* cuds,int nof_destinations)
{
    bcm_error_t rv = BCM_E_NONE;
    int is_ingress = 1;
    int is_egress = 0;
    bcm_multicast_t multicast_group = multicast_id;
    int index;
    
    printf("adding group 0x%x:\n",multicast_id);
    rv = create_multicast_group(unit,is_ingress,is_egress,&multicast_group);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }

    printf("adding group entries:\n");
    for (index = 0; index < nof_destinations; index++) {        
        printf("adding dest 0x%x cud 0x%x\n", destinations[index],cuds[index]);
        rv = bcm_multicast_ingress_add(unit,multicast_group,destinations[index],cuds[index]);
        if (rv != BCM_E_NONE) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        } 
    }   

    return rv;
}

/*    
 * This application creates Ingress+Egress multicast that replicates in ingress a single
 * replication to each destination FAP (holding a member port in that MC group). 
 * Those FAP-level replicated copies are sent to the Egress Replication Port (ERP) 
 * of the appropriate FAP. At the Egress the MC traffic is further replicated to 
 * each member port using egress multicast.
 * The configuration is a partially scheduled MC Scheme.   
 * parameters destinations defined as system-ports/modports/local ports
 *
 * This function does not support a multiple device system.
 */
int device_level_scheduled_multicast(int unit,int multicast_id,bcm_gport_t* destinations,int* cuds,int nof_destinations)
{
    bcm_error_t rv = BCM_E_NONE;
    int is_ingress = 1;
    int is_egress = 1;
    bcm_multicast_t multicast_group = multicast_id;
    int index;        
    int port_max = nof_destinations;
    bcm_gport_t dest_erp;
    /* hashing of module IDs is used to check if a replication was already added to ingress multicast for the module ID */
    int sizeof_hash = 16, hash_empty_val = -1;
    int modid, modid_hash[sizeof_hash];
    bcm_gport_t temp[2];
    int port_erp;
    int count_erp;
    int port;
    int cud;
    int num_cores = is_device_or_above(unit, JERICHO) ? 2:1;

    
    
    if (multicast_id < 4096) {
        /* First 4K multicast entries are opened by default */
        is_egress = 0;
        rv = bcm_multicast_egress_set(unit, multicast_id,0, 0, 0); /* If we assume that the egress group exists, remove any existing replication */
        if (rv != BCM_E_NONE) {
            printf("failed to configure egress MC group %d which is expected to exist to no replications. (%s) \n", multicast_id, bcm_errmsg(rv));
            return rv;
        }
    }
    printf("step 1: create_multicast_group \n");
    rv = create_multicast_group(unit,is_ingress,is_egress,&multicast_group);
    if (rv != BCM_E_NONE) {
        printf("(%s) \n",bcm_errmsg(rv));
        return rv;
    }
    /* init hash of module IDs used to check if we configured (added ingress multicast replication to) a module ID */
    for (index = 0; index < sizeof_hash; index++) modid_hash[index] = hash_empty_val;

    printf("step 2: For each port, find his module id, and send destination accordingly \n");

     /* CUD on multicast ingress must be the multicast egress group id */
     cud = multicast_id;             

    /* Multicast ingress - FAP level destionation (ERP) */
    /* For each port, find his module id, and send destination accordingly */
    for (index = 0; index < nof_destinations; index++) {
         /* get a modport gport for the port, for getting the module ID of the port */
         rv = bcm_stk_sysport_gport_get(unit,destinations[index],&(temp[0]));
         if (rv != BCM_E_NONE) {
             printf("(%s) \n",bcm_errmsg(rv));
             return rv;
         }
         modid = BCM_GPORT_MODPORT_MODID_GET(temp[0]);

         /* If we already configured this modid, skip adding it's ERP to the ingress multicast group */
         if (modid_hash[modid % sizeof_hash] != hash_empty_val) {
             if (modid_hash[modid % sizeof_hash] != modid) {
                 printf("modid hash conflict between modids %d and %d\n", modid, modid_hash[modid % sizeof_hash]);
                 return -111;
             }
             continue;
         }
         modid_hash[modid % sizeof_hash] = modid;

         /* Call ERP system port */      
         rv = bcm_port_internal_get(unit,BCM_PORT_INTERNAL_EGRESS_REPLICATION,1*num_cores,temp,&count_erp);
         if (rv != BCM_E_NONE) {
             printf("(%s) \n",bcm_errmsg(rv));
             return rv;
         }

         if (count_erp == 0) {
             printf("ERP is not enable, Multicast creation failed \n");
             bcm_multicast_destroy(unit,multicast_group);
             return BCM_E_PARAM;
         }
         /*In Jericho temp hold two ERP-ports - one of each core, use the ERP port relevant for the destination-port's core.*/
         rv = bcm_stk_gport_sysport_get(unit,temp[modid % num_cores],&dest_erp);
         if (rv != BCM_E_NONE) {
             printf("(%s) \n",bcm_errmsg(rv));
             return rv;
         }

         rv = bcm_multicast_ingress_add(unit,multicast_group,dest_erp,cud);
         if (rv != BCM_E_NONE) {
             printf("(%s) \n",bcm_errmsg(rv));
             return rv;
         }
    }  

    printf("step 3: Multicast egress, for each port add destination on egress device \n");
    /* Multicast egress, for each port add destination on egress device */
    for (index = 0; index < nof_destinations; index++) {
        /* CUD on multicast ingress must be the multicast egress group id */
        cud = cuds[index];                             
        rv = bcm_multicast_egress_add(unit, multicast_group, destinations[index], cud);
        if (rv != BCM_E_NONE && rv != BCM_E_EXISTS) {
            printf("(%s) \n",bcm_errmsg(rv));
            return rv;
        }      
    }              
       
    return rv;
}




