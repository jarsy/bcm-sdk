
/** \file diag_dnx_dbal_internal.h
 * Purpose: Extern declarations for command functions and
 *          their associated usage strings.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef DIAG_DNX_DBAL_INTERNAL_H_INCLUDED
#  define DIAG_DNX_DBAL_INTERNAL_H_INCLUDED

/*************
 * INCLUDES  *
 *************/
#  include <soc/dnx/dbal/dbal.h>
#  include <shared/shrextend/shrextend_debug.h>
#  include <src/soc/dnx/dbal/dbal_internal.h>
#  include <appl/diag/sand/diag_sand_framework.h>

/*************
 *  DEFINES  *
 *************/
#  define CMD_MAX_STRING_LENGTH   DBAL_MAX_STRING_LENGTH

#  define CMD_MAX_NOF_INPUTS      5

/*************
 *  MACROES  *
 *************/
/**
* \brief
* print to the consule:
*   string name
*   ===========
* if name not exists use "\0" or ""
*  \par DIRECT INPUT:
*    \param [in] header - string\n
*    \param [in] name - string \n
*****************************************************/
#  define DIAG_DBAL_HEADER_DUMP(header, name)                         \
{                                                                     \
  int iter = 0, iter_num = sal_strlen(header)+sal_strlen(name);       \
  LOG_CLI((BSL_META("\n%s %s\n"), header, name));                     \
  for(iter=0;iter<iter_num+1;iter++)                                  \
  {                                                                   \
    LOG_CLI((BSL_META("=")));                                         \
  }                                                                   \
  LOG_CLI((BSL_META("\n\n")));                                        \
}

 /**
 * \brief
 * print to the consule:
 *   string name
 *   ~~~~~~~~~~~
 * if name not exists use "\0" or ""
 *  \par DIRECT INPUT:
 *    \param [in] header - string\n
 *    \param [in] name - string \n
 ****************************************************/

#  define DIAG_DBAL_SUBHEADER_DUMP(header, name)                        \
{                                                                     \
  int iter = 0, iter_num = sal_strlen(header)+sal_strlen(name);       \
  LOG_CLI((BSL_META("%s %s\n"), header, name));                       \
  for(iter=0;iter<iter_num+1;iter++)                                  \
  {                                                                   \
    LOG_CLI((BSL_META("~")));                                         \
  }                                                                   \
  LOG_CLI((BSL_META("\n")));                                          \
}

/**
 *  setting the maximum value for N bits field
 *  UINT/INT support
 */
#define DIAG_DBAL_MAX_FIELD_VAL(field_arr,field_len,field_type)       \
{                                                                     \
  int iter_num = (field_len-1)/32;                                    \
  int residue = field_len%32;                                         \
  int iter = 0;                                                       \
  for(iter=0;iter<iter_num;iter++)                                    \
  {                                                                   \
    field_arr[iter] = 0xFFFFFFFF;                                     \
  }                                                                   \
  if(residue == 0)                                                    \
  {                                                                   \
    field_arr[iter_num] = (field_type == DBAL_FIELD_TYPE_INT32)?      \
                          0x7FFFFFFF : 0xFFFFFFFF;                    \
  }                                                                   \
  else                                                                \
  {                                                                   \
    field_arr[iter_num] = (field_type == DBAL_FIELD_TYPE_INT32)?      \
                          ((1<<(residue-1))-1) : ((1<<residue)-1);    \
  }                                                                   \
}

/**
 *  setting the minimum value for N bits field
 *  UINT/INT support
 */
#define DIAG_DBAL_MIN_FIELD_VAL(field_arr,field_len,field_type)       \
{                                                                     \
  int iter_num = (field_len-1)/32;                                    \
  int residue = field_len%32;                                         \
  int iter = 0;                                                       \
  for(iter=0;iter<iter_num;iter++)                                    \
  {                                                                   \
    field_arr[iter] = 0x0;                                            \
  }                                                                   \
  if(residue == 0)                                                    \
  {                                                                   \
    field_arr[iter_num] = (field_type == DBAL_FIELD_TYPE_INT32)?      \
                          0x10000000 : 0x0;                           \
  }                                                                   \
  else                                                                \
  {                                                                   \
    field_arr[iter_num] = (field_type == DBAL_FIELD_TYPE_INT32)?      \
                          (1<<(residue-1)) : 0x0;                     \
  }                                                                   \
}

/**
 *  setting random value for N bits field
 *  random cannot set minimum or maximum values (except of field
 *  size is 1bit), in that case - for result field the maximum
 *  should be returned
 */
#define DIAG_DBAL_RANDOM_FIELD_VAL(field_arr,field_len,is_key)        \
{                                                                     \
  int iter_num = (field_len+31)/32;                                   \
  int residue = field_len%32;                                         \
  int iter = 0;                                                       \
  uint32 mask = 0xFFFFFFFF;                                           \
  for(iter=0;iter<iter_num;iter++)                                    \
  {                                                                   \
    field_arr[iter] = (sal_rand() + ((sal_rand()&0x4000)<<1))         \
                    + ((sal_rand() + ((sal_rand()&0x4000)<<1))<<16);  \
  }                                                                   \
  iter = iter_num - 1;                                                \
  if(residue != 0)                                                    \
  {                                                                   \
    mask = ((1<<residue)-1);                                          \
    field_arr[iter] &= mask;                                          \
  }                                                                   \
  if(field_len == 1)                                                  \
  {                                                                   \
    if(is_key == 0)                                                   \
    {                                                                 \
      field_arr[0] = 1;                                               \
    }                                                                 \
  }                                                                   \
  else                                                                \
  {                                                                   \
    if(field_arr[iter] == mask)                                       \
    {                                                                 \
      field_arr[iter] &= 0xFFFFFFFE;                                  \
    }                                                                 \
    if(field_arr[iter] == 0)                                          \
    {                                                                 \
      field_arr[iter] = 1;                                            \
    }                                                                 \
  }                                                                   \
}

