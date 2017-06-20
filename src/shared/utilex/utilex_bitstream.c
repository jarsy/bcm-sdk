/* $Id: utilex_bitstream.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/** \file utilex_bitstream.c 
 *  
 * All common utilities related to bitstream. 
 *  
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDSWDNX_UTILSDNX

/*************
* INCLUDES  *
*************/
/* { */

#include <shared/bsl.h>
#include <sal/core/libc.h>
/*
 * SW is included
 */
#include <shared/swstate/access/sw_state_access.h>

#include <shared/shrextend/shrextend_debug.h>
#include <shared/utilex/utilex_bitstream.h>
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_integer_arithmetic.h>

/* } */

/*************
 * DEFINES   *
 *************/

#define UTILEX_BITSTREAM_FIELD_SIZE (UTILEX_NOF_BITS_IN_UINT32)

shr_error_e
utilex_bitstream_clear(
    uint32 * bit_stream,
    uint32 size)
{
    uint32 iii;
    /*
     */
    for (iii = 0; iii < size; ++iii)
    {
        bit_stream[iii] = 0;
    }
    return _SHR_E_NONE;
}

shr_error_e
utilex_bitstream_fill(
    uint32 * bit_stream,
    uint32 size)
{
    uint32 iii;
    /*
     */
    for (iii = 0; iii < size; ++iii)
    {
        bit_stream[iii] = 0xFFFFFFFF;
    }
    return _SHR_E_NONE;
}

shr_error_e
utilex_bitstream_set(
    uint32 * bit_stream,
    uint32 place,
    uint32 bit_indicator)
{
    shr_error_e ret;
    if (bit_indicator)
    {
        ret = utilex_bitstream_set_bit(bit_stream, place);
    }
    else
    {
        ret = utilex_bitstream_reset_bit(bit_stream, place);
    }
    return ret;
}

shr_error_e
utilex_bitstream_set_field(
    uint32 * bit_stream,
    uint32 start_bit,
    uint32 nof_bits,
    uint32 field)
{
    uint32 bit_stream_bit_i, field_bit_i;
    SHR_FUNC_INIT_VARS(NO_UNIT);
    /*
     * 32 bits at most
     */
    if (nof_bits > UTILEX_BITSTREAM_FIELD_SIZE)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_PARAM,
                                 "Number of bits (%u) is out of range. Maximum value is %u %s\r\n",
                                 nof_bits, (uint32) UTILEX_BITSTREAM_FIELD_SIZE, EMPTY);
    }
    for (bit_stream_bit_i = start_bit, field_bit_i = 0; field_bit_i < nof_bits; ++bit_stream_bit_i, ++field_bit_i)
    {
        SHR_IF_ERR_EXIT(utilex_bitstream_set(bit_stream, bit_stream_bit_i, (field >> field_bit_i) & 0x1));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_bitstream_get_field(
    uint32 * bit_stream,
    uint32 start_bit,
    uint32 nof_bits,
    uint32 * field)
{
    uint32 bit_stream_bit_i, field_bit_i;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    SHR_NULL_CHECK(field, _func_rv, "field");
    /*
     * 32 bits at most
     */
    if (nof_bits > UTILEX_BITSTREAM_FIELD_SIZE)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_PARAM,
                                 "Number of bits (%u) is out of range. Maximum value is %u %s\r\n",
                                 nof_bits, (uint32) UTILEX_BITSTREAM_FIELD_SIZE, EMPTY);
    }
    *field = 0;
    for (bit_stream_bit_i = start_bit, field_bit_i = 0; field_bit_i < nof_bits; ++bit_stream_bit_i, ++field_bit_i)
    {
        *field |= ((uint32) utilex_bitstream_test_bit(bit_stream, bit_stream_bit_i)) << field_bit_i;
    }
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_bitstream_set_bit(
    uint32 * bit_stream,
    uint32 place)
{
    bit_stream[place >> 5] |= SAL_BIT(place & 0x0000001F);
    return _SHR_E_NONE;
}

shr_error_e
utilex_bitstream_set_bit_range(
    uint32 * bit_stream,
    uint32 start_place,
    uint32 end_place)
{
    uint32 bit_i;

    for (bit_i = start_place; bit_i <= end_place; ++bit_i)
    {
        utilex_bitstream_set_bit(bit_stream, bit_i);
    }

    return _SHR_E_NONE;
}

shr_error_e
utilex_bitstream_reset_bit(
    uint32 * bit_stream,
    uint32 place)
{
    bit_stream[place >> 5] &= SAL_RBIT(place & 0x0000001F);
    return _SHR_E_NONE;
}

shr_error_e
utilex_bitstream_reset_bit_range(
    uint32 * bit_stream,
    uint32 start_place,
    uint32 end_place)
{
    uint32 bit_i;

    for (bit_i = start_place; bit_i <= end_place; ++bit_i)
    {
        utilex_bitstream_reset_bit(bit_stream, bit_i);
    }

    return _SHR_E_NONE;
}

int
utilex_bitstream_test_bit(
    uint32 * bit_stream,
    uint32 place)
{
    uint32 result;
    /*
     */
    result = bit_stream[place >> 5] & SAL_BIT(place & 0x0000001F);
    /*
     */
    if (result)
    {
        return 1;
    }
    return 0;
}

int
utilex_bitstream_test_and_reset_bit(
    uint32 * bit_stream,
    uint32 place)
{
    int result;

    /*
     * Get the bit. 
     */
    result = utilex_bitstream_test_bit(bit_stream, place);

    /*
     * Set the bit to zero. 
     */
    utilex_bitstream_reset_bit(bit_stream, place);

    return result;
}

int
utilex_bitstream_test_and_set_bit(
    uint32 * bit_stream,
    uint32 place)
{
    int result;

    /*
     * Get the bit. 
     */
    result = utilex_bitstream_test_bit(bit_stream, place);

    /*
     * Set the bit to one. 
     */
    utilex_bitstream_set_bit(bit_stream, place);

    return result;
}

shr_error_e
utilex_bitstream_bit_flip(
    uint32 * bit_stream,
    uint32 place)
{
    shr_error_e shr_error_e = _SHR_E_NONE;

    bit_stream[place >> 5] ^= SAL_BIT(place & 0x0000001F);

    return shr_error_e;
}

int
utilex_bitstream_have_one_in_range(
    uint32 * bit_stream,
    uint32 start_place,
    uint32 end_place)
{
    uint32 bit_i;
    int result;

    result = FALSE;

    for (bit_i = start_place; bit_i <= end_place; ++bit_i)
    {
        if (utilex_bitstream_test_bit(bit_stream, bit_i))
        {
            result = TRUE;
            goto exit;
        }
    }

exit:
    return result;
}

int
utilex_bitstream_have_one(
    uint32 * bit_stream,
    uint32 size                 /* in uint32s */
    )
{
    uint32 word_i;
    int result;
    const uint32 *pointer;

    result = FALSE;

    pointer = bit_stream;
    for (word_i = 0; word_i < size; ++word_i, ++pointer)
    {
        if ((*pointer) != 0)
        {
            result = TRUE;
            goto exit;
        }
    }

exit:
    return result;
}

shr_error_e
utilex_bitstream_or(
    uint32 * bit_stream1,
    uint32 * bit_stream2,
    uint32 size                 /* in uint32s */
    )
{
    uint32 i;
    /*
     */
    for (i = 0; i < size; ++i)
    {
        bit_stream1[i] |= bit_stream2[i];
    }
    return _SHR_E_NONE;
}

shr_error_e
utilex_bitstream_and(
    uint32 * bit_stream1,
    uint32 * bit_stream2,
    uint32 size                 /* in uint32s */
    )
{
    uint32 i;
    /*
     */
    for (i = 0; i < size; ++i)
    {
        bit_stream1[i] &= bit_stream2[i];
    }
    return _SHR_E_NONE;
}

shr_error_e
utilex_bitstream_xor(
    uint32 * bit_stream1,
    uint32 * bit_stream2,
    uint32 size                 /* in uint32s */
    )
{
    uint32 i;
    /*
     */
    for (i = 0; i < size; ++i)
    {
        bit_stream1[i] ^= bit_stream2[i];
    }
    return _SHR_E_NONE;
}

