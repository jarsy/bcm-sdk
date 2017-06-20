#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_fabric.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>

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
*     This procedure enables/disables flow-control on fabric
*     links.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC           *info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_ENABLE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(info);

  res = jer2_arad_fabric_fc_enable_verify(
          unit,
          direction_ndx,
          info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fabric_fc_enable_set_unsafe(
          unit,
          direction_ndx,
          info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_enable_set()",0,0);
}

/*********************************************************************
*     This procedure enables/disables flow-control on fabric
*     links.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC           *info_rx,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC           *info_tx
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_ENABLE_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(info_rx);
  DNX_SAND_CHECK_NULL_INPUT(info_tx);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fabric_fc_enable_get_unsafe(
          unit,
          info_rx,
          info_tx
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_enable_get()",0,0);
}

/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_shaper_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure sets parameters of both fabric link 
*     data shapers and flow control shapers. 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode -
*    The mode of the shapers - shape transmitted bytes
*    or transmitted cells.
*  DNX_SAND_IN JER2_ARAD_FABRIC_FC_SHAPER    *info -
*     Struct holds the data shaper and
*     flow control shapers information.
*  DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER    *exact_info -
*     Struct holds the exact data shaper and
*     flow control shapers information that was set.
*  
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                        link_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER            *info,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER            *exact_info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_SHAPER_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(shaper_mode);
  DNX_SAND_CHECK_NULL_INPUT(info);
  DNX_SAND_CHECK_NULL_INPUT(exact_info);

  res = jer2_arad_fabric_fc_shaper_verify(
          unit,
          link_ndx,
          shaper_mode,
          info,
          exact_info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fabric_fc_shaper_set_unsafe(
          unit,
          link_ndx,
          shaper_mode,
          info,
          exact_info
        );

  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_shaper_set()",link_ndx,0);
}



/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_shaper_get
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure sets parameters of both fabric link 
*     data shapers and flow control shapers. 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER    *info -
*     Struct holds the data shaper and
*     flow control shaper information of the link.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode -
*    The mode of the shapers - shape transmitted bytes
*    or transmitted cells.
*  
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_get(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  uint32                         link_ndx,
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER            *info
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_ENABLE_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(shaper_mode);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fabric_fc_shaper_get_unsafe(
          unit,
          link_ndx,
          shaper_mode,
          info
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_shaper_get()",link_ndx,0);
}



/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_stand_alone_fap_mode_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint8                 *is_single_fap_mode
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(is_single_fap_mode);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fabric_stand_alone_fap_mode_get_unsafe(
    unit,
    is_single_fap_mode
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_stand_alone_fap_mode_get()",0,0);
}

/*********************************************************************
*     Configure the fabric mode to work in one of the
*     following modes: FE, back to back, mesh or multi stage
*     FE.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_connect_mode_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_CONNECT_MODE_SET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;


  res = jer2_arad_fabric_connect_mode_verify(
    unit,
    fabric_mode
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fabric_connect_mode_set_unsafe(
    unit,
    fabric_mode
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_connect_mode_set()",0,0);
}


/*********************************************************************
*     Retrieve the connectivity map from the device.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_topology_status_connectivity_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                    link_index_min,
    DNX_SAND_IN  int                    link_index_max,
    DNX_SAND_OUT JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR *connectivity_map
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TOPOLOGY_STATUS_CONNECTIVITY_GET);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(connectivity_map);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_fabric_topology_status_connectivity_get_unsafe(
    unit,
    link_index_min,
    link_index_max,
    connectivity_map
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_topology_status_connectivity_get()",0,0);
}

/*********************************************************************
*     Get Fabric link status up/down
*     Details: in the H file. (search for prototype)
*********************************************************************/

void
  jer2_arad_JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR_clear(
    DNX_SAND_OUT JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_FABRIC_CELL_FORMAT_clear(
    DNX_SAND_OUT JER2_ARAD_FABRIC_CELL_FORMAT *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_FABRIC_CELL_FORMAT_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_FABRIC_FC_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_FABRIC_FC_clear(unit, info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_FABRIC_FC_SHAPER_clear(
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER *shaper
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(shaper);

  DNX_TMC_FABRIC_FC_SHAPER_clear(shaper);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO_clear(
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(shaper_mode);

  DNX_TMC_FABRIC_FC_SHAPER_MODE_INFO_clear(shaper_mode);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_LINK_STATE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_LINK_STATE_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_LINK_STATE_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

const char*
  jer2_arad_JER2_ARAD_FABRIC_CONNECT_MODE_to_string(
    DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE enum_val
  )
{
  return DNX_TMC_FABRIC_CONNECT_MODE_to_string(enum_val);
}

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */
