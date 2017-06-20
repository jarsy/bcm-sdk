/* $Id: cint_mact_bulk_operations.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/*
 * demonstrate bulk operation on MACT (using flush DB)
 *  
 * 1. mact_bulk_group_traverse_example:
 *      - dump/traverse MACT table according to group (user defined value)
 *      - group is user define value set when adding static entries
 *      - value:
 *          - 88650: 1-7 groups. group 0: not assigned to group (backward comp)
 * 2. mact_bulk_multiple_rules_example:
 *      - perform several rules in one flush traverse
 *      - MAX number of rules
 *          - 88650: up to 8 rules.
 *  
 * 3. mact_bulk_rule_mask_vlan_example mact_bulk_rule_mask_vlan_port_example: 
 *      - delete L2 entries according to rule that include value and MASK
 */

int verbose;
verbose = 2;

/* call back to print entry includeing the group */
int call_back_get_block(int unit, bcm_l2_addr_t *l2e, void* ud) {
    printf("| %02x:%02x:%02x:%02x:%02x:%02x |",
		 l2e->mac[0],l2e->mac[1],l2e->mac[2],
		 l2e->mac[3],l2e->mac[4],l2e->mac[5]);
    printf(" VLAN=0x%04x | PORT=0x%08x |", l2e->vid, l2e->port);
    printf("GROUP=0x%x|\n", l2e->group);
	 return BCM_E_NONE;
}

/* 
 * 1. adding entries to different groups
 * 2. dump MACT table according to specific group 
 * 3. traverse&delete  according to specific group 
 *  
 * Parameters: 
 * - port: to set as destination of MACT entry 
 *  
 * Example call: 
 * - mact_bulk_group_traverse_example(0,13); 
 */
int
 mact_bulk_group_traverse_example(int unit, int local_port) {
    int rv;
    int a=0;
    int flags=0;

    bcm_l2_addr_t l2_addr;
    bcm_gport_t dest_port;
    bcm_mac_t mac1;
    bcm_vlan_t vid;
    int grp1, grp2;
    bcm_l2_addr_t match_addr;

    /* init data */
    vid=10;
    mac1[5] = 0x32;
    grp1 = 3;
    grp2 = 4;
    bcm_l2_addr_t_init(&l2_addr,mac1,vid);
    l2_addr.port = dest_port;
    BCM_GPORT_LOCAL_SET(dest_port,local_port);

    /* add L2 entries with group grp1*/
    l2_addr.group = grp1;
    rv = bcm_l2_addr_add(unit,&l2_addr);

    l2_addr.vid++;
    rv = bcm_l2_addr_add(unit,&l2_addr);

    l2_addr.vid++;
    rv = bcm_l2_addr_add(unit,&l2_addr);

    /* add L2 entries with group 4*/
    l2_addr.group = grp2;

    l2_addr.vid++;
    rv = bcm_l2_addr_add(unit,&l2_addr);

    l2_addr.vid++;
    rv = bcm_l2_addr_add(unit,&l2_addr);

    if (verbose >= 2) {
        printf("dump all entries (showing group of each entry) \n");
    }
    /* traverse: show group of each entry */
    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_traverse\n");
         return rv;
    }

    /* traverse according to group grp1 */
    if (verbose >= 2) {
        printf("dump entries in group %d\n",grp1);
    }
    bcm_l2_addr_t_init(&match_addr,mac1,vid);
    match_addr.group = grp1;
    rv = bcm_l2_matched_traverse(unit,flags, &match_addr, call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_matched_traverse\n");
         return rv;
    }
    /* traverse according to group grp2 */
    if (verbose >= 2) {
        printf("dump entries in group %d\n",grp2);
    }

    bcm_l2_addr_t_init(&match_addr,mac1,vid);
    match_addr.group = grp2;
    rv = bcm_l2_matched_traverse(unit,flags, &match_addr, call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_matched_traverse\n");
         return rv;
    }

    if (verbose >= 2) {
        printf("delete entries in group %d\n",grp1);
    }
    flags = BCM_L2_REPLACE_MATCH_STATIC|BCM_L2_REPLACE_DELETE|BCM_L2_REPLACE_NO_CALLBACKS|BCM_L2_REPLACE_IGNORE_DISCARD_SRC|BCM_L2_REPLACE_IGNORE_DES_HIT;
    bcm_l2_addr_t_init(&match_addr,mac1,vid);
    match_addr.group = grp1;
    rv = bcm_l2_replace(unit, flags, match_addr, 0,0,0);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_replace\n");
         return rv;
    }

    if (verbose >= 2) {
        printf("dump entries (after remove group %d) \n",grp1);
    }
    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_replace\n");
         return rv;
    }

    return rv;
}


