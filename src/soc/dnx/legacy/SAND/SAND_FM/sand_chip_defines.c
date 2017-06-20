/* $Id: sand_chip_defines.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       dnx_sand_chip_defines.c
*
* FILE DESCRIPTION: dnx_sand chip general utilities
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/


#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
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
*   dnx_sand_entity_enum_to_str
*TYPE:
*  PROC
*DATE:
*  30/05/2006
*FUNCTION:
*  returns a string representing the appropriate entity type
*INPUT:
*  DNX_SAND_DIRECT:
*    const char* - the output string representing the entity_type
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    a string representing the appropriate entity type
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
char *
  dnx_sand_entity_enum_to_str(
    DNX_SAND_IN DNX_SAND_DEVICE_ENTITY entity_type
  )
{
  char
    *str = "";

  switch(entity_type)
  {
   case(DNX_SAND_FE1_ENTITY):
      str = "FE1";
      break;
    case(DNX_SAND_FE2_ENTITY):
      str = "FE2";
      break;
    case(DNX_SAND_FE3_ENTITY):
      str = "FE3";
      break;
    case(DNX_SAND_FAP_ENTITY):
      str = "FAP";
      break;
    case(DNX_SAND_FOP_ENTITY):
      str = "FOP";
      break;
    case(DNX_SAND_FIP_ENTITY):
      str = "FIP";
      break;
    case(DNX_SAND_FE13_ENTITY):
      str = "FE13";
      break;
    case(DNX_SAND_DONT_CARE_ENTITY):
    default:
      str = "???";
      break;
  }

  return str;
}

/*****************************************************
*NAME
*  dnx_sand_entity_from_level
*TYPE:
*  PROC
*DATE:
*  30/05/2006
*FUNCTION:
*  Returns dnx_sand device entity corresponding to a given source level type value
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_ENTITY_LEVEL_TYPE level_val - source level type value
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    dnx_sand device entity
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
DNX_SAND_DEVICE_ENTITY
  dnx_sand_entity_from_level(
    DNX_SAND_IN DNX_SAND_ENTITY_LEVEL_TYPE level_val
  )
{
  DNX_SAND_DEVICE_ENTITY
    res = DNX_SAND_DONT_CARE_ENTITY;

  switch(level_val)
  {
    case(DNX_SAND_ACTUAL_FAP_VALUE_1):
    case(DNX_SAND_ACTUAL_FIP_VALUE):
    case(DNX_SAND_ACTUAL_FOP_VALUE):
    case(DNX_SAND_ACTUAL_FAP_VALUE):
      res = DNX_SAND_FAP_ENTITY;
      break;
    case(DNX_SAND_ACTUAL_FE1_VALUE):
      res = DNX_SAND_FE1_ENTITY;
      break;
    case(DNX_SAND_ACTUAL_FE2_VALUE):
    case(DNX_SAND_ACTUAL_FE2_VALUE_1):
      res = DNX_SAND_FE2_ENTITY;
      break;
    case(DNX_SAND_ACTUAL_FE3_VALUE):
      res = DNX_SAND_FE3_ENTITY;
      break;
    default:
      res = DNX_SAND_DONT_CARE_ENTITY;
      break;
  }

  return res;
}

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
