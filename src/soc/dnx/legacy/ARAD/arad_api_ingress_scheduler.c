/* $Id: jer2_arad_api_ingress_scheduler.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INGRESS
/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_ingress_scheduler.h>

#include <soc/dnx/legacy/ARAD/arad_api_ingress_scheduler.h>

#include <soc/mcm/memregs.h>
#include <soc/dnx/legacy/drv.h>


/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 *  MACROS   *
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

/*********************************************************************
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info
  )
{
  uint32 res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_MESH_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(mesh_info);
  DNX_SAND_CHECK_NULL_INPUT(exact_mesh_info);

  res = jer2_arad_ingress_scheduler_mesh_verify(
    unit,
    mesh_info,
    exact_mesh_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ingress_scheduler_mesh_set_unsafe(
    unit,
    mesh_info,
    exact_mesh_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_mesh_set()",0,0);
}


soc_error_t
  jer2_arad_ingress_scheduler_mesh_bandwidth_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_IN  uint32              rate
    )
{
    uint32 dnx_sand_rc;
    DNX_TMC_ING_SCH_MESH_INFO mesh_info, exact_mesh_info;

    DNXC_INIT_FUNC_DEFS;

    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_get(unit, &mesh_info);

    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);

    jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_clear(&exact_mesh_info);

    /*setting all shaper fields to don't touch (0xffffffff)*/
    jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_SHAPERS_dont_touch(&mesh_info);

    if (_SHR_GPORT_IS_FABRIC_MESH_LOCAL(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_LOCAL].shaper.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV1(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON1].shaper.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV2(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON2].shaper.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV3(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON3].shaper.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV4(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON4].shaper.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV5(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON5].shaper.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV6(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON6].shaper.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV7(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON7].shaper.max_rate = rate;
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric mesh type\n"), gport));
    }
    
    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_set(unit, &mesh_info, &exact_mesh_info);
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);

exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_mesh_sched_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_IN  int                 weight
    )
{
    uint32 dnx_sand_rc;
    DNX_TMC_ING_SCH_MESH_INFO mesh_info, exact_mesh_info;

    DNXC_INIT_FUNC_DEFS;

    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_get(unit, &mesh_info);
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);

    jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_clear(&exact_mesh_info);

    /*setting all shaper fields to don't touch (0xffffffff)*/
    jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_SHAPERS_dont_touch(&mesh_info);

    if (_SHR_GPORT_IS_FABRIC_MESH_LOCAL(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_LOCAL].weight = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV1(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON1].weight = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV2(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON2].weight = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV3(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON3].weight = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV4(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON4].weight = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV5(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON5].weight = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV6(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON6].weight = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV7(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON7].weight = weight;
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric mesh type\n"), gport));
    }

    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_set(unit, &mesh_info, &exact_mesh_info);
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);
      
exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_mesh_burst_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_IN  int                 burst
    )
{
    uint32 dnx_sand_rc;
    DNX_TMC_ING_SCH_MESH_INFO mesh_info, exact_mesh_info;

    DNXC_INIT_FUNC_DEFS; 
    
    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_get(unit, &mesh_info);
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);

    jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_clear(&exact_mesh_info);

    /*setting all shaper fields to don't touch (0xffffffff)*/
    jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_SHAPERS_dont_touch(&mesh_info);

    if (_SHR_GPORT_IS_FABRIC_MESH_LOCAL(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_LOCAL].shaper.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV1(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON1].shaper.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV2(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON2].shaper.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV3(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON3].shaper.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV4(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON4].shaper.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV5(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON5].shaper.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV6(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON6].shaper.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV7(gport)) {
        mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON7].shaper.max_burst = burst;
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric mesh type\n"), gport));
    }
    
    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_set(unit, &mesh_info, &exact_mesh_info);

    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);    
      
exit:    
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info
  )
{
  uint32  res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INGRESS_SCHEDULER_MESH_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(mesh_info);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_ingress_scheduler_mesh_get_unsafe(
    unit,
    mesh_info
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ingress_scheduler_mesh_get()",0,0);
}

soc_error_t
  jer2_arad_ingress_scheduler_mesh_bandwidth_get(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_OUT uint32              *rate
    )
{
    JER2_ARAD_ING_SCH_MESH_INFO mesh_info;
    uint32 dnx_sand_rc;

    DNXC_INIT_FUNC_DEFS;

    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_get(unit, &mesh_info);
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);

    if (_SHR_GPORT_IS_FABRIC_MESH_LOCAL(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_LOCAL].shaper.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV1(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON1].shaper.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV2(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON2].shaper.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV3(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON3].shaper.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV4(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON4].shaper.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV5(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON5].shaper.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV6(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON6].shaper.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV7(gport)) {
        *rate = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON7].shaper.max_rate;
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric mesh type\n"), gport));
    }

exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_mesh_sched_get(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_OUT int                 *weight
    )
{
    JER2_ARAD_ING_SCH_MESH_INFO mesh_info;
    uint32 dnx_sand_rc;

    DNXC_INIT_FUNC_DEFS;

    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_get(unit, &mesh_info);
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);  
    
    if (_SHR_GPORT_IS_FABRIC_MESH_LOCAL(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_LOCAL].weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV1(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON1].weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV2(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON2].weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV3(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON3].weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV4(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON4].weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV5(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON5].weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV6(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON6].weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV7(gport)) {
        *weight = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON7].weight;
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric mesh type\n"), gport));
    }

exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_mesh_burst_get(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_OUT int                 *burst
    )
{
    JER2_ARAD_ING_SCH_MESH_INFO mesh_info;
    uint32 dnx_sand_rc;

    DNXC_INIT_FUNC_DEFS;

    dnx_sand_rc = jer2_arad_ingress_scheduler_mesh_get(unit, &mesh_info);
    DNXC_SAND_IF_ERR_EXIT(dnx_sand_rc);    

    if (_SHR_GPORT_IS_FABRIC_MESH_LOCAL(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_LOCAL].shaper.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV1(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON1].shaper.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV2(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON2].shaper.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV3(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON3].shaper.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV4(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON4].shaper.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV5(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON5].shaper.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV6(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON6].shaper.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_MESH_DEV7(gport)) {
        *burst = mesh_info.contexts[JER2_ARAD_ING_SCH_MESH_CON7].shaper.max_burst;
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric mesh type\n"), gport));
    }

exit:    
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info   
  )
{

  uint32  res;
  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   exact_clos_info;

DNXC_INIT_FUNC_DEFS;

  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_clear(&exact_clos_info);
  res = jer2_arad_ingress_scheduler_clos_verify(
    unit,
    clos_info,
    &exact_clos_info
  );
 DNXC_SAND_IF_ERR_EXIT(res);

  res = jer2_arad_ingress_scheduler_clos_set_unsafe(
    unit,
    clos_info,
    &exact_clos_info
  );

  DNXC_SAND_IF_ERR_EXIT(res);
exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     This procedure configure the ingress scheduler shaper rates when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_bandwidth_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_IN  uint32              rate
    )
{
    JER2_ARAD_ING_SCH_CLOS_INFO clos_info;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_get(unit, 0, &clos_info));

    /*setting all shaper fields to don't touch (0xffffffff)*/
    jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_SHAPER_dont_touch(&clos_info);

    if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL(gport)) {
        clos_info.shapers.local.max_rate = rate;
    }
    else if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_HIGH(gport)) {
        clos_info.shapers.hp.local.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC(gport)){
        clos_info.shapers.fabric.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_HIGH(gport)) {
        clos_info.shapers.hp.fabric_unicast.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_GUARANTEED(gport)) {
        clos_info.shapers.hp.fabric_multicast.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_BESTEFFORT(gport)) {
        clos_info.shapers.lp.fabric_multicast.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_LOW(gport)) {
        clos_info.shapers.lp.fabric_unicast.max_rate = rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_HIGH(gport) || _SHR_GPORT_IS_FABRIC_CLOS_FABRIC_LOW(gport) || 
             _SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_LOW(gport)) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_DNXC_MSG("cannot set bandwidth, gport does not have shaper\n")));
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT,(_BSL_DNXC_MSG("gport type is not matched to fabric clos type\n")));
    }
    
    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_set(unit, 0, &clos_info));

      
exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_clos_sched_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_IN  int                 weight
    )
{
    JER2_ARAD_ING_SCH_CLOS_INFO clos_info;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_get(unit, 0, &clos_info));

    /*setting all shaper fields to don't touch (0xffffffff)*/
    jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_SHAPER_dont_touch(&clos_info);

    if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_HIGH(gport)) {
        clos_info.weights.fabric_hp.weight1 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_GUARANTEED(gport)) {
        clos_info.weights.fabric_hp.weight2 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_LOW(gport)) {
        clos_info.weights.fabric_lp.weight1 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_BESTEFFORT(gport)) {
        clos_info.weights.fabric_lp.weight2 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_HIGH(gport)) {
        clos_info.weights.global_hp.weight1 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_HIGH(gport)) {
        clos_info.weights.global_hp.weight2 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_LOW(gport)) {
        clos_info.weights.global_lp.weight1 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_LOW(gport)) {
        clos_info.weights.global_lp.weight2 = weight;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL(gport) || _SHR_GPORT_IS_FABRIC_CLOS_FABRIC(gport)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("cannot get scheduling policy, gport %d does not have weights\n"), gport));
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric clos type\n"), gport));
    } 
    
    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_set(unit, 0, &clos_info));

exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_clos_burst_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_IN  int                 burst
    )
{
    JER2_ARAD_ING_SCH_CLOS_INFO clos_info;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_get(unit, 0, &clos_info));

    /*setting all shaper fields to don't touch (0xffffffff)*/
    jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_SHAPER_dont_touch(&clos_info);

    
    if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL(gport)) {
        clos_info.shapers.local.max_burst = burst;
    }
    else if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_HIGH(gport)) {
        clos_info.shapers.hp.local.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC(gport)){
        clos_info.shapers.fabric.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_HIGH(gport)) {
        clos_info.shapers.hp.fabric_unicast.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_GUARANTEED(gport)) {
        clos_info.shapers.hp.fabric_multicast.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_BESTEFFORT(gport)) {
        clos_info.shapers.lp.fabric_multicast.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_LOW(gport)) {
        clos_info.shapers.lp.fabric_unicast.max_burst = burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_HIGH(gport) || _SHR_GPORT_IS_FABRIC_CLOS_FABRIC_LOW(gport) || 
             _SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_LOW(gport)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("cannot set max burst, gport %d does not have a shaper\n"), gport));
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric clos type\n"), gport));
    }
    
    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_set(unit, 0, &clos_info));

exit:    
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
*     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  )
{

  uint32  res;

DNXC_INIT_FUNC_DEFS;

  res = jer2_arad_ingress_scheduler_clos_get_unsafe(
    unit,
    clos_info
  );
  DNXC_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_clos_bandwidth_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_OUT uint32              *rate
  )
{
    JER2_ARAD_ING_SCH_CLOS_INFO clos_info;

    DNXC_INIT_FUNC_DEFS;
        
    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_get(unit, 0, &clos_info));
 
    if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL(gport)) {
        *rate = clos_info.shapers.local.max_rate;
    }
    else if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_HIGH(gport)) {
        *rate = clos_info.shapers.hp.local.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC(gport)){
        *rate = clos_info.shapers.fabric.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_HIGH(gport)) {
        *rate = clos_info.shapers.hp.fabric_unicast.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_GUARANTEED(gport)) {
        *rate = clos_info.shapers.hp.fabric_multicast.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_BESTEFFORT(gport)) {
        *rate = clos_info.shapers.lp.fabric_multicast.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_LOW(gport)) {
        *rate = clos_info.shapers.lp.fabric_unicast.max_rate;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_HIGH(gport) || _SHR_GPORT_IS_FABRIC_CLOS_FABRIC_LOW(gport) || 
             _SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_LOW(gport)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL,(_BSL_DNXC_MSG("cannot get bandwidth, specified gport does not have a shaper\n")));
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT,(_BSL_DNXC_MSG("specified gport type is not matched to fabric clos type\n")));
    }

exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_clos_sched_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_OUT int                 *weight
  )
{
    JER2_ARAD_ING_SCH_CLOS_INFO clos_info;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_get(unit, 0, &clos_info));    

    if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_HIGH(gport)) {
        *weight = clos_info.weights.fabric_hp.weight1;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_GUARANTEED(gport)) {
        *weight = clos_info.weights.fabric_hp.weight2;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_LOW(gport)) {
        *weight = clos_info.weights.fabric_lp.weight1;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_BESTEFFORT(gport)) {
        *weight = clos_info.weights.fabric_lp.weight2;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_HIGH(gport)) {
        *weight = clos_info.weights.global_hp.weight1;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_HIGH(gport)) {
        *weight = clos_info.weights.global_hp.weight2;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_LOW(gport)) {
        *weight = clos_info.weights.global_lp.weight1;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_LOW(gport)) {
        *weight = clos_info.weights.global_lp.weight2;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL(gport) || _SHR_GPORT_IS_FABRIC_CLOS_FABRIC(gport)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("cannot get scheduling policy, gport %d does not have weights\n"), gport));
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric clos type\n"), gport));
    }


exit:    
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_arad_ingress_scheduler_clos_burst_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_gport_t         gport, 
    DNX_SAND_OUT int                 *burst
  )
{
    JER2_ARAD_ING_SCH_CLOS_INFO clos_info;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_arad_ingress_scheduler_clos_get(unit, 0, &clos_info));
        
    if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL(gport)) {
        *burst = clos_info.shapers.local.max_burst;
    }
    else if(_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_HIGH(gport)) {
        *burst = clos_info.shapers.hp.local.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC(gport)){
        *burst = clos_info.shapers.fabric.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_HIGH(gport)) {
        *burst = clos_info.shapers.hp.fabric_unicast.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_GUARANTEED(gport)) {
        *burst = clos_info.shapers.hp.fabric_multicast.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FMQ_BESTEFFORT(gport)) {
        *burst = clos_info.shapers.lp.fabric_multicast.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_UNICAST_FABRIC_LOW(gport)) {
        *burst = clos_info.shapers.lp.fabric_unicast.max_burst;
    }
    else if (_SHR_GPORT_IS_FABRIC_CLOS_FABRIC_HIGH(gport) || _SHR_GPORT_IS_FABRIC_CLOS_FABRIC_LOW(gport) || 
             _SHR_GPORT_IS_FABRIC_CLOS_UNICAST_LOCAL_LOW(gport)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("cannot get max burst, gport %d does not have a shaper\n"), gport));
    }
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("gport %d type is not matched to fabric clos type\n"), gport));
    }

exit:    
    DNXC_FUNC_RETURN;
}

void
  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_SHAPER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_SHAPER_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_MESH_CONTEXT_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_MESH_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_SHAPERS_dont_touch(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);
  DNX_TMC_ING_SCH_MESH_INFO_SHAPERS_dont_touch(info);
  
  exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQS_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_WFQS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_WFQS_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_HP_SHAPERS_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_SHAPERS_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_SHAPERS_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_SHAPER_dont_touch(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);
  DNX_TMC_ING_SCH_CLOS_INFO_SHAPERS_dont_touch(info);
  
  exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if JER2_ARAD_DEBUG_IS_LVL1

const char*
  jer2_arad_JER2_ARAD_ING_SCH_MESH_CONTEXTS_to_string(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_CONTEXTS enum_val
  )
{
  return DNX_TMC_ING_SCH_MESH_CONTEXTS_to_string(enum_val);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_SHAPER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_SHAPER_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_MESH_CONTEXT_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_MESH_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQS_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_WFQS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_WFQS_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_HP_SHAPERS_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_SHAPERS_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_SHAPERS *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_SHAPERS_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_ING_SCH_CLOS_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

uint8
  jer2_arad_ingress_scheduler_conversion_test_api(
    DNX_SAND_IN uint8 is_regression,
    DNX_SAND_IN uint8 silent
  )
{
  uint8 pass;

  pass = jer2_arad_ingress_scheduler_conversion_test(
           is_regression,
           silent
         );

  if (!pass)
  {
    LOG_CLI((BSL_META("The jer2_arad_ingress_scheduler_conversion_test has FAILED!"
                      "\n\r"
                 )));
    goto exit;
  }
  else
  {
    LOG_CLI((BSL_META("The jer2_arad_ingress_scheduler_conversion_test has passed successfully!"
                      "\n\r"
                 )));
  }

exit:
  return pass;

}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h> 

