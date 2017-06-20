/* $Id: jer2_arad_scheduler_element_converts.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_SCHEDULER_ELEMENT_CONVERTS_H_INCLUDED__
/* { */
#define __JER2_ARAD_SCHEDULER_ELEMENT_CONVERTS_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
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
*NAME
* jer2_arad_sch_INTERNAL_CLASS_TYPE_to_CLASS_TYPE_convert
* jer2_arad_sch_CLASS_TYPE_to_INTERNAL_CLASS_TYPE_convert
*TYPE:
*  PROC
*DATE:
*  23/10/2007
*FUNCTION:
*  Conversion function between API format and internal table format
*INPUT:
*  JER2_ARAD_SCH_INTERNAL_CLASS_TYPE   *internal_class_type -
*     SCT table format.
*  JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type -
*     API format.
*OUTPUT:
*  converted structure
*REMARKS:
*****************************************************/
DNX_SAND_RET
  jer2_arad_sch_INTERNAL_CLASS_TYPE_to_CLASS_TYPE_convert(
    DNX_SAND_IN     JER2_ARAD_SCH_SCT_TBL_DATA     *internal_class_type,
    DNX_SAND_OUT    JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type
  );
DNX_SAND_RET
  jer2_arad_sch_CLASS_TYPE_to_INTERNAL_CLASS_TYPE_convert(
    DNX_SAND_IN   int                   unit,
    DNX_SAND_IN   JER2_ARAD_SCH_SE_CL_CLASS_INFO *class_type,
    DNX_SAND_OUT  JER2_ARAD_SCH_SCT_TBL_DATA     *internal_class_type
  );

uint32
  jer2_arad_sch_INTERNAL_HR_MODE_to_HR_MODE_convert(
    DNX_SAND_IN     uint32                internal_hr_mode,
    DNX_SAND_OUT    JER2_ARAD_SCH_SE_HR_MODE  *hr_mode
  );

uint32
  jer2_arad_sch_HR_MODE_to_INTERNAL_HR_MODE_convert(
    DNX_SAND_IN    JER2_ARAD_SCH_SE_HR_MODE  hr_mode,
    DNX_SAND_OUT   uint32               *internal_hr_mode
  );

/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_SCHEDULER_ELEMENT_CONVERTS_H_INCLUDED__*/
#endif


