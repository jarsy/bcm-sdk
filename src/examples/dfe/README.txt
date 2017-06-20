 $Id: README.txt,v 1.55 Broadcom SDK $
 $Copyright: (c) 2016 Broadcom.
 Broadcom Proprietary and Confidential. All rights reserved.$
   
   
 ~ Name:   
+-----------------------+                                                     
|    Link Monitoring    |                                                          
+-----------------------+                                                        
 ~ Description:                                                                
    An example application to monitor the state of the links and to react to port/link failures.                                            
	1. Setting topology for local module.
	2. Setting topology for FAP groups.  
	
 ~ File Reference:                                                              
    cint_linkscan.c                                                           
     
	 
 ~ Name:	 
+-------------------------+                                                        
|    Snake Application    |                                                         
+-------------------------+                                                       
 ~ Description:
	The test is a stress test on the BCM88750 (FE1600) links. The test validates the links performance under a burst of cells.                                                                                                   
                                                                              
 ~ File Reference:                                                              
    cint_snake_test.c                                                              

	
 ~ Name:   
+----------------+
|    Topology    |
+----------------+
 ~ Description:  
	Examples of topology configuration.
	 - Topology for local module
	 - Topology for FAP groups
	 
 ~ File Reference:
	cint_topology.c	

	 
 ~ Name:   
+-----------------+                                                        
|    Multicast    |                                                       
+-----------------+                                                         
 ~ Description:  
	The cint demonstrate two examples of multicast applications, direct and indirect.
	 - Direct: Set multicast table
	 - Indirect: Set multicast table and create a static topology
                  
 ~ File Reference:
	cint_multicast_test.c                  

	
 ~ Name:   
+-----------------+
|    Warm Boot    |
+-----------------+
 ~ Description:
	Test for BCM88750 (FE1600) warm reboot.

 ~ File Reference: 
	cint_warmboot.c
 
 
 ~ Name:   
+--------------------+
|    Flow Control    |
+--------------------+
 ~ Description:
	This example demonstrates and tests the FIFO threshold sequence.

 ~ File Reference:
	cint_fifos.c

	  
 ~ Name:   
+---------------------------+
|    Flow Control FE3200    |
+---------------------------+
 ~ Description:
	This example demonstrates and tests the FIFO threshold pipe_set/get API for BCM88770 (FE3600, previously known as FE3200).

 ~ File Reference:
	cint_flow_control_fe3200.c
    
	  
 ~ Name:   	  
+----------------------+
|    FE3200 Interop    |
+----------------------+
 ~ Description:
	This example demonstrate the configuration required when connecting BCM88770 (FE3600, previously known as FE3200) to legacy devices.

 ~ File Reference:
	cint_fe3200_interop.c


 ~ Name:   	  
+----------+
|    RX    |
+----------+
 ~ Description:
	This example demonstrates a configuration when using the CPU2CPU packets mechanism.

 ~ File Reference:
	cint_dfe_cpu_packets.c