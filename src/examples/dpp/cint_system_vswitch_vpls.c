/*
 * $Id: cint_system_vswitch.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * System resource example:
 * In the following CINT we will try to provide an example of System resources in local mode.
 *    The focus of the CINT is on PWE-LIFs (bcm_mpls_port_t object in BCM API).
 * Similiar principles are apply also to other LIF types such as bcm_vlan_port_t
 * (see cint_system_vswitch.c).
 * Each PWE-LIF allocates two objects: An Ingress LIF entry and an Egress LIF entry.
 * In case of 1:1 protection also a L2 FEC object that stores
 * the protection pairs is also being created.
 *    When working with system resources in global mode all 3 objects are required to be
 * allocated on all devices with the same object ID.
 * Please see an example of such schemes in:
 * cint_vswitch_metro_mp_multi_devices.c, cint_vswitch_cross_connect_p2p_multi_device.c
 *    When working with system resources in local mode not all 3 objects are required to be
 * allocated on all devices:
 *  * In/Out-PWE-Lif need to be created only on devices where the PWE is attached to.
 *  In case of LAGs, these may be all the potential devices that connect to the LAG port members.
 *  * The L2 FEC needs to be created only on devices that forward to the Lif.
 *  The following CINT provides some basic examples of PWE-LIf VPLS vswitch allocation schemes when
 *  working in local mode.
 *  1. MPLS P2P unprotected: see system_mpls_port_p2p_example
 *  2. MPLS MP unprotection: see system_mpls_port_mp_example
 *  3. MPLS P2P/MP protected: see system_protected_mpls_port_example
 * 
 * assumptions:
 * 1. run two devices in same CPU.
 * 2. in call unit1 != unit2.
 * 3. diag_chassis=1
 * 4. bcm88xxx_system_resource_management.<unit1>=1
 *    bcm88xxx_system_resource_management.<unit2>=1
 
 * How to run:
cint;
cint_reset();
exit; 
cint ../../../../src/examples/dpp/utility/cint_utils_global.c
cint ../../../../src/examples/dpp/utility/cint_utils_mpls.c
cint ../../../../src/examples/dpp/utility/cint_utils_mpls_port.c 
cint ../../../../src/examples/dpp/utility/cint_utils_multicast.c 
cint ../../../../src/examples/dpp/cint_port_tpid.c 
cint ../../../../src/examples/dpp/cint_mact.c 
cint ../../../../src/examples/dpp/cint_qos.c  
cint ../../../../src/examples/dpp/utility/cint_utils_l3.c    
cint ../../../../src/examples/dpp/cint_mpls_lsr.c 
cint ../../../../src/examples/dpp/cint_vswitch_vpls.c
cint ../../../../src/examples/dpp/cint_vswitch_cross_connect_p2p.c 
cint ../../../../src/examples/dpp/cint_vswitch_metro_mp.c 
cint ../../../../src/examples/dpp/cint_multi_device_utils.c 
cint ../../../../src/examples/dpp/cint_system_vswitch_encoding.c  
cint ../../../../src/examples/dpp/cint_system_vswitch.c 
cint ../../../../src/examples/dpp/utilities/cint_utils_mpls_port.c 
cint ../../../../src/examples/dpp/cint_system_vswitch_vpls.c 
cint
verbose = 3;
int rv; 
int unit1 = 0; 
int unit2 = 1; 
int sysport1, sysport2, sysport3, sysport4; 
print port_to_system_port(unit1, 13, &sysport1); 
print port_to_system_port(unit1, 14, &sysport2); 
print port_to_system_port(unit2, 15, &sysport3); 
print port_to_system_port(unit2, 14, &sysport4);
for MPLS P2P case: 
print system_mpls_port_p2p_example(unit1,sysport1,unit2,sysport3);
 
for MPLS MP case: 
print system_mpls_port_mp_example(unit1,sysport1,sysport2,unit2,sysport3); 
 
for MPLS P2P protection case: 
print system_protected_mpls_port_example(unit1, sysport1, unit2, sysport3,sysport4,0); 
 
for MPLS MP protection case: 
print system_protected_mpls_port_example(unit1, sysport1, unit2, sysport3,sysport4,1); 
  
*/

/* set to one for informative prints */
int verbose = 3;
int mpls_tunnel_push_profile = 1;
int mpls_port_push_profile = 2;


/* 
 * The following utility functions replaces the call to bcm_l2_gport_frwrd_info_get,
 * by mapping the MPLS PORT gport info to frwrd-info directly
 * handling seperated into 3 cases according to gport-id and vlan_port usage.
 *  A) 1:1 PWE porection: Forwarding = FEC.
 *  B) 1+1 PWE proection: Forwarding = Multicast
 *  C) MPLS tunnel protection: <L3 FEC, EEI-VC>
 *  D) No protection: <Port, PWE-Outlif>  
 */
