/* $Id: sand_code_hamming.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       soc_sand_code_hamming.h
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


#ifndef __SOC_SAND_CODE_HAMMING_H_INCLUDED__
/* { */
#define __SOC_SAND_CODE_HAMMING_H_INCLUDED__

#include <soc/dpp/SAND/Utils/sand_header.h>

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_framework.h>

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
 * See details in soc_sand_code_hamming.h
 *****************************************************/
SOC_SAND_RET
  soc_sand_code_hamming_check_data_no_check(
    SOC_SAND_INOUT uint32  data[],
    SOC_SAND_IN    uint32  data_bit_wide,
    SOC_SAND_IN    uint32  data_nof_longs,
    SOC_SAND_IN    uint32  rotated_generation_matrix[],
    SOC_SAND_IN    uint32  generation_matrix_bit_search[],
    SOC_SAND_IN    uint32  p_bit_wide,
    SOC_SAND_IN    uint32  e,
    SOC_SAND_INOUT uint32  work_data[],
    SOC_SAND_OUT   uint32* number_of_fixed_errors
  );

/*****************************************************
 * See details in soc_sand_code_hamming.h
 *****************************************************/
SOC_SAND_RET
  soc_sand_code_hamming_check_data(
    SOC_SAND_INOUT uint32  data[],
    SOC_SAND_IN    uint32  data_bit_wide,
    SOC_SAND_IN    uint32  rotated_generation_matrix[],
    SOC_SAND_IN    uint32  generation_matrix_bit_search[],
    SOC_SAND_IN    uint32  p_bit_wide,
    SOC_SAND_IN    uint32  e,
    SOC_SAND_OUT   uint32* number_of_fixed_errors
  );

/*****************************************************
 * See details in soc_sand_code_hamming.h
 *****************************************************/
SOC_SAND_RET
  soc_sand_code_hamming_get_p(
    SOC_SAND_IN  uint32  data[],
    SOC_SAND_IN  uint32  data_bit_wide,
    SOC_SAND_IN  uint32  generation_matrix[],
    SOC_SAND_IN  uint32  m,
    SOC_SAND_OUT uint32* s
  );

/*****************************************************
 * See details in soc_sand_code_hamming.h
 *****************************************************/
SOC_SAND_RET
  soc_sand_code_hamming_get_p_no_check(
    SOC_SAND_IN    uint32  data[],
    SOC_SAND_IN    uint32  data_bit_wide,
    SOC_SAND_IN    uint32  data_nof_longs,
    SOC_SAND_IN    uint32  generation_matrix[],
    SOC_SAND_IN    uint32  p,
    SOC_SAND_INOUT uint32  work_data[],
    SOC_SAND_OUT   uint32* s
  );

/*****************************************************
 * See details in soc_sand_code_hamming.h
 *****************************************************/
SOC_SAND_RET
  soc_sand_code_hamming_get_p_bit_wide(
    SOC_SAND_IN  uint32  data_bit_wide,
    SOC_SAND_OUT uint32* p
  );


/*****************************************************
 * See details in soc_sand_code_hamming.h
 *****************************************************/
SOC_SAND_RET
  soc_sand_code_hamming_generate_gen_matrix(
    SOC_SAND_IN  uint32 data_bit_wide,
    SOC_SAND_IN  uint32 search_gen_mat_nof_entires,
    SOC_SAND_OUT uint32 gen_mat[],
    SOC_SAND_OUT uint32 search_gen_mat[]
  );

/* } */


#if defined (SOC_SAND_DEBUG)
/* { */


/*****************************************************
 * See details in soc_sand_code_hamming.h
 *****************************************************/
uint32
  soc_sand_code_hamming_TEST(
    SOC_SAND_IN uint32 silent
  );

/*****************************************************
 * See details in soc_sand_code_hamming.h
 *****************************************************/
void
  soc_sand_code_hamming_print_gen_matrix(
    SOC_SAND_IN uint32 data_bit_wide
  );


/* } */
#endif

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_SAND_CODE_HAMMING_H_INCLUDED__*/
#endif
