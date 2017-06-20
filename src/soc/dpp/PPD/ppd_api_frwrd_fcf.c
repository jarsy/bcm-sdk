
/* $Id: arad_pp_api_frwrd_fcf.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FORWARD

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Management/sand_chip_descriptors.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_fcf.h>

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
*     Setting global information of the IP routing (including
 *     resources to use)
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_glbl_info_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FRWRD_FCF_GLBL_INFO_SET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  res = arad_pp_frwrd_fcf_glbl_info_set_verify(
          unit,
          glbl_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_glbl_info_set_unsafe(
          unit,
          glbl_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_glbl_info_set()", 0, 0);
}

/*********************************************************************
*     Setting global information of the IP routing (including
 *     resources to use)
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_glbl_info_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FRWRD_FCF_GLBL_INFO_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  res = arad_pp_frwrd_fcf_glbl_info_get_verify(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_glbl_info_get_unsafe(
          unit,
          glbl_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_glbl_info_get()", 0, 0);
}

/*********************************************************************
*     Add fcf route entry to the routing table. Binds between
 *     fcf Unicast route key (fcf-address/prefix) and a FEC
 *     entry identified by route_info for a given router. As a
 *     result of this operation, Unicast fcf packets
 *     designated to the IP address matching the given key (as
 *     long there is no more-specific route key) will be routed
 *     according to the information in the FEC entry identified
 *     by route_info.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_route_add(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_INFO            *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ROUTE_ADD);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(route_key);
  SOC_SAND_CHECK_NULL_INPUT(success);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_route_add_unsafe(
          unit,
          route_key,
          route_info,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_route_add()", 0, 0);
}

/*********************************************************************
*     Gets the routing information (system-fec-id) associated
 *     with the given route key.
 *     Details: in the H file. (search for prototype)
*********************************************************************/

uint32
  soc_ppd_frwrd_fcf_route_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY         *route_key,
    SOC_SAND_IN  uint8                               exact_match,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS      *route_status,
    SOC_SAND_OUT uint8                               *found
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ROUTE_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(route_key);
  SOC_SAND_CHECK_NULL_INPUT(route_info);
  SOC_SAND_CHECK_NULL_INPUT(found);

  res = arad_pp_frwrd_fcf_route_get_verify(
          unit,
          route_key,
          exact_match
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_route_get_unsafe(
          unit,
          route_key,
          exact_match,
          route_info,
          route_status,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_route_get()", 0, 0);
}

/*********************************************************************
*     Gets the fcf UC routing table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_route_get_block(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO            *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT uint32                                  *nof_entries
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ROUTE_GET_BLOCK);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(block_range);
  SOC_SAND_CHECK_NULL_INPUT(route_keys);
  SOC_SAND_CHECK_NULL_INPUT(routes_info);
  SOC_SAND_CHECK_NULL_INPUT(routes_status);
  SOC_SAND_CHECK_NULL_INPUT(nof_entries);

  res = arad_pp_frwrd_fcf_route_get_block_verify(
          unit,
          block_range
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_route_get_block_unsafe(
          unit,
          block_range,
          route_keys,
          routes_info,
          routes_status,
          nof_entries
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_route_get_block()", 0, 0);
}

/*********************************************************************
*     Remove entry from the routing table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_route_remove(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ROUTE_REMOVE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(route_key);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_frwrd_fcf_route_remove_verify(
          unit,
          route_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_route_remove_unsafe(
          unit,
          route_key,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_route_remove()", 0, 0);
}

/*********************************************************************
*     Clear the fcf UC routing table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_routing_table_clear(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_VFI                vfi_ndx,
    SOC_SAND_IN  uint32                               flags
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ROUTING_TABLE_CLEAR);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  res = arad_pp_frwrd_fcf_routing_table_clear_verify(
          unit,
          vfi_ndx,
          flags
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_routing_table_clear_unsafe(
          unit,
          vfi_ndx,
          flags
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_routing_table_clear()", 0, 0);
}

/*********************************************************************
*     Add fcf entry to the zoning table. Binds between Host and
 *     next hop information.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_zoning_add(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ZONING_ADD);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(zoning_key);
  SOC_SAND_CHECK_NULL_INPUT(routing_info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = arad_pp_frwrd_fcf_zoning_add_verify(
          unit,
          zoning_key,
          routing_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_zoning_add_unsafe(
          unit,
          zoning_key,
          routing_info,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_zoning_add()", 0, 0);
}

/*********************************************************************
*     Gets the routing information associated with the given
 *     route key on VFI.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_zoning_get(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY       *zoning_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS     *route_status,
    SOC_SAND_OUT uint8                             *found
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ZONING_GET);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(zoning_key);
  SOC_SAND_CHECK_NULL_INPUT(routing_info);
  SOC_SAND_CHECK_NULL_INPUT(route_status);
  SOC_SAND_CHECK_NULL_INPUT(found);

  res = arad_pp_frwrd_fcf_zoning_get_verify(
          unit,
          zoning_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_zoning_get_unsafe(
          unit,
          zoning_key,
          routing_info,
          route_status,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_zoning_get()", 0, 0);
}

/*********************************************************************
*     Gets the zoning table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_zoning_get_block(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range_key,
    SOC_SAND_IN  uint32                                      flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT uint32                              *nof_entries
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ZONING_GET_BLOCK);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(block_range_key);
  SOC_SAND_CHECK_NULL_INPUT(zoning_keys);
  SOC_SAND_CHECK_NULL_INPUT(routes_info);
  SOC_SAND_CHECK_NULL_INPUT(routes_status);
  SOC_SAND_CHECK_NULL_INPUT(nof_entries);

  res = arad_pp_frwrd_fcf_zoning_get_block_verify(
          unit,
          block_range_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_zoning_get_block_unsafe(
          unit,
          block_range_key,
          flags,
          zoning_keys,
          routes_info,
          routes_status,
          nof_entries
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_zoning_get_block()", 0, 0);
}

/*********************************************************************
*     Remove fcf route entry from the routing table of a
 *     virtual fabric identifier (VFI).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_zoning_remove(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ZONING_REMOVE);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(zoning_key);

  res = arad_pp_frwrd_fcf_zoning_remove_verify(
          unit,
          zoning_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_frwrd_fcf_zoning_remove_unsafe(
          unit,
          zoning_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_zoning_remove()", 0, 0);
}

uint32
  soc_ppd_frwrd_fcf_zoning_table_clear(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               flags,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY         *key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO        *info
  )
{
    uint32
      res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_FCF_ZONING_REMOVE);

    SOC_SAND_CHECK_DRIVER_AND_DEVICE;

    res = arad_pp_frwrd_fcf_zoning_table_clear_verify(
            unit,
            flags,
            key,
            info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    SOC_SAND_TAKE_DEVICE_SEMAPHORE;

    res = arad_pp_frwrd_fcf_zoning_table_clear_unsafe(
            unit,
            flags,
            key,
            info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

  exit_semaphore:
    SOC_SAND_GIVE_DEVICE_SEMAPHORE;
    ARAD_PP_DO_NOTHING_AND_EXIT;
  exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_frwrd_fcf_zoning_table_clear()", 0, 0);
}
 

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>