int system_mpls_port_to_l2_gport_forward_info(
   int unit,
   bcm_mpls_port_t  *mpls_port,
   bcm_l2_gport_forward_info_t *frwrd_info)
{

    int outlif=0;
    int eei=0;
    int fec_id;
    uint32 mc_id;

    bcm_l2_gport_forward_info_t_init(frwrd_info);

    /* 
     * A) in case of 1:1 protection the forwarding information is FEC
     * the FEC id is passed in the mpls_port_id.
     */

    /* 1:1 protection identified if failover-id != 0*/
    if (mpls_port->failover_id != 0) {
        if (!(mpls_port->flags & BCM_MPLS_PORT_WITH_ID)) {
            printf("Error, WITH_ID flag has to be set\n"); 
            return BCM_E_PARAM;
        }
        fec_id = BCM_GPORT_SUB_TYPE_PROTECTION_GET(mpls_port->mpls_port_id);
        BCM_GPORT_FORWARD_PORT_SET(frwrd_info->phy_gport,fec_id);
        frwrd_info->encap_id = BCM_FORWARD_ENCAP_ID_INVALID;
    }

    /* 
     * B) in case of 1+1 protection the forwarding information is Multicast (Bi-cast)
     * the Multicast id is passed in the mpls_port_id.
     */

    /* 1+1 protection identified if failover_mc_group is valid MC group*/
    else if (mpls_port->failover_mc_group != 0) {
        if (!(mpls_port->flags & BCM_MPLS_PORT_WITH_ID)) {
            printf("Error, WITH_ID flag has to be set\n"); 
            return BCM_E_PARAM;
        }
        mc_id = BCM_GPORT_SUB_TYPE_MULTICAST_VAL_GET(mpls_port->mpls_port_id);
        BCM_GPORT_MCAST_SET(frwrd_info->phy_gport,mc_id);        
        frwrd_info->encap_id = BCM_FORWARD_ENCAP_ID_INVALID;
    }
    /* C) MPLS protection , check if egress tunnel_if encoded is FEC */
    else if (BCM_L3_ITF_TYPE_IS_FEC(mpls_port->egress_tunnel_if)) {
        if (!(mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL)) {
            printf("Error, EGRESS_TUNNEL flag has to be set\n"); 
            return BCM_E_PARAM;
        }
        /* L3 MPLS FEC */
        fec_id = BCM_L3_ITF_VAL_GET(mpls_port->egress_tunnel_if);
        BCM_GPORT_FORWARD_PORT_SET(frwrd_info->phy_gport,fec_id);

        int int_label = mpls_port->egress_label.label;

        BCM_FORWARD_ENCAP_ID_EEI_USAGE_MPLS_PORT_SET(eei, int_label, mpls_port_push_profile); 
        BCM_FORWARD_ENCAP_ID_VAL_SET(frwrd_info->encap_id, BCM_FORWARD_ENCAP_ID_TYPE_EEI, BCM_FORWARD_ENCAP_ID_EEI_USAGE_MPLS_PORT, eei);
    } else {
        /* 
         * D) otherwise no protection the forwarding information is <port + outLIF>
         * the outLIF id is passed in the encap_id or on the mpls_port_id.
         */
        if (mpls_port->flags & BCM_MPLS_PORT_ENCAP_WITH_ID) {
            outlif = BCM_ENCAP_ID_GET(mpls_port->encap_id);
        }
        else if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
            outlif = BCM_GPORT_SUB_TYPE_LIF_VAL_GET(mpls_port->mpls_port_id);
        }
        else{
            printf("Need to supply WITH_ID or ENCAP_WITH_ID flags\n"); 
            return BCM_E_PARAM;
        }
        frwrd_info->phy_gport = mpls_port->port;
        BCM_FORWARD_ENCAP_ID_VAL_SET(frwrd_info->encap_id, BCM_FORWARD_ENCAP_ID_TYPE_OUTLIF, BCM_FORWARD_ENCAP_ID_OUTLIF_USAGE_MPLS_PORT, outlif);
    }

    return BCM_E_NONE;
}

/*
* SYSTEM EXAMPLE
*/

/*
 * Create MPLS intf next hop egress object (FEC) 
 * FEC points to MPLS outlif and encap_id 
 * parameters: 
 *  * [IN] port_id - destination physical port FEC points to.
 *  * [IN] tunnel_intf - destination MPLS intf (outlif) FEC points to.
 *  * [INOUT] egress_intf - returns FEC pointer
 */
int
system_mpls_tunnel_l3_egress_create(
    int unit,
    bcm_port_t port_id,
    bcm_if_t tunnel_intf,
    bcm_if_t *egress_intf
  )
{
    create_l3_egress_s l3eg;
    bcm_if_t l3egid;
    uint32 flags = BCM_L3_INGRESS_ONLY;
    int rv;
    
    l3eg.out_tunnel_or_rif = tunnel_intf;    
    l3eg.out_gport     = port_id;
    l3eg.arp_encap_id = 0; /* not used */
    bcm_if_t egress_intf_encoding;

    if (*egress_intf != 0) {
        flags |= BCM_L3_WITH_ID;   
        l3eg.fec_id = *egress_intf;     
    }


    if (encoding) {
        BCM_L3_ITF_SET(egress_intf_encoding, BCM_L3_ITF_TYPE_FEC, *egress_intf);
    }
    
    l3eg.allocation_flags = flags;
    rv = l3__egress__create(unit,&l3eg);
    if (rv != BCM_E_NONE){
        printf("error, in l3__egress__create\n");
        return rv;
    }
	if (encoding) {
		printf("after l3__egress__create: l3egid:%d egress_intf_encoding:%d\n ",l3eg.fec_id,egress_intf_encoding);
	}

    if (!encoding) {
        *egress_intf = l3eg.fec_id;
    }
    else{
        *egress_intf = egress_intf_encoding;
    }

    return rv;
}

/*
 * Create MPLS tunnel intf (initiator and terminator) 
 * Create InLIF for MPLS termination and OutLIF for MPLS encapsulation. 
 * parameters: 
 *  * [IN] port_id - destination physical port MPLS tunnel attached to.
 *  * [IN] vlan - Router interface (VSI).
 *  * [IN] in_tunnel_label - Tunnel termination label
 *  * [IN] out_tunnel_label - Tunnel initiator labal
 *  * [INOUT] in_tunnel_intf - Tunnel termination LIF ID
 *  * [INOUT] out_tunnel_intf - Tunnel initiator LIF ID
 *  * [INOUT] encap_id - Link Layer (OutLIF) ID that MPLS tunnel attached to.
 */
   
