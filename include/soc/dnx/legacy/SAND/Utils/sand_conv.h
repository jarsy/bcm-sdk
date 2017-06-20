/* $Id: sand_conv.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
* FILENAME:       dnx_sand_conv.h
*
* AUTHOR:         Dune (Y.P.)
*
* FILE DESCRIPTION:
*   DNX_SAND units conversion module
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/
#ifndef DNX_SAND_CONV_H_INCLUDED
/* { */
#define DNX_SAND_CONV_H_INCLUDED
#ifdef  __cplusplus
extern "C" {
#endif
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

#define DNX_SAND_IP_STR_SIZE (4*3+3+1+2)
/*****************************************************
*NAME:
* dnx_sand_kbits_per_sec_to_clocks
*DATE:
* 31/MAY/2005
*FUNCTION:
*  Convert a rate given in Kbits/sec units to an interval
* between 2 consecutive credits in device clocks.
* The conversion is done according to:
*                       Credit [Kbits] * Num_of_clocks_in_sec [clocks/sec]
*  Rate [Kbits/Sec] =   -------------------------------------------------------------
*                          interval_between_credits_in_clocks [clocks]
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN       uint32    rate  -
*       Rate in Kbits/sec
*    DNX_SAND_IN       DNX_SAND_CREDIT_SIZE  credit -
*       Credit size in bytes
*    DNX_SAND_IN       uint32    ticks_per_sec -
*       Number of ticks per second (device dependant)
*    DNX_SAND_OUT      uint32*    interval         -
*       Loaded with calculated interval between credits,
*       in device clocks.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   interval
*REMARKS:
* For rounding issues we don't actually divide the credit
* size by 1000 to get size in KBits. Instead we divide
* the ticks parameter (which is much bigger) by 1000.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_kbits_per_sec_to_clocks(
    DNX_SAND_IN       uint32    rate,     /* in Kbits/sec */
    DNX_SAND_IN       uint32     credit,   /* in Bytes */
    DNX_SAND_IN       uint32    ticks_per_sec,
    DNX_SAND_OUT      uint32*   interval  /* in device clocks */
  );

/*****************************************************
*NAME:
* dnx_sand_clocks_to_kbits_per_sec
*DATE:
* 31/MAY/2005
*FUNCTION:
*  Convert an interval between 2 consecutive credits given in
* device clocks to rate in Kbits/sec units.
* The conversion is done according to:
*                       Credit [Kbits] * Num_of_clocks_in_sec [clocks/sec]
*  Rate [Kbits/Sec] =   -------------------------------------------------------------
*                          interval_between_credits_in_clocks [clocks]
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN       uint32    interval         -
*       Interval between credits in device clocks.
*    DNX_SAND_IN       DNX_SAND_CREDIT_SIZE  credit -
*       Credit size in bytes
*    DNX_SAND_IN       uint32    ticks_per_sec -
*       Number of ticks per second (device dependant)
*    DNX_SAND_OUT      uint32*   rate  -
*       Loaded with the rate in Kbits/sec
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   interval
*REMARKS:
* For rounding issues we don't actually divide the credit
* size by 1000 to get size in KBits. Instead we divide
* the ticks parameter (which is much bigger) by 1000.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_clocks_to_kbits_per_sec(
    DNX_SAND_IN       uint32    interval, /* in device clocks */
    DNX_SAND_IN       uint32     credit,   /* in Bytes */
    DNX_SAND_IN       uint32    ticks_per_sec,
    DNX_SAND_OUT      uint32*   rate      /* in Kbits/sec */
  );



/*****************************************************
*NAME
*  dnx_sand_ip_addr_numeric_to_string
*TYPE: PROC
*DATE: 17/FEB/2002
*FUNCTION:
*  Convert an uint32 to Ascii representation
*  of 4 decimal numbers separated by dots.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32 ip_addr -
*      IP address in long representation:
*      First decimal number is MS byte.
*    DNX_SAND_IN  uint8 int  short_format -
*      Indicator
*      TRUE  - IP will be 10.0.0.1
*      FALSE - IP will be 010.000.000.001
*    DNX_SAND_OUT char *decimal_ip -
*      Pointer to a char array. To contain the
*      resultant string. Must be 16 bytes long
*      at least.
*  DNX_SAND_INDIRECT:
*    none.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    POINTER TO A CHAR ARRAY -
*      Points to the input buffer. Makes it easier
*      to use within 'printf'.
*  DNX_SAND_INDIRECT:
*    See dnx_sand_ip_addr_string_to_numeric.
*REMARKS:
*  Make sure input buffer is at least 16 bytes.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_ip_addr_numeric_to_string(
    DNX_SAND_IN uint32 ip_addr,
    DNX_SAND_IN uint8  short_format,
    DNX_SAND_OUT char   decimal_ip[DNX_SAND_IP_STR_SIZE]
  );


/*****************************************************
*NAME
*  dnx_sand_ip_addr_string_to_numeric
*TYPE: PROC
*DATE: 17/OCT/2007
*FUNCTION:
*  Convert ascii representation
*  of 4 decimal numbers separated by dots to an uint32 to .
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN char *decimal_ip -
*      Pointer to a char array. To contain the
*      resultant string. Must be 16 bytes long
*      at least.
*    DNX_SAND_OUT U32 *ip_addr -
*      IP address in long representation:
*      First decimal number is MS byte.
*  DNX_SAND_INDIRECT:
*    none.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    POINTER TO A CHAR ARRAY -
*      Points to the input buffer. Makes it easier
*      to use within 'printf'.
*  DNX_SAND_INDIRECT:
*    See dnx_sand_ip_addr_numeric_to_string.
*REMARKS:
*  Make sure input buffer is at least 16 bytes.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_ip_addr_string_to_numeric(
    DNX_SAND_IN char   decimal_ip[DNX_SAND_IP_STR_SIZE],
    DNX_SAND_OUT uint32 *ip_addr
  );


#ifdef  __cplusplus
}
#endif

/* } DNX_SAND_CONV_H_INCLUDED*/
#endif

