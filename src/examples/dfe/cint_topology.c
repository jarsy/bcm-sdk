/* 
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DFE topology example example:
 * 
 * The example simulate:
 *  1. set topology for local module
 *  2. set topology for faps group
 *
 */


/*set topology for local module*/
int
local_mapping_set(int unit) {
    int rv;
    bcm_port_t links_array[3];
    bcm_module_t local_module;

    /*define local module id*/
    local_module = 1;
    BCM_FABRIC_LOCAL_MODID_SET(local_module);
    
    /*map local module id with module id*/ 
    rv = bcm_fabric_modid_local_mapping_set(unit, local_module, 512);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_fabric_modid_local_mapping_set, rv=%d, \n", rv);
        return rv;
    }

    /*select links*/
    links_array[0] = 1;
    links_array[1] = 2;
    links_array[2] = 3;

    /*set topology using module id*/
    rv = bcm_fabric_link_topology_set(unit, 512, 3, links_array); 
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_fabric_link_topology_set, rv=%d, \n", rv);
        return rv;
    }

    /*define second local module id*/
    local_module = 2;                                              
    BCM_FABRIC_LOCAL_MODID_SET(local_module);

    /*map local module id with module id*/ 
    rv = bcm_fabric_modid_local_mapping_set(unit, local_module, 25);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_fabric_modid_local_mapping_set, rv=%d, \n", rv);
        return rv;
    }

    /*select links*/
    links_array[0] = 4;
    links_array[1] = 5;
    links_array[2] = 6;

    /*set topology using local module id*/
    rv = bcm_fabric_link_topology_set(unit, local_module, 3, links_array); /*with local modid*/
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_fabric_link_topology_set, rv=%d, \n", rv);
        return rv;
    }

    printf("local_mapping_set: PASS\n");
    return BCM_E_NONE;
}

/*set topology for faps group*/
int
faps_group_set(int unit) {
    int rv;
    bcm_module_t modid_array[3];
    bcm_port_t links_array[3];
    bcm_module_t g;

    /*define faps group*/
    g = 1;
    BCM_FABRIC_GROUP_MODID_SET(g);

    /*select module ids*/
    modid_array[0] = 1;
    modid_array[1] = 2;
    modid_array[2] = 3;

    /*map faps groups with selected module ids*/
    rv = bcm_fabric_modid_group_set(unit,  g, 3, modid_array);
    if (rv != BCM_E_NONE) {
        printf("Error, in  bcm_fabric_modid_group_set, rv=%d, \n", rv);
        return rv;
    }

    /*select links*/
    links_array[0] = 1;
    links_array[1] = 2;
    links_array[2] = 3;

    /*set faps group topology*/
    rv = bcm_fabric_link_topology_set(unit, g, 3, links_array); /*with local modid*/
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_fabric_link_topology_set, rv=%d, \n", rv);
        return rv;
    }
    
    printf("faps_group_set: PASS\n");
    return BCM_E_NONE;
}