int
system_mpls_tunnel_intf_create(
    int unit,
    bcm_port_t port_id,
    bcm_vlan_t vlan,
    bcm_mpls_label_t in_tunnel_label,
    bcm_mpls_label_t out_tunnel_label,
    bcm_gport_t *in_tunnel_intf,
    bcm_if_t *out_tunnel_intf,
    bcm_if_t *encap_id
    )
{
    int rv;
    int ingress_intf, ingress_intf_encoding;    
    bcm_mpls_egress_label_t label;
    create_l3_egress_s l3eg;
    bcm_if_t out_tunnel_intf_encoding, encap_id_encoding;
    int term_flags;    
    int l3egid;
    uint8 my_mac[6]      = {0x0, 0x0, 0x0, 0x0, 0x0, 0x11};
    uint8 nh_mac_addr[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x22};

    if (encoding) {
        int vlan_temp = vlan;
        BCM_L3_ITF_SET(ingress_intf_encoding, BCM_L3_ITF_TYPE_RIF, vlan_temp);
    }
 
    /* l2 termination for mpls routing and routed to: packet received on those VID+MAC will be L2 terminated  */
    create_l3_intf_s intf;
    intf.vsi = vlan;
    sal_memcpy(intf.my_global_mac, my_mac, sizeof(my_mac));
    sal_memcpy(intf.my_lsb_mac, my_mac, sizeof(my_mac));
    
    rv = l3__intf_rif__create(unit, &intf);
    ingress_intf = intf.rif;
    if (rv != BCM_E_NONE) {
    	printf("Error, in l3__intf_rif__create\n");
    	return rv;
    }

    if (encoding) {
        printf("after create_l3_intf: ingress_intf:%d ingress_intf_encoding:%d\n ",ingress_intf,ingress_intf_encoding);
    }

    /* set tunnel over this l3 interface, so packet forwarded to this interface will be tunneled */
    bcm_mpls_egress_label_t_init(&label);    

    label.flags = (BCM_MPLS_EGRESS_LABEL_TTL_COPY|BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT);  
    label.flags |= BCM_MPLS_EGRESS_LABEL_EXP_COPY;
    label.flags |= BCM_MPLS_EGRESS_LABEL_WITH_ID;
    label.label = out_tunnel_label;
    label.ttl = 64;

    if (!encoding) {
        label.l3_intf_id = ingress_intf;
    } else {
        label.l3_intf_id = ingress_intf_encoding;
    }

    label.tunnel_id = *out_tunnel_intf;  
	
    if (encoding) {
        BCM_L3_ITF_SET(out_tunnel_intf_encoding, BCM_L3_ITF_TYPE_LIF, *out_tunnel_intf);
    }  
    

    if (encoding) {
      /* update sw push profile */
        rv = vswitch_vpls_allocate_push_profile(unit, label.exp, label.ttl, 0/*is_cw*/, mpls_tunnel_push_profile);
        if (rv != BCM_E_NONE) {
            printf("Error, in vswitch_vpls_allocate_push_profile %d\n",mpls_tunnel_push_profile);
            print rv;
            return rv;
        }
    }

    rv = bcm_mpls_tunnel_initiator_create(unit,0,1,&label);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_mpls_tunnel_initiator_create\n");
        print rv;
        return rv;
    }

    if (encoding) {
        printf("after bcm_mpls_tunnel_initiator_create: label.tunnel_id:%d out_tunnel_intf_encoding:%d\n ",label.tunnel_id,out_tunnel_intf_encoding);
    }

    /* Return pointer to MPLS tunnel intf */
    if (!encoding) {
        *out_tunnel_intf = label.tunnel_id;  
    }
    else{
        *out_tunnel_intf = out_tunnel_intf_encoding;
    }
    
    l3eg.out_tunnel_or_rif = *out_tunnel_intf;
    sal_memcpy(l3eg.next_hop_mac_addr, nh_mac_addr, 6);
    l3eg.vlan     = vlan;     
    /* l3eg.gport     = port_id; */
    l3eg.arp_encap_id = *encap_id;
	 
    if (encoding) {
        BCM_L3_ITF_SET(encap_id_encoding, BCM_L3_ITF_TYPE_LIF, l3eg.arp_encap_id);
    }
    

    rv = l3__egress_only_encap__create(unit,&l3eg);
    if (rv != BCM_E_NONE){
        printf("error, in bcm_l3_egress_create\n");
        return rv;
    }

    if (encoding) {
        printf("after bcm_l3_egress_create: l3eg.encap_id:%d encap_id_encoding:%d\n ",l3eg.arp_encap_id,encap_id_encoding);
    }

    /* Return pointer to L2 NH encap*/
    if (!encoding) {
        *encap_id = l3eg.arp_encap_id;
    }
    else{
        *encap_id = encap_id_encoding;
    }
    
    
    if (*in_tunnel_intf != 0) {
        term_flags = BCM_MPLS_SWITCH_WITH_ID;
    }
    rv = mpls_add_term_entry_info(unit, in_tunnel_label, term_flags, 0x0, in_tunnel_intf);
    
    if (rv != BCM_E_NONE){
        printf("error, in mpls_add_term_entry\n");
        return rv;
    }
       
    return rv;
}
    


/*
 *  bcm_mpls_port_add: for protected gport that allocate only LIF.
 *  the indication is by setting failover_port_id (protection gport) this will be used as learning
 *  information of the LIF
 */
