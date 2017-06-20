/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_pp_trap.h
 */

#ifndef __QAX_PP_TRAP_INCLUDED__
#define __QAX_PP_TRAP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/JER/JER_PP/jer_pp_trap.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/SAND/Utils/sand_footer.h>

#include <soc/dpp/PPC/ppc_api_eg_mirror.h>

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

soc_error_t soc_qax_pp_trap_ingress_ser_set(int unit,int enable,int trap_hw_id);


/* } */
#endif /* __QAX_PP_OAM_INCLUDED__ */
