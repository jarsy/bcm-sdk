/* $Id: ppc_api_extender.h,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       $SDK/include/soc/dpp/PPC/ppc_api_extender.h
*
* MODULE PREFIX:  soc_ppc_extender
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

#ifndef __SOC_PPC_API_EXTENDER_INCLUDED__
/* { */
#define __SOC_PPC_API_EXTENDER_INCLUDED__


/*************
 * INCLUDES  *
 *************/
/* { */

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

typedef enum
{
    SOC_PPC_EXTENDER_PORT_ING_ECID_NOP,
    SOC_PPC_EXTENDER_PORT_ING_ECID_KEEP
} SOC_PPC_EXTENDER_PORT_ING_ECID_MODE;

typedef struct
{
  /*
   *  Indication how to resolve Ing-ECID-Base. Either Set according to 
   *  incoming ECID or do not touch.
   */
  SOC_PPC_EXTENDER_PORT_ING_ECID_MODE ing_ecid_mode;
} SOC_PPC_EXTENDER_PORT_INFO;




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


/* } __SOC_PPC_API_EXTENDER_INCLUDED__*/
#endif

