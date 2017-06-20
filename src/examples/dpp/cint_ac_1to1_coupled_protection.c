/* $Id: cint_ac_1to1_coupled_protection.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/* **************************************************************************************************************************************************
 * Set following SOC properties:                                                                                                                    *
 * Configure the device to be in Coupled Egress Protection (As the default)                                                                         *
 *                                                                                                                                                  *
 * The CINT provides two protection examples:                                                                                                       *
 *                                                                                                                                                  *
 * Example 1:                                                                                                                                       *
 * ==========                                                                                                                                       *
 * AC 1:1 Protection for UC & MC traffic.                                                                                                           *
 *                                                                                                                                                  *
 * Packets can be sent from the In-AC towards the Out-ACs:                                                                                          *
 *   UC - Unicast traffic is achieved by using the FEC Protection and sending a packet with a known DA.                                             *
 *   MC - Multicast Protection is achieved by defining a MC group an using Egress Protection for unknown DA packets.                                *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 *                                                  ________________________________                                                                *
 *                                                 |                   _____        |                                                               *
 *                           In-AC                 |                  |     | ----> | -----                                                         *
 *                               ----------------> | -   -   -   -  > | FEC |       |      |                                                        *
 *                                                 |                  |_____| ----> | ---  |                                                        *
 *                                                 |                                |    | |                                                        *
 *                                                 |                                |    | |                                                        *
 *                           Working Out-AC        |      ______                    |    | |                                                        *
 *                               <---------------- | < --|  Eg  | -   -   -   -  -  | <--  |                                                        *
 *                           Protecting Out-AC     |     | Prot |                   |      |                                                        *
 *                               <---------------- | < --|______| -   -   -   -  -  | <----                                                         *
 *                                                 |                                |                                                               *
 *                                                 |________________________________|                                                               *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 * Example 2:                                                                                                                                       *
 * ==========                                                                                                                                       *
 * AC 1:1 & 1+1 Protection for UC traffic                                                                                                           *
 *                                                                                                                                                  *
 * Packets can be sent from the In-AC towards the Out-ACs (FEC failover connection)                                                                 *
 * Packets can be sent from the Out-ACs (Ingress failover connection) towards the In-AC                                                             *
 *                                                                                                                                                  *
 *                                                  ________________________________                                                                *
 *                                                 |                   _____        |                                                               *
 *                                      In-AC      |                  |     | ----> |- - > Working In/Out-AC                                        *
 *                               ----------------> | ---------------> | FEC |       |                                                               *
 *                                                 |                  |_____| ----> |- - > Protecting In/Out-AC                                     *
 *                                                 |                                |                                                               *
 *                             Working In/Out-AC   |     _____                      |                                                               *
 *                                ---------------> |--->| In  |\                    |                                                               *
 *                                                 |    | LIF | \                   |                                                               *
 *                                                 |    |_____|   --------------->  | - - - -> In-AC                                                *
 *                          Protecting In/Out-AC   |    | In  | /                   |                                                               *
 *                                ---------------> |--->| LIF |/                    |                                                               *
 *                                                 |    |_____|                     |                                                               *
 *                                                 |________________________________|                                                               *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 * Configuration:                                                                                                                                   *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 * run:                                                                                                                                             *
 * cd ../../../../src/examples/dpp                                                                                                                  *
 * cint utility/cint_utils_global.c                                                                                                                 *
 * cint utility/cint_utils_l2.c                                                                                                                     *
 * cint utility/cint_utils_multicast.c                                                                                                              *
 * cint utility/cint_multi_device_utils.c                                                                                                           *
 * cint cint_ac_1to1_coupled_protection.c.c                                                                                                         *
 * cint                                                                                                                                             *
 * int unit = 0;                                                                                                                                    *
 * ac_1to1_coupled_protection__start_run(int unit,  ac_1to1_coupled_protection_s *param) - param = NULL for default params                          *
 *                                                                                                                                                  *
 *                                                                                                                                                  *
 * For Example 1 set ac_1to1_coupled_protection->is_egress = 1 (default)                                                                            *
 * For Example 2 set ac_1to1_coupled_protection->is_egress = 0                                                                                      *
 *                                                                                                                                                  *
 * All default values could be re-written with initialization of the global structure 'g_ac_1to1_coupled_protection', before calling the main       *
 * function. In order to re-write only part of the values, call 'ac_1to1_coupled_protection_struct_get(ac_1to1_coupled_protection_s)'               *
 * function and re-write the values prior to calling the main function.                                                                             *
 ************************************************************************************************************************************************** */
  
 
