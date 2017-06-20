/* $Id: jer2_tmc_api_packet.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_tmc/src/soc_jer2_tmcapi_packet.c
*
* MODULE PREFIX:  soc_jer2_tmcpkt
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

/*************
 * INCLUDES  *
 *************/
/* { */



#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_packet.h>

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

void
  DNX_TMC_PKT_PACKET_BUFFER_clear(
    DNX_SAND_OUT DNX_TMC_PKT_PACKET_BUFFER *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PKT_PACKET_BUFFER));
  for (ind = 0; ind < DNX_TMC_PKT_MAX_CPU_PACKET_BYTE_SIZE; ++ind)
  {
    info->data[ind] = 0;
  }
  info->data_byte_size = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PKT_TX_PACKET_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PKT_TX_PACKET_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PKT_TX_PACKET_INFO));

  DNX_TMC_PKT_PACKET_BUFFER_clear(&(info->packet));
  info->path_type = DNX_TMC_PACKET_SEND_NOF_PATH_TYPES;

  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PKT_RX_PACKET_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PKT_RX_PACKET_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PKT_RX_PACKET_INFO));

  DNX_TMC_PKT_PACKET_BUFFER_clear(&(info->packet));

  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PKT_PACKET_TRANSFER_clear(
    DNX_SAND_OUT DNX_TMC_PKT_PACKET_TRANSFER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_PKT_PACKET_TRANSFER));
  info->packet_send = NULL;
  info->packet_recv = NULL;

  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_PACKET_SEND_PATH_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_PACKET_SEND_PATH_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PACKET_SEND_PATH_TYPE_INGRESS:
    str = "ingress";
  break;
  case DNX_TMC_PACKET_SEND_PATH_TYPE_EGRESS:
    str = "egress";
  break;
  default:
    str = "Unknown";
  }
  return str;
}

const char*
  DNX_TMC_PKT_PACKET_RECV_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_PKT_PACKET_RECV_MODE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_PKT_PACKET_RECV_MODE_MSB_TO_LSB:
    str = "msb_to_lsb";
  break;
  case DNX_TMC_PKT_PACKET_RECV_MODE_LSB_TO_MSB:
    str = "lsb_to_msb";
  break;
  default:
    str = " Unknown";
  }
  return str;
}
void
  DNX_TMC_PKT_PACKET_BUFFER_print(
    DNX_SAND_IN DNX_TMC_PKT_PACKET_BUFFER *info,
    DNX_SAND_IN DNX_TMC_PKT_PACKET_RECV_MODE recv_to_msb
  )
{
  uint32 ind=0;
  char next_char;
  static int32 bytes_per_row = 10;


  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  if (info->data_byte_size > DNX_TMC_PKT_MAX_CPU_PACKET_BYTE_SIZE)
  {
    LOG_CLI((BSL_META_U(unit,
                        "Invalid data size (%u)\n\r"
                        "Maximal allowed is %u"),
             info->data_byte_size,
             DNX_TMC_PKT_MAX_CPU_PACKET_BYTE_SIZE
             ));
    DNX_TMC_DO_NOTHING_AND_EXIT;
  }

  for (ind=0; ind < info->data_byte_size; ++ind)
  {
    if (recv_to_msb == DNX_TMC_PKT_PACKET_RECV_MODE_MSB_TO_LSB)
    {
      next_char = info->data[DNX_TMC_PKT_MAX_CPU_PACKET_BYTE_SIZE - ind - 1];
    }
    else
    {
      next_char = info->data[ind];
    }

    if ((ind % bytes_per_row) == 0)
    {
      LOG_CLI((BSL_META_U(unit,
                          "Data[%04u-%04u]:   "), ind, DNX_SAND_MIN((ind+bytes_per_row-1), (info->data_byte_size-1))));
    }

    LOG_CLI((BSL_META_U(unit,
                        "%02x "), next_char));

    if ((((ind+1) % bytes_per_row) == 0) || ((ind+1) == info->data_byte_size))
    {
      LOG_CLI((BSL_META_U(unit,
                          "\n\r")));
    }
  }

  LOG_CLI((BSL_META_U(unit,
                      "Data_byte_size: %u\n\r"),info->data_byte_size));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PKT_TX_PACKET_INFO_print(
    DNX_SAND_IN DNX_TMC_PKT_TX_PACKET_INFO *info,
    DNX_SAND_IN DNX_TMC_PKT_PACKET_RECV_MODE recv_to_msb
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_PKT_PACKET_BUFFER_print(&(info->packet), recv_to_msb);
  LOG_CLI((BSL_META_U(unit,
                      "Path type:     %s \n\r"),
           DNX_TMC_PACKET_SEND_PATH_TYPE_to_string(info->path_type)
           ));

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PKT_RX_PACKET_INFO_print(
    DNX_SAND_IN DNX_TMC_PKT_RX_PACKET_INFO *info,
    DNX_SAND_IN DNX_TMC_PKT_PACKET_RECV_MODE recv_to_msb
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_TMC_PKT_PACKET_BUFFER_print(&(info->packet), recv_to_msb);

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_PKT_PACKET_TRANSFER_print(
    DNX_SAND_IN DNX_TMC_PKT_PACKET_TRANSFER *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

