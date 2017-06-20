/* $Id: soc_tmcapi_header_parsing_utils.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_HEADER_PARSING_UTILS_INCLUDED__
/* { */
#define __SOC_TMC_HEADER_PARSING_UTILS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/TMC/tmc_api_general.h>

#include <soc/dpp/TMC/tmc_api_ports.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* $Id: soc_tmcapi_header_parsing_utils.h,v 1.4 Broadcom SDK $
 *  Number of longs (32 bit) per FTMH.
 */
#define SOC_TMC_HPU_FTMH_SIZE_UINT32S         2
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
   *  Ingress TM header - forwarding or ingress shaping. Bits
   *  0 - 31 are valid.
   */
  uint32 base;
  /*
   *  ITMH Source Port extension. Bits 0 - 15 are valid.
   */
  uint32 extention_src_port;
}SOC_TMC_HPU_ITMH_HDR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Packet size in bytes. Range: 0-16K.
   */
  uint32 packet_size;
  /*
   *  Traffic class. Range: 0-7
   */
  uint32 tr_cls;
  /*
   *  This field identifies the system level source LAG or
   *  physical system port at the ingress (see DEST_SYS_PORT
   *  field description in the DS). This field is either
   *  copied from the Incoming TM Header if a source extension
   *  is present, or if not present, then it is generated from
   *  a configuration of the Incoming FAP Port
   */
  SOC_TMC_DEST_SYS_PORT_INFO src_sys_port;
  /*
   *  Outgoing FAP port on destination device. Range: 0-79.
   */
  uint32 ofp;
  /*
   *  Drop precedence. Range: 0-3.
   */
  uint32 dp;
  /*
   *  Signature key of the Queue. Range: 0-3.
   */
  uint32 signature;
  /*
   *  Indicates whether a packet-processor-header is above.
   */
  uint8 pp_header_present;
  /*
   *  Indicates whether port outbound mirroring is enabled
   *  /disabled.
   */
  uint8 out_mirror_dis;
  /*
   *  Indicates whether to filter the packet at the egress
   *  when it arrives with source system-port-id the same as
   *  destination system-port-id
   */
  uint8 exclude_src;
  /*
   *  Indicates whether packet is system-multicast.
   */
  uint8 multicast;
}SOC_TMC_HPU_FTMH_BASE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Indicates whether the extension exists. If this value is
   *  FALSE the next field in this structure is meaningless.
   */
  uint8 enable;
  /*
   *  If previous field is TRUE then, if packet is ingress
   *  replicated, then this is the OUT_LIF, as derived from
   *  the multicast linked list; else, it is a copy of the LSB
   *  of the FWD_ACTION_DESTINATION_INFO.
   */
  uint32 outlif;
}SOC_TMC_HPU_FTMH_EXT_OUTLIF;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The FTMH base fields information.
   */
  SOC_TMC_HPU_FTMH_BASE base;
  /*
   *  The FTMH OutLif extension info.
   */
  SOC_TMC_HPU_FTMH_EXT_OUTLIF extension;
}SOC_TMC_HPU_FTMH;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Fabric TM header. Bits 0 - 47 are valid
   */
  uint32 base[SOC_TMC_HPU_FTMH_SIZE_UINT32S];
  /*
   *  FTMH Outlif extension. Bits 0 - 15 are valid.
   */
  uint32 extension_outlif;
}SOC_TMC_HPU_FTMH_HDR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Indicates whether a packet-processor-header is above.
   */
  uint8 pp_header_present;
  /*
   *  Signature key of the Queue. Range: 0-3.
   */
  uint32 signature;
  /*
   *  Indicates whether packet is system-multicast.
   */
  uint8 multicast;
  /*
   *  Drop precedence. Range: 0-3.
   */
  uint32 dp;
  /*
   *  Packet size in bytes. Range: 0-16K.
   */
  uint32 packet_size;
  /*
   *  Traffic class. Range: 0-7
   */
  uint32 tr_cls;
  /*
   *  Normally, the outlif value from the FTMH. Otherwise,
   *  configurable
   */
  uint32 outlif;
}SOC_TMC_HPU_OTMH_BASE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Indicates whether the extension exists. If this value is
   *  FALSE the next field in this structure is meaningless.
   */
  uint8 enable;
  /*
   *  This field identifies the system level source LAG or
   *  physical system port at the ingress (see DEST_SYS_PORT
   *  field description in the DS). This field is either
   *  copied from the Incoming TM Header if a source extension
   *  is present, or if not present, then it is generated from
   *  a configuration of the Incoming FAP Port
   */
  SOC_TMC_DEST_SYS_PORT_INFO src_sys_port;
}SOC_TMC_HPU_OTMH_EXT_SRC_PORT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Indicates whether the extension exists. If this value is
   *  FALSE the next field in this structure is meaningless.
   */
  uint8 enable;
  /*
   *  This field identifies the system level source LAG or
   *  physical system port at the ingress (see DEST_SYS_PORT
   *  field description in the DS). This field is either
   *  copied from the Incoming TM Header if a source extension
   *  is present, or if not present, then it is generated from
   *  a configuration of the Incoming FAP Port
   */
  SOC_TMC_DEST_SYS_PORT_INFO dest_sys_port;
}SOC_TMC_HPU_OTMH_EXT_DEST_PORT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  These fields indicate whether the extensions of the
   *  packets exist and their values. This represents the OTMH
   *  oulif extension.
   */
  SOC_TMC_HPU_FTMH_EXT_OUTLIF outlif;
  /*
   *  This represents the OTMH source sys-port extension.
   */
  SOC_TMC_HPU_OTMH_EXT_SRC_PORT src;
  /*
   *  This represents the OTMH destination sys-port extension.
   */
  SOC_TMC_HPU_OTMH_EXT_DEST_PORT dest;
}SOC_TMC_HPU_OTMH_EXTENSIONS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The FTMH base fields information.
   */
  SOC_TMC_HPU_OTMH_BASE base;
  /*
   *  The FTMH OutLif extension info.
   */
  SOC_TMC_HPU_OTMH_EXTENSIONS extension;
}SOC_TMC_HPU_OTMH;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Outgoing TM header. Bits 0 - 15 are valid
   */
  uint32 base;
  /*
   *  OTMH Out-LIF extention. Bits 0 - 15 are valid
   */
  uint32 extension_outlif;
  /*
   *  OTMH Source Port extention. Bits 0 - 15 are valid
   */
  uint32 extension_src_port;
  /*
   *  OTMH Destination Port extention. Bits 0 - 15 are valid
   */
  uint32 extension_dest_port;
}SOC_TMC_HPU_OTMH_HDR;

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
  SOC_TMC_HPU_ITMH_HDR_clear(
    SOC_SAND_OUT SOC_TMC_HPU_ITMH_HDR *info
  );

