/* $Id: sand_64cnt.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_SAND_64CNT_H_INCLUDED__
/* { */
#define __SOC_SAND_64CNT_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/SAND/Utils/sand_u64.h>

/* $Id: sand_64cnt.h,v 1.4 Broadcom SDK $
 * 64 bit counter
 */
typedef struct
{
  /*
   * 64 bit number;
   */
  SOC_SAND_U64      u64;

  /*
   * Flag. If non-zero then 'high', specified above,
   * has overflown since it was last cleared.
   * Note:
   * Flag should be cleared after having been reported.
   */
  uint32   overflowed ;

} SOC_SAND_64CNT;


/*****************************************************
*NAME
* soc_sand_64cnt_clear
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Clears all fields in the counter pointed by 'counter'.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_64CNT* counter -
*      Counter to clear.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    void
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  soc_sand_64cnt_clear(
    SOC_SAND_INOUT SOC_SAND_64CNT* counter
  );

/*****************************************************
*NAME
* soc_sand_64cnt_clear_ov
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Clears Over-Flow field in the counter pointed by 'counter'.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_64CNT* counter -
*      Counter to clear.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    void
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  soc_sand_64cnt_clear_ov(
    SOC_SAND_INOUT SOC_SAND_64CNT* counter
  );

/*****************************************************
*NAME
* soc_sand_64cnt_add_long
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Add uint32 value to counter.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_64CNT* counter -
*      Counter to add.
*    uint32 value_to_add -
*      32 bit value to add to the counter.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    void
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  soc_sand_64cnt_add_long(
    SOC_SAND_INOUT SOC_SAND_64CNT*   counter,
    SOC_SAND_IN    uint32 value_to_add
  );

/*****************************************************
*NAME
* soc_sand_64cnt_add_64cnt
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Add long value to counter.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_64CNT* counter -
*      Counter to add.
*    uint32 value_to_add -
*      64 bit counter to add to the counter.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    void
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  soc_sand_64cnt_add_64cnt(
    SOC_SAND_INOUT SOC_SAND_64CNT* counter,
    SOC_SAND_IN    SOC_SAND_64CNT* value_to_add
  );

#if SOC_SAND_DEBUG
/* { */
/*
 */

/*****************************************************
*NAME
* soc_sand_64cnt_print
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Print service to SOC_SAND_64CNT.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_64CNT* counter -
*      Counter to print.
*    uint32 short_format -
*      Short or long print format.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    void
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  soc_sand_64cnt_print(
    SOC_SAND_IN SOC_SAND_64CNT*  counter,
    SOC_SAND_IN uint32 short_format
  );

/*****************************************************
*NAME
* soc_sand_64cnt_test
*TYPE:
*  PROC
*DATE:
*  9-Sep-03
*FUNCTION:
*  Test function of SOC_SAND_64CNT.
*  View prints and return value to see pass/fail
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32
*       TRUE  - PASS.
*       FALSE - FAIL
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  soc_sand_64cnt_test(uint32 silent);

/*
 * }
 */
#endif


#ifdef  __cplusplus
}
#endif


/* } __SOC_SAND_64CNT_H_INCLUDED__*/
#endif
