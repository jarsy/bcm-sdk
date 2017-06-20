/* $Id: sand_bitstream.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/



#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_bitstream.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>


/* $Id: sand_bitstream.c,v 1.9 Broadcom SDK $
 */
/*****************************************************
*NAME:
* dnx_sand_bitstream_clear
*DATE:
* 27/OCT/2002
*FUNCTION:
*  clears a bit stream
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size         -
*       size of the array (in uint32s)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the array of uint32s is zeroed
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_clear(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 iii;
  /*
   */
  for (iii=0; iii<size; ++iii)
  {
    bit_stream[iii] = 0;
  }
  return DNX_SAND_OK;
}

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
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size         -
*       size of the array (in uint32s)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the array of uint32s is set to all-ones
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_fill(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 iii;
  /*
   */
  for (iii=0; iii<size; ++iii)
  {
    bit_stream[iii] = 0xFFFFFFFF;
  }
  return DNX_SAND_OK;
}

/*****************************************************
*NAME:
* dnx_sand_bitstream_set
*DATE:
* 27/OCT/2002
*FUNCTION:
*  set 1 bit in the uint32 array to 'bit'
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    place         -
*       the bit number to set (counted in bits)
*    DNX_SAND_IN       uint32     bit_indicator -
*       indicator (0 or non-zero)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the right bit is set
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_set(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place,
    DNX_SAND_IN       uint32     bit_indicator
  )
{
  DNX_SAND_RET dnx_sand_ret;

  if(bit_indicator)
  {
    dnx_sand_ret = dnx_sand_bitstream_set_bit(bit_stream, place);
  }
  else
  {
    dnx_sand_ret = dnx_sand_bitstream_reset_bit(bit_stream, place);
  }
  return dnx_sand_ret;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_set_field
*DATE:
* 27/JAN/2004
*FUNCTION:
*  set field onto the bitstream
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    start_bit         -
*       the first bit number to set (the next one is 'start_bit+1')
*    DNX_SAND_IN       uint32    nof_bits         -
*       Number of bits to set in 'bit_stream' and to take from 'field'.
*       Range 0:32.
*    DNX_SAND_IN       uint32    field -
*       field to set
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    bit_stream
*REMARKS:
*  Example:
*  {
*    uint32
*      bit_stream[2];
*
*    dnx_sand_bitstream_clear(bit_stream, 2); ==> bit_stream = {0x0, 0x0}
*    dnx_sand_bitstream_set_field(bit_stream, 4, 16, 0x12345); ==> bit_stream = {0x0, 0x23450}
*  }
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_set_field(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_bit,
    DNX_SAND_IN       uint32    nof_bits,
    DNX_SAND_IN       uint32    field
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    bit_stream_bit_i,
    field_bit_i;

  dnx_sand_ret = DNX_SAND_OK;

  /*
   * 32 bits at most
   */
  if( nof_bits > DNX_SAND_BIT_STREAM_FIELD_SET_SIZE)
  {
    dnx_sand_ret = DNX_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR;
    goto exit;
  }


  for( bit_stream_bit_i=start_bit, field_bit_i = 0;
       field_bit_i< nof_bits;
       ++bit_stream_bit_i, ++field_bit_i)
  {
    dnx_sand_ret = dnx_sand_bitstream_set(bit_stream, bit_stream_bit_i, (field>>field_bit_i)& 0x1);
    if( dnx_sand_ret != DNX_SAND_OK )
    {
      goto exit;
    }
  }

exit:
  return dnx_sand_ret;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_get_field
