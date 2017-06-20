/**
 * \file diag_sand_utils.c
 *
 * Misc. utilities for shell commands
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <sal/core/regex.h>
#include <sal/appl/sal.h>

#include <shared/utilex/utilex_integer_arithmetic.h>
#include <shared/shrextend/shrextend_debug.h>
#include <appl/diag/system.h>
#include <appl/diag/sand/diag_sand_utils.h>

#define BSL_LOG_MODULE BSL_LS_APPL_SHELL

/*
 * Utility routine to concatenate the first argument ("first"), with
 * the remaining arguments, with commas separating them.
 */
void
diag_sand_utils_collect_comma_args(
    args_t * a,
    char *valstr,
    char *first)
{
    char *s;

    sal_strncpy(valstr, first, ARGS_BUFFER);

    while ((s = ARG_GET(a)) != 0)
    {
        sal_strncat(valstr, ",", ARGS_BUFFER - sal_strlen(valstr) - 1);
        sal_strncat(valstr, s, ARGS_BUFFER - sal_strlen(valstr) - 1);
    }
}

cmd_result_t
diag_sand_error_get(
    int shr_ret)
{
    cmd_result_t ret;
    switch (shr_ret)
    {
        case _SHR_E_NONE:
            ret = CMD_OK;
            break;
        case _SHR_E_PARAM:
            ret = CMD_USAGE;
            break;
        case _SHR_E_NOT_FOUND:
            ret = CMD_NFND;
            break;
        default:
            ret = CMD_FAIL;
            break;
    }
    return ret;;
}

void
diag_sand_value_to_str(
    uint32 * value,
    int value_bit_size,
    char *value_str,
    int value_str_size)
{
    int i, j;
    int byte_size, long_size;
    int dest_byte_index, source_byte_index, real_byte_index;
    uint8 *source = (uint8 *) value;

    byte_size = BITS2BYTES(value_bit_size);
    if (byte_size >= value_str_size)
    {
        cli_out("String length:%d cannot contain value size:%d\n", value_str_size, value_bit_size);
        return;
    }

    long_size = BYTES2WORDS(byte_size);
    sal_memset(value_str, 0, value_str_size);

    dest_byte_index = 0;
    for (i = 0; i < long_size; i++)
    {
        for (j = 0; j < 4; j++)
        {
            source_byte_index = 4 * (long_size - 1 - i) + j;
            real_byte_index = 4 * (long_size - 1 - i) + 3 - j;
            if (real_byte_index >= byte_size)
                continue;
            sprintf(&value_str[2 * dest_byte_index], "%02x", source[source_byte_index]);
            dest_byte_index++;
        }
    }
}

shr_error_e
diag_sand_compare_init(
    char *match_n,
    void **compare_handle_p)
{
    SHR_FUNC_INIT_VARS(NO_UNIT);

    if (ISEMPTY(match_n))
    {
        *compare_handle_p = NULL;
    }
    else
    {
#if !defined(__KERNEL__) && !defined(VXWORKS) && !defined(NO_REGEX)
        regex_t *regex = sal_alloc(sizeof(regex_t), "regex");
        /*
         * Compile regular expression 
         */
        if (regcomp(regex, match_n, REG_EXTENDED | REG_NOSUB))
        {
            SHR_CLI_EXIT(_SHR_E_PARAM, "Unable to compile:%s \n", match_n);
        }
        *compare_handle_p = (void *) regex;
#else
        *compare_handle_p = (void *) match_n;
        SHR_EXIT();
#endif
    }

exit:
    SHR_FUNC_EXIT;
}

int
diag_sand_compare(
    void *compare_handle,
    char *string)
{
    if (compare_handle == NULL) /* No comparison requested */
        return TRUE;
#if !defined(__KERNEL__) && !defined(VXWORKS) && !defined(NO_REGEX)
    if (!regexec((regex_t *) compare_handle, string, 0, NULL, 0))
    {
        return TRUE;
    }
#else
    if (sal_strcasestr(compare_handle, string) != NULL)
    {
        return TRUE;
    }
#endif
    else
        return FALSE;
}

void
diag_sand_compare_close(
    void *compare_handle)
{
    if (compare_handle == NULL) /* No comparison was requested */
        return;
#if !defined(__KERNEL__) && !defined(VXWORKS) && !defined(NO_REGEX)
    regfree(compare_handle);
    sal_free(compare_handle);
#endif
    return;
}
