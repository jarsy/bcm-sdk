/* $Id: sand_trigger.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_TRIGGER_H_INCLUDED__
/* { */
#define __DNX_SAND_TRIGGER_H_INCLUDED__
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

/* $Id: sand_trigger.h,v 1.3 Broadcom SDK $
 * Most of the device triggers are in bit 0, and direct writing
 * can be done. In case that different bit (than ZERO) is used the
 * trigger mechanisim is using read/modify/write to trig it.
 */
#define DNX_SAND_GENERAL_TRIG_BIT   0


/*****************************************************
*NAME
* dnx_sand_trigger_verify_0
*TYPE:
*  PROC
*DATE:
*  13-Jan-03
*FUNCTION:
*  verify that the dnx_sand-trigger is 0
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int   unit -
*       Device identifier to verify.
*    DNX_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    DNX_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    DNX_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than DNX_SAND_BIT-0 triggers.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*         DNX_SAND_OK          -- trigger is 0
*         DNX_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_trigger_verify_0(
    DNX_SAND_IN int   unit,
    DNX_SAND_IN uint32  offset,
    DNX_SAND_IN uint32  timeout, /* in nano seconds */
    DNX_SAND_IN uint32  trigger_bit
  );

/*****************************************************
*NAME
* dnx_sand_trigger_assert_1
*TYPE:
*  PROC
*DATE:
*  13-Jan-03
*FUNCTION:
*  Set the trigger to 1.
*  If given timeout is bigger than zero --
*  wait for trigger to become zero.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int   unit -
*       Device identifier to trigger.
*    DNX_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    DNX_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    DNX_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than DNX_SAND_BIT-0 triggers.
*       Use read/modify/write to trig it - not just write.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*         DNX_SAND_OK          -- trigger was set to 1
*         DNX_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_trigger_assert_1(
    DNX_SAND_IN int   unit,
    DNX_SAND_IN uint32  offset,
    DNX_SAND_IN uint32  timeout, /* in nano seconds */
    DNX_SAND_IN uint32  trigger_bit
  );


/*****************************************************
*NAME
* dnx_sand_trigger_verify_0_by_base
*TYPE:
*  PROC
*DATE:
*  01-SEP-07
*FUNCTION:
*  verify that the dnx_sand-trigger is 0
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    DNX_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    DNX_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than DNX_SAND_BIT-0 triggers.
*     DNX_SAND_IN uint32  *device_base_address -
*       the base address from which the offset is
*       counted for the physical read of the trigger.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*         DNX_SAND_OK          -- trigger is 0
*         DNX_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_trigger_verify_0_by_base(
    DNX_SAND_IN uint32  offset,
    DNX_SAND_IN uint32  timeout, /* in nano seconds */
    DNX_SAND_IN uint32  trigger_bit,
    DNX_SAND_IN uint32  *device_base_address
  );

/*****************************************************
*NAME
* dnx_sand_trigger_assert_1_by_base
*TYPE:
*  PROC
*DATE:
*  01-SEP-07
*FUNCTION:
*  Set the trigger to 1.
*  If given timeout is bigger than zero --
*  wait for trigger to become zero.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    DNX_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    DNX_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than DNX_SAND_BIT-0 triggers.
*       Use read/modify/write to trig it - not just write.
*    DNX_SAND_IN uint32  *device_base_address -
*       the base address from which the offset is
*       counted for the physical read & write of the trigger.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*         DNX_SAND_OK          -- trigger was set to 1
*         DNX_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_trigger_assert_1_by_base(
    DNX_SAND_IN     uint32  offset,
    DNX_SAND_IN     uint32  timeout, /* in nano seconds */
    DNX_SAND_IN     uint32  trigger_bit,
    DNX_SAND_INOUT  uint32  *device_base_address
  );

#ifdef  __cplusplus
}
#endif

/* } __DNX_SAND_TRIGGER_H_INCLUDED__*/
#endif
