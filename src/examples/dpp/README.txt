 $Id: README.txt,v 1.99 Broadcom SDK $
 $Copyright: (c) 2016 Broadcom.
 Broadcom Proprietary and Confidential. All rights reserved.$
   
 ~ Name:      
+-----------------------+                                                     
|    Link Monitoring    |                                                      
+-----------------------+                                                        
 ~ Description:                                                                
    Example application to monitor the state of the links and to react to ports/links failure.                                            
                                                                        
 ~ File Reference:                                                              
    cint_linkscan.c                                                           
  
  
 ~ Name:     
+-------------------------+                                                        
|    Snake Application    |                                                         
+-------------------------+                                                       
 ~ Description:                                                                 
    Setup that enables running 80G traffic, while only injecting 10G from the external traffic generator.                                  
                                                                              
 ~ File Reference:                                                              
    cint_snake.c                                                             

	
 ~ Name:   
+---------------------------------------------------+                                                        
|    TM Application - Priority Over Port (Ardon)    |                                                         
+---------------------------------------------------+                                                       
 ~ Description:                                                                 
    Provides an example of building scheduling scheme for 'Priority Over Port' with 8 priorities. 
    This cint is directed towards systems with reduced resources (Ardon).     
    Also includes examples for: Adding ports, MC, ISQ, CGM and other configurations.                                    
                                                                              
 ~ File Reference:                                                              
    cint_tm_prio_o_port_reduce_resources.c    
 
 
 ~ Name:   
+-------------------------+                                                        
|    Multicast Options    |                                                         
+-------------------------+                                                         
 ~ Description:  
    Broadcom devices support a variety of multicast options, with different scheduling options. 
                  
 ~ File Reference:
    cint_multicast_applications.c                  
 

 ~ Name:   
+--------------------------------------+                                                        
|    Port-Level Scheduled Multicast    |                                                         
+--------------------------------------+ 
 ~ Description: 
    Configuration results in a fully scheduled MC scheme. Its drawback is that it consumes a lot of resources.

 ~ File Reference:                                                              
    cint_multicast_application_port_lvl_example.c
    cint_multicast_application_port_lvl_example_multi_device.c          
   
 ~ Name:   
+----------------------------------------+                                                        
|    Device-Level Scheduled Multicast    |                                                         
+----------------------------------------+    
 ~ Description:
    Configuration results in a partially scheduled MC scheme. It consumes relatively few resources. 
	It is a viable compromise between port-level scheduled MC and Fabric MC.    
    
 ~ File Reference:
    cint_multicast_application_device_lvl_example.c              
    cint_multicast_application_device_lvl_example_multi_device.c 
    
	
 ~ Name:
+------------------------+                                                        
|    Fabric Multicast    |                                                         
+------------------------+  
 ~ Description:
    Configuration results in unscheduled MC scheme. This configuration provides no end-to-end scheduling by the Credit Scheduler.
	Ingress scheduling is based on the ingress queue.

 ~ File Reference:
    cint_fabric_mc_credit_elements.c          
    cint_enhance_application_fap.c        

	
 ~ Name:	
+------------------------------+       
|    Arad Special Multicast    |    
+------------------------------+
 ~ Description:
    An example of port level multicast configuration. 
 
 ~ File Reference:
    cint_multicast_special_egress_mode.c   
 
 
 ~ Name: 
+------------+     
|    CosQ    |    
+------------+ 
 ~ Description:                                                             
    Examples of traffic-management applications.

~ File Reference:


 ~ Name: 
+--------------------+     
|    QoS   Utilites  |    
+--------------------+ 
 ~ Description:                                                             
    QoS module utils.

~ File Reference:
	cint_utils_qos.c
	
 ~ Name:
+-----------------------+                                                        
|    Egress Transmit    |                                                         
+-----------------------+  
 ~ Description:
    Covers simple egress transmit scheduling setup and the setup of egress thresholds.  

 ~ File Reference:
    cint_egress_transmit_scheduler.c           
    cint_egress_threshold.c      
   

 ~ Name:
+----------------------+                                                        
|    E2E Scheduling    |                                                         
+----------------------+  
 ~ Description:
    Covers the E2E hierarchy setup. Examples are given for the hookup of different element types.            

 ~ File Reference:
    - General: cint_tm_fap_with_id.c        
    - SP and Ingress Shapping Validation: cint_tm_fap_config2.c
    - WFQ Validation: cint_e2e_wfq.c   
    - Composite SE Validation: cint_e2e_composite_se.c
    - Composite Connector Validation: cint_e2e_composite_connectors.c  
    - Dual Shaper Validation: cint_e2e_dual_shaper.c
    - Interdigitated mode Validateion: cint_e2e_interdigitated.c
    cint_tm_fap.c
    cint_tm_fap_2_devices.c
    cint_tm_fap_config_large_scheduling_scheme.c

	
 ~ Name:
+----------------------+                                                        
|    Flow Control 1    |                                                         
+----------------------+ 
 ~ Description:
    Covers configurations for different FC options. Each function configures a specific option (such as PFC Reception).

 ~ File Reference:
    - Calendar interfaces: cint_fc_arad.c 
    - Reception examples: cint_fc_rx.c
    - Generation examples: cint_fc_tx.c
    - OOB examples: cint_arad_fc_oob_config_example.c
                    cint_jer_fc_oob_config_example.c
    - Inband examples: cint_arad_fc_inbnd_config_example.c
                       cint_jer_fc_ilkn_inb_config_example.c
                       cint_jer_fc_trigger_config_example.c
    - PFC Reception mapping examples: cint_arad_pfc_reception_mapping.c
    - COE examples: cint_jer_fc_coe_config_example.c

	
 ~ Name:
+----------------------+                                                        
|    Flow Control 2    |                                                         
+----------------------+   
 ~ Description:
    An example of how to configure flow control for specific VSQs (LLFC and PFCl).  

 ~ File Reference:
    cint_vsq_fc_examples.c
	
	
 ~ Name:
+-----------------------------+                                                        
|    Ingress Queue Manager    |                                                         
+-----------------------------+  
 ~ Description:
    Covers configurations such as setting queue maximum size, WRED application, and Incoming TC mapping. 
    
 ~ File Reference: 
    cint_itm_examples.c

	
 ~ Name:	
+----------------------+                                                        
|    VOQ Attributes    |                                                        
+----------------------+  
 ~ Description:
    Various functions for testing Arad VOQ attributes.

 ~ File Reference:
    cint_queue_tests.c

	
 ~ Name:
+------------------+                                                        
|    System RED    |                                                        
+------------------+   
 ~ Description:  
    An example of setting System RED configuration.  
                  
 ~ File Reference:
    cint_system_red.c


 ~ Name:
+-------------------------------------+                                                        
|    Unicast Flow-based Forwarding    |                                                        
+-------------------------------------+  
 ~ Description:
    Unicast Flow-based Forwarding includes a TM hierarchy scheduling example and a PP application associating destination to TM-flow ID.

 ~ File Reference:
    cint_pp_config2.c, cint_tm_fap_config2.c


 ~ Name:
+------------------------+     
|    VSQ Applications    |    
+------------------------+ 
 ~ Description:
    Examples for VSQ applications.
	
 ~ File Reference:
    
	
 ~ Name:
+-------------------------+                                                        
|    Global Thresholds    |                                                        
+-------------------------+  
 ~ Description:
    Global VSQ holds counters on available resources per resource type. This is useful to trigger DP-dependent drop decisions and flow control.   
    
 ~ File Reference:
    cint_vsq_example.c    
    cint_vsq_tc_examples.c
    cint_vsq_gl_example.c    

	
 ~ Name:
+-------------------------+                                                        
|    VSQ Traffic Class    |                                                         
+-------------------------+   
 ~ Description:
    The focus of the cint is to show how to enable a tail drop and WRED settings using this property.  
    
 ~ File Reference:
    cint_vsq_tc_examples.c

	
 ~ Name:
+-------------------------------+                                                        
|    VSQ Resource Allocation    |                                                         
+-------------------------------+   
 ~ Description:
    An example of how to configure global and per VSQ resource allocation.

 ~ File Reference:
    cint_vsq_resource_allocation.c


 ~ Name:
+----------------------------+                                                        
|    OTN/TDM Applications    |                                                         
+----------------------------+  
 ~ Description:
    Examples of OTN/TDM applications including TDM standard, TDM optimize, and TDM packet mode. 
    Packet mode is often used when expected TDM traffic consists of packet sizes that are higher than 256.
    
 ~ File Reference:
    cint_tdm_example.c 
    cint_tdm_packet_mode_example.c
	
	
 ~ Name:
+--------------+                                                        
|    Bridge    |                                                         
+--------------+
 ~ Description:
    Enable activating the BCM88650 (Arad) in various modes: IPv4/6 Bridge Router, VPLS LER, MPLS LSR, etc. Relevant to PP users. 
    
 ~ File Reference:
 
	
 ~ Name:
+----------------------------+                                                        
|    Traditional Bridge 1    |                                                         
+----------------------------+ 
 ~ Description:
    After the boot up, the device acts as a traditional bridge.  
    By this setting, you can switch packets from one port to another by means of the Ethernet header. This also enables users to define VLANs.
    
 ~ File Reference:
    - Vlan bridge: cint_vlan_basic_bridge.c
    - Mact Management: cint_l2_mact.c      
    - L2 Cache: cint_l2_cache.c     
    - Port TPID: cint_port_tpid.c    
    - Spanning-Tree Groups: cint_stg.c        
    - VLAN Port gport membership: cint_vlan_gport_add.c
    - Utility function for MACT configuration: cint_mact.c
	
	
 ~ Name:
+----------------------------+                                                        
|    Traditional Bridge 2    |                                                         
+----------------------------+ 
 ~ Description:
    Examples of L2 QoS.
	
 ~ File Reference:
    cint_qos_l2.c
    
  
 ~ Name:
+--------------------+                                                        
|    Metro Bridge    |                                                         
+--------------------+ 
 ~ Description:
    This application enables you to define an L2VPN service. 
	In VPN, you can create a Virtual Switch Instance (VSI/VPN) and connect logical ports to it.       
    
 ~ File Reference:
    - P2P Service: cint_vswitch_metro_p2p.c                                 
				   cint_vswitch_metro_p2p_multi_device.c             
    - VPLS QoS P2P Service: Example of Settings QoS in VPLS application
				   cint_qos_vpls.c
    - Multipoint Metro Service: 
				   cint_vswitch_metro_mp.c - Supports multi device. Please note that some function signatures might have changed.
    - Multipoint VPLS Service: cint_vswitch_vpls.c                               
				   Simple example of Metro scheme: cint_vswitch_metro_simple.c
    

 ~ Name:
+------------------------------------+                                                        
|    Bridge Flooding VLAN Setting    |                                                         
+------------------------------------+ 
 ~ Description:
    Examples for bridge(VLAN/VSI/VPN) flooding VLAN settings. 
    Demonstrate how to separately set unknown unicast groups, unknow multicast groups and broadcast groups of a Virtual Switch Instance (VLAN/VSI/VPN).

 ~ File Reference:
    cint_vlan_control_config.c

	
 ~ Name:
+---------------------+                                                        
|    Bridge Router    |                                                         
+---------------------+ 
 ~ Description:
    This application enables you to define L3 routers. You can also define IP Unicast Routing, IP Multicast Routing and MPLS LSR.           
    
 ~ File Reference:
    - L3 Routing Examples: cint_ipv4_fap.c                                             
						   cint_ip_route_tunnel.c
    - IPMC Examples: cint_ipmc_example.c                   
					 cint_ipmc_example_multi_device.c      
    - MPLS LSR: cint_mpls_lsr.c 
				cint_mpls_tunnel_initiator.c
				cint_mpls_mc.c
    - ARP request/reply trapping: cint_arp_traps.c

		
 ~ Name:
+--------------------------+                                                        
|    IP Unicast Routing    |                                                         
+--------------------------+ 
 ~ Description:
    An example of IPv4 unicast router. Demonstrates how to: 
    - Open the router interface 
    - Create egress objects 
    - Add route and host entries 
    - Add host entries while saving FEC entries 
    - Set VRF default destination 

 ~ File Reference:
    cint_ip_route.c


 ~ Name:
+---------------------------------------+                                                        
|    IP Unicast Routing Explicit RIF    |                                                         
+---------------------------------------+ 
 ~ Description:
    An example of IPv4 routing over 802.1q where the VSI/RIF is explicitly supplied by the user and not determined by the incoming VLAN value. 
    The examples include: 
    -	LIF Creation with an explicit VSI 
    -	RIF Creation according to <port, VLAN> 
    -	Egress VLAN edit configuration 
     
 ~ File Reference:
    src/examples/dpp/cint_ip_route_explicit_rif.c
    src/examples/dpp/utility/cint_utils_global.c
    src/examples/dpp/utility/cint_utils_l2.c
    src/examples/dpp/utility/cint_utils_l3.c
    src/examples/dpp/utility/cint_utils_port.c
    src/examples/dpp/utility/cint_utils_vlan.c

	
 ~ Name:
+----------------------+                                                        
|    IP Unicast RPF    |                                                         
+----------------------+ 
 ~ Description:
    An example of IPv4 unicast RPF check. Demonstrates how to:
    - Set loose and strict RPF mode
    - Configure the RPF traps to drop or trap packet

 ~ File Reference:
    cint_ip_route_rpf.c

	
 ~ Name:
+--------------------+                                                        
|    IP Multicast    |                                                         
+--------------------+
 ~ Description:
    An example of configuring ingress and egress for IP multicast, adding IPv4 and IPv6 MC entries and traversing IPMC table.

 ~ File Reference:
    cint_ipmc.c  

	
 ~ Name:
+------------------------+                                                        
|    IP Multicast RPF    |                                                         
+------------------------+
 ~ Description:
    An Example of IPv4 Multicast RPF check. Demonstrates how to set PF mode: SIP, explicit and none per egress object.

 ~ File Reference:
    cint_ipmc_example_rpf.c

	
 ~ Name:
+--------------------------+                                                        
|    IP Multicast BIDIR    |                                                         
+--------------------------+
 ~ Description:
    An example of IPv4 Multicast with BIDIR. Demonstrates mapping IP group to BIDIR and forwarding packets according to <VRF,Group> with admission test.

 ~ File Reference:
    cint_ipmc_bidir.c

	
 ~ Name:
+--------------------------+                                                        
|    IP Multicast Flows    |                                                         
+--------------------------+
 ~ Description:
    Example of the various IPMC flows. Each flow is exemplified with its unique configuration and an exemplary packet flow.

 ~ File Reference:
    cint_ipmc_flows.c

	
 ~ Name:
+-------------+                                                        
|    IPv6     |                                                         
+-------------+
 ~ Description:
    An example of IPv6 unicast and multicast routing. 

 ~ File Reference:
    cint_ipv6_fap.c
  

 ~ Name:
+---------------------------+                                                        
|    IP Routing Enablers    |                                                         
+---------------------------+ 
 ~ Description:
    Tests the routing enablers feature by using all the possible resources of the routing enablers.
    This test combines IPV4 UC (with and without RPF/VPN), IPV6 UC (with and without RPF), IP V4 MC (bridge and route) and IPV6 (bridge and route).

 ~ File Reference:
    cint_enablers.c
  
  
 ~ Name:
+-----------------+                                                        
|    ARP Traps    |                                                         
+-----------------+ 
 ~ Description:
    An example of Trapping ARP packets to CPU.

 ~ File Reference:
    cint_arp_traps.c

	
 ~ Name:
+--------------------+                                                        
|    Trap Setting    |                                                         
+--------------------+ 
 ~ Description:
    Demonstrates how to configure trap and snoop according to trap code.

 ~ File Reference:
    cint_trap_mgmt.c
    cint_egr_mtu.c


 ~ Name:
+------------+                                                        
|    ECMP    |                                                         
+------------+ 
 ~ Description:
    An example of multipath routing.

 ~ File Reference:
    cint_l3_multipath.c

	
 ~ Name:
+------------+                                                        
|    EVPN    |                                                         
+------------+ 
 ~ Description:
    Example of an EVPN application.
	
 ~ File Reference:
    cint_evpn.c
    cint_field_evpn.c

	
 ~ Name:
+--------------------+                                                        
|    MPLS LSR QoS    |                                                         
+--------------------+ 
 ~ Description:
    Example of MPLS QoS.
	
 ~ File Reference:
    cint_qos.c, cint_qos_elsr.c

	
 ~ Name:
+---------------------+                                                        
|    Fiber Channel    |                                                         
+---------------------+ 
 ~ Description:  
    Example of FCoE. Demonstrates forwarding FCoE packet according to fiber headers. Includes zoning check.
	
 ~ File Reference:
    cint_fcoe_route.c


 ~ Name:
+-----------------+                                                        
|    MPLS ELSP    |                                                         
+-----------------+ 
 ~ Description:  
    Example of MPLS ELSP.
	
 ~ File Reference:
    cint_qos_elsr.c, cint_mpls_elsp.c

	
 ~ Name:
+-------------------------------------+                                                        
|    MPLS/EXP Marking for Bridging    |                                                         
+-------------------------------------+ 
 ~ Description:
    Example of marking MPLS/EXP values when the packet is bridged (not routed).
    (BCM88660 Only)
	
 ~ File Reference:
    cint_qos_marking.c


 ~ Name:
+----------------------+                                                        
|    MPLS Utilities    |                                                         
+----------------------+
 ~ Description:
    Utility functions for MPLS applications.
	
 ~ File Reference:
    cint_utils_mpls.c

	
 ~ Name:
+----------------------+                                                        
|    VPLS Utilities    |                                                         
+----------------------+ 
 ~ Description:
    Utility functions for VPLS applications.
	
 ~ File Reference:
    cint_utils_mpls_port.c

	
 ~ Name:
+---------------------+                                                        
|    OAM Utilities    |                                                         
+---------------------+ 
 ~ Description:
    Utility functions for OAM applications.
	
 ~ File Reference:
    cint_utils_oam.c

	
 ~ Name:
+--------------------------+                                                        
|    Multicast Utilities   |                                                         
+--------------------------+ 
 ~ Description:
    Utility functions for multicast applications.
	
 ~ File Reference:
    cint_utils_multicast.c


 ~ Name:
+---------------------------------+                                                        
|    Field Processor Utilities    |                                                         
+---------------------------------+ 
 ~ Description:
    Utility functions for field processor applications.
	
 ~ File Reference:
    cint_utils_field.c

 ~ Name:
+------------------------------------------------+                                                        
|    Field Processor Egress TC DP QoS actions    |                                                         
+------------------------------------------------+ 
 ~ Description:
    Field Processor Egress TC DP QoS actions configuration
	
 ~ File Reference:
    cint_field_egr_action_dp_tc_qos.c
	
 ~ Name:
+------------------------------------------------------------------+                                                        
|    Field Processor Program Selection Mpls any label and L2/L3    |                                                         
+------------------------------------------------------------------+ 
 ~ Description:
    Field Processor Program Selection Preselector for header format, MPLS any label any L2/L3
	
 ~ File Reference:
    cint_field_mpls_x_presel.c


 ~ Name:
+------------------+                                                        
|    L3 Routing    |                                                         
+------------------+
 ~ Description:
    Example of L3 QoS.
	
 ~ File Reference:
    cint_qos_l3.c


 ~ Name:
+------------+                                                        
|    FCoE    |                                                         
+------------+
 ~ Description:
    Example of FCoE FCF (routing according to FC header).
    - Open router interface
    - Create egress object
    - Add several routes and zoning rules

 ~ File Reference:
    cint_foce_route.c

	
 ~ Name:	
+---------------------+                                                        
|    Termination 1    |                                                         
+---------------------+
 ~ Description:
    Example of MyMac termination, where MyMac is set based on VRID. 
    The cint demonstrates the different VRID-My-MAC-Modes.

 ~ File Reference:
    cint_l3_vrrp.c
    cint_ip_route.c
	
	
 ~ Name:	
+---------------------+                                                        
|    Termination 2    |                                                         
+---------------------+ 
 ~ Description:
    Example of flexiable multiple MyMac termination.
    Available in BCM88660 only.

 ~ File Reference:
    cint_multiple_mymac_term.c


 ~ Name:
+----------------------------------+                                                        
|    Ring Protection Fast Flush    |                                                         
+----------------------------------+
 ~ Description:
    Example of L2 Ring Protection setup and Fast Flush operations. Based on a vswitch setup.
    Includes failover group creation, gport association and removing the association to and from a group and Fast Flush execution.

 ~ File Reference:
    cint_l2_fast_flush.c
    cint_port_tpid.c
    cint_l2_mact.c
    cint_multi_device_utils.c
    cint_vswitch_metro_mp.c

	
 ~ Name:
+------------------+                                                        
|    Port Match    |                                                         
+------------------+
 ~ Description:
    Configure several ingress/egress mappings over the same gport.
    Also demonstrates usage of asymmetric ingress-only and egress-only LIFs.

 ~ File Reference:
    cint_port_match.c


 ~ Name:
+----------------------+                                                        
|    Port Filtering    |                                                         
+----------------------+
 ~ Description:
    Configure egress acceptable frames both for ERPP and ETPP discard.

 ~ File Reference:
    cint_egress_acceptable_frame_type.c

	
 ~ Name:	
+-------------------------+ 
|    Field Processor 1    |
+-------------------------+ 
 ~ Description: 
    Examples related to ACL/PCL functionality. Relevant for PP users.
    
 ~ File Reference:
    cint_fid_demo.c   

	
 ~ Name:	
+-------------------------+ 
|    Field Processor 2    |
+-------------------------+
 ~ Description:
    Add bcmFieldActionStat and bcmFieldActionRedirect for egress ACE table with per-core config params.

 ~ File Reference:        
    cint_field_action_config.c    

	
 ~ Name:	
+-------------------------+ 
|    Field Processor 3    |
+-------------------------+
 ~ Description:
    For MPLS-forwarded packets, in case of MPLS pop, modify the identification of the upper-layer protocol.

 ~ File Reference:
    cint_field_change_upper_layer_protocol_for_pop_mpls.c     

	
 ~ Name:	
+-------------------------+ 
|    Field Processor 4    |
+-------------------------+ 
 ~ Description:
    For IP-routed packets, map the PCP to the DSCP.
    
 ~ File Reference:        
    cint_field_copy_pcp_to_dscp_for_ip_packets.c    

	
 ~ Name:	
+-------------------------+ 
|    Field Processor 5    |
+-------------------------+ 
 ~ Description:
    Perform static forwarding at egress and associate each port to a count.

 ~ File Reference:
    cint_field_egress_modify_tc_per_port.c

	
 ~ Name:	
+-------------------------+ 
|    Field Processor 6    |
+-------------------------+ 
 ~ Description:
    Perform exhaustive test case field configurations.

 ~ File Reference:
    cint_field_exhaustive.c
	
	
 ~ Name:	
+-------------------------+ 
|    Field Processor 7    |
+-------------------------+ 
 ~ Description:
    Correct the learn information in VxLAN and NVGRE application.

 ~ File Reference: 
    cint_field_gre_learn_data_rebuild.c    
    
	
 ~ Name:	
+-------------------------+ 
|    Field Processor 8    |
+-------------------------+ 
 ~ Description:
    Usage of Key-A with single bank database.

 ~ File Reference: 
    cint_field_key_a.c    
    
	
 ~ Name:	
+-------------------------+ 
|    Field Processor 9    |
+-------------------------+ 
 ~ Description:
    Correct the learn information in MAC-in-MAC application. Used in case of Egress-PBB MAC-in-MAC termination.

 ~ File Reference: 
    cint_field_mim_learn_info_set.c    
    
	
 ~ Name:	
+--------------------------+ 
|    Field Processor 10    |
+--------------------------+
 ~ Description:
    Mirror ARP reply packets to local Port 0 (in general CPU).

 ~ File Reference: 
    cint_field_mirror_arp_reply_frame.c    
    
	
 ~ Name:	
+--------------------------+ 
|    Field Processor 11    |
+--------------------------+
 ~ Description:
    Map the forwarding label EXP + RIF.CoS-Profile to TC and DP.

 ~ File Reference:
    cint_field_mpls_lsr_copy_exp_to_tc_dp.c    

	
 ~ Name:	
+--------------------------+ 
|    Field Processor 12    |
+--------------------------+ 
 ~ Description:
    Modify the PCP value according to the destination-MAC after a VLAN translation, for outer-tag packets.
	
 ~ File Reference:
    cint_field_modify_pcp_per_da_after_vlan_translation.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 13    |
+--------------------------+
 ~ Description:
    Redirect traffic according to L2 / L3 header.
	
 ~ File Reference:
    cint_field_policer_redirect_per_l2_l3_addresses.c    
        

 ~ Name:	
+--------------------------+ 
|    Field Processor 14    |
+--------------------------+        
 ~ Description:
    Accept/drop IP-routed packets according to source and destination L4 port ranges.
	
 ~ File Reference:
    cint_field_range_multi_demo.c    
        

 ~ Name:	
+--------------------------+ 
|    Field Processor 15    |
+--------------------------+ 
 ~ Description:
    Accept/drop IP-routed packets according to packet length ranges.
	
 ~ File Reference:
    cint_field_range_pktlen_demo.c    
        

 ~ Name:	
+--------------------------+ 
|    Field Processor 16    |
+--------------------------+ 
 ~ Description:
    Snoop IP-routed packets.
	
 ~ File Reference:
    cint_field_snoop_tc_change_ip_routed_frame.c    


 ~ Name:	
+------------------------------------+ 
|    Field Processor: Presel Stage   |
+------------------------------------+ 
 ~ Description:
    Example of advanced pre-selection with explicit stage.
	
 ~ File Reference:
    cint_field_presel_create_stage_id.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 17    |
+--------------------------+ 
 ~ Description:
    Example of pre-selection use with in/out-port as a qualifier.
	
 ~ File Reference:
    cint_field_presel_in_out_ports.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 18    |
+--------------------------+ 
 ~ Description:
     Example of how to use presel management advanced mode
	 
 ~ File Reference:
    cint_field_presel_mgmt_advanced_mode.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 19    |
+--------------------------+ 
 ~ Description:
    For TM packets, extract the LAG load-balancing key after the ITMH and move the desired number of bytes (set per packet).
	
 ~ File Reference:
    cint_field_tm_copy_lag_lb_key_after_itmh.c    

	
 ~ Name:	
+--------------------------+ 
|    Field Processor 20    |
+--------------------------+ 
 ~ Description:
    For TM packets, extract the scheduler-compensation value after the ITMH, and add padding after ITMH according to the compensation value.
	
 ~ File Reference:
    cint_field_tm_scheduler_compensation.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 21    |
+--------------------------+ 
 ~ Description:
    An example of the use of direct table for cascaded lookup.
	
 ~ File Reference:
    cint_field_mact_llid_limit.c    

	
 ~ Name:	
+--------------------------+ 
|    Field Processor 22    |
+--------------------------+  
 ~ Description:
    Rebuild the learn-data with the current source-system-port and current InLIF 
	
 ~ File Reference:
    cint_field_learn_data_rebuild.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 23    |
+--------------------------+  
 ~ Description:
    Configure ingress field processor to add learn extension to PPH and verify it is being added to the packet in the egress.
	
 ~ File Reference:
    cint_field_learn_extension.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 24    |
+--------------------------+ 
 ~ Description:
    If the forwarding-header-offset is over 32 Bytes, remove up to 14 Bytes after Ethernet header.
	This rule is a walk-around due to the limited capacities of the egress stage to terminate large termination headers.
	
 ~ File Reference:
    cint_field_ingress_large_termination.c    


 ~ Name:	
+--------------------------+ 
|    Field Processor 25    |
+--------------------------+  
 ~ Description:
    According to the packet format unless simple bridged, set the rules to extract the ECN value (2 bits), and set: 
    - If the ECN is supported (ECN-capable)
    - In this case, if the packet is congested (CNI bit)
	
 ~ File Reference:
    cint_field_ecn_cni_extract.c    
    

 ~ Name:	
