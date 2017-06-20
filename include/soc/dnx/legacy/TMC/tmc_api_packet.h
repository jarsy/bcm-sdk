/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_API_PACKET_INCLUDED__
/* { */
#define __DNX_TMC_API_PACKET_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* $Id$
 * Minimum size of sent packets in bytes
 */
/*
 * Minimum size of received packets in bytes
 */
/*
 * Maximum size of received/sent packets in bytes
 */
#define DNX_TMC_PKT_MAX_CPU_PACKET_BYTE_SIZE        (10 * 1024)


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

typedef enum
{
 /*
  *  This is the "normal" packet sending path:
  *  packet pass through the soc_petra starting from the ingress.
  *  this packet may contain
  *   - payload only (for RAW ports)
  *   - ITMH header and payload (for TM ports)
  *       - refer to soc_jer2_jer2_jer2_tmchpu_itmh_build in order to build  ITMH header.
  *   - Ethernet Header (with or without VLAN Tag) (for Ethernet ports).
  *       - refer to soc_jer2_jer2_jer2_tmcpp_ethernet_header_build in order to build the Eth Header.
  */
  DNX_TMC_PACKET_SEND_PATH_TYPE_INGRESS=0,
 /*
  *  This is the "reduced" packet sending path:
  *  packet pass through the soc_petra starting from the egress.
  *  this packet may contain
  *   - packets transmitted through this path are in fabric format (FTMH)
  *     and may be transmitted out to any port type (RAW, TM, Ethernet).
  */
  DNX_TMC_PACKET_SEND_PATH_TYPE_EGRESS=1,

  DNX_TMC_PACKET_SEND_NOF_PATH_TYPES=2
}DNX_TMC_PACKET_SEND_PATH_TYPE;

typedef enum
{
 /*
  * copy packets received by CPU to buffer starting from MSB byte
  * downward to LSB bytes
  */
  DNX_TMC_PKT_PACKET_RECV_MODE_MSB_TO_LSB=0,
 /*
  * copy packets received by CPU to buffer starting from LSB byte
  * upward to MSB bytes
  */
  DNX_TMC_PKT_PACKET_RECV_MODE_LSB_TO_MSB=1,

  DNX_TMC_PACKET_SEND_NOF_RECV_MODES=2
}DNX_TMC_PKT_PACKET_RECV_MODE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  10K byte Data buffer to carry on packet
   */
  uint8 data[DNX_TMC_PKT_MAX_CPU_PACKET_BYTE_SIZE];
  /*
   *  Number of bytes in packet
   */
  uint32 data_byte_size;

}DNX_TMC_PKT_PACKET_BUFFER;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
 /*
  *  Buffer to include packet to send. With an asynchronous
  *  interface, the buffer is sent starting from the buffer MSB.
  */
  DNX_TMC_PKT_PACKET_BUFFER packet;
 /*
  *  Packet sending path
  */
  DNX_TMC_PACKET_SEND_PATH_TYPE path_type;

}DNX_TMC_PKT_TX_PACKET_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
 /*
  *  Buffer to include packet to receive
  */
  DNX_TMC_PKT_PACKET_BUFFER packet;

}DNX_TMC_PKT_RX_PACKET_INFO;

typedef
  uint32
    (*DNX_TMC_PKT_PACKET_SEND_TRANSFER_PTR)(
      DNX_SAND_IN  int                    unit,
      DNX_SAND_IN  DNX_TMC_PKT_TX_PACKET_INFO     *tx_packet
   );

typedef
  uint32
    (*DNX_TMC_PKT_PACKET_RECV_TRANSFER_PTR)(
      DNX_SAND_IN  int                    unit,
      DNX_SAND_IN  uint32                     bytes_to_get,
      DNX_SAND_OUT DNX_TMC_PKT_RX_PACKET_INFO     *rx_packet
    );

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
  * Send function pointer.
  */
  DNX_TMC_PKT_PACKET_SEND_TRANSFER_PTR packet_send;

  /*
  * Receive function pointer.
  */
  DNX_TMC_PKT_PACKET_RECV_TRANSFER_PTR  packet_recv;
} DNX_TMC_PKT_PACKET_TRANSFER;

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
  );

void
  DNX_TMC_PKT_TX_PACKET_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PKT_TX_PACKET_INFO *info
  );

void
  DNX_TMC_PKT_RX_PACKET_INFO_clear(
    DNX_SAND_OUT DNX_TMC_PKT_RX_PACKET_INFO *info
  );

void
  DNX_TMC_PKT_PACKET_TRANSFER_clear(
    DNX_SAND_OUT DNX_TMC_PKT_PACKET_TRANSFER *info
  );

#if DNX_TMC_DEBUG_IS_LVL1
const char*
  DNX_TMC_PACKET_SEND_PATH_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_PACKET_SEND_PATH_TYPE enum_val
  );

const char*
  DNX_TMC_PKT_PACKET_RECV_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_PKT_PACKET_RECV_MODE enum_val
  );

void
  DNX_TMC_PKT_PACKET_BUFFER_print(
    DNX_SAND_IN DNX_TMC_PKT_PACKET_BUFFER *info,
    DNX_SAND_IN DNX_TMC_PKT_PACKET_RECV_MODE recv_to_msb
  );

void
  DNX_TMC_PKT_TX_PACKET_INFO_print(
    DNX_SAND_IN DNX_TMC_PKT_TX_PACKET_INFO *info,
    DNX_SAND_IN DNX_TMC_PKT_PACKET_RECV_MODE recv_to_msb
  );

void
  DNX_TMC_PKT_RX_PACKET_INFO_print(
    DNX_SAND_IN DNX_TMC_PKT_RX_PACKET_INFO *info,
    DNX_SAND_IN DNX_TMC_PKT_PACKET_RECV_MODE recv_to_msb
  );

void
  DNX_TMC_PKT_PACKET_TRANSFER_print(
    DNX_SAND_IN DNX_TMC_PKT_PACKET_TRANSFER *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_PACKET_INCLUDED__*/
#endif