/*************
 * TYPEDEFS  *
 *************/
typedef enum
{
    MIN_FIELD_VAL,
    RANDOM_FIELD_VAL,
    MAX_FIELD_VAL
} fields_values_types_e;

typedef enum
{
    LTT_RUN_ITERATOR = SAL_BIT(0),
    LTT_TABLE_CLEAR_END_OF_TEST = SAL_BIT(1),
    LTT_TABLE_CLEAR_STT_OF_TEST = SAL_BIT(2),
    LTT_IS_REGRESSION = SAL_BIT(3),
    LTT_FULL_ITERATOR_TABLE_CLEAR_TEST = SAL_BIT(4),
    LTT_DO_NOT_REMOVE_ENTRIES = SAL_BIT(5),
    LTT_RUN__TEST_WITH_3_ENTRIES = SAL_BIT(6),
    LTT_NOF_FLAGS = SAL_BIT(7)
} dbal_ltt_flags_e;

/*************
 * FUNCTIONS *
 *************/
/**
* \brief
* Get a random/maximum/minimum value for field.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] field_id
*    \param [in] type_of_value \n random/max/min
*    \param [in] field_len \n length in bits
*    \param [in] is_key \n key or reault indication
*    \param [in] field_value \n ptr to return value
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      result cmd
*****************************************************/
shr_error_e dbal_fields_get_value_for_field(
    int unit,
    dbal_fields_e field_id,
    fields_values_types_e type_of_value,
    uint32 field_len,
    uint8 is_key,
    uint32 * field_value);
/**
* \brief
* dump to consule iterator information.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] iterator_info - the iterator information\n
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      result cmd
*****************************************************/
shr_error_e dbal_table_iterator_info_dump(
    int unit,
    dbal_iterator_info_t * iterator_info);

/**
* \brief
* dump to consule logical table information.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] table_id - table ID to print\n
*    \param [in] print_mode  - 0 dump only interface part, 1 dump also logical to phy mapping\n
*    \param [in] sand_control  - sand control command
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      result cmd
*****************************************************/
shr_error_e diag_dbal_logical_table_dump(
    int unit,
    dbal_tables_e table_id,
    int print_mode,             /* 0=partial, 1=full */
    sh_sand_control_t * sand_control);
/**
* \brief
* dump to consule field value according to type.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] field_id - requested field ID \n
*    \param [in] table_id - related table \n
*    \param [in] field_val - field value \n
*    \param [in] field_mask - field maske \n
*    \param [in] is_key - indication for key of result field\n
*    \param [in] result_type_idx - in case of result field, result type index\n
*    \param [in] in_offset - used only if the value is not aligned to start of the buffer\n
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      result cmd
*****************************************************/
shr_error_e diag_dbal_field_dump(
    int unit,
    dbal_fields_e field_id,
    dbal_tables_e table_id,
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES],
    uint32 field_mask[DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES],
    uint8 is_key,
    int result_type_idx,
    int in_offset /* offset in buffer */ );

/**
* \brief
* creates a string that represents the lables.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] labels - labels array to use\n
*    \param [in] str  - a pointer to the requested string created\n
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      result cmd
* \par INDIRECT OUTPUT
*       str  - filled the str \n
*****************************************************/
void diag_dbal_lables_string_build(
    int unit,
    CONST dbal_labels_e labels[DBAL_MAX_NOF_ENTITY_LABEL_TYPES],
    char *str);

/**
* \brief
* creates a string that represents the encode information.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] encode_info - encoding info to use\n
*    \param [in] str  - pointer to the requested string created\n
*  \par DIRECT OUTPUT:
*    shr_error_e - \n
*      result cmd
* \par INDIRECT OUTPUT
*       str  - filled the str \n
*****************************************************/
void diag_dbal_offset_encode_info_to_str(
    int unit,
    CONST dbal_offset_encode_info_t * encode_info,
    char *str);

/**
* \brief
* load a spesific dbal xml files and cmpare the parsed results to an hard coded expected results.
*
*  \par DIRECT INPUT:
*    \param [in] unit
*  \par DIRECT OUTPUT:
*    cmd_result_t - \n
*      result cmd
*****************************************************/
cmd_result_t diag_dbal_test_xml_parsing(
    int unit);

/**
* \brief 
* a basic logical tests for a given logical table
* the test insert entries to the table, retrieve them and compare the retrieved entry to the expected one
* tests is: 
* adding MAX and MIN entries and getting the entries 
* adding 32 random values entries and getting each entry 
* running traverse to validate that tere is only 32 entries. 
* deleting the entries at the end of the regression. 
*
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] table_id - \n
*       table_id
*    \param [in] flags - \n
*       test flags 
*  \par DIRECT OUTPUT:
*    cmd_result_t - \n
*      result cmd
*****************************************************/
cmd_result_t diag_dbal_test_logical_table(
    int unit,
    dbal_tables_e table_id,
    uint32 flags);

/**
* \brief 
* count entries in a table using dbal iterator 
*  
*  \par DIRECT INPUT:
*    \param [in] unit
*    \param [in] table_id - \n
*       table_id
*    \param [in] counter - \n
*       ptr to be updated with counter result 
*  \par DIRECT OUTPUT:
*    cmd_result_t - \n
*      result cmd
*  \par INDIRECT OUTPUT:
*    counter - \n
*      See in direct input section
*****************************************************/
shr_error_e diag_dbal_iterator_count_entries(
    int unit,
    dbal_tables_e table_id,
    int *counter);

#endif /* DIAG_DNX_DBAL_INTERNAL_H_INCLUDED */