/* **************************************************************************************************
  --------------          Global Variables Definition and Initialization  START     -----------------
 **************************************************************************************************** */
int FAILOVER_PROTECTING=0;
int FAILOVER_WORKING=1;
int NOF_FAILOVER_IDS=2;

struct protection_ac_s {
    bcm_gport_t port;
    bcm_vlan_t  vlan;
    bcm_gport_t vlan_port_id;
    bcm_gport_t encap_id;
};

/*  Main Struct  */
struct ac_1to1_coupled_protection_s {
    protection_ac_s in_ac;
    protection_ac_s out_ac[NOF_FAILOVER_IDS];
    bcm_mac_t mac_address_out;
    bcm_mac_t mac_address_in;
    int is_egress;
    bcm_vlan_t vsi;
    bcm_failover_t fec_failover_id;
    bcm_failover_t eg_in_failover_id;
};


/* Initialization of global struct*/
ac_1to1_coupled_protection_s g_ac_1to1_coupled_protection = { 

        /*  AC configurations
            Phy Port    VLAN    vlan_port_id    encap_id   */
            { 200,      9,         0x44800000, 0   }, /* In-AC  */
           {{ 201,      6,         0x44001001, 0   }, /* Out-ACs */
            { 202,      7,         0x44001000, 0   }},

            /* Additional parameters */
            {0x00, 0x11, 0x00, 0x00, 0x00, 0x13},   /* MAC Address Out */
            {0x00, 0x33, 0x00, 0x00, 0x00, 0x33},   /* MAC Address In */
            1,                                      /* Is Egress */
            20,                                     /* VSI */
            1,                                      /* FEC failover_id */
            2};                                     /* Egress/Ingress failover_id */



/* **************************************************************************************************
  --------------          Global  Variables Definitions and Initialization  END       ---------------
 **************************************************************************************************** */

/* Initialization of the main struct
 * Function allows the re-write of default values, SOC Property validation and
 * other general operation such as VSI creation.
 *
 * INPUT: 
 *   params: new values for g_ac_1to1_coupled_protection
 */
int ac_1to1_coupled_protection_init(int unit, ac_1to1_coupled_protection_s *params) {

    int rv, flags;
    bcm_multicast_t mc_group;

    if (params != NULL) {
       sal_memcpy(&g_ac_1to1_coupled_protection, params, sizeof(g_ac_1to1_coupled_protection));
    }

    /* LIF idx must be greater than nof RIFs */
    g_ac_1to1_coupled_protection.in_ac.vlan_port_id += SOC_DPP_DEFS_MAX_NOF_RIFS();

    /* Init for the failover module */
    rv = bcm_failover_init(unit);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection_init failed during bcm_failover_init, rv - %d\n", rv);
        return rv;

    }

    /* Create a FEC failover ID for UC */
    rv = bcm_failover_create(unit, (BCM_FAILOVER_WITH_ID | BCM_FAILOVER_FEC), &g_ac_1to1_coupled_protection.fec_failover_id);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection_init failed during FEC bcm_failover_create, rv - %d\n", rv);
        return rv;

    }

    /* Create Egress/Ingress failover ID */
    rv = bcm_failover_create(unit, (BCM_FAILOVER_WITH_ID | (params->is_egress ? BCM_FAILOVER_ENCAP : BCM_FAILOVER_INGRESS)), &g_ac_1to1_coupled_protection.eg_in_failover_id);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection_init failed during Ingress/Egress bcm_failover_create, rv - %d\n", rv);
        return rv;

    }

    /* Create a vswitch */
    rv = bcm_vswitch_create_with_id(unit, g_ac_1to1_coupled_protection.vsi);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection_init failed during bcm_vswitch_create for vsi %d, rv - %d\n", g_ac_1to1_coupled_protection.vsi, rv);
        return rv;
    }

    /* Create MC Group */
	mc_group = g_ac_1to1_coupled_protection.vsi;
    rv = multicast__open_mc_group(unit, &mc_group, BCM_MULTICAST_TYPE_L2);
    if (rv != BCM_E_NONE) {
        printf("Error, multicast__open_mc_group, rv - %d\n");
        return rv;
    }

    return BCM_E_NONE;
}