int
system_mpls_port_lif_only_create(
    int unit,
    bcm_port_t port_id,  
    bcm_mpls_label_t vc_label,
    bcm_if_t intf, /* MPLS tunnel intf */
    int vpn,
    bcm_gport_t learn_fec, 
    bcm_gport_t *gport,
    bcm_mpls_port_t *mp
    ){

    int rv;
    bcm_mpls_port_t_init(mp);
    mp->criteria = BCM_MPLS_PORT_MATCH_LABEL;
    mp->port = port_id;
    BCM_GPORT_MPLS_PORT_ID_SET(mp->failover_port_id,learn_fec);
    mp->match_label = vc_label; 
    mp->egress_label.label = vc_label; /* when forwarded to this port, packet will be set with this out-vc-label */
    mp->failover_id = 0; /* no FEC allocated */
    mp->egress_tunnel_if = intf; /* when forwarded to this port, packet will be going through intf */
    mp->flags = BCM_MPLS_PORT_EGRESS_TUNNEL | (vpn? BCM_MPLS_PORT_NETWORK: 0);
    rv = mpls_port__update_network_group_id(unit, mp);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_port_lif_only_create\n");
        return rv;
    }
    if (*gport!=0) {
        mp->flags |= BCM_MPLS_PORT_WITH_ID;
        BCM_GPORT_MPLS_PORT_ID_SET(mp->mpls_port_id,*gport); 
    }
    rv = bcm_mpls_port_add(unit, vpn, mp);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_mpls_port_add\n"); 
        return rv;
    }
    if(verbose >= 1) {
        printf("unit (%d): created , mpls_port (LIF) :0x%08x with learn info FEC:0x%08x\n",unit,mp->mpls_port_id,mp->failover_port_id);
    }
    *gport = mp->mpls_port_id;  

    if(vpn > 0 && verbose){
          printf("add port  0x%08x to vswitch in unit %d\n",*gport, unit);
    }

    return BCM_E_NONE;
}

/* 
 * Creates unprotected MPLS-PORT LIF
 */
int
system_mpls_port_create(
    int unit,
    bcm_port_t port_id,  
    bcm_mpls_label_t vc_label,
    bcm_if_t intf_fec,
    bcm_vpn_t vpn, /* 0 means P2P */
    bcm_gport_t *gport,
    bcm_mpls_port_t *mp
    ){

    int rv;
    bcm_gport_t gport_encoding_subtype, gport_encoding;
    
    bcm_mpls_port_t_init(mp);
  
    mp->criteria = BCM_MPLS_PORT_MATCH_LABEL;
    mp->port = port_id;
    mp->match_label = vc_label; 
    mp->egress_label.label = vc_label; /* when forwarded to this port, packet will be set with this out-vc-label */    
    mp->egress_tunnel_if = intf_fec; /* when forwarded to this port, packet will be going through intf */
    mp->flags = BCM_MPLS_PORT_EGRESS_TUNNEL| (vpn? BCM_MPLS_PORT_NETWORK: 0);  /* This flag is only needed in mp */
    mp->egress_label.ttl = 32;
    rv = mpls_port__update_network_group_id(unit, mp);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_port_create\n");
        return rv;
    }

    if (*gport!=0) {
        mp->flags |= BCM_MPLS_PORT_WITH_ID;
        BCM_GPORT_MPLS_PORT_ID_SET(mp->mpls_port_id,*gport); 
    }
    if (encoding) {
        BCM_GPORT_SUB_TYPE_LIF_SET(gport_encoding_subtype, 0, *gport);
        BCM_GPORT_MPLS_PORT_ID_SET(gport_encoding,gport_encoding_subtype); 

        /* update sw db */
        rv = vswitch_vpls_allocate_push_profile(unit, mp->egress_label.exp, mp->egress_label.ttl, (mp->flags & BCM_MPLS_PORT_CONTROL_WORD), mpls_port_push_profile);
        if (rv != BCM_E_NONE) {
            printf("Error, in vswitch_vpls_allocate_push_profile\n");
            print rv;
            return rv;
        }
    }
    rv = bcm_mpls_port_add(unit, vpn, mp);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_mpls_port_add\n"); 
        print rv;
        return rv;
    }

    if (encoding) {
        printf("after bcm_mpls_port_add: mp->mpls_port_id:%d gport_encoding:%d\n ",mp->mpls_port_id,gport_encoding);
    }

    if (encoding) {
        printf("gport_encoding: %d  mp->mpls_port_id: %d", gport_encoding, mp->mpls_port_id);
    }
    if(verbose >= 1) {
        printf("created mpls_port:0x%08x in unit %d \n",mp->mpls_port_id, unit);
    }
    if (!encoding) {
        *gport = mp->mpls_port_id; 
    }
    else{
        *gport = gport_encoding;
    }

    if (vpn > 0) {        
        if(verbose){
            printf("add port   0x%08x to vswitch in unit %d\n",*gport, unit);
        }
    }
    return BCM_E_NONE;
}

/* 
 * P2P connection VLAN-Port unit1 <-> MPLS-Port unit2
 */
int system_mpls_port_p2p_example(int unit1, int port1, int unit2, int port2){
    int rv = BCM_E_NONE;
    uint32 v1 = 102;
    uint32 v2 = 202;
    bcm_vlan_port_t vp1;
    bcm_gport_t gport1=16384;
    uint32 pwe2 = 201;
    bcm_mpls_port_t mp2;
    bcm_gport_t gport2=16384;
    bcm_mpls_label_t l2 = 100;
    bcm_gport_t in_tunnel_intf = 32770;
    bcm_if_t tunnel_intf = 32768,encap_id = 36864, egress_intf = (is_device_or_above(unit1, JERICHO)) ? 6000 : 1024;
    bcm_l2_gport_forward_info_t forward_info;
    bcm_vswitch_cross_connect_t cross_connect;

    bcm_vswitch_cross_connect_t_init(&cross_connect);

    /* create vswitch on both units */
    rv = bcm_vlan_create(unit1, v1);
    rv = bcm_vlan_create(unit2, v2);
    
    /* create tunnel intf on unit2 */    
    rv = system_mpls_tunnel_intf_create(unit2,port2,v2,l2,l2,&in_tunnel_intf, &tunnel_intf,&encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_tunnel_intf_create port 1\n"); 
        return rv;
    }

    /* create Egress object that points to tunnel intf on unit1 */
    rv = system_mpls_tunnel_l3_egress_create(unit1,port2,tunnel_intf,&egress_intf);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_mpls_tunnel_l3_egress_create\n");
        return rv;
    }

    /* create Egress object that points to tunnel intf on unit2 */
    rv = system_mpls_tunnel_l3_egress_create(unit2,port2,tunnel_intf,&egress_intf);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_mpls_tunnel_l3_egress_create\n");
        return rv;
    }

    /* create gport2 (different gport) in unit2 */
    rv = system_mpls_port_create(unit2,port2,pwe2,egress_intf,0,&gport2,&mp2);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_port_create port 2\n"); 
        return rv;
    }

    /* create gport1 in unit1 attach to vsi */
    rv = system_vlan_port_create(unit1,port1,v1,0,&gport1,&vp1);
    if (rv != BCM_E_NONE) {
        printf("Error, system_vlan_port_create port 1\n"); 
        return rv;
    }
   
    /* 
     * adding MACT entries to unit1
     */

    /* gport1 --> gport2 */
    bcm_vswitch_cross_connect_t_init(&cross_connect);
    cross_connect.flags = BCM_VSWITCH_CROSS_CONNECT_DIRECTIONAL;
    cross_connect.port1 = gport1;
    /* remote gport info */
    if (encoding) {
      rv = system_mpls_port_to_l2_gport_forward_info(unit2, &mp2, &forward_info);
    } else {
      rv = bcm_l2_gport_forward_info_get(unit2,gport2, &forward_info);
    }
    if (rv != BCM_E_NONE) {
      printf("Error, bcm_l2_gport_forward_info_get\n");
      return rv;
    }
    cross_connect.port2 = forward_info.phy_gport;
    cross_connect.encap2 = forward_info.encap_id;
    if(verbose >= 2) {
      printf("forwarding info phy_port:0x%08x encap_id:0x%08x \n",forward_info.phy_gport, forward_info.encap_id);
    }
    rv = bcm_vswitch_cross_connect_add(unit1, &cross_connect);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_vswitch_cross_connect_add\n");
        return rv;
    }

    /* gport2 --> gport1 */
    bcm_vswitch_cross_connect_t_init(&cross_connect);
    cross_connect.flags = BCM_VSWITCH_CROSS_CONNECT_DIRECTIONAL;
    cross_connect.port1 = gport2;
    /* remote gport info */
    rv = bcm_l2_gport_forward_info_get(unit1,gport1, &forward_info);
    if (rv != BCM_E_NONE) {
      printf("Error, bcm_l2_gport_forward_info_get\n");
      return rv;
    }
    cross_connect.port2 = forward_info.phy_gport;
    cross_connect.encap2 = forward_info.encap_id;
    if(verbose >= 2) {
      printf("forwarding info phy_port:0x%08x encap_id:0x%08x \n",forward_info.phy_gport, forward_info.encap_id);
    }
    rv = bcm_vswitch_cross_connect_add(unit2, &cross_connect);
    if (rv != BCM_E_NONE) {
      printf("Error, bcm_vswitch_cross_connect_add\n");
      return rv;
    }

    return rv;
}

/* 
 * Scheme is: 
 * unit1 -> MPLS_PORT1 pwe1 (gport1) and VLAN_PORT2 v1 (second_gport1) 
 * uint2 -> MPLS_PORT2 pwe2 (gport2) 
 */
int system_mpls_port_mp_example(int unit1, int port1, int second_port1, int unit2, int port2){
    int rv = BCM_E_NONE;    
    uint32 v1 = 10;
    bcm_mpls_port_t mp1;
    bcm_gport_t gport1=16384;
    uint32 v2 = 30;
    bcm_mpls_port_t mp2;
    bcm_gport_t gport2=16384;
    bcm_vlan_port_t second_gport_vp1;
    bcm_gport_t second_gport1=8196;
    bcm_l2_addr_t l2addr;
    bcm_mac_t mac1  = {0x00, 0x00, 0x00, 0x00, 0x0a, 0x11};
    bcm_mac_t mac2  = {0x00, 0x00, 0x00, 0x00, 0x0a, 0x22};
    bcm_mac_t mac3  = {0x00, 0x00, 0x00, 0x00, 0x0a, 0x33};
    bcm_vlan_t vsi = 6000;
    bcm_l2_gport_forward_info_t forward_info;
    bcm_mpls_label_t l1 = 100;
    bcm_mpls_label_t l2 = 101;
    uint32 pwe1 = 201;
    uint32 pwe2 = 202;
    bcm_if_t tunnel_intf = 32768,encap_id = 36864,
        egress_intf1 = (is_device_or_above(unit1, JERICHO)) ? 6000 : 1024, 
        egress_intf2 = (is_device_or_above(unit1, JERICHO)) ? 6002 : 1026;
    bcm_gport_t in_tunnel_intf = 32770;

    rv = mpls__mpls_pipe_mode_exp_set(unit1);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    rv = mpls__mpls_pipe_mode_exp_set(unit2);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }
    

    /* create vswitch VPLS on both devices*/
    mpls_port__vswitch_vpls_vpn_create__set(unit1, vsi);
    mpls_port__vswitch_vpls_vpn_create__set(unit2, vsi);

    /* create tunnel intf gport1 on unit1 */    
    rv = system_mpls_tunnel_intf_create(unit1,port1,v1,l1,l1,&in_tunnel_intf, &tunnel_intf,&encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_tunnel_intf_create port 1\n"); 
        return rv;
    }

    /* create Egress object that points to tunnel intf gport1 on unit1 */
    rv = system_mpls_tunnel_l3_egress_create(unit1,port1,tunnel_intf,&egress_intf1);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_mpls_tunnel_l3_egress_create\n");
        return rv;
    }

    /* create Egress object that points to tunnel intf gport1 on unit2 */
    rv = system_mpls_tunnel_l3_egress_create(unit2,port1,tunnel_intf,&egress_intf1);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_mpls_tunnel_l3_egress_create\n");
        return rv;
    }
    
    if(verbose >= 2) {
      printf("created MPLS tunnel nhp egrses_intf: 0x%08x , encap_id: 0x%08x , tunnel_intf: 0x%08x \n",
             egress_intf1,encap_id,tunnel_intf);
    }
    tunnel_intf = 32768; encap_id = 36864;
    /* create tunnel intf gport2 on unit2 */    
    rv = system_mpls_tunnel_intf_create(unit2,port2,v1,l2,l2,&in_tunnel_intf, &tunnel_intf,&encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_tunnel_intf_create port 1\n"); 
        return rv;
    }

    /* create Egress object that points to tunnel intf gport2 on unit1 */
    rv = system_mpls_tunnel_l3_egress_create(unit1,port2,tunnel_intf,&egress_intf2);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_mpls_tunnel_l3_egress_create, third one\n");
        return rv;
    }

    /* create Egress object that points to tunnel intf gport2 on unit2 */
    rv = system_mpls_tunnel_l3_egress_create(unit2,port2,tunnel_intf,&egress_intf2);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_mpls_tunnel_l3_egress_create\n");
        return rv;
    }

    if(verbose >= 2) {
      printf("created MPLS tunnel nhp egrses_intf: 0x%08x , encap_id: 0x%08x , tunnel_intf: 0x%08x \n",
             egress_intf2,encap_id,tunnel_intf);
    }

    /* create gport1 in unit1 attach to vsi */
    rv = system_mpls_port_create(unit1,port1,pwe1,egress_intf1,vsi,&gport1,&mp1);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_port_create port 1\n"); 
        return rv;
    }

    /* create second_gport1 in unit1 */
    rv = system_vlan_port_create(unit1,second_port1,v2,vsi,&second_gport1,&second_gport_vp1);
    if (rv != BCM_E_NONE) {
        printf("Error, system_vlan_port_create port 2\n"); 
        return rv;
    }

    /* create gport2 (different gport) in unit2 */
    rv = system_mpls_port_create(unit2,port2,pwe2,egress_intf2,vsi,&gport2,&mp2);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_port_create port 2\n"); 
        return rv;
    }

    /* Adding the gports to the multicast */
    rv = system_multicast_mpls_port_add(unit1, unit2, port1, gport1, vsi);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_multicast_mpls_port_add\n");
        return rv;
    }

    rv = system_multicast_vlan_port_add(unit1, unit2, second_port1, second_gport1, vsi);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_multicast_vlan_port_add\n");
        return rv;
    }

    rv = system_multicast_mpls_port_add(unit2, unit1, port2, gport2, vsi);
    if (rv != BCM_E_NONE) {
        printf("Error, in system_multicast_mpls_port_add\n");
        return rv;
    }

    /* 
     * adding MACT entries to unit1
     */

    /* dest is on gport2 in unit2 (remote)*/
    bcm_l2_addr_t_init(&l2addr, mac1, vsi);
    l2addr.vid = vsi;
    l2addr.flags = BCM_L2_STATIC;
    /* map gport to system encapsulation info */
    if (encoding) {
        rv = system_mpls_port_to_l2_gport_forward_info(unit2, &mp2, &forward_info);
    } else {
        rv = bcm_l2_gport_forward_info_get(unit2,gport2, &forward_info);
    }
    l2addr.port = forward_info.phy_gport;
    l2addr.encap_id = forward_info.encap_id;

    if(verbose >= 2) {  
        printf("forwarding info phy_port:0x%08x encap_id:0x%08x \n",forward_info.phy_gport, forward_info.encap_id);
    }

    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_gport_forward_info_get\n");
        return rv;
    }
    rv = bcm_l2_addr_add(unit1, &l2addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add 1\n");
        return rv;
    }
    if (verbose > 1) {
        printf("add 1:");
        bcm_l2_addr_dump(&l2addr);
    }
    /* dest is gport1 in unit1 (local)
       there are two options:
        1. set port as gport1.
        2. similar to above */
    bcm_l2_addr_t_init(&l2addr, mac2, vsi);
    l2addr.vid = vsi;
    l2addr.flags = BCM_L2_STATIC;
    l2addr.port = gport1;
    rv = bcm_l2_addr_add(unit1, &l2addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add 1\n");
        return rv;
    }
    if (verbose > 1) {
        printf("add 2:");
        bcm_l2_addr_dump(&l2addr);
    }

    /* Dest is second gport1, we'll add it in both units */
    bcm_l2_addr_t_init(&l2addr, mac3, vsi);
    l2addr.vid = vsi;
    l2addr.flags = BCM_L2_STATIC;
    rv = bcm_l2_gport_forward_info_get(unit1,second_gport1, &forward_info);
    l2addr.port = forward_info.phy_gport;
    l2addr.encap_id = forward_info.encap_id;
    if(verbose >= 2) {
     printf("forwarding info phy_port:0x%08x encap_id:0x%08x \n",forward_info.phy_gport, forward_info.encap_id);
    }
    
    rv = bcm_l2_addr_add(unit2, &l2addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add 1\n");
        return rv;
    }
    if (verbose > 1) {
        printf("Unit %d: add 3:", unit2);
        bcm_l2_addr_dump(&l2addr);
    }

    rv = bcm_l2_addr_add(unit1, &l2addr);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_addr_add 1\n");
        return rv;
    }
    if (verbose > 1) {
        printf("Unit %d: add 3:", unit1);
        bcm_l2_addr_dump(&l2addr);
    }


    return rv;
}