*DATE:
* 27/JAN/2004
*FUNCTION:
*  get field from the bitstream
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    start_bit         -
*       the first bit number to get (the next one is 'start_bit+1')
*    DNX_SAND_IN       uint32    nof_bits         -
*       Number of bits to get from 'bit_stream' and to load to 'field'.
*       Range 0:32.
*    DNX_SAND_OUT      uint32    *field -
*       Loaded with value from the bit stream.
*       Points to one 'uint32'.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    bit_stream
*REMARKS:
*  Example:
*  {
*    uint32
*      bit_stream[2],
*      field;
*
*    bit_stream[0] = 0x23450;
*    bit_stream[1] = 0x0;     ==> bit_stream = {0x0, 0x23450}
*    dnx_sand_bitstream_get_field(bit_stream, 8, 12, &field);
*    ==> field is  0x234
*  }
*
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_get_field(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_bit,
    DNX_SAND_IN       uint32    nof_bits,
    DNX_SAND_OUT      uint32    *field
  )
{
  DNX_SAND_RET
    dnx_sand_ret;
  uint32
    bit_stream_bit_i,
    field_bit_i;

  dnx_sand_ret = DNX_SAND_OK;


  if(field == NULL)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  /*
   * 32 bits at most
   */
  if( nof_bits > DNX_SAND_BIT_STREAM_FIELD_SET_SIZE)
  {
    dnx_sand_ret = DNX_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR;
    goto exit;
  }

  *field = 0;
  for( bit_stream_bit_i=start_bit, field_bit_i = 0;
       field_bit_i< nof_bits;
       ++bit_stream_bit_i, ++field_bit_i)
  {
    *field |= ((uint32)dnx_sand_bitstream_test_bit(bit_stream, bit_stream_bit_i)) << field_bit_i;
  }

exit:
  return dnx_sand_ret;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_set_bit
*DATE:
* 27/OCT/2002
*FUNCTION:
*  set 1 bit in the uint32 array to 1
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    place         -
*       the bit number to set (counted in bits)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the right bit is set
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_set_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  )
{
  bit_stream[place>>5] |= DNX_SAND_BIT(place & 0x0000001F);
  return DNX_SAND_OK;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_set_bit_range
*DATE:
* 16/DEC/2002
*FUNCTION:
*  set range of bits in the uint32 array to 1
*  The range is [start_place -- end_place]
*  INCLUDING end_place.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    start_place  -
*       the start bit number to set (counted in bits)
*    DNX_SAND_IN       uint32    end_place    -
*       the end bit number to set (counted in bits)
*       This bit is also get set.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the right bit is reset
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_set_bit_range(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_place,
    DNX_SAND_IN       uint32    end_place
  )
{
  uint32
    bit_i;

  for(bit_i = start_place; bit_i <= end_place; ++bit_i)
  {
    dnx_sand_bitstream_set_bit(bit_stream, bit_i);
  }

  return DNX_SAND_OK;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_reset_bit
*DATE:
* 27/OCT/2002
*FUNCTION:
*  set 1 bit in the uint32 array to 0
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    place         -
*       the bit number to reset (counted in bits)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the right bit is reset
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_reset_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  )
{
  bit_stream[place>>5] &= DNX_SAND_RBIT(place & 0x0000001F);
  return DNX_SAND_OK;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_reset_bit_range
*DATE:
* 16/DEC/2002
*FUNCTION:
*  set range of bits in the uint32 array to 0
*  The range is [start_place -- end_place]
*  INCLUDING end_place.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    start_place  -
*       the start bit number to reset (counted in bits)
*    DNX_SAND_IN       uint32    end_place    -
*       the end bit number to reset (counted in bits)
*       This bit is also get reset.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*   the right bit is reset
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_reset_bit_range(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_place,
    DNX_SAND_IN       uint32    end_place
  )
{
  uint32
    bit_i;

  for(bit_i = start_place; bit_i <= end_place; ++bit_i)
  {
    dnx_sand_bitstream_reset_bit(bit_stream, bit_i);
  }

  return DNX_SAND_OK;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_test_bit
*DATE:
* 27/OCT/2002
*FUNCTION:
*  return the value (0/1) of 1 bit in the uint32 array
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN    uint32    place         -
*       the bit number to fetch (counted in bits)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    bit value (0 / 1)
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_bitstream_test_bit(
    DNX_SAND_IN  uint32    *bit_stream,
    DNX_SAND_IN  uint32    place
  )
{
  uint32 result;
  /*
   */
  result = bit_stream[place>>5] & DNX_SAND_BIT(place & 0x0000001F);
  /*
   */
  if (result)
  {
    return 1;
  }
  return 0;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_test_and_reset_bit
*DATE:
* 30/APR/2003
*FUNCTION:
*  Read the bit and auto-clear it.
*  1. Return the value (0/1) of 1 bit in the uint32 array
*  2. Set the bit to zero.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    place         -
*       the bit number to fetch and reset (counted in bits)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    bit value (0 / 1)
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_bitstream_test_and_reset_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  )
{
  int
    result;

  /* Get the bit. */
  result = dnx_sand_bitstream_test_bit(bit_stream, place);

  /* Set the bit to zero. */
  dnx_sand_bitstream_reset_bit(bit_stream, place);

  return result;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_test_and_set_bit
*DATE:
* 30/APR/2003
*FUNCTION:
*  Read the bit and auto-set it.
*  1. Return the value (0/1) of 1 bit in the uint32 array
*  2. Set the bit to one.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    place         -
*       the bit number to fetch and set (counted in bits)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    bit value (0 / 1)
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_bitstream_test_and_set_bit(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  )
{
  int
    result;

  /* Get the bit. */
  result = dnx_sand_bitstream_test_bit(bit_stream, place);

  /* Set the bit to one. */
  dnx_sand_bitstream_set_bit(bit_stream, place);

  return result;
}

/*****************************************************
*NAME:
* dnx_sand_bitstream_bit_flip
*FUNCTION:
*  Flip the value of specific bit (XOR with one)
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    place         -
*       the bit number to set (counted in bits)
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_bit_flip(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    place
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;

  bit_stream[place>>5] ^= DNX_SAND_BIT(place & 0x0000001F);

  return dnx_sand_ret;
}

/*****************************************************
*NAME:
* dnx_sand_bitstream_have_one_in_range
*DATE:
* 16/DEC/2002
*FUNCTION:
*  Test if in the range there is '1' or all is '0'.
*  The range is [start_place -- end_place]
*  INCLUDING end_place.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN       uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    start_place  -
*       the start bit number (counted in bits)
*    DNX_SAND_IN       uint32    end_place    -
*       the end bit number (counted in bits)
*       This bit is also checked.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    TRUE iff 1 is the bit range
*  DNX_SAND_INDIRECT:
*   NONE
*REMARKS:
*SEE ALSO:
* dnx_sand_bitstream_test_bit()
* dnx_sand_bitstream_have_one()
*****************************************************/
int
  dnx_sand_bitstream_have_one_in_range(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    start_place,
    DNX_SAND_IN       uint32    end_place
  )
{
  uint32
    bit_i;
  int
    result;

  result = FALSE;

  for(bit_i = start_place; bit_i <= end_place; ++bit_i)
  {
    if (dnx_sand_bitstream_test_bit(bit_stream, bit_i))
    {
      result = TRUE;
      goto exit;
    }
  }

exit:
  return result;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_have_one
*DATE:
* 16/DEC/2002
*FUNCTION:
*  Test if in all the bitstream any bit is on.
*  (work faster than dnx_sand_bitstream_have_one_in_range())
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN       uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size   -
*             in longs
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    TRUE iff 1 is the bit range (all the bitstream).
*  DNX_SAND_INDIRECT:
*   NONE
*REMARKS:
*SEE ALSO:
*  dnx_sand_bitstream_have_one_in_range()
*****************************************************/
int
  dnx_sand_bitstream_have_one(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32
    word_i;
  int
    result;
  const uint32
    *pointer;

  result = FALSE;

  pointer = bit_stream;
  for(word_i = 0; word_i < size; ++word_i, ++pointer)
  {
    if ( (*pointer) != 0)
    {
      result = TRUE;
      goto exit;
    }
  }

exit:
  return result;
}
/*
 */
/*****************************************************
*NAME:
* dnx_sand_bitstream_or
*DATE:
* 18/NOV/2002
*FUNCTION:
*  or 2 bitstreams
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size   -
*             in longs
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_or(
    DNX_SAND_INOUT    uint32    *bit_stream1,
    DNX_SAND_IN       uint32    *bit_stream2,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream1[i] |= bit_stream2[i];
  }
  return DNX_SAND_OK;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_and
*DATE:
* 14/APR/2003
*FUNCTION:
*  and 2 bitstreams
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size   -
*             in longs
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_and(
    DNX_SAND_INOUT    uint32    *bit_stream1,
    DNX_SAND_IN       uint32    *bit_stream2,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream1[i] &= bit_stream2[i];
  }
  return DNX_SAND_OK;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_xor
*DATE:
* 14/APR/2003
*FUNCTION:
*  xor 2 bitstreams
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size   -
*             in longs
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_xor(
    DNX_SAND_INOUT    uint32    *bit_stream1,
    DNX_SAND_IN       uint32    *bit_stream2,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream1[i] ^= bit_stream2[i];
  }
  return DNX_SAND_OK;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_parity
*DATE:
* 13/MAy/2008
*FUNCTION:
*  Get the parity of a bitstream
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN  uint32    start_bit -
*       Specifies the bit from which parity is calculated.
*    DNX_SAND_IN  uint32    nof_bits -
*       Specifies the number of bits from the start on
*       which parity is calculated.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    0 - numbers of 1 in the stream is even.
*    1 - numbers of 1 in the stream is odd.
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_bitstream_parity(
    DNX_SAND_IN  uint32    *bit_stream,
    DNX_SAND_IN  uint32    start_bit,
    DNX_SAND_IN  uint32    nof_bits
  )
{
  uint32
    buffer = 0;
  uint32
    idx = 0;
  uint32
    parity = 0;
  uint32
    start_bit_aligned = start_bit / DNX_SAND_NOF_BITS_IN_UINT32;
  const uint32
    *bit_stream_aligned = bit_stream + start_bit_aligned;

  for (idx = start_bit_aligned; idx < start_bit_aligned + nof_bits; ++idx)
  {
    buffer = *(bit_stream_aligned + (idx / DNX_SAND_NOF_BITS_IN_UINT32));
    parity = (~((~parity) ^ (DNX_SAND_BIT(idx % DNX_SAND_NOF_BITS_IN_UINT32) & buffer))) & DNX_SAND_BIT(idx % DNX_SAND_NOF_BITS_IN_UINT32);
  }

  return parity;
}
/*****************************************************
*NAME:
* dnx_sand_bitstream_not
*DATE:
* 14/APR/2003
*FUNCTION:
*  Do 'not' operation on the bitstream
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size   -
*             in longs
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_bitstream_not(
    DNX_SAND_INOUT    uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream[i] ^= 0xFFFFFFFF;
  }
  return DNX_SAND_OK;
}

/*****************************************************
*NAME:
* dnx_sand_bitstream_get_nof_on_bits
*DATE:
* 14/APR/2003
*FUNCTION:
*  Count the number of "ones" in the bit stream.
*  Examples:
*   + 0x0000_0000 ==> will results with 0.
*   + 0x0100_0000 ==> will results with 1.
*   + 0x0100_0000 0x0C00_0000 ==> will results with 3.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size   -
*             in longs
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   uint32
*     The number of ones
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_bitstream_get_nof_on_bits(
    DNX_SAND_IN       uint32    *bit_stream,
    DNX_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32
    nof_on_bits,
    i;

  nof_on_bits = 0;
  /*
   */
  if (NULL == bit_stream)
  {
    goto exit;
  }

  for(i=0; i<size; ++i)
  {
    nof_on_bits += dnx_sand_nof_on_bits_in_long(bit_stream[i]);
  }

exit:
  return nof_on_bits;
}

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
  dnx_sand_bitstream_offline_test(uint32 silent)
{
  uint32
    bit_stream[2],
    field;
  uint32
    pass;

  pass = TRUE;

  dnx_sand_bitstream_clear(bit_stream, 2);
  dnx_sand_bitstream_set_field(bit_stream, 0, 4, 0xF);
  if(!silent)
  {
    dnx_sand_bitstream_print(bit_stream, 2);
    dnx_sand_bitstream_print_beautify_1(bit_stream, 2, 7, 100);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }

  dnx_sand_bitstream_clear(bit_stream, 2);
  dnx_sand_bitstream_set_field(bit_stream, 4, 4, 0xF);
  if(!silent)
  {
    dnx_sand_bitstream_print(bit_stream, 2);
    dnx_sand_bitstream_print_beautify_1(bit_stream, 2, 5, 100);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }

  dnx_sand_bitstream_clear(bit_stream, 2);
  dnx_sand_bitstream_set_field(bit_stream, 30, 4, 0xF);
  if(!silent)
  {
    dnx_sand_bitstream_print(bit_stream, 2);
    dnx_sand_bitstream_print_beautify_1(bit_stream, 2, 4, 100);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }


  dnx_sand_bitstream_clear(bit_stream, 2);
  dnx_sand_bitstream_set_field(bit_stream, 4, 16, 0x12345);
  dnx_sand_bitstream_set_field(bit_stream, 20, 28, 0xFFFFFFFF);
  if(!silent)
  {
    dnx_sand_bitstream_print(bit_stream, 2);
    dnx_sand_bitstream_print_beautify_1(bit_stream, 2, 3, 20);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }

  dnx_sand_bitstream_get_field(bit_stream, 8, 12, &field);
  if(field != 0x234)
  {
    pass &= 0;
  }

  return pass;
}


/*
 * Printing utility.
 */
void
  dnx_sand_bitstream_print(
    DNX_SAND_IN uint32 *bit_stream,
    DNX_SAND_IN uint32 size /* in uint32s */
  )
{
  uint32
    line_i,
    not_lines;
  int
    long_i;
  const int
    nof_longs_per_line = 8;

  if (NULL == bit_stream)
  {
    LOG_CLI((BSL_META("dnx_sand_bitstream_print received NULL ptr\n\r")));
    goto exit;
  }

  LOG_CLI((BSL_META("(Hex format, Maximum of %u bits per line.)\r\n"),
nof_longs_per_line*(int)DNX_SAND_NOF_BITS_IN_UINT32));
  /*
   */
  not_lines = DNX_SAND_DIV_ROUND_UP(size, nof_longs_per_line);
  for(line_i=0; line_i< not_lines; ++line_i)
  {
    LOG_CLI((BSL_META("[%4u-%4u]"),
DNX_SAND_MIN( (line_i+1)*nof_longs_per_line*(int)DNX_SAND_NOF_BITS_IN_UINT32-1, size * (int)DNX_SAND_NOF_BITS_IN_UINT32 - 1),
             line_i*nof_longs_per_line*(int)DNX_SAND_NOF_BITS_IN_UINT32
             ));
    for(long_i = DNX_SAND_MIN( (line_i+1)*nof_longs_per_line - 1, size-1);
        long_i >= (int)(line_i*nof_longs_per_line);
        --long_i)
    {
      LOG_CLI((BSL_META("%08X"), bit_stream[long_i]));
    }
    LOG_CLI((BSL_META("\r\n")));
  }

exit:
  return;
}

/*****************************************************
*NAME:
* dnx_sand_bitstream_set_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function writes the first buffer
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
  )
{
  uint32
    end,
    nof_bits_iter_acc,
    nof_bits_iter,
    nof_words,
    iter;
  uint32
    res = DNX_SAND_OK;
  uint32
    field;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_BITSTREAM_SET_ANY_FIELD);

  DNX_SAND_CHECK_NULL_INPUT(output_buffer);
  DNX_SAND_CHECK_NULL_INPUT(input_buffer);

  end = nof_bits + start_bit - 1;
  if((end / DNX_SAND_NOF_BITS_IN_UINT32) == (start_bit / DNX_SAND_NOF_BITS_IN_UINT32))
  {
    field = DNX_SAND_GET_BITS_RANGE(*input_buffer, nof_bits - 1, 0);
    *(output_buffer + (start_bit / DNX_SAND_NOF_BITS_IN_UINT32)) &= DNX_SAND_ZERO_BITS_MASK(end % DNX_SAND_NOF_BITS_IN_UINT32, start_bit % DNX_SAND_NOF_BITS_IN_UINT32);
    *(output_buffer + (start_bit / DNX_SAND_NOF_BITS_IN_UINT32)) |= DNX_SAND_SET_BITS_RANGE(field, end % DNX_SAND_NOF_BITS_IN_UINT32, start_bit % DNX_SAND_NOF_BITS_IN_UINT32);
  }
  else
  {
    nof_words = DNX_SAND_DIV_ROUND_UP(nof_bits, DNX_SAND_NOF_BITS_IN_UINT32);
    for(
        iter = 0, nof_bits_iter_acc = nof_bits;
        iter < nof_words;
        ++iter, nof_bits_iter_acc -= DNX_SAND_NOF_BITS_IN_UINT32
       )
    {
      nof_bits_iter = nof_bits_iter_acc > DNX_SAND_NOF_BITS_IN_UINT32 ? DNX_SAND_NOF_BITS_IN_UINT32 : nof_bits_iter_acc;

      res = dnx_sand_bitstream_set_field(
              output_buffer,
              start_bit + iter * DNX_SAND_NOF_BITS_IN_UINT32,
              nof_bits_iter,
              *(input_buffer + iter)
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, iter, exit);
    }
  }

 exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in dnx_sand_bitstream_set_any_field()",0,0);
}

/*****************************************************
*NAME:
* dnx_sand_bitstream_get_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function reads the first buffer
*  from an offset and writes it to the second buffer.
*  The function zeroes the bits in the second buffer
*  from the end of the field to the end of the word (word is uint32)
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
  )
{
  uint32
    end,
    nof_bits_iter,
    nof_words,
    iter;
  uint32
    res = DNX_SAND_OK;
  uint32
    field;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_BITSTREAM_GET_ANY_FIELD);

  DNX_SAND_CHECK_NULL_INPUT(input_buffer);
  DNX_SAND_CHECK_NULL_INPUT(output_buffer);

  nof_words = DNX_SAND_DIV_ROUND_UP(nof_bits, DNX_SAND_NOF_BITS_IN_UINT32);

  end = nof_bits + start_bit - 1;
  if((end / DNX_SAND_NOF_BITS_IN_UINT32) == (start_bit / DNX_SAND_NOF_BITS_IN_UINT32))
  {
    /* There is no bug here - the pointer arithmatic done here is correct */
    /* coverity [ptr_arith]*/
    field = DNX_SAND_GET_BITS_RANGE(*(input_buffer + (start_bit / DNX_SAND_NOF_BITS_IN_UINT32)), end % DNX_SAND_NOF_BITS_IN_UINT32, start_bit % DNX_SAND_NOF_BITS_IN_UINT32);

    *output_buffer = 0;
    *output_buffer |= DNX_SAND_SET_BITS_RANGE(field, nof_bits - 1, 0);
  }
  else
  {
    for(
        iter = 0, nof_bits_iter = nof_bits;
        iter < nof_words;
        ++iter, nof_bits_iter -= DNX_SAND_NOF_BITS_IN_UINT32
       )
    {
      res = dnx_sand_bitstream_get_field(
              input_buffer,
              start_bit + iter * DNX_SAND_NOF_BITS_IN_UINT32,
              nof_bits_iter > DNX_SAND_NOF_BITS_IN_UINT32 ? DNX_SAND_NOF_BITS_IN_UINT32 : nof_bits_iter,
              output_buffer + iter
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit);
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in dnx_sand_bitstream_get_any_field()",0,0);
}

/*****************************************************
*NAME:
* dnx_sand_bitstream_print_beautify_1
*DATE:
* 14/APR/2003
*FUNCTION:
*  Beautify printing utility.
*  Prints the bit-stream as a GROUP of items.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    DNX_SAND_IN       uint32    size   -
*       in longs
*    DNX_SAND_IN uint32 max_dec_digits -
*       Item max printing size in Decimal.
*    DNX_SAND_IN uint32 max_nof_printed_items -
*       Maximum number of items to print.
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   uint32
*     The number of ones
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
void
  dnx_sand_bitstream_print_beautify_1(
    DNX_SAND_IN uint32 *bit_stream,
    DNX_SAND_IN uint32 size, /* in uint32s */
    DNX_SAND_IN uint32 max_dec_digits,
    DNX_SAND_IN uint32 max_nof_printed_items
  )
{
  uint32
    item_i,
    nof_on_bits,
    bit_i;
  const int
    nof_number_per_line = 8;
  char
    format[20]="";

  if (NULL == bit_stream)
  {
    LOG_CLI((BSL_META("dnx_sand_bitstream_print_beautify_1 received NULL ptr\n\r")));
    goto exit;
  }

  if (max_dec_digits>10)
  {
    LOG_CLI((BSL_META("dnx_sand_bitstream_print_beautify_1: received 'max_dec_digits>10'\n\r")));
    goto exit;
  }

  nof_on_bits = dnx_sand_bitstream_get_nof_on_bits(bit_stream, size);
  LOG_CLI((BSL_META("Out of %u, %u are ON (%u item exists).\r\n"),
size*(int)DNX_SAND_NOF_BITS_IN_UINT32,
           nof_on_bits,
           nof_on_bits
           ));
  sal_sprintf(format, "%%-%uu ", max_dec_digits);
  /*
   */
  if (nof_on_bits == 0)
  {
    /*
     * No items to print.
     */
    goto exit;
  }

  LOG_CLI((BSL_META("[ ")));
  item_i=0;
  for (bit_i=0; bit_i<(size*DNX_SAND_NOF_BITS_IN_UINT32); bit_i++)
  {
    if( item_i >= max_nof_printed_items)
    {
      /*
       * Do not show more items
       */
      LOG_CLI((BSL_META(" .... More Items to print. Exist .... \r\n")));
      break;
    }
    if ( item_i >= nof_on_bits)
    {
      /*
       * No items more to show.
       */
      break;
    }
    if (dnx_sand_bitstream_test_bit(bit_stream, bit_i) == FALSE)
    {
      continue;
    }
    item_i ++;
    LOG_CLI((format, bit_i)); /*Print the Number*/
    if ( (item_i%nof_number_per_line) == 0)
    {
      LOG_CLI((BSL_META("\r\n"
                        "  ")));
    }
  }
  LOG_CLI((BSL_META("]\r\n")));

exit:
  return;
}

DNX_SAND_RET
  dnx_sand_buff_xor(
    DNX_SAND_INOUT    unsigned  char    *buff1,
    DNX_SAND_IN       unsigned  char    *buff2,
    DNX_SAND_IN       uint32     size /* in chars */
  )
{
  uint32 i = 0;
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;

  if (!buff1)
  {
    dnx_sand_ret = 1;
    goto exit;
  }

  if (!buff2)
  {
    dnx_sand_ret = 2;
    goto exit;
  }

  for(i=0; i<size; ++i)
  {
    buff1[i] ^= buff2[i];
  }

exit:
  return dnx_sand_ret;
}

DNX_SAND_RET
  dnx_sand_buff_print_non_zero(
    DNX_SAND_IN unsigned char* buff,
    DNX_SAND_IN uint32 buff_size
  )
{
  uint32
    curr_line = 0,
    curr_col = 0,
    rem_offset = 0,
    only_zer_this_line = TRUE,
    size_in_longs = 0,
    rem_from_long = 0,
    max_nof_lines = 0;
  const uint32
    max_columns = 6; /* must be non-zero! */
  uint32
    curr_offset = 0,
    curr_long = 0;
  const uint32*
    buff_longs = NULL;

  unsigned char
    curr_char = 0;
  const unsigned char*
    buff_chars = NULL;
  char* str_ptr = NULL;
  char str_line[200] = "";

  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  char*
    func_name = "dnx_sand_buff_print_non_zero";

  str_ptr = str_line;

  if (!buff)
  {
    dnx_sand_ret = 1;
    goto exit;
  }

  if (!buff_size)
  {
    dnx_sand_ret = 2;
    goto exit;
  }

  buff_longs = (const uint32* )buff;
  buff_chars = buff;

  rem_from_long = buff_size % sizeof(uint32);

  size_in_longs =
    DNX_SAND_DIV_ROUND_DOWN(buff_size, sizeof(uint32));

  if (!rem_from_long)
  {
    max_nof_lines = DNX_SAND_DIV_ROUND_UP(size_in_longs, max_columns);
  }
  else
  {
    max_nof_lines = DNX_SAND_DIV_ROUND_UP(size_in_longs + 1, max_columns);
  }

  for(curr_line = 0; curr_line < max_nof_lines; curr_line++)
  {
    /* ITERATION INITIALIZATIONS { */
    only_zer_this_line = TRUE;

    dnx_sand_ret =
      dnx_sand_os_memset(
        str_line,
        0x0,
        200
       );

    if (dnx_sand_ret)
    {
      dnx_sand_ret = 7;
      goto exit;
    }

    str_ptr = str_line;

    str_ptr += sal_sprintf(str_ptr, "OFFSET LONGS: %8d ", curr_offset);
    /* ITERATION INITIALIZATIONS } */

    for (curr_col = 0; curr_col < max_columns; curr_col++)
    {
      /* build one line and check whether there are non-zero values in it */

      if (curr_offset < size_in_longs)
      {
        curr_long = (uint32)( *(buff_longs + curr_offset));
        if (curr_long)
        {
          only_zer_this_line = FALSE;
        }

        str_ptr += sal_sprintf(str_ptr, "0x%8.8X  ", curr_long);
      }
      else /* finished printing all uint32s, maybe we have a remainder less then uint32 */
      {
        if (rem_from_long && (rem_offset < rem_from_long) )
        {
          str_ptr += sal_sprintf(str_ptr, "0x");
          for (rem_offset = 0; rem_offset < rem_from_long; rem_offset++)
          {
            curr_char = (unsigned char)( *(buff_chars + (curr_offset*sizeof(uint32) + rem_offset)));
            if (curr_char)
            {
              only_zer_this_line = FALSE;
            }
            str_ptr += sal_sprintf(str_ptr, "%X", curr_char);
          }
        }
      }

      curr_offset++; /* curr_offset = (curr_line * max_columns) + curr_col */
    }

    str_ptr += sal_sprintf(str_ptr, "\n\r");

    if (!only_zer_this_line)
    {
      LOG_CLI((str_line));
    }
  }

exit:
  if (dnx_sand_ret)
  {
    LOG_CLI((BSL_META("ERROR: function %s exited with error num. %d\n\r"),
             func_name,
             dnx_sand_ret
             ));
  }

  return dnx_sand_ret;
}

DNX_SAND_RET
  dnx_sand_buff_print_all(
    DNX_SAND_IN unsigned char* buff,
    DNX_SAND_IN uint32 buff_size,
    uint32 nof_columns
  )
{
  uint32
    curr_line = 0,
    curr_col = 0,
    max_nof_lines = 0;
  uint32
    max_columns = 1; /* must be non-zero! */
  uint32
    curr_offset = 0;
  unsigned char
    curr_char = 0;
  const unsigned char*
    buff_chars = NULL;
  char* str_ptr = NULL;
  char str_line[200] = "";

  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  char*
    func_name = "dnx_sand_buff_print_non_zero";

  str_ptr = str_line;

  if (!buff)
  {
    dnx_sand_ret = 1;
    goto exit;
  }

  if (!buff_size)
  {
    dnx_sand_ret = 2;
    goto exit;
  }

  if (nof_columns != 0)
  {
    max_columns = nof_columns * sizeof(uint32);
  }
  buff_chars = buff;


  max_nof_lines = DNX_SAND_DIV_ROUND_UP(buff_size, max_columns);

  LOG_CLI((BSL_META("\n\r"
                    "Buffer of %u Bytes, values given in hexa:\n\r"
                    "-----------------------------------------\n\r"),
           (uint32)buff_size
           ));

  for(curr_line = 0; curr_line < max_nof_lines; curr_line++)
  {
    /* ITERATION INITIALIZATIONS { */

    dnx_sand_ret =
      dnx_sand_os_memset(
        str_line,
        0x0,
        200
       );

    if (dnx_sand_ret)
    {
      dnx_sand_ret = 7;
      goto exit;
    }

    str_ptr = str_line;

    str_ptr +=
      sal_sprintf(
        str_ptr,
        "OFFSET LONGS: %6d",
        (uint32)(curr_offset / sizeof(uint32)));
    /* ITERATION INITIALIZATIONS } */

    for (curr_col = 0; curr_col < max_columns; curr_col++)
    {
      /* build one line and check whether there are non-zero values in it */

      if (curr_offset < buff_size)
      {
        curr_char = (unsigned char)( *(buff_chars + curr_offset));
        if (((curr_offset) % sizeof(uint32)) == 0)
        {
            str_ptr += sal_sprintf(str_ptr, " %02X", curr_char);
        }
        else
        {
          str_ptr += sal_sprintf(str_ptr, "%02X", curr_char);
        }
     }
      curr_offset++; /* curr_offset = (curr_line * max_columns) + curr_col */
    }

    str_ptr += sal_sprintf(str_ptr, "\n\r");
    LOG_CLI((str_line));
  }

exit:
  if (dnx_sand_ret)
  {
    LOG_CLI((BSL_META("ERROR: function %s exited with error num. %d\n\r"),
             func_name,
             dnx_sand_ret
             ));
  }

  return dnx_sand_ret;
}

/*
 *  Having two char buffers, orders msB->lsB, the function reads
 *  the first buffer from offset (offset taken from msb) and writes
 *  it to the second buffer.
 */
DNX_SAND_RET
  dnx_sand_bitsteam_u8_ms_byte_first_get_field(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint8  *input_buffer,
    DNX_SAND_IN  uint32 start_bit_msb,
    DNX_SAND_IN  uint32 nof_bits,
    DNX_SAND_OUT uint32 *output_value
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK;
  uint32
    idx,
    buf_sizes=0,
    tmp_output_value[2]={0},
    first_byte_ndx,
    last_byte_ndx;
  uint8
    *tmp_output_value_u8_ptr = (uint8*)&tmp_output_value;
 

  /* 32 bits at most */
  if (nof_bits > DNX_SAND_BIT_STREAM_FIELD_SET_SIZE)
  {
    dnx_sand_ret = DNX_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR;
    goto exit;
  }

  /* Reverse input buffer relevant part, since received msb->lsb, but would be parsed lsb->msb */
  first_byte_ndx = start_bit_msb / DNX_SAND_NOF_BITS_IN_BYTE;
  last_byte_ndx = ((start_bit_msb + nof_bits - 1) / DNX_SAND_NOF_BITS_IN_BYTE);*output_value=0;

  for (idx = first_byte_ndx;
       idx <= last_byte_ndx;
       ++idx)
  {
    tmp_output_value_u8_ptr[last_byte_ndx - idx] = input_buffer[idx];
    buf_sizes += DNX_SAND_NOF_BITS_IN_BYTE;
  }

  /* If big endian, swap bytes (input is chars, but will be evaluated in uint32s) */

 #ifndef LE_HOST
  {
    tmp_output_value[0] = DNX_SAND_BYTE_SWAP(tmp_output_value[0]);
    
    if (last_byte_ndx > 4)
    {
      tmp_output_value[1] = DNX_SAND_BYTE_SWAP(tmp_output_value[1]);
    }
  }
#endif

  dnx_sand_ret = dnx_sand_bitstream_get_field(
    tmp_output_value,
    buf_sizes-(start_bit_msb%DNX_SAND_NOF_BITS_IN_BYTE+nof_bits),
    nof_bits,
    output_value
    );
  if( dnx_sand_ret != DNX_SAND_OK )
  {
    goto exit;
  }

exit:
  return dnx_sand_ret;
}

/* } */
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
