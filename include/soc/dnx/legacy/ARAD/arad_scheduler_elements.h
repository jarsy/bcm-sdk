/* $Id: jer2_arad_scheduler_elements.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_SCHEDULER_ELEMENTS_H_INCLUDED__
/* { */
#define __JER2_ARAD_SCHEDULER_ELEMENTS_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

#define JER2_ARAD_SCH_SE_IS_CL(se_id) DNX_SAND_IS_VAL_IN_RANGE(se_id, JER2_ARAD_CL_SE_ID_MIN,JER2_ARAD_CL_SE_ID_MAX)
#define JER2_ARAD_SCH_SE_IS_HR(se_id) DNX_SAND_IS_VAL_IN_RANGE(se_id, JER2_ARAD_HR_SE_ID_MIN,JER2_ARAD_HR_SE_ID_MAX)
#define JER2_ARAD_SCH_SE_IS_FQ(se_id) DNX_SAND_IS_VAL_IN_RANGE(se_id, JER2_ARAD_FQ_SE_ID_MIN,JER2_ARAD_FQ_SE_ID_MAX)

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/*****************************************************
* NAME
*   jer2_arad_sch_se_state_get
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Get scheduler element state (enabled/disabled) from the device
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     int             core -
*     Identifier of core on device to access.
*   DNX_SAND_IN     JER2_ARAD_SCH_SE_ID          se_ndx -
*     Scheduling element index. Range: 0 - 16K-1.
*   DNX_SAND_OUT    uint8                   *is_se_enabled -
*     TRUE if the scheduling element is enabled
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_se_state_get(
    DNX_SAND_IN     int                unit,
    DNX_SAND_IN     int                core,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_ID     se_ndx,
    DNX_SAND_OUT    uint8              *is_se_enabled
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_state_set
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Set scheduler element state (enabled/disabled) to the device
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     JER2_ARAD_SCH_FLOW_ID          se_ndx -
*     Scheduling element index. Range: 0 - 16K-1.
*   DNX_SAND_OUT    uint8                   is_se_enabled -
*     TRUE if the scheduling element is enabled.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    It is assumed that se_ndx is verified by the calling function.
*****************************************************/
uint32
  jer2_arad_sch_se_state_set(
    DNX_SAND_IN     int                unit,
    DNX_SAND_IN     int                core,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_IN     uint8                is_se_enabled
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_dual_shaper_get
* TYPE:
*   PROC
* DATE:
*   13/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     int             core -
*     Identifier of core on device to access.
*   DNX_SAND_IN   JER2_ARAD_SCH_SE_ID    se_ndx -
*     Scheduling element index. Range: 0 - 16K-1.
*   DNX_SAND_OUT    uint8                   *is_dual_shaper -
*     True if the aggregate is configured as dual shaper
*     (according to CIR/EIR configuration).
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    It is assumed that se_ndx is verified by the calling function.
*****************************************************/
uint32
  jer2_arad_sch_se_dual_shaper_get(
    DNX_SAND_IN     int                unit,
    DNX_SAND_IN     int                core,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_ID          se_ndx,
    DNX_SAND_IN     JER2_ARAD_SCH_SE_TYPE        se_type,
    DNX_SAND_OUT    uint8                *is_dual_shaper
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_dual_shaper_set
* TYPE:
*   PROC
* DATE:
*   13/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   DNX_SAND_IN   JER2_ARAD_SCH_SE_ID    se_ndx -
*     Scheduling element index. Range: 0 - 16K-1.
*   DNX_SAND_IN    uint8                   is_dual_shaper -
*     True if the aggregate is configured as dual shaper
*     (according to CIR/EIR configuration).
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_se_dual_shaper_set(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     int                   core,
    DNX_SAND_IN     JER2_ARAD_SCH_FLOW_ID          se_ndx,
    DNX_SAND_IN     uint8                   is_dual_shaper
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_config_get
* TYPE:
*   PROC
* DATE:
*   13/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     int             core -
*     Identifier of core on device to access.
*   DNX_SAND_IN   JER2_ARAD_SCH_SE_ID    se_ndx -
*     Scheduling element index. Range: 0 - 16K-1.
*   DNX_SAND_OUT  JER2_ARAD_SCH_SE_INFO  *se -
*     Scheduling element info.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    It is assumed that se_ndx is verified by the calling function.
*****************************************************/
uint32
  jer2_arad_sch_se_config_get(
    DNX_SAND_IN   int          unit,
    DNX_SAND_IN   int          core,
    DNX_SAND_IN   JER2_ARAD_SCH_SE_ID    se_ndx,
    DNX_SAND_IN   JER2_ARAD_SCH_SE_TYPE  se_type,
    DNX_SAND_OUT  JER2_ARAD_SCH_SE_INFO  *se
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_config_set
* TYPE:
*   PROC
* DATE:
*   13/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN   JER2_ARAD_SCH_SE_INFO  *se -
*     Scheduling element info.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Dual shaper configuration (part of the se configuration)
*   is only set when the spouse aggregate is enabled -
*   after enabling the second aggregate.
*   When the spouse is not enabled, dual shaper configuration  will be unset.
*****************************************************/
uint32
  jer2_arad_sch_se_config_set(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  int              core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO     *se,
    DNX_SAND_IN  uint32              nof_subflows
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_verify_unsafe
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Verify scheduling element info.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO      *se -
*     scheduling element info.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_se_verify_unsafe(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO      *se
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_get_unsafe
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     int             core -
*     Identifier of core on device to access.
*   DNX_SAND_IN   JER2_ARAD_SCH_SE_ID    se_ndx -
*     Scheduling element index. Range: 0 - 16K-1.
*   DNX_SAND_OUT  JER2_ARAD_SCH_SE_INFO  *se -
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_se_get_unsafe(
    DNX_SAND_IN   int         unit,
    DNX_SAND_IN   int         core,
    DNX_SAND_IN   JER2_ARAD_SCH_SE_ID    se_ndx,
    DNX_SAND_OUT  JER2_ARAD_SCH_SE_INFO  *se
  );

/*****************************************************
* NAME
*   jer2_arad_sch_se_set_unsafe
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   DNX_SAND_IN     int             unit -
*     Identifier of device to access.
*   DNX_SAND_IN     JER2_ARAD_SCH_SE_INFO      *se -
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_se_set_unsafe(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  int              core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_INFO      *se,
    DNX_SAND_IN  uint32              nof_subflows
  );

/* CL { */

/*****************************************************
* NAME
*   jer2_arad_sch_se_id_and_type_match_verify
* TYPE:
*   PROC
* DATE:
*   24/10/2007
* FUNCTION:
*  Verify the id is in the range
*   that matches se_type id-s range
* INPUT:
*   JER2_ARAD_SCH_SE_ID                   se_id -
*   JER2_ARAD_SCH_SE_TYPE                 se_type -
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_sch_se_id_and_type_match_verify(
    JER2_ARAD_SCH_SE_ID                   se_id,
    JER2_ARAD_SCH_SE_TYPE                 se_type
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_class_type_params_verify
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Sets a single class type in the table. The driver writes
*     to the following tables: CL-Schedulers Type (SCT)
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_TYPE_ID cl_type_ndx -
*     The index of the class type to configure (0-255).
*  DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type -
*     A Scheduler class type.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_TYPE_ID cl_type_ndx,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_class_type_params_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Sets a single class type in the table. The driver writes
*     to the following tables: CL-Schedulers Type (SCT)
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type -
*     A Scheduler class type.
*     The class index is part of the class_type definition
*  DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_INFO *exact_class_type -
*     Loaded with the actual parameters given difference due
*     to rounding.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_INFO *exact_class_type
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_class_type_params_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See jer2_arad_sch_class_type_params_set_unsafe
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  int                 unit -
*     Identifier of core on device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_TYPE_ID cl_type_ndx -
*     The index of the class type to configure (0-255).
*  DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type -
*     A Scheduler class type.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_CL_CLASS_TYPE_ID cl_type_ndx,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_class_type_params_table_verify
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See jer2_arad_sch_class_type_params_table_set
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct -
*     A Scheduler class type table.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_table_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_class_type_params_table_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Sets the scheduler class type table as a whole.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct -
*     A Scheduler class type table.
*  DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_TABLE *exact_sct -
*     Loaded with the actual parameters given difference due
*     to rounding.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_table_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_TABLE *exact_sct
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_class_type_params_table_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See jer2_arad_sch_class_type_params_table_set_unsafe
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct -
*     A Scheduler class type table.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_class_type_params_table_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT JER2_ARAD_SCH_SE_CL_CLASS_TABLE *sct
  );

/* CL } */

/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_SCHEDULER_ELEMENTS_H_INCLUDED__*/
#endif


