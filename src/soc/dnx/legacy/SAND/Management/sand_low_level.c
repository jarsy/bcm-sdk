  /* $Id: sand_low_level.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
/* $Id: sand_low_level.c,v 1.10 Broadcom SDK $
 */


#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>


#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>

#define PRINT_LOW_LEVEL_ACESS 0

#define TRACE_LOW_LEVEL 0
/*
 */
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
/*
 * To use the simulator with the low-level driver,
 * note the following:
 * dnx_sand_low_level.c is using the the define USING_CHIP_SIM.
 * Which is defined in this file.
 * To activate this define one need to add SAND_LOW_LEVEL_SIMULATION
 * define to compilation (via -dDEFINE).
 * Alternatively, one may take out the '#if 0' clause below.
 */
#ifdef    SAND_LOW_LEVEL_SIMULATION
  #define   USING_CHIP_SIM 1
#endif

#undef USING_CHIP_SIM
/*
 */
#ifdef USING_CHIP_SIM
  #include <sim/dnx/ChipSim/chip_sim_low.h>
  extern int chip_sim_task_is_alive(void) ;
#endif
/*
 */
#ifndef DNX_SAND_LOW_LEVEL_ERR_MSG
  #define DNX_SAND_LOW_LEVEL_ERR_MSG 0
#endif
/*
 */

/*
 *  Global Variables {
 */

/*
 * deal with the endian issue
 * {
 */

/*
 * Indicator:
 * TRUE  - DNX_SAND driver solved the big/little endian issue.
 * FALSE - DNX_SAND driver DID NOT solved the big/little endian issue.
 */
uint32
  Dnx_soc_sand_big_endian_was_checked = FALSE;

#ifdef    SAND_LOW_LEVEL_SIMULATION
static uint8
  Dnx_soc_sand_low_level_is_sim_active = TRUE;
#endif

/*
 * Indicator:
 * TRUE  - DNX_SAND driver is access the device in big endian.
 * FALSE - DNX_SAND driver is access the device in little endian.
 */

uint32
  Dnx_soc_sand_big_endian = TRUE;


#ifdef DNX_SAND_LL_TIMER

/*
 *  The function counters
 */
DNX_SAND_LL_TIMER_FUNCTION Dnx_soc_sand_ll_timer_cnt[DNX_SAND_LL_TIMER_MAX_NOF_TIMERS];
uint8 Dnx_soc_sand_ll_is_any_active;
uint32  Dnx_soc_sand_ll_timer_total;
#endif

#ifdef DNX_SAND_LL_ACCESS_STATISTICS

/*
 *  The indications below are per-tag + the overall statistics
 */
uint8 Dnx_soc_sand_ll_stat_is_active[DNX_SAND_LL_NOF_STAT_IDS];
uint32  Dnx_soc_sand_ll_access_count[DNX_SAND_LL_NOF_STAT_IDS][DNX_SAND_LL_NOF_ACCESS_DIRECTIONS];

/*
 *  Set if overflow occurs on one of the coutners
 */
uint8 Dnx_soc_sand_ll_overflow[DNX_SAND_LL_NOF_STAT_IDS];

#endif

/*
 *  Global Variables }
 */

#ifdef DNX_SAND_LL_ACCESS_STATISTICS
/*
 *  Clear all access statistics counters and enablers.
 *  Must be run before using the access statistics.
 */
void
  dnx_sand_ll_stat_clear(void)
{
  uint32
    stat_i,
    dir_i;

  for (stat_i = 0; stat_i < DNX_SAND_LL_NOF_STAT_IDS; stat_i++)
  {
    Dnx_soc_sand_ll_stat_is_active[stat_i] = FALSE;
    Dnx_soc_sand_ll_overflow[stat_i] = FALSE;

    for (dir_i = 0; dir_i < DNX_SAND_LL_NOF_ACCESS_DIRECTIONS; dir_i++)
    {
      Dnx_soc_sand_ll_access_count[stat_i][dir_i] = 0;
    }
  }
}


/*
 *  Activate/diactivate access statistics collection.
 *  When activating, the overall statistics is always activated.
 *  To activate just the overall collection, without per-tag collection, 
 *  use DNX_SAND_LL_STAT_ID_TOTAL as an index.
 *  If per-tag collection is also needed, use the requested tag index.
 */
void 
  dnx_sand_ll_stat_is_active_set(
    uint32 stat_ndx,
    uint8 is_active

  )
{

  if (stat_ndx >= DNX_SAND_LL_NOF_STAT_IDS)
  {
    /* Invalid */
    goto exit;
  }

  Dnx_soc_sand_ll_stat_is_active[stat_ndx] = is_active;
 
  if (is_active == TRUE)
  {
    Dnx_soc_sand_ll_stat_is_active[DNX_SAND_LL_STAT_ID_TOTAL] = TRUE;
  }

exit:
   return;
}

static uint8
  dnx_sand_ll_is_stat_active(void)
{
  return Dnx_soc_sand_ll_stat_is_active[DNX_SAND_LL_STAT_ID_TOTAL];
}

/*
 *  Increment the access counter, per direction (Read/Write)
 *  if the statistics collection is enabled. 
 */
void
  dnx_sand_ll_stat_increment_if_active(
    DNX_SAND_LL_ACCESS_DIRECTION direction
  )
{
  uint32
    stat_i;
  
  if (dnx_sand_ll_is_stat_active())
  {
    if (direction >= DNX_SAND_LL_NOF_ACCESS_DIRECTIONS)
    {
      /* Invalid */
      goto exit;
    }

    for (stat_i = 0; stat_i < DNX_SAND_LL_NOF_STAT_IDS; stat_i++)
    {
      if (Dnx_soc_sand_ll_stat_is_active[stat_i] == TRUE)
      {
        Dnx_soc_sand_ll_access_count[stat_i][direction]++;
      }

      if (Dnx_soc_sand_ll_access_count[stat_i][direction] == DNX_SAND_U32_MAX)
      {
        Dnx_soc_sand_ll_access_count[stat_i][direction] = 0;
        Dnx_soc_sand_ll_overflow[stat_i] = TRUE;
      }
    }
  }
  
exit:
  return;
}

void 
  dnx_sand_ll_stat_print(void)
{
  uint32
    stat_i,
    dir_i;

  for (stat_i = 0; stat_i < DNX_SAND_LL_NOF_STAT_IDS; stat_i++)
  {
    if (stat_i == DNX_SAND_LL_STAT_ID_TOTAL)
    {
      LOG_CLI((BSL_META("------------------ \n\r")));
      LOG_CLI((BSL_META("Total: \n\r")));
    }
    else
    {
      LOG_CLI((BSL_META("Tag %u: \n\r"), stat_i));
    }

    if (Dnx_soc_sand_ll_overflow[stat_i] == TRUE)
    {
      LOG_CLI((BSL_META("OVERFLOW occurred, counter not available! \n\r")));
      continue;
    }

    for (dir_i = 0; dir_i < DNX_SAND_LL_NOF_ACCESS_DIRECTIONS; dir_i++)
    {
      LOG_CLI((BSL_META("  -- %s: %lu\n\r"), (dir_i == DNX_SAND_LL_ACCESS_DIRECTION_READ)?"Read ":"Write",
               Dnx_soc_sand_ll_access_count[stat_i][dir_i]
               ));
      
    }
  }
}