+--------------------------+ 
|    Field Processor 26    |
+--------------------------+  
 ~ Description:
    According to the packet format, if packet ID IPv4 and ECN is supported, enable DM ECN according to a given threshold.
	
 ~ File Reference:
    cint_field_ecn_dm.c    
    

 ~ Name:	
+--------------------------+ 
|    Field Processor 27    |
+--------------------------+ 	
 ~ Description:
    Create direct extraction field-group with action as counter 0 that returns the value of the InLIF.
    
 ~ File Reference:        
    cint_field_dir_ext_counter_inlif.c


 ~ Name:	
+--------------------------+ 
|    Field Processor 28    |
+--------------------------+     
 ~ Description:
    Create different field groups for fiber channel applications.
    
 ~ File Reference:        
    cint_field_fiber_channel.c


 ~ Name:	
+--------------------------+ 
|    Field Processor 29    |
+--------------------------+  
 ~ Description:
    Create field groups for IP multicast bidirectional applications.
    
 ~ File Reference:        
    cint_field_ipmc_bidir.c


 ~ Name:	
+--------------------------+ 
|    Field Processor 30    |
+--------------------------+ 
 ~ Description:
    Create field groups for VPLS over GRE work-around.
    
 ~ File Reference:        
    cint_field_vpls_gre_wa.c


~ Name:	
+--------------------------+ 
|    Field Processor 31    |
+--------------------------+ 
 ~ Description:
    Create field entries to copy I-SID to system headers when working with 32K I-SIDs.
    
 ~ File Reference:        
    cint_field_mim_32k_isids.c


~ Name:	
+--------------------------+ 
|    Field Processor 32    |
+--------------------------+ 
 ~ Description:
    Create an ELK ACL based on 5-tuples ACL keys for all IPv4 unicast packets and use the external lookup results as Meter-ID if matched.
    
 ~ File Reference:        
    cint_field_elk_acl_5_tuples_ip4_uc.c


~ Name:	
+--------------------------+ 
|    Field Processor 33    |
+--------------------------+ 
 ~ Description:
    Create field group for VxLAN if there is no hit during SIP or VNI mapping lookups.
    
 ~ File Reference:        
    cint_field_vxlan_lookup_unfound.c


~ Name:
+--------------------------------------+
|    Field Processor Data Qualifier    |
+--------------------------------------+
 ~ Description:
    Create a data qualifier using base bcmFieldQualifyEthernetData

 ~ File Reference:
    cint_field_data_qualifier.c


~ Name:	
+--------------------------+ 
|    Field Processor 34    |
+--------------------------+ 
 ~ Description:
    Create a field group that drops all packets with static SA, but without a match for destination (transplant).
    
 ~ File Reference:        
    cint_field_drop_static_sa_transplant.c


~ Name:	
+--------------------------+ 
|    Field Processor 35    |
+--------------------------+  
 ~ Description:
    Create two field groups, using a flag indicating these databases should be compared with each other. 
    Use compare result in the second field group action (Direct Extraction).
        
 ~ File Reference:        
    cint_field_dir_ext_compare_result.c.c


~ Name:	
+--------------------------+ 
|    Field Processor 36    |
+--------------------------+ 
 ~ Description:
    Create 5-tuples field groups that classify frame with 5-tuples parameters (SIP,DIP, IP protocol, L4 Src and DST ports) to InLIF.
        
 ~ File Reference:        
    cint_field_flexible_qinq_example.c


~ Name:	
+--------------------------+ 
|    Field Processor 37    |
+--------------------------+  
 ~ Description:
    Explain the sequence to enable IPv6 extension header parsing and demonstrate the use in field processor.

 ~ File Reference:
    cint_field_ipv6_extension_header.c
 ~ Name:	