/* Configuration of a protection In-AC.
 * Define the In-AC and attach it to a vswitch.
 *
 * INPUT: 
 *   protection_ac_s: Configuration info for a single In-AC.
 */
int ac_1to1_coupled_protection__set_in_ac(int unit, ac_1to1_coupled_protection_s *ac_1to1_coupled_protection_info) {

    bcm_vlan_port_t in_ac;
    bcm_if_t encap_id;
    int rv;

    bcm_vlan_port_t_init(&in_ac);
    in_ac.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    in_ac.flags = BCM_VLAN_PORT_WITH_ID;
    in_ac.port = ac_1to1_coupled_protection_info->in_ac.port;
    in_ac.match_vlan = ac_1to1_coupled_protection_info->in_ac.vlan;
    in_ac.vsi = ac_1to1_coupled_protection_info->vsi;
    in_ac.vlan_port_id = ac_1to1_coupled_protection_info->in_ac.vlan_port_id;
    rv = bcm_vlan_port_create(unit, &in_ac);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection__set_in_ac failed during bcm_vlan_port_create, rv - %d\n", rv);
        return rv;

    }
    ac_1to1_coupled_protection_info->in_ac.vlan_port_id = in_ac.vlan_port_id;

    /* Attach the In-AC to the vswitch */
    rv = bcm_vswitch_port_add(unit, in_ac.vsi, in_ac.vlan_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection__set_in_ac failed during bcm_vswitch_port_add for vsi %d, rv - %d\n", in_ac.vsi, rv);
        return rv;
    }
    printf("In-AC = %d, encap_id - %d\n", in_ac.vlan_port_id, in_ac.encap_id);
    return BCM_E_NONE;
}



/* Configuration of the protection Out-ACs.
 * Define the Working & Protecting Out-ACs and attach them to a vswitch and MC group.
 * The ACs are defined with the appropriate failover_ids for FEC & Egress Protection.
 * INPUT: 
 *   protection_ac_s: Configuration info for a couple of protection Out-ACs.
 */
