/** \file utilex_str.h
 * Purpose:    Misc. routines for string/buffer handling
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef UTILEX_STR_H_INCLUDED
#define UTILEX_STR_H_INCLUDED

#include <sal/core/alloc.h>
#include <sal/types.h>
#include <shared/bsl.h>
#include <shared/error.h>

/* Misc defines */
#define  RHFILE_MAX_SIZE      128
#define  RHNAME_MAX_SIZE      64
#define  RHKEYWORD_MAX_SIZE   12

/**
 * \brief Check the pointer and the content. If it is not NULL and the not empty return true
 * \par DIRECT INPUT
 *   \param [in] name_macro pointer to the string to be checked
 */
#define ISEMPTY(name_macro)     ((name_macro == NULL) || (name_macro[0] == 0))

/**
 * \brief Make string empty one
 * \par DIRECT INPUT
 *   \param [in] name_macro pointer to the string to be empty
 */
#define SET_EMPTY(name_macro)   (name_macro[0] = 0)

/**
 * \brief Return uint32 value hidden in the string
 * \par DIRECT INPUT
 *   \param [in] value_string pointer to the uint32 value
 */
#define VALUE(value_string)     (*((uint32 *)value_string))

/**
 * \brief Fill certain number of characters in string with character
 * \par DIRECT INPUT
 *   \param [in,out] string pointer to the start of string to be filled
 *   \param [in] num number of bytes to be filled
 *   \param [in] ch character that will be put into the string
 */
void utilex_str_fill(
    char *string,
    int num,
    char ch);

/**
 * \brief Split input string into tokens - multiple strings separated by certain character
 * \par DIRECT INPUT
 * \param [in] string input string for splitting
 * \param [in] delim  delimiter character for splitting
 * \param [in] maxtokens maximum number of expected token
 * \param [in,out] realtokens  pointer to the variable that discovered count of tokens will be filled by
 * \par DIRECT OUTPUT
 *   \retval pointer to string array containing list of tokens
 * \par INDIRECT OUTPUT
 *   *realtokens - number of tokens found in the string
 * \remark
 *   String array should be freed once not needed through utilex_str_split_free
 */
char **utilex_str_split(
    char *string,
    char *delim,
    uint32 maxtokens,
    uint32 * realtokens);

/**
 * \brief Free array of tokens allocated by utilex_str_split
 * \par DIRECT INPUT
 *   \param [in] tokens pointer to token array
 *   \param [in] token_num delim  delimiter character for splitting
 */
void utilex_str_split_free(
    char **tokens,
    uint32 token_num);

/**
 * \brief Swap bytes inside uint32
 * \par DIRECT INPUT
 *   \param [in] buffer pointer to buffer
 *   \param [in] uint32_num number of words to be swapped
 * \par INDIRECT OUTPUT
 *   *buffer buffer content will present bytes in reversed order
 */
void utilex_str_swap_long(
    uint8 * buffer,
    int uint32_num);

/**
 * \brief Replace white space by underscore in source and put it in destination
 * \par DIRECT INPUT
 *   \param [in] dest buffer where result will be placed
 *   \param [in] source pointer to string with white spaces
 * \par INDIRECT OUTPUT
 *   *dest  buffer where result of replace operation will be placed
 */
void utilex_str_replace_whitespace(
    char *dest,
    char *source);

/**
 * \brief Replace end of line by white space
 * \par DIRECT INPUT
 *   \param [in,out] str string in which EOL will be replaced
 *   \param [in] replacement The character to replace EOL with
 * \par INDIRECT OUTPUT
 *   *str  buffer where result of replace operation will be placed
 */
void utilex_str_replace_eol(
    char *str,
    char replacement);

/**
 * \brief Replace multiple white spaces and/or tabs to 1 white space and remove them from the end
 * \par DIRECT INPUT
 *   \param [in,out] str string in which replacement will be take place
 * \par INDIRECT OUTPUT
 *   *str  buffer where result of replace operation will be placed
 */
void utilex_str_shrink(
    char *str);

int utilex_str_get_shift(char *string, int token_size);

/**
 * \brief Convert string to unsigned int
 * \par DIRECT INPUT
 *   \param [in] str pointer to the string to be converted
 *   \param [in] value_p pointer to the value
 * \par INDIRECT OUTPUT
 *   *value_p value obtained from the string
 */
uint32 utilex_str_stoul(
    char *str,
    uint32 * value_p);

void *utilex_alloc(
    unsigned int size);

void utilex_free(
    void *mem);

int utilex_pow(
    int n,
    int k);

#endif /* UTILEX_STR_H_INCLUDED */
