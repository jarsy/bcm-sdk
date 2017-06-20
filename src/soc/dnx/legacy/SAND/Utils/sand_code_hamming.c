/* $Id: sand_code_hamming.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/



#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_code_hamming.h>
#include <soc/dnx/legacy/SAND/Utils/sand_bitstream.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

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
* dnx_sand_code_hamming_check_data_no_check
* dnx_sand_code_hamming_check_data
*FUNCTION:
*
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT uint32  data[] -
*      Pointer to data buffer of the size 'data_bit_wide'/'data_nof_longs'.
*      It holds the only the data bits has read from the device.
*    DNX_SAND_IN    uint32  data_bit_wide -
*      Number of valid bits in data.
*    DNX_SAND_IN    uint32  data_nof_longs -
*      number of uint32s the 'data_bit_wide' are in.
*      That is, 'data_bit_wide' devided with 32, rounded up.
*    DNX_SAND_IN    uint32  rotated_generation_matrix[] -
*    DNX_SAND_IN    uint32  generation_matrix_bit_search[] -
*    DNX_SAND_IN    uint32  p_bit_wide -
*    DNX_SAND_IN    uint32  e -
*    DNX_SAND_INOUT uint32  work_data[] -
*    DNX_SAND_OUT   uint32* number_of_fixed_errors -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not FAP10M_NO_ERR then
*        specific error codes:
*          None.
*      Otherwise, no error has been detected and device
*        has been written.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
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
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  uint32
    p,
    syndrom,
    bit_to_flip;

  dnx_sand_ret = dnx_sand_code_hamming_get_p_no_check(
               data,
               data_bit_wide,
               data_nof_longs,
               rotated_generation_matrix,
               p_bit_wide,
               work_data,
               &p
             );
  if(dnx_sand_ret)
  {
    goto exit;
  }

  syndrom = p ^ e;
  if(syndrom == 0)
  {
    *number_of_fixed_errors = 0;
    goto exit;
  }

  /*
   * We have an error
   */
  bit_to_flip = generation_matrix_bit_search[syndrom];
  if(bit_to_flip == 0xFFFFFFFF)
  {
    /*ECC bit error - do nothing*/
  }
  else
  {
    /*error in data*/
    dnx_sand_bitstream_bit_flip(data, bit_to_flip);
  }
  *number_of_fixed_errors = 1;


exit:
  return dnx_sand_ret;
}

DNX_SAND_RET
  dnx_sand_code_hamming_check_data(
    DNX_SAND_INOUT uint32  data[],
    DNX_SAND_IN    uint32  data_bit_wide,
    DNX_SAND_IN    uint32  rotated_generation_matrix[],
    DNX_SAND_IN    uint32  generation_matrix_bit_search[],
    DNX_SAND_IN    uint32  p_bit_wide,
    DNX_SAND_IN    uint32  e,
    DNX_SAND_OUT   uint32* number_of_fixed_errors
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  uint32
    p_local_size,
    data_nof_longs,
    *local_buff = NULL;

  data_nof_longs = DNX_SAND_DIV_ROUND_UP(data_bit_wide, DNX_SAND_NOF_BITS_IN_UINT32);
  local_buff = (uint32*) dnx_sand_os_malloc(data_nof_longs * sizeof(uint32));

  dnx_sand_code_hamming_get_p_bit_wide(data_bit_wide, &p_local_size);
  if(p_bit_wide != p_local_size)
  {
    dnx_sand_ret = DNX_SAND_CODE_HAMMING_P_BIT_WIDE_UN_MATCH_ERR;
    goto exit;
  }

  if( (NULL == data)                         ||
      (NULL == rotated_generation_matrix)    ||
      (NULL == generation_matrix_bit_search) ||
      (NULL == number_of_fixed_errors)
    )
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  dnx_sand_ret = dnx_sand_code_hamming_check_data_no_check(
               data,
               data_bit_wide,
               data_nof_longs,
               rotated_generation_matrix,
               generation_matrix_bit_search,
               p_bit_wide,
               e,
               local_buff,
               number_of_fixed_errors
             );
  if(dnx_sand_ret)
  {
    goto exit;
  }

exit:
  DNX_SAND_FREE(local_buff);
  return dnx_sand_ret;
}