/* 
 * 1. adding entries to MACT
 * 2. set traverse mode to aggregate
 * 3. call several delete-by APIs 
 * 4. commit all rules 
 *  
 * Parameters: 
 * - dest_ports must be array of size = 4. use as destination of L2 entries
 * Example call: 

cint ../../../../src/examples/dpp/cint_mact.c 
cint ../../../../src/examples/dpp/cint_mact_bulk_operations.c 
cint 
int unit = 0;
int local_port = 13;
int dest_ports[4] = {13,14,15,16};
mact_bulk_multiple_rules_example(unit,local_port,dest_ports); 
 */
int
 mact_bulk_multiple_rules_example(int unit, int local_port, int *dest_ports) {
    uint32 rv;
    int a=0;
    /* (int unit, int vlan, int in_port, int dest_port, int nof_entries); */
    example_new_vlan_entries(unit,1,local_port,dest_ports[0],5);
    example_new_vlan_entries(unit,2,local_port,dest_ports[1],5);
    example_new_vlan_entries(unit,2,local_port,dest_ports[2],5);
    example_new_vlan_entries(unit,3,local_port,dest_ports[3],5);

    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_traverse\n");
         return rv;
    }
    
    bshell(0, "g IHP_MACT_COUNTER_LIMIT_MACT_DB_ENTRIES_COUNT");

    /* set aggregate mode */
    if (verbose >= 2) {
          printf("set traverse to aggregate mode\n");
    }
    rv = bcm_switch_control_set(unit, bcmSwitchTraverseMode, bcmSwitchTableUpdateRuleAdd);
    if (rv != BCM_E_NONE) {
         printf("Error, bcmSwitchTraverseMode:bcmSwitchTraverseMode\n");
         return rv;
    }

    /* delete entries frwrd to port[0] */
    if (verbose >= 2) {
          printf("delete entries with dest = 0x%08x\n", dest_ports[0]);
    }
    rv = bcm_l2_addr_delete_by_port(unit,0,dest_ports[0],BCM_L2_DELETE_NO_CALLBACKS|BCM_L2_DELETE_STATIC);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_addr_delete_by_port\n");
         return rv;
    }

    /* delete entries frwrd to port[1] */
    if (verbose >= 2) {
          printf("delete entries with dest = 0x%08x\n", dest_ports[1]);
    }
    rv = bcm_l2_addr_delete_by_port(unit,0,dest_ports[1],BCM_L2_DELETE_NO_CALLBACKS|BCM_L2_DELETE_STATIC);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_addr_delete_by_port\n");
         return rv;
    }
    /* show no change yet */
    /* cannot dump table as dump us traverse */
    if (verbose >= 2) {
          printf("sleep 1 second\n");
    }
    sal_sleep(1);
    bshell(0, "g IHP_MACT_COUNTER_LIMIT_MACT_DB_ENTRIES_COUNT");

    if (verbose >= 2) {
          printf("commit rules \n");
    }
    rv = bcm_switch_control_set(unit, bcmSwitchTraverseMode, bcmSwitchTableUpdateRuleCommit);
    if (rv != BCM_E_NONE) {
         printf("Error, bcmSwitchTraverseMode:bcmbcmSwitchTableUpdateRuleCommit\n");
         return rv;
    }

    if (verbose >= 2) {
          printf("sleep 1 second\n");
    }
    sal_sleep(1);
    bshell(0, "g IHP_MACT_COUNTER_LIMIT_MACT_DB_ENTRIES_COUNT");

    /* restor normal behavoir of traverse */
    if (verbose >= 2) {
          printf("clear rules and back to normal mode\n");
    }

    rv = bcm_switch_control_set(unit, bcmSwitchTraverseMode, bcmSwitchTableUpdateRuleClear);
    if (rv != BCM_E_NONE) {
         printf("Error, bcmSwitchTraverseMode:bcmSwitchTableUpdateRuleClear\n");
         return rv;
    }

    rv = bcm_switch_control_set(unit, bcmSwitchTraverseMode, bcmSwitchTableUpdateNormal);
    if (rv != BCM_E_NONE) {
         printf("Error, bcmSwitchTraverseMode:bcmSwitchTableUpdateNormal\n");
         return rv;
    }

    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_traverse\n");
         return rv;
    }

    return rv;
}






