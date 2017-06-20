/* $Id: sand_bitstream.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef SOC_SAND_BITSTREAM_H_INCLUDED
/* { */
#define SOC_SAND_BITSTREAM_H_INCLUDED
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>

/* $Id: sand_bitstream.h,v 1.4 Broadcom SDK $
 * In 'soc_sand_bitstream_set_field'/'soc_sand_bitstream_get_field':
 * 'nof_bits' can be at most 32.
 */
#define SOC_SAND_BIT_STREAM_FIELD_SET_SIZE    (SOC_SAND_NOF_BITS_IN_UINT32)

/*
 */
SOC_SAND_RET
  soc_sand_bitstream_clear(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in longs */
    );
/*****************************************************
*NAME:
* soc_sand_bitstream_fill
*DATE:
* 27/OCT/2002
*FUNCTION:
*  clears a bit stream
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of unsigned longs,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size         -
*       size of the array (in longs)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the array of longs is set to all-ones
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_fill(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in longs */
  );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_set(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place,
    SOC_SAND_IN       uint32     bit_indicator
    );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_set_field(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_bit,
    SOC_SAND_IN       uint32    nof_bits,
    SOC_SAND_IN       uint32    field
    );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_get_field(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_bit,
    SOC_SAND_IN       uint32    nof_bits,
    SOC_SAND_OUT      uint32    *field
    );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_set_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
    );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_set_bit_range(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_place,
    SOC_SAND_IN       uint32    end_place
    );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_reset_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
    );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_reset_bit_range(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_place,
    SOC_SAND_IN       uint32    end_place
  );
/*
 */
int
  soc_sand_bitstream_test_bit(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  );
/*
 */
int
  soc_sand_bitstream_test_and_reset_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  );
/*
 */
int
  soc_sand_bitstream_test_and_set_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_bit_flip(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  );
/*
 */
int
  soc_sand_bitstream_have_one_in_range(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_place,
    SOC_SAND_IN       uint32    end_place
  );
/*
 */
int
  soc_sand_bitstream_have_one(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in longs */
  );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_or(
    SOC_SAND_INOUT    uint32    *bit_stream1,
    SOC_SAND_IN       uint32    *bit_stream2,
    SOC_SAND_IN       uint32    size /* in longs */
  );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_and(
    SOC_SAND_INOUT    uint32    *bit_stream1,
    SOC_SAND_IN       uint32    *bit_stream2,
    SOC_SAND_IN       uint32    size /* in longs */
  );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_xor(
    SOC_SAND_INOUT    uint32    *bit_stream1,
    SOC_SAND_IN       uint32    *bit_stream2,
    SOC_SAND_IN       uint32    size /* in longs */
  );
/*
 */
uint32
  soc_sand_bitstream_parity(
    SOC_SAND_IN  uint32    *bit_stream,
    SOC_SAND_IN  uint32    start_bit,
    SOC_SAND_IN  uint32    nof_bits
  );
/*
 */
SOC_SAND_RET
  soc_sand_bitstream_not(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in longs */
  );
/*
 */

uint32
  soc_sand_bitstream_get_nof_on_bits(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in longs */
  );
/*
 */

/*****************************************************
*NAME
* soc_sand_buff_xor
*TYPE:
*  PROC
*DATE:
*  07/03/2006
*FUNCTION:
*  performs bitwise XOR of buff1 and buff2,
*   and stores the result in buff1
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    unsigned  char    *buff1 - first buffer
*    SOC_SAND_IN       unsigned  char    *buff2 - second buffer
*    SOC_SAND_IN       uint32     size  size int bytes
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 - soc_sand error word
*  SOC_SAND_INDIRECT:
*    buff1 - the result of the XOR function
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_buff_xor(
    SOC_SAND_INOUT    unsigned  char    *buff1,
    SOC_SAND_IN       unsigned  char    *buff2,
    SOC_SAND_IN       uint32     size /* in bytes */
  );


/*****************************************************
*NAME:
* soc_sand_bitstream_set_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function writes the second buffer
*  to an offset in the first buffer.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN  uint32    *input_buffer
*      Input buffer from which the function reads
*    SOC_SAND_IN  uint32    start_bit,
*      The first bit to start the writing in the output buffer
*    SOC_SAND_IN  uint32    nof_bits,
*      Number of bits to read / write
*    SOC_SAND_OUT uint32    *output_buffer
*      Output buffer to which the function writes
*OUTPUT:
*  OK or ERROR indication.
*****************************************************/
uint32
  soc_sand_bitstream_set_any_field(
    SOC_SAND_IN  uint32    *input_buffer,
    SOC_SAND_IN  uint32    start_bit,
    SOC_SAND_IN  uint32    nof_bits,
    SOC_SAND_OUT uint32    *output_buffer
  );