uint32
utilex_bitstream_parity(
    uint32 * bit_stream,
    uint32 start_bit,
    uint32 nof_bits)
{
    uint32 buffer = 0;
    uint32 idx = 0;
    uint32 parity = 0;
    uint32 start_bit_aligned = start_bit / UTILEX_NOF_BITS_IN_UINT32;
    const uint32 *bit_stream_aligned = bit_stream + start_bit_aligned;

    for (idx = start_bit_aligned; idx < start_bit_aligned + nof_bits; ++idx)
    {
        buffer = *(bit_stream_aligned + (idx / UTILEX_NOF_BITS_IN_UINT32));
        parity =
            (~((~parity) ^ (SAL_BIT(idx % UTILEX_NOF_BITS_IN_UINT32) & buffer))) & SAL_BIT(idx %
                                                                                           UTILEX_NOF_BITS_IN_UINT32);
    }

    return parity;
}

shr_error_e
utilex_bitstream_not(
    uint32 * bit_stream,
    uint32 size                 /* in uint32s */
    )
{
    uint32 i;
    /*
     */
    for (i = 0; i < size; ++i)
    {
        bit_stream[i] ^= 0xFFFFFFFF;
    }
    return _SHR_E_NONE;
}

uint32
utilex_bitstream_get_nof_on_bits(
    uint32 * bit_stream,
    uint32 size                 /* in uint32s */
    )
{
    uint32 nof_on_bits, i;

    nof_on_bits = 0;
    /*
     */
    if (NULL == bit_stream)
    {
        goto exit;
    }

    for (i = 0; i < size; ++i)
    {
        nof_on_bits += utilex_nof_on_bits_in_long(bit_stream[i]);
    }

exit:
    return nof_on_bits;
}

uint32
utilex_bitstream_offline_test(
    uint32 silent)
{
    uint32 bit_stream[2], field;
    uint32 pass;
    shr_error_e res ;

    pass = TRUE;
    res = _SHR_E_NONE;

    utilex_bitstream_clear(bit_stream, 2);
    utilex_bitstream_set_field(bit_stream, 0, 4, 0xF);
    if (!silent)
    {
        utilex_bitstream_print(bit_stream, 2);
        utilex_bitstream_print_beautify_1(bit_stream, 2, 7, 100);
        LOG_INFO(BSL_LS_SOC_COMMON, (BSL_META("\r\n")));
    }

    utilex_bitstream_clear(bit_stream, 2);
    utilex_bitstream_set_field(bit_stream, 4, 4, 0xF);
    if (!silent)
    {
        utilex_bitstream_print(bit_stream, 2);
        utilex_bitstream_print_beautify_1(bit_stream, 2, 5, 100);
        LOG_INFO(BSL_LS_SOC_COMMON, (BSL_META("\r\n")));
    }

    utilex_bitstream_clear(bit_stream, 2);
    utilex_bitstream_set_field(bit_stream, 30, 4, 0xF);
    if (!silent)
    {
        utilex_bitstream_print(bit_stream, 2);
        utilex_bitstream_print_beautify_1(bit_stream, 2, 4, 100);
        LOG_INFO(BSL_LS_SOC_COMMON, (BSL_META("\r\n")));
    }

    utilex_bitstream_clear(bit_stream, 2);
    utilex_bitstream_set_field(bit_stream, 4, 16, 0x12345);
    utilex_bitstream_set_field(bit_stream, 20, 28, 0xFFFFFFFF);
    if (!silent)
    {
        utilex_bitstream_print(bit_stream, 2);
        utilex_bitstream_print_beautify_1(bit_stream, 2, 3, 20);
        LOG_INFO(BSL_LS_SOC_COMMON, (BSL_META("\r\n")));
    }

    res = utilex_bitstream_get_field(bit_stream, 8, 12, &field);
    if (res != _SHR_E_NONE)
    {
        pass &= 0;
    }
    else if (field != 0x234)
    {
        pass &= 0;
    }

    return pass;
}

