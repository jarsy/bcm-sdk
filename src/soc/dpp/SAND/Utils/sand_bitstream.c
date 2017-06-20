/* $Id: sand_bitstream.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/



#include <shared/bsl.h>
#include <soc/dpp/drv.h>



#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Utils/sand_bitstream.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_low_level.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>


/* $Id: sand_bitstream.c,v 1.9 Broadcom SDK $
 */
/*****************************************************
*NAME:
* soc_sand_bitstream_clear
*DATE:
* 27/OCT/2002
*FUNCTION:
*  clears a bit stream
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size         -
*       size of the array (in uint32s)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the array of uint32s is zeroed
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_clear(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 iii;
  /*
   */
  for (iii=0; iii<size; ++iii)
  {
    bit_stream[iii] = 0;
  }
  return SOC_SAND_OK;
}

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
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size         -
*       size of the array (in uint32s)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the array of uint32s is set to all-ones
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_fill(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 iii;
  /*
   */
  for (iii=0; iii<size; ++iii)
  {
    bit_stream[iii] = 0xFFFFFFFF;
  }
  return SOC_SAND_OK;
}

/*****************************************************
*NAME:
* soc_sand_bitstream_set
*DATE:
* 27/OCT/2002
*FUNCTION:
*  set 1 bit in the uint32 array to 'bit'
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    place         -
*       the bit number to set (counted in bits)
*    SOC_SAND_IN       uint32     bit_indicator -
*       indicator (0 or non-zero)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the right bit is set
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_set(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place,
    SOC_SAND_IN       uint32     bit_indicator
  )
{
  SOC_SAND_RET soc_sand_ret;

  if(bit_indicator)
  {
    soc_sand_ret = soc_sand_bitstream_set_bit(bit_stream, place);
  }
  else
  {
    soc_sand_ret = soc_sand_bitstream_reset_bit(bit_stream, place);
  }
  return soc_sand_ret;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_set_field
*DATE:
* 27/JAN/2004
*FUNCTION:
*  set field onto the bitstream
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    start_bit         -
*       the first bit number to set (the next one is 'start_bit+1')
*    SOC_SAND_IN       uint32    nof_bits         -
*       Number of bits to set in 'bit_stream' and to take from 'field'.
*       Range 0:32.
*    SOC_SAND_IN       uint32    field -
*       field to set
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*    bit_stream
*REMARKS:
*  Example:
*  {
*    uint32
*      bit_stream[2];
*
*    soc_sand_bitstream_clear(bit_stream, 2); ==> bit_stream = {0x0, 0x0}
*    soc_sand_bitstream_set_field(bit_stream, 4, 16, 0x12345); ==> bit_stream = {0x0, 0x23450}
*  }
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_set_field(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_bit,
    SOC_SAND_IN       uint32    nof_bits,
    SOC_SAND_IN       uint32    field
  )
{
  SOC_SAND_RET
    soc_sand_ret;
  uint32
    bit_stream_bit_i,
    field_bit_i;

  soc_sand_ret = SOC_SAND_OK;

  /*
   * 32 bits at most
   */
  if( nof_bits > SOC_SAND_BIT_STREAM_FIELD_SET_SIZE)
  {
    soc_sand_ret = SOC_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR;
    goto exit;
  }


  for( bit_stream_bit_i=start_bit, field_bit_i = 0;
       field_bit_i< nof_bits;
       ++bit_stream_bit_i, ++field_bit_i)
  {
    soc_sand_ret = soc_sand_bitstream_set(bit_stream, bit_stream_bit_i, (field>>field_bit_i)& 0x1);
    if( soc_sand_ret != SOC_SAND_OK )
    {
      goto exit;
    }
  }

exit:
  return soc_sand_ret;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_get_field
