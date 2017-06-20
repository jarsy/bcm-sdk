/* $Id: dnx_sand_trigger.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#include <soc/dnx/legacy/SAND/SAND_FM/sand_trigger.h>

#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>

/*****************************************************
 * See details in dnx_sand_trigger.h
 *****************************************************/
/* $Id: dnx_sand_trigger.c,v 1.2 Broadcom SDK $
 *  Number of busy-wait retries on trigger verify
 */
#define DNX_SAND_TRIGGER_VERIFY_0_NOF_BW_RETRY  50
/*
 *  Number of timed delay retries on trigger verify
 */
#define DNX_SAND_TRIGGER_VERIFY_0_NOF_TD_RETRY  7
#define DNX_SAND_TRIGGER_VERIFY_0_NOF_RETRY  \
  (DNX_SAND_TRIGGER_VERIFY_0_NOF_BW_RETRY + DNX_SAND_TRIGGER_VERIFY_0_NOF_TD_RETRY)
/*
 *  Before last iteration on polling trigger-down on indirect access
 */
#define DNX_SAND_TRIGGER_VERIFY_BEFORE_LAST_TRY_DELAY_MS 10
/*
 *  Last iteration on polling trigger-down on indirect access
 */
#define DNX_SAND_TRIGGER_VERIFY_LAST_TRY_DELAY_MS 1000


DNX_SAND_RET
  dnx_sand_trigger_verify_0(
    DNX_SAND_IN int   unit,
    DNX_SAND_IN uint32  offset,
    DNX_SAND_IN uint32  timeout, /* in nano seconds */
    DNX_SAND_IN uint32  trigger_bit
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    err;

  uint32
      *device_base_address ;

  device_base_address = dnx_sand_get_chip_descriptor_base_addr(unit) ;
  err = 0 ;
  ex  = DNX_SAND_OK ;


  ex = dnx_sand_trigger_verify_0_by_base(
           offset,
           timeout,
           trigger_bit,
           device_base_address
         );
    if (DNX_SAND_OK != ex)
    {
      err = 1 ;
      goto exit ;
    }

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_TRIGGER_VERIFY_0,
        "General error in dnx_sand_trigger_verify_0()",err,0,0,0,0,0) ;
  return ex ;
}

/*****************************************************
 * See details in dnx_sand_trigger.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_trigger_assert_1(
    DNX_SAND_IN int   unit,
    DNX_SAND_IN uint32  offset,
    DNX_SAND_IN uint32  timeout, /* in nano seconds */
    DNX_SAND_IN uint32  trigger_bit
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    err ;
  uint32
    *device_base_address ;

  device_base_address = dnx_sand_get_chip_descriptor_base_addr(unit) ;
  err = 0 ;
  ex = DNX_SAND_OK ;

  ex = dnx_sand_trigger_assert_1_by_base(
         offset,
         timeout,
         trigger_bit,
         device_base_address
       );

  if (DNX_SAND_OK != ex)
  {
    err = 1 ;
    goto exit ;
  }

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_TRIGGER_ASSERT_1,
        "General error in dnx_sand_trigger_assert_1()",err,0,0,0,0,0) ;
  return ex ;
}


/*****************************************************
 * See details in dnx_sand_trigger.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_trigger_verify_0_by_base(
    DNX_SAND_IN uint32  offset,
    DNX_SAND_IN uint32  timeout, /* in nano seconds */
    DNX_SAND_IN uint32  trigger_bit,
    DNX_SAND_IN uint32  *device_base_address
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    reg = 0;
  uint32
    i ;
  uint32
    err,
    trig_mask,
    timeout_i;

  err = 0 ;
  ex  = DNX_SAND_OK ;

  reg = 0 ;
  trig_mask = DNX_SAND_BIT(trigger_bit);
  /*
   * this is timeout heuristics. Since the timeout is in nano seconds,
   * there's no much point in slicing it to many iterations,
   * but only 1 iteration, seemed not very fair, so we make it 4 times,
   * hard coded, right now, and in the future a modification may be in order.
   */
  timeout_i = timeout>>2;
  for (i = 0 ; i < DNX_SAND_TRIGGER_VERIFY_0_NOF_RETRY ; ++i)
  {
    ex = dnx_sand_physical_read_from_chip(
           &reg,
           device_base_address,
           offset,
           sizeof(reg)
         ) ;
    if (DNX_SAND_OK != ex)
    {
      err = 1 ;
      goto exit ;
    }
    if ( (reg & trig_mask) != trig_mask )
    {
      /*
       * Trigger is down.
       */
      goto exit ;
    }
    if (i >= DNX_SAND_TRIGGER_VERIFY_0_NOF_BW_RETRY) /* start Time Delay */ 
    {
      if (i <= (DNX_SAND_TRIGGER_VERIFY_0_NOF_RETRY - 4)) /* */
      {
          dnx_sand_os_nano_sleep(timeout_i, NULL) ;
          timeout_i *= 4;
      } 
      else if(i == (DNX_SAND_TRIGGER_VERIFY_0_NOF_RETRY - 3)) /* before last one wait DNX_SAND_TRIGGER_VERIFY_BEFORE_LAST_TRY_DELAY_MS milisec */
      {
          dnx_sand_os_task_delay_milisec(DNX_SAND_TRIGGER_VERIFY_BEFORE_LAST_TRY_DELAY_MS);
      } 
      else if(i == (DNX_SAND_TRIGGER_VERIFY_0_NOF_RETRY - 2)) /* last one wait DNX_SAND_TRIGGER_VERIFY_LAST_TRY_DELAY_MS milisec */
      {
          dnx_sand_os_task_delay_milisec(DNX_SAND_TRIGGER_VERIFY_LAST_TRY_DELAY_MS);
      }
    }
  }
  ex = DNX_SAND_TRG_TIMEOUT ; /* timeout */
  err = 2;
  goto exit ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_TRIGGER_VERIFY_0,
        "General error in dnx_sand_trigger_verify_0_by_base()",err,0,0,0,0,0) ;
  return ex ;
}

/*****************************************************
 * See details in dnx_sand_trigger.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_trigger_assert_1_by_base(
    DNX_SAND_IN     uint32  offset,
    DNX_SAND_IN     uint32  timeout, /* in nano seconds */
    DNX_SAND_IN     uint32  trigger_bit,
    DNX_SAND_INOUT  uint32  *device_base_address
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    val ;
  uint32
    err ;
  /*
   */
  err = 0 ;
  ex = DNX_SAND_OK ;
  /*
   * make sure the CPU ended all the writing,
   * before triggering the indirect.
   */
  DNX_SAND_SYNC_IOS ;
  /*
   * Write the trigger bit.
   * Most of the times the trigger is: 0x00000001.
   */
  val = DNX_SAND_BIT(trigger_bit) ;

  /*
   * read-modify-write the trigger.
   */
  ex = dnx_sand_read_modify_write(
         device_base_address,
         offset,
         trigger_bit,
         val,
         1
       ) ;
  if (DNX_SAND_OK != ex)
  {
    err = 1 ;
    goto exit ;
  }
  /*
   * make sure the CPU finished triggering,
   * before testing it to zero.
   */
  DNX_SAND_SYNC_IOS ;
  if(timeout > 0)
  {
    /*
     * Asked to wait for trigger to come down.
     */
    ex = dnx_sand_trigger_verify_0_by_base(offset,timeout, trigger_bit,device_base_address) ;
    if (DNX_SAND_OK != ex)
    {
      err = 2 ;
      goto exit ;
    }
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_TRIGGER_ASSERT_1,
        "General error in dnx_sand_trigger_assert_1_by_base()",err,0,0,0,0,0) ;
  return ex ;
}