void
  SOC_TMC_HPU_FTMH_BASE_clear(
    SOC_SAND_OUT SOC_TMC_HPU_FTMH_BASE *info
  );

void
  SOC_TMC_HPU_FTMH_EXT_OUTLIF_clear(
    SOC_SAND_OUT SOC_TMC_HPU_FTMH_EXT_OUTLIF *info
  );

void
  SOC_TMC_HPU_FTMH_clear(
    SOC_SAND_OUT SOC_TMC_HPU_FTMH *info
  );

void
  SOC_TMC_HPU_FTMH_HDR_clear(
    SOC_SAND_OUT SOC_TMC_HPU_FTMH_HDR *info
  );

void
  SOC_TMC_HPU_OTMH_BASE_clear(
    SOC_SAND_OUT SOC_TMC_HPU_OTMH_BASE *info
  );

void
  SOC_TMC_HPU_OTMH_EXT_SRC_PORT_clear(
    SOC_SAND_OUT SOC_TMC_HPU_OTMH_EXT_SRC_PORT *info
  );

void
  SOC_TMC_HPU_OTMH_EXT_DEST_PORT_clear(
    SOC_SAND_OUT SOC_TMC_HPU_OTMH_EXT_DEST_PORT *info
  );

void
  SOC_TMC_HPU_OTMH_EXTENSIONS_clear(
    SOC_SAND_OUT SOC_TMC_HPU_OTMH_EXTENSIONS *info
  );

void
  SOC_TMC_HPU_OTMH_clear(
    SOC_SAND_OUT SOC_TMC_HPU_OTMH *info
  );

void
  SOC_TMC_HPU_OTMH_HDR_clear(
    SOC_SAND_OUT SOC_TMC_HPU_OTMH_HDR *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

void
  SOC_TMC_HPU_ITMH_HDR_print(
    SOC_SAND_IN SOC_TMC_HPU_ITMH_HDR *info
  );

void
  SOC_TMC_HPU_FTMH_BASE_print(
    SOC_SAND_IN SOC_TMC_HPU_FTMH_BASE *info
  );

void
  SOC_TMC_HPU_FTMH_EXT_OUTLIF_print(
    SOC_SAND_IN SOC_TMC_HPU_FTMH_EXT_OUTLIF *info
  );

void
  SOC_TMC_HPU_FTMH_print(
    SOC_SAND_IN SOC_TMC_HPU_FTMH *info
  );

void
  SOC_TMC_HPU_FTMH_HDR_print(
    SOC_SAND_IN SOC_TMC_HPU_FTMH_HDR *info
  );

void
  SOC_TMC_HPU_OTMH_BASE_print(
    SOC_SAND_IN SOC_TMC_HPU_OTMH_BASE *info
  );

void
  SOC_TMC_HPU_OTMH_EXT_SRC_PORT_print(
    SOC_SAND_IN SOC_TMC_HPU_OTMH_EXT_SRC_PORT *info
  );

void
  SOC_TMC_HPU_OTMH_EXT_DEST_PORT_print(
    SOC_SAND_IN SOC_TMC_HPU_OTMH_EXT_DEST_PORT *info
  );

void
  SOC_TMC_HPU_OTMH_EXTENSIONS_print(
    SOC_SAND_IN SOC_TMC_HPU_OTMH_EXTENSIONS *info
  );

void
  SOC_TMC_HPU_OTMH_print(
    SOC_SAND_IN SOC_TMC_HPU_OTMH *info
  );

void
  SOC_TMC_HPU_OTMH_HDR_print(
    SOC_SAND_IN SOC_TMC_HPU_OTMH_HDR *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_HEADER_PARSING_UTILS_INCLUDED__*/
#endif
