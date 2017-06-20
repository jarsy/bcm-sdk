/* $Id: sand_64cnt.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_64CNT_H_INCLUDED__
/* { */
#define __DNX_SAND_64CNT_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>

/* $Id: sand_64cnt.h,v 1.4 Broadcom SDK $
 * 64 bit counter
 */
typedef struct
{
  /*
   * 64 bit number;
   */
  DNX_SAND_U64      u64;

  /*
   * Flag. If non-zero then 'high', specified above,
   * has overflown since it was last cleared.
   * Note:
   * Flag should be cleared after having been reported.
   */
  uint32   overflowed ;

} DNX_SAND_64CNT;


/*****************************************************
*NAME
* dnx_sand_64cnt_clear
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Clears all fields in the counter pointed by 'counter'.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_64CNT* counter -
*      Counter to clear.
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
  dnx_sand_64cnt_clear(
    DNX_SAND_INOUT DNX_SAND_64CNT* counter
  );

/*****************************************************
*NAME
* dnx_sand_64cnt_clear_ov
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Clears Over-Flow field in the counter pointed by 'counter'.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_64CNT* counter -
*      Counter to clear.
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
  dnx_sand_64cnt_clear_ov(
    DNX_SAND_INOUT DNX_SAND_64CNT* counter
  );

/*****************************************************
*NAME
* dnx_sand_64cnt_add_long
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Add uint32 value to counter.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_64CNT* counter -
*      Counter to add.
*    uint32 value_to_add -
*      32 bit value to add to the counter.
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
  dnx_sand_64cnt_add_long(
    DNX_SAND_INOUT DNX_SAND_64CNT*   counter,
    DNX_SAND_IN    uint32 value_to_add
  );

/*****************************************************
*NAME
* dnx_sand_64cnt_add_64cnt
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Add long value to counter.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_64CNT* counter -
*      Counter to add.
*    uint32 value_to_add -
*      64 bit counter to add to the counter.
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
  dnx_sand_64cnt_add_64cnt(
    DNX_SAND_INOUT DNX_SAND_64CNT* counter,
    DNX_SAND_IN    DNX_SAND_64CNT* value_to_add
  );

#if DNX_SAND_DEBUG
/* { */
/*
 */

/*****************************************************
*NAME
* dnx_sand_64cnt_print
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Print service to DNX_SAND_64CNT.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_64CNT* counter -
*      Counter to print.
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
  dnx_sand_64cnt_print(
    DNX_SAND_IN DNX_SAND_64CNT*  counter,
    DNX_SAND_IN uint32 short_format
  );

/*****************************************************
*NAME
* dnx_sand_64cnt_test
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Test function of DNX_SAND_64CNT.
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
  dnx_sand_64cnt_test(uint32 silent);

/*
 * }
 */
#endif


#ifdef  __cplusplus
}
#endif


/* } __DNX_SAND_64CNT_H_INCLUDED__*/
#endif