/*****************************************************
*NAME
* dnx_sand_code_hamming_get_p()
* dnx_sand_code_hamming_get_p_no_check()
*FUNCTION:
*  1. Multiply data[] vector by rotated_generation_matrix[][] matrix.
*     Return the vector answer. All calculations are in the over GF(2).
*  2. dnx_sand_code_hamming_get_p_no_check() and dnx_sand_code_hamming_get_p()
*     have same functionality as: dnx_sand_code_hamming_get_p().
*     With no input-parameters check (NULL, sizes, ...)
*     This is good for fast calculations.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN    uint32  data[] -
*      'uint32' array. 'data_nof_longs' elemnets in the array.
*      This is the data that need the hamming calculatation.
*    DNX_SAND_IN    uint32  data_bit_wide -
*      Number of valid bits in data.
*    DNX_SAND_IN    uint32  data_nof_longs -
*      number of uint32s the 'data_bit_wide' are in.
*      That is, 'data_bit_wide' devided with 32, rounded up.
*    DNX_SAND_IN    uint32  rotated_generation_matrix[] -
*      Generation matrix. Number of uint32 in the matrix: 'data_bit_wide*p_bit_wide'
*      The matrix is ordered as 'rotated_generation_matrix[p_bit_wide][data_bit_wide]'.
*    DNX_SAND_IN    uint32  p_bit_wide -
*      Number of valid bits in the 'p'
*    DNX_SAND_INOUT uint32  work_data[] -
*      'uint32' array.
*      Use for function calculation. The values are not relevant for in/out.
*      This is done in order that this function will not need to allocate memory of its on.
*      Speed up the function.
*    DNX_SAND_OUT   uint32* p -
*      On return the value ofthe multiplation of 'data[]' with 'rotated_generation_matrix[]'
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      Always DNX_SAND_OK
*SEE ALSO:
*  dnx_sand_code_hamming_get_p()
*****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_get_p(
    DNX_SAND_IN  uint32  data[],
    DNX_SAND_IN  uint32  data_bit_wide,
    DNX_SAND_IN  uint32  rotated_generation_matrix[],
    DNX_SAND_IN  uint32  p_bit_wide,
    DNX_SAND_OUT uint32* p
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  uint32
    p_local = 0,
    data_nof_longs,
    *local_buff = NULL;

  data_nof_longs = DNX_SAND_DIV_ROUND_UP(data_bit_wide, DNX_SAND_NOF_BITS_IN_UINT32);
  local_buff = (uint32*) dnx_sand_os_malloc(data_nof_longs * sizeof(uint32));

  dnx_sand_code_hamming_get_p_bit_wide(data_bit_wide, &p_local);
  if(p_bit_wide != p_local)
  {
    dnx_sand_ret = DNX_SAND_CODE_HAMMING_P_BIT_WIDE_UN_MATCH_ERR;
    goto exit;
  }

  if( (NULL == data)  ||
      (NULL == rotated_generation_matrix) ||
      (NULL == p)
    )
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  dnx_sand_ret = dnx_sand_code_hamming_get_p_no_check(
               data,
               data_bit_wide,
               data_nof_longs,
               rotated_generation_matrix,
               p_bit_wide,
               local_buff,
               p
             );
  if(dnx_sand_ret)
  {
    goto exit;
  }

exit:
  DNX_SAND_FREE(local_buff);
  return dnx_sand_ret;
}

/* $Id: sand_code_hamming.c,v 1.3 Broadcom SDK $
 * refer to dnx_sand_code_hamming_get_p()
 */
DNX_SAND_RET
  dnx_sand_code_hamming_get_p_no_check(
    DNX_SAND_IN    uint32  data[],
    DNX_SAND_IN    uint32  data_bit_wide,
    DNX_SAND_IN    uint32  data_nof_longs,
    DNX_SAND_IN    uint32  rotated_generation_matrix[],
    DNX_SAND_IN    uint32  p_bit_wide,
    DNX_SAND_INOUT uint32  work_data[],
    DNX_SAND_OUT   uint32* p
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  uint32
    p_i,
    nof_ones;
  const uint32
    *generation_matrix_local= NULL;

  generation_matrix_local = rotated_generation_matrix;
  *p=0;

  for(p_i=0; p_i<p_bit_wide; ++p_i)
  {
    dnx_sand_os_memcpy(work_data, data, data_nof_longs * sizeof(uint32));
    dnx_sand_bitstream_and(work_data, generation_matrix_local, data_nof_longs);
    nof_ones = dnx_sand_bitstream_get_nof_on_bits(work_data, data_nof_longs);
    if(dnx_sand_is_even(nof_ones) == FALSE)
    {
      *p |= DNX_SAND_BIT(p_i);
    }

    generation_matrix_local+=data_nof_longs;
  }

  return dnx_sand_ret;
}

/*****************************************************
*NAME
* dnx_sand_code_hamming_get_p_bit_wide
*FUNCTION:
*  Gets '*p_bit_wide', s.t., (2**p_bit_wide >= p_bit_wide+k+1).
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  uint32  data_bit_wide -
*      data bit wide
*    DNX_SAND_OUT uint32* p_bit_wide -
*      pointer to 'uint32'.
*      loaded iwth result - if no error
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      If error code is not DNX_SAND_OK then
*        specific error codes:
*          DNX_SAND_CODE_HAMMING_UN_SUPPORTED_DATA_BIT_WIDE_ERR.
*****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_get_p_bit_wide(
    DNX_SAND_IN  uint32  data_bit_wide,
    DNX_SAND_OUT uint32* p_bit_wide
  )
{
  uint32
    p_local,
    two_power_m_local;
  uint32
    found = FALSE;
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;

  /*
   * Search for p_bit_wide such that: 2**p_bit_wide >= p_bit_wide+k+1
   */
  for(p_local=1; p_local<31; ++p_local)
  {
    two_power_m_local = 2 << p_local;

    if( two_power_m_local >= (data_bit_wide + p_local + 1) )
    {
      found = TRUE;
      break;
    }
  }

  if(found)
  {
    *p_bit_wide = p_local+1;
    dnx_sand_ret = DNX_SAND_OK;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_CODE_HAMMING_UN_SUPPORTED_DATA_BIT_WIDE_ERR;
  }

  return dnx_sand_ret;
}


/*****************************************************
*NAME
* dnx_sand_code_hamming_generate_gen_matrix
*FUNCTION:
*  Create the generation matrix for data with 'data_bit_wide' bits.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  uint32 data_bit_wide -
*      Number of bits in data.
*    DNX_SAND_OUT uint32 gen_mat[] -
*      Pointer to buffer of size 'sizeof(uint32)*data_bit_wide'
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      DNX_SAND_OK, otherwise error
*****************************************************/
DNX_SAND_RET
  dnx_sand_code_hamming_generate_gen_matrix(
    DNX_SAND_IN  uint32 data_bit_wide,
    DNX_SAND_IN  uint32 search_gen_mat_nof_entires,
    DNX_SAND_OUT uint32 gen_mat[],
    DNX_SAND_OUT uint32 search_gen_mat[]
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  uint32
    not_power_of_2,
    bit_i;

  if(gen_mat == NULL)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  for(bit_i=0; bit_i<search_gen_mat_nof_entires; ++bit_i)
  {
    search_gen_mat[bit_i] = 0xFFFFFFFF;
  }

  /*
   * Create generation matrix. Skip all powers of 2.
   */
  not_power_of_2 = 3;
  for(bit_i=0; bit_i<data_bit_wide; ++bit_i)
  {
    while( dnx_sand_is_power_of_2(not_power_of_2) == TRUE )
    {
      not_power_of_2++;
    }
    gen_mat[bit_i] = not_power_of_2;
    search_gen_mat[not_power_of_2] = bit_i;
    not_power_of_2++;
  }

exit:
  return dnx_sand_ret;
}




/* } */




#if DNX_SAND_DEBUG
/* { */
/*
 */

