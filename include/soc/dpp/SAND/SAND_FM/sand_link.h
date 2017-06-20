/* $Id: sand_link.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_SAND_LINK_INCLUDED__
/* { */
#define __SOC_SAND_LINK_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

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

uint32
  soc_sand_link_fe_bkt_fill_rate_get(void);

uint32
  soc_sand_link_fe_up_link_th_get(void);

uint32
  soc_sand_link_fe_dn_link_th_get(void);

uint32
  soc_sand_link_fe_reachability_rate_get(
    SOC_SAND_IN  uint32                            core_rate,
    SOC_SAND_IN  uint32                            nof_links,
    SOC_SAND_IN  uint32                            nof_rmgr_units
  );

uint32
  soc_sand_link_fe_reachability_watchdog_period_get(
    SOC_SAND_IN  uint32                            core_rate,
    SOC_SAND_IN  uint32                            nof_links
  );

uint32
  soc_sand_link_fap_bkt_fill_rate_get(void);

uint32
  soc_sand_link_fap_up_link_th_get(void);

uint32
  soc_sand_link_fap_dn_link_th_get(void);

uint32
  soc_sand_link_fap_reachability_rate_get(
    SOC_SAND_IN  uint32                            core_rate,
    SOC_SAND_IN  uint32                            nof_links,
    SOC_SAND_IN  uint32                            nof_rmgr_units
  );

uint32
  soc_sand_link_fap_reachability_watchdog_period_get(
    SOC_SAND_IN  uint32                            core_rate
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_SAND_LINK_INCLUDED__*/
#endif