*DATE:
* 27/JAN/2004
*FUNCTION:
*  get field from the bitstream
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    start_bit         -
*       the first bit number to get (the next one is 'start_bit+1')
*    SOC_SAND_IN       uint32    nof_bits         -
*       Number of bits to get from 'bit_stream' and to load to 'field'.
*       Range 0:32.
*    SOC_SAND_OUT      uint32    *field -
*       Loaded with value from the bit stream.
*       Points to one 'uint32'.
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
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
*    soc_sand_bitstream_get_field(bit_stream, 8, 12, &field);
*    ==> field is  0x234
*  }
*
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_get_field(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_bit,
    SOC_SAND_IN       uint32    nof_bits,
    SOC_SAND_OUT      uint32    *field
  )
{
  SOC_SAND_RET
    soc_sand_ret;
  uint32
    bit_stream_bit_i,
    field_bit_i;

  soc_sand_ret = SOC_SAND_OK;


  if(field == NULL)
  {
    soc_sand_ret = SOC_SAND_NULL_POINTER_ERR;
    goto exit;
  }

  /*
   * 32 bits at most
   */
  if( nof_bits > SOC_SAND_BIT_STREAM_FIELD_SET_SIZE)
  {
    soc_sand_ret = SOC_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR;
    goto exit;
  }

  *field = 0;
  for( bit_stream_bit_i=start_bit, field_bit_i = 0;
       field_bit_i< nof_bits;
       ++bit_stream_bit_i, ++field_bit_i)
  {
    *field |= ((uint32)soc_sand_bitstream_test_bit(bit_stream, bit_stream_bit_i)) << field_bit_i;
  }

exit:
  return soc_sand_ret;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_set_bit
*DATE:
* 27/OCT/2002
*FUNCTION:
*  set 1 bit in the uint32 array to 1
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    place         -
*       the bit number to set (counted in bits)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the right bit is set
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_set_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  )
{
  bit_stream[place>>5] |= SOC_SAND_BIT(place & 0x0000001F);
  return SOC_SAND_OK;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_set_bit_range
*DATE:
* 16/DEC/2002
*FUNCTION:
*  set range of bits in the uint32 array to 1
*  The range is [start_place -- end_place]
*  INCLUDING end_place.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    start_place  -
*       the start bit number to set (counted in bits)
*    SOC_SAND_IN       uint32    end_place    -
*       the end bit number to set (counted in bits)
*       This bit is also get set.
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the right bit is reset
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_set_bit_range(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_place,
    SOC_SAND_IN       uint32    end_place
  )
{
  uint32
    bit_i;

  for(bit_i = start_place; bit_i <= end_place; ++bit_i)
  {
    soc_sand_bitstream_set_bit(bit_stream, bit_i);
  }

  return SOC_SAND_OK;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_reset_bit
*DATE:
* 27/OCT/2002
*FUNCTION:
*  set 1 bit in the uint32 array to 0
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    place         -
*       the bit number to reset (counted in bits)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the right bit is reset
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_reset_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  )
{
  bit_stream[place>>5] &= SOC_SAND_RBIT(place & 0x0000001F);
  return SOC_SAND_OK;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_reset_bit_range
*DATE:
* 16/DEC/2002
*FUNCTION:
*  set range of bits in the uint32 array to 0
*  The range is [start_place -- end_place]
*  INCLUDING end_place.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    start_place  -
*       the start bit number to reset (counted in bits)
*    SOC_SAND_IN       uint32    end_place    -
*       the end bit number to reset (counted in bits)
*       This bit is also get reset.
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*   the right bit is reset
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_reset_bit_range(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_place,
    SOC_SAND_IN       uint32    end_place
  )
{
  uint32
    bit_i;

  for(bit_i = start_place; bit_i <= end_place; ++bit_i)
  {
    soc_sand_bitstream_reset_bit(bit_stream, bit_i);
  }

  return SOC_SAND_OK;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_test_bit
*DATE:
* 27/OCT/2002
*FUNCTION:
*  return the value (0/1) of 1 bit in the uint32 array
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN    uint32    place         -
*       the bit number to fetch (counted in bits)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    bit value (0 / 1)
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  soc_sand_bitstream_test_bit(
    SOC_SAND_IN  uint32    *bit_stream,
    SOC_SAND_IN  uint32    place
  )
{
  uint32 result;
  /*
   */
  result = bit_stream[place>>5] & SOC_SAND_BIT(place & 0x0000001F);
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
* soc_sand_bitstream_test_and_reset_bit
*DATE:
* 30/APR/2003
*FUNCTION:
*  Read the bit and auto-clear it.
*  1. Return the value (0/1) of 1 bit in the uint32 array
*  2. Set the bit to zero.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    place         -
*       the bit number to fetch and reset (counted in bits)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    bit value (0 / 1)
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  soc_sand_bitstream_test_and_reset_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  )
{
  int
    result;

  /* Get the bit. */
  result = soc_sand_bitstream_test_bit(bit_stream, place);

  /* Set the bit to zero. */
  soc_sand_bitstream_reset_bit(bit_stream, place);

  return result;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_test_and_set_bit
*DATE:
* 30/APR/2003
*FUNCTION:
*  Read the bit and auto-set it.
*  1. Return the value (0/1) of 1 bit in the uint32 array
*  2. Set the bit to one.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    place         -
*       the bit number to fetch and set (counted in bits)
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    bit value (0 / 1)
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  soc_sand_bitstream_test_and_set_bit(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  )
{
  int
    result;

  /* Get the bit. */
  result = soc_sand_bitstream_test_bit(bit_stream, place);

  /* Set the bit to one. */
  soc_sand_bitstream_set_bit(bit_stream, place);

  return result;
}

/*****************************************************
*NAME:
* soc_sand_bitstream_bit_flip
*FUNCTION:
*  Flip the value of specific bit (XOR with one)
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    place         -
*       the bit number to set (counted in bits)
*OUTPUT:
*  SOC_SAND_DIRECT:
*    Non-zero in case of an error
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_bit_flip(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    place
  )
{
  SOC_SAND_RET
    soc_sand_ret = SOC_SAND_OK;

  bit_stream[place>>5] ^= SOC_SAND_BIT(place & 0x0000001F);

  return soc_sand_ret;
}

/*****************************************************
*NAME:
* soc_sand_bitstream_have_one_in_range
*DATE:
* 16/DEC/2002
*FUNCTION:
*  Test if in the range there is '1' or all is '0'.
*  The range is [start_place -- end_place]
*  INCLUDING end_place.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN       uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    start_place  -
*       the start bit number (counted in bits)
*    SOC_SAND_IN       uint32    end_place    -
*       the end bit number (counted in bits)
*       This bit is also checked.
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    TRUE iff 1 is the bit range
*  SOC_SAND_INDIRECT:
*   NONE
*REMARKS:
*SEE ALSO:
* soc_sand_bitstream_test_bit()
* soc_sand_bitstream_have_one()
*****************************************************/
int
  soc_sand_bitstream_have_one_in_range(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    start_place,
    SOC_SAND_IN       uint32    end_place
  )
{
  uint32
    bit_i;
  int
    result;

  result = FALSE;

  for(bit_i = start_place; bit_i <= end_place; ++bit_i)
  {
    if (soc_sand_bitstream_test_bit(bit_stream, bit_i))
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
* soc_sand_bitstream_have_one
*DATE:
* 16/DEC/2002
*FUNCTION:
*  Test if in all the bitstream any bit is on.
*  (work faster than soc_sand_bitstream_have_one_in_range())
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN       uint32    *bit_stream  -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size   -
*             in longs
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    TRUE iff 1 is the bit range (all the bitstream).
*  SOC_SAND_INDIRECT:
*   NONE
*REMARKS:
*SEE ALSO:
*  soc_sand_bitstream_have_one_in_range()
*****************************************************/
int
  soc_sand_bitstream_have_one(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in uint32s */
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
* soc_sand_bitstream_or
*DATE:
* 18/NOV/2002
*FUNCTION:
*  or 2 bitstreams
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size   -
*             in longs
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_or(
    SOC_SAND_INOUT    uint32    *bit_stream1,
    SOC_SAND_IN       uint32    *bit_stream2,
    SOC_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream1[i] |= bit_stream2[i];
  }
  return SOC_SAND_OK;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_and
*DATE:
* 14/APR/2003
*FUNCTION:
*  and 2 bitstreams
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size   -
*             in longs
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_and(
    SOC_SAND_INOUT    uint32    *bit_stream1,
    SOC_SAND_IN       uint32    *bit_stream2,
    SOC_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream1[i] &= bit_stream2[i];
  }
  return SOC_SAND_OK;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_xor
*DATE:
* 14/APR/2003
*FUNCTION:
*  xor 2 bitstreams
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size   -
*             in longs
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_xor(
    SOC_SAND_INOUT    uint32    *bit_stream1,
    SOC_SAND_IN       uint32    *bit_stream2,
    SOC_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream1[i] ^= bit_stream2[i];
  }
  return SOC_SAND_OK;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_parity
*DATE:
* 13/MAy/2008
*FUNCTION:
*  Get the parity of a bitstream
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN  uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN  uint32    start_bit -
*       Specifies the bit from which parity is calculated.
*    SOC_SAND_IN  uint32    nof_bits -
*       Specifies the number of bits from the start on
*       which parity is calculated.
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*    0 - numbers of 1 in the stream is even.
*    1 - numbers of 1 in the stream is odd.
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  soc_sand_bitstream_parity(
    SOC_SAND_IN  uint32    *bit_stream,
    SOC_SAND_IN  uint32    start_bit,
    SOC_SAND_IN  uint32    nof_bits
  )
{
  uint32
    buffer = 0;
  uint32
    idx = 0;
  uint32
    parity = 0;
  uint32
    start_bit_aligned = start_bit / SOC_SAND_NOF_BITS_IN_UINT32;
  const uint32
    *bit_stream_aligned = bit_stream + start_bit_aligned;

  for (idx = start_bit_aligned; idx < start_bit_aligned + nof_bits; ++idx)
  {
    buffer = *(bit_stream_aligned + (idx / SOC_SAND_NOF_BITS_IN_UINT32));
    parity = (~((~parity) ^ (SOC_SAND_BIT(idx % SOC_SAND_NOF_BITS_IN_UINT32) & buffer))) & SOC_SAND_BIT(idx % SOC_SAND_NOF_BITS_IN_UINT32);
  }

  return parity;
}
/*****************************************************
*NAME:
* soc_sand_bitstream_not
*DATE:
* 14/APR/2003
*FUNCTION:
*  Do 'not' operation on the bitstream
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT    uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size   -
*             in longs
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   Non-zero in case of an error
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_bitstream_not(
    SOC_SAND_INOUT    uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in uint32s */
  )
{
  uint32 i;
  /*
   */
  for(i=0; i<size; ++i)
  {
    bit_stream[i] ^= 0xFFFFFFFF;
  }
  return SOC_SAND_OK;
}

/*****************************************************
*NAME:
* soc_sand_bitstream_get_nof_on_bits
*DATE:
* 14/APR/2003
*FUNCTION:
*  Count the number of "ones" in the bit stream.
*  Examples:
*   + 0x0000_0000 ==> will results with 0.
*   + 0x0100_0000 ==> will results with 1.
*   + 0x0100_0000 0x0C00_0000 ==> will results with 3.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size   -
*             in longs
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   uint32
*     The number of ones
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  soc_sand_bitstream_get_nof_on_bits(
    SOC_SAND_IN       uint32    *bit_stream,
    SOC_SAND_IN       uint32    size /* in uint32s */
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
    nof_on_bits += soc_sand_nof_on_bits_in_long(bit_stream[i]);
  }

exit:
  return nof_on_bits;
}

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
  soc_sand_bitstream_offline_test(uint32 silent)
{
  uint32
    bit_stream[2],
    field;
  uint32
    pass;

  pass = TRUE;

  soc_sand_bitstream_clear(bit_stream, 2);
  soc_sand_bitstream_set_field(bit_stream, 0, 4, 0xF);
  if(!silent)
  {
    soc_sand_bitstream_print(bit_stream, 2);
    soc_sand_bitstream_print_beautify_1(bit_stream, 2, 7, 100);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }

  soc_sand_bitstream_clear(bit_stream, 2);
  soc_sand_bitstream_set_field(bit_stream, 4, 4, 0xF);
  if(!silent)
  {
    soc_sand_bitstream_print(bit_stream, 2);
    soc_sand_bitstream_print_beautify_1(bit_stream, 2, 5, 100);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }

  soc_sand_bitstream_clear(bit_stream, 2);
  soc_sand_bitstream_set_field(bit_stream, 30, 4, 0xF);
  if(!silent)
  {
    soc_sand_bitstream_print(bit_stream, 2);
    soc_sand_bitstream_print_beautify_1(bit_stream, 2, 4, 100);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }


  soc_sand_bitstream_clear(bit_stream, 2);
  soc_sand_bitstream_set_field(bit_stream, 4, 16, 0x12345);
  soc_sand_bitstream_set_field(bit_stream, 20, 28, 0xFFFFFFFF);
  if(!silent)
  {
    soc_sand_bitstream_print(bit_stream, 2);
    soc_sand_bitstream_print_beautify_1(bit_stream, 2, 3, 20);
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("\r\n")));
  }

  soc_sand_bitstream_get_field(bit_stream, 8, 12, &field);
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
  soc_sand_bitstream_print(
    SOC_SAND_IN uint32 *bit_stream,
    SOC_SAND_IN uint32 size /* in uint32s */
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
    LOG_CLI((BSL_META("soc_sand_bitstream_print received NULL ptr\n\r")));
    goto exit;
  }

  LOG_CLI((BSL_META("(Hex format, Maximum of %u bits per line.)\r\n"),
nof_longs_per_line*(int)SOC_SAND_NOF_BITS_IN_UINT32));
  /*
   */
  not_lines = SOC_SAND_DIV_ROUND_UP(size, nof_longs_per_line);
  for(line_i=0; line_i< not_lines; ++line_i)
  {
    LOG_CLI((BSL_META("[%4u-%4u]"),
SOC_SAND_MIN( (line_i+1)*nof_longs_per_line*(int)SOC_SAND_NOF_BITS_IN_UINT32-1, size * (int)SOC_SAND_NOF_BITS_IN_UINT32 - 1),
             line_i*nof_longs_per_line*(int)SOC_SAND_NOF_BITS_IN_UINT32
             ));
    for(long_i = SOC_SAND_MIN( (line_i+1)*nof_longs_per_line - 1, size-1);
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
* soc_sand_bitstream_set_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function writes the first buffer
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
  )
{
  uint32
    end,
    nof_bits_iter_acc,
    nof_bits_iter,
    nof_words,
    iter;
  uint32
    res = SOC_SAND_OK;
  uint32
    field;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_BITSTREAM_SET_ANY_FIELD);

  SOC_SAND_CHECK_NULL_INPUT(output_buffer);
  SOC_SAND_CHECK_NULL_INPUT(input_buffer);

  end = nof_bits + start_bit - 1;
  if((end / SOC_SAND_NOF_BITS_IN_UINT32) == (start_bit / SOC_SAND_NOF_BITS_IN_UINT32))
  {
    field = SOC_SAND_GET_BITS_RANGE(*input_buffer, nof_bits - 1, 0);
    *(output_buffer + (start_bit / SOC_SAND_NOF_BITS_IN_UINT32)) &= SOC_SAND_ZERO_BITS_MASK(end % SOC_SAND_NOF_BITS_IN_UINT32, start_bit % SOC_SAND_NOF_BITS_IN_UINT32);
    *(output_buffer + (start_bit / SOC_SAND_NOF_BITS_IN_UINT32)) |= SOC_SAND_SET_BITS_RANGE(field, end % SOC_SAND_NOF_BITS_IN_UINT32, start_bit % SOC_SAND_NOF_BITS_IN_UINT32);
  }
  else
  {
    nof_words = SOC_SAND_DIV_ROUND_UP(nof_bits, SOC_SAND_NOF_BITS_IN_UINT32);
    for(
        iter = 0, nof_bits_iter_acc = nof_bits;
        iter < nof_words;
        ++iter, nof_bits_iter_acc -= SOC_SAND_NOF_BITS_IN_UINT32
       )
    {
      nof_bits_iter = nof_bits_iter_acc > SOC_SAND_NOF_BITS_IN_UINT32 ? SOC_SAND_NOF_BITS_IN_UINT32 : nof_bits_iter_acc;

      res = soc_sand_bitstream_set_field(
              output_buffer,
              start_bit + iter * SOC_SAND_NOF_BITS_IN_UINT32,
              nof_bits_iter,
              *(input_buffer + iter)
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, iter, exit);
    }
  }

 exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_sand_bitstream_set_any_field()",0,0);
}

/*****************************************************
*NAME:
* soc_sand_bitstream_get_any_field
*DATE:
* 01/10/2007
*FUNCTION:
*  Having two buffers, the function reads the first buffer
*  from an offset and writes it to the second buffer.
*  The function zeroes the bits in the second buffer
*  from the end of the field to the end of the word (word is uint32)
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
  )
{
  uint32
    end,
    nof_bits_iter,
    nof_words,
    iter;
  uint32
    res = SOC_SAND_OK;
  uint32
    field;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_BITSTREAM_GET_ANY_FIELD);

  SOC_SAND_CHECK_NULL_INPUT(input_buffer);
  SOC_SAND_CHECK_NULL_INPUT(output_buffer);

  nof_words = SOC_SAND_DIV_ROUND_UP(nof_bits, SOC_SAND_NOF_BITS_IN_UINT32);

  end = nof_bits + start_bit - 1;
  if((end / SOC_SAND_NOF_BITS_IN_UINT32) == (start_bit / SOC_SAND_NOF_BITS_IN_UINT32))
  {
    /* There is no bug here - the pointer arithmatic done here is correct */
    /* coverity [ptr_arith]*/
    field = SOC_SAND_GET_BITS_RANGE(*(input_buffer + (start_bit / SOC_SAND_NOF_BITS_IN_UINT32)), end % SOC_SAND_NOF_BITS_IN_UINT32, start_bit % SOC_SAND_NOF_BITS_IN_UINT32);

    *output_buffer = 0;
    *output_buffer |= SOC_SAND_SET_BITS_RANGE(field, nof_bits - 1, 0);
  }
  else
  {
    for(
        iter = 0, nof_bits_iter = nof_bits;
        iter < nof_words;
        ++iter, nof_bits_iter -= SOC_SAND_NOF_BITS_IN_UINT32
       )
    {
      res = soc_sand_bitstream_get_field(
              input_buffer,
              start_bit + iter * SOC_SAND_NOF_BITS_IN_UINT32,
              nof_bits_iter > SOC_SAND_NOF_BITS_IN_UINT32 ? SOC_SAND_NOF_BITS_IN_UINT32 : nof_bits_iter,
              output_buffer + iter
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 2, exit);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_sand_bitstream_get_any_field()",0,0);
}

/*****************************************************
*NAME:
* soc_sand_bitstream_print_beautify_1
*DATE:
* 14/APR/2003
*FUNCTION:
*  Beautify printing utility.
*  Prints the bit-stream as a GROUP of items.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN       uint32    *bit_stream -
*       pointer to an array of uint32s,
*       which function as the bit stream
*    SOC_SAND_IN       uint32    size   -
*       in longs
*    SOC_SAND_IN uint32 max_dec_digits -
*       Item max printing size in Decimal.
*    SOC_SAND_IN uint32 max_nof_printed_items -
*       Maximum number of items to print.
*  SOC_SAND_INDIRECT:
*OUTPUT:
*  SOC_SAND_DIRECT:
*   uint32
*     The number of ones
*  SOC_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
void
  soc_sand_bitstream_print_beautify_1(
    SOC_SAND_IN uint32 *bit_stream,
    SOC_SAND_IN uint32 size, /* in uint32s */
    SOC_SAND_IN uint32 max_dec_digits,
    SOC_SAND_IN uint32 max_nof_printed_items
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
    LOG_CLI((BSL_META("soc_sand_bitstream_print_beautify_1 received NULL ptr\n\r")));
    goto exit;
  }

  if (max_dec_digits>10)
  {
    LOG_CLI((BSL_META("soc_sand_bitstream_print_beautify_1: received 'max_dec_digits>10'\n\r")));
    goto exit;
  }

  nof_on_bits = soc_sand_bitstream_get_nof_on_bits(bit_stream, size);
  LOG_CLI((BSL_META("Out of %u, %u are ON (%u item exists).\r\n"),
size*(int)SOC_SAND_NOF_BITS_IN_UINT32,
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
  for (bit_i=0; bit_i<(size*SOC_SAND_NOF_BITS_IN_UINT32); bit_i++)
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
    if (soc_sand_bitstream_test_bit(bit_stream, bit_i) == FALSE)
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

SOC_SAND_RET
  soc_sand_buff_xor(
    SOC_SAND_INOUT    unsigned  char    *buff1,
    SOC_SAND_IN       unsigned  char    *buff2,
    SOC_SAND_IN       uint32     size /* in chars */
  )
{
  uint32 i = 0;
  SOC_SAND_RET
    soc_sand_ret = SOC_SAND_OK;

  if (!buff1)
  {
    soc_sand_ret = 1;
    goto exit;
  }

  if (!buff2)
  {
    soc_sand_ret = 2;
    goto exit;
  }

  for(i=0; i<size; ++i)
  {
    buff1[i] ^= buff2[i];
  }

exit:
  return soc_sand_ret;
}

SOC_SAND_RET
  soc_sand_buff_print_non_zero(
    SOC_SAND_IN unsigned char* buff,
    SOC_SAND_IN uint32 buff_size
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

  SOC_SAND_RET
    soc_sand_ret = SOC_SAND_OK;
  char*
    func_name = "soc_sand_buff_print_non_zero";

  str_ptr = str_line;

  if (!buff)
  {
    soc_sand_ret = 1;
    goto exit;
  }

  if (!buff_size)
  {
    soc_sand_ret = 2;
    goto exit;
  }

  buff_longs = (const uint32* )buff;
  buff_chars = buff;

  rem_from_long = buff_size % sizeof(uint32);

  size_in_longs =
    SOC_SAND_DIV_ROUND_DOWN(buff_size, sizeof(uint32));

  if (!rem_from_long)
  {
    max_nof_lines = SOC_SAND_DIV_ROUND_UP(size_in_longs, max_columns);
  }
  else
  {
    max_nof_lines = SOC_SAND_DIV_ROUND_UP(size_in_longs + 1, max_columns);
  }

  for(curr_line = 0; curr_line < max_nof_lines; curr_line++)
  {
    /* ITERATION INITIALIZATIONS { */
    only_zer_this_line = TRUE;

    soc_sand_ret =
      soc_sand_os_memset(
        str_line,
        0x0,
        200
       );

    if (soc_sand_ret)
    {
      soc_sand_ret = 7;
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
  if (soc_sand_ret)
  {
    LOG_CLI((BSL_META("ERROR: function %s exited with error num. %d\n\r"),
             func_name,
             soc_sand_ret
             ));
  }

  return soc_sand_ret;
}

SOC_SAND_RET
  soc_sand_buff_print_all(
    SOC_SAND_IN unsigned char* buff,
    SOC_SAND_IN uint32 buff_size,
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

  SOC_SAND_RET
    soc_sand_ret = SOC_SAND_OK;
  char*
    func_name = "soc_sand_buff_print_non_zero";

  str_ptr = str_line;

  if (!buff)
  {
    soc_sand_ret = 1;
    goto exit;
  }

  if (!buff_size)
  {
    soc_sand_ret = 2;
    goto exit;
  }

  if (nof_columns != 0)
  {
    max_columns = nof_columns * sizeof(uint32);
  }
  buff_chars = buff;


  max_nof_lines = SOC_SAND_DIV_ROUND_UP(buff_size, max_columns);

  LOG_CLI((BSL_META("\n\r"
                    "Buffer of %u Bytes, values given in hexa:\n\r"
                    "-----------------------------------------\n\r"),
           (uint32)buff_size
           ));

  for(curr_line = 0; curr_line < max_nof_lines; curr_line++)
  {
    /* ITERATION INITIALIZATIONS { */

    soc_sand_ret =
      soc_sand_os_memset(
        str_line,
        0x0,
        200
       );

    if (soc_sand_ret)
    {
      soc_sand_ret = 7;
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
  if (soc_sand_ret)
  {
    LOG_CLI((BSL_META("ERROR: function %s exited with error num. %d\n\r"),
             func_name,
             soc_sand_ret
             ));
  }

  return soc_sand_ret;
}

/*
 *  Having two char buffers, orders msB->lsB, the function reads
 *  the first buffer from offset (offset taken from msb) and writes
 *  it to the second buffer.
 */
SOC_SAND_RET
  soc_sand_bitsteam_u8_ms_byte_first_get_field(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint8  *input_buffer,
    SOC_SAND_IN  uint32 start_bit_msb,
    SOC_SAND_IN  uint32 nof_bits,
    SOC_SAND_OUT uint32 *output_value
  )
{
  SOC_SAND_RET
    soc_sand_ret = SOC_SAND_OK;
  uint32
    idx,
    buf_sizes=0,
    tmp_output_value[2]={0},
    first_byte_ndx,
    last_byte_ndx;
  uint8
    *tmp_output_value_u8_ptr = (uint8*)&tmp_output_value;
 

  /* 32 bits at most */
  if (nof_bits > SOC_SAND_BIT_STREAM_FIELD_SET_SIZE)
  {
    soc_sand_ret = SOC_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR;
    goto exit;
  }

  /* Reverse input buffer relevant part, since received msb->lsb, but would be parsed lsb->msb */
  first_byte_ndx = start_bit_msb / SOC_SAND_NOF_BITS_IN_BYTE;
  last_byte_ndx = ((start_bit_msb + nof_bits - 1) / SOC_SAND_NOF_BITS_IN_BYTE);*output_value=0;

  for (idx = first_byte_ndx;
       idx <= last_byte_ndx;
       ++idx)
  {
    tmp_output_value_u8_ptr[last_byte_ndx - idx] = input_buffer[idx];
    buf_sizes += SOC_SAND_NOF_BITS_IN_BYTE;
  }

  /* If big endian, swap bytes (input is chars, but will be evaluated in uint32s) */

 #ifndef LE_HOST
  {
    tmp_output_value[0] = SOC_SAND_BYTE_SWAP(tmp_output_value[0]);
    
    if (last_byte_ndx > 4)
    {
      tmp_output_value[1] = SOC_SAND_BYTE_SWAP(tmp_output_value[1]);
    }
  }
#endif

  soc_sand_ret = soc_sand_bitstream_get_field(
    tmp_output_value,
    buf_sizes-(start_bit_msb%SOC_SAND_NOF_BITS_IN_BYTE+nof_bits),
    nof_bits,
    output_value
    );
  if( soc_sand_ret != SOC_SAND_OK )
  {
    goto exit;
  }

exit:
  return soc_sand_ret;
}

/* } */
#endif

#include <soc/dpp/SAND/Utils/sand_footer.h>