#endif /* DNX_SAND_LL_ACCESS_STATISTICS */

#ifdef DNX_SAND_LL_TIMER

uint32
  dnx_sand_ll_get_time_in_ms(
    void
  )
{
  uint32
    seconds,
    nano_seconds;

  dnx_sand_os_get_time(&seconds, &nano_seconds);
  return seconds*1000 + nano_seconds/1000000;
}


/*
 *  Init the counters
 */
void
  dnx_sand_ll_timer_clear(
    void
  )
{
  uint32
    cnt_ndx;
  for (cnt_ndx = 0; cnt_ndx < DNX_SAND_LL_TIMER_MAX_NOF_TIMERS; ++cnt_ndx)
  {
    dnx_sand_os_strncpy(Dnx_soc_sand_ll_timer_cnt[cnt_ndx].name, "no_function", DNX_SAND_LL_TIMER_MAX_NOF_CHARS_IN_TIMER_NAME);
    Dnx_soc_sand_ll_timer_cnt[cnt_ndx].nof_hits = 0;
    Dnx_soc_sand_ll_timer_cnt[cnt_ndx].total_time = 0;
    Dnx_soc_sand_ll_timer_cnt[cnt_ndx].start_timer = 0;
    Dnx_soc_sand_ll_timer_cnt[cnt_ndx].active = 0;
  }
  Dnx_soc_sand_ll_is_any_active = FALSE;
  Dnx_soc_sand_ll_timer_total = 0;
}

void
  dnx_sand_ll_timer_stop_all(
    void
  )
{
  uint32
    cnt_ndx;
  for (cnt_ndx = 0; cnt_ndx < DNX_SAND_LL_TIMER_MAX_NOF_TIMERS; ++cnt_ndx)
  {
    if( Dnx_soc_sand_ll_timer_cnt[cnt_ndx].active){
      dnx_sand_ll_timer_stop(cnt_ndx);
    }
  }
}

/*
 *  Associate a counter id with a string and take the start time.
 *  Note 1: can be stopped and restarted multiple times.
 *  The resulting time will be the accumulated time between all start-stop pairs
 *  Note 2: The string is used only for printing the result. 
 */
void
  dnx_sand_ll_timer_set(
    DNX_SAND_IN char name[DNX_SAND_LL_TIMER_MAX_NOF_CHARS_IN_TIMER_NAME],
    DNX_SAND_IN uint32 timer_ndx
  )
{

  if (timer_ndx >= DNX_SAND_LL_TIMER_MAX_NOF_TIMERS)
  {
    /* Invalid */
    goto exit;
  }

  if(Dnx_soc_sand_ll_timer_cnt[timer_ndx].active)
  {
    goto exit;
  }

  dnx_sand_os_strncpy(Dnx_soc_sand_ll_timer_cnt[timer_ndx].name, name, DNX_SAND_LL_TIMER_MAX_NOF_CHARS_IN_TIMER_NAME);    
  Dnx_soc_sand_ll_timer_cnt[timer_ndx].start_timer = sal_time_usecs();

  if (!Dnx_soc_sand_ll_is_any_active)
  {
    Dnx_soc_sand_ll_is_any_active = TRUE;
  }

  Dnx_soc_sand_ll_timer_cnt[timer_ndx].active = 1;

exit:
  return;
}

/*
 *  Stop the counter associated with the function
 */
void
  dnx_sand_ll_timer_stop(
    DNX_SAND_IN uint32 timer_ndx
  )
{
  uint32
    new_delta;

  if (timer_ndx >= DNX_SAND_LL_TIMER_MAX_NOF_TIMERS)
  {
    /* Invalid */
    goto exit;
  }

  if(!Dnx_soc_sand_ll_timer_cnt[timer_ndx].active)
  {
    goto exit;
  }

  new_delta = sal_time_usecs() - Dnx_soc_sand_ll_timer_cnt[timer_ndx].start_timer;
  Dnx_soc_sand_ll_timer_cnt[timer_ndx].nof_hits += 1;
  Dnx_soc_sand_ll_timer_cnt[timer_ndx].total_time += new_delta;
  Dnx_soc_sand_ll_timer_cnt[timer_ndx].start_timer = 0;

  Dnx_soc_sand_ll_timer_total += new_delta;
  Dnx_soc_sand_ll_timer_cnt[timer_ndx].active = 0;

exit:
  return;
}

/*
 *  Print all the results
 */
void 
  dnx_sand_ll_timer_print_all(
    void
  )
{
  uint32
    cnt_ndx;
  COMPILER_UINT64
    total_time_1000, total_time_100, timer_total ;

  LOG_CLI((BSL_META("\r\n")));
  if (Dnx_soc_sand_ll_timer_total == 0)
  {
    LOG_CLI((BSL_META("No timers were hit, total measured execution time: 0\n\r")));
  }
  else
  {
    LOG_CLI((BSL_META(" Execution Time Measurements.\n\r")));
    LOG_CLI((BSL_META(" Note: Percents are calculated relative to to the total measured time,\n\r")));
    LOG_CLI((BSL_META(" not accounting for overlapping timers\n\r")));
    LOG_CLI((BSL_META(" +-----------------------------------------------------------------------------------------+\n\r")));
    LOG_CLI((BSL_META(" | Timer Name                             |Hit Count |Total Time[us] |Per Hit[us] |Percent |\n\r")));
    LOG_CLI((BSL_META(" +-----------------------------------------------------------------------------------------+\n\r")));
    COMPILER_64_SET(timer_total, 0, Dnx_soc_sand_ll_timer_total);
    for (cnt_ndx = 0; cnt_ndx < DNX_SAND_LL_TIMER_MAX_NOF_TIMERS; ++cnt_ndx)
    {
      if (Dnx_soc_sand_ll_timer_cnt[cnt_ndx].nof_hits != 0)
      {
        COMPILER_64_SET(total_time_1000 ,0 ,Dnx_soc_sand_ll_timer_cnt[cnt_ndx].total_time);
        COMPILER_64_SET(total_time_100, 0, Dnx_soc_sand_ll_timer_cnt[cnt_ndx].total_time);
        COMPILER_64_UMUL_32(total_time_1000,1000);
        COMPILER_64_UMUL_32(total_time_100,100);
        COMPILER_64_UDIV_64(total_time_1000,timer_total);
        COMPILER_64_UDIV_64(total_time_100,timer_total);
        LOG_CLI((BSL_META(" |%-40s| %-9d|%-15d|%-12d|%3d.%1d%%  |\n\r"), 
            Dnx_soc_sand_ll_timer_cnt[cnt_ndx].name, 
            Dnx_soc_sand_ll_timer_cnt[cnt_ndx].nof_hits,
            Dnx_soc_sand_ll_timer_cnt[cnt_ndx].total_time, 
            Dnx_soc_sand_ll_timer_cnt[cnt_ndx].total_time / Dnx_soc_sand_ll_timer_cnt[cnt_ndx].nof_hits,
            COMPILER_64_LO(total_time_100),
            COMPILER_64_LO(total_time_1000) - COMPILER_64_LO(total_time_100)*10
        ));
        LOG_CLI((BSL_META(" +-----------------------------------------------------------------------------------------+\n\r")));
      }
    }
    LOG_CLI((BSL_META(" Total time: %u[us]\n\r"), Dnx_soc_sand_ll_timer_total));
  } /* Timer hits != 0 */
}