int ac_1to1_coupled_protection__set_protection_acs(int unit, ac_1to1_coupled_protection_s *ac_1to1_coupled_protection_info) {

    bcm_vlan_port_t out_ac;
    bcm_vlan_port_t out_ac_info;
    bcm_if_t encap_id;
    int rv, mc_group;
    int check_pass = 1;

    /* Create the Protecting Out-AC */
    bcm_vlan_port_t_init(&out_ac);
    out_ac.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    out_ac.flags = BCM_VLAN_PORT_WITH_ID;
    out_ac.port = ac_1to1_coupled_protection_info->out_ac[FAILOVER_PROTECTING].port;
    out_ac.match_vlan = ac_1to1_coupled_protection_info->out_ac[FAILOVER_PROTECTING].vlan;
    out_ac.vsi = ac_1to1_coupled_protection_info->vsi;
    out_ac.failover_id = ac_1to1_coupled_protection_info->fec_failover_id;
    out_ac.vlan_port_id = ac_1to1_coupled_protection_info->out_ac[FAILOVER_PROTECTING].vlan_port_id;

    if (ac_1to1_coupled_protection_info->is_egress) {
        out_ac.flags |= BCM_VLAN_PORT_EGRESS_PROTECTION;
        out_ac.egress_failover_id = ac_1to1_coupled_protection_info->eg_in_failover_id;
    } else {
        out_ac.ingress_failover_id = ac_1to1_coupled_protection_info->eg_in_failover_id;
    }

    rv = bcm_vlan_port_create(unit, &out_ac);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection__set_protection_acs failed during Protecting Out-AC creation, rv - %d\n", rv);
        return rv;
    }

    ac_1to1_coupled_protection_info->out_ac[FAILOVER_PROTECTING].vlan_port_id = out_ac.vlan_port_id;
    ac_1to1_coupled_protection_info->out_ac[FAILOVER_PROTECTING].encap_id = out_ac.encap_id;

    /* Attach the Protecting Out-AC to the vswitch */
    rv = bcm_vswitch_port_add(unit, out_ac.vsi, out_ac.vlan_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection__set_protection_acs failed during Protecting bcm_vswitch_port_add for vsi %d, rv - %d\n", out_ac.vsi, rv);
        return rv;
    }

    /* Attach to a MC group */
    mc_group = out_ac.vsi;
    rv = multicast__vlan_port_add(unit, mc_group, out_ac.port, out_ac.vlan_port_id, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, multicast__vlan_port_add\n");
        return rv;
    }

    printf("Out-AC Protecting = %d, encap_id - %d\n", out_ac.vlan_port_id, out_ac.encap_id);

    /* Create the Working Out-AC */
    bcm_vlan_port_t_init(&out_ac);
    out_ac.flags = BCM_VLAN_PORT_WITH_ID;
    out_ac.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    out_ac.port = ac_1to1_coupled_protection_info->out_ac[FAILOVER_WORKING].port;
    out_ac.match_vlan = ac_1to1_coupled_protection_info->out_ac[FAILOVER_WORKING].vlan;
    out_ac.vsi = ac_1to1_coupled_protection_info->vsi;
    out_ac.vlan_port_id = ac_1to1_coupled_protection_info->out_ac[FAILOVER_WORKING].vlan_port_id;
    out_ac.failover_id = ac_1to1_coupled_protection_info->fec_failover_id;
    out_ac.failover_port_id = ac_1to1_coupled_protection_info->out_ac[FAILOVER_PROTECTING].vlan_port_id;

    if (ac_1to1_coupled_protection_info->is_egress) {
        out_ac.flags |= BCM_VLAN_PORT_EGRESS_PROTECTION;
        out_ac.egress_failover_id = ac_1to1_coupled_protection_info->eg_in_failover_id;
        out_ac.egress_failover_port_id = ac_1to1_coupled_protection_info->out_ac[FAILOVER_PROTECTING].encap_id;
    } else {
        out_ac.ingress_failover_id = ac_1to1_coupled_protection_info->eg_in_failover_id;
    }
    rv = bcm_vlan_port_create(unit, &out_ac);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection__set_protection_acs failed during Working Out-AC creation, rv - %d\n", rv);
        return rv;
    }

    /*Find the working Out-AC out*/
    bcm_vlan_port_t_init(&out_ac_info);
    out_ac_info.vlan_port_id = out_ac.vlan_port_id;
    rv = bcm_vlan_port_find(unit, &out_ac_info);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection__set_protection_acs failed during getting Working Out-AC, rv - %d\n", rv);
        return rv;
    }

    if (out_ac.flags & BCM_VLAN_PORT_EGRESS_PROTECTION) {
        if (!(out_ac_info.flags & BCM_VLAN_PORT_EGRESS_PROTECTION)) {
            printf("Error, ac_1to1_coupled_protection__set_protection_acs failed,get Working Out-AC flags failed");
            check_pass = 0;
        }
        if (!out_ac.egress_failover_port_id != !out_ac_info.egress_failover_port_id) {
            printf("Error, ac_1to1_coupled_protection__set_protection_acs failed,get Working Out-AC faolover port failed");
            check_pass = 0;
        }
    }
    if (check_pass == 0) {
        printf("Error! Out-Ac info is not the same between creation and getting.\n");
        printf("Out-Ac information used in creation: \n");
        print out_ac;
        printf("Out-Ac information got after creating: \n");
        print out_ac_info;
        assert(FALSE);
    }

    ac_1to1_coupled_protection_info->out_ac[FAILOVER_WORKING].vlan_port_id = out_ac.vlan_port_id;
    ac_1to1_coupled_protection_info->out_ac[FAILOVER_WORKING].encap_id = out_ac.encap_id;

    /* Attach the Working Out-AC to the vswitch */
    rv = bcm_vswitch_port_add(unit, out_ac.vsi, out_ac.vlan_port_id);
    if (rv != BCM_E_NONE) {
        printf("Error, ac_1to1_coupled_protection__set_protection_acs failed during Working bcm_vswitch_port_add for vsi %d, rv - %d\n", out_ac.vsi, rv);
        return rv;
    }

    /* Attach to a MC group */
    mc_group = out_ac.vsi;
    rv = multicast__vlan_port_add(unit, mc_group, out_ac.port, out_ac.vlan_port_id, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, multicast__vlan_port_add\n");
        return rv;
    }

    printf("Out-AC Working = %d, encap_id - %d\n", out_ac.vlan_port_id, out_ac.encap_id);

    return BCM_E_NONE;
}


