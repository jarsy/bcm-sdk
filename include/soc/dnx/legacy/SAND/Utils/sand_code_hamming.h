/* $Id: sand_code_hamming.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       dnx_sand_code_hamming.h
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/


#ifndef __DNX_SAND_CODE_HAMMING_H_INCLUDED__
/* { */
#define __DNX_SAND_CODE_HAMMING_H_INCLUDED__

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

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
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_check_data_no_check(
    DNX_SAND_INOUT uint32  data[],
    DNX_SAND_IN    uint32  data_bit_wide,
    DNX_SAND_IN    uint32  data_nof_longs,
    DNX_SAND_IN    uint32  rotated_generation_matrix[],
    DNX_SAND_IN    uint32  generation_matrix_bit_search[],
    DNX_SAND_IN    uint32  p_bit_wide,
    DNX_SAND_IN    uint32  e,
    DNX_SAND_INOUT uint32  work_data[],
    DNX_SAND_OUT   uint32* number_of_fixed_errors
  );

/*****************************************************
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_check_data(
    DNX_SAND_INOUT uint32  data[],
    DNX_SAND_IN    uint32  data_bit_wide,
    DNX_SAND_IN    uint32  rotated_generation_matrix[],
    DNX_SAND_IN    uint32  generation_matrix_bit_search[],
    DNX_SAND_IN    uint32  p_bit_wide,
    DNX_SAND_IN    uint32  e,
    DNX_SAND_OUT   uint32* number_of_fixed_errors
  );

/*****************************************************
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_get_p(
    DNX_SAND_IN  uint32  data[],
    DNX_SAND_IN  uint32  data_bit_wide,
    DNX_SAND_IN  uint32  generation_matrix[],
    DNX_SAND_IN  uint32  m,
    DNX_SAND_OUT uint32* s
  );

/*****************************************************
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_get_p_no_check(
    DNX_SAND_IN    uint32  data[],
    DNX_SAND_IN    uint32  data_bit_wide,
    DNX_SAND_IN    uint32  data_nof_longs,
    DNX_SAND_IN    uint32  generation_matrix[],
    DNX_SAND_IN    uint32  p,
    DNX_SAND_INOUT uint32  work_data[],
    DNX_SAND_OUT   uint32* s
  );

/*****************************************************
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_get_p_bit_wide(
    DNX_SAND_IN  uint32  data_bit_wide,
    DNX_SAND_OUT uint32* p
  );


/*****************************************************
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_generate_gen_matrix(
    DNX_SAND_IN  uint32 data_bit_wide,
    DNX_SAND_IN  uint32 search_gen_mat_nof_entires,
    DNX_SAND_OUT uint32 gen_mat[],
    DNX_SAND_OUT uint32 search_gen_mat[]
  );

/* } */


#if defined (DNX_SAND_DEBUG)
/* { */


/*****************************************************
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
uint32
  dnx_sand_code_hamming_TEST(
    DNX_SAND_IN uint32 silent
  );

/*****************************************************
 * See details in dnx_sand_code_hamming.h
 *****************************************************/
void
  dnx_sand_code_hamming_print_gen_matrix(
    DNX_SAND_IN uint32 data_bit_wide
  );


/* } */
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_SAND_CODE_HAMMING_H_INCLUDED__*/
#endif
