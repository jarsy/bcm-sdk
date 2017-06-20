/* $Id: utilex_str.c,v 1.00 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/*
 * \file:    utilex_str.c
 * Purpose:    Misc. routines used by export/import/show facilities
 */

#include <sal/core/libc.h>
#include <sal/core/alloc.h>
#include <sal/appl/sal.h>
#include <sal/types.h>
#include <shared/util.h>
#include <shared/error.h>
#include <shared/utilex/utilex_str.h>

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDDNX_UTILEXDNX

void
utilex_str_fill(
    char *str,
    int num,
    char ch)
{
    int i;
    for (i = 0; i < num; i++)
        str[i] = ch;

    str[num] = 0;
}

char **
utilex_str_split(
    char *string,
    char *delim,
    uint32 maxtokens,
    uint32 * realtokens)
{
    char **tokens = NULL;
    int i = 0;
    char *next;

    if (ISEMPTY(string))
        goto exit;

    tokens = sal_alloc(sizeof(char *) * maxtokens, "tokens");
    if (tokens == NULL)
        goto exit;

    memset(tokens, 0, sizeof(char *) * maxtokens);

    for (i = 0; i < maxtokens; i++)
        tokens[i] = NULL;

    i = 0;
    while (((next = strstr(string, delim)) != NULL) && (i < (maxtokens - 1)))
    {
        tokens[i] = sal_alloc(sal_strlen(string) + 1, "token");
        if (tokens[i] == NULL)
            goto exit;
        sal_memset(tokens[i], 0, sal_strlen(string) + 1);
        /*
         * Copy strictly number of characters until next 
         */
        sal_memcpy(tokens[i++], string, next - string);
        /*
         * move string to after the delimiter 
         */
        string = next + sal_strlen(delim);
    }
    /*
     * Once end of line or maxtokens achieved - string will keep the last part 
     */
    tokens[i] = sal_alloc(sal_strlen(string) + 1, "token");
    if (tokens[i] == NULL)
        goto exit;
    sal_memset(tokens[i], 0, sal_strlen(string) + 1);
    strcpy(tokens[i++], string);

exit:
    *realtokens = i;
    return tokens;
}

void
utilex_str_split_free(
    char **tokens,
    uint32 token_num)
{
    int i = 0;

    if (tokens == NULL)
    {
        return;
    }

    for (i = 0; i < token_num; i++)
    {
        if (tokens[i] != NULL)
            sal_free(tokens[i]);
    }

    sal_free(tokens);
    return;
}

void
utilex_str_swap_long(
    uint8 * buffer,
    int uint32_num)
{
    uint32 tmp_value;
    uint8 *tmp_array = (uint8 *) (&tmp_value);
    int i;

    for (i = 0; i < uint32_num; i++)
    {
        tmp_value = *((uint32 *) buffer);
        buffer[0] = tmp_array[3];
        buffer[1] = tmp_array[2];
        buffer[2] = tmp_array[1];
        buffer[3] = tmp_array[0];
        buffer += 4;
    }
    return;
}

void
utilex_str_replace_whitespace(
    char *dest,
    char *source)
{
    int i;
    for (i = 0; i < sal_strlen(source); i++)
    {
        if (source[i] == ' ')
            dest[i] = '_';
        else
            dest[i] = source[i];
    }
    dest[i] = 0;
    return;
}

void
utilex_str_replace_eol(
    char *str,
    char replacement)
{
    int i;
    for (i = 0; i < sal_strlen(str); i++)
    {
        if (str[i] == '\n')
            str[i] = replacement;
    }
    return;
}

int utilex_str_get_shift(char *string, int token_size)
{
    int i_char;
    int actual_size;

    actual_size = (token_size < strlen(string)) ? token_size : strlen(string);
    /*
     * First check if we have new line inside
     */
    for(i_char = 0; i_char < actual_size; i_char++)
    {
        if(string[i_char] == '\n')
        {   /* return the offset */
            return i_char + 1;
        }
    }
    /*
     * If what is left from string is less then token, just return the leftover
     */
    if(actual_size < token_size)
    {
        return actual_size;
    }
    /*
     * No eol, look for white space
     */
    for(i_char = actual_size - 1; i_char > 0; i_char--)
    {
        if(string[i_char] == ' ')
        {
            return i_char + 1;
        }
    }
    /*
     * We reached this place means that there is no white space inside token_size
     */
    return actual_size;
}

void
utilex_str_shrink(
    char *str)
{
    int i_char;
    int shift = 0;
    char last_char = 0;
    int str_size = sal_strlen(str);
    /*
     * Step 1:shrink all white spaces to one
     */
    for (i_char = 0; i_char < str_size; i_char++)
    {
        if ((str[i_char] == ' ') && (last_char == ' '))
            shift++;
        else
        {
            if (shift != 0)
                str[i_char - shift] = str[i_char];
        }
        last_char = str[i_char];
    }
    /*
     * Step 2:zero all the dirty characters left at the end of string
     */
    for (i_char = str_size - 1; i_char > str_size - 1 - shift; i_char--)
    {
        str[i_char] = 0;
    }

    /*
     * Step 3:zero all trailing white spaces, dots and tabs
     */
    for (i_char = sal_strlen(str) - 1; i_char > 0; i_char--)
    {
        if ((str[i_char] == ' ') || (str[i_char] == '.') || (str[i_char] == '\t') || (str[i_char] == '\n'))
            str[i_char] = 0;
        else
            break;
    }

    return;
}

uint32
utilex_str_stoul(
    char *str,
    uint32 * value_p)
{
    int i;
    char ch;

    for (i = 0; i < strlen(str); i++)
    {
        ch = str[i];
        if (((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F')))
            continue;
        else
            return _SHR_E_PARAM;
    }

    *value_p = _shr_ctoi(str);

    return _SHR_E_NONE;
}

void *
utilex_alloc(
    unsigned int size)
{
    void *mem;
    if ((mem = sal_alloc(size, "utilex")) != NULL)
        sal_memset(mem, 0, size);

    return mem;
}

void
utilex_free(
    void *mem)
{
    sal_free(mem);
}

int
utilex_pow(
    int n,
    int k)
{
    int i, result = 1;
    for (i = 0; i < k; i++)
        result *= n;

    return result;
}