/* Retrieve the initial confgiuration for the example.
 *
 * INPUT: 
 *   protection_ac_s: Configuration info for a single In-AC.
 */
void ac_1to1_coupled_protection__struct_get(ac_1to1_coupled_protection_s *param){

    sal_memcpy(param, &g_ac_1to1_coupled_protection, sizeof(*param));
    return;
}


/* Main function that performs all the configuarations for the example.
 * Performs init configuration before configuring the ACs and adding a static MAC 
 * entry for the UC packets. 
 * INPUT: 
 *   protection_ac_s: Configuration info for running the example.
 */
int ac_1to1_coupled_protection__start_run(int *unit_ids, int nof_units, ac_1to1_coupled_protection_s *params) {

    int rv,  unit, i;
    l2__mact_properties_s mact_properties;

    for (i = 0 ; i < nof_units ; i++) {
        unit = units_ids[i];

        /* Initialize the test parameters */
        rv = ac_1to1_coupled_protection_init(unit, params);
        if (rv != BCM_E_NONE) {
            printf("Error in ac_1to1_coupled_protection_init, rv - %d\n", rv);
            return rv;
        }

        /* Configure the In-AC */
        rv = ac_1to1_coupled_protection__set_in_ac(unit, &g_ac_1to1_coupled_protection);
        if (rv != BCM_E_NONE) {
            printf("Error in ac_1to1_coupled_protection__start_run, failed to create an In-AC, rv - %d\n", rv);
            return rv;
        }

        /* Configure the protection Out-ACs */
        rv = ac_1to1_coupled_protection__set_protection_acs(unit, &g_ac_1to1_coupled_protection);
        if (rv != BCM_E_NONE) {
            printf("Error in ac_1to1_coupled_protection__start_run, failed to create an Out-AC, rv - %d\n", rv);
            return rv;
        }

        /* Add a MACT entry */
        mact_properties.gport_id = g_ac_1to1_coupled_protection.out_ac[FAILOVER_PROTECTING].vlan_port_id;
        mact_properties.mac_address = g_ac_1to1_coupled_protection.mac_address_out;
        mact_properties.vlan = g_ac_1to1_coupled_protection.vsi;

        rv = l2__mact_entry_create(unit, &mact_properties);
        if (rv != BCM_E_NONE) {
            printf("Error in ac_1to1_coupled_protection__start_run, failed to add a MACT entry, rv - %d\n", rv);
            return rv;
        }

        /* Add a MACT entry */
        mact_properties.gport_id = g_ac_1to1_coupled_protection.in_ac.vlan_port_id;
        mact_properties.mac_address = g_ac_1to1_coupled_protection.mac_address_in;
        mact_properties.vlan = g_ac_1to1_coupled_protection.vsi;

        rv = l2__mact_entry_create(unit, &mact_properties);
        if (rv != BCM_E_NONE) {
            printf("Error in ac_1to1_coupled_protection__start_run, failed to add a MACT entry, rv - %d\n", rv);
            return rv;
        }
    }

    return BCM_E_NONE;
}


