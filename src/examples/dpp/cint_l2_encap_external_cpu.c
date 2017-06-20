/*
 * $Id: cint_l2_encap_external_cpu.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$SDK/src/examples/dpp/cint cint_l2_encap_external_cpu.c
 * 
 * configuration
 * BCM> print l2_external_cpu_setup(unit, cpu_port1, cpu_port2)
 * 
 * execution:
 * * BCM>cint
 * cint>l2_external_cpu_uc_forwarding(unit);
 * cint>exit;
 * for uc send
 * BCM> tx 1 DATA=0x0000000000eb0000000000138100000108990123456789098765432123456789c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5 PSRC=13
 * the added l2 header will be 23456789abcd0a1b2c3d4e5f810050648999+system headers
 * 
 * for uc ingress trap
 * execution:
 * BCM>cint
 * cint>l2_external_cpu_uc_trap_set(unit);
 * cint>exit;
 * for uc send (trap for same DA and SA)
 * BCM> tx 1 DATA=0x0000000000eb00x0000000000eb8100000108990123456789098765432123456789c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5 PSRC=13
 * the added l2 header will be 23456789abcd0a1b2c3d4e5f810050648999+system headers
 *
 * 
 * for mc send
 * BCM>cint
 * cint>l2_external_cpu_mc_forwarding(unit);
 * cint>exit;
 * BCM> tx 1 DATA=0xff00000000eb0000000000138100000108990123456789098765432123456789c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5 PSRC=13
 * the added l2 header will be 23456789abcd0a1b2c3d4e5f810050648999+system headers
 */
struct l2_external_cpu_l2_cpu_config_t
{
   bcm_port_encap_config_t encap_config[3];
   bcm_tunnel_initiator_t l2_encap_tunnel[3];
   bcm_l3_intf_t tunnel_info[3];
   bcm_port_t port_list[2];
   int num_of_cpu_ports;
};

l2_external_cpu_l2_cpu_config_t l2_cpu_config;

int l2_external_cpu_snoop_by_src_port(int unit, bcm_port_t src_port)
{
	int rv = BCM_E_NONE;	
	bcm_field_presel_set_t presel_set;
	bcm_field_presel_t presel_id;
	bcm_field_qset_t qset;
	bcm_field_aset_t aset;
	bcm_field_group_config_t fg;
	bcm_field_group_t fg_group = 2;
	bcm_field_entry_t entry_1;
	bcm_port_t src_port_mask = 0xffffffff;
    int flags = 0;                            
    int snoop_command;                      /* Snoop command */
    bcm_rx_snoop_config_t snoop_config;
    int trap_id;
    int trap_dest_strength = 0;             /* No influence on the destination update */         
    int trap_snoop_strength = 3;            /* Strongest snoop strength for this trap */
    bcm_rx_trap_config_t trap_config;       /* Snoop attributes */ 
    bcm_gport_t snoop_trap_gport_id;

	bcm_port_t dest_port=l2_cpu_config.port_list[0];
	int encap_id=l2_cpu_config.tunnel_info[2].l3a_intf_id;
	int presel_flags = BCM_FIELD_QUALIFY_PRESEL;
	int advanced_mode=soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE);

/*  Create Snoop command */

    rv = bcm_rx_snoop_create(unit, flags, &snoop_command);
    if (rv != BCM_E_NONE) 
    {
      printf("Error in bcm_rx_snoop_create\n");
      return rv;
    }
	
	bcm_rx_snoop_config_t_init(&snoop_config);                      /* Initialize the structure */

	snoop_config.flags |= BCM_RX_SNOOP_UPDATE_DEST|BCM_RX_SNOOP_UPDATE_ENCAP_ID;    /* Define Destination flag for snoop */
	BCM_GPORT_LOCAL_SET(snoop_config.dest_port, dest_port);                  /* Set the snoop destination to dest */
	
	snoop_config.encap_id=encap_id;
    snoop_config.size = -1;                                         /* Full packet snooping */
    snoop_config.probability = 100000;                              /* Set probability to 100% */
    rv = bcm_rx_snoop_set(unit, snoop_command, &snoop_config);  /* Set snoop configuration */
    if (rv != BCM_E_NONE) 
    {
      printf("Error in bcm_rx_snoop_set\n");
      return rv;
    }