/* 
 * 1. adding entries to MACT on different vlans
 * 2. delete all entries with VLAN value with mask
 *  
 * Parameters: 
 * - local_port: to be use as destination of the L2 entries
 * Example call: 

cint ../../../../src/examples/dpp/cint_mact.c 
cint ../../../../src/examples/dpp/cint_mact_bulk_operations.c 
cint 
int unit = 0;
int local_port = 13;
mact_bulk_rule_mask_vlan_example(unit,local_port); 
 */
int
 mact_bulk_rule_mask_vlan_example(int unit, int local_port) {
    uint32 rv;
    int a=0;
    int flags;
    bcm_mac_t mac_init = {0};
    bcm_l2_addr_t match_addr;
    bcm_l2_addr_t match_mask_addr;
    bcm_l2_addr_t replace_mask_addr;
    bcm_l2_addr_t replace_addr;
    bcm_vlan_t vlan =0;

    /* (int unit, int vlan, int in_port, int dest_port, int nof_entries);*/
    example_new_vlan_entries(unit,0xf0f,local_port,local_port,5);
    example_new_vlan_entries(unit,0x70f,local_port,local_port,5);
    example_new_vlan_entries(unit,0xf0e,local_port,local_port,5);
    example_new_vlan_entries(unit,0xf1f,local_port,local_port,5);
    example_new_vlan_entries(unit,0xfff,local_port,local_port,5);
    example_new_vlan_entries(unit,0xfaf,local_port,local_port,5);

    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_traverse\n");
         return rv;
    }

    flags = BCM_L2_REPLACE_MATCH_STATIC|BCM_L2_REPLACE_DELETE|BCM_L2_REPLACE_NO_CALLBACKS|BCM_L2_REPLACE_IGNORE_DISCARD_SRC|BCM_L2_REPLACE_IGNORE_DES_HIT;
    flags |= BCM_L2_REPLACE_MATCH_VLAN;
    bcm_l2_addr_t_init(&match_addr,mac_init,0);
    bcm_l2_addr_t_init(&match_mask_addr,mac_init,0);
    bcm_l2_addr_t_init(&replace_addr,mac_init,0);
    bcm_l2_addr_t_init(&replace_mask_addr,mac_init,0);

    /* delete according to vlan, all entries with vlan =  f?f, middle nibble is ignored */
    match_addr.vid = 0xf0f;
    match_mask_addr.vid = 0xf0f;
    rv = bcm_l2_replace_match(unit, flags, &match_addr, match_mask_addr,replace_addr,replace_mask_addr);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_replace_match\n");
         return rv;
    }

    /* sleep till replace done */
    if (verbose >= 2) {
          printf("sleep 1 second\n");
    }
    sal_sleep(1);
    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_traverse\n");
         return rv;
    }

    return rv;
}



/* 
 * 1. adding entries to MACT on different vlans
 * 2. delete all entries with VLAN value with mask
 *  
 * Parameters: 
 * - local_port: to be use as destination of the L2 entries
 * Example call: 

cint ../../../../src/examples/dpp/cint_mact.c 
cint ../../../../src/examples/dpp/cint_mact_bulk_operations.c 
cint 
int unit = 0;
int local_port = 13;
mact_bulk_rule_mask_vlan_port_example(unit,local_port); 
 */