int system_multicast_mpls_port_add(int local_unit, int remote_unit, bcm_port_t port, bcm_gport_t gport, int mc_group_id) {
    int rv;
    int encap_id;
    int modid, modport;

    rv = bcm_multicast_vpls_encap_get(local_unit, mc_group_id, port, gport, &encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, in bcm_multicast_vpls_encap_get mc_group_id:  0x%08x  phy_port:  0x%08x  gport:  0x%08x \n", mc_group_id, port, gport);
        return rv;
    }

    if (!BCM_GPORT_IS_MODPORT(port) && !BCM_GPORT_IS_SYSTEM_PORT(port)) {
        if (is_device_or_above(local_unit,JERICHO)) {
            rv = get_core_and_tm_port_from_port(unit,port,&modid,&port); 
            if (rv != BCM_E_NONE){
                printf("Error, in get_core_and_tm_port_from_port\n");
                return rv;
            }
        }
        else {
            rv = bcm_stk_modid_get(local_unit, &modid);
            if (rv != BCM_E_NONE){
                printf("Error, in bcm_stk_modid_get\n");
                return rv;
            }
        }

        BCM_GPORT_MODPORT_SET(modport, modid, port);
    } else {
        modport = port;
    }

    rv = multicast__add_multicast_entry(local_unit, mc_group_id, &modport , &encap_id, 1, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, in multicast__add_multicast_entry local_unit: %d mc_group_id:  0x%08x  modport:  0x%08x  encap_id:  0x%08x \n", local_unit, mc_group_id, modport, encap_id);
        return rv;
    }

    rv = multicast__add_multicast_entry(remote_unit, mc_group_id, &modport , &encap_id, 1, 0);
    if (rv != BCM_E_NONE) {
        printf("Error, in multicast__add_multicast_entry remote_unit: %d mc_group_id:  0x%08x  modport:  0x%08x  encap_id:  0x%08x \n", remote_unit, mc_group_id, modport, encap_id);
        return rv;
    }

    return rv;
}

/* 
 * MPLS_PORT 1 (gport2) 5000+5002 
 * VLAN_PORT 1 (gport1) 5000
 */