#endif /* DNX_SAND_LL_TIMER */

void
  dnx_sand_ssr_set_big_endian(
    uint32 dnx_sand_big_endian_was_checked,
    uint32 dnx_sand_big_endian
  )
{
  Dnx_soc_sand_big_endian_was_checked = dnx_sand_big_endian_was_checked;
  Dnx_soc_sand_big_endian             = dnx_sand_big_endian;
}

void
  dnx_sand_ssr_get_big_endian(
    uint32 *dnx_sand_big_endian_was_checked,
    uint32 *dnx_sand_big_endian
  )
{
  *dnx_sand_big_endian_was_checked = Dnx_soc_sand_big_endian_was_checked;
  *dnx_sand_big_endian             = Dnx_soc_sand_big_endian;
}


/*
 *  Low Level Simulation configuration
 *  When running over a simulation layer, 
 *  some driver code is skipped.
 *  For example, if a HW is expected to raise some indication upon certain conditions,
 *  and the mechanism for raising this indication is not simulated,
 *  the code checking the indication is skipped.
 *  This is implemented as follows:
 *  If SAND_LOW_LEVEL_SIMULATION is not defined, the code is executed - 
 *  assuming running on a real HW.
 *  If SAND_LOW_LEVEL_SIMULATION is defined, code execution can be controlled on-the-fly,
 *  by calling dnx_sand_low_is_sim_active_set().
 *  The code that supports this mechanism must follow the below logic:
 *   uint8 is_real_not_sim;
 *   #ifndef  SAND_LOW_LEVEL_SIMULATION
 *      is_real_not_sim = TRUE;
 *   #else
 *      is_real_not_sim = dnx_sand_low_is_sim_active_get();
 *   endif#
 *   if(is_real_not_sim)
 *   {Driver Code to run on real HW only, not on simulation}
 *
 *  Note! this logic is not implemented on the old devices.
 *  In this case, the code is backward compatible, meaning:
 *  The code runs if and only if SAND_LOW_LEVEL_SIMULATION is not defined.
 */
#ifdef  SAND_LOW_LEVEL_SIMULATION
uint8
  dnx_sand_low_is_sim_active_get(void)
{
  return Dnx_soc_sand_low_level_is_sim_active;
}

void dnx_sand_low_is_sim_active_set(
       DNX_SAND_IN uint8 is_sim_active
     )
{
  Dnx_soc_sand_low_level_is_sim_active = is_sim_active;
}
#endif

/*****************************************************
*NAME
* dnx_sand_check_chip_type_decide_big_endian
*TYPE:
*  PROC
*DATE:
*  21-Sep-03
*FUNCTION:
* Decide if the CPU is BIG/LITTLE endian,
* depending on the version register.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*      Pointer to physical access function to the devices.
*    DNX_SAND_IN uint32 version_reg -
*      Version register value.
*    DNX_SAND_IN uint32 chip_type_shift -
*    DNX_SAND_IN uint32 chip_type_mask  -
*      Chip-Type shift/mask in the register.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_check_chip_type_decide_big_endian(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 version_reg,
    DNX_SAND_IN uint32 chip_type_shift,
    DNX_SAND_IN uint32 chip_type_mask
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    chip_type_on_device;

  dnx_sand_ret = DNX_SAND_OK;

  dnx_sand_os_task_lock();

  chip_type_on_device = DNX_SAND_GET_FLD_FROM_PLACE (version_reg, chip_type_shift, chip_type_mask);
  if(dnx_sand_get_chip_descriptor_chip_type(unit) == chip_type_on_device)
  {
    /*
     * Chip type is fine.
     * nothing to do.
     */
    dnx_sand_ret = DNX_SAND_OK;

    /*
     * From now, the device is considered checked.
     */
    Dnx_soc_sand_big_endian_was_checked = TRUE;
    goto exit;
  }

  /*
   * System Little/Big endian was NOT checked already.
   * Try to flip the bytes.
   */
  chip_type_on_device = DNX_SAND_GET_FLD_FROM_PLACE (DNX_SAND_BYTE_SWAP(version_reg), chip_type_shift, chip_type_mask);
  if(dnx_sand_get_chip_descriptor_chip_type(unit) == chip_type_on_device)
  {
    /*
     * Chip type is fine.
     * Flip the bytes.
     */
    Dnx_soc_sand_big_endian = !Dnx_soc_sand_big_endian ;

    /*
     * From now, the device is considered checked.
     */
    Dnx_soc_sand_big_endian_was_checked = TRUE;

    dnx_sand_ret = DNX_SAND_OK;
    goto exit;
  }

  /*
   * This is an error.
   */
  dnx_sand_ret = DNX_SAND_ERR;


exit:
  dnx_sand_os_task_unlock();
  return dnx_sand_ret;
}

/*
 * Return Indicator:
 * TRUE  - DNX_SAND driver is access the device in big endian.
 * FALSE - DNX_SAND driver is access the device in little endian.
 */
uint32
  dnx_sand_system_is_big_endian(
    void
  )
{
  return Dnx_soc_sand_big_endian;
}

/*
 * }
 */

/*
 * {
 * access hooks
 */
/*
 * 2 function pointers.
 * These are the physical access hooks user may supply,
 * in case not the External CPU interface is connected.
 * Examples:
 *  + DNX_SAND_FE200  have only ECI. Hence, there is no need to change these functions.
 *  + DNX_SAND_FAP10M have ECI, UpLink and I2C interfaces.
 *    Hence, it is board depended to which interface to hook.
 */
DNX_SAND_PHYSICAL_ACCESS
  Dnx_soc_sand_physical_access = { dnx_sand_eci_write, dnx_sand_eci_read} ;

/*****************************************************
* see remarks & definitions in the dnx_sand_low_level.h
*****************************************************/
DNX_SAND_RET
  dnx_sand_set_physical_access_hook(
    DNX_SAND_IN DNX_SAND_PHYSICAL_ACCESS* physical_access
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;
  if (NULL == physical_access)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }
  /*
   * The driver may not run at this stage.
   */
  dnx_sand_os_task_lock() ;
  if (physical_access->physical_write)
  {
    Dnx_soc_sand_physical_access.
      physical_write = physical_access->physical_write ;
  }
  else
  {
    Dnx_soc_sand_physical_access.physical_write = dnx_sand_eci_write ;
  }
  if (physical_access->physical_read)
  {
    Dnx_soc_sand_physical_access.
      physical_read = physical_access->physical_read ;
  }
  else
  {
    Dnx_soc_sand_physical_access.physical_read = dnx_sand_eci_read ;
  }
  dnx_sand_os_task_unlock() ;
exit:
  return dnx_sand_ret ;
}
/*****************************************************
* see remarks & definitions in the dnx_sand_low_level.h
*****************************************************/
DNX_SAND_RET
  dnx_sand_get_physical_access_hook(
    DNX_SAND_OUT DNX_SAND_PHYSICAL_ACCESS* physical_access
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;

  if (NULL != physical_access)
  {
    physical_access->physical_write = Dnx_soc_sand_physical_access.physical_write ;
    physical_access->physical_read = Dnx_soc_sand_physical_access.physical_read ;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }

exit:
  return dnx_sand_ret ;
}

/*
 * }
 */

/*
 */
/*****************************************************
*NAME:
* dnx_sand_physical_write_to_chip
*DATE:
* 04/SEP/2002
*FUNCTION:
* writes array of size into the chip at offset
*INPUT:
*  DNX_SAND_DIRECT:
*   DNX_SAND_IN     uint32     *array      - array of 32bits to write to
*   DNX_SAND_IN     uint32     offset      - offset to write to
*   DNX_SAND_IN     uint32     size        - array size (in bytes)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS: 1. buffer is comprised of longs (32bit - 4bytes words)
*         2. a black list of addresses that must not be written into
*            should be maintained - for instance user should never
*            write directly into the interrupt mask registers, because
*            race condition could occur with the interrupt handler
*SEE ALSO:
*****************************************************/

uint32 Dnx_soc_sand_physical_write_enable=TRUE;
void
  dnx_sand_set_physical_write_enable(
    uint32 physical_write_enable
  )
{
  Dnx_soc_sand_physical_write_enable = physical_write_enable;
  return;
}

void
  dnx_sand_get_physical_write_enable(
    uint32 *physical_write_enable
  )
{
  *physical_write_enable = Dnx_soc_sand_physical_write_enable;
  return;
}

uint32 Dnx_soc_sand_physical_print_when_writing=FALSE;
uint32 Dnx_soc_sand_physical_print_asic_style=FALSE;
uint32 Dnx_soc_sand_physical_print_indirect_write=FALSE;
uint32 Dnx_soc_sand_physical_print_part_of_indirect_read=FALSE;
uint32 Dnx_soc_sand_physical_print_part_of_indirect_write=FALSE;
uint32 Dnx_soc_sand_physical_print_part_of_read_modify_write=FALSE;
uint32 Dnx_soc_sand_physical_print_unit_or_base_address=TRUE;

uint32 Dnx_soc_sand_physical_print_first_reg = 0;
uint32 Dnx_soc_sand_physical_print_last_reg = 0xFFFF;

void
  dnx_sand_set_print_when_writing_reg_range(
    uint32 first_reg,
    uint32 last_reg
  )
{
  Dnx_soc_sand_physical_print_first_reg = first_reg;
  Dnx_soc_sand_physical_print_last_reg  = last_reg;
}

void
  dnx_sand_set_print_when_writing(
    uint32 physical_print_when_writing,
    uint32 asic_style,
    uint32 indirect_write
  )
{
  Dnx_soc_sand_physical_print_when_writing = physical_print_when_writing;
  Dnx_soc_sand_physical_print_asic_style = asic_style;
  Dnx_soc_sand_physical_print_indirect_write = indirect_write;
  return;
}

void
  dnx_sand_get_print_when_writing(
    uint32 *physical_print_when_writing,
    uint32 *asic_style,
    uint32 *indirect_write
  )
{
  *physical_print_when_writing = Dnx_soc_sand_physical_print_when_writing;
  *asic_style                  = Dnx_soc_sand_physical_print_asic_style;
  *indirect_write              = Dnx_soc_sand_physical_print_indirect_write;
  return;
}


void
  dnx_sand_get_print_when_writing_unit_or_base_address(
    uint32 *unit_or_base_address
    )
{
  *unit_or_base_address = Dnx_soc_sand_physical_print_unit_or_base_address;
  return;
}

void
  dnx_sand_set_print_when_writing_unit_or_base_address(
    uint32 unit_or_base_address
    )
{
  Dnx_soc_sand_physical_print_unit_or_base_address = unit_or_base_address;
}

void
  dnx_sand_set_print_when_writing_part_of_indirect_read(
    uint32 part_of_indirect
  )
{
  Dnx_soc_sand_physical_print_part_of_indirect_read = part_of_indirect;
}


void
  dnx_sand_set_print_when_writing_part_of_indirect_write(
    uint32 part_of_indirect
  )
{
  Dnx_soc_sand_physical_print_part_of_indirect_write = part_of_indirect;
}

static DNX_SAND_RET
  dnx_sand_physical_print_when_write_to_chip(
    DNX_SAND_IN     uint32 *array,
    DNX_SAND_IN     uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 size
  )
{
  DNX_SAND_RET
    dnx_sand_ret ;
  dnx_sand_ret = DNX_SAND_OK ;

  if(Dnx_soc_sand_physical_print_when_writing)
  {
    uint32
      print_num = size/4,
      print_i=0;

    while(print_i<print_num)
    {
      if(Dnx_soc_sand_physical_print_part_of_indirect_read ||
          (Dnx_soc_sand_physical_print_part_of_indirect_write &&
           !Dnx_soc_sand_physical_print_indirect_write
          )
        )
      {
        print_i++;
        continue;
      }

      if(Dnx_soc_sand_physical_print_part_of_indirect_write ||
         Dnx_soc_sand_physical_print_part_of_read_modify_write
        )
      {
        print_i++;
        continue;
      }

      if(Dnx_soc_sand_physical_print_first_reg > offset/4 ||
         Dnx_soc_sand_physical_print_last_reg  < offset/4
        )
      {
        print_i++;
        continue;
      }

      if(Dnx_soc_sand_physical_print_asic_style)
      {
        LOG_CLI((BSL_META("WRITE_REG 0x%x 0x%08x\n\r"),
                 offset/4 + print_i,
                 *(array+print_i)
                 ));
      }
      else if (Dnx_soc_sand_physical_print_unit_or_base_address)
      {
        LOG_CLI((BSL_META("0x%08x 0x%08x 0x%08X\n\r"),
                 PTR_TO_INT(base_address),
                 offset/4 + print_i,
                 *(array+print_i)
                 ));
      }
      else
      {
        LOG_CLI((BSL_META("0x%08x 0x%08X\n\r"),
                 offset/4 + print_i,
                 *(array+print_i)
                 ));
      }
      print_i++;
    }
  }

  if(!Dnx_soc_sand_physical_write_enable)
  {
    LOG_CLI((BSL_META("dnx_sand_physical_write_to_chip FAILED. Writing is disabled.\n\r"
                      "(0x%08x 0x%08x).\n\r"),
             offset/4,
             *(array)
             ));
  }
  goto exit;
exit:
  return dnx_sand_ret;
}


DNX_SAND_RET
  dnx_sand_physical_write_to_chip(
    DNX_SAND_IN     uint32 *array,
    DNX_SAND_INOUT  uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 size
  )
{
  DNX_SAND_RET
    dnx_sand_ret ;
  dnx_sand_ret = DNX_SAND_OK ;

#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  if ((Dnx_soc_sand_physical_print_when_writing) || (!Dnx_soc_sand_physical_write_enable))
  {
    dnx_sand_physical_print_when_write_to_chip(
      array,base_address,offset,size
    );
  }
#endif

#ifdef DNX_SAND_LL_ACCESS_STATISTICS
  dnx_sand_ll_stat_increment_if_active(DNX_SAND_LL_ACCESS_DIRECTION_WRITE);
#endif

  if(!Dnx_soc_sand_physical_write_enable)
  {
    goto exit;
  }
  if (Dnx_soc_sand_physical_access.physical_write != NULL)
  {
    dnx_sand_ret = Dnx_soc_sand_physical_access.physical_write(
                 array,
                 base_address,
                 offset,
                 size
               ) ;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_NULL_USER_CALLBACK_FUNC ;
    goto exit ;
  }

exit:
  return dnx_sand_ret ;
}

/*
 */
/*****************************************************
*NAME:
* dnx_sand_physical_read_from_chip
*DATE:
* 04/SEP/2002
*FUNCTION:
* reads into an array of size from the chip at offset
*INPUT:
*  DNX_SAND_DIRECT:
*   DNX_SAND_INOUT  uint32     *array        - array of 32bits to write to
*   DNX_SAND_IN     uint32     *base_address - the beginning of the chip address space
*   DNX_SAND_IN     uint32     offset      - offset to read from chip
*   DNX_SAND_IN     uint32     size        - number (of bytes) to read from chip
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_physical_read_from_chip(
    DNX_SAND_INOUT  uint32 *array,
    DNX_SAND_IN     uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 size
  )
{
  DNX_SAND_RET
    dnx_sand_ret ;
  dnx_sand_ret = DNX_SAND_OK ;

#ifdef DNX_SAND_LL_ACCESS_STATISTICS
  dnx_sand_ll_stat_increment_if_active(DNX_SAND_LL_ACCESS_DIRECTION_READ);;
#endif

  if (Dnx_soc_sand_physical_access.physical_read != NULL)
  {
    dnx_sand_ret = Dnx_soc_sand_physical_access.physical_read(
                 array,
                 base_address,
                 offset,
                 size
               ) ;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_NULL_USER_CALLBACK_FUNC ;
    goto exit ;
  }

exit:
  return dnx_sand_ret ;
}
/*****************************************************
*NAME:
* dnx_sand_read_modify_write
*DATE:
* 11/NOV/2002
*FUNCTION:
* reads modify writes information
*INPUT:
*  DNX_SAND_DIRECT:
*   DNX_SAND_INOUT  uint32 *base_address   -
*     beginning of device address space
*   DNX_SAND_IN     uint32 offset          -
*     offset of the field
*   DNX_SAND_IN     uint32 shift           -
*     shift of the field within 32 bit register
*   DNX_SAND_IN     uint32 mask            -
*     mask of the field within 32 bit register
*   DNX_SAND_IN     uint32 data_to_write      -
*     the new data to write
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*SEE ALSO:
*****************************************************/
static DNX_SAND_RET
  dnx_sand_physical_print_when_read_modify_write_to_chip(
    DNX_SAND_IN     uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 mask,
    DNX_SAND_IN     uint32 data_to_write
  )
{
  DNX_SAND_RET
    dnx_sand_ret ;
  dnx_sand_ret = DNX_SAND_OK ;

  if(Dnx_soc_sand_physical_print_when_writing)
  {
    if(Dnx_soc_sand_physical_print_part_of_indirect_read   ||
       (Dnx_soc_sand_physical_print_part_of_indirect_write &&
        !Dnx_soc_sand_physical_print_indirect_write)       ||
       (Dnx_soc_sand_physical_print_first_reg > offset/4   ||
        Dnx_soc_sand_physical_print_last_reg  < offset/4)
      )
    {
      goto exit;
    }

    if(Dnx_soc_sand_physical_print_asic_style)
    {
      LOG_CLI((BSL_META("`READ_MOD_WRITE (14'h%x,32'h%x,32'h%x);\n\r"),
               offset/4,
               mask,
               data_to_write
               ));
    }
    else if (Dnx_soc_sand_physical_print_unit_or_base_address)
    {
      LOG_CLI((BSL_META("0x%08x 0x%08x 0x%08x [ mask 0x%X]\n\r"),
               PTR_TO_INT(base_address),
               offset/4,
               data_to_write,
               mask
               ));
    }
    else
    {
      LOG_CLI((BSL_META("0x%08x 0x%08x [ mask 0x%X]\n\r"),
               offset/4,
               data_to_write,
               mask
               ));
    }
  }

  if(!Dnx_soc_sand_physical_write_enable)
  {
    LOG_CLI((BSL_META("dnx_sand_read_modify_write FAILED. Writing is disabled.\n\r"
                      "(0x%08x 0x%08x [mask 0x%X]).\n\r"),
             offset/4,
             data_to_write,
             mask
             ));
  }
  goto exit;
exit:
  return dnx_sand_ret;
}

DNX_SAND_RET
  dnx_sand_read_modify_write(
    DNX_SAND_INOUT  uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 shift,
    DNX_SAND_IN     uint32 mask,
    DNX_SAND_IN     uint32 data_to_write
  )
{
  DNX_SAND_RET ex ;
  uint32 tmp[1];
  /* Read */
  Dnx_soc_sand_physical_print_part_of_read_modify_write = TRUE;
  ex = dnx_sand_physical_read_from_chip( tmp, base_address, offset, sizeof(uint32)) ;
  if (DNX_SAND_OK != ex)
  {
    goto exit ;
  }
  /* Modify */
  *tmp &= ~mask ;
  *tmp |= DNX_SAND_SET_FLD_IN_PLACE(data_to_write, shift, mask) ;

#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  if(!Dnx_soc_sand_physical_print_part_of_indirect_write ||
     !Dnx_soc_sand_physical_print_indirect_write
    )
  {
    dnx_sand_physical_print_when_read_modify_write_to_chip(
      base_address, offset,mask,data_to_write
    );
  }
#endif
  /* Write */
  ex  = dnx_sand_physical_write_to_chip( tmp, base_address, offset, sizeof(uint32)) ;
  if (DNX_SAND_OK != ex)
  {
    goto exit ;
  }
  /*
   */
exit:
  Dnx_soc_sand_physical_print_part_of_read_modify_write = FALSE;
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_READ_MODIFY_WRITE,
        "error in dnx_sand_read_modify_write(): Cannot access chip",0,0,0,0,0,0) ;
  return ex ;
}

/*
 * {
 * 'dnx_sand_eci_write', 'dnx_sand_eci_read' are default implementation of the physical interface:
 * These 2 functions are the default device low access to be used,
 * if non other are given. These functions assume the device has
 * External CPU interface - ECI. This interface is assumed to be byte aligned.
 * That is, offset 0x0 of the device is the first device register.
 * Next device register is at offset 0x4, ...
 * Other device access methods are available (board depended). User should supply
 * its specific access method (BSP layer).
 * To connect those BSP function use 'dnx_sand_set_physical_access_hook'.
 * This access replacement should be done before any device access,
 * even before device registration.
 */

DNX_SAND_RET
  dnx_sand_eci_write(
    DNX_SAND_IN     uint32 *array,
    DNX_SAND_INOUT  uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 size
 )
{
  VOLATILE uint32
    *ul_ptr;
  const VOLATILE uint32
    *local_buff_ptr ;
  uint32
    ui,
    local_offset,
    local_size;
  uint32
    big_endian;
  DNX_SAND_RET
    ex;

  ex = DNX_SAND_OK ;
  big_endian = Dnx_soc_sand_big_endian;

#if PRINT_LOW_LEVEL_ACESS
  LOG_INFO(BSL_LS_SOC_MANAGEMENT,
           (BSL_META("\r\n eci_write() base: 0x%X ; offset: 0x%X ; size %d ; data: 0x%X"), base_address, offset, size, *array));
#endif
  /*
   * Notice that due to the endian issue this method
   * handles only memory that is comprized of longs
   */
  local_offset  = offset >> 2 ;
  local_size    = size   >> 2 ;
  ul_ptr        = base_address + local_offset ;

  /*
   * Use local pointer
   */
  local_buff_ptr = array;

  for (ui=0 ; ui<local_size ; ++ui)
  {
#ifdef USING_CHIP_SIM
/* { */
    if ( chip_sim_task_is_alive() )
    {
      if (chip_sim_write(PTR_TO_INT(ul_ptr), *local_buff_ptr) )
      {
#if DNX_SAND_LOW_LEVEL_ERR_MSG
/* { */
        logMsg(
          "dnx_sand_physical_write_to_chip()"
          " chip_sim_write(0x%X) returned with an error\r\n",
          ul_ptr,0,0,0,0,0
        ) ;
/* } */
#endif/*DNX_SAND_LOW_LEVEL_ERR_MSG*/  
        ex = DNX_SAND_ERR ;
        goto exit ;
      }
      local_buff_ptr++ ;
      ul_ptr++ ;
    }
    else
    {
      ex = DNX_SAND_ERR ;
      goto exit ;
    }
/* } */
#else/*USING_CHIP_SIM*/
/* { */
    if(big_endian) 
    {
      *ul_ptr++ = *local_buff_ptr++ ;
    }
    else
    {
      *ul_ptr = DNX_SAND_BYTE_SWAP(*local_buff_ptr) ;
      ul_ptr++;
      local_buff_ptr++;
    }
/* } */
#endif/*USING_CHIP_SIM*/
  }
  goto exit ;
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_PHYSICAL_WRITE_TO_CHIP,
        "error in dnx_sand_physical_write_to_chip(): Cannot access chip",
         PTR_TO_INT(array), PTR_TO_INT(base_address),
         offset, size, 0,0
  ) ;
  return ex ;
}

DNX_SAND_RET
  dnx_sand_eci_read(
    DNX_SAND_INOUT  uint32 *array,
    DNX_SAND_IN     uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 size
 )
{
  const VOLATILE uint32
      *ul_ptr ;
  uint32
    ui,
    local_offset,
    local_size,
    local_value,
#if PRINT_LOW_LEVEL_ACESS
    *tmp_array,
#endif
    big_endian;
  DNX_SAND_RET
    ex;

#if TRACE_LOW_LEVEL
  uint32
    before, after ;
  uint32
    elapsed ;
#endif

  ex = DNX_SAND_OK ;
  big_endian = dnx_sand_system_is_big_endian();
#if PRINT_LOW_LEVEL_ACESS
  tmp_array = array ;
#endif
  /*
   * Notice that due to the endian issue this method
   * handles only memory that is comprized of longs
   */
  local_offset  = offset  >> 2 ;
  local_size    = size    >> 2 ;
  ul_ptr        = base_address + local_offset ;
  for (ui = 0 ; ui < local_size ; ui++)
  {
#ifdef USING_CHIP_SIM
/* { */
    if ( chip_sim_task_is_alive())
    {
#if TRACE_LOW_LEVEL
/* { */
      before = dnx_sand_os_get_time_micro() ;
/* } */
#endif
      if (chip_sim_read(PTR_TO_INT(ul_ptr), (UINT32*)array) )
      {
#if DNX_SAND_LOW_LEVEL_ERR_MSG
/* { */
        logMsg(
          "dnx_sand_physical_read_from_chip() chip_sim_read(0x%X) returned with an error\r\n",
          ul_ptr,0,0,0,0,0) ;
/* } */
#endif  
        ex = DNX_SAND_ERR ;
        goto exit ;
      }
#if TRACE_LOW_LEVEL
/* { */
      after = dnx_sand_os_get_time_micro() ;
      elapsed = after - before;
/* } */
#endif
      array++ ;
      ul_ptr++ ;
    }
    else
    {
      ex = DNX_SAND_ERR ;
      goto exit ;
    }

    DNX_SAND_IGNORE_UNUSED_VAR(local_value);
/* } */
#else
/* { */
    if(big_endian) 
    {
      *array++ = *ul_ptr++ ;
    }
    else
    {
      local_value = *ul_ptr ;
      *array = DNX_SAND_BYTE_SWAP(local_value) ;
      array++;
      ul_ptr++;
    }
/* } */
#endif
  }

#if PRINT_LOW_LEVEL_ACESS
  LOG_INFO(BSL_LS_SOC_MANAGEMENT,
           (BSL_META("\r\n eci_read() base: 0x%X ; offset: 0x%X ; size %d ; data: 0x%X"), base_address, offset, size, *tmp_array));
#endif
  goto exit ;
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_PHYSICAL_READ_FROM_CHIP,
        "error in dnx_sand_physical_read_from_chip(): Cannot access chip", 
         PTR_TO_INT(array),
         PTR_TO_INT(base_address),
         offset,
         size,0,0) ;
  return ex ;
}

/*
 * }
 */

#if DNX_SAND_DEBUG
/* { */

/*
 * Set printing when writing to registers.
 */
void
  dnx_sand_physical_print_when_writing(
    uint32 do_print
  )
{
  Dnx_soc_sand_physical_print_when_writing = do_print;
}

/*
 * Print some debug information.
 */
void
  dnx_sand_physical_access_print(
    void
  )
{

  if (Dnx_soc_sand_big_endian_was_checked == FALSE)
  {
    LOG_CLI((BSL_META("DNX_SAND did NOT checked and NOT decide on CPU BIG/LITTLE endian\n\r")));
  }
  else
  {
    LOG_CLI((BSL_META("DNX_SAND checked and decide on CPU BIG/LITTLE endian\n\r")));
  }

  /*
   */
  if (dnx_sand_system_is_big_endian() == FALSE)
  {
    LOG_CLI((BSL_META("DNX_SAND identifies the CPU as LITTLE endian\n\r")));
  }
  else
  {
    LOG_CLI((BSL_META("DNX_SAND identifies the CPU as BIG endian\n\r")));
  }

  LOG_CLI((BSL_META("Physical access:\n\r")));

  /*
   * WRITE
   */
  LOG_CLI((BSL_META("  + Device write access function:")));

  if (Dnx_soc_sand_physical_access.physical_write == dnx_sand_eci_write)
  {
    LOG_CLI((BSL_META(" ECI - supplied by the driver.")));
  }
  else if (Dnx_soc_sand_physical_access.physical_write == NULL)
  {
    LOG_CLI((BSL_META(" NULL (probably an error).")));
  }
  else
  {
    LOG_CLI((BSL_META(" user supplied.")));
  }
  LOG_CLI((BSL_META("\n\r")));

  /*
   * READ
   */
  LOG_CLI((BSL_META("  + Device read access function:")));

  if (Dnx_soc_sand_physical_access.physical_read == dnx_sand_eci_read)
  {
    LOG_CLI((BSL_META(" ECI - supplied by the driver.")));
  }
  else if (Dnx_soc_sand_physical_access.physical_read == NULL)
  {
    LOG_CLI((BSL_META(" NULL (probably an error).")));
  }
  else
  {
    LOG_CLI((BSL_META(" user supplied.")));
  }
  LOG_CLI((BSL_META("\n\r")));


}


/*
 * {
 * 'dnx_sand_eci_write_and_print', 'dnx_sand_eci_read_and_print' are debug implementation
 *  of the physical interface.
 *  They print the offset/address and call 'dnx_sand_eci_write', 'dnx_sand_eci_read'.
 *
 * Note: Do not use these function - unless you wish to print ALOT to the screen.
 */

DNX_SAND_RET
  dnx_sand_eci_write_and_print(
    DNX_SAND_IN     uint32 *array,
    DNX_SAND_INOUT  uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 size
 )
{
  DNX_SAND_RET
    ex;
  uint32
    access_i;

  /*
   * Print values.
   */
  for(access_i=0; access_i<(size/4); ++access_i)
  {

    LOG_CLI((BSL_META("mem write 0x%04X data 0x%08x offset silent type gp1_fap20v_1 address_in_longs\n"),
             (offset+access_i*4),
             array[access_i]
             ));

  }

  /*
   * Write to device.
   */
  ex = dnx_sand_eci_write(array, base_address, offset, size);
  return ex;
}

DNX_SAND_RET
  dnx_sand_eci_read_and_print(
    DNX_SAND_INOUT  uint32 *array,
    DNX_SAND_IN     uint32 *base_address,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 size
 )
{
  DNX_SAND_RET
    ex;
  uint32
    access_i;

  /*
   * Read from device.
   */
  ex = dnx_sand_eci_read(array, base_address, offset, size);

  /*
   * Print values.
   */
  for(access_i=0; access_i<(size/4); ++access_i)
  {
    LOG_CLI((BSL_META("Read Offset:0x%04X Value:0x%08x\n\r"),
             (offset + 4*access_i),
             array[access_i]
             ));
  }
  return ex;
}

/*
 * }
 */

#ifdef BROADCOM_DEBUG

#ifdef FUNCTION_TIME_PROFILING
#include <stdlib.h>
#include <stdio.h>
#include <sal/core/sync.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
 
typedef struct {
  unsigned long long tot_usec;
  struct timespec last_ts;
} ht_value_t;

typedef struct {
  void *addr;
} ht_key_t;

typedef struct entry_s {
	ht_key_t key;
  ht_value_t value;

	struct entry_s *next;
} entry_t;

static inline unsigned int ht_default_hash_func(const ht_key_t key) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline int ht_default_compare_func(const ht_key_t left, const ht_key_t right) __attribute__((no_instrument_function)) __attribute__((always_inline));

static inline unsigned int ht_default_hash_func(const ht_key_t key) 
{
  return (unsigned int)key.addr;
}

static inline int ht_default_compare_func(const ht_key_t left, const ht_key_t right) 
{
  return left.addr == right.addr ? 0 : 1;
}
 
/* Return the hash of a key (which will later be mapped to the hashtable range with modulo). */ 
typedef unsigned int (*ht_hash_func_t)(const ht_key_t key);
/* Return 0 if equal, non-zero otherwise. */
typedef int (*ht_compare_func_t)(const ht_key_t left, const ht_key_t right);

typedef struct {
	int size;
	struct entry_s **table;	
  
  ht_hash_func_t hash_func;
  ht_compare_func_t compare_func;

} hashtable_t;
 
typedef int (*ht_iter_cb_t)(ht_key_t key, ht_value_t value, void *opaque);

static inline hashtable_t *ht_create(int size, ht_hash_func_t hash_func, ht_compare_func_t compare_func) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline entry_t *ht_newpair( ht_key_t key, ht_value_t value ) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline int ht_set( hashtable_t *hashtable, ht_key_t key, ht_value_t value ) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline ht_value_t ht_get( hashtable_t *hashtable, ht_key_t key, char *found ) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline int ht_iterate(hashtable_t *ht, ht_iter_cb_t cb, void *opaque) __attribute__((no_instrument_function)) __attribute__((always_inline));

/* Create a new hashtable. */
static inline hashtable_t *ht_create(int size, const ht_hash_func_t hash_func, const ht_compare_func_t compare_func) 
{
	hashtable_t *hashtable = NULL;
  ht_hash_func_t hash_func_tmp = hash_func != (ht_hash_func_t)NULL ? hash_func : ht_default_hash_func;
  ht_compare_func_t compare_func_tmp = compare_func != (ht_compare_func_t)NULL ? compare_func : ht_default_compare_func;
	int i;
 
	if (size < 1) {
    return NULL;
  }

  hashtable = malloc(sizeof(hashtable_t));
	if (hashtable == NULL) {
		return NULL;
	}

  hashtable->table = malloc(sizeof(entry_t *) * size);
  if (hashtable->table == NULL ) {
		return NULL;
	}

	for( i = 0; i < size; i++ ) {
		hashtable->table[i] = NULL;
	}
 
	hashtable->size = size;
  hashtable->hash_func = hash_func_tmp;
  hashtable->compare_func = compare_func_tmp;
 
	return hashtable;	
}
 
/* Create a key-value pair. */
static inline entry_t *ht_newpair(ht_key_t key, ht_value_t value)
{
	entry_t *newpair;
 
  newpair = malloc(sizeof(entry_t));
	if (newpair == NULL) {
		return NULL;
	}

  newpair->key = key;
  newpair->value = value;
	newpair->next = NULL;
 
	return newpair;
}
 
/* Insert a key-value pair into a hash table. */
/* Return 0 on success, non-zero on malloc failure */
static inline int ht_set( hashtable_t *hashtable, ht_key_t key, ht_value_t value )
{
	int bin = 0;
	entry_t *newpair = NULL;
	entry_t *next = NULL;
	entry_t *last = NULL;
 
	bin = hashtable->hash_func(key) % hashtable->size;
 
	next = hashtable->table[bin];
 
	while (next != NULL && hashtable->compare_func(key, next->key)) {
		last = next;
		next = next->next;
	}
 
	/* There's already a pair.  Let's replace that string. */
	if (next != NULL && hashtable->compare_func(key, next->key) == 0) {
    next->value = value;

	/* Nope, could't find it.  Time to grow a pair. */
	} else {
		newpair = ht_newpair(key, value);
    if (newpair == NULL) {
      return -1;
    }
 
		/* We're at the start of the linked list in this bin. */
		if(next == hashtable->table[bin]) {
			newpair->next = next;
			hashtable->table[ bin ] = newpair;
	
		/* We're at the end of the linked list in this bin. */
		} else if ( next == NULL ) {
			last->next = newpair;
	
		/* We're in the middle of the list. */
		} else  {
			newpair->next = next;
			last->next = newpair;
		}
	}

  return 0;
}
 