void
utilex_bitstream_print(
    uint32 * bit_stream,
    uint32 size                 /* in uint32s */
    )
{
    uint32 line_i, not_lines;
    int long_i;
    const int nof_longs_per_line = 8;

    if (NULL == bit_stream)
    {
        LOG_CLI((BSL_META("utilex_bitstream_print received NULL ptr\n\r")));
        goto exit;
    }

    LOG_CLI((BSL_META("(Hex format, Maximum of %u bits per line.)\r\n"),
             nof_longs_per_line * (int) UTILEX_NOF_BITS_IN_UINT32));
    /*
     */
    not_lines = UTILEX_DIV_ROUND_UP(size, nof_longs_per_line);
    for (line_i = 0; line_i < not_lines; ++line_i)
    {
        LOG_CLI((BSL_META("[%4u-%4u]"),
                 UTILEX_MIN((line_i +
                             1) * nof_longs_per_line *
                            (int) UTILEX_NOF_BITS_IN_UINT32 - 1,
                            size * (int) UTILEX_NOF_BITS_IN_UINT32 - 1),
                 line_i * nof_longs_per_line * (int) UTILEX_NOF_BITS_IN_UINT32));
        for (long_i =
             UTILEX_MIN((line_i + 1) * nof_longs_per_line - 1, size - 1);
             long_i >= (int) (line_i * nof_longs_per_line); --long_i)
        {
            LOG_CLI((BSL_META("%08X"), bit_stream[long_i]));
        }
        LOG_CLI((BSL_META("\r\n")));
    }

exit:
    return;
}

