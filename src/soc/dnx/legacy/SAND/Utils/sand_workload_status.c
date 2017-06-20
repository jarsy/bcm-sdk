/* $Id: sand_workload_status.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/



#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_workload_status.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#if DNX_SAND_DEBUG
/* { */
/* $Id: sand_workload_status.c,v 1.5 Broadcom SDK $
 */


/*
 * Total work to be done, per device.
 */
static
  uint32
    Dnx_soc_sand_workload_status_total[DNX_SAND_MAX_DEVICE]
  = {0};

/*
 * Current percent of work that we already did, per device
 */
static
  uint32
    Dnx_soc_sand_workload_status_percent[DNX_SAND_MAX_DEVICE]
  = {0};


/*
 * Printing workload as percentage of total work to be done, per device.
 * Note, It is user responsibility to prevent from several printing to occur together.
 */
static
  void
    dnx_sand_workload_status_print(
      int unit
    )
{
  if(unit < DNX_SAND_MAX_DEVICE)
  {
    LOG_CLI((BSL_META_U(unit,
                        "\r%3u%%\r"),
             Dnx_soc_sand_workload_status_percent[unit]
             ));
  }
}
/*
 * Set the total work to be done, per device.
 */
void
  dnx_sand_workload_status_start(
    int  unit,
    uint32 total_work_load
  )
{
  if(unit >= DNX_SAND_MAX_DEVICE)
  {
    goto exit;
  }

  Dnx_soc_sand_workload_status_total[unit] = total_work_load;
  if (0 == Dnx_soc_sand_workload_status_total[unit])
  {
    Dnx_soc_sand_workload_status_total[unit] = 1;
  }
  /*
   */
  Dnx_soc_sand_workload_status_percent[unit] = 0;

  dnx_sand_workload_status_print(unit);

exit:
  return;
}

/*
 * Advances workload per device.
 */
void
  dnx_sand_workload_status_run_no_print(
    int  unit,
    uint32 current_workload
  )
{
  uint32
    percent;
  /*
   */
  percent = 0;
  /*
   */

  if(unit >= DNX_SAND_MAX_DEVICE)
  {
    goto exit;
  }


  /*
   */
  if (0 == Dnx_soc_sand_workload_status_total[unit])
  {
    percent = 100;
  }
  else
  {
    /*
     * Avoid overflow/underflow.
     */
    if ( current_workload < (0xFFFFFFFF/100))
    {
      percent = (current_workload * 100) / Dnx_soc_sand_workload_status_total[unit] ;
    }
    else
    {
      percent = current_workload  / (Dnx_soc_sand_workload_status_total[unit]/100) ;
    }
  }

  Dnx_soc_sand_workload_status_percent[unit] = percent;

exit:
  return;

}

void
  dnx_sand_workload_status_get(
    int  unit,
    uint32 *percent
  )
{
  if(unit >= DNX_SAND_MAX_DEVICE)
  {
    goto exit;
  }

  *percent = Dnx_soc_sand_workload_status_percent[unit];

exit:
  return;
}

/*
 * Advances workload and prints percentage if change detected, per device.
 */
void
  dnx_sand_workload_status_run(
    int  unit,
    uint32 current_workload
  )
{
  uint32
    percent_old,
    percent_new;

  if (unit >= DNX_SAND_MAX_DEVICE)
  {
    goto exit;
  }

  dnx_sand_workload_status_get(unit, &percent_old);
  dnx_sand_workload_status_run_no_print(unit, current_workload);
  dnx_sand_workload_status_get(unit, &percent_new);

  if (percent_old < percent_new)
  {
    dnx_sand_workload_status_print(unit);
  }

exit:
  return;

}

/* } */
#else
/* { */

/*
 * If not in debug mode empty implementation.
 */

void
  dnx_sand_workload_status_start(
    int unit,
    uint32 total_work_load
  )
{
  return;
}

void
  dnx_sand_workload_status_run(
    int unit,
    uint32 current_workload
  )
{
  return;
}

void
  dnx_sand_workload_status_get(
    int  unit,
    uint32 *percent
  )
{
  return;
}

void
  dnx_sand_workload_status_run_no_print(
    int  unit,
    uint32 current_workload
  )
{
  return;
}


/* } */
#endif

