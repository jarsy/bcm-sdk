/* $Id: sand_pp_mac.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef  DNX_SAND_DRIVER_MAC_H
#define DNX_SAND_DRIVER_MAC_H

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>
#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>

/* } */
/*************
* DEFINES   *
*************/
/* { */


/* $Id: sand_pp_mac.h,v 1.5 Broadcom SDK $
 * number of characters in MAC address string.
 */
#define  DNX_SAND_PP_MAC_ADDRESS_STRING_LEN 12
/*
 * number of bytes in MAC address
 */
#define  DNX_SAND_PP_MAC_ADDRESS_NOF_U8 6
/*
 * number of bits in MAC address
 */
#define  DNX_SAND_PP_MAC_ADDRESS_NOF_BITS (DNX_SAND_PP_MAC_ADDRESS_NOF_U8 * DNX_SAND_NOF_BITS_IN_CHAR)
/*
 * number of longs in MAC address
 */
#define  DNX_SAND_PP_MAC_ADDRESS_NOF_UINT32S DNX_SAND_U64_NOF_UINT32S


/* } */

/*************
* MACROS    *
*************/
/* { */

#define DNX_SAND_PP_MAC_ADDRESS_IS_ZERO(_mac_)  \
    (((_mac_).address[0] | (_mac_).address[1] | (_mac_).address[2] | \
      (_mac_).address[3] | (_mac_).address[4] | (_mac_).address[5]) == 0) 

/* } */

/*************
* TYPE DEFS *
*************/
/* { */

/*
 *  MAC Address.mac[0] includes the lsb of the MAC address
 *  (network order).
 */
  typedef struct
  {
    DNX_SAND_MAGIC_NUM_VAR
    uint8  address[DNX_SAND_PP_MAC_ADDRESS_NOF_U8];
  }DNX_SAND_PP_MAC_ADDRESS;


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
*     dnx_sand_pp_mac_address_struct_to_long
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     Converts MAC address from struct to uint32
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac_add -
*    mac address as struct (6 chars)
*  DNX_SAND_OUT uint32            *mac_add_U32 -
*    mac address as 2 uint32s
* RETORNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_struct_to_long(
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac_add,
    DNX_SAND_OUT uint32            mac_add_U32[DNX_SAND_PP_MAC_ADDRESS_NOF_UINT32S]
  );

/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_long_to_struct
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     Converts MAC address from an array of two uint32s to a struct
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32            mac_add_long[DNX_SAND_PP_MAC_ADDRESS_NOF_UINT32S] -
*    mac address as long (2 uint32s)
*  DNX_SAND_OUT  DNX_SAND_PP_MAC_ADDRESS    *mac_add_struct -
*    mac address as struct (6 chars)
* RETORNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_long_to_struct(
    DNX_SAND_IN  uint32            mac_add_long[DNX_SAND_PP_MAC_ADDRESS_NOF_UINT32S],
    DNX_SAND_OUT DNX_SAND_PP_MAC_ADDRESS    *mac_add_struct
  );


/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_inc
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     calculate mac1 += 1
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_INOUT  DNX_SAND_PP_MAC_ADDRESS   *mac -
*    mac address to add to.
* RETORNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_inc(
    DNX_SAND_INOUT  DNX_SAND_PP_MAC_ADDRESS   *mac
  );

/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_are_equal
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     checks if the mac address are equal
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_INOUT  DNX_SAND_PP_MAC_ADDRESS   *mac1 -
*    mac address 1.
*  DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS      *mac2 -
*    mac address 2.
* RETORNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_are_equal(
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac1,
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac2,
    DNX_SAND_OUT  uint8              *equal
  );

/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_is_smaller
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     checks if the mac address are equal
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_INOUT  DNX_SAND_PP_MAC_ADDRESS   *mac1 -
*    mac address 1.
*  DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS      *mac2 -
*    mac address 2.
* RETORNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_is_smaller(
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac1,
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac2,
    DNX_SAND_OUT  uint8              *is_smaller
  );

/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_add
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     calculate mac1 += mac2
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_INOUT  DNX_SAND_PP_MAC_ADDRESS   *mac1 -
*    mac address to add to.
*  DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS      *mac2 -
*    mac address to add.
* RETORNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_add(
    DNX_SAND_INOUT  DNX_SAND_PP_MAC_ADDRESS   *mac1,
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS      *mac2
  );


/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_sub
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     Substract mac2 from mac1
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac1 -
*    mac address as struct (6 chars)
*  DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *mac2 -
*    mac address as struct (6 chars)
* RETORNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_sub(
    DNX_SAND_INOUT  DNX_SAND_PP_MAC_ADDRESS   *mac1,
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS      *mac2
  );


/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_string_parse
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     parse an input string of mac address.
*     For example input "01234500" is parsed to 00:00:01:23:45:00
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN char mac[DNX_SAND_PP_MAC_ADDRESS_STRING_LEN] -
*    mac address as string, array of chars.
*  DNX_SAND_OUT  DNX_SAND_PP_MAC_ADDRESS   mac_addr -
*    mac address as struct (6 chars)
* REMARKS:
*  - the input string doesn't include 0x
*  - the string is padded with initial zeros if needed.
*  - the lsb in the MAC array (mac[0]) is the right most value in the string.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_string_parse(
    DNX_SAND_IN char               mac_string[DNX_SAND_PP_MAC_ADDRESS_STRING_LEN],
    DNX_SAND_OUT  DNX_SAND_PP_MAC_ADDRESS   *mac_addr
  );


/*********************************************************************
* NAME:
*     dnx_sand_pp_mac_address_reverse
* TYPE:
*   PROC
* DATE:
*   Oct  13 2007
* FUNCTION:
*     Reverse the order of the mac address in bytes resolution.
*     Example: for in_mac_add 00:00:01:23:45:00 the out_mac_add is 00:45 23:01:00:00.
* INPUT:
*  DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *in_mac_add -
*    input mac address,
*  DNX_SAND_OUT  DNX_SAND_PP_MAC_ADDRESS   mac_addr -
*    reversed mac address
* REMARKS:
*  - the input string doesn't include 0x
*  - the string is padded with initial zeros if needed.
*  - the lsb in the MAC array (mac[0]) is the right most value in the string.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  dnx_sand_pp_mac_address_reverse(
    DNX_SAND_IN  DNX_SAND_PP_MAC_ADDRESS    *in_mac_add,
    DNX_SAND_OUT DNX_SAND_PP_MAC_ADDRESS    *out_mac_add
  );

/*
 * }
 */

void
  dnx_sand_SAND_PP_MAC_ADDRESS_clear(
    DNX_SAND_OUT DNX_SAND_PP_MAC_ADDRESS *mac_addr
  );

uint32
  DNX_SAND_PP_MAC_ADDRESS_verify(
    DNX_SAND_IN DNX_SAND_PP_MAC_ADDRESS *mac_addr
  );

#if DNX_SAND_DEBUG
/* { */

/*
 *  Formatted print of the MAC address (00:00:01:23:45:00)
 *  print msb first (mac_addr[5])
 */
void
  dnx_sand_SAND_PP_MAC_ADDRESS_print(
    DNX_SAND_IN DNX_SAND_PP_MAC_ADDRESS *mac_addr
  );

void
  dnx_sand_SAND_PP_MAC_ADDRESS_array_print(
    DNX_SAND_IN uint8       mac_address[DNX_SAND_PP_MAC_ADDRESS_NOF_U8]
  );


void
  dnx_sand_pp_mac_tests(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint8 silent
  );


/* } */
#endif


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif
