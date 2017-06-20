/* $Id: arad_pp_frwrd_ipv4_test.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FRWRD_IPV4_TEST_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_IPV4_TEST_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_frwrd_ipv4.h>


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

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  VRF
   */
  uint32 vrf;
  /*
   *  subnet
   */
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY key;

} ARAD_PP_DIAG_IPV4_TEST_VPN_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  lookup key
   */
  ARAD_PP_DIAG_IPV4_TEST_VPN_KEY key;

} ARAD_PP_FRWRD_IPV4_TEST_LKUP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  seed
   */
  uint32 seed;
  /*
   *  Number of routes to add/remove
   */
  uint32 nof_routes;
  /*
   *  Change caching status after these number of routes, if 0
   *  then all is cached, IF cache_change < nof_routes, then
   *  no caching, start status as uncached
   */
  uint32 cache_change;
  /*
   *  after cached enabled how many routes to add in cache mode.
   *  this is the number of cached in each cache iteration
   */
  uint32 nof_cached;
  /*
   *  perform defragment every this count of actions add/remove
   *  set to 0xFFFFFFFF for no defragment
   *  print level > 1 will print memory staus before and after defragment
   */
  uint32 defragment_rate;
  /*
   *  bitmap includes which bank to defragment
   */
  uint32 defragment_banks_bmp;
  /*
   *  Test when add/remove, or just add/remove
   *  BIT 0: hw sim
   *  BIT 1: SW sim
   */
  uint8 test;
  /*
   *  Test by hardware diag
   */
  uint8 hw_test;
  /*
   *  Print level
   */
  uint32 print_level;
  /*
   *  Remove probability
   */
  uint32 remove_prob;
  /*
   *  Addition probability
   */
  uint32 add_prob;
  /*
   *  FEC id for all added routesIf 0xFFFFFFFF then add random
   */
  uint32 fec_id;

} ARAD_PP_FRWRD_IPV4_TEST_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Vrf-id to rest set to 0xFFFFFFFF to reset all VRFs
   */
  uint32 vrf;

} ARAD_PP_FRWRD_IPV4_CLEAR_INFO;

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


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_test_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   init test module..
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_CLEAR_INFO *clear_info -
 *     init information
 * REMARKS:
 *   None
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
  arad_pp_frwrd_ipv4_test_init(
    SOC_SAND_IN int                   unit,
    SOC_SAND_IN ARAD_PP_FRWRD_IPV4_CLEAR_INFO *reset_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_test_clear_vrf
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear all databases
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_CLEAR_INFO *clear_info -
 *     Clear information
 * REMARKS:
 *   None
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_test_clear_vrf(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_CLEAR_INFO *clear_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_diag_frwrd_lpm_lkup_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   lookup query to HW 
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_DIAG_IPV4_TEST_VPN_KEY *lpm_key -
 *     lpm key
 * REMARKS:
 *   None
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 
  arad_pp_frwrd_ipv4_test_lpm_lkup_get(
    SOC_SAND_IN int                               unit,
    SOC_SAND_IN  ARAD_PP_DIAG_IPV4_TEST_VPN_KEY      *lpm_key,
    SOC_SAND_OUT  uint32                             *fec_ptr,
    SOC_SAND_OUT  uint8                              *found
  );


#if SOC_PPC_DEBUG_IS_LVL1
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_test_print_mem
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Print memory status
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_test_print_mem(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  bnk_bmp,
    SOC_SAND_IN  uint32                  print_level
  );
#endif
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_test_lookup
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Lookup for given key in all databases
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_TEST_LKUP_INFO  *lkup_info -
 *     Key to lookupINPUT
 * REMARKS:
 *   None
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_test_lookup(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_TEST_LKUP_INFO  *lkup_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_test_vrf
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Perform random test on VRF
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                  vrf_ndx -
 *     VRF-id
 *   SOC_SAND_IN  uint32                  nof_iterations -
 *     Number of iteration to perform, set to 0xFFFFFFFFINPUT
 * REMARKS:
 *   None
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_test_vrf(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  vrf_ndx,
    SOC_SAND_IN  uint32                  nof_iterations
  );
#if SOC_PPC_DEBUG_IS_LVL1
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_test_run
 * TYPE:
 *   PROC
 * FUNCTION:
 *   run random test on VRF with additions/remove
 * INPUT:
 *   SOC_SAND_IN  int                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_TEST_INFO  *tes_info -
 *     Test informationINPUT
 * REMARKS:
 *   None
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_test_run(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_TEST_INFO  *tes_info
  );
#endif

void
  ARAD_PP_FRWRD_IPV4_TEST_LKUP_INFO_clear(
    SOC_SAND_OUT ARAD_PP_FRWRD_IPV4_TEST_LKUP_INFO *info
  );

void
  ARAD_PP_FRWRD_IPV4_TEST_INFO_clear(
    SOC_SAND_OUT ARAD_PP_FRWRD_IPV4_TEST_INFO *info
  );

void
  ARAD_PP_FRWRD_IPV4_CLEAR_INFO_clear(
    SOC_SAND_OUT ARAD_PP_FRWRD_IPV4_CLEAR_INFO *info
  );

void
  ARAD_PP_DIAG_IPV4_TEST_VPN_KEY_clear(
    SOC_SAND_OUT ARAD_PP_DIAG_IPV4_TEST_VPN_KEY *info
  );

#if SOC_PPC_DEBUG_IS_LVL1
void
  ARAD_PP_FRWRD_IPV4_TEST_LKUP_INFO_print(
    SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_TEST_LKUP_INFO *info
  );

void
  ARAD_PP_FRWRD_IPV4_TEST_INFO_print(
    SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_TEST_INFO *info
  );

void
  ARAD_PP_FRWRD_IPV4_CLEAR_INFO_print(
    SOC_SAND_IN  ARAD_PP_FRWRD_IPV4_CLEAR_INFO *info
  );

void
  ARAD_PP_DIAG_IPV4_TEST_VPN_KEY_print(
    SOC_SAND_IN  ARAD_PP_DIAG_IPV4_TEST_VPN_KEY *info
  );

#endif

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_IPV4_TEST_INCLUDED__*/
#endif