/*  Create a User-defined trap for snooping */

    rv = bcm_rx_trap_type_create(unit, flags, bcmRxTrapUserDefine, &trap_id);
    if (rv != BCM_E_NONE) 
    {
      printf("Error in bcm_rx_trap_type_create\n");
      return rv;
    }
	
    bcm_rx_trap_config_t_init(&trap_config);                        /* Configure the trap to the snoop command */ 
    trap_config.flags |= BCM_RX_TRAP_REPLACE;                       /* Set flag for trap - replace */
    trap_config.trap_strength = trap_dest_strength;                 /* Set trap strength */
    trap_config.snoop_cmnd = snoop_command;                         /* set trap snoop command */
    trap_config.snoop_strength = trap_snoop_strength; 
    rv = bcm_rx_trap_set(unit, trap_id, &trap_config);
    if (rv != BCM_E_NONE) 
    {
      printf("Error in bcm_rx_trap_set\n");
      return rv;
    }

    BCM_GPORT_TRAP_SET(snoop_trap_gport_id, trap_id, trap_dest_strength, trap_snoop_strength); /* Get the trap gport to snoop */

/*  Create preselector with qualifer src Port */

	bcm_field_presel_set_t_init(&presel_set);
	if (advanced_mode) {
	presel_flags |= BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS;
	presel_id = 4;
	rv = bcm_field_presel_create_stage_id(unit, bcmFieldStageIngress, presel_id);
		if (BCM_E_NONE != rv) {
			printf("Error in bcm_field_presel_create_stage_id\n");
			bcm_field_presel_destroy(unit, presel_id | BCM_FIELD_QUALIFY_PRESEL_ADVANCED_MODE_STAGE_INGRESS);
			return rv;
		}
    } else {
        rv = bcm_field_presel_create(unit,&presel_id);
        if (BCM_E_NONE != rv) {
            printf("Error in bcm_field_presel_create_id\n");
            bcm_field_presel_destroy(unit, presel_id);
            return rv;
        }
    }
	
	rv = bcm_field_qualify_InPort(unit, presel_id | presel_flags, src_port, src_port_mask);
	if (rv != BCM_E_NONE) 
	{
	        printf("Error, bcm_field_qualify_InPort\n");
	        return rv;
	}

/*  Add preselector to preselector set */

	BCM_FIELD_PRESEL_ADD(presel_set,presel_id);
	
/*  Create qset - create a key with qualifer in port */ 
    	
	BCM_FIELD_QSET_INIT(&qset);
	BCM_FIELD_QSET_ADD(qset,bcmFieldQualifyInPort);

/*  Create aset - snoop action result from lookup */

	BCM_FIELD_ASET_INIT(&aset);
	BCM_FIELD_ASET_ADD(aset,bcmFieldActionSnoop);
	
/*  Define a database (Field-Group) and associate it with a pre-selector, QSET and ASET */
    	
	bcm_field_group_config_t_init(&fg);
	fg.group = fg_group;
	fg.flags = BCM_FIELD_GROUP_CREATE_WITH_ASET | BCM_FIELD_GROUP_CREATE_WITH_PRESELSET | BCM_FIELD_GROUP_CREATE_WITH_MODE;
	fg.qset = qset;
	fg.priority = 2;
	fg.mode = bcmFieldGroupModeAuto;
	fg.aset = aset;
	fg.preselset = presel_set;

	bcm_field_group_config_create(unit, &fg);
	if (rv != BCM_E_NONE) 
	{
	        printf("Error, bcm_field_group_config_create\n");
	        return rv;
	}

