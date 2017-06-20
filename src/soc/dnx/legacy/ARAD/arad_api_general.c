#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_api_general.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COMMON
/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>


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
*     Verifies validity of port id
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fap_port_id_verify(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID  port_id
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FAP_PORT_ID_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    port_id, JER2_ARAD_MAX_FAP_PORT_ID,
    JER2_ARAD_FAP_PORT_ID_INVALID_ERR,10,exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fap_port_id_verify()",port_id,0);
}

uint32
  jer2_arad_drop_precedence_verify(
    DNX_SAND_IN  uint32      dp_ndx
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_DROP_PRECEDENCE_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    dp_ndx, JER2_ARAD_MAX_DROP_PRECEDENCE,
    JER2_ARAD_DROP_PRECEDENCE_OUT_OF_RANGE_ERR,10,exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fap_port_id_verify()",dp_ndx,0);
}

uint32
  jer2_arad_traffic_class_verify(
    DNX_SAND_IN  uint32      tc_ndx
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_TRAFFIC_CLASS_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    tc_ndx, JER2_ARAD_TR_CLS_MAX,
    JER2_ARAD_TRAFFIC_CLASS_OUT_OF_RANGE_ERR,10,exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_traffic_class_verify()",tc_ndx,0);
}


void
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_DEST_SYS_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_DEST_SYS_PORT_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_DEST_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_DEST_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_DEST_INFO_clear(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_THRESH_WITH_HYST_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_THRESH_WITH_HYST_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_THRESH_WITH_HYST_INFO_clear(info);
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_SWAP_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SWAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SWAP_INFO_clear(info);
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if JER2_ARAD_DEBUG_IS_LVL1
const char*
  jer2_arad_JER2_ARAD_FAR_DEVICE_TYPE_to_string(
    DNX_SAND_IN  JER2_ARAD_FAR_DEVICE_TYPE enum_val
  )
{
  return DNX_TMC_FAR_DEVICE_TYPE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_INTERFACE_TYPE_to_string(
    DNX_SAND_IN  JER2_ARAD_INTERFACE_TYPE enum_val
  )
{
  return DNX_TMC_INTERFACE_TYPE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_INTERFACE_ID_to_string(
    DNX_SAND_IN  JER2_ARAD_INTERFACE_ID enum_val
  )
{
  return DNX_TMC_INTERFACE_ID_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_FC_DIRECTION_to_string(
    DNX_SAND_IN  JER2_ARAD_FC_DIRECTION enum_val
  )
{
  return DNX_TMC_FC_DIRECTION_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_COMBO_QRTT_to_string(
    DNX_SAND_IN  JER2_ARAD_COMBO_QRTT enum_val
  )
{
  return DNX_TMC_COMBO_QRTT_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_DEST_TYPE_to_string(
    DNX_SAND_IN JER2_ARAD_DEST_TYPE enum_val,
    DNX_SAND_IN uint8       short_name
  )
{
  return DNX_TMC_DEST_TYPE_to_string(enum_val, short_name);
}

const char*
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_TYPE_to_string(
    DNX_SAND_IN  JER2_ARAD_DEST_SYS_PORT_TYPE enum_val
  )
{
  return DNX_TMC_DEST_SYS_PORT_TYPE_to_string(enum_val);
}

const char*
  jer2_arad_JER2_ARAD_CONNECTION_DIRECTION_to_string(
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION enum_val
  )
{
  return DNX_TMC_CONNECTION_DIRECTION_to_string(enum_val);
}

void
  jer2_arad_JER2_ARAD_INTERFACE_ID_print(
    DNX_SAND_IN JER2_ARAD_INTERFACE_ID if_ndx
  )
{
  LOG_CLI((BSL_META("Interface index: %s\n\r"),jer2_arad_JER2_ARAD_INTERFACE_ID_to_string(if_ndx)));
}

void
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_INFO_print(
    DNX_SAND_IN  JER2_ARAD_DEST_SYS_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_DEST_SYS_PORT_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_DEST_INFO_print(
    DNX_SAND_IN  JER2_ARAD_DEST_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_DEST_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  jer2_arad_JER2_ARAD_THRESH_WITH_HYST_INFO_print(
    DNX_SAND_IN JER2_ARAD_THRESH_WITH_HYST_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_THRESH_WITH_HYST_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_INFO_table_format_print(
    DNX_SAND_IN JER2_ARAD_DEST_SYS_PORT_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_DEST_SYS_PORT_INFO_table_format_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

const char*
  jer2_arad_JER2_ARAD_SWAP_MODE_to_string(
    DNX_SAND_IN  JER2_ARAD_SWAP_MODE enum_val
  )
{
  return DNX_TMC_SWAP_MODE_to_string(enum_val);
}

void
  jer2_arad_JER2_ARAD_SWAP_INFO_print(
    DNX_SAND_IN JER2_ARAD_SWAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_SWAP_INFO_print(info);
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /*JER2_ARAD_DEBUG_IS_LVL1*/
/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */
