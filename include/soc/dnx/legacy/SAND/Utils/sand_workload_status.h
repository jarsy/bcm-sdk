/* $Id: sand_workload_status.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_WORKLOAD_STATUS_H_INCLUDED__
/* { */
#define __DNX_SAND_WORKLOAD_STATUS_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

#if DNX_SAND_DEBUG

/* $Id: sand_workload_status.h,v 1.3 Broadcom SDK $
 */
void
  dnx_sand_workload_status_start(
    int unit,
    uint32 total_work_load
  );
/*
 */
void
  dnx_sand_workload_status_run(
    int unit,
    uint32 current_workload
  );

void
  dnx_sand_workload_status_get(
    int  unit,
    uint32 *percent
  );

void
  dnx_sand_workload_status_run_no_print(
    int  unit,
    uint32 current_workload
  );

#ifdef  __cplusplus
}
#endif


#endif /* DNX_SAND_DEBUG */

/* } __DNX_SAND_WORKLOAD_STATUS_H_INCLUDED__*/
#endif
