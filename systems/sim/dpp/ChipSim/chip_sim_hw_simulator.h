/* $Id: chip_sim_hw_simulator.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef __HW_CLIENT_SAND_H_INCLUDED__
/* { */
#define __HW_CLIENT_SAND_H_INCLUDED__
#ifdef __VXWORKS__
  #include <vxworks.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

uint32
  hw_client_activate(void);

/*
 *	Print the file in an output
 */
uint32
  chip_sim_print_output(void);

/*
 *	Allocation of place
 */
uint32
  chip_sim_buffer_alloc(void);

/*
 *	Add the CLI name to the file
 */
uint32
  chip_sim_cli_name_add(
    SOC_SAND_IN  uint32       driver_api_result
  );

#ifdef  __cplusplus
}
#endif


/* } __HW_CLIENT_SAND_H_INCLUDED__ */
#endif