/*****************************************************
*NAME
*  dnx_sand_code_hamming_TEST
*FUNCTION:
*  Unit test of this module.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32 silent -
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      TRUE  - PASS
*      FALSE - FAIL
*****************************************************/
uint32
  dnx_sand_code_hamming_TEST(
    DNX_SAND_IN uint32 silent
  )
{
  uint32
    pass = TRUE;
  uint32
    p_1,
    p_2,
    number_of_fixed_errors;
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;

#define TEST_CODE_HAMMING_P                  (7)
#define TEST_CODE_HAMMING_DATA_BIT_WIDE      (68)
#define TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S (DNX_SAND_DIV_ROUND_UP(TEST_CODE_HAMMING_DATA_BIT_WIDE, DNX_SAND_NOF_BITS_IN_UINT32))
  const uint32
    TEST_code_hamming_gen_mat[TEST_CODE_HAMMING_P][TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S] =
    {
      {0x56AAAD5B, 0xAB555555, 0x0000000A },
      {0x9B33366D, 0xCD999999, 0x0000000C },
      {0xE3C3C78E, 0xF1E1E1E1, 0x00000000 },
      {0x03FC07F0, 0x01FE01FE, 0x0000000F },
      {0x03FFF800, 0x01FFFE00, 0x00000000 },
      {0xFC000000, 0x01FFFFFF, 0x00000000 },
      {0x00000000, 0xFE000000, 0x0000000F }
    };
  uint32
    TEST_code_hamming_search_mat[] =
    {
    /*   0:*/ 0xFFFFFFFF,  /*   1:*/ 0xFFFFFFFF,  /*   2:*/ 0xFFFFFFFF,
    /*   3:*/ 0x00000000,  /*   4:*/ 0xFFFFFFFF,  /*   5:*/ 0x00000001,
    /*   6:*/ 0x00000002,  /*   7:*/ 0x00000003,  /*   8:*/ 0xFFFFFFFF,
    /*   9:*/ 0x00000004,  /*  10:*/ 0x00000005,  /*  11:*/ 0x00000006,
    /*  12:*/ 0x00000007,  /*  13:*/ 0x00000008,  /*  14:*/ 0x00000009,
    /*  15:*/ 0x0000000A,  /*  16:*/ 0xFFFFFFFF,  /*  17:*/ 0x0000000B,
    /*  18:*/ 0x0000000C,  /*  19:*/ 0x0000000D,  /*  20:*/ 0x0000000E,
    /*  21:*/ 0x0000000F,  /*  22:*/ 0x00000010,  /*  23:*/ 0x00000011,
    /*  24:*/ 0x00000012,  /*  25:*/ 0x00000013,  /*  26:*/ 0x00000014,
    /*  27:*/ 0x00000015,  /*  28:*/ 0x00000016,  /*  29:*/ 0x00000017,
    /*  30:*/ 0x00000018,  /*  31:*/ 0x00000019,  /*  32:*/ 0xFFFFFFFF,
    /*  33:*/ 0x0000001A,  /*  34:*/ 0x0000001B,  /*  35:*/ 0x0000001C,
    /*  36:*/ 0x0000001D,  /*  37:*/ 0x0000001E,  /*  38:*/ 0x0000001F,
    /*  39:*/ 0x00000020,  /*  40:*/ 0x00000021,  /*  41:*/ 0x00000022,
    /*  42:*/ 0x00000023,  /*  43:*/ 0x00000024,  /*  44:*/ 0x00000025,
    /*  45:*/ 0x00000026,  /*  46:*/ 0x00000027,  /*  47:*/ 0x00000028,
    /*  48:*/ 0x00000029,  /*  49:*/ 0x0000002A,  /*  50:*/ 0x0000002B,
    /*  51:*/ 0x0000002C,  /*  52:*/ 0x0000002D,  /*  53:*/ 0x0000002E,
    /*  54:*/ 0x0000002F,  /*  55:*/ 0x00000030,  /*  56:*/ 0x00000031,
    /*  57:*/ 0x00000032,  /*  58:*/ 0x00000033,  /*  59:*/ 0x00000034,
    /*  60:*/ 0x00000035,  /*  61:*/ 0x00000036,  /*  62:*/ 0x00000037,
    /*  63:*/ 0x00000038,  /*  64:*/ 0xFFFFFFFF,  /*  65:*/ 0x00000039,
    /*  66:*/ 0x0000003A,  /*  67:*/ 0x0000003B,  /*  68:*/ 0x0000003C,
    /*  69:*/ 0x0000003D,  /*  70:*/ 0x0000003E,  /*  71:*/ 0x0000003F,
    /*  72:*/ 0x00000040,  /*  73:*/ 0x00000041,  /*  74:*/ 0x00000042,
    /*  75:*/ 0x00000043,
    };

#define TEST_CODE_HAMMING_NOF_TESTS (5)
  uint32
    TEST_data_to_test[TEST_CODE_HAMMING_NOF_TESTS][TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S] =
    {
      {0x00000000, 0x00000000, 0x8         }, /*all OK*/
      {0x00000001, 0x00000000, 0x8         }, /*one error*/
      {0x00000000, 0x00001000, 0x8         }, /*one errors*/
      {0x56AAAD5B, 0xAB555555, 0x0000000A, }, /*all OK*/
      {0x00000000, 0x40000000, 0x0         }, /*all OK*/
    },
    TEST_data_to_test_p[TEST_CODE_HAMMING_NOF_TESTS] =
    {
      0x4B,
      0x48,
      0x78,
      0x03,
      0x46,
    },
    TEST_data_p_from_hw[TEST_CODE_HAMMING_NOF_TESTS] =
    {
      0x4B,
      0x4B,
      0x4B,
      0x03,
      0x46,
    },
    TEST_data_answer[TEST_CODE_HAMMING_NOF_TESTS][TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S] =
    {
      {0x00000000, 0x00000000, 0x8         }, /*all OK*/
      {0x00000000, 0x00000000, 0x8         },
      {0x00000000, 0x00000000, 0x8         },
      {0x56AAAD5B, 0xAB555555, 0x0000000A, },
      {0x00000000, 0x40000000, 0x0         }, /*all OK*/
    };
  uint32
    work_data[TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S],
    test_i;


    for(test_i=0; test_i<TEST_CODE_HAMMING_NOF_TESTS; ++test_i)
    {
      dnx_sand_ret = dnx_sand_code_hamming_get_p(
                   TEST_data_to_test[test_i],
                   TEST_CODE_HAMMING_DATA_BIT_WIDE,
                   (uint32*)TEST_code_hamming_gen_mat,
                   TEST_CODE_HAMMING_P,
                   &p_1
                 );
      if(dnx_sand_ret)
      {
        goto exit;
      }

      dnx_sand_ret = dnx_sand_code_hamming_get_p_no_check(
                   TEST_data_to_test[test_i],
                   TEST_CODE_HAMMING_DATA_BIT_WIDE,
                   TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S,
                   (uint32*)TEST_code_hamming_gen_mat,
                   TEST_CODE_HAMMING_P,
                   work_data,
                   &p_2
                 );
      if(dnx_sand_ret)
      {
        goto exit;
      }

      if(p_1 != p_2)
      {
        LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META("dnx_sand_code_hamming_TEST(): ERROR 10\n\r")));
        pass = FALSE;
        goto exit;
      }
      if(p_1 != TEST_data_to_test_p[test_i])
      {
        LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META("dnx_sand_code_hamming_TEST(): ERROR 20\n\r")));
        pass = FALSE;
        goto exit;
      }

      dnx_sand_os_memcpy(work_data, TEST_data_to_test[test_i], TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S*sizeof(uint32));
      dnx_sand_ret = dnx_sand_code_hamming_check_data(
                   work_data,
                   TEST_CODE_HAMMING_DATA_BIT_WIDE,
                   (uint32*)TEST_code_hamming_gen_mat,
                   (uint32*)TEST_code_hamming_search_mat,
                   TEST_CODE_HAMMING_P,
                   TEST_data_p_from_hw[test_i],
                   &number_of_fixed_errors
                );
      if(dnx_sand_ret)
      {
        goto exit;
      }
      if(
          dnx_sand_os_memcmp(
            work_data,
            TEST_data_answer[test_i],
            TEST_CODE_HAMMING_DATA_BIT_NOF_UINT32S*sizeof(uint32)
          ) != 0
        )
      {
        LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META("dnx_sand_code_hamming_TEST(): ERROR 30\n\r")));
        pass = FALSE;
        goto exit;
      }


    }

