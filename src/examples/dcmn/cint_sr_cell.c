/*
 * $Id: cint_sr_cell.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * DCMN send \ receive SR cell example:
 * 
 * The example simulate:
 *  1. define single route and send  sr cell to this route
 *  2. define route group and cell sr cell to the group
 *  3. receive sr cells
 *
 */

uint32 data_set[16];
uint32 data_get[76];


/*define single route and send  sr cell to this route*/
int
send_route(int unit) {
    int rv, route_id;
    int link_ids[2];
    int is_qax;
    bcm_fabric_route_t route;

    bcm_fabric_route_t_init(&route);

   
    /*define a single route from FE1 to FE3*/
    route.number_of_hops = 2;
    /*goto FE through link 15*/
    link_ids[0] = 15;
    /*goto FAP from FE through link 72 for Kalia or 0 for other FAPs*/
    rv = is_device_qax(unit, &is_qax);
    if (rv != BCM_E_NONE) {
        printf("Error, in is_device_qax, rv=%d, \n", rv);
        return rv;
    }
    if(is_qax) {
        link_ids[1] = 72;
    } else {
        link_ids[1] = 0;
    }
    route.hop_ids = link_ids;

    /*build data*/
    data_set[0]  = 0x00000000;
    data_set[1]  = 0x11111111;
    data_set[2]  = 0x22222222;
    data_set[3]  = 0x33333333;
    data_set[4]  = 0x44444444;
    data_set[5]  = 0x55555555;
    data_set[6]  = 0x66666666;
    data_set[7]  = 0x77777777;
    data_set[8]  = 0x88888888;
    data_set[9]  = 0x99999999;
    data_set[10] = 0xaaaaaaaa;
    data_set[11] = 0xbbbbbbbb;
    data_set[12] = 0xcccccccc;
    data_set[13] = 0xdddddddd;
    data_set[14] = 0xeeeeeeee;
    data_set[15] = 0xffffffff;

    /*send source-routed cell*/
    rv = bcm_fabric_route_tx(unit, 0,route, 16, data_set);
      if (rv != BCM_E_NONE) {
          printf("Error, in bcm_fabric_route_tx, rv=%d, \n", rv);
          return rv;
      }
    
    return BCM_E_NONE;    
}

/*receive sr cells*/
int
receive_sr_cell(int unit, int max_messages) {
    int rv, count, i;
    uint32 data_actual_size;

    /* 
     * in case several sr cells received the cells are accumulated in SW 
     * For that reason it's important to read in loop (even if the relevant interrupt is down) 
     * until soc_receive_sr_cell return EMPTY error.
    */
    count = 0;
    while(count < max_messages) {
        /*receive sr cell data*/
        rv = bcm_fabric_route_rx(unit, 0, 76, data_get, &data_actual_size);
         /*all messages was read*/
        if(BCM_E_EMPTY == rv) {
            printf("No more messages to read \n");
            break;
        } else if (rv != BCM_E_NONE) {
            printf("Error, in soc_receive_sr_cell, rv=%d, \n", rv);
            return rv;
        }

         /*print received data*/
        printf("Message received: ");
        for(i=0 ; i<data_actual_size ; i++) {
            printf("0x%x ",data_get[i]);
        }
        printf("\n");

        count++;
    }

   printf("%d messages received \n", count);

   return BCM_E_NONE;    
}


int run_sr_cell(int unit) {
  send_route(unit); 
  receive_sr_cell(unit, 1);
}

int cint_sr_cell_system_test(int unit_fap_0, int unit_fap_1) {

  int i;
  int pass = 1;
  int rv;

  /*Clear buffer*/
  receive_sr_cell(unit_fap_1, 1000);

  rv = send_route(unit_fap_0); 
  if (rv != BCM_E_NONE)
  {
      printf("failed to send cell\n");
      return rv;
  }

  rv = receive_sr_cell(unit_fap_1, 1);
  if (rv != BCM_E_NONE)
  {
      printf("failed to send cell\n");
      return rv;
  }

  for (i = 0; i < 16; i++) {
      if (data_set[i] != data_get[i]) {
          pass = 0;
      }
  }
  if (pass) {
      printf("cint_sr_cell_system_test: PASS\n");
  }

}