/* This function runs the main test function with specified ports
 *  
 * INPUT: in_port  - ingress port 
 *        out_port - egress port
 */
int ac_1to1_coupled_protection_with_ports__start_run(int *unit_ids, int nof_units, int in_port, int out_port_working, int out_port_protecting, int is_egress) {
    int rv;

    ac_1to1_coupled_protection_s param;

    ac_1to1_coupled_protection__struct_get(&param);

    param.in_ac.port = in_port;
    param.out_ac[FAILOVER_WORKING].port = out_port_working;
    param.out_ac[FAILOVER_PROTECTING].port = out_port_protecting;
    param.is_egress = is_egress;

    return ac_1to1_coupled_protection__start_run(unit_ids, nof_units, &param);
}


/* Performs failover setting for the two protection Out-ACs
 *  
 * INPUT: is_fec_protection  - Determine if the setting is for FEC-Protection or Egress/Ingress Protection
 *        is_enable - The failover state to set. 1: Working Path, 0: Protecting Path.
 */
int ac_1to1_coupled_protection_with_ports__failover_set(int *unit_ids, int nof_units, int is_fec_protection, int is_enable) {

    int rv, i, unit;
    bcm_failover_t  failover_id = (is_fec_protection) ? g_ac_1to1_coupled_protection.fec_failover_id : g_ac_1to1_coupled_protection.eg_in_failover_id;

    for (i = 0 ; i < nof_units ; i++) {
        unit = units_ids[i];
        rv = bcm_failover_set(unit, failover_id, is_enable);    
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_failover_set failover_id:  0x%08x, is_fec: %d, is_enable: %d \n", failover_id, is_fec_protection, is_enable);
            return rv;
        }
    }
}

int ac_1to1_coupled_protection__cleanup(int *unit_ids, int nof_units) {

    int rv, i, unit;

    for (i = 0 ; i < nof_units ; i++) {

        unit = units_ids[i];

        rv = bcm_vlan_port_destroy(unit, g_ac_1to1_coupled_protection.in_ac.vlan_port_id);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_vlan_port_destroy gport:  0x%08x\n", g_ac_1to1_coupled_protection.in_ac.vlan_port_id);
            return rv;
        }

        rv = bcm_vlan_port_destroy(unit, g_ac_1to1_coupled_protection.out_ac[FAILOVER_PROTECTING].vlan_port_id);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_vlan_port_destroy gport:  0x%08x\n", g_ac_1to1_coupled_protection.out_ac[FAILOVER_PROTECTING].vlan_port_id);
            return rv;
        }

        rv = bcm_vlan_port_destroy(unit, g_ac_1to1_coupled_protection.out_ac[FAILOVER_WORKING].vlan_port_id);
        if (rv != BCM_E_NONE) {
            printf("Error, in bcm_vlan_port_destroy gport:  0x%x\n", g_ac_1to1_coupled_protection.out_ac[FAILOVER_WORKING].vlan_port_id);
            return rv;
        }
    }

    return rv;
}