/* Retrieve a key-value pair from a hash table. */
static inline ht_value_t ht_get( hashtable_t *hashtable, ht_key_t key, char *found )
{
	int bin = 0;
	entry_t *pair;
  ht_value_t garbage;
 
	bin = hashtable->hash_func(key) % hashtable->size;
 
	/* Step through the bin, looking for our value. */
	pair = hashtable->table[bin];
	while (pair != NULL && hashtable->compare_func(key, pair->key)) {
		pair = pair->next;
	}
 
	/* Did we actually find anything? */
	if( pair == NULL || hashtable->compare_func(key, pair->key)) {
    *found = 0;
		return garbage;
	} /* else {*/
    *found = 1;
    return pair->value;
  /* } */
}

static inline int ht_iterate(hashtable_t *ht, ht_iter_cb_t cb, void *opaque)
{
  int i;
  entry_t *e;
  for (i=0; i<ht->size; i++) {
    for (e = ht->table[i]; e != NULL; e = e->next) {
      int rv = cb(e->key, e->value, opaque);
      if (rv) {
        return rv;
      }
    }
  }

  return 0;
}
 
static hashtable_t *ht;
static FILE *htb_file;
static char *htb_file_name = "function-call.dump";
enum INSTRUMENT_STATUS{
  /* Before init. */
  INSTRUMENT_STATUS_NEW,
  /* Don't do any instrumentation in internal functions. */
  INSTRUMENT_STATUS_NO_INSTRUMENT,
  /* Do instrumentation for all non-internal functions. */
  INSTRUMENT_STATUS_DO_INSTRUMENT,
  /* There was an error, stop. */
  INSTRUMENT_STATUS_ERROR
};
static int status = INSTRUMENT_STATUS_NEW;

static inline int flush_htb_cb(ht_key_t key, ht_value_t value, void *opaque) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline int flush_htb_cb(ht_key_t key, ht_value_t value, void *opaque)
{
  int rv = fprintf(htb_file, "%p: %llu\n", key.addr, value.tot_usec);
  if (rv == 0){
    printf("Error writing to file!\n");
  }

  return 0;
}

static inline void flush_htb() __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline void flush_htb()
{
  /* if (status == INSTRUMENT_STATUS_DO_INSTRUMENT) { */
    status = INSTRUMENT_STATUS_NO_INSTRUMENT;
    htb_file = fopen(htb_file_name, "w");
    if (htb_file == NULL) {
      printf("Error - Could not open file %s for rw.\n", htb_file_name);
      return;
    }

    ht_iterate(ht, flush_htb_cb, NULL);
    fclose(htb_file);
    status = INSTRUMENT_STATUS_DO_INSTRUMENT;
  /* } */
  
}

/* static sal_mutex_t mutex;    */
   
inline static int init_instrumentation() __attribute__((no_instrument_function)) __attribute__((always_inline));
inline static int init_instrumentation()
{
  char *error_func;
  int rv;

  /* mutex = sal_mutex_create("xxx"); */

  error_func = "atexit";
  rv = atexit(flush_htb);
  if (rv != 0) {
    goto exit;
  }

  error_func = "ht_create";
  ht = ht_create(1000000, NULL, NULL);
  if (ht == NULL) {
    goto exit;
  }
  
exit:
  if (rv != 0) {
    printf("Error - Failed doing %s (%d).\n", error_func, rv);
    return rv;
  } 
  return 0;
}

/* This callback will be called in a thread-safe manner. */
static inline void instrument_enter_callback(void *func) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline void instrument_enter_callback(void *func)
{
  int rv;
  ht_key_t k;
  ht_value_t v;
  char found;
  
  k.addr = func;

  v = ht_get(ht, k, &found);
  if (!found) {
    v.tot_usec = 0;
  }

  rv = clock_gettime(CLOCK_REALTIME, &v.last_ts);

  if (rv) {
    printf("Error in clock_gettime.\n");
    v.last_ts.tv_sec = 0;
    v.last_ts.tv_nsec = 0;
  }

  rv = ht_set(ht, k, v);
  if (rv != 0) {
    printf("Error - Failed doing ht_set (%d).\n", rv);
  }

}

static inline void instrument_exit_callback(void *func) __attribute__((no_instrument_function)) __attribute__((always_inline));
static inline void instrument_exit_callback(void *func) 
{
  int rv;
  ht_key_t k;
  ht_value_t v;
  struct timespec ts;
  long long call_time;
  char found;
  
  k.addr = func;

  v = ht_get(ht, k, &found);
  if (!found) {
    printf("Error - Could not find key at exit_callback.\n");
    return;
  }

  rv = clock_gettime(CLOCK_REALTIME, &ts);
  if (rv) {
    printf("Error in clock_gettime.\n");
    return;
  }

  call_time = (long long)(ts.tv_sec - v.last_ts.tv_sec) * 1000000000 + ts.tv_nsec - v.last_ts.tv_nsec;
  if (call_time < 0) {
    printf("Error - Negative call time!\n");
    return;
  }
  v.tot_usec += call_time;

  rv = ht_set(ht, k, v);
  if (rv != 0) {
    printf("Error - Failed doing ht_set (%d).\n", rv);
  }
}

static int init_status = 0;
static sal_thread_t main_thread;

extern inline void __wrap___cyg_profile_func_enter (void *this_fn, void *call_site) __attribute__((no_instrument_function)) __attribute__((always_inline));
inline void __wrap___cyg_profile_func_enter (void *this_fn, void *call_site)
{
  /* Since this happens before main, it should happen before any threads are spawned. */
  if (init_status == 0) {
    int rv;

    main_thread = sal_thread_self();
    rv = init_instrumentation();
    
    if (rv) {
      printf("Error - Could not initialize instrumentation!\n");
      init_status = -1;
    } else {
      init_status = 1;
    }
  } else if (main_thread != sal_thread_self()) {
    return;
  } else if (init_status == 1) {
    /* sal_mutex_take(mutex, sal_mutex_FOREVER);    */
    instrument_enter_callback(this_fn);
    /* sal_mutex_give(mutex); */
  }
  
}

extern inline void __wrap___cyg_profile_func_exit  (void *this_fn, void *call_site) __attribute__((no_instrument_function)) __attribute__((always_inline));
inline void __wrap___cyg_profile_func_exit  (void *this_fn, void *call_site)
{

  if (init_status == 1 && main_thread == sal_thread_self()) {
    /* sal_mutex_take(mutex, sal_mutex_FOREVER); */
    instrument_exit_callback(this_fn);
    /* sal_mutex_give(mutex); */
  }
}
#endif /* FUNCTION_TIME_PROFILING */
#endif /* BROADCOM_DEBUG */

/* } */
#endif