/*  Create entry and associate it with field group */

	rv = bcm_field_entry_create(unit,fg.group,&entry_1);
	if (rv != BCM_E_NONE) 
	{
        	printf("Error, bcm_field_entry_create\n");
        	return rv;
    }
	
    bcm_field_qualify_InPort(unit,entry_1,src_port,src_port_mask);

    bcm_field_action_add(unit, entry_1, bcmFieldActionSnoop, snoop_trap_gport_id, 0);
	
/*  Write the entry to the HW database - commit all changes to the hardware */
    	
	bcm_field_group_install(unit,fg.group);
	if (rv != BCM_E_NONE) 
	{
	        printf("Error, bcm_field_group_install\n");
        	return rv;
	}

	return BCM_E_NONE;
}

 /* create encapsulation information*/
int l2_external_cpu_setup(int unit, int num_of_cpu_ports, bcm_port_t out_port1, bcm_port_t out_port2)
{
	int i;
	int pcp=3;
    int dei=1;
	bcm_error_t rv;

	l2_cpu_config.port_list[0]=out_port1;

	if (num_of_cpu_ports=2) {
		l2_cpu_config.port_list[1]=out_port2;
	}
	else
	{
		l2_cpu_config.port_list[1]=out_port1;
	}


    for (i=0;i<3;++i) {
	  l2_cpu_config.encap_config[i].encap = BCM_PORT_ENCAP_IEEE;
	  l2_cpu_config.encap_config[i].dst_mac[0]=0x23;
	  l2_cpu_config.encap_config[i].dst_mac[1]=0x45;
	  l2_cpu_config.encap_config[i].dst_mac[2]=0x67;
	  l2_cpu_config.encap_config[i].dst_mac[3]=0x89;
	  l2_cpu_config.encap_config[i].dst_mac[4]=0xAB;
	  l2_cpu_config.encap_config[i].dst_mac[5]=i;

	  l2_cpu_config.encap_config[i].src_mac[0]=0x0A;
	  l2_cpu_config.encap_config[i].src_mac[1]=0x1B;
	  l2_cpu_config.encap_config[i].src_mac[2]=0x2C;
	  l2_cpu_config.encap_config[i].src_mac[3]=0x3D;
	  l2_cpu_config.encap_config[i].src_mac[4]=0x4E;
	  l2_cpu_config.encap_config[i].src_mac[5]=0x5F;
	   

	   l2_cpu_config.encap_config[i].tpid=0x8100;
	   l2_cpu_config.encap_config[i].vlan=100;
	   l2_cpu_config.encap_config[i].oui_ethertype=0x8999;


	   l2_cpu_config.encap_config[i].tos=pcp<<1|dei;
  
	   bcm_tunnel_initiator_t_init(&l2_cpu_config.l2_encap_tunnel[i]);
	   bcm_l3_intf_t_init(&l2_cpu_config.tunnel_info[i]);
	   /* span info */
	   l2_cpu_config.l2_encap_tunnel[i].type       = bcmTunnelTypeL2EncapExternalCpu;


	   rv = bcm_tunnel_initiator_create(unit, &l2_cpu_config.tunnel_info[i] , &l2_cpu_config.l2_encap_tunnel[i]);
	   if(rv != BCM_E_NONE) {
		printf("Error, create tunnel initiator \n");
		return rv;
	   }

	   printf("Allocated tunnel id:%x\n",l2_cpu_config.l2_encap_tunnel[i].tunnel_id);

	   rv = bcm_port_encap_config_set(unit,l2_cpu_config.l2_encap_tunnel[i].tunnel_id,&(l2_cpu_config.encap_config[i]));
	   if (rv != BCM_E_NONE) {
		  printf("Error, bcm_port_encap_config_set \n");
		   return rv;
	   }

	}

	return BCM_E_NONE;

}