exit:
  if(dnx_sand_ret)
  {
    pass = FALSE;
  }

  if(pass == FALSE)
  {
    if(!silent)
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META("dnx_sand_code_hamming_TEST(): Fail \n\r")));
    }
  }
  return pass;
}


/*****************************************************
*NAME
*  dnx_sand_code_hamming_print_gen_matrix
*FUNCTION:
*  Print generation matrix.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32 data_bit_wide -
*****************************************************/
void
  dnx_sand_code_hamming_print_gen_matrix(
    DNX_SAND_IN uint32 data_bit_wide
  )
{
  uint32
    bit_i,
    p_bit_wide,
    search_gen_mat_nof_entires,
    p_i,
    data_to_print,
    print_flag;
  uint32
    *gen_mat = NULL,
    *search_gen_mat = NULL;

  if( dnx_sand_code_hamming_get_p_bit_wide(data_bit_wide, &p_bit_wide) )
  {
    LOG_CLI((BSL_META("dnx_sand_code_hamming_generate_gen_matrix():\n\r")));
    LOG_CLI((BSL_META("   ERROR: Unsupported data_bit_wide - %lu \n\r"), data_bit_wide));
    goto exit;
  }
  search_gen_mat_nof_entires = p_bit_wide + data_bit_wide+32;

  gen_mat        = (uint32*) dnx_sand_os_malloc(sizeof(uint32) * data_bit_wide);
  search_gen_mat = (uint32*) dnx_sand_os_malloc(sizeof(uint32) * search_gen_mat_nof_entires);
  if(NULL == gen_mat)
  {
    LOG_CLI((BSL_META("dnx_sand_code_hamming_generate_gen_matrix():\n\r")));
    LOG_CLI((BSL_META("   ERROR: malloc failed: data_bit_wide - %lu \n\r"), data_bit_wide));
    goto exit;
  }

  dnx_sand_os_memset(gen_mat, 0x0, sizeof(uint32) * data_bit_wide);

  /*
   * Create generation matrix
   */
  if(dnx_sand_code_hamming_generate_gen_matrix(data_bit_wide, search_gen_mat_nof_entires, gen_mat, search_gen_mat))
  {
    goto exit;
  }

  /*
   * Print Matrix as comment
   */
  LOG_CLI((BSL_META("/* {\n\r")));
  for(bit_i=0; bit_i<data_bit_wide; ++bit_i)
  {
    data_to_print = gen_mat[bit_i];

    LOG_CLI((BSL_META("%4lu: 0x%08lX  "), bit_i, data_to_print));
    dnx_sand_print_u_long_binary_format(data_to_print, p_bit_wide);
    LOG_CLI((BSL_META("\n\r")));
  }
  LOG_CLI((BSL_META("}*/\n\r")));

  /*
   * Print generation matrix rotated.
   */
  LOG_CLI((BSL_META("#define XXX_CODE_HAMMING_P                  (%lu)\n\r"), p_bit_wide));
  LOG_CLI((BSL_META("#define XXX_CODE_HAMMING_DATA_BIT_WIDE      (%lu)\n\r"), data_bit_wide));
  LOG_CLI((BSL_META("#define XXX_CODE_HAMMING_DATA_BIT_NOF_UINT32S (DNX_SAND_DIV_ROUND_UP(XXX_CODE_HAMMING_DATA_BIT_WIDE, DNX_SAND_NOF_BITS_IN_UINT32))\n\r")));
  LOG_CLI((BSL_META("static const uint32\n\r")));
  LOG_CLI((BSL_META("  XXX_code_hamming_gen_mat[XXX_CODE_HAMMING_P][XXX_CODE_HAMMING_DATA_BIT_NOF_UINT32S] =\n\r")));
  LOG_CLI((BSL_META("{\n\r")));
  for(p_i=0; p_i<p_bit_wide; ++p_i)
  {
    data_to_print = 0;
    print_flag = FALSE;
    LOG_CLI((BSL_META("  {")));
    for(bit_i=0x0; bit_i<data_bit_wide; ++bit_i)
    {
      data_to_print |= ((gen_mat[bit_i] & DNX_SAND_BIT(p_i)) >> p_i) << (bit_i%DNX_SAND_NOF_BITS_IN_UINT32);
      if( ( bit_i+1 == data_bit_wide) /*last loop*/ ||
          ((bit_i+1)%DNX_SAND_NOF_BITS_IN_UINT32 == 0) /*32 bit boundary*/
        )
      {
        print_flag = TRUE;
      }

      if(print_flag)
      {
        /*
        dnx_sand_print_u_long_binary_format(data_to_print, DNX_SAND_NOF_BITS_IN_UINT32);
        LOG_CLI((BSL_META(" ")));
        */
        LOG_CLI((BSL_META("0x%08lX, "), data_to_print));
        print_flag = FALSE;
        data_to_print = 0;
      }
    }
    LOG_CLI((BSL_META("},\n\r")));
  }
  LOG_CLI((BSL_META("};\n\r")));

  /*
   * Print search Matrix
   */
  LOG_CLI((BSL_META("\n\r")));
  LOG_CLI((BSL_META("static const uint32\n\r")));
  LOG_CLI((BSL_META("  XXX_code_hamming_search_mat[] =\n\r")));
  LOG_CLI((BSL_META("{\n\r")));
  for(bit_i=0; bit_i<search_gen_mat_nof_entires; ++bit_i)
  {
    data_to_print = search_gen_mat[bit_i];
    LOG_CLI((BSL_META("/*%4lu:*/ 0x%08lX,  "), bit_i, data_to_print));
    if((bit_i+1)%3 == 0)
    {
      LOG_CLI((BSL_META("\n\r")));
    }
    if(search_gen_mat[bit_i] == (data_bit_wide-1))
    {
      break;
    }
  }
  LOG_CLI((BSL_META("\n\r")));
  LOG_CLI((BSL_META("};\n\r")));
  LOG_CLI((BSL_META("\n\r")));


exit:
  DNX_SAND_FREE(gen_mat);
  DNX_SAND_FREE(search_gen_mat);

  return;
}

/*
 * }
 */
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
