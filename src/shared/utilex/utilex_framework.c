/*
 * $Id: utilex_framework.c,v 1.9 Broadcom SDK $
 $Copyright: (c) 2016 Broadcom.
 Broadcom Proprietary and Confidential. All rights reserved.$ 
 */
/** \file utilex_framework.c 
 *  
 * All common utilities related to framework. 
 *  
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDSWDNX_UTILSDNX

/*************
* INCLUDES  *
*************/
/*
 * { 
 */

#include <shared/bsl.h>
#include <shared/shrextend/shrextend_debug.h>
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_integer_arithmetic.h>
#include <sal/types.h>

/*
 * } 
 */
shr_error_e
utilex_pp_mac_address_struct_to_long(
    utilex_pp_mac_address_t * mac_add_struct,
    uint32 mac_add_long[UTILEX_PP_MAC_ADDRESS_NOF_UINT32S])
{
    uint32 tmp;
    uint32 char_indx, long_indx, write_to;
    SHR_FUNC_INIT_VARS(NO_UNIT);

    write_to = 0;
    mac_add_long[0] = 0;
    mac_add_long[1] = 0;
    long_indx = 0;

    for (char_indx = 0; char_indx < UTILEX_PP_MAC_ADDRESS_NOF_U8; ++char_indx)
    {
        tmp = mac_add_struct->address[UTILEX_PP_MAC_ADDRESS_NOF_U8 - char_indx - 1];
        mac_add_long[long_indx] |=
            UTILEX_SET_BITS_RANGE(tmp,
                                  (UTILEX_NOF_BITS_IN_CHAR * (write_to + 1) - 1), UTILEX_NOF_BITS_IN_CHAR * write_to);
        ++write_to;
        if (write_to >= sizeof(uint32))
        {
            long_indx = 1;
            write_to = 0;
        }
    }
    SHR_EXIT() ;
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
utilex_pp_mac_address_long_to_struct(
    uint32 mac_add_long[UTILEX_PP_MAC_ADDRESS_NOF_UINT32S],
    utilex_pp_mac_address_t * mac_add_struct)
{
    uint32 char_indx, long_indx, read_from;
    SHR_FUNC_INIT_VARS(NO_UNIT);

    read_from = 0;
    long_indx = 0;
    for (char_indx = 0; char_indx < UTILEX_PP_MAC_ADDRESS_NOF_U8; ++char_indx)
    {
        mac_add_struct->address[UTILEX_PP_MAC_ADDRESS_NOF_U8 - char_indx - 1] =
            (uint8) UTILEX_GET_BITS_RANGE(mac_add_long[long_indx],
                                          UTILEX_NOF_BITS_IN_CHAR * (read_from + 1) - 1,
                                          UTILEX_NOF_BITS_IN_CHAR * read_from);
        ++read_from;
        if (read_from >= sizeof(uint32))
        {
            long_indx = 1;
            read_from = 0;
        }
    }
    SHR_EXIT() ;
exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 *  set field onto the register
 * 
 * \par DIRECT INPUT
 *   \param [in,out] *reg_val  - \n
 *       the value of the register to be changed
 *   \param [in] ms_bit         -\n
 *       most significant bit where to set the field_val,
 *   \param [in] ls_bit         -\n
 *       less significant bit where to set the field_val 
 *   \param [in] field_val         -\n
 *       field to set into reg_val
 *  \par DIRECT OUTPUT:
 *    Non-zero in case of an error
 *  \par INDIRECT OUTPUT
 *   \param *reg_val the value of the register after setting the value into it.
 */
shr_error_e
utilex_set_field(
    uint32 * reg_val,
    uint32 ms_bit,
    uint32 ls_bit,
    uint32 field_val)
{
    uint32 tmp_reg;
    shr_error_e ret;

    ret = _SHR_E_NONE;
    tmp_reg = *reg_val;

    /*
     * 32 bits at most
     */
    if (ms_bit - ls_bit + 1 > 32)
    {
        ret = _SHR_E_PARAM;
        goto exit;
    }

    tmp_reg &= UTILEX_ZERO_BITS_MASK(ms_bit, ls_bit);

    tmp_reg |= UTILEX_SET_BITS_RANGE(field_val, ms_bit, ls_bit);

    *reg_val = tmp_reg;

exit:
    return ret;

}

/**
 * \brief
 * Convert an uint8 array to an uint32 array 
 * 
 * \par DIRECT INPUT
 *   \param [in] *u8_val  - \n
 *       pointer to an array of uint8, 
 *   \param [in] nof_bytes -\n
 *       size of the array  
 *  \par DIRECT OUTPUT:
 *    Non-zero in case of an error
 *  \par INDIRECT OUTPUT
 *   \param *u32_val -\n
 *   the array of uint32  
 */
shr_error_e
utilex_U8_to_U32(
    uint8 * u8_val,
    uint32 nof_bytes,
    uint32 * u32_val)
{
    uint32 u8_indx, cur_u8_indx, u32_indx;
    uint8 *cur_u8;
    shr_error_e ret = _SHR_E_NONE;

    if (!u8_val || !u32_val)
    {
        ret = _SHR_E_PARAM;
        goto exit;
    }

    cur_u8_indx = 0;
    u32_indx = 0;

    for (cur_u8 = u8_val, u8_indx = 0; u8_indx < nof_bytes; ++u8_indx, ++cur_u8)
    {
        utilex_set_field(&(u32_val[u32_indx]),
                         (cur_u8_indx + 1) * SAL_UINT8_NOF_BITS - 1, cur_u8_indx * SAL_UINT8_NOF_BITS, *cur_u8);

        cur_u8_indx++;
        if (cur_u8_indx >= sizeof(uint32))
        {
            cur_u8_indx = 0;
            ++u32_indx;
        }
    }
exit:
    return ret;
}

/**
 * \brief
 * Convert an uint32 array to an uint8 array 
 * 
 * \par DIRECT INPUT
 *   \param [in] *u32_val  - \n
 *       pointer to an array of uint32
 *   \param [in] nof_bytes -\n
 *       nof bytes  
 *  \par DIRECT OUTPUT:
 *    Non-zero in case of an error
 *  \par INDIRECT OUTPUT
 *   \param *u8_val the array of uint8  
 */
shr_error_e
utilex_U32_to_U8(
    uint32 * u32_val,
    uint32 nof_bytes,
    uint8 * u8_val)
{
    uint32 u8_indx, cur_u8_indx;
    uint32 *cur_u32;

    shr_error_e ret = _SHR_E_NONE;

    if (!u8_val || !u32_val)
    {
        ret = _SHR_E_PARAM;
        goto exit;
    }

    cur_u8_indx = 0;
    for (cur_u32 = u32_val, u8_indx = 0; u8_indx < nof_bytes; ++u8_indx)
    {
        u8_val[u8_indx] = (uint8)
            UTILEX_GET_BITS_RANGE(*cur_u32, (cur_u8_indx + 1) * SAL_UINT8_NOF_BITS - 1,
                                  cur_u8_indx * SAL_UINT8_NOF_BITS);

        ++cur_u8_indx;
        if (cur_u8_indx >= sizeof(uint32))
        {
            cur_u8_indx = 0;
            ++cur_u32;
        }
    }
exit:
    return ret;
}
