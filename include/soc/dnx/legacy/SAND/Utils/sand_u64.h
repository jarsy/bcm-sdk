/* $Id: sand_u64.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_U64_H_INCLUDED__
/* { */
#define __DNX_SAND_U64_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

#define DNX_SAND_U64_NOF_UINT32S (2)
/* $Id: sand_u64.h,v 1.5 Broadcom SDK $
 * 64 bit unsigned number
 */
typedef struct
{
  /*
   * Low  32 bits - arr[0]
   * High 32 bits - arr[1]
   */
  uint32 arr[DNX_SAND_U64_NOF_UINT32S];

} DNX_SAND_U64;


/*****************************************************
*NAME
* soc_long_to_u64
*TYPE:
*  PROC
*DATE:
*  8-Feb-15
*FUNCTION:
*  Creates a u64 out of a uint32
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32 u32 -
*      Source unit32
*    DNX_SAND_OUT DNX_SAND_U64* u64 -
*      Output structre
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    void
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_long_to_u64(
    DNX_SAND_IN  uint32      		u32,
    DNX_SAND_OUT DNX_SAND_U64* 		u64
  );

/*****************************************************
*NAME
* dnx_sand_u64_copy
*TYPE:
*  PROC
*DATE:
*  8-Feb-15
*FUNCTION:
*  Copy one u64 to the other, return FALSE in case of an error
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN DNX_SAND_U64* src -
*      Structure to copy
*    DNX_SAND_OUT DNX_SAND_U64* dst -
*      Output structre
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    void
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_copy(
    DNX_SAND_IN  DNX_SAND_U64*      src,
    DNX_SAND_OUT DNX_SAND_U64* 		dst
  );
/*****************************************************
*NAME
* dnx_sand_u64_clear
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Clears all fields in the structure.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_U64* u64 -
*      Structure to clear.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    void
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  dnx_sand_u64_clear(
    DNX_SAND_INOUT DNX_SAND_U64* u64
  );

/*****************************************************
*NAME
* dnx_sand_u64_to_long
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Convert 'u64' from 64 bit number to 'ulong' 32 bit number.
*  If 'u64' > 0xFFFF_FFFF than 'ulong' is 0xFFFF_FFFF
*  and overflowed indicator is retruned.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_U64*      u64 -
*      Structure to convert.
*    DNX_SAND_OUT uint32* ulong -
*      the number after conversion.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*      Can overflowed
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_to_long(
    DNX_SAND_IN  DNX_SAND_U64*      u64,
    DNX_SAND_OUT uint32* ulong
  );


/*****************************************************
*NAME
* dnx_sand_u64_multiply_longs
*TYPE:
*  PROC
*DATE:
*  03/03/2004
*FUNCTION:
*  Multiply 2 longs and load result into DNX_SAND_U64.
*  result = x * y.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  uint32 x -
*      value to multipy
*    DNX_SAND_IN  uint32 y -
*      value to multipy
*    DNX_SAND_OUT DNX_SAND_U64*     result -
*      Loaded with result
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  dnx_sand_u64_multiply_longs(
    DNX_SAND_IN  uint32 x,
    DNX_SAND_IN  uint32 y,
    DNX_SAND_OUT DNX_SAND_U64*     result
  );


/*****************************************************
*NAME
* dnx_sand_u64_devide_u64_long
*TYPE:
*  PROC
*DATE:
*  03/03/2004
*FUNCTION:
*  Calculates quotient result = x div v
*  Returns remainder r = x mod v
*  where resultq, u are multiprecision integers of 64 bit each
*  and d, v are long digits.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_U64*     x -
*      value to device
*    DNX_SAND_IN  uint32 y  -
*      value to device
*    DNX_SAND_OUT uint32 result -
*      Loaded with result
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*      remainder
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_devide_u64_long(
    DNX_SAND_IN  DNX_SAND_U64*     x,
    DNX_SAND_IN  uint32 y,
    DNX_SAND_OUT DNX_SAND_U64*     result
  );

/*****************************************************
*NAME
* dnx_sand_u64_add_long
*TYPE:
*  PROC
*DATE:
*  03/03/2004
*FUNCTION:
*  Calculates x = x + y
*  Returns carryif overflowed
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_U64*     x -
*      value to add
*    DNX_SAND_IN     uint32 y -
*      value to add
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*      carry.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_add_long(
    DNX_SAND_INOUT  DNX_SAND_U64*     x,
    DNX_SAND_IN     uint32 y
  );

/*****************************************************
*NAME
* dnx_sand_u64_add_u64
*TYPE:
*  PROC
*DATE:
*  03/03/2004
*FUNCTION:
*  Calculates result = x + y
*  Returns carryif overflowed
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_U64*     x -
*      value to add
*    DNX_SAND_IN  uint32 y  -
*      value to add
*    DNX_SAND_OUT uint32 result -
*      Loaded with result
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*      carry.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_add_u64(
    DNX_SAND_INOUT  DNX_SAND_U64*     x,
    DNX_SAND_IN     DNX_SAND_U64*     y
  );


/*****************************************************
*NAME
* dnx_sand_u64_subtract_u64
*TYPE:
*  PROC
*DATE:
*  03/03/2004
*FUNCTION:
*  Calculates x = x - y
*  Returns FALSE if y > x
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT  DNX_SAND_U64*     x -
*      value to subtract from
*    DNX_SAND_IN     DNX_SAND_U64*     y -
*      value to subtract
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    TRUE x > y.
*    FALSE: x < y.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  If x < y; x<-0, and return FALSE.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_subtract_u64(
    DNX_SAND_INOUT  DNX_SAND_U64*     x,
    DNX_SAND_IN     DNX_SAND_U64*     y
  );

/*****************************************************
*NAME
* dnx_sand_u64_is_bigger
*TYPE:
*  PROC
*DATE:
*  01/07/2004
*FUNCTION:
*  Calculates (x > y)
*  Returns TRUE or FALSE
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_U64*     x -
*      value to compare
*    DNX_SAND_IN  DNX_SAND_U64*     y  -
*      value to compare
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*      Indicator.
*      TRUE iff (x > y).
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_is_bigger(
    DNX_SAND_IN     DNX_SAND_U64*     x,
    DNX_SAND_IN     DNX_SAND_U64*     y
  );

/*****************************************************
*NAME
* dnx_sand_u64_is_zero
*TYPE:
*  PROC
*DATE:
*  01/07/2004
*FUNCTION:
*  Calculates (x == y)
*  Returns TRUE or FALSE
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_U64*     x -
*      value to compare
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*      Indicator.
*      TRUE iff (x == 0).
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_is_zero(
    DNX_SAND_IN     DNX_SAND_U64*     x
  );

/*****************************************************
*NAME
* dnx_sand_u64_are_equal
*TYPE:
*  PROC
*DATE:
*  01/07/2004
*FUNCTION:
*  Calculates (x == y)
*  Returns TRUE or FALSE
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_U64*     x -
*      value to compare
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_U64*     y -
*      value to compare
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*      Indicator.
*      TRUE iff (x == y).
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_are_equal(
    DNX_SAND_IN     DNX_SAND_U64*     x,
    DNX_SAND_IN     DNX_SAND_U64*     y
  );

/*****************************************************
*NAME
*  dnx_sand_u64_log2_round_up
*TYPE:
*  PROC
*DATE:
*  16-Jul-07
*FUNCTION:
*  Given number - x.
*  Return the integer round up of log2(x)
*CALLING SEQUENCE:
*  dnx_sand_log2_round_up(x)
*INPUT:
*  DNX_SAND_DIRECT:
*    const uint32 x -
*     Number to log.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*       Log with base 2. Round up
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  Examples:
*    dnx_sand_log2_round_up(0) = 0 -- definition
*    dnx_sand_log2_round_up(1) = 0
*    dnx_sand_log2_round_up(2) = 1
*    dnx_sand_log2_round_up(3) = 2
*    dnx_sand_log2_round_up(4) = 2
*    dnx_sand_log2_round_up(5) = 3
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_log2_round_up(
    DNX_SAND_IN DNX_SAND_U64 *x
  );

/*****************************************************
*NAME
* dnx_sand_u64_shift_left
*TYPE:
*  PROC
*DATE:
*  03/03/2004
*FUNCTION:
*  Computes a = b << x.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_OUT DNX_SAND_U64*    a -
*      loaded with result
*    DNX_SAND_IN  DNX_SAND_U64*    b -
*      number to shift
*    DNX_SAND_IN  uint32 result -
*      Loaded with result
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 - the carry.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_shift_left(
    DNX_SAND_OUT DNX_SAND_U64*    a,
    DNX_SAND_IN  DNX_SAND_U64*    b,
    DNX_SAND_IN  uint32 x
  );

#if DNX_SAND_DEBUG
/* { */
/*
 */

/*****************************************************
*NAME
* dnx_sand_u64_print
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Print service to DNX_SAND_U64.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_U64* u64 -
*      Structure to print.
*    DNX_SAND_IN uint32 print_type -
*      0 - Decimal format.
*      1 - Hexadecimal format.
*    uint32 short_format -
*      Short or long print format.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    void
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  dnx_sand_u64_print(
    DNX_SAND_IN DNX_SAND_U64*    u64,
    DNX_SAND_IN uint32 print_type,
    DNX_SAND_IN uint32 short_format
  );


/*****************************************************
*NAME
* dnx_sand_u64_test
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Test function of DNX_SAND_U64.
*  View prints and return value to see pass/fail
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*       TRUE  - PASS.
*       FALSE - FAIL
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_u64_test(uint32 silent);

/*
 * }
 */
#endif

#ifdef  __cplusplus
}
#endif


/* } __DNX_SAND_U64_H_INCLUDED__*/
#endif
