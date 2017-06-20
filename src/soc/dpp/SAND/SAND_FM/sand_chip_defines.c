/* $Id: sand_chip_defines.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       soc_sand_chip_defines.c
*
* FILE DESCRIPTION: soc_sand chip general utilities
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/


#include <soc/dpp/SAND/Utils/sand_header.h>

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/SAND_FM/sand_chip_defines.h>
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
*   soc_sand_entity_enum_to_str
*TYPE:
*  PROC
*DATE:
*  30/05/2006
*FUNCTION:
*  returns a string representing the appropriate entity type
*INPUT:
*  SOC_SAND_DIRECT:
*    const char* - the output string representing the entity_type
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    a string representing the appropriate entity type
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
char *
  soc_sand_entity_enum_to_str(
    SOC_SAND_IN SOC_SAND_DEVICE_ENTITY entity_type
  )
{
  char
    *str = "";

  switch(entity_type)
  {
   case(SOC_SAND_FE1_ENTITY):
      str = "FE1";
      break;
    case(SOC_SAND_FE2_ENTITY):
      str = "FE2";
      break;
    case(SOC_SAND_FE3_ENTITY):
      str = "FE3";
      break;
    case(SOC_SAND_FAP_ENTITY):
      str = "FAP";
      break;
    case(SOC_SAND_FOP_ENTITY):
      str = "FOP";
      break;
    case(SOC_SAND_FIP_ENTITY):
      str = "FIP";
      break;
    case(SOC_SAND_FE13_ENTITY):
      str = "FE13";
      break;
    case(SOC_SAND_DONT_CARE_ENTITY):
    default:
      str = "???";
      break;
  }

  return str;
}

/*****************************************************
*NAME
*  soc_sand_entity_from_level
*TYPE:
*  PROC
*DATE:
*  30/05/2006
*FUNCTION:
*  Returns soc_sand device entity corresponding to a given source level type value
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_ENTITY_LEVEL_TYPE level_val - source level type value
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    soc_sand device entity
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
SOC_SAND_DEVICE_ENTITY
  soc_sand_entity_from_level(
    SOC_SAND_IN SOC_SAND_ENTITY_LEVEL_TYPE level_val
  )
{
  SOC_SAND_DEVICE_ENTITY
    res = SOC_SAND_DONT_CARE_ENTITY;

  switch(level_val)
  {
    case(SOC_SAND_ACTUAL_FAP_VALUE_1):
    case(SOC_SAND_ACTUAL_FIP_VALUE):
    case(SOC_SAND_ACTUAL_FOP_VALUE):
    case(SOC_SAND_ACTUAL_FAP_VALUE):
      res = SOC_SAND_FAP_ENTITY;
      break;
    case(SOC_SAND_ACTUAL_FE1_VALUE):
      res = SOC_SAND_FE1_ENTITY;
      break;
    case(SOC_SAND_ACTUAL_FE2_VALUE):
    case(SOC_SAND_ACTUAL_FE2_VALUE_1):
      res = SOC_SAND_FE2_ENTITY;
      break;
    case(SOC_SAND_ACTUAL_FE3_VALUE):
      res = SOC_SAND_FE3_ENTITY;
      break;
    default:
      res = SOC_SAND_DONT_CARE_ENTITY;
      break;
  }

  return res;
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>
