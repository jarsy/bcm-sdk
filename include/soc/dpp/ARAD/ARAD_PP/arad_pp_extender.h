/* $Id: arad_pp_extender.h,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       $SDK/include/soc/dpp/ARAD/ARAD_PP/arad_pp_extender.h
*
* MODULE PREFIX:  arad_pp_extender_
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __ARAD_PP_EXTENDER_INCLUDED__
/* { */
#define __ARAD_PP_EXTENDER_INCLUDED__


/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>
#include <soc/dpp/PPC/ppc_api_extender.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * MACROS    *
 *************/
/* { */


/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* Global sw state info for port extender */
typedef struct arad_pp_extender_info_s {
    uint16                        etag_ethertype; /* E-tag Ethertype for use in the TPID field of the E-tag to recognize E-    
                                                      tag packets at Cascade and Upstream ports. */
} arad_pp_extender_info_t;


/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

soc_error_t arad_pp_extender_init(int unit);
soc_error_t arad_pp_extender_deinit(int unit);

soc_error_t arad_pp_extender_port_info_set(int unit, SOC_PPC_PORT port, SOC_PPC_EXTENDER_PORT_INFO *port_info);
soc_error_t arad_pp_extender_port_info_get(int unit, SOC_PPC_PORT port, SOC_PPC_EXTENDER_PORT_INFO *port_info);


soc_error_t arad_pp_extender_global_etag_ethertype_set(int unit, uint16 etag_tpid);
soc_error_t arad_pp_extender_global_etag_ethertype_get(int unit, uint16 *etag_tpid);

soc_error_t arad_pp_extender_eve_etag_format_set(int unit, uint32 edit_profile, uint8 is_extender);
soc_error_t arad_pp_extender_eve_etag_format_get(int unit, uint32 edit_profile, uint8 *is_extender);

/* } __ARAD_PP_EXTENDER_INCLUDED__*/
#endif