int l2_external_cpu_uc_forwarding(int unit) {

	bcm_error_t rv;
    uint8 fwd_mac[6] = {0x00,0x00,0x00,0x00,0x00,0xeb}; 

    bcm_l2_addr_t l2_addr;

    bcm_l2_addr_t_init(&l2_addr,fwd_mac,1);
	
	l2_addr.vid=1;
	l2_addr.port=l2_cpu_config.port_list[0];
    l2_addr.flags = BCM_L2_STATIC;
    l2_addr.encap_id = l2_cpu_config.tunnel_info[0].l3a_intf_id; /*outlif (not a gport)*/

    rv = bcm_l2_addr_add(unit, &l2_addr);
    if (rv != BCM_E_NONE) {
             printf("Error, l2_addr_add\n");
             return rv;
        }
	
	return BCM_E_NONE;

}

int l2_external_cpu_uc_trap_set(int unit) {

	bcm_error_t rv;
    bcm_rx_trap_config_t config;
    int flags=0;
    int trap_id=0;

	sal_memset(&config, 0, sizeof(config));

	/*for port dest change*/
	config.flags = BCM_RX_TRAP_UPDATE_DEST | BCM_RX_TRAP_UPDATE_ENCAP_ID;

	config.trap_strength = 7; /*FOR USER DEFINE ONLY !!! */
	config.dest_port=l2_cpu_config.port_list[1];
    config.encap_id=l2_cpu_config.tunnel_info[1].l3a_intf_id;

	config.snoop_cmnd = 0; 
	config.snoop_strength = 0; 


	rv = bcm_rx_trap_type_create(unit,flags,bcmRxTrapSaEqualsDa,&trap_id);
	if (rv != BCM_E_NONE) {
		printf("Error, in trap create, trap id %d \n",trap_id);
		return rv;
	}
	

	rv = bcm_rx_trap_set(unit,trap_id,&config);
	if (rv != BCM_E_NONE) {
		printf("Error, in trap set \n");
		return rv;
	}
	
	return BCM_E_NONE;

}

int l2_external_cpu_mc_forwarding(int unit) {

	bcm_error_t rv;
    uint8 fwd_mac[6] = {0xff,0x00,0x00,0x00,0x00,0xeb}; 
    bcm_l2_addr_t l2_addr;
	int i;
		 
	bcm_multicast_t mc_group = 111;
	int flags;
	flags =  BCM_MULTICAST_INGRESS_GROUP | BCM_MULTICAST_WITH_ID ;
	rv = bcm_multicast_destroy(unit, mc_group);
	if (rv != BCM_E_NONE) {
        printf("Warning, in mc destroy,  mc_group $mc_group \n");
    }
    rv = bcm_multicast_create(unit, flags, &mc_group);
    if (rv != BCM_E_NONE) {
        printf("Error, in mc create, flags $flags mc_group $mc_group \n");
        return rv;
    }

	printf("mc group created %d, \n",mc_group);


    bcm_l2_addr_t_init(&l2_addr,fwd_mac,1);
	
	l2_addr.vid=1;
	l2_addr.flags = BCM_L2_STATIC | BCM_L2_MCAST;
    l2_addr.l2mc_group = mc_group;

    rv = bcm_l2_addr_add(unit, &l2_addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add\n");
        print rv;
        return rv;
    }

    bcm_gport_t gport;
    BCM_GPORT_LOCAL_SET(gport, l2_cpu_config.port_list[0]); 
    rv = bcm_multicast_ingress_add(unit, mc_group, gport, l2_cpu_config.tunnel_info[1].l3a_intf_id);
    if (rv != BCM_E_NONE) {
        printf("Error, in mc ingress add, mc_group $mc_group dest_gport $dest_gport \n");
        return rv;
    }

	rv = bcm_multicast_ingress_add(unit, mc_group, gport, l2_cpu_config.tunnel_info[2].l3a_intf_id);
    if (rv != BCM_E_NONE) {
        printf("Error, in mc ingress add, mc_group $mc_group dest_gport $dest_gport \n");
        return rv;
    }

	
	return BCM_E_NONE;

}

