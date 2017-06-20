/* $Id: qax_tbls.h,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __QAX_TBLS_INCLUDED__
/* { */
#define __QAX_TBLS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/cosq.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/error.h>

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

int soc_qax_tbls_init(int unit); /* Init QAX's tables */
int soc_qax_ihp_tbls_init(int unit);
int soc_qax_ips_tbls_init(int unit);
int soc_qax_pts_tbls_init(int unit);
int soc_qax_fcr_tbls_init(int unit);
soc_error_t soc_qax_txq_tbls_init (int unit);
int soc_qax_egq_tbls_init(int unit);
int soc_qax_epni_tbls_init(int unit);
int soc_qax_sch_tbls_init(int unit);
int soc_qax_cgm_tbls_init(int unit);
int soc_qax_imp_tbls_init(int unit);
int soc_qax_iep_tbls_init(int unit);
int soc_qax_tar_tbls_init(int unit);

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __QAX_TBLS_INCLUDED__*/
#endif