int
 mact_bulk_rule_mask_vlan_port_example(int unit, int local_port) {
    uint32 rv;
    int a=0;
    int flags;
    bcm_mac_t mac_init = {0};
    bcm_l2_addr_t match_addr;
    bcm_l2_addr_t match_mask_addr;
    bcm_l2_addr_t replace_mask_addr;
    bcm_l2_addr_t replace_addr;
    bcm_vlan_t vlan =0;
    int nof_gports = 20;
    int gports[20] = {0};

    /* fill mact:*/
    rv = travers_mask_init_mact_gports(unit,local_port,nof_gports,gports);
    if (rv != BCM_E_NONE) {
         printf("Error, travers_mask_init_mact_gports\n");
         return rv;
    }

    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_traverse\n");
         return rv;
    }

    flags = BCM_L2_REPLACE_MATCH_STATIC|BCM_L2_REPLACE_DELETE|BCM_L2_REPLACE_NO_CALLBACKS|BCM_L2_REPLACE_IGNORE_DISCARD_SRC|BCM_L2_REPLACE_IGNORE_DES_HIT;
    flags |= BCM_L2_REPLACE_MATCH_DEST;
    bcm_l2_addr_t_init(&match_addr,mac_init,0);
    bcm_l2_addr_t_init(&match_mask_addr,mac_init,0);
    bcm_l2_addr_t_init(&replace_addr,mac_init,0);
    bcm_l2_addr_t_init(&replace_mask_addr,mac_init,0);
    match_addr.port = gports[0];
    match_mask_addr.port = 0x5; /* consider only bit[0] and and bit[2] */

    /* delete according to vlan-port (outlif),
       all entries with outlif: xxx1x0
     */
    if (verbose >= 2) {
        printf("delete gport match [0x%08x/0x%08x],\n",match_addr.port,match_mask_addr.port);
    }
    
    rv = bcm_l2_replace_match(unit, flags, &match_addr, match_mask_addr,replace_addr,replace_mask_addr);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_replace_match\n");
         return rv;
    }

    if (verbose >= 2) {
          printf("sleep 1 second\n");
    }
    sal_sleep(1);

    rv = bcm_l2_traverse(unit,call_back_get_block, &a);
    if (rv != BCM_E_NONE) {
         printf("Error, bcm_l2_traverse\n");
         return rv;
    }

    return rv;
}


/* utility functions */

/* create vlan-port */
int
vswitch_metro_add_port(int unit, int port, int vlan,  bcm_gport_t *port_id){
    int rv;
    bcm_vlan_port_t vp1;
    bcm_vlan_port_t_init(&vp1);
    
    vp1.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    vp1.port = port;
    vp1.match_vlan = vlan;
    vp1.egress_vlan = vlan;
    vp1.flags = 0;
    vp1.vlan_port_id = 0;
    rv = bcm_vlan_port_create(unit,&vp1);
    
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vlan_port_create\n");
        print rv;
        return rv;
    }

    if(verbose >= 3){
        printf("Add vlan-port-id:0x%08x in-port:0x%08x match_vlan:0x%08x match_inner_vlan:0x%08x in unit %d\n",vp1.vlan_port_id, vp1.port, vp1.match_vlan, vp1.match_inner_vlan, unit);
    }
    *port_id = vp1.vlan_port_id;
  return rv;
}

uint32 travers_mask_init_mact_gports(int unit, int local_port, int nof_ports, int *gports){
    uint32 in_vlan = 0xabc;
    int in_port = local_port;
    uint32 rv;
    uint32 index;

    for (index = 0; index < nof_ports; index++) {
        rv = vswitch_metro_add_port(unit,in_port,in_vlan++,&gports[index]);
        if (rv != BCM_E_NONE) {
            return rv;
        }
        example_new_vlan_entries(unit,1,in_port,gports[index],2);
    }
    if(verbose >= 1)
        print gports;

    return rv;
}