int l2_external_cpu_sanitiy_check(int unit) {

	bcm_error_t rv;
	bcm_port_config_t encap_config;
	bcm_tunnel_initiator_t l2_encap_tunnel;
    bcm_l3_intf_t tunnel_info;
	int i;

	for (i=0;i<100;++i) {
		bcm_tunnel_initiator_t_init(&l2_encap_tunnel);
		bcm_l3_intf_t_init(&tunnel_info);
		l2_encap_tunnel.type       = bcmTunnelTypeL2EncapExternalCpu;


		rv = bcm_tunnel_initiator_create(unit, &tunnel_info , &l2_encap_tunnel);
		if(rv != BCM_E_NONE) {
		 printf("Error, create tunnel initiator \n");
		 return rv;
		}


		rv = bcm_tunnel_initiator_clear(unit, &tunnel_info);
		if(rv != BCM_E_NONE) {
		 printf("Error, create tunnel initiator \n");
		 return rv;
		}
	}

    bcm_tunnel_initiator_t_init(&l2_encap_tunnel);
    bcm_l3_intf_t_init(&tunnel_info);
	/* span info */
	l2_encap_tunnel.type       = bcmTunnelTypeL2EncapExternalCpu;

    l2_encap_tunnel.tunnel_id=0x0f000;
    l2_encap_tunnel.flags|=BCM_TUNNEL_WITH_ID;

	rv = bcm_tunnel_initiator_create(unit, &tunnel_info , &l2_encap_tunnel);
	if(rv != BCM_E_NONE) {
	 printf("Error, create tunnel initiator \n");
	 return rv;
	}

    rv = bcm_tunnel_initiator_clear(unit, &tunnel_info);
	if(rv != BCM_E_NONE) {
	 printf("Error, create tunnel initiator \n");
	 return rv;
	}

    return BCM_E_NONE;
}


int l2_external_cpu_run_all(int unit, int cpu_ports, bcm_port_t src_port, bcm_port_t out_port1, bcm_port_t out_port2) {

	int rv=BCM_E_NONE;

	 printf("l2_external_cpu_setup\n");
	rv = l2_external_cpu_setup(unit, cpu_ports,out_port1, out_port2);
	if (rv != BCM_E_NONE) {
	 printf("Error, l2_external_cpu_setup \n");
	 print rv;
	 return rv;
    }

	 printf("l2_external_cpu_sanitiy_check\n");
    rv = l2_external_cpu_sanitiy_check(unit);
	if (rv != BCM_E_NONE) {
        printf("Error, l2_external_cpu_sanitiy_check \n");
        print rv;
        return rv;
    }

	 printf("l2_external_cpu_uc_forwarding\n");
	rv = l2_external_cpu_uc_forwarding(unit);
	if (rv != BCM_E_NONE) {
        printf("Error, l2_external_cpu_uc_forwarding \n");
        print rv;
        return rv;
    }

	rv = l2_external_cpu_snoop_by_src_port(unit, src_port);
	if (rv != BCM_E_NONE) {
        printf("Error, l2_external_cpu_uc_forwarding \n");
        print rv;
        return rv;
    }

	 printf("l2_external_cpu_uc_trap_set\n");
	rv = l2_external_cpu_uc_trap_set(unit);
	if (rv != BCM_E_NONE) {
        printf("Error, l2_external_cpu_uc_trap_set \n");
        print rv;
        return rv;
    }


	 printf("l2_external_cpu_mc_forwarding\n");
	rv = l2_external_cpu_mc_forwarding(unit);
	if (rv != BCM_E_NONE) {
        printf("Error, l2_external_cpu_mc_forwarding \n");
        print rv;
        return rv;
    }
	
	return rv;

}

