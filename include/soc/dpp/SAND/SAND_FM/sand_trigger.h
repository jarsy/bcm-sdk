/* $Id: sand_trigger.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_SAND_TRIGGER_H_INCLUDED__
/* { */
#define __SOC_SAND_TRIGGER_H_INCLUDED__
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dpp/SAND/Utils/sand_framework.h>

/* $Id: sand_trigger.h,v 1.3 Broadcom SDK $
 * Most of the device triggers are in bit 0, and direct writing
 * can be done. In case that different bit (than ZERO) is used the
 * trigger mechanisim is using read/modify/write to trig it.
 */
#define SOC_SAND_GENERAL_TRIG_BIT   0


/*****************************************************
*NAME
* soc_sand_trigger_verify_0
*TYPE:
*  PROC
*DATE:
*  13-Jan-03
*FUNCTION:
*  verify that the soc_sand-trigger is 0
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN int   unit -
*       Device identifier to verify.
*    SOC_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    SOC_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    SOC_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than SOC_SAND_BIT-0 triggers.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 -
*         SOC_SAND_OK          -- trigger is 0
*         SOC_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_trigger_verify_0(
    SOC_SAND_IN int   unit,
    SOC_SAND_IN uint32  offset,
    SOC_SAND_IN uint32  timeout, /* in nano seconds */
    SOC_SAND_IN uint32  trigger_bit
  );

/*****************************************************
*NAME
* soc_sand_trigger_assert_1
*TYPE:
*  PROC
*DATE:
*  13-Jan-03
*FUNCTION:
*  Set the trigger to 1.
*  If given timeout is bigger than zero --
*  wait for trigger to become zero.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN int   unit -
*       Device identifier to trigger.
*    SOC_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    SOC_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    SOC_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than SOC_SAND_BIT-0 triggers.
*       Use read/modify/write to trig it - not just write.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 -
*         SOC_SAND_OK          -- trigger was set to 1
*         SOC_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_trigger_assert_1(
    SOC_SAND_IN int   unit,
    SOC_SAND_IN uint32  offset,
    SOC_SAND_IN uint32  timeout, /* in nano seconds */
    SOC_SAND_IN uint32  trigger_bit
  );


/*****************************************************
*NAME
* soc_sand_trigger_verify_0_by_base
*TYPE:
*  PROC
*DATE:
*  01-SEP-07
*FUNCTION:
*  verify that the soc_sand-trigger is 0
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    SOC_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    SOC_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than SOC_SAND_BIT-0 triggers.
*     SOC_SAND_IN uint32  *device_base_address -
*       the base address from which the offset is
*       counted for the physical read of the trigger.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 -
*         SOC_SAND_OK          -- trigger is 0
*         SOC_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/

SOC_SAND_RET
  soc_sand_trigger_verify_0_by_base(
    SOC_SAND_IN uint32  offset,
    SOC_SAND_IN uint32  timeout, /* in nano seconds */
    SOC_SAND_IN uint32  trigger_bit,
    SOC_SAND_IN uint32  *device_base_address
  );

/*****************************************************
*NAME
* soc_sand_trigger_assert_1_by_base
*TYPE:
*  PROC
*DATE:
*  01-SEP-07
*FUNCTION:
*  Set the trigger to 1.
*  If given timeout is bigger than zero --
*  wait for trigger to become zero.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN uint32  offset -
*       Offset of trigger in the device (offset in direct memory).
*    SOC_SAND_IN uint32  timeout -
*       Timeout in nanoseconds to wait for trigger to be zero.
*    SOC_SAND_IN uint32  trigger_bit -
*       Trigger bit number. Normally -0- will do.
*       Support triggers different than SOC_SAND_BIT-0 triggers.
*       Use read/modify/write to trig it - not just write.
*    SOC_SAND_IN uint32  *device_base_address -
*       the base address from which the offset is
*       counted for the physical read & write of the trigger.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 -
*         SOC_SAND_OK          -- trigger was set to 1
*         SOC_SAND_TRG_TIMEOUT -- reached time out and trigger is still 1.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/

SOC_SAND_RET
  soc_sand_trigger_assert_1_by_base(
    SOC_SAND_IN     uint32  offset,
    SOC_SAND_IN     uint32  timeout, /* in nano seconds */
    SOC_SAND_IN     uint32  trigger_bit,
    SOC_SAND_INOUT  uint32  *device_base_address
  );

#ifdef  __cplusplus
}
#endif

/* } __SOC_SAND_TRIGGER_H_INCLUDED__*/
#endif
