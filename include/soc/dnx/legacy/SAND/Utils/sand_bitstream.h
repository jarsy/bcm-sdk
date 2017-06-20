/* $Id: sand_bitstream.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef DNX_SAND_BITSTREAM_H_INCLUDED
/* { */
#define DNX_SAND_BITSTREAM_H_INCLUDED
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

/* $Id: sand_bitstream.h,v 1.4 Broadcom SDK $
 * In 'dnx_sand_bitstream_set_field'/'dnx_sand_bitstream_get_field':
 * 'nof_bits' can be at most 32.
 */
#define DNX_SAND_BIT_STREAM_FIELD_SET_SIZE    (DNX_SAND_NOF_BITS_IN_UINT32)

/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_clear(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in longs */
    );
/*****************************************************
*NAME:
* dnx_sand_bitstream_fill
*DATE:
* 27/OCT/2002
*FUNCTION:
*  clears a bit stream
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of unsigned longs,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size         -
*       size of the array (in longs)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the array of longs is set to all-ones
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_fill(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in longs */
  );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_set(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place,
    DNX_SAND_IN       uint32     bit_indicator
    );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_set_field(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_bit,
    DNX_SAND_IN       uint32    nof_bits,
    DNX_SAND_IN       uint32    field
    );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_get_field(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_bit,
    DNX_SAND_IN       uint32    nof_bits,
    DNX_SAND_OUT      uint32    *field
    );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_set_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
    );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_set_bit_range(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_place,
    DNX_SAND_IN       uint32    end_place
    );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_reset_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
    );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_reset_bit_range(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_place,
    DNX_SAND_IN       uint32    end_place
  );
/*
 */
int
  dnx_sand_bitstream_test_bit(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  );
/*
 */
int
  dnx_sand_bitstream_test_and_reset_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  );
/*
 */
int
  dnx_sand_bitstream_test_and_set_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_bit_flip(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  );
/*
 */
int
  dnx_sand_bitstream_have_one_in_range(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_place,
    DNX_SAND_IN       uint32    end_place
  );
/*
 */
int
  dnx_sand_bitstream_have_one(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in longs */
  );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_or(
    DNX_SAND_INOUT    uint32    *bit_stream1,
    DNX_SAND_IN       uint32    *bit_stream2,
    DNX_SAND_IN       uint32    size /* in longs */
  );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_and(
    DNX_SAND_INOUT    uint32    *bit_stream1,
    DNX_SAND_IN       uint32    *bit_stream2,
    DNX_SAND_IN       uint32    size /* in longs */
  );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_xor(
    DNX_SAND_INOUT    uint32    *bit_stream1,
    DNX_SAND_IN       uint32    *bit_stream2,
    DNX_SAND_IN       uint32    size /* in longs */
  );
/*
 */
uint32
  dnx_sand_bitstream_parity(
    DNX_SAND_IN  uint32    *bit_stream,
    DNX_SAND_IN  uint32    start_bit,
    DNX_SAND_IN  uint32    nof_bits
  );
/*
 */
DNX_SAND_RET
  dnx_sand_bitstream_not(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in longs */
  );
/*
 */

uint32
  dnx_sand_bitstream_get_nof_on_bits(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in longs */
  );
/*
 */

/*****************************************************
*NAME
* dnx_sand_buff_xor
*TYPE:
*  PROC
*DATE:
*  07/03/2006
*FUNCTION:
*  performs bitwise XOR of buff1 and buff2,
*   and stores the result in buff1
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    unsigned  char    *buff1 - first buffer
*    DNX_SAND_IN       unsigned  char    *buff2 - second buffer
*    DNX_SAND_IN       uint32     size  size int bytes
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 - dnx_sand error word
*  DNX_SAND_INDIRECT:
*    buff1 - the result of the XOR function
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_buff_xor(
    DNX_SAND_INOUT    unsigned  char    *buff1,
    DNX_SAND_IN       unsigned  char    *buff2,
    DNX_SAND_IN       uint32     size /* in bytes */
  );


/*****************************************************
*NAME:
* dnx_sand_bitstream_set_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function writes the second buffer
*  to an offset in the first buffer.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  uint32    *input_buffer
*      Input buffer from which the function reads
*    DNX_SAND_IN  uint32    start_bit,
*      The first bit to start the writing in the output buffer
*    DNX_SAND_IN  uint32    nof_bits,
*      Number of bits to read / write
*    DNX_SAND_OUT uint32    *output_buffer
*      Output buffer to which the function writes
*OUTPUT:
*  OK or ERROR indication.
*****************************************************/
uint32
  dnx_sand_bitstream_set_any_field(
    DNX_SAND_IN  uint32    *input_buffer,
    DNX_SAND_IN  uint32    start_bit,
    DNX_SAND_IN  uint32    nof_bits,
    DNX_SAND_OUT uint32    *output_buffer
  );

/*****************************************************
*NAME:
* dnx_sand_bitstream_get_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function reads the first buffer 
*  from an offset and writes it to the second buffer.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  uint32    *input_buffer
*      Input buffer from which the function reads
*    DNX_SAND_IN  uint32    start_bit,
*      The first bit to start the reading in the input buffer
*    DNX_SAND_IN  uint32    nof_bits,
*      Number of bits to read / write
*    DNX_SAND_OUT uint32    *output_buffer
*      Output buffer to which the function writes
*OUTPUT:
*  OK or ERROR indication.
*****************************************************/
uint32
  dnx_sand_bitstream_get_any_field(
    DNX_SAND_IN  uint32    *input_buffer,
    DNX_SAND_IN  uint32    start_bit,
    DNX_SAND_IN  uint32    nof_bits,
    DNX_SAND_OUT uint32    *output_buffer
  );

/*****************************************************
*NAME
* dnx_sand_bitsteam_char_ms_byte_first_get_any_field
*TYPE:
*  PROC
*
*  Having two char buffers, orders msB->lsB, the function reads
*  the first buffer from offset (offset taken from msb) and writes
*  it to the second buffer.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  uint8    *input_buffer -
*      Input buffer from which the function reads
*    DNX_SAND_IN  uint32    start_bit_msb -
*      The first bit to start the reading in the input buffer. This is the msb.
*    DNX_SAND_IN  uint32    nof_bits -
*      Number of bits to read / write. Range: 0-31.
*    DNX_SAND_OUT uint32    *output_value -
*      Output buffer to which the function writes
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    This function should be used when the input buffer is ordered msB->lsB
*    (in contrast to most bitstream functions, which handle a buffer of lsB->msB).
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitsteam_u8_ms_byte_first_get_field(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint8  *input_buffer,
    DNX_SAND_IN  uint32 start_bit_msb,
    DNX_SAND_IN  uint32 nof_bits,
    DNX_SAND_OUT uint32 *output_value
  );

/* } */

#if DNX_SAND_DEBUG
/* { */


/*****************************************************
*NAME
* dnx_sand_bitstream_offline_test
*TYPE:
*  PROC
*DATE:
*  27-Jan-04
*FUNCTION:
*  Test function of dnx_sand_bitstream.
*  View prints and return value to see pass/fail
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*       TRUE  - PASS.
*       FALSE - FAIL
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_bitstream_offline_test(uint32 silent);

/*
 * Printing utility.
 */
void
  dnx_sand_bitstream_print(
    DNX_SAND_IN uint32 *bit_stream,
    DNX_SAND_IN uint32 size /* in longs */
  );

void
  dnx_sand_bitstream_print_beautify_1(
    DNX_SAND_IN uint32 *bit_stream,
    DNX_SAND_IN uint32 size, /* in longs */
    DNX_SAND_IN uint32 max_dec_digits,
    DNX_SAND_IN uint32 max_nof_printed_items
  );

/*****************************************************
*NAME
* dnx_sand_buff_print_non_zero
*TYPE:
*  PROC
*DATE:
*  07/03/2006
*FUNCTION:
*  prints buffer's contents
*  only lines containing non-zero values are printed
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN char* buff - the buffer to be printed
*    DNX_SAND_IN uint32 buff_size - buffer's size in bytes
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 - dnx_sand error word
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    this function is active only when DNX_SAND_DEBUG flag is on.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_buff_print_non_zero(
    DNX_SAND_IN unsigned char* buff,
    DNX_SAND_IN uint32 buff_size
  );

/*****************************************************
*NAME
* dnx_sand_buff_print_all
*TYPE:
*  PROC
*DATE:
*  04/07/2006
*FUNCTION:
*  Prints a given buffer as a set of chars
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN unsigned char* buff - the buffer to print
*    DNX_SAND_IN uint32 buff_size - buffer size in bytes
*    uint32 nof_columns - number of columns in each printed line
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_buff_print_all(
    DNX_SAND_IN unsigned char* buff,
    DNX_SAND_IN uint32 buff_size,
    uint32 nof_columns
  );


#endif /* DNX_SAND_DEBUG */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
/* } DNX_SAND_BITSTREAM_H_INCLUDED*/
#endif
