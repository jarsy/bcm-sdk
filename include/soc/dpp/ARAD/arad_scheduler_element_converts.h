/* $Id: arad_scheduler_element_converts.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_SCHEDULER_ELEMENT_CONVERTS_H_INCLUDED__
/* { */
#define __ARAD_SCHEDULER_ELEMENT_CONVERTS_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
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
* arad_sch_INTERNAL_CLASS_TYPE_to_CLASS_TYPE_convert
* arad_sch_CLASS_TYPE_to_INTERNAL_CLASS_TYPE_convert
*TYPE:
*  PROC
*DATE:
*  23/10/2007
*FUNCTION:
*  Conversion function between API format and internal table format
*INPUT:
*  ARAD_SCH_INTERNAL_CLASS_TYPE   *internal_class_type -
*     SCT table format.
*  ARAD_SCH_SE_CL_CLASS_INFO *class_type -
*     API format.
*OUTPUT:
*  converted structure
*REMARKS:
*****************************************************/
SOC_SAND_RET
  arad_sch_INTERNAL_CLASS_TYPE_to_CLASS_TYPE_convert(
    SOC_SAND_IN     ARAD_SCH_SCT_TBL_DATA     *internal_class_type,
    SOC_SAND_OUT    ARAD_SCH_SE_CL_CLASS_INFO *class_type
  );
SOC_SAND_RET
  arad_sch_CLASS_TYPE_to_INTERNAL_CLASS_TYPE_convert(
    SOC_SAND_IN   int                   unit,
    SOC_SAND_IN   ARAD_SCH_SE_CL_CLASS_INFO *class_type,
    SOC_SAND_OUT  ARAD_SCH_SCT_TBL_DATA     *internal_class_type
  );

uint32
  arad_sch_INTERNAL_HR_MODE_to_HR_MODE_convert(
    SOC_SAND_IN     uint32                internal_hr_mode,
    SOC_SAND_OUT    ARAD_SCH_SE_HR_MODE  *hr_mode
  );

uint32
  arad_sch_HR_MODE_to_INTERNAL_HR_MODE_convert(
    SOC_SAND_IN    ARAD_SCH_SE_HR_MODE  hr_mode,
    SOC_SAND_OUT   uint32               *internal_hr_mode
  );

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_SCHEDULER_ELEMENT_CONVERTS_H_INCLUDED__*/
#endif