+--------------------------+ 
|    Field Processor 38    |
+--------------------------+ 
 ~ Description:
    Use SoC property pmf_vsi_profile_full_range enabling PMF to use 4 bits of VSI profile (L2CP traps diUse SoC property pmf_vsi_profile_full_range enabling PMF to use 4 bits of VSI profile (L2CP traps disabled).

 ~ File Reference:
    cint_field_qual_vsi_profile_full_range.c

	
 ~ Name:	
+--------------------------+ 
|    Field Processor 39    |
+--------------------------+ 
 ~ Description:
	Create an FG and in_rif_profile=1. Action drop for matching packet. 
	
 ~ File Reference:
	cint_field_qualify_IncomingIpIfClass.c


 ~ Name:	
+--------------------------+ 
|    Field Processor 40    |
+--------------------------+        
 ~ Description:
    Accept/drop L2 packets according to OUTLIF (=vport) ranges on Ingress (pre selector).
	
 ~ File Reference:
    cint_field_range_vport_demo.c

 ~ Name:
+-----------------------------+
|    PMF KAPS direct table    |
+-----------------------------+
 ~ Description:
	Create a direct table FG to locate in KAPS.

 ~ File Reference:
	cint_field_kaps.c


 ~ Name:
+----------------------------------------+
|    Field remove directExtraction Entry |
+----------------------------------------+
 ~ Description:
	Example of a sequence of install-remove-reinstall direct extraction entry in Field Processor

 ~ File Reference:
	cint_field_remove_directExtractEntry.c		

 ~ Name:	
+--------------------------+ 
|    Field Small Banks     |
+--------------------------+ 
 ~ Description:
    Create a field group in the TCAM small banks with a key limit of 320b.

 ~ File Reference:
    cint_field_small_banks.c    

 ~ Name:	
+--------------------------+ 
|    VLAN Translation 1    |       
+--------------------------+ 
 ~ Description:
    Set the default VLAN action, and the VLAN action after ingress and egress lookup. Relevant for PP users.       
    
 ~ File Reference:
    cint_fap_vlan_translation.c  
 
 
 ~ Name:	
+--------------------------+ 
|    VLAN Translation 2    |       
+--------------------------+
 ~ Description:
    Examples of using VLAN translation actions in the translation new mode (VLAN translation new mode is set when SoC property bcm886xx_vlan_translate_mode is enabled).
	Relevant for PP users.
    1. Transparent TPID in ingress/egress vlan editing
    2. Default translation actions in ingress/egress  
    
 ~ File Reference:
    cint_advanced_vlan_translation_mode.c                   

	
 ~ Name:	
+--------------------------+ 
|    VLAN Translation 3    |       
+--------------------------+		
 ~ Description: 
   Examples of using advanced VLAN translation with trill.

 ~ File Reference:
    cint_trill.c
    cint_trill_advanced_vlan_translation_mode.c

	
 ~ Name:	
+--------------------------+ 
|    VLAN Translation 4    |       
+--------------------------+
 ~ Description: 
   Examples of setting a port default VLAN translation to advanced mode.

 ~ File Reference:
  src/examples/dpp/cint_vlan_edit_port_default_lif.c
	
 
 ~ Name: 
+-----------------------+ 
|    MACT Learning 1    |
+-----------------------+ 
 ~ Description: 
    Example of different learning modes and message types.      
    Relevant for PP users.                                      

 ~ File Reference: 
    cint_l2_learning.c     
    cint_l2_learning_multi_device.c
        - Right now used only to support cint_vswitch_metro_mp.c. Not yet ready to be used on its own.

		
 ~ Name: 
+-----------------------+ 
|    MACT Learning 2    |
+-----------------------+ 		
 ~ Description:
    An example of learning. Demonstrate how to control the learning message per vlan, send CPU or to OLP (for automatic learning)

 ~ File Reference:
    cint_l2_vlan_learning.c


 ~ Name:	
+---------------------------+
|    Learn Event Parsing    |
+---------------------------+
 ~ Description:
    - Manually construct DSP packets according to predefined parameters
    - Transmit packet and save LEM Payload
    - Read entry from LEM, add via BCM API
    - Save LEM Payload and compare to previous entry

 ~ File Reference:
    cint_l2_learn_event_parsing.c

	
 ~ Name:
+----------------------------+ 
|    MACT Bulk Operations    |
+----------------------------+ 
 ~ Description: 
    - Dump/traverse MACT table according to group (user defined value)
    - Perform several rules in one hardware traverse
    - Traverse rule includes masking on VLAN, gport, etc.
	
 ~ File Reference: 
    cint_mact_bulk_operations.c     

	
 ~ Name:
+------------------+                                                        
|    CPU Shadow    |                                                         
+------------------+
 ~ Description:
    An example of sending MACT events to CPU.

 ~ File Reference:
    cint_l2_cpu_learning.c

	
 ~ Name:
+---------------+                                                        
|    SA Drop    |                                                         
+---------------+ 
 ~ Description:
    An example of dropping Packets according to SA.

 ~ File Reference:
    cint_l2_mact_sa_discard.c
   

 ~ Name:   
+----------------+
|    Warmboot    | 
+----------------+
 ~ Description:
    Save and restore SW databases.
    
 ~ File Reference:
    cint_warmboot_utilities.c

 ~ Name:
+-------------+
|    Trill    |
+-------------+
 ~ Description:
    Trill UC/MC scenarios.
    
 ~ File Reference:
    - Petra-B trill unicast: cint_trill_uni_petrab.c
    - Petra-B trill ingress multicast: cint_trill_mc_ing_petrab.c
    - Petra-B trill transit multicast: cint_trill_mc_transit_petrab.c
    - Petra-B trill egress multicast: cint_trill_mc_egr_petrab.c
    - ARAD (UC+MC in the same CINT): cint_trill.c
    - ARAD PP example for trill to pass (In transit, snoop a copy to recycle port): cint_pmf_trill_2pass_snoop.c
    - ARAD+ trill multi homing: cint_trill_multi_homing.c
    - ARAD (UC+MC transparent service in the same CINT): cint_trill_tts.c


 ~ Name:
+--------------------------------+
|    Internal Ports (OLP/RCY)    |
+--------------------------------+
 ~ Description:
    Examples for retrieving and configuring internal ports (OLP / Recycling).
    
 ~ File Reference:
    cint_internal_ports.c


 ~ Name:
+----------------+
|    Mirror 1    |
+----------------+
 ~ Description:
    Examples of Mirror tests and usage.

 ~ File Reference:
    cint_mirror_tests.c
	

 ~ Name:
+----------------+
|    Mirror 2    |
+----------------+
 ~ Description:
    An example of mirror packet according to port x VLAN, with inbound/outbound mirroring. where the mirror packet:
    - Is SPAN mirrored
    - Includes RSPAN encapsulation
    - Includes ERPSAN encapsulation

 ~ File Reference:
    cint_mirror_erspan.c


 ~ Name:
+----------------+
|    Mirror 3    |
+----------------+
 ~ Description:
    An example of mirror packet according to port x VLAN, with inbound/outbound mirroring. The example is only used in Petra-B.

 ~ File Reference:
    cint_petra_mirror_tests.c


 ~ Name:
+----------------+
|    Mirror 4    |
+----------------+
 ~ Description:
    An example of mirror packet according to port/port x VLAN, with inbound/outbound mirroring. 
 
~ File Reference:
    cint_old_mirror_tests.c

	
 ~ Name:
+--------------------+
|    Trap\Snoop 1    |
+--------------------+
 ~ Description:
    Example of setting a trap type code and a snoop command.

 ~ File Reference:
    cint_rx_trap_fap.c


 ~ Name:
+--------------------+
|    Trap\Snoop 2    |
+--------------------+ 
 ~ Description:
    Example of setting an egress trap type unknown DA / unknown MC / BC

 ~ File Reference:
    cint_port_eg_filter_example.c 


 ~ Name:
+--------------------+
|    Trap\Snoop 3    |
+--------------------+ 
 ~ Description:
    General trap utilities

 ~ File Reference:
    cint_utils_rx.c 

	
 ~ Name:
+--------------------+
|    Trap\Snoop 4    |
+--------------------+
 ~ Description:
	Example of configuring MTU filter per LIF in egress.

 ~ File Reference:
	cint_etpp_lif_mtu.c
	
	
 ~ Name:
+--------------------+
|    Trap/Snoop 5    |
+--------------------+
 ~ Description:
	Configure destination per core for ingress traps.

 ~ File Reference:
	cint_config_dest_per_core.c

 ~ Name:
+--------------------+
|    Trap\Snoop 6    |
+--------------------+
 ~ Description:
	Example of configuring MTU filter trap in egress, can be mirrored/forward/dropped

 ~ File Reference:
	cint_etpp_mtu.c	
	
 ~ Name:
+--------------------+
|    Trap\Snoop 7    |
+--------------------+
 ~ Description:
	Example of configuring MTU filter per RIF in egress.

 ~ File Reference:
	cint_etpp_rif_mtu.c	

	
	
 ~ Name:
+--------------+                                                        
|    Switch    |                                                         
+--------------+  
 ~ Description:
    General switch utilities.

 ~ File Reference:
    cint_utils_switch.c 

	
 ~ Name:
+----------------+                                                        
|    Stacking    |                                                         
+----------------+                                                         
 ~ Description:  
    A setup that configures a stacking state.
    Example: in cint_stk_mc_dyn_3tmd.c - the application should run on 3 connected Arads. 
    A stacking system is created where each Arad is a TM domain, with a multicast group that passes through this system.   
                  
 ~ File Reference:
    cint_stk_uc.c 
    cint_stk_uc_dyn.c
    cint_stk_uc_dyn_3tmd.c  
    cint_stk_mc.c


 ~ Section:
+-----------------+                                                        
|    Tunneling    |                                                         
+-----------------+                                                         
 ~ Description: 
 
  ~ File Reference:
  

 ~ Name:
+--------------------------+                                                        
|    IPV4 Tunneling        |                                                         
+--------------------------+
 ~ Description:
    An example of IPv4  packet forwarding into IPv4 tunnel with/without GRE encapsulation.
    
 ~ File Reference:
    cint_ip_tunnel.c

	
 ~ Name:
+-------------------------------+                                                        
|    IPV4 Tunnel Termination    |                                                         
+-------------------------------+
 ~ Description:
    An example of IPv4 tunnel termination and forwarding according to internal IPv4 header. Demonstrates how to:
    - Create tunnel terminator
    - Associate packet to VRF according to terminated tunnel.
    - Route according to VRF

 ~ File Reference:
    cint_ip_tunnel_term.c

	
 ~ Name:
+----------------------+                                                        
|    IPv6 Tunneling    |                                                         
+----------------------+
 ~ Description:
    An example of IPv4 packet forwarding into an IPv6 tunnel.

 ~ File Reference:
    cint_ipv6_tunnel.c


 ~ Name:
+-------------------------------+                                                        
|    IPV6 Tunnel Termination    |                                                         
+-------------------------------+
 ~ Description:
    An example of IPv6 tunnel termination and forwarding according to internal IPv4 header.

 ~ File Reference:
    cint_ipv6_tunnel_term.c

	
 ~ Name:
+-------------+                                                        
|    L2Gre    |                                                         
+-------------+                                                         
 ~ Description:
    An example of an L2Gre application. 

 ~ File Reference:
    cint_l2gre.c

	
 ~ Name:
+----------------+                                                        
|    VPLSoGRE    |                                                         
+----------------+
 ~ Description:
    An example of VPLSoGRE using recycle.

 ~ File Reference:
    cint_vswitch_vpls_gre.c

	
 ~ Name:
+-------------+                                                        
|    VxLAN    |                                                         
+-------------+                                                         
 ~ Description:
    An example of VXLAN application.

 ~ File Reference:
    cint_vxlan.c
	cint_utils_vxlan.c


 ~ Name:
+-------------------------+                                                        
|    SA Authentication    |                                                         
+-------------------------+
 ~ Description:
    An example of an SA authentication application.

 ~ File Reference:
    cint_l2_auth.c


 ~ Section:
+------------------+                                                        
|    Mac-in-Mac    |                                                         
+------------------+                                                         
 ~ Description:
 
 ~ File Reference:
 

 ~ Name:
+----------+                                                        
|    MP    |                                                         
+----------+
 ~ Description:
    An example of Mac-in-Mac MP. Demonstrates the following:
    - Creating B-VID and VSI
    - Creating a Mac-in-Mac (backbone) port
    - Setting B-tag TPID
    - Creating a customer (access) port
    - Differential flooding for packets arriving from Mac-in-Mac (backbone) ports and from customer (access) ports
    - Bridging between 2 Mac-in-Mac (backbone) ports
    - Setting MyB-Mac for termination and encapsulation
    - Setting drop-and-continue to pass functionality for I-SID broadcast
    - setting IPv4/6 UC ROO MIM
    - setting IPv4 MC ROO MIM

 ~ File Reference:
    cint_mim_mp.c
    cint_pmf_2pass_snoop.c
    src/examples/dpp/cint_mim_mp_roo.c
    

 ~ Name:
+-------------+                                                        
|    P2P 1    |                                                         
+-------------+
 ~ Description:
    An example of Mac-in-Mac P2P. Demonstrates the following:
    - Creating B-VID
    - Creating a Mac-in-Mac (backbone) port
    - Setting B-tag TPID
    - Creating a customer (access) port
    - Cross-connecting a Mac-in-Mac (backbone) port and a customer (access) port

 ~ File Reference:
    src/examples/dpp/cint_mim_p2p.c
    src/examples/dpp/cint_mim_mp.c


 ~ Name:
+-------------+                                                        
|    P2P 2    |                                                         
+-------------+                                                         
 ~ Description:
    An example of P2P. Demonstrates cross-connecting between:
    - 2 VLAN ports 
    - 2 Mac-in-Mac (backbone) ports 
    - MPLS port and VLAN port 
    - MPLS port and Mac-in-Mac (backbone) port 
    - VLAN port and Mac-in-Mac (backbone) port 
    Cross-connections can also be performed with facility/path protection.

 ~ File Reference:
    src/examples/dpp/cint_mpls_lsr.c 
    src/examples/dpp/cint_vswitch_cross_connect_p2p.c 
    src/examples/dpp/cint_vswitch_cross_connect_p2p_multi_device.c  
     - can be used to cross connect two or more devices 
    src/examples/dpp/cint_vswitch_cross_connect_p2p_directional.c 
     - used for directional cross connect PWE to AC 


 ~ Section:		
+-----------+                                                        
|    QoS    |                                                        
+-----------+                                                         
 ~ Description:
 
 ~ File Reference:
 

 ~ Name:
+--------------+                                                        
|    L3 QoS    |                                                         
+--------------+
 ~ Description:
    An example of L3 quality of service. Demonstrates the following: 
    - Creating ingress router interface, associated with in-RIF QoS-profile-ID 
    - Ingress packet DSCP mapping to in-DSCP 
    - In-DSCP mapping to TC+DP 
    - Creating egress router interface, associated with out-RIF QoS-profile-ID 
    - Egress in-DSCP+DP mapping to out-DSCP 

 ~ File Reference:
    src/examples/dpp/cint_qos_l3_rif_cos.c 
    src/examples/dpp/cint_ip_route.c 


 ~ Section:
+---------------+                                                        
|    Policer    |                                                         
+---------------+                                                         
 ~ Description:
 
 ~ File Reference:
  

 ~ Name:
+----------------+                                                        
|    Metering    |                                                         
+----------------+
 ~ Description:
    An example of the different metering modes. Demonstrates the following: 
    - Creating several meter entries, in different modes 
    - Getting each entry, to see the rates and bursts that were set in HW 
    - Mapping packets to meter and testing the configured rate with traffic 
    - Testing packets arriving with green or yellow color 
    - Mapping two meters per-packet 
    - Destroying the meters 

 ~ File Reference:
    src/examples/dpp/cint_policer_metering_example.c 

	
 ~ Name:
+---------------------------+                                                        
|    QoS Policer Mapping    |                                                         
+---------------------------+
 ~ Description:
    An example of mapping drop procedure to PCP-DEI. Demonstrates the following:
    - Creating QoS map, mapping the colors green and yellow to PCP-DEI of outer tag or inner tag
    - Mapping packets to meters and test packets color
    - Testing outer tag or inner tag PCP-DEI of packets colored green or yellow
    - Destroying QoS map

 ~ File Reference:
    src/examples/dpp/cint_qos_policer.c
	
	
 ~ Section:
+------------+                                                        
|    Rate    |                                                         
+------------+                                                         
 ~ Description:
 
 ~ File Reference:
  

 ~ Name:
+------------------------+                                                        
|    Ethernet Policer    |                                                         
+------------------------+
 ~ Description:
    An example of configuring Ethernet policer. Demonstrates the following:
    - Configuring different rates and burst sizes for each traffic type (UC, unknown UC, MC, unknown MC and BC).
    - Getting each entry, to see the rates and bursts that were actually set in HW 
    - Testing the configured rates with traffic 
    - Destroying the Ethernet policer configuration 

 ~ File Reference:
    src/examples/dpp/cint_rate_policer_example.c 
    

 ~ Name:
+----------------------+                                                        
|    Counter Engine    |                                                         
+----------------------+                                                        
 ~ Description:
    An example of counting all the ingress/egress VOQ counters. Demonstrates the following: 
    - Setting the counter engines to count in high resolution per VOQ 
    - Getting all the possible counters (according to the Color and Drop / Forward) per VOQ 
    - Mapping a LIF stack range 

 ~ File Reference:
    src/examples/dpp/cint_voq_count.c 
    src/examples/dpp/cint_ctrp_egress_received_tm.c 
    src/examples/dpp/cint_counter_processor.c 


 ~ Name:
+------------------------+                                                        
|    Flexible Hashing    |
+------------------------+ 
 ~ Description:
	An example of flexible hashing. 
	Flexible hashing utilizes the key construction of stateful load balancing without learning or looking up the keys. 
	Once a key has been constructed in the SLB, it may be copied to the 80 msbs of one (or more) of the PMF keys in the 2nd cycle. 
   
 ~ File Reference:
	src/examples/dpp/cint_flexible_hashing.c 
	
	
 ~ Section:
+------------+                                                        
|    ECMP    |                                                         
+------------+                                                         
 ~ Description:
 
 ~ File Reference:
  

 ~ Name:
+--------------------+                                                        
|    ECMP Hashing    |                                                         
+--------------------+
 ~ Description:
    An example of creating an ECMP, mapping IPv4 host and MPLS LSR to this ECMP and showing how ECMP hashing is done according to different parts of the header. 
	Demonstrates the following:
    - Creating an ECMP containing several FEC entries, each pointing to the same encapsulation with a different out-port 
    - Adding an IPv4 host and mapping it to this ECMP 
    - Adding an MPLS LSR entry and mapping the label to this ECMP 
    - Configuring the ECMP hashing: Disabling the hashing; setting the hashing according to a certain part of the header 
    - Testing with traffic and seeing how the hashing configuration affects the traffic distribution 

 ~ File Reference:
    src/examples/dpp/cint_ecmp_hashing.c 
 

 ~ Section: 
+-----------+                                                        
|    LAG    |                                                         
+-----------+                                                         
 ~ Description:
 
 ~ File Reference:
  

 ~ Name:
+-------------------+                                                        
|    LAG Hashing    |                                                         
+-------------------+
 ~ Description:
    An example of creating a trunk (LAG), mapping IPv4 host and MPLS LSR to this LAG and showing how LAG hashing is done according to different parts of the header.
	Demonstrates the following: 
    - Creating a LAG and add several ports to it 
    - Adding an IPv4 host and mapping it to this LAG 
    - Adding an MPLS LSR entry and mapping the label to this LAG 
    - Configuring the LAG hashing: Disabling the hashing;
	  setting the hashing according to a certain part of the header and setting the number of headers to look at the starting header 
    - Testing with traffic and seeing how the hashing configuration affects the traffic distribution 

 ~ File Reference:
    src/examples/dpp/cint_lag_hashing.c     

	
 ~ Section:
+-----------+                                                        
|    PON    |                                                         
+-----------+                                                          
 ~ Description:
 
 ~ File Reference:
  

 ~ Name:
+-----------------------+                                                        
|    PON Application    |                                                         
+-----------------------+
 ~ Description:
    An example of PON application for bringing up three main PON service categories: 1:1 Model, N:1 Model and TLS.
    Demonstrates the following:
    - Setting ports as NNI ports and PON ports 
    - Creating PON LIFs to match Tunnel_ID and VLAN tags 
    - Setting PON LIF ingress VLAN editor to perform upstream VLAN translation
    - Setting PON LIF egress VLAN editor to perform downstream VLAN translation 
    - Creating NNI LIFs to match port VLAN domain and VLAN tags 
    - Adding PON LIFs and NNI LIFs to vswitch or cross connecting two of them as a 1:1 service 
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c 
    src/examples/dpp/pon/cint_pon_utils.c 
    src/examples/dpp/pon/cint_pon_application.c 

	
 ~ Name:
+------------------------------------------------+                                                        
|    PON TUNNEL_ID to Channel Profile Mapping    |                                                         
+------------------------------------------------+
 ~ Description:
    An example of how to map tunnel_ID to PP port of PON application and set different properties (VTT lookup mode and VLAN range) per LLID profile. 
    Demonstrates the following:
    - Setting ports as NNI ports and PON ports 
    - Creating PON PP ports with different PON tunnel profile IDs 
    - Mapping different tunnel IDs to PON PP ports 
    - Enabling different port tunnel lookups in PON PP ports 
    - Adding different VLAN range info to PON PP ports 
    - Creating PON LIFs to match Tunnel_ID and VLAN tags 
    - Setting PON LIF ingress VLAN editor to perform upstream VLAN translation 
    - Setting PON LIF egress VLAN editor to perform downstream VLAN translation 
    - Creating NNI LIFs to match port VLAN domain and VLAN tags 
    - Setting NNI LIF ingress VLAN editor to replace ctag or both stag and ctag 
    - Adding PON LIFs and NNI LIFs to vswitch or cross connecting two of them as a 1:1 service 
	
 ~ File Reference: 
    src/examples/dpp/cint_port_tpid.c 
    src/examples/dpp/pon/cint_pon_utils.c    
    src/examples/dpp/pon/cint_pon_tunnel_mapping.c 
   

 ~ Name: 
+----------------------+                                                        
|    PON VLAN Range    |                                                         
+----------------------+ 
 ~ Description:
    An example of how to match incoming VLAN using VLAN range of PON application.
    Demonstrates the following:
    - Setting ports as NNI ports and PON ports 
    - Adding different VLAN range info to PON ports 
    - Creating PON LIFs to match Tunnel_ID and VLAN tags 
    - Setting PON LIF ingress VLAN editor to perform upstream VLAN translation 
    - Setting PON LIF egress VLAN editor to perform downstream VLAN translation 
    - Creating NNI LIFs to match port VLAN domain and VLAN tags 
    - Setting NNI LIF ingress VLAN editor to replace ctag or both stag and ctag 
    - Adding PON LIFs and NNI LIFs to vswitch or cross connecting two of them as a 1:1 service 
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c 
    src/examples/dpp/pon/cint_pon_utils.c    
    src/examples/dpp/pon/cint_pon_vlan_range.c 
	
	
 ~ Name:
+-------------------------------------+                                                        
|    PON Tunnel Profile Properties    |                                                         
+-------------------------------------+
 ~ Description:
    An example of how to map tunnel_ID to PP port of PON application and set different properties (VTT lookup mode, VLAN range and frame acceptable mode) per LLID profile.
    Demonstrates the following:
    - Setting ports as NNI ports and PON ports 
    - Creating PON PP ports with different PON tunnel profile IDs 
    - Mapping different tunnel IDs to PON PP ports 
    - Enabling different port tunnel lookup in PON PP ports 
    - Adding different VLAN range info to PON PP ports 
    - Setting different frame acceptable mode to PON PP ports 
    - Creating PON LIFs to match Tunnel_ID and VLAN tags 
    - Setting PON LIF ingress VLAN editor to perform upstream VLAN translation 
    - Setting PON LIF egress VLAN editor to perform downstream VLAN translation 
    - Creating NNI LIFs to match port VLAN domain and VLAN tags 
    - Setting NNI LIF ingress VLAN editor to replace ctag or both stag and ctag 
    - Adding PON LIFs and NNI LIFs to vswitch or cross connecting two of them as a 1:1 service 
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c 
    src/examples/dpp/pon/cint_pon_utils.c   
    src/examples/dpp/pon/cint_pon_tunnel_profile.c 

	
 ~ Name:
+----------------+                                                        
|    PON VMAC    |                                                         
+----------------+
 ~ Description:
    An example of how to use BCM APIs to setup VMAC function.  
    Demonstrate the following: 
    - Setting ports as NNI ports and PON ports 
    - Enabling VMAC and setting VMAC encoding value and mask 
    - Setting field_class_id_size 
    - Disabling ARP (next hop mac extension) feature 
    - Setting a PP model for PON application 
    - Enabling VMAC per AC 
    - Updating user-headers by PMF 
	
 ~ File Reference:
    src/examples/dpp/pon/cint_pon_vmac.c    
    src/examples/dpp/pon/cint_pon_field_vmac.c 

	
 ~ Name:
+------------------------+                                                        
|    PON up Rate Limit   |                                                         
+------------------------+
 ~ Description:
    An example of two level rate limits based on upstream.
    First level rate limit is performed per outer VLAN within LLID.
    Second level rate limit is performed per LLID.
    Demonstrate the following:
    - Creating the second level policer in group = 1 
    - Creating PMF entries to qualify LLID and outer VLAN 
    - Creating the first level policer in group = 0 
    - Attaching the first level policer to the PMF entry 
    - Attaching the second level policer to the PMF entry 
    - Installing PMF entries 
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c 
    src/examples/dpp/pon/cint_pon_utils.c   
    src/examples/dpp/pon/cint_pon_up_ratelimit.c 

	
 ~ Name:
+--------------------+                                                        
|    PON IPv4 ACL    |                                                         
+--------------------+
 ~ Description:
    Examples of IPv4 classification and ACLs.
    IPv4 classification based on IPv4, DA, IP and IPv4 DA IP + L4 destination port. Classification means mapping traffic to a new queue and new COS.
    ACLs drop packets based on DA, SA, VLAN, IPPROTOCOL, SIP, L4SrcPort, L4DstPort.
    Demonstrates the following:
    - Creating IPv4 classification PMF entries to qualify LLID, DA, SIP and other properties of IPv4 
    - Setting PMF entry actions to modify outer priority and internal priority 
    - Installing PMF entries 
    - Creating ACL PMF entries to qualify LLID, DA, SIP and other properties of IPv4 
    - Setting PMF entry actions to drop 
    - Installing PMF entries 
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c 
    src/examples/dpp/pon/cint_pon_utils.c 
    src/examples/dpp/pon/cint_pon_up_scheduler.c 
    src/examples/dpp/pon/cint_pon_ipv4_acl.c 

	
 ~ Name:
+--------------------+                                                        
|    PON IPv6 ACL    |                                                         
+--------------------+
 ~ Description:
    Examples of IPv6 classification and ACLs.
    IPv6 classification based on IPv6, SIP and DIP. Classification means mapping traffic to a new queue and new COS.
    ACLs drop packets based on DA, SA, VLAN, IPPROTOCOL, SIP, L4SrcPort, L4DstPort.
    Demonstrate the following:
    - Creating IPv6 classification PMF entries to qualify LLID, SIP and DIP of IPv6 
    - Setting PMF entry actions to modify outer priority and internal priority 
    - Installing PMF entries 
    - Creating ACL PMF entries to qualify LLID, DA, SIP and other properties of IPv6 
    - Setting PMF entries action to drop 
    - Installing PMF entries 
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c 
    src/examples/dpp/pon/cint_pon_utils.c 
    src/examples/dpp/pon/cint_pon_up_scheduler.c 
    src/examples/dpp/pon/cint_pon_ipv6_acl.c 
    
	
 ~ Name:
+-----------------------------+                                                        
|    PON Channelized Ports    |                                                         
+-----------------------------+
 ~ Description:
    An example of the PON channelized port application and how to set different properties (VTT lookup mode and VLAN range) per LLID profile based on channelized port. 
    Demonstrate the following:
    - Setting ports as NNI ports and channelized PON ports
    - Creating PON PP ports with different PON tunnel profile IDs
    - Mapping different tunnel IDs to PON PP ports
    - Enabling different port tunnel lookups in PON PP ports
    - Adding different VLAN range info to PON PP ports
    - Setting different frame acceptable mode to PON PP ports
    - Creating PON LIFs to match Tunnel_ID and VLAN tags
    - Setting PON LIF ingress VLAN editor to perform upstream VLAN translation
    - Setting PON LIF egress VLAN editor to perform downstream VLAN translation
    - Creating NNI LIFs to match port VLAN domain and VLAN tags
    - Setting NNI LIF ingress VLAN editor to replace ctag or both stag and ctag
    - Adding PON LIFs and NNI LIFs to vswitch or cross connecting two of them as 1:1 service
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c   
    src/examples/dpp/pon/cint_pon_channelized_port.c

	
 ~ Name:
+----------------------------+                                                        
|    PON Additional TPIDs    |                                                         
+----------------------------+
 ~ Description:
    An example of how to use additional TPID for NNI port of PON application.
    Demonstrate the following:
    - Setting ports as NNI ports and PON ports.
    - Adding additional TPIDs.
    - Creating PON PP ports with different PON tunnel profile IDs
    - Mapping different tunnel IDs to PON PP ports
    - Enabling different port tunnel lookups in PON PP ports
    - Adding different TPIDs to PON PP ports
    - Creating PON LIFs to match Tunnel_ID and VLAN tags
    - Setting PON LIF ingress VLAN editor to perform upstream VLAN translation
    - Setting PON LIF egress VLAN editor to perform downstream VLAN translation
    - Creating NNI LIFs to match port VLAN domain and VLAN tags
    - Setting NNI LIF ingress VLAN editor to replace ctag or both stag and ctag
    - Adding PON LIFs and NNI LIFs to vswitch or cross connecting two of them as 1:1 service
	
 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c   
    src/examples/dpp/pon/cint_pon_additional_tpids.c

	
 ~ Name:
+---------------------------+                                                        
|    PON Routing Over AC    |                                                         
+---------------------------+
 ~ Description:
    An example of how to perform setup route over AC function in PON application downstream.
    Demonstrate the following:
    - Setting ports as NNI ports and PON ports
    - Setting normal L3 application
    - Creating PON LIF and NNI LIF
    - Adding a route in L3 host table
    - Setting PON LIF egress VLAN editor to add tunnel ID and outer VID
    - Setting PMF rules to extract ARP to OutLIF
	
 ~ File Reference:
    src/examples/dpp/utility/cint_utils_global.c
    src/examples/dpp/utility/cint_utils_l2.c
    src/examples/dpp/utility/cint_utils_l3.c
    src/examples/dpp/utility/cint_utils_vlan.c
    src/examples/dpp/utility/cint_utils_port.c
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_route_over_ac.c
    src/examples/dpp/pon/cint_pon_field_router_over_ac.c

	
 ~ Name:
+-----------------+                                                        
|    CMIC RCPU    |                                                         
+-----------------+                                                         
 ~ Description:
	An example of using RCPU. 
     - An example of system configuration
     - Packet flow
     - Packet format
	 
 ~ File Reference:
    src/examples/dpp/cint_cmic_rcpu.c
    
	
 ~ Name:
+--------------+                                                        
|    CPU RX    |                                                         
+--------------+                                                         
 ~ Description:
	An example of CPU RX packets receive configuration. 
     - With and without COS use
     - Starting RX
     - Configuring different RX ports
     - Configuring COS to different RX ports
     - configuring appropriate callbacks to different RX ports
	 
 ~ File Reference:
    src/examples/dpp/cint_rx_cos.c
    src/examples/dpp/cint_rx_no_cos.c
	
	
 ~ Name:
+---------------+                                                        
|    Trunk 1    |                                                         
+---------------+                                                         
 ~ Description:
	Example of trunk manipulation.
     - Creating trunk
     - Adding and removing ports to or from trunk
     - Removing all ports from trunk
     - Removing a trunk
	 
 ~ File Reference:
    src/examples/dpp/cint_trunk.c


 ~ Name:
+---------------+                                                        
|    Trunk 2    |                                                         
+---------------+  
 ~ Description:
    This cint emulates the trunk (LAG) member and FEC resolution process performed by the BCM88650 (Arad) device.

 ~ File Reference:
    cint_trunk_ecmp_lb_key_and_member_retrieve.c

	
 ~ Name:
+------------------------------+
|    ARAD Scheduling Scheme    |
+------------------------------+
 ~ Description:
    Building a BCM88650 (Arad) scheduling scheme in 8 priorities mode.

 ~ File Reference:
    cint_tm_fap_multi_devices_8_priorities_example.c

	
 ~ Name:
+-------------------------------------+
|    Congestion Notification (CNM)    |
+-------------------------------------+
 ~ Description:
	Configuration of the CNM mechanism.
	 - Enabling/Disabling the CNM mechanism
	 - Setting the managed queues range
	 - Enabling/Disabling a managed queue
	 - Configuring the congestion test parameters
	 - Configuring the transport attributes of the CNM packets
			 
 ~ File Reference:
	cint_cnm.c


 ~ Name:
+--------------------------+
|    Multi Device Cints    |
+--------------------------+
 ~ Description:
	Those cints are the same as the regular cints in terms of device configurations, but the configurations are done on multiple devices. 
	There are two differences between calling multi device cints and calling regular cints:
	 * Instead of accepting unit as parameter, the run functions accepts an array of units as a first argument,
	   and the number of units as a second argument. Every unit should appear only once in the array.
	 * Instead of accepting port number as an argument, all multi device cints require a system port number. 
	   To obtain the system port number, you can use port_to_system_port in cint_multi_device_utils.c
              
 ~ File Reference:
	cint_multi_device_utils.c
	cint_vswitch_cross_connect_p2p_multi_device.c
	cint_vswitch_metro_mp.c
	cint_mpls_lsr.c
	cint_ip_route.c
	cint_ip_route_tunnel.c
	cint_ip_route_tunnel_remark.c

	
 ~ Name:	
+------------------------------------+ 
|    MAC Limiting per Tunnel ID 1    |    
+------------------------------------+ 
 ~ Description:
    Example of MAC limiting base Tunnel ID.  
     - Set MAC limiting mode by SoC property 
     - Create PON application service
     - Set MAC limiting base tunnel ID      

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_mact_limit.c
	

 ~ Name:	
+------------------------------------+ 
|    MAC Limiting per Tunnel ID 1    |    
+------------------------------------+ 
 ~ Description:
    An example of the use of direct table for cascaded lookup in order to drop packets when MAC limiting is reached.
	
 ~ File Reference:
    src/examples/dpp/pon/cint_pon_field_mact_llid_limit.c

	
 ~ Name:
+-----------------------+ 
|    Port VLAN Range    |    
+-----------------------+ 
 ~ Description:
    An example of how to match incoming VLAN using VLAN range of BCM88X4X (Petra) and BCM88650 (Arad).
    Demonstrates the following:
    - Adding a different VLAN range info to ports
    - Creating LIFs
    - Cross connect the 2 LIFs
	
~ File Reference:
    src/examples/dpp/cint_port_tpid.c    
    src/examples/dpp/cint_port_vlan_range.c     


 ~ Name:
+------------------------------+ 
|    Anti Spoofing Function    |  
+------------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement the anti-spoofing (L3 Source Binding) function in PON application service.    
     - Setting a PON application configuration and L3 source bind mode by SoC property 
     - Enabling a PON application service
     - Setting L3 source binding configuration
     - Enabling FP to drop unmatched traffic     

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_anti_spoofing.c
	

 ~ Name:	
+--------------------------------------+ 
|    General Anti Spoofing Function    |  
+--------------------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement general anti-spoofing (host & subnet anti-spoofing) function in a PON application service.
     - Setting a PON application configuration and L3 source bind mode by SoC property 
     - Enabling a PON application service
     - Setting L3 source binding configuration
     - Enabling FP to drop unmatched traffic     

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_general_anti_spoofing.c

	
 ~ Name:
+------------------------------+ 
|    L2 Management Function    |   
+------------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement L2 management function in a PON application service.     
     - Setting a PON application configuration 
     - Implementing L2 management function
       Adding a L2 entry
       Getting a L2 entry
       Deleting a L2 entry
       Deleting L2 entries by port
       Deleting dynamic L2 entries by port
       Deleting static L2 entries by port
       Getting the number of L2 entries by port
       Getting the number dynamic L2 entries by port
       Getting the number static L2 entries by port

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_mac_management.c
  

 ~ Name:  
+---------------------------+ 
|    COS Remark Function    |  
+---------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement COS remark function in a PON application service.    
     - Setting a PON application configuration
     - Enabling a PON application service
     - Setting COS remark configuration

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_cos_remark.c

	
 ~ Name:
+--------------------------+ 
|    Multicast function    |  
+--------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement the multicast function in a PON application service.    
     - Setting a PON application configuration
     - Enabling a PON application service
     - Creating a multicast group
     - Joining a port group
     - Leaving a port group

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_multicast.c

	
 ~ Name:
+--------------------------------------+ 
|    Downstream Rate Limit Function    |  
+--------------------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement the downstream rate limit function in a PON application service.    
    - Creating a scheduler
    - Creating a VOQ connector
    - Creating a VOQ
    - Setting a PON application configuration
    - Enabling a PON application service

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_dn_ratelimit.c

	
 ~ Name:
+-------------------------------------------------+
|    Downstream Rate Limit and PFC RX Function    |
+-------------------------------------------------+
 ~ Description:
    An example of how to use BCM APIs to implement downstream rate limit function and configure PFC RX processing for 10G and 1G channels in a PON application service.
    - Creating a scheduler
    - Creating a VOQ connector
    - Creating a VOQ
    - Setting a PON application configuration
    - Enabling a PON application service
    - Setting a PON QoS mapping
    - Enable PFC reception on a PON port

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_dn_ratelimit_dml.c
    src/examples/dpp/pon/cint_pon_pfc_inbnd_receive.c
    src/examples/dpp/pon/cint_pon_cos_remark_llid_dml.c

	
 ~ Name:
+-----------------------------------+ 
|    Upstream Scheduler Function    |  
+-----------------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement upstream scheduler function in a PON application service.     
    - Creating a SP/WRR/SP+WRR scheduler
    - Creating a VOQ connector
    - Creating a VOQ
    - Setting a PON application configuration
    - Enabling a PON application service

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_up_scheduler.c
    

 ~ Name:
+------------------------------+ 
|    CPU RX and TX Function    |  
+------------------------------+ 
 ~ Description:
    An example of how to use BCM APIs to implement CPU RX and TX functions in PON application service.    
    - Setting a PON application configuration
    - Enabling a PON application service
    - RX PPPOE by PMF
    - RX DHCP/IGMP/MLD by switch port control
    - TX packet on port
    - TX packet on lag.

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_cpu_rx_tx.c
    

 ~ Name:
+-------------------------+ 
|    MAC Move per Port    |    
+-------------------------+ 
 ~ Description:
    An example of MAC move base PON or NNI port.  
     - Setting a MAC limiting mode using SoC properties 
     - Creating a PON application service
     - Setting a MAC limiting base tunnel ID 
     - Setting a MAC move base PON or NNI port     

 ~ File Reference:
    src/examples/dpp/cint_port_tpid.c
    src/examples/dpp/pon/cint_pon_utils.c
    src/examples/dpp/pon/cint_pon_mact_move.c

	
 ~ Name:
+------------------------+ 
|    Forward Group AC    |    
+------------------------+ 
 ~ Description:
     An example of using forward group AC and PMF for DPoE IP-HSD application 
     - Setting a PON application configuration
     - Setting a PON DPoE IP-HSD application service
     - Setting FG to update L2 lookup results with service flows in downstream	 

 ~ File Reference:
    src/examples/dpp/pon/cint_pon_dml_fec_app.c

	
 ~ Name:
+--------------------+
|    OAM Ethernet    |
---------------------+
 ~ Description:
	An example of Ethernet OAM. 
	 - Creating groups
	 - Creating local and remote MEPs
	 - Getting and deleting endpoints and groups
	 - Injecting packets
	 - Registrating events
              
 ~ File Reference:
	src/examples/dpp/cint_oam.c


 ~ Name:
+--------------------+
|    OAM Over LAG    |
+--------------------+
 ~ Description:
	An example of creating Ethernet OAM over LAG.
	 - Creating a lag
	 - Creating Ethernet port over the lag
	 - Creating OAM group
	 - Creating MEP
	 - Adding all lag ports to the MEP

 ~ File Reference:
	src/examples/dpp/cint_oam_endpoint_over_lag.c

	
 ~ Name:
+------------------------------------------+
|    OAM Acceleration Advanced Features    |
+------------------------------------------+
 ~ Description:
	An example of Ethernet OAM with additional functionalities available in BCM88660 (Arad+).
	 - Creating groups
	 - Creating local (accelerated) and remote MEPs
	 - Creating additional LM/DM entries per accelerated endpoint
	 - Creating global loopback entry and loopback entries per accelerated endpoint
              
 ~ File Reference:
	src/examples/dpp/cint_oam_acceleration_advanced_features.c

	
 ~ Name:
+------------------------------------------------+
|    OAM Over Ethernet Over PWE over Ethernet    |
+------------------------------------------------+
 ~ Description:
	An example configuration for OAMoEthernetoPWEoEth. Includes P2P and P2MP.
	 - Creating VPLS with default values
	 - Adding vswitch cross connect (if P2P has been selected)
	 - Creating OAM group, OAM endpoint

 ~ File Reference:
	 src/examples/dpp/cint_oam_cfm_o_eth_o_pwe_o_eth.c

	 
 ~ Name:
+---------------------------------------------+
|    OAM Endpoint Connected to VPLS Tunnel    |
+---------------------------------------------+
 ~ Description:
	An example of creating Ethernet OAM endpoint connected to VPLS tunnel.
	 - Creating LSP for cross connected ports
	 - Creating a PW port
	 - Creating an OAM group; OAM endpoint

 ~ File Reference:
	src/examples/dpp/cint_oam_up_mep_over_vpls.c

	
 ~ Name:
+-----------+ 
|    BFD    |
+-----------+
 ~ Description:   
	An example of BFD.        
	 - Creating IPV4 endpoint     
	 - Creating MPLS endpoint    
	 - Creating PWE endpoint       
	 - Destroying endpoints   
	 - Registrating events     
     
 ~ File Reference:
	 src/examples/dpp/cint_bfd.c

	 
 ~ Name:
+----------------+ 
|    BFD IPv6    |
+----------------+ 
 ~ Description:   
	An example of BFD:        
	 - Creating IPv6 endpoint     
	 - Getting IPv6 endpoint    
	 - Destroying endpoints   
	 - Trapping BFD IPv6 PMF
	 - Sending BFD IPv6 traffic     
     
 ~ File Reference:
	src/examples/dpp/cint_bfd_ipv6.c 
	src/examples/dpp/cint_bfd_ipv6_max_sessions.c

	
 ~ Name:
+--------------------+ 
|    BFD Over LAG    |
+--------------------+
 ~ Description:   
	An example of BFD over LAG:        
	 - Creating LAG across multiple devices.
	 - Creating BFD endpoints on server and client sides
     
 ~ File Reference:
	src/examples/dpp/cint_bfd_over_lag.c 


 ~ Name:
+-----------------------------+
|    OAM Y1731 over tunnel    |
+-----------------------------+
 ~ Description:
	An example of Y1731 OAM over MPLS-TP, PWE. 
	 - Configuring MPLS-TP/PWE tunnel
	 - Creating groups
	 - Creating local and remote MEPs
	 - Injecting packets
	 - Registrating events
              
 ~ File Reference:
	src/examples/dpp/cint_oam_y1731_over_tunnel.c


 ~ Name:
+---------------+
|    OAM MIM    |
+---------------+
 ~ Description:
	An example of OAM MIM.
	 - Creating group
	 - Creating local MIM endpoint
              
 ~ File Reference:
	src/examples/dpp/cint_oam_mim.c


 ~ Name:	
+-------------------------------------------------------+
|    Reflector (RFC-2544 - benchmarking methodology)    |
+-------------------------------------------------------+
 ~ Description:
	An example of using reflector u-code functionality.
	Setting up an egress field processor rule that modifies out TM ports for all traffic arriving on a given port.
            
 ~ File Reference:
	src/examples/dpp/cint_benchmarking_methodology.c


 ~ Section:
+-----------+                                                        
|    ECN    |                                                         
+-----------+                                                         
 ~ Description:
 
  ~ File Reference:
  

 ~ Name:
+--------------+                                                        
|    ECN PP    |                                                         
+--------------+
 ~ Description:
    An example of ECN marking during forwarding and encapsulation. Demonstrates the following:
    - Enabling ECN on the device
    - Setting extraction of ECN (capable and congestion bits) 
    - Configuring congestion for every packet that is sent to out_port.
    - Configuring MPLS encapsulation and egress remark profile mapping of {in-DSCP,DP}, to out-DSCP
    - Testing with traffic and seeing how ECN marking appears in forwarding and encapsulation

 ~ File Reference:
    src/examples/dpp/cint_ecn_example.c 

	
 ~ Name:
+------------+
|    1588    |
+------------+
 ~ Description:
	An examples of 1588 (Precision Time Protocol).
	 - Enabling/disabling 1588 one/two step Transparent clock (TC) per port
	 - Trap/drop/snoop/forward control over 1588 message types per port
              
 ~ File Reference:
	 src/examples/dpp/cint_1588.c


 ~ Name:
+------------+
|    Time    |
+------------+
 ~ Description:
	An examples of the BroadSync API. 
	 - BroadSync ToD capture.
	 - Time interface traverse/delete/heartbeat_enable_get 
              
 ~ File Reference:
	 src/examples/dpp/cint_time.c

	 
 ~ Name:
+-----------+
|    EVB    |
+-----------+
 ~ Description:
	Examples of EVB (Edge Virtual Bridging).
	Edge virtual bridge (defined in 802.1Qbg) describes device and protocols connecting between end stations (VMs) and the data centers network. 

 ~ File Reference:
	src/examples/dpp/cint_evb_example.c

	
 ~ Name:
+------------------------+
|    XGS MAC Extender    |
+------------------------+
 ~ Description:
	Examples of XGS MAC extender mappings.
	In this system BCM88650 (Arad) is in device mode PP and connected to several XGS devices usually as 1G Port extender.
	The cint provides an example of setting for BCM88650 (Arad).

 ~ File Reference:
	src/examples/dpp/cint_xgs_mac_extender_mappings.c

	
 ~ Name:
+-------------------+
|    Initial VID    |
+-------------------+
 ~ Description:
	Examples of initial-VID.
	 - By default, BCM API differs between tagged packets and untagged packets.
	 - The cint introduces a per port decision whether to act as default or use initial VID with no difference between untagged packets and tagged packets.
	   Databases used in this case match MATCH_PORT_VLAN and MATCH_PORT_VLAN_VLAN (no use for MATCH_PORT_INITIAL_VLAN).
	   
 ~ File Reference:
	src/examples/dpp/cint_vlan_port_initial_vid.c      
   

 ~ Name:   
+-------------------+
|    Local Route    |
+-------------------+
 ~ Description:
	Examples of local route.
	 - Supporting the local route on PON port means PON upstream traffic can go back to other PONs
	 - Supporting the local route based on VSI
	 - Supporting the local route split-horizon filter
	 
 ~ File Reference:
	src/examples/dpp/pon/cint_pon_local_route.c

	
 ~ Name:
+--------------------------------+
|    System Vswitch Resources    |
+--------------------------------+
 ~ Description:
	An example of system resources in local mode.
	 - All CINTs assume global resources for LIF and L2 FEC
	 - The CINTs provide an example of how to allocate in local mode
	 - Provided examples: VLAN-Port, MPLS-Port
	 
 ~ File Reference:
	 src/examples/dpp/cint_system_vswitch.c
	src/examples/dpp/cint_system_vswitch_vpls.c
 

 ~ Name:
+-------------------------------+
|    Stateful Load balancing    |
+-------------------------------+
 ~ Description:
	An example of stateful load balancing configuration.
		
 ~ File Reference:
	src/examples/dpp/cint_stateful_load_balancing.c
 
 
 ~ Name:
+------------+                                                        
|    IGMP    |                                                         
+------------+                                                         
 ~ Description:
	An example of IGMP application, 
	 - IGMP packet snooping
	 - IGMP multicast 

 ~ File Reference:
	cint_igmp_example.c


 ~ Name:
+-------------------+                                                        
|    GALoPWEoLSP    |                                                         
+-------------------+                                                         
 ~ Description:
	An example of setup for supporting GALoPWEoLSP packets trapping.
	The packets are trapped with MplsUnexpectedNoBos.
	The PWE LIF appears in the PPH header.

 ~ File Reference:
	cint_gal_o_pwe_o_mpls.c
	cint_field_pwe_gal_lif_update.c

	
 ~ Name:
+-----------------------------------+                                                        
|    Compensation  Configuration    |                                                         
+-----------------------------------+                                                         
 ~ Description:
	Configuring compensation example both for ingress and egress.

 ~ File Reference:
	cint_compensation_setting.c
        
		
 ~ Name:
+-------------------+
|    L2 Traverse    |   
+-------------------+
 ~ Description:
	An example of L2 traverse.
    - Traverse all entries in the L2 table using bcm_l2_traverse
    - Traverse matching entries in the L2 table in the L2 table by bcm_l2_matched_traverse
    - Traverse matching entries with flexible mask in the l2 table in the l2 table by bcm_l2_match_masked_traverse.

 ~ File Reference:
    src/examples/dpp/cint_l2_traverse.c


 ~ Name:
+--------------------------+                                                        
|    Routing Over VXLAN    |                                                         
+--------------------------+                                                         
 ~ Description:
    An example of routing over VXLAN UC and MC.

 ~ File Reference:
    cint_vxlan_roo.c
    cint_vxlan_roo_mc.c
    src/examples/dpp/utility/cint_utils_roo.c


 ~ Name:
+------------------------------+
|    VSwitch VPLS with ECMP    |
+------------------------------+
 ~ Description:
	An example of virtual switch that contains VPLS with ECMP.
	
 ~ File Reference:
	src/examples/dpp/cint_vswitch_vpls_ecmp.c

	
 ~ Name:
+-------------------------------------+
|    VSwitch VPLS Routing Over PWE    |
+-------------------------------------+
 ~ Description:
	An example of virtual switch with routing over PWE.
	
 ~ File Reference:
	src/examples/dpp/cint_vswitch_vpls_roo.c
	src/examples/dpp/utility/cint_utils_roo.c

	
 ~ Name:
+--------------------+
|    Dynamic Port    |
+--------------------+    
 ~ Description:
	1. An example of how to add dynamic ports.
	2. An example of adding default ports.
	
 ~ File Reference:
	1. src/examples/dpp/cint_dynamic_port_add_remove.c
	2. src/examples/dpp/cint_dynamic_ports_init_example.c

	
 ~ Name:
+-----------------------+                                                        
|    Routing Over AC    |                                                         
+-----------------------+                                                         
 ~ Description:
    An example of routing over AC.    
     
 ~ File Reference:
    src/examples/dpp/cint_route_over_ac.c 
    src/examples/dpp/utility/cint_utils_global.c
    src/examples/dpp/utility/cint_utils_port.c
    src/examples/dpp/utility/cint_utils_l2.c
    src/examples/dpp/utility/cint_utils_l3.c
    src/examples/dpp/utility/cint_utils_vlan.c
    src/examples/dpp/cint_port_tpid.c

	
 ~ Name:
+-----------------------------+
|    Routing Over Extender    |
+-----------------------------+
 ~ Description:
    An example of routing over extender port.

 ~ File Reference:
    src/examples/dpp/cint_route_over_extender.c
    src/examples/dpp/utility/cint_utils_global.c
    src/examples/dpp/utility/cint_utils_port.c
    src/examples/dpp/utility/cint_utils_l2.c
    src/examples/dpp/utility/cint_utils_l3.c
    src/examples/dpp/utility/cint_utils_extender.c
    src/examples/dpp/utility/cint_utils_vlan.c
    src/examples/dpp/cint_port_tpid.c

	
 ~ Name:
+--------------------+                                                        
|    Mesh System     |                                                         
+--------------------+                                                         
 ~ Description:
    An example of configuration in a mesh system.
     - Setting modid groups
     - Configuring link topology
     - Configuring multicast replication (in Mesh MC) 
     
 ~ File Reference:
    src/examples/dpp/cint_fabric_mesh.c 


 ~ Name:   	  
+---------------------------------------+
|    FAP above ARAD - FE1600 Interop    |
+---------------------------------------+
 ~ Description:
	This example demonstrates the configuration required for a FAP of generation above ARAD: 
	BCM88670/BCM88680/BCM88476 (Jericho, QMX, Jericho plus, Kalia), when connecting it to the legacy device BCM88750 (FE1600).

 ~ File Reference:
	src/examples/dpp/cint_fap_above_arad_fe1600_interop.c

	
 ~ Name:
+---------------------------------------+
|    Port Extender UC Control Bridge    |
+---------------------------------------+
 ~ Description:
    An example of a port extender control bridge configuration for UC traffic.
     - Configuring the device to be a control bridge and prepend attributes
     - Configuring a VSI
     - Attaching cascaded ports to the VSI
     - Configuring port extender LIFs
     - Attaching end stations to the VSI

 ~ File Reference:
    src/examples/dpp/cint_port_extender_cb_uc.c 

	
 ~ Name:
+------------------------------------------+
|    Port Extender L2 MC Control Bridge    |
+------------------------------------------+
 ~ Description:
    An example of a port extender control Bridge configuration for L2 multicast traffic.
    - Configuring the device to be a control bridge and prepend attributes
    - Configuring a VSI and attaching cascaded ports to it
    - Configuring unicast and multicast port extender LIFs
    - Creating L2 multicast groups
    - Adding port extender CUDs (MC OutLif) to the multicast group
    - Adding unicast and multicast MAC entries

 ~ File Reference:
    src/examples/dpp/utility/cint_utils_global.c
    src/examples/dpp/utility/cint_utils_port.c
    src/examples/dpp/utility/cint_utils_l2.c
    src/examples/dpp/utility/cint_utils_vlan.c
    src/examples/dpp/utility/cint_utils_extender.c
    src/examples/dpp/cint_port_extender_cb_l2_mc.c

	
 ~ Name:
+------------------------------------------+
|    Port Extender L3 MC Control Bridge    |
+------------------------------------------+
 ~ Description:
    An example of a port extender control bridge configuration for L3 ,unicast traffic.
    - Configuring the device to be a control bridge and prepend attributes
    - Configuring port extender LIFs
    - Configuring VSI per each port extender and attaching the ports to it
    - Creating L3 multicast Groups
    - Creating L3 interface per each port extender
    - Adding port extender L2 (OutLif) and L3 (OuRif) CUDs to the multicast group
    - Adding routing table entry

 ~ File Reference:
    src/examples/dpp/utility/cint_utils_global.c
    src/examples/dpp/utility/cint_utils_port.c
    src/examples/dpp/utility/cint_utils_l2.c
    src/examples/dpp/utility/cint_utils_vlan.c
    src/examples/dpp/utility/cint_utils_extender.c
    src/examples/dpp/cint_port_extender_cb_l3_mc.c

	
 ~ Name:
+-----------------------------+
|    Port Extender Mapping    |
+-----------------------------+
 ~ Description:
	An example of setting ingress PP port mapping via user define value.
	
 ~ File Reference:
	src/examples/dpp/cint_port_extender_mapping.c

	
 ~ Name:
+---------------------------+
|    Initial QoS Mapping    |
+---------------------------+
 ~ Description:
	Example of the initial QoS application based on port.
	   
 ~ File Reference:
	src/examples/dpp/cint_qos_initial.c

	
 ~ Name:
+----------------+
|    QoS MSPW    |
+----------------+
 ~ Description:
	An example of COS mapping for MS PWE transit node.
	
 ~ File Reference:
	src/examples/dpp/cint_qos_mspw.c
 
 
 ~ Name:
+--------------------------------------------------+
|    Enable TLV Workaround for BCM88660 (Arad+)    |
+--------------------------------------------------+
 ~ Description:
	An example of programming the PMF to enable a workaround for port/interface status TLV in OAM CCM packets received from Remote MEPs.
	
 ~ File Reference:
	src/examples/dpp/cint_field_oam_bfd_advanced.c

	
 ~ Name:
+---------------------------------------+
|    Port Extender Dynamic Switching    |
+---------------------------------------+
 ~ Description:
	An example of the port extender dynamic switching for COE ports.
	
 ~ File Reference:
	src/examples/dpp/cint_port_extender_dynamic_switching.c

	
 ~ Name:
+--------------+
|    L3 VPN    |
+--------------+
 ~ Description:
	An example of L3VPN application.
	In CINT following traffic scenarios: 
	 1. Route unicast traffic.  
	 2. mVPN application (multicast IPMC traffic).  
	 3. For 88670_A0: Demonstrates how to configure hierarchical FEC for L3VPN. 
	 
 ~ File Reference:
	 1. src/examples/dpp/cint_l3vpn.c
	 2. src/examples/dpp/cint_mvpn.c
	 3. src/examples/dpp/cint_l3vpn.c

	 
 ~ Name:
+------------------------------------+
|    BFD IPv4 Single Hop Extended    |
+------------------------------------+
 ~ Description:
	PMF stage in BFD IPv4 single hop extended solution.  
	
 ~ File Reference:
	src/examples/dpp/cint_field_bfd_ipv4_single_hop.c
        
		
 ~ Name:		
----------------------------+
|    BFD IPv6 Single Hop    |
+---------------------------+
 ~ Description:
	PMF stage in BFD IPv6 single hop solution. 
	
 ~ File Reference:
	src/examples/dpp/cint_field_bfd_ipv6_single_hop.c

	
 ~ Name:
-------------------------------+
|    OAM per MEP Statistics    |
+------------------------------+
 ~ Description:
	PMF stage in OAM per MEP statistics solution.  
	
 ~ File Reference:
	src/examples/dpp/cint_field_oam_statistics.c

	
 ~ Name:
----------------------+
|    MTU Filleting    |
+---------------------+
 ~ Description:
	An example of MTU filtering configuration.
	
 ~ File Reference:
	src/examples/dpp/cint_field_mtu_filtering.c
	src/examples/dpp/cint_egr_mtu.c

	
 ~ Name:
+------------+
|    MLDP    |
+------------+
 ~ Description:
	An example of IP-GRE L3VPN which show BCM886XX being PE-based MLDP.
	
 ~ File Reference:
	src/examples/dpp/cint_mldp.c
	
	
 ~ Name:
+---------------------------------+
|       Additional TPIDs          |
+---------------------------------+
 ~ Description:
	An example of setting BCM88670 (Jericho) additional TPIDs to enable additional TPID port lookup.
	
 ~ File Reference:
	src/examples/dpp/cint_additional_tpids.c 


 ~ Name:
+--------------------------+                                                        
|    Routing Over Trill    |                                                         
+--------------------------+                                                         
 ~ Description:
    An example of routing over trill. 

 ~ File Reference:
    cint_trill_roo.c


 ~ Name:
+------------------------------------+                                                        
|    Hierarchical Protection VPLS    |                                                         
+------------------------------------+                                                         
 ~ Description:
    An example of VPLS hierarchical protection. 

 ~ File Reference:
    cint_vswitch_vpls_hvpls.c

	
 ~ Name:
+------------------------------+
|    3level Protection VPLS    |
+------------------------------+
 ~ Description:
    An example of VPLS 3level protection.

 ~ File Reference:
    cint_vswitch_vpls_3level_protection.c


	 ~ Name:
+------------------------------+
|    Port based PWE            |
+------------------------------+
 ~ Description:
    An example of port based PWE. A demo showing how to create and update a port based PWE.

 ~ File Reference:
    cint_vswitch_port_based_pwe.c

 ~ Name:
+------------------------------------+
|  PWE raw/tag mode with native EVE  |
+------------------------------------+

~ Description:
    Example for PWE raw/tag mode with the native EVE feature.

~ File Reference:
    cint_pw_raw_tag_mode.c

	
 ~ Name:
+-----------------------+                                                        
|    MPLS Protection    |                                                        
+-----------------------+                                                         
 ~ Description:
	An example of MPLS protection. 
	
 ~ File Reference:
    cint_mpls_protection.c

	
 ~ Name:
+------------+                                                        
|    MPLS    |                                                        
+------------+                                                         
 ~ Description:
    Examples of various MPLS scenarios. 
	
 ~ File Reference:
    cint_mpls_various_scenarios.c

	
 ~ Name:
+--------------------------------+                                                        
|    E2E Symmetric Connection    |                                                         
+--------------------------------+                                                         
 ~ Description:
    An example of building a symmetric scheduling scheme.

 ~ File Reference:
    cint_e2e_symmetric_connection.c

	
 ~ Name:
+---------------------------------------------------+
|    Two Way Active Measurement Protocol (TWAMP)    |
+---------------------------------------------------+
 ~ Description:
    An example of handling TWAMP.

 ~ File Reference:
    cint_twamp.c

	
 ~ Name:
+------------------------+
|    Auto Negotiation    |
+------------------------+
 ~ Description:
    An example of auto negotiation in BCM88670 (jericho).

 ~ File Reference:
    cint_autoneg.c
 

 ~ Name: 
+-----------------------------------------------------------+
|    L2 Encapsulation of a Packet Punted to External CPU    |
+-----------------------------------------------------------+
 ~ Description:
    An example of L2 encapsulated punt packet configuration.  

 ~ File Reference:
    src/examples/dpp/cint_l2_encap_external_cpu.c

	
 ~ Name:
+--------------------------------+
|    Two Level ECMP with MPLS    |
+--------------------------------+
 ~ Description:
    An example of hierarchical ECMPs, each for selecting MPLS tunnels.

 ~ File Reference:
    cint_mpls_ecmp.c

	
 ~ Name:
+-----------------------------------------------------+
|    RFC2544 Reflector - 1-Pass programmable usage    |
+-----------------------------------------------------+
 ~ Description:
    An example of enabling and programming a reflector, using a user defined criteria that does not use a 2nd cycle through the device for reflection.

 ~ File Reference:
    cint_field_reflector_1pass.c

	
 ~ Name:
+-------------------------+
|    AC 1:1 Protection    |
+-------------------------+
 ~ Description:
    An example of AC 1:1 protection configuration. 
     - Packets can be sent from the In-AC towards the Out-ACs
     - UC - Unicast traffic is achieved by using the FEC Protection and sending a packet with a known DA
     - MC - Multicast Protection is achieved by defining a MC group an using egress protection for unknown DA packets
    Another example is of a FEC facility protection where the traffic selection is according to the actual working
    state of the ports. Also a simultaneous mode is presented, where the traffic is selected according to standard FEC
    protection, but can be overridden when the selected ports isn't working.
     

 ~ File Reference:
    src/examples/dpp/cint_ac_1to1_coupled_protection.c
    Facility protection - src/examples/dpp/cint_ac_fec_facility_protection.c
    
	
 ~ Name:
+---------------+
|    ELK ACL    |
+---------------+
 ~ Description:
    An example of configuration in ELK ACL. 
     - Indicating via SoC property that the KBP device is connected and can be used for external lookups 
     - Indicating whether the IPv4 routing tables (including IPv4 unicast with and without RPF and IPv4 Multicast lookups) are located either in BCM88650 (Arad) or in KBP via SOC properties.
     - Initializing the BCM88650 (Arad) device.
     - Defining the field groups:
        - In the forwarding stage, defining a 5 tuples of IPv4 for the IPv4 unicast packets (both bridged and routed).
		  This lookup is the third lookup in the KBP device. The lookup hit indication and the lookup result are passed to the ingress field processor (FP) stage.
        - In the ingress FP stage, define a field group preselecting only on the same IPv4 unicast packets and on the hit indication.
		  If matched, define the lookup result as meter value (direct extraction field group).

 ~ File Reference:
    src/examples/dpp/cint_field_elk_acl.c

	
 ~ Name:
+-------------------+
|    ELK IPv4 DC    |
+-------------------+
 ~ Description:
    An example of configuration in ELK IPv4 DC.
     - Indicating via SoC property that the KBP device is connected and can be used for external lookups
     - Indicating whether the IPv4 DC routing tables are located in BCM88650 (Arad) or in KBP via SoC properties
     - Initializing the BCM88650 (Arad) device
     - Adding entries to DC routing table

 ~ File Reference:
    src/examples/dpp/cint_ipv4_dc.c.c

	
 ~ Name:
+--------------------------------------------------------+
|    MAC Extender application in Higig TM and PP Mode    |
+--------------------------------------------------------+
 ~ Description:
    An example of MAC extender application in Higig TM and PP mode.
     - IXIA ports 3-7 send PP traffic and map to XE22 (PP port) on the QMX side
     - IXIA port 3-5 send TM traffic and map to XE21(TM port) on the QMX side
     - IXIA port 3-6 send normal traffic to XE13 and out of chip by Higig port XE21
     - PP traffic is switched to XE13 through MC while TM traffic is forwarded to XE13 according to ITMH(0x01000d00)

 ~ File Reference:
    src/examples/dpp/cint_xgs_mac_extender_mappings_tm.c

	
 ~ Name:
+--------------------------------------+
|    PMF Stage in BFD Echo Solution    |
+--------------------------------------+
 ~ Description:
    An example of configuring the PMF stage in a BFD echo solution.
     - Enabling SoC property bfd_echo_enabled=1 custom_feature_bfd_echo_with_lem=1 (together with all other OAM SoC properties)
     - Adding a PMF rule using the cint: cint_field_bfd_echo.c function:bfd_echo_with_lem_preselector
     - Adding a BFD endpoint by calling bcm_bfd_endpoint_create with: flags |= BCM_BFD_ECHO & BCM_BFD_ENDPOINT_MULTIHOP

 ~ File Reference:
    src/examples/dpp/cint_field_bfd_echo.c
	

 ~ Name:
+-------------------+
|    Field Snoop    |
+-------------------+
 ~ Description:
    An example of filtering all packets that are IPv4/6 over MPLS.
     -  Setting a snoop profile to send to some destination local ports. Alternatively, a system port can be the destination of the mirror port.
     -  Receive the snoop gport as an output. This gport will be the parameter used for snoop action

 ~ File Reference:
    src/examples/dpp/cint_field_snoop_example.c

	
 ~ Name:
+------------------------+
|    MPLS LIF Counter    |
+------------------------+
 ~ Description:
    An example of MPLS LIF Counter.
     - Setting the counting source and LIF range of the LIF counting profile
     - Setting up the MPLS service and getting the inlif_gport or outlif_gport; also performing the shaping or QoS according to the stat_mode
     - Associating inlif_gport or outlif_gport to the counting profile
     - Sending the packet
     - Getting 32-bit counter value for specific statistic types

 ~ File Reference:
    src/examples/dpp/cint_mpls_lif_counter.c

	
 ~ Name:
+-------------------------------------------+
|    ECMP Meter Resolution to FEC Member    |
+-------------------------------------------+
 ~ Description:
    An example of ECMP meter resolution to FEC member.
     - Simulates the ECMP meter resolution to FEC member performed by BCM88660 (Arad+) and BCM88670 (Jericho) and runs independently(!) from the SDK
     - b\Based on cint_trunk_ecmp_lb_key_and_member_retrieve.c which is dependent on the SDK

 ~ File Reference:
    src/examples/dpp/cint_ecmp_hash_resolve.c


 ~ Name:
+---------------------------+
|    VLAN VSI Membership    |
+---------------------------+
 ~ Description:
    An example of VLAN VSI membership.
     - Ingress VLAN membership filter with drop packet trap
     - Ingress VLAN membership filter with send to CPU packet trap
     - Enabling egress VLAN membership filter 
     - Disabling egress VLAN membership filter 

 ~ File Reference:
    src/examples/dpp/cint_vlan_vsi_membership.c

	
 ~ Name:
+--------------------------------------+
|    BFD Endpoints Time Measurement    |
+--------------------------------------+
 ~ Description:
    An example of BFD endpoints time measurement.
     - Time measurement for creating and removing BFD endpoints

 ~ File Reference:
    src/examples/dpp/cint_bfd_performance_test.c

	
 ~ Name:
+-------------------------------+
|    MPLS NOP Action in EEDB    |
+-------------------------------+
 ~ Description:
    An example of using MPLS NOP action.
     - Adding EEDB SWAP LIFs
     - Adding EEDB NOP LIFs
     - Adding a PMF rule to edit OutLif=NOP and EEI.Outlif=swap
     - Packet sent is label swapped

 ~ File Reference:
    src/examples/dpp/cint_mpls_nop_action_encapsulation.c

	
 ~ Name:
+-----------------------+
|    OAM OAMP Server    |
+-----------------------+
 ~ Description:
    An example of using OAMP server.
     - Using two units, one for the server and one for the client 
     - Creating and connecting VLAN ports
     - Adding down and up MEPs to each unit 

 ~ File Reference:
    src/examples/dpp/cint_oam_server.c

	
 ~ Name:
+---------------------+
|    Push MPLS GRE    |
+---------------------+
 ~ Description:
    An example of IP routing to MPLS + IP-GRE tunnel encapsulation. 

 ~ File Reference:
    src/examples/dpp/cint_push_mpls_gre.c

	
 ~ Name:
+---------------------------+
|    VLAN Tag Acceptance    |
+---------------------------+
 ~ Description:
    An example of egress tag acceptance.
     - Enabling egress tag acceptance
     - Adding a VLAN to discard list
     - Only in advanced mode: Setting packet untagged after EVE

 ~ File Reference:
    src/examples/dpp/cint_vlan_egress_tag_acceptance.c

	
 ~ Name:
+------------------------------+
|    Egress VLAN Membership    |
+------------------------------+
 ~ Description:
    An example of VLAN membership.
     - Enabling egress VLAN membership filter
     - Adding a port to VLAN membership list
     - Removing a port from VLAN membership list

 ~ File Reference:
    src/examples/dpp/cint_vlan_egress_membership.c

	
 ~ Name:
+---------------------------+
|    VLAN Tag Acceptance    |
+---------------------------+
 ~ Description:
    An example of egress tag acceptance.
     - Enabling egress tag acceptance
     - Adding a VLAN to discard list
     - Only in advanced mode: Setting packet untagged after EVE

 ~ File Reference:
    src/examples/dpp/cint_vlan_egress_tag_acceptance.c

	
 ~ Name:
+------------------------------+
|    Egress VLAN Membership    |
+------------------------------+
 ~ Description:
    An example of VLAN membership.
     - Enabling egress VLAN membership filter
     - Adding a port to VLAN membership list
     - Removing a port from VLAN membership list

 ~ File Reference:
    src/examples/dpp/cint_vlan_egress_membership.c

	
 ~ Name:
+--------------------------+
|    Outbound Mirroring    |
+--------------------------+
 ~ Description:
    An example of outbound mirroring and the BCM_VLAN_ACTION_SET_OUTER_VID_UNTAGGED flag.
     - Enabling outbound mirroring
     - Setting the BCM_VLAN_ACTION_SET_OUTER_VID_UNTAGGED flag

 ~ File Reference:
    src/examples/dpp/cint_vlan_mirror_outbound_untagged.c

	
 ~ Name:
+------------------------------------------------+
|    Speculative Parsing of Headers Over MPLS    |
+------------------------------------------------+
~ Description:
    An example of utilizing the field APIs to filter all packets that are IPv4/6 over MPLS.

~ File Reference:
    src/examples/dpp/cint_field_mpls_speculative_parse.c

	
 ~ Name:
+------------------------------+
|    MPLS Additional Labels    |
+------------------------------+
 ~ Description:
    An example of deep MPLS stack encapsulation.  

 ~ File Reference:
    src/examples/dpp/cint_ip_route_tunnel_mpls_additional_labels.c

	
 ~ Name:
+------------------------+
|    VPLS Tagged Mode    |
+------------------------+
 ~ Description:
    An example of configuring ingress forwarding of VPLS tagged mode.  
       - Use cross connect to connect between 2 ports and create P2P service
       - Use flexible connect to connect existing MPLS port p2p service with user defined VPLS to newly created destination port

 ~ File Reference:
    src/examples/dpp/cint_vswitch_vpws_tagged.c


 ~ Name:
+----------------------------------------------+
|    Customer Specific Tunnel Encapsulation    |
+----------------------------------------------+
~ Description:
    An example of customer specific tunnel encapsulation.
	When working as an ingress module the DNX device works as a pure PP, adding customer format TM headers and PP headers to the original packets.
	When working as an egress module the DNX device processes customer format TM headers and PP headers.

~ File Reference:
    cint_connect_to_np_vpws.c
    cint_nph_egress.c
	cint_customer_protection.c


	 ~ Name:
+---------------------------+
|    1+1 Protection VPLS    |
+---------------------------+
~ Description:
    An example of VPLS 1+1 protection. In this scenario, 2 PWE works as a 1+1 protection group (in a multicast group).
	In ingress, a protection path decides which PW receives packets.
	In Egress, traffic is replicated to 2 PWE.

~ File Reference:
    cint_vswitch_vpls_1plus1_protection.c


         ~ Name:
+---------------------------+
|    LSR with context       |
+---------------------------+
~ Description:
    An example of MPLS LSR using label+VRF or label+IN-RIF as ILM lookup key. 

~ File Reference:
    cint_mpls_lsr_context.c





 ~ Name:
+-----------+
|    SER    |
+-----------+
~ Description:
    An example of customer trap SER packets. Configure trap in Ingress if SER occurs and send packet to CPU.

~ File Reference:
    cint_ser.c
	
	
	 ~ Name:
+------------------------------------+
|    Ingress Latency Measurements    |
+------------------------------------+
 ~ Description:
    An example of configuring ingress latency measurements.
       - Use PMF to configure OAM-TS-HEADER Type=LATENCY
	   - Use PMF to set the Latency flow ID.
       - Use Counter processor to store the max latency value (and read it).	   	  

 ~ File Reference:
    src/examples/dpp/cint_ingress_latency_measurements.c

    ~ Name:
+----------------------------------------------+
|    BFD Remote Detect Multiplier Monitoring   |
+----------------------------------------------+
 ~ Description:
    An example of configuring BFD detect multiplier monitoring.
       - PMF solution for detect multiplier monitoring using TCAM compare
       - PMF comparison between LEM results. Configuration detects multiplier on EP and value from packet.
         In case it is an equal packet continue to OAMP. If it is a different packet, copy send to OAMP and second to CPU.

 ~ File Reference:
    src/examples/dpp/cint_field_bfd_ipv4_single_hop_remote_detect_compare.c

 ~ Name:
+-----------------------------------------------------+
|    MPLS Tunnel Termination and Forwarding TTL0/1    |
+-----------------------------------------------------+
 ~ Description:
    An example of MPLS tunnel termination and forwarding TTL0/1 trap configuration.

 ~ File Reference:
    cint_rx_mpls_tunnel_termination_forwarding_ttl0_1.c
 ~ Name:    
+-----------------------------------------------------+
|    IPv4 Tunnel Termination and Forwarding TTL0/1    |
+-----------------------------------------------------+
 ~ Description:
    An example of IPv4 tunnel termination and forwarding TTL0/1 trap configuration.

 ~ File Reference:
    cint_rx_ipv4_tunnel_termination_forwarding_ttl0_1.c
  ~ Name:   
+-----------------------------------------------------------+
|    IPv6 Tunnel Termination and Forwarding HOP COUNT0/1    |
+-----------------------------------------------------------+
 ~ Description:
    An example of IPv6 tunnel termination and forwarding HOP COUNT0/1 trap configuration.

 ~ File Reference:
    cint_rx_ipv6_tunnel_termination_forwarding_hop_count0_1.c

~ Name:
+-----------------------------------------------------+
|    UDP Tunnel Encapsulation                         |
+-----------------------------------------------------+
 ~ Description:
    An example of UDP tunnel encapsulation.

 ~ File Reference:
    cint_udp_tunnel_encap.c

    ~ Name:
+-----------------------------------------------------+
|    UDP Tunnel Termination                           |
+-----------------------------------------------------+
 ~ Description:
    An example of UDP tunnel termination.

 ~ File Reference:
    cint_udp_tunnel_term.c

    ~ Name:
+-----------------------+
|    Tunnel Utilities   |
+-----------------------+
 ~ Description:
    Utility functions for tunneling applications.

 ~ File Reference:
    cint_utils_tunnel.c

    ~ Name:
+-------------------------------------------+
|    GRE with LB key Tunnel Encapsulation   |
+-------------------------------------------+
 ~ Description:
    An example of GRE with LB key tunnel encapsulation.

 ~ File Reference:
    cint_gre_with_lb_tunnel_encap.c

    ~ Name:
+-------------------------------------------+
|    Lawful Interception Tunnel Encapsulation   |
+-------------------------------------------+
 ~ Description:
    An example of Lawful Interception tunnel encapsulation.

 ~ File Reference:
    cint_lawful_interception.c

    ~ Name:
+-------------------------------------------+
|    MPLS Termination with BoS check        |
+-------------------------------------------+
 ~ Description:
    An example of MPLS termination with BoS check.

 ~ File Reference:
    cint_mpls_termination_bos_check.c

    ~ Name:
+-------------------------------------------+
|    snoop +PMF drop action with check cnts |
+-------------------------------------------+
 ~ Description:
    An example of snoop + pmf drop action with counters test.

 ~ File Reference:
    cint_fix_drop_counter.c
                                                     
    ~ Name:
+-------------------------------------------+
|  VOQ and VOQ connector create and connect |
+-------------------------------------------+
 ~ Description:
    Basic functions to create and connect voq / voq connector for a single port.

 ~ File Reference:
    cint_voq_connection.c

  ~ Name:
+--------------------------------------+
|    OAM Y1731 over MPLS-TP Section    |
+--------------------------------------+
 ~ Description:
	An example/reference of Y1731 OAM over MPLS-TP Section. 
	 - Configuring VPLS service
	 - Configuring LIF with non zero VSI
	 - Configuring MPLS-TP/PWE tunnel
	 - Creating groups
	 - Creating local and remote MEPs
	 - Registrating events
 
 ~ File Reference:
	src/examples/dpp/cint_oam_mpls_tp_section.c

  ~ Name:
+--------------------------------------+
|    OAM Ethernet Egress Injection    |
+--------------------------------------+
 ~ Description:
    An example of Ethernet OAM with Egress Injection. 
      - Configuring VSI
      - Configuring 2 LIF's for endpoints
      - Configuring Egress Vlan editing for untagged packets
      - Creating groups
      - Creating local and remote MEPs(2) with VSI
 
 ~ File Reference:
    src/examples/dpp/cint_oam_egress_injection.c

  ~ Name:
+--------------------------------------+
|    OAM Hierarchical Loss Measurement |
+--------------------------------------+
 ~ Description:
    An example of OAM HLM. 
      - Configuring HLM by LIF
      - Configuring HLM by MDL
 
 ~ File Reference:
    src/examples/dpp/cint_oam_hlm.c

 ~ Name:
+--------------------------------------+
|    DRAM bound thresholds             |
+--------------------------------------+
 ~ Description:
    An example how to configure DRAM bound thresholds
 
 ~ File Reference:
    src/examples/dpp/cint_dram_bound_thresh.c