/*****************************************************
*NAME:
* soc_sand_bitstream_get_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function reads the first buffer 
*  from an offset and writes it to the second buffer.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN  uint32    *input_buffer
*      Input buffer from which the function reads
*    SOC_SAND_IN  uint32    start_bit,
*      The first bit to start the reading in the input buffer
*    SOC_SAND_IN  uint32    nof_bits,
*      Number of bits to read / write
*    SOC_SAND_OUT uint32    *output_buffer
*      Output buffer to which the function writes
*OUTPUT:
*  OK or ERROR indication.
*****************************************************/
uint32
  soc_sand_bitstream_get_any_field(
    SOC_SAND_IN  uint32    *input_buffer,
    SOC_SAND_IN  uint32    start_bit,
    SOC_SAND_IN  uint32    nof_bits,
    SOC_SAND_OUT uint32    *output_buffer
  );

/*****************************************************
*NAME
* soc_sand_bitsteam_char_ms_byte_first_get_any_field
*TYPE:
*  PROC
*
*  Having two char buffers, orders msB->lsB, the function reads
*  the first buffer from offset (offset taken from msb) and writes
*  it to the second buffer.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN  uint8    *input_buffer -
*      Input buffer from which the function reads
*    SOC_SAND_IN  uint32    start_bit_msb -
*      The first bit to start the reading in the input buffer. This is the msb.
*    SOC_SAND_IN  uint32    nof_bits -
*      Number of bits to read / write. Range: 0-31.
*    SOC_SAND_OUT uint32    *output_value -
*      Output buffer to which the function writes
*OUTPUT:
*  SOC_SAND_DIRECT:
*    error indication
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*    This function should be used when the input buffer is ordered msB->lsB
*    (in contrast to most bitstream functions, which handle a buffer of lsB->msB).
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitsteam_u8_ms_byte_first_get_field(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint8  *input_buffer,
    SOC_SAND_IN  uint32 start_bit_msb,
    SOC_SAND_IN  uint32 nof_bits,
    SOC_SAND_OUT uint32 *output_value
  );

/* } */

#if SOC_SAND_DEBUG
/* { */


/*****************************************************
*NAME
* soc_sand_bitstream_offline_test
*TYPE:
*  PROC
*DATE:
*  27-Jan-04
*FUNCTION:
*  Test function of soc_sand_bitstream.
*  View prints and return value to see pass/fail
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32
*       TRUE  - PASS.
*       FALSE - FAIL
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
uint32
  soc_sand_bitstream_offline_test(uint32 silent);

/*
 * Printing utility.
 */
void
  soc_sand_bitstream_print(
    SOC_SAND_IN uint32 *bit_stream,
    SOC_SAND_IN uint32 size /* in longs */
  );

void
  soc_sand_bitstream_print_beautify_1(
    SOC_SAND_IN uint32 *bit_stream,
    SOC_SAND_IN uint32 size, /* in longs */
    SOC_SAND_IN uint32 max_dec_digits,
    SOC_SAND_IN uint32 max_nof_printed_items
  );

/*****************************************************
*NAME
* soc_sand_buff_print_non_zero
*TYPE:
*  PROC
*DATE:
*  07/03/2006
*FUNCTION:
*  prints buffer's contents
*  only lines containing non-zero values are printed
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN char* buff - the buffer to be printed
*    SOC_SAND_IN uint32 buff_size - buffer's size in bytes
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 - soc_sand error word
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*    this function is active only when SOC_SAND_DEBUG flag is on.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_buff_print_non_zero(
    SOC_SAND_IN unsigned char* buff,
    SOC_SAND_IN uint32 buff_size
  );

/*****************************************************
*NAME
* soc_sand_buff_print_all
*TYPE:
*  PROC
*DATE:
*  04/07/2006
*FUNCTION:
*  Prints a given buffer as a set of chars
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN unsigned char* buff - the buffer to print
*    SOC_SAND_IN uint32 buff_size - buffer size in bytes
*    uint32 nof_columns - number of columns in each printed line
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    error indication
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_buff_print_all(
    SOC_SAND_IN unsigned char* buff,
    SOC_SAND_IN uint32 buff_size,
    uint32 nof_columns
  );


#endif /* SOC_SAND_DEBUG */

#include <soc/dpp/SAND/Utils/sand_footer.h>
/* } SOC_SAND_BITSTREAM_H_INCLUDED*/
#endif