int system_protected_mpls_port_example(int unit1, int port1, int unit2, int port2,int port3,int is_mp){
    int rv = BCM_E_NONE;    
    uint32 v1 = 10;
    int base = 0;
    /* on unit2 */
    bcm_mpls_port_t mp2_lif_a;
    bcm_gport_t gport2_lif_a =17000+base;
    bcm_mpls_port_t mp2_lif_b;
    bcm_gport_t gport2_lif_b =17002+base;
    /* on unit1 */
    bcm_gport_t gport2_fec_a =6000+base;
    bcm_gport_t gport2_fec_b =6001+base;
    uint32 v2 = 20;
    uint32 v3 = 30;
    bcm_mpls_label_t l1 = 100;
    bcm_mpls_label_t l2 = 101;
    uint32 pwe1 = 201;
    uint32 pwe2 = 202;
    /* on unit1 */
    bcm_vlan_port_t vp1_lif;
    bcm_gport_t gport1_lif   =17000+base;

    /* mp data */
    bcm_l2_addr_t l2addr;
    bcm_mac_t mac1  = {0x00, 0x00, 0x00, 0x00, 0x0a, 0x11};
    bcm_mac_t mac2  = {0x00, 0x00, 0x00, 0x00, 0x0a, 0x22};
    bcm_mac_t mac3  = {0x00, 0x00, 0x00, 0x00, 0x0a, 0x33};
    bcm_vlan_t vsi;
    bcm_failover_t failover_id;
    /* p2p data */
    bcm_vswitch_cross_connect_t cross_connect;
    bcm_l2_gport_forward_info_t forward_info;
    bcm_if_t tunnel_intf = 32768,encap_id = 36864;
    bcm_gport_t in_tunnel_intf = 32770;
    mpls_port__fec_only_info_s mpls_port_fec_info;

    bcm_vswitch_cross_connect_t_init(&cross_connect);

    rv = mpls__mpls_pipe_mode_exp_set(unit1);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    rv = mpls__mpls_pipe_mode_exp_set(unit2);
    if (rv != BCM_E_NONE) {
        printf("Error, in mpls__mpls_pipe_mode_exp_set\n");
        return rv;
    }

    if (is_mp) {
      vsi = 7000;
      /* create vswitch on both devices*/
      mpls_port__vswitch_vpls_vpn_create__set(unit1, vsi);
      mpls_port__vswitch_vpls_vpn_create__set(unit2, vsi);      
    }
    else {
      vsi = 0;/* 0 means not connected to mp VSI */
    }

    /* create gport1 in unit1 without protection  */
    rv = system_vlan_port_create(unit1,port1,v1,vsi,&gport1_lif,&vp1_lif);
    if (rv != BCM_E_NONE) {
        printf("Error, system_vlan_port_create port 1\n"); 
        return rv;
    }

    /* create failover id */
    rv = bcm_failover_create(unit1, BCM_FAILOVER_FEC, &failover_id);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_failover_create 1\n"); 
        return rv;
    }

    g_failover_id = failover_id;

    printf("Failover id: 0x%x", failover_id);

    /* create tunnel intf on unit2 */    
    rv = system_mpls_tunnel_intf_create(unit2,port2,v2,l2,l2,&in_tunnel_intf,&tunnel_intf,&encap_id);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_tunnel_intf_create port 1\n"); 
        return rv;
    }

    /* 
     * 4 calls below, create protected mpls-port which includes:
     *  unit1: FEC1, FEC2
     *  unit2: LIF1, LIF2
     */

    /* create two FEC entries, each point to <port,LIF> and share the same Failver-id*/
    mpls_port_fec_info.port_id = port2;
    mpls_port_fec_info.failover_id = failover_id;
    mpls_port_fec_info.encap_id = gport2_lif_b;
    mpls_port_fec_info.gport = gport2_fec_b;

    rv = mpls_port__fec_only__create(unit1, &mpls_port_fec_info);
    if (rv != BCM_E_NONE) {
        printf("Error, mpls_port__fec_only__create 1\n"); 
        return rv;
    }

    gport2_fec_b = mpls_port_fec_info.gport;

    mpls_port_fec_info.port_id = port3;
    mpls_port_fec_info.failover_port_id = mpls_port_fec_info.gport;
    mpls_port_fec_info.encap_id = gport2_lif_a;
    mpls_port_fec_info.gport = gport2_fec_a;

    rv = mpls_port__fec_only__create(unit1, &mpls_port_fec_info);
    if (rv != BCM_E_NONE) {
        printf("Error, mpls_port__fec_only__create 1\n"); 
        return rv;
    }

    gport2_fec_a = mpls_port_fec_info.gport;

    /* create gport2.LIFa with learning info as FEC but without creating FEC entry */
    rv = system_mpls_port_lif_only_create(unit2,port2,pwe1,tunnel_intf,vsi,gport2_fec_b,&gport2_lif_b,&mp2_lif_b);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_port_lif_only_create port 2\n"); 
        return rv;
    }
    /* create gport2.LIFb with learning info as FEC but without creating FEC entry */
    rv = system_mpls_port_lif_only_create(unit2,port3,pwe2,tunnel_intf,vsi,gport2_fec_a,&gport2_lif_a,&mp2_lif_a);
    if (rv != BCM_E_NONE) {
        printf("Error, system_mpls_port_lif_only_create port 3\n"); 
        return rv;
    }


    /* 
     * adding MACT entries to unit1 point to the FEC
     */

    if (is_mp) {
		/* Adding the gports to the multicast */
        rv = system_multicast_vlan_port_add(unit1, unit2, port1, gport1_lif, vsi);
        if (rv != BCM_E_NONE) {
            printf("Error, in system_multicast_vlan_port_add\n");
            return rv;
        }

        rv = system_multicast_mpls_port_add(unit2, unit1, port2, gport2_lif_a, vsi);
        if (rv != BCM_E_NONE) {
            printf("Error, in system_multicast_mpls_port_add\n");
            return rv;
        }

        rv = system_multicast_mpls_port_add(unit2, unit1, port3, gport2_lif_b, vsi);
        if (rv != BCM_E_NONE) {
            printf("Error, in system_multicast_mpls_port_add\n");
            return rv;
        }
		
        bcm_l2_addr_t_init(&l2addr, mac2, vsi);
        l2addr.vid = vsi;
        l2addr.flags = BCM_L2_STATIC;
        l2addr.port = gport2_fec_a;
        rv = bcm_l2_addr_add(unit1, &l2addr);
        if (rv != BCM_E_NONE) {
          printf("Error, bcm_l2_addr_add 1\n");
          return rv;
        }
        if (verbose > 1) {
          printf("add 2:");
          bcm_l2_addr_dump(&l2addr);
        }
   }
   else{

     /* gport1_lif --> gport2_fec_a (both exist in unit1 */
     bcm_vswitch_cross_connect_t_init(&cross_connect);
     cross_connect.flags = BCM_VSWITCH_CROSS_CONNECT_DIRECTIONAL;
     /* gport info [optional see option2 below ] */
     if (encoding) {
         bcm_mpls_port_t mp_fec;
         bcm_mpls_port_t_init(&mp_fec);

         mp_fec.failover_id = failover_id;
         mp_fec.flags |= BCM_MPLS_PORT_WITH_ID;
         mp_fec.mpls_port_id = mpls_port_fec_info.gport;
         rv = system_mpls_port_to_l2_gport_forward_info(unit1,&mp_fec, &forward_info);
     } else {
       rv = bcm_l2_gport_forward_info_get(unit1,gport2_fec_a, &forward_info);
     }
     if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_gport_forward_info_get\n");
        return rv;
     }
     cross_connect.port1 = gport1_lif;
     cross_connect.port2 = forward_info.phy_gport;
     cross_connect.encap2 = forward_info.encap_id;
     if(verbose >= 2) {
        printf("forwarding info of gport(gport2_fec_a):0x%08x is phy_port:0x%08x encap_id:0x%08x \n",gport2_fec_a, forward_info.phy_gport, forward_info.encap_id);
     }
     rv = bcm_vswitch_cross_connect_add(unit1, &cross_connect);
     if (rv != BCM_E_NONE) {
        printf("Error, bcm_vswitch_cross_connect_add\n");
        return rv;
     }
     /* option2: FOR FEC IT POSSIBLE TO ADD as destination-gport directly (without calling bcm_l2_gport_forward_info_get) as in this case gport = FEC,
          bcm_vswitch_cross_connect_t_init(&cross_connect);
          cross_connect.flags = BCM_VSWITCH_CROSS_CONNECT_DIRECTIONAL;
          cross_connect.port1 = gport1_lif;
          cross_connect.port2 = forward_info.gport2_fec_a;
          rv = bcm_vswitch_cross_connect_add(unit1, &cross_connect);
          if (rv != BCM_E_NONE) {
              printf("Error, bcm_vswitch_cross_connect_add:gport1_lif\n");
              return rv;
          }
      */


      /* gport2_lif_a --> gport1_lif */
      bcm_vswitch_cross_connect_t_init(&cross_connect);
      cross_connect.flags = BCM_VSWITCH_CROSS_CONNECT_DIRECTIONAL;
      /* remote gport info */
      rv = bcm_l2_gport_forward_info_get(unit1,gport1_lif, &forward_info);
      if (rv != BCM_E_NONE) {
        printf("Error, bcm_l2_gport_forward_info_get\n");
        return rv;
      }
      cross_connect.port1 = gport2_lif_a;
      cross_connect.port2 = forward_info.phy_gport;
      cross_connect.encap2 = forward_info.encap_id;
      if(verbose >= 2) {
          printf("forwarding info of gport(gport1_lif):0x%08x is phy_port:0x%08x encap_id:0x%08x \n",gport2_fec_a, forward_info.phy_gport, forward_info.encap_id);
      }
      rv = bcm_vswitch_cross_connect_add(unit2, &cross_connect);
      if (rv != BCM_E_NONE) {
          printf("Error, bcm_vswitch_cross_connect_add:gport2_lif_a\n");
          return rv;
      }

      /* gport2_lif_b --> gport1_lif */
      cross_connect.port1 = gport2_lif_b;
      rv = bcm_vswitch_cross_connect_add(unit2, &cross_connect);
      if (rv != BCM_E_NONE) {
          printf("Error, bcm_vswitch_cross_connect_add:gport2_lif_b\n");
          return rv;
      }
   }

   return rv;
}

