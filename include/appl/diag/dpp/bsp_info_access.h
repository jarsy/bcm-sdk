/* $Id: bsp_info_access.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef BSP_INFO_ACCESS_H_INCLUDED
/* { */
#define BSP_INFO_ACCESS_H_INCLUDED

#ifdef  __cplusplus
extern "C" {
#endif

/*************
 * INCLUDES  *
 *************/

 /*************
 * DEFINES   *
 *************/

/*************
 * TYPEDEFS  *
 *************/

/*************
 * FUNCTIONS *
 *************/

/*
 * gets the requested FE base address, according to its index (fe_idx).
 * the index starts with 0.
 */
unsigned long bsp_get_fe_base_addr(unsigned int fe_idx);



#ifdef  __cplusplus
}
#endif

/* } */
#endif