shr_error_e
utilex_bitstream_set_any_field(
    uint32 * input_buffer,
    uint32 start_bit,
    uint32 nof_bits,
    uint32 * output_buffer)
{
    uint32 end, nof_bits_iter_acc, nof_bits_iter, nof_words, iter;
    shr_error_e res = _SHR_E_NONE;
    uint32 field;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    SHR_NULL_CHECK(output_buffer, _func_rv, "output_buffer");
    SHR_NULL_CHECK(input_buffer, _func_rv, "input_buffer");

    end = nof_bits + start_bit - 1;
    if ((end / UTILEX_NOF_BITS_IN_UINT32) == (start_bit / UTILEX_NOF_BITS_IN_UINT32))
    {
        field = UTILEX_GET_BITS_RANGE(*input_buffer, nof_bits - 1, 0);
        *(output_buffer + (start_bit / UTILEX_NOF_BITS_IN_UINT32)) &=
            UTILEX_ZERO_BITS_MASK(end % UTILEX_NOF_BITS_IN_UINT32, start_bit % UTILEX_NOF_BITS_IN_UINT32);
        *(output_buffer + (start_bit / UTILEX_NOF_BITS_IN_UINT32)) |=
            UTILEX_SET_BITS_RANGE(field, end % UTILEX_NOF_BITS_IN_UINT32, start_bit % UTILEX_NOF_BITS_IN_UINT32);
    }
    else
    {
        nof_words = UTILEX_DIV_ROUND_UP(nof_bits, UTILEX_NOF_BITS_IN_UINT32);
        for (iter = 0, nof_bits_iter_acc = nof_bits;
             iter < nof_words; ++iter, nof_bits_iter_acc -= UTILEX_NOF_BITS_IN_UINT32)
        {
            nof_bits_iter =
                nof_bits_iter_acc > UTILEX_NOF_BITS_IN_UINT32 ? UTILEX_NOF_BITS_IN_UINT32 : nof_bits_iter_acc;

            res = utilex_bitstream_set_field(output_buffer,
                                             start_bit +
                                             iter * UTILEX_NOF_BITS_IN_UINT32, nof_bits_iter, *(input_buffer + iter));
            SHR_IF_ERR_EXIT(res);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_bitstream_get_any_field(
    uint32 * input_buffer,
    uint32 start_bit,
    uint32 nof_bits,
    uint32 * output_buffer)
{
    uint32 end, nof_bits_iter, nof_words, iter;
    shr_error_e res = _SHR_E_NONE;
    uint32 field;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    SHR_NULL_CHECK(input_buffer, _func_rv, "field");
    SHR_NULL_CHECK(output_buffer, _func_rv, "output_buffer");

    nof_words = UTILEX_DIV_ROUND_UP(nof_bits, UTILEX_NOF_BITS_IN_UINT32);

    end = nof_bits + start_bit - 1;
    if ((end / UTILEX_NOF_BITS_IN_UINT32) == (start_bit / UTILEX_NOF_BITS_IN_UINT32))
    {
        /*
         * There is no bug here - the pointer arithmetic done here is correct
         */
        /*
         * coverity [ptr_arith]
         */
        field =
            UTILEX_GET_BITS_RANGE(*
                                  (input_buffer +
                                   (start_bit / UTILEX_NOF_BITS_IN_UINT32)),
                                  end % UTILEX_NOF_BITS_IN_UINT32, start_bit % UTILEX_NOF_BITS_IN_UINT32);

        *output_buffer = 0;
        *output_buffer |= UTILEX_SET_BITS_RANGE(field, nof_bits - 1, 0);
    }
    else
    {
        for (iter = 0, nof_bits_iter = nof_bits; iter < nof_words; ++iter, nof_bits_iter -= UTILEX_NOF_BITS_IN_UINT32)
        {
            res = utilex_bitstream_get_field(input_buffer,
                                             start_bit +
                                             iter * UTILEX_NOF_BITS_IN_UINT32,
                                             nof_bits_iter >
                                             UTILEX_NOF_BITS_IN_UINT32 ?
                                             UTILEX_NOF_BITS_IN_UINT32 : nof_bits_iter, output_buffer + iter);
            SHR_IF_ERR_EXIT(res);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

void
utilex_bitstream_print_beautify_1(
    uint32 * bit_stream,
    uint32 size,                /* in uint32s */
    uint32 max_dec_digits,
    uint32 max_nof_printed_items)
{
    uint32 item_i, nof_on_bits, bit_i;
    const int nof_number_per_line = 8;
    char format[20] = "";

    if (NULL == bit_stream)
    {
        LOG_CLI((BSL_META("utilex_bitstream_print_beautify_1 received NULL ptr\n\r")));
        goto exit;
    }

    if (max_dec_digits > 10)
    {
        LOG_CLI((BSL_META("utilex_bitstream_print_beautify_1: received 'max_dec_digits>10'\n\r")));
        goto exit;
    }

    nof_on_bits = utilex_bitstream_get_nof_on_bits(bit_stream, size);
    LOG_CLI((BSL_META("Out of %u, %u are ON (%u item exists).\r\n"),
             size * (int) UTILEX_NOF_BITS_IN_UINT32, nof_on_bits, nof_on_bits));
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
    item_i = 0;
    for (bit_i = 0; bit_i < (size * UTILEX_NOF_BITS_IN_UINT32); bit_i++)
    {
        if (item_i >= max_nof_printed_items)
        {
            /*
             * Do not show more items
             */
            LOG_CLI((BSL_META(" .... More Items to print. Exist .... \r\n")));
            break;
        }
        if (item_i >= nof_on_bits)
        {
            /*
             * No items more to show.
             */
            break;
        }
        if (utilex_bitstream_test_bit(bit_stream, bit_i) == FALSE)
        {
            continue;
        }
        item_i++;
        LOG_CLI((format, bit_i));       /*Print the Number */
        if ((item_i % nof_number_per_line) == 0)
        {
            LOG_CLI((BSL_META("\r\n" "  ")));
        }
    }
    LOG_CLI((BSL_META("]\r\n")));

exit:
    return;
}

shr_error_e
utilex_bitstream_buff_print_all(
    unsigned char *buff,
    uint32 buff_size,
    uint32 nof_columns)
{
    uint32 curr_line = 0, curr_col = 0, max_nof_lines = 0;
    uint32 max_columns = 1;     /* must be non-zero! */
    uint32 curr_offset = 0;
    unsigned char curr_char = 0;
    const unsigned char *buff_chars = NULL;
    char *str_ptr = NULL;
    char str_line[200] = "";

    shr_error_e shr_error_e = _SHR_E_NONE;
    char *func_name = "utilex_bitstream_buff_print_all";

    str_ptr = str_line;

    if (!buff)
    {
        shr_error_e = 1;
        goto exit;
    }

    if (!buff_size)
    {
        shr_error_e = 2;
        goto exit;
    }

    if (nof_columns != 0)
    {
        max_columns = nof_columns * sizeof(uint32);
    }
    buff_chars = buff;

    max_nof_lines = UTILEX_DIV_ROUND_UP(buff_size, max_columns);

    LOG_CLI((BSL_META("\n\r"
                      "Buffer of %u Bytes, values given in hexa:\n\r"
                      "-----------------------------------------\n\r"), (uint32) buff_size));

    for (curr_line = 0; curr_line < max_nof_lines; curr_line++)
    {
        /*
         * ITERATION INITIALIZATIONS { 
         */

        sal_memset(str_line, 0x0, 200);

        str_ptr = str_line;

        str_ptr += sal_sprintf(str_ptr, "OFFSET LONGS: %6d", (uint32) (curr_offset / sizeof(uint32)));
        /*
         * ITERATION INITIALIZATIONS } 
         */

        for (curr_col = 0; curr_col < max_columns; curr_col++)
        {
            /*
             * build one line and check whether there are non-zero values in it 
             */

            if (curr_offset < buff_size)
            {
                curr_char = (unsigned char) (*(buff_chars + curr_offset));
                if (((curr_offset) % sizeof(uint32)) == 0)
                {
                    str_ptr += sal_sprintf(str_ptr, " %02X", curr_char);
                }
                else
                {
                    str_ptr += sal_sprintf(str_ptr, "%02X", curr_char);
                }
            }
            curr_offset++;      /* curr_offset = (curr_line * max_columns) + curr_col */
        }

        str_ptr += sal_sprintf(str_ptr, "\n\r");
        LOG_CLI((str_line));
    }

exit:
    if (shr_error_e)
    {
        LOG_CLI((BSL_META("ERROR: function %s exited with error num. %d\n\r"), func_name, shr_error_e));
    }

    return shr_error_e;
}

shr_error_e
utilex_bitstream_u8_ms_byte_first_get_field(
    int unit,
    uint8 * input_buffer,
    uint32 start_bit_msb,
    uint32 nof_bits,
    uint32 * output_value)
{
    uint32 idx, buf_sizes = 0, tmp_output_value[2] = {
        0
    }, first_byte_ndx, last_byte_ndx;
    uint8 *tmp_output_value_u8_ptr = (uint8 *) & tmp_output_value;

    SHR_FUNC_INIT_VARS(NO_UNIT);
    /*
     * 32 bits at most 
     */
    if (nof_bits > UTILEX_BIT_STREAM_FIELD_SET_SIZE)
    {
        SHR_IF_ERR_EXIT(_SHR_E_PARAM);
    }
    /*
     * Reverse input buffer relevant part, since received msb->lsb, but would be parsed lsb->msb 
     */
    first_byte_ndx = start_bit_msb / UTILEX_NOF_BITS_IN_BYTE;
    last_byte_ndx = ((start_bit_msb + nof_bits - 1) / UTILEX_NOF_BITS_IN_BYTE);
    *output_value = 0;

    for (idx = first_byte_ndx; idx <= last_byte_ndx; ++idx)
    {
        tmp_output_value_u8_ptr[last_byte_ndx - idx] = input_buffer[idx];
        buf_sizes += UTILEX_NOF_BITS_IN_BYTE;
    }
    /*
     * If big endian, swap bytes (input is chars, but will be evaluated in uint32s) 
     */
#ifndef LE_HOST
    {
        tmp_output_value[0] = UTILEX_BYTE_SWAP(tmp_output_value[0]);

        if (last_byte_ndx > 4)
        {
            tmp_output_value[1] = UTILEX_BYTE_SWAP(tmp_output_value[1]);
        }
    }
#endif

    SHR_IF_ERR_EXIT(utilex_bitstream_get_field(tmp_output_value,
                                               buf_sizes -
                                               (start_bit_msb %
                                                UTILEX_NOF_BITS_IN_BYTE + nof_bits), nof_bits, output_value));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_bitstream_add_one(
    uint32 * bit_stream,
    uint32 size                 /* in uint32s */
    )
{
    uint32 i;

    SHR_FUNC_INIT_VARS(NO_UNIT);

    /*
     * Find the first word that not full 1's and add one to it. 
     * Zero all former words in bitstream 
     */
    for (i = 0; i < (size - 1); i++)
    {
        if (bit_stream[i] == UTILEX_U32_MAX)
        {
            bit_stream[i] = 0;
        }
        else
        {
            bit_stream[i] += 1;
            break;
        }
    }

    /* 
     * In case of reached the last word: 
     * If last word is full 1's, return error of overflow
     */
    if (i == (size - 1))
    {
        if (bit_stream[i] == UTILEX_U32_MAX)
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Cannot add 1 to bitsteram of size %d. overflow detected.\r\n", size);
        }
        else
        {
            bit_stream[i] += 1;
        }
    }

exit:
    SHR_FUNC_EXIT;
}
