/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include "pemladrv_physical_access.h"
#include "pemladrv_logical_access.h"
#include "pemladrv_meminfo_init.h"
#include "pemladrv_meminfo_access.h"
#include "pemladrv_cfg_api_defines.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#ifdef _MSC_VER
#if (_MSC_VER >= 1400) 
#pragma warning(push)
#pragma warning(disable: 4996) /* To avoid Microsoft compiler warning (4996 for _CRT_SECURE_NO_WARNINGS equivalent)*/
#endif
#endif

extern ApiInfo api_info;


/* The following function convert string representing stream of bits
* in format n'hxxxx (e.g. 288'h000000000000000000000000000000000000000000000000000000000000000000000000), 
 * and convert it to an unsigned int array.
* It returns array of unsigned int and the length of the bits stream (in bits)
*/
unsigned int* hexstr2uints(char *str, unsigned int *len) { 
  char *ptr, *start_value;
  int array_size = 0;
  unsigned int* arr;
  unsigned int* curr_entry_val;
  int nibble_ndx;
  unsigned int curr_nibble;

  /* Get the stream length*/
  *len = strtoul(str, &ptr, 10);

  /* skip the "'h" characters*/
  ptr = ptr + 2;
  start_value = ptr;

  /* Alloc enough room array for the converted string*/
  array_size = ((*len % 32) == 0) ? *len / 32 : *len / 32 + 1;
  arr = (unsigned int*) malloc(array_size * 4);
  memset(arr, 0, array_size * 4);

  /* goto the last nibble*/
  
  for (nibble_ndx = 0, ptr = str + strlen(str) - 1; ptr >= start_value; --ptr, ++nibble_ndx)
  {
    curr_entry_val = arr + (nibble_ndx / 8);
    curr_nibble = (*ptr <= 'F' && *ptr >= 'A') ? *ptr - 'A' + 10
                                                            : (*ptr <= 'f' && *ptr >= 'a') ? *ptr -'a' + 10
                                                            : *ptr - '0';
    *curr_entry_val |= (curr_nibble<<((nibble_ndx % 8) *4));
  }

  return arr;
}

/* The following function convert string representing memory address
* in format 40'hxxxx ('h7110003), where the 8 MSb are block-id and the 32 LSb are address in this block.
* It return block-id, and 32 bits memory address.
*/
unsigned int hexstr2addr(char *str, unsigned int *block_id) {
  unsigned int addr;
  unsigned int len;
  unsigned int* addr_arr;
  /* Check that length is 40*/
  addr_arr = hexstr2uints(str, &len);
  assert(len == 40);
  /* First read the address skip the prefix (4) + block id(2)*/
  addr = addr_arr[0];
  *block_id = addr_arr[1];
  free(addr_arr);
  return addr;
}

int parse_meminfo_definition_file(const int restore_after_reset, const char *file_name) {
  FILE *fp;
  int ret_val = 0;
  char line[512];       /* input line */
  int line_no = 1;
  int in_comment = 0;  
  if ((fp = fopen(file_name, "r")) == NULL) {
    /*printf("Can't open file '%s'. Exit.\n", file_name);*/
    return UINT_MAX;
  }

   ucode_init(); 
   init_api_info();

  /* Read one line per iteration and parse it */
  while((fgets(line, sizeof(line), fp) != NULL) && (ret_val >= 0)) { 

    /* Skip empty lines */
    if (strlen(line) == 0) {line_no = line_no + 1; continue;} 

    /* Handle comments */
    if (strstr(line, END_COMMENT) != 0)    { in_comment = 0; ++line_no; continue;} /* Note that this match also one-line-comment */
    if (strstr(line, START_COMMENT) != 0)  { in_comment = 1; ++line_no; continue;}  
    if (in_comment) { ++line_no; continue;}

    

    /* UCODE INIT*/
    /* Coverity: We shouldn't fix such strange defects */
    /* coverity[tainted_data] */
    if (strncmp(line, KEYWORD_UCODE_MEM_LINE_START_INFO, KEYWORD_UCODE_MEM_LINE_START_INFO_SIZE) == 0)                      { mem_line_insert(line)                     ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_UCODE_REG_LINE_START_INFO, KEYWORD_UCODE_REG_LINE_START_INFO_SIZE) == 0)                      { reg_line_insert(line)                     ;  ++line_no; continue; }
    /* LOGICAL DB MAPPING INFO*/
    if (strncmp(line, KEYWORD_DB_INFO, KEYWORD_DB_INFO_SIZE) == 0)                                                          { db_info_insert(line)                      ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_KEY_INFO, KEYWORD_KEY_INFO_SIZE) == 0)                                                        { db_field_info_insert(line)                ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_RESULT_INFO, KEYWORD_RESULT_INFO_SIZE) == 0)                                                  { db_field_info_insert(line)                ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_DIRECT_INFO, KEYWORD_DB_DIRECT_INFO_SIZE) == 0)                                            { direct_result_chunk_insert(line)          ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_TCAM_KEY_INFO, KEYWORD_DB_TCAM_KEY_INFO_SIZE) == 0)                                        { tcam_key_chunk_insert(line)               ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_TCAM_RESULT_INFO, KEYWORD_DB_TCAM_RESULT_INFO_SIZE) == 0)                                  { tcam_result_chunk_insert(line)            ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_EXACT_MATCH_KEY_INFO, KEYWORD_DB_EXACT_MATCH_KEY_INFO_SIZE) == 0)                          { em_key_chunk_insert(line)                 ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_EXACT_MATCH_RESULT_INFO, KEYWORD_DB_EXACT_MATCH_RESULT_INFO_SIZE) == 0)                    { em_result_chunk_insert(line)              ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_LONGEST_PERFIX_MATCH_KEY_INFO, KEYWORD_DB_LONGEST_PERFIX_MATCH_KEY_INFO_SIZE) == 0)        { lpm_key_chunk_insert(line)                ;  ++line_no; continue; }  
    if (strncmp(line, KEYWORD_DB_LONGEST_PERFIX_MATCH_RESULT_INFO, KEYWORD_DB_LONGEST_PERFIX_MATCH_RESULT_INFO_SIZE) == 0)  { lpm_result_chunk_insert(line)             ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_VIRTUAL_REGISTER_MAPPING, KEYWORD_REGISTER_INFO_SIZE) == 0)                                   { register_insert(line)                     ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_REG_AND_DBS_NUM_INFO, KEYWORD_REG_AND_DBS_NUM_INFO_SIZE) == 0)                                { init_all_db_arr_by_size(line)             ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO, KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO_SIZE) == 0)                    { init_logical_db_chunk_mapper(line)        ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_VIRTUAL_REGISTER_NOF_FIELDS, KEYWORD_VIRTUAL_REGISTER_NOF_FIELDS_SIZE) == 0)                  { init_reg_field_info(line)                 ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS, KEYWORD_VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS_SIZE) == 0)  { init_reg_field_mapper(line)               ;  ++line_no; continue; }
    if (strncmp(line, KEYWORD_PEM_APPLET_REG, KEYWORD_PEM_APPLET_REG_SIZE) == 0)                                            { init_pem_applet_reg(line)                 ;  ++line_no; continue; }    
    if (strncmp(line, KEYWORD_PEM_APPLET_MEM, KEYWORD_PEM_APPLET_MEM_SIZE) == 0)                                            { init_pem_applet_mem(line)                 ;  ++line_no; continue; }    
    if (strncmp(line, KEYWORD_GENERAL_INFO_FOR_APPLET, KEYWORD_GENERAL_INFO_FOR_APPLET_SIZE) == 0)                          { init_meminfo_array_for_applets(line)      ;  ++line_no; continue; }    
    if (strncmp(line, KEYWORD_MEMINFO_FOR_APPLET, KEYWORD_MEMINFO_FOR_APPLET_SIZE) == 0)                                    { insert_meminfo_to_array_for_applets(line) ;  ++line_no; continue; }    

    /* DBUG INFO*/

    ++line_no;

  } /* End of while loop over all lines in file*/
  fclose(fp);
  
  if(restore_after_reset) /*In  this case, physical is Up to date and lpm/em caches need to be updated*/
    read_physical_and_update_cache();
  else 
    zero_last_chunk_of_cam_based_dbs(); /*In order to 'clear' any previous data, that might be still written in the DBs*/

  return ret_val;

} /* end of parse_meminfo_definition_file() */


/*Initialize api_info struct*/
void init_api_info() {
  if((api_info.db_direct_container.nof_direct_dbs) || 
    (api_info.db_em_container.nof_em_dbs)          || 
    (api_info.db_lpm_container.nof_lpm_dbs)        ||
    (api_info.db_tcam_container.nof_tcam_dbs)      ||
    (api_info.reg_container.nof_registers)         )

    free_api_info();
}

/*Free all prior alocated memory in api_info*/
void free_api_info() {
  free_db_direct_container();
  free_db_tcam_container();
  free_db_lpm_container();
  free_db_em_container();
  free_reg_container();
}

void free_db_direct_container() {
  unsigned int i;
  for (i = 0; i < api_info.db_direct_container.nof_direct_dbs; ++i) {
    if (NULL == api_info.db_direct_container.db_direct_info_arr[i].result_chunk_mapper_matrix_arr)    /*In case db was not mapped*/
      continue;
    free_logical_direct_info(&api_info.db_direct_container.db_direct_info_arr[i]);
  }
  free(api_info.db_direct_container.db_direct_info_arr);
  api_info.db_direct_container.db_direct_info_arr = NULL;
  api_info.db_direct_container.nof_direct_dbs     = 0;
}

void free_db_tcam_container() {
  unsigned int i;
  for (i = 0; i < api_info.db_tcam_container.nof_tcam_dbs; ++i) {
    if (NULL == api_info.db_tcam_container.db_tcam_info_arr[i].result_chunk_mapper_matrix_arr)     /*In case db was not mapped*/
      continue;
    free_logical_tcam_info(&api_info.db_tcam_container.db_tcam_info_arr[i]);
  }
  free(api_info.db_tcam_container.db_tcam_info_arr);
  api_info.db_tcam_container.db_tcam_info_arr = NULL;
  api_info.db_tcam_container.nof_tcam_dbs     = 0;
}

void free_db_lpm_container() {
  unsigned int i;
  for (i = 0; i < api_info.db_lpm_container.nof_lpm_dbs; ++i) {
    if (NULL == api_info.db_lpm_container.db_lpm_info_arr[i].logical_lpm_info.result_chunk_mapper_matrix_arr)      /*In case db was not mapped*/
      continue;
    free_logical_lpm_info(&api_info.db_lpm_container.db_lpm_info_arr[i]);
  }
  free(api_info.db_lpm_container.db_lpm_info_arr);
  api_info.db_lpm_container.db_lpm_info_arr = NULL;
  api_info.db_lpm_container.nof_lpm_dbs     = 0;
}

void free_db_em_container() {
  unsigned int i;
  for (i = 0; i < api_info.db_em_container.nof_em_dbs; ++i) {
    if (NULL == api_info.db_em_container.db_em_info_arr[i].logical_em_info.result_chunk_mapper_matrix_arr)      /*In case db was not mapped*/
      continue;
    free_logical_em_info(&api_info.db_em_container.db_em_info_arr[i]);
  }
  free(api_info.db_em_container.db_em_info_arr);
  api_info.db_em_container.db_em_info_arr = NULL;
  api_info.db_em_container.nof_em_dbs     = 0;
}

void free_reg_container() {
  unsigned int i;
  for (i = 0; i < api_info.reg_container.nof_registers; ++i) {
    if (!api_info.reg_container.reg_info_arr[i].is_mapped)      /*In case register was not mapped*/
      continue;
    free_logical_reg_info(&api_info.reg_container.reg_info_arr[i]);
  }

  free(api_info.reg_container.reg_info_arr);
  api_info.reg_container.reg_info_arr  = NULL;
  api_info.reg_container.nof_registers = 0;
}

void free_logical_direct_info(LogicalDirectInfo* logical_direct_info) {
  free_chunk_mapper_matrix(logical_direct_info->nof_chunk_matrices, logical_direct_info->result_chunk_mapper_matrix_arr);
  free(logical_direct_info->field_bit_range_arr);
  logical_direct_info->field_bit_range_arr = NULL;
}

void free_logical_tcam_info(LogicalTcamInfo* logical_tcam_info) {
  free_chunk_mapper_matrix(logical_tcam_info->nof_chunk_matrices, logical_tcam_info->key_chunk_mapper_matrix_arr);
  free_chunk_mapper_matrix(logical_tcam_info->nof_chunk_matrices, logical_tcam_info->result_chunk_mapper_matrix_arr);
  free(logical_tcam_info->key_field_bit_range_arr);
  free(logical_tcam_info->result_field_bit_range_arr);
  logical_tcam_info->key_field_bit_range_arr    = NULL;
  logical_tcam_info->result_field_bit_range_arr = NULL;
}

void free_logical_lpm_info(LogicalLpmInfo* logical_lpm_info) {
  free_logical_tcam_info(&logical_lpm_info->logical_lpm_info);
  free_lpm_cache(&logical_lpm_info->lpm_cache);
}

void free_logical_em_info(LogicalEmInfo* logical_em_info) {
  free_logical_tcam_info(&logical_em_info->logical_em_info);
  free_em_cache(&logical_em_info->em_cache);
}

void free_logical_reg_info(LogicalRegInfo* logical_reg_info) {
  free(logical_reg_info->reg_field_info_arr->reg_field_mapping_arr);
  logical_reg_info->reg_field_info_arr->reg_field_mapping_arr = NULL;

  free(logical_reg_info->reg_field_info_arr);
  logical_reg_info->reg_field_info_arr = NULL;

  free(logical_reg_info->reg_field_bit_range_arr);
  logical_reg_info->reg_field_bit_range_arr = NULL;
}
void free_chunk_mapper_matrix(const int nof_implamentations, LogicalDbChunkMapperMatrix* chunk_mapper_matrix) {
  int implamentation_index;
  unsigned int row_index, col_index; 

  for (implamentation_index = 0; implamentation_index < nof_implamentations; ++implamentation_index) {
    for (row_index = 0; row_index < chunk_mapper_matrix->nof_rows_in_chunk_matrix; ++row_index) {
      for (col_index = 0; col_index < chunk_mapper_matrix->nof_cols_in_chunk_matrix; ++col_index) {
        free(chunk_mapper_matrix[implamentation_index].db_chunk_mapper[row_index][col_index]);
      }
      free(chunk_mapper_matrix[implamentation_index].db_chunk_mapper[row_index]);
    }
    free(chunk_mapper_matrix[implamentation_index].db_chunk_mapper);
  }
  free(chunk_mapper_matrix);
}

void free_lpm_cache(LpmDbCache* lpm_cache) {
  free(lpm_cache->lpm_key_entry_arr);
  free(lpm_cache->lpm_result_entry_arr);
  free(lpm_cache->lpm_key_entry_prefix_length_arr);
  free(lpm_cache->physical_entry_occupation);
}

void free_em_cache(EmDbCache* em_cache) {
  free(em_cache->em_key_entry_arr);
  free(em_cache->em_result_entry_arr);
  free(em_cache->physical_entry_occupation);
}


/* Insert MEMORY LINE*/
void mem_line_insert(const char* line) {
  char pem_mem_address[40];                              /* this contain hex string address in format 40'hxxxx*/
  char str_value[MAX_MEM_DATA_LENGTH];                   /* this contain hex string value in format n'hxxxx*/
  char key_word[MAX_NAME_LENGTH], pem_mem_name[MAX_NAME_LENGTH] ;
  unsigned int phy_mem_idx, block_id;
  int unit = 0;
  unsigned int value_length, pem_mem_address_val;
  unsigned int* data_value;
  if (strncmp(line, KEYWORD_UCODE_MEM_LINE_START_INFO, KEYWORD_UCODE_MEM_LINE_START_INFO_SIZE) == 0) {
 
    if (sscanf( line, "%s %s %s %u %s", key_word, pem_mem_name, pem_mem_address,
      &phy_mem_idx, str_value) != PEM_NOF_MEM_LINE_START_TOKEN) {
        /*printf("Bad ucode line format. Skip and continue with next line.\n");*/
    } else {
      pem_mem_address_val = hexstr2addr(pem_mem_address, &block_id);
      data_value = hexstr2uints(str_value, &value_length);
      ucode_add(unit, block_id, pem_mem_address_val, phy_mem_idx, data_value, value_length, 1);
    }
  }
}
 
/*Insert REG LINE*/
void reg_line_insert(const char* line) {
  char pem_reg_address[40];                              /* this contain hex string address in format 40'hxxxx*/
  char str_value[MAX_MEM_DATA_LENGTH];                   /* this contain hex string value in format n'hxxxx*/
  char key_word[MAX_NAME_LENGTH], pem_reg_name[MAX_NAME_LENGTH] ;
  int unit = 0;
  unsigned int block_id, value_length, pem_mem_address_val;
  unsigned int* data_value;
  if (sscanf( line, "%s %s %s %s", key_word, pem_reg_name, pem_reg_address, str_value) != PEM_NOF_REG_LINE_START_TOKEN) {
    /*printf("Bad ucode line format. Skip and continue with next line\n.");*/
  } else {
    pem_mem_address_val = hexstr2addr(pem_reg_address, &block_id);
    data_value = hexstr2uints(str_value, &value_length);
    ucode_add(unit, block_id, pem_mem_address_val, 0, data_value, value_length, 0);
  }
}





void db_info_insert(const char* line) {
  char key_word[MAX_NAME_LENGTH],db_type[MAX_NAME_LENGTH], db_name_debug[MAX_NAME_LENGTH];
  int db_id, nof_entries, key_width, result_width;
  int nof_key_fields, nof_result_fields;

  if (sscanf( line, "%s %s %s %d %d %d %d %d %d ", key_word, db_type, db_name_debug, &db_id, &nof_entries, &key_width, &result_width, &nof_key_fields, &nof_result_fields) != PEM_NOF_DB_INFO_TOKEN) {
    /*printf("Bad line format. Skip and continue with next line.\n");*/
    return;
  } 
  else {
    if ((strcmp(db_type, KEYWORD_DB_DIRECT) == 0)) {
      api_info.db_direct_container.db_direct_info_arr[db_id].total_nof_entries = nof_entries;
      api_info.db_direct_container.db_direct_info_arr[db_id].total_result_width = result_width;
      api_info.db_direct_container.db_direct_info_arr[db_id].nof_fields = nof_result_fields;
      api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr = NULL;
      init_logical_fields_location(&api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr, nof_result_fields);
    }
    if ((strcmp(db_type, KEYWORD_DB_TCAM) == 0)) {
      api_info.db_tcam_container.db_tcam_info_arr[db_id].total_nof_entries = nof_entries;
      api_info.db_tcam_container.db_tcam_info_arr[db_id].total_key_width = key_width;
      api_info.db_tcam_container.db_tcam_info_arr[db_id].total_result_width = result_width;
      api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_key = nof_key_fields;
      api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_result = nof_result_fields;
      api_info.db_tcam_container.db_tcam_info_arr[db_id].key_chunk_mapper_matrix_arr = NULL;
      api_info.db_tcam_container.db_tcam_info_arr[db_id].result_chunk_mapper_matrix_arr = NULL;
      init_logical_fields_location(&(api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr), nof_key_fields);
      init_logical_fields_location(&(api_info.db_tcam_container.db_tcam_info_arr[db_id].result_field_bit_range_arr), nof_result_fields);
    }
    if ((strcmp(db_type, KEYWORD_DB_EM) == 0)) {
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.total_nof_entries = nof_entries;
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.total_key_width = key_width;
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.total_result_width = result_width;
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_key = nof_key_fields;
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_result = nof_result_fields;
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_chunk_mapper_matrix_arr = NULL;
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_chunk_mapper_matrix_arr = NULL;
      init_logical_fields_location(&(api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_field_bit_range_arr), nof_key_fields);
      init_logical_fields_location(&(api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_field_bit_range_arr), nof_result_fields);
    }
    if ((strcmp(db_type, KEYWORD_DB_LPM) == 0)) {
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.total_nof_entries = nof_entries;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.total_key_width = key_width;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.total_result_width = result_width;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_key = nof_result_fields;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_result = nof_result_fields;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_chunk_mapper_matrix_arr = NULL;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_chunk_mapper_matrix_arr = NULL;
      init_logical_fields_location(&(api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_field_bit_range_arr), nof_key_fields);
      init_logical_fields_location(&(api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_field_bit_range_arr), nof_result_fields);
    }
  }
}

/* Insert msb/lsb mapping*/
void db_field_info_insert(const char* line) {
  char key_word[MAX_NAME_LENGTH], db_name_debug[MAX_NAME_LENGTH], field_name_debug[MAX_NAME_LENGTH], db_type[MAX_NAME_LENGTH];
  int db_id, field_id, lsb_bit, msb_bit;

  if (sscanf( line, "%s %s %s %d %s %d %d %d", key_word, db_name_debug, db_type, &db_id, field_name_debug, &field_id, &lsb_bit, &msb_bit) != PEM_NOF_KEY_FIELD_TOKEN) {    /*can also be 'PEM_NOF_RESULT_FIELD_TOKEN'*/
    /*printf("Bad line format. Skip and continue with next line.\n");*/
    return;
  } 
    if ((strcmp(key_word, KEYWORD_KEY_INFO) == 0)) 
      db_key_field_info_insert(db_type, db_id, field_id, lsb_bit, msb_bit);
    else if((strcmp(key_word, KEYWORD_RESULT_INFO) == 0)) 
      db_result_field_info_insert(db_type, db_id, field_id, lsb_bit, msb_bit);
    return;
  }



/* Insert msb/lsb mapping to logical_key_fields_location*/
void db_key_field_info_insert(const char* db_type, const int db_id, const int field_id, const int lsb_bit, const int msb_bit) {
  if ((strcmp(db_type, KEYWORD_DB_TCAM) == 0)) {
    api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr[field_id].lsb = lsb_bit;
    api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr[field_id].msb = msb_bit;
  }
  else if ((strcmp(db_type, KEYWORD_DB_EM) == 0)) {
    api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_field_bit_range_arr[field_id].lsb = lsb_bit;
    api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_field_bit_range_arr[field_id].msb = msb_bit;
  }
  else if ((strcmp(db_type, KEYWORD_DB_LPM) == 0)) {
    if (field_id == 0) {
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_field_bit_range_arr[field_id].lsb = lsb_bit;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_field_bit_range_arr[field_id].msb = msb_bit;
    }
  }
}

/* Insert msb/lsb mapping to logical_result_fields_location*/
void db_result_field_info_insert(const char* db_type, const int db_id, const int field_id, const int lsb_bit, const int msb_bit) {
  if ((strcmp(db_type, KEYWORD_DB_DIRECT) == 0)) {
    api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr[field_id].lsb = lsb_bit;
    api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr[field_id].msb = msb_bit;
  }
  else if ((strcmp(db_type, KEYWORD_DB_TCAM) == 0)) {
    api_info.db_tcam_container.db_tcam_info_arr[db_id].result_field_bit_range_arr[field_id].lsb = lsb_bit;
    api_info.db_tcam_container.db_tcam_info_arr[db_id].result_field_bit_range_arr[field_id].msb = msb_bit;
  }
  else if ((strcmp(db_type, KEYWORD_DB_EM) == 0)) {
    api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_field_bit_range_arr[field_id].lsb = lsb_bit;
    api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_field_bit_range_arr[field_id].msb = msb_bit;
  }
  else if ((strcmp(db_type, KEYWORD_DB_LPM) == 0)) {
    if (field_id == 0) {
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_field_bit_range_arr[field_id].lsb = lsb_bit;
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_field_bit_range_arr[field_id].msb = msb_bit;
    }
  }
}


/* Updates is_mapped per each field in key or result. Writes 1 if field was mapped to physical and 0 else.*/
void is_field_mapped_update(const int num_of_fields, FieldBitRange* field_bit_range_arr , DbChunkMapper*const chunk_info_ptr) {
  int i;
  unsigned int chunk_lsb_bit = chunk_info_ptr->virtual_mem_width_offset;
  unsigned int chunk_msb_bit = chunk_lsb_bit + chunk_info_ptr->chunk_width - 1;
  for (i = 0; i < num_of_fields; ++i) {
    if (field_bit_range_arr[i].msb < chunk_lsb_bit && field_bit_range_arr[i].lsb > chunk_msb_bit ) 
      continue;
    else
      field_bit_range_arr[i].is_field_mapped = 1;
  }
}


/*Insert one chunk of DIRECT database into api_info.db_direct_container.db_direct_info_arr*/
void direct_result_chunk_insert(const char* line) {
  unsigned int db_id;
  DbChunkMapper* chunk_info_ptr = (DbChunkMapper*)calloc(1, sizeof(DbChunkMapper));
  db_id = build_direct_chunk_from_ucode(line, chunk_info_ptr);
  if (db_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");*/
    free(chunk_info_ptr);
    return;
  }
  db_chunk_insert(api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr, chunk_info_ptr);
  is_field_mapped_update(api_info.db_direct_container.db_direct_info_arr[db_id].nof_fields,
                         api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr,
                         chunk_info_ptr);
  free(chunk_info_ptr);
}

/*Insert one key chunk of TCAM database into api_info.db_tcam_container.db_tcam_info_arr*/
void tcam_key_chunk_insert(const char* line) {
  unsigned int db_id;
  DbChunkMapper* chunk_info_ptr = (DbChunkMapper*)calloc(1, sizeof(DbChunkMapper));
  db_id = build_cam_key_chunk_from_ucode(line, chunk_info_ptr);
  if (db_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");*/
    free(chunk_info_ptr);
    return;
  }
  db_chunk_insert(api_info.db_tcam_container.db_tcam_info_arr[db_id].key_chunk_mapper_matrix_arr, chunk_info_ptr);
  is_field_mapped_update(api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_key,
                         api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr,
                         chunk_info_ptr);
  free(chunk_info_ptr);
}

/*Insert one result chunk of TCAM database into api_info.db_tcam_container.db_tcam_info_arr*/
void tcam_result_chunk_insert(const char* line) {
  unsigned int db_id;
  DbChunkMapper* chunk_info_ptr = (DbChunkMapper*)calloc(1, sizeof(DbChunkMapper));
  db_id = build_cam_result_chunk_from_ucode(line, chunk_info_ptr);
  if (db_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");*/
    free(chunk_info_ptr);
    return;
  }
  db_chunk_insert(api_info.db_tcam_container.db_tcam_info_arr[db_id].result_chunk_mapper_matrix_arr, chunk_info_ptr);
  is_field_mapped_update(api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_result,
                         api_info.db_tcam_container.db_tcam_info_arr[db_id].result_field_bit_range_arr,
                         chunk_info_ptr);
  free(chunk_info_ptr);
}

/*Insert one key chunk of EM database into api_info.db_em_container.db_em_info_arr*/
void em_key_chunk_insert(const char* line) {
  unsigned int db_id;
  DbChunkMapper* chunk_info_ptr = (DbChunkMapper*)calloc(1, sizeof(DbChunkMapper));
  db_id = build_cam_key_chunk_from_ucode(line, chunk_info_ptr);
  if (db_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");*/
    free(chunk_info_ptr);
    return;
  }
  db_chunk_insert(api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_chunk_mapper_matrix_arr, chunk_info_ptr);
  is_field_mapped_update(api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_key,
                         api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_field_bit_range_arr,
                         chunk_info_ptr);
  free(chunk_info_ptr);
}

/*Insert one result chunk of EM database into api_info.db_em_container.db_em_info_arr*/
void em_result_chunk_insert(const char* line) {
  unsigned int db_id;
  DbChunkMapper* chunk_info_ptr = (DbChunkMapper*)calloc(1, sizeof(DbChunkMapper));
  db_id = build_cam_result_chunk_from_ucode(line, chunk_info_ptr);
  if (db_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");*/
    free(chunk_info_ptr);
    return;
  }
  db_chunk_insert(api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_chunk_mapper_matrix_arr, chunk_info_ptr);
  is_field_mapped_update(api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_result,
                         api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_field_bit_range_arr,
                         chunk_info_ptr);
}

/*Insert one key chunk of LPM database into api_info.db_lpm_container.db_lpm_info_arr*/
void lpm_key_chunk_insert(const char* line) {
  unsigned int db_id;
  DbChunkMapper* chunk_info_ptr = (DbChunkMapper*)calloc(1, sizeof(DbChunkMapper));
  db_id = build_cam_key_chunk_from_ucode(line, chunk_info_ptr);
  if (db_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");*/
    free(chunk_info_ptr);
    return;
  }
  db_chunk_insert(api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_chunk_mapper_matrix_arr, chunk_info_ptr);
  is_field_mapped_update(api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_key,
                         api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_field_bit_range_arr,
                         chunk_info_ptr);
}

/*Insert one result chunk of LPM database into api_info.db_lpm_container.db_lpm_info_arr*/
void lpm_result_chunk_insert(const char* line) {
  unsigned int db_id;
  DbChunkMapper* chunk_info_ptr = (DbChunkMapper*)calloc(1, sizeof(DbChunkMapper));
  db_id = build_cam_result_chunk_from_ucode(line, chunk_info_ptr);
  if (db_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");*/
    free(chunk_info_ptr);
    return;
  }
  db_chunk_insert(api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_chunk_mapper_matrix_arr, chunk_info_ptr);
  is_field_mapped_update(api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_result,
                         api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_field_bit_range_arr,
                         chunk_info_ptr);
  free(chunk_info_ptr);
}

/* Build result chunk of DIRECT db. Modifies chunk_info_ptr and returns db_id*/
unsigned int build_direct_chunk_from_ucode(const char* line, DbChunkMapper* chunk_info_ptr) {
  char pem_mem_address[40];                /* this contain hex string address in format 40'hxxxx*/
  unsigned int pem_mem_width;
  unsigned int curr_offset, previous_chunks_to_add;
  unsigned int db_id, field_id, field_width, pe_matrix_col, phy_mem_idx, mem_start_row, mem_start_colomn, implementation_index;
  unsigned int index_range[2], bits_range[2], db_dim[2];
  char key_word[MAX_NAME_LENGTH],db_type[MAX_NAME_LENGTH], db_name_debug[MAX_NAME_LENGTH], field_name_debug[MAX_NAME_LENGTH];
  char pem_mem_name[MAX_NAME_LENGTH];
  char colon, x;

  if (sscanf( line, "%s %s %d %s %d %u %d%c%d %d%c%d %d%c%d %s %d %d %s %s %d %d %d %d ",
    key_word, db_name_debug, (int *)&db_id, field_name_debug, (int *)&field_id, &field_width,
    (int *)&db_dim[DB_ROWS], &x, (int *)&db_dim[DB_COLUMNS], 
    (int *)&index_range[PEM_RANGE_HIGH_LIMIT], &colon, (int *)&index_range[PEM_RANGE_LOW_LIMIT], 
    (int *)&bits_range[PEM_RANGE_HIGH_LIMIT], &colon, (int *)&bits_range[PEM_RANGE_LOW_LIMIT], 
    db_type, (int *)&pe_matrix_col, (int *)&phy_mem_idx, pem_mem_name, pem_mem_address, 
    (int *)&mem_start_row, (int *)&mem_start_colomn, (int *)&pem_mem_width, (int *)&implementation_index) != PEM_NOF_DB_DIRECT_INFO_TOKEN) {
      /*printf("Bad line format. Skip and continue with next line.\n");*/
      return UINT_MAX;
  } 
  else {
    curr_offset = api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr[field_id].lsb;
    chunk_info_ptr->chunk_matrix_row_ndx  = index_range[PEM_RANGE_LOW_LIMIT] / PEM_CFG_API_MAP_CHUNK_N_ENTRIES;
    previous_chunks_to_add = (curr_offset > 0 )? 1 + (curr_offset - 1)/PEM_CFG_API_MAP_CHUNK_WIDTH  : curr_offset / PEM_CFG_API_MAP_CHUNK_WIDTH;     /*(x > 0)? 1 + (x - 1)/y: (x / y);*/
    chunk_info_ptr->chunk_matrix_col_ndx  = (bits_range[PEM_RANGE_LOW_LIMIT] / PEM_CFG_API_MAP_CHUNK_WIDTH) + previous_chunks_to_add; 
    chunk_info_ptr->chunk_matrix_ndx = implementation_index;
    chunk_info_ptr->chunk_entries = index_range[PEM_RANGE_HIGH_LIMIT] - index_range[PEM_RANGE_LOW_LIMIT] + 1;
    chunk_info_ptr->chunk_width = bits_range[PEM_RANGE_HIGH_LIMIT] - bits_range[PEM_RANGE_LOW_LIMIT] + 1;
    chunk_info_ptr->phy_mem_addr = hexstr2addr(pem_mem_address, &chunk_info_ptr->mem_block_id);
    chunk_info_ptr->phy_mem_entry_offset = mem_start_row;
    chunk_info_ptr->phy_mem_index = phy_mem_idx;
    chunk_info_ptr->phy_mem_name = (char*)malloc(strlen(pem_mem_name) + 1);
    strcpy(chunk_info_ptr->phy_mem_name, pem_mem_name);
    chunk_info_ptr->phy_mem_width = pem_mem_width;
    chunk_info_ptr->phy_mem_width_offset = mem_start_colomn;
    chunk_info_ptr->phy_pe_matrix_col = pe_matrix_col;
    chunk_info_ptr->virtual_mem_entry_offset = index_range[PEM_RANGE_LOW_LIMIT];
    chunk_info_ptr->virtual_mem_width_offset = bits_range[PEM_RANGE_LOW_LIMIT] + curr_offset;

    return db_id;
  }
}


/* Build key chunk of CAM based db. Modifies chunk_info_ptr and returns db_id*/
unsigned int build_cam_key_chunk_from_ucode(const char* line, DbChunkMapper* chunk_info_ptr) {
  char pem_mem_address[40];                /* this contain hex string address in format 40'hxxxx*/
  unsigned int pem_mem_width, mem_nof_rows_debug, total_width;
  unsigned int db_id, pe_matrix_col, phy_mem_idx, mem_start_row, valid_col, implementation_index;
  unsigned int mem_valid_coloms, mem_mask_st_col, mem_key_st_col;
  unsigned int index_range[2], mask_bit_range[2], key_bit_range[2];
  char key_word[MAX_NAME_LENGTH],db_type[MAX_NAME_LENGTH], db_name_debug[MAX_NAME_LENGTH], field_name_debug[MAX_NAME_LENGTH];
  char pem_mem_name[MAX_NAME_LENGTH];
  char colon, x;
  unsigned int ret ;

  ret = UINT_MAX ;
  if (sscanf( line, "%s %s %d %s %d%c%d %d%c%d %d %d%c%d %d%c%d %s %d %d %s %s %d %d %d %d %d %d ",
    key_word, db_name_debug, (int *)&db_id, field_name_debug,  (int *)&mem_nof_rows_debug, &x, (int *)&total_width,
    (int *)&index_range[PEM_RANGE_HIGH_LIMIT], &colon, (int *)&index_range[PEM_RANGE_LOW_LIMIT], (int *)&valid_col,
    (int *)&mask_bit_range[PEM_RANGE_HIGH_LIMIT], &colon, (int *)&mask_bit_range[PEM_RANGE_LOW_LIMIT],
    (int *)&key_bit_range[PEM_RANGE_HIGH_LIMIT], &colon, (int *)&key_bit_range[PEM_RANGE_LOW_LIMIT],
    db_type, (int *)&pe_matrix_col, (int *)&phy_mem_idx, pem_mem_name, pem_mem_address,
    (int *)&mem_start_row, (int *)&mem_valid_coloms, (int *)&mem_mask_st_col,
    (int *)&mem_key_st_col, (int *)&pem_mem_width, (int *)&implementation_index) != PEM_NOF_DB_CAM_KEY_MAPPING_INFO_TOKEN) {

      /*printf("Bad line format. Skip and continue with next line.\n");*/
      ret = UINT_MAX;
  } 
  else {
    chunk_info_ptr->chunk_matrix_row_ndx  = index_range[PEM_RANGE_LOW_LIMIT] / PEM_CFG_API_MAP_CHUNK_N_ENTRIES;
    chunk_info_ptr->chunk_matrix_col_ndx  = key_bit_range[PEM_RANGE_LOW_LIMIT]  / PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH;
    chunk_info_ptr->chunk_matrix_ndx = implementation_index;
    chunk_info_ptr->chunk_entries = index_range[PEM_RANGE_HIGH_LIMIT] - index_range[PEM_RANGE_LOW_LIMIT] + 1;
    chunk_info_ptr->chunk_width = key_bit_range[PEM_RANGE_HIGH_LIMIT] - key_bit_range[PEM_RANGE_LOW_LIMIT] + 1;
    chunk_info_ptr->phy_mem_addr = hexstr2addr(pem_mem_address, &chunk_info_ptr->mem_block_id);
    chunk_info_ptr->phy_mem_entry_offset = mem_start_row;
    chunk_info_ptr->phy_mem_index = phy_mem_idx;
    chunk_info_ptr->phy_mem_name = (char*)malloc(strlen(pem_mem_name) + 1);
    strcpy(chunk_info_ptr->phy_mem_name, pem_mem_name);
    chunk_info_ptr->phy_mem_width = pem_mem_width;
    chunk_info_ptr->phy_mem_width_offset = mem_key_st_col;
    chunk_info_ptr->phy_pe_matrix_col = pe_matrix_col;
    chunk_info_ptr->virtual_mem_entry_offset = index_range[PEM_RANGE_LOW_LIMIT];
    chunk_info_ptr->virtual_mem_width_offset = key_bit_range[PEM_RANGE_LOW_LIMIT];

    ret = db_id;
  }
  return (ret) ;
}
/* Build result chunk of CAM based db. Modifies chunk_info_ptr and returns db_id*/
unsigned int build_cam_result_chunk_from_ucode(const char* line, DbChunkMapper* chunk_info_ptr) {
  char pem_mem_address[40];                /* this contain hex string address in format 40'hxxxx*/
  unsigned int pem_mem_width, mem_nof_rows_debug, total_width;
  unsigned int db_id, pe_matrix_col, phy_mem_idx, mem_start_row, mem_start_colomn, implementation_index;
  unsigned int index_range[2], bits_range[2];
  char key_word[MAX_NAME_LENGTH], db_type[MAX_NAME_LENGTH], db_name_debug[MAX_NAME_LENGTH], field_name_debug[MAX_NAME_LENGTH];
  char pem_mem_name[MAX_NAME_LENGTH];
  char colon, x;
  if (sscanf( line, "%s %s %d %s %d%c%d %d%c%d %d%c%d %s %d %d %d %s %s %d %d %d ",
    key_word, db_name_debug, (int *)&db_id, field_name_debug,  (int *)&mem_nof_rows_debug, &x, (int *)&total_width,
    (int *)&index_range[PEM_RANGE_HIGH_LIMIT], &colon, (int *)&index_range[PEM_RANGE_LOW_LIMIT],
    (int *)&bits_range[PEM_RANGE_HIGH_LIMIT], &colon, (int *)&bits_range[PEM_RANGE_LOW_LIMIT], 
    db_type, (int *)&pe_matrix_col, (int *)&pem_mem_width, (int *)&phy_mem_idx, pem_mem_name, pem_mem_address,
    (int *)&mem_start_row, (int *)&mem_start_colomn, (int *)&implementation_index) != PEM_NOF_DB_CAM_RESULT_MAPPING_INFO_TOKEN) {

      /*printf("Bad line format. Skip and continue with next line.\n");*/
      return UINT_MAX;
  } 
  else {
    chunk_info_ptr->chunk_matrix_row_ndx  = index_range[PEM_RANGE_LOW_LIMIT] / PEM_CFG_API_CAM_RESULT_CHUNK_N_ENTRIES;
    chunk_info_ptr->chunk_matrix_col_ndx  = bits_range[PEM_RANGE_LOW_LIMIT]  / PEM_CFG_API_CAM_RESULT_CHUNK_WIDTH;
    chunk_info_ptr->chunk_matrix_ndx = implementation_index;
    chunk_info_ptr->chunk_entries = index_range[PEM_RANGE_HIGH_LIMIT] - index_range[PEM_RANGE_LOW_LIMIT] + 1;
    chunk_info_ptr->chunk_width = bits_range[PEM_RANGE_HIGH_LIMIT] - bits_range[PEM_RANGE_LOW_LIMIT] + 1;
    chunk_info_ptr->phy_mem_addr = hexstr2addr(pem_mem_address, &chunk_info_ptr->mem_block_id);
    chunk_info_ptr->phy_mem_entry_offset = mem_start_row;
    chunk_info_ptr->phy_mem_index = phy_mem_idx;
    chunk_info_ptr->phy_mem_name = (char*)malloc(strlen(pem_mem_name) + 1);
    strcpy(chunk_info_ptr->phy_mem_name, pem_mem_name);
    chunk_info_ptr->phy_mem_width = pem_mem_width;
    chunk_info_ptr->phy_mem_width_offset = mem_start_colomn;
    chunk_info_ptr->phy_pe_matrix_col = pe_matrix_col;
    chunk_info_ptr->virtual_mem_entry_offset = index_range[PEM_RANGE_LOW_LIMIT];
    chunk_info_ptr->virtual_mem_width_offset = bits_range[PEM_RANGE_LOW_LIMIT];

    return db_id;
  }
}

/*Insert register*/
void register_insert(const char* line) {
  unsigned int reg_id;
  RegFieldMapper* reg_field_info_ptr = (RegFieldMapper*)calloc(1, sizeof(RegFieldMapper));
  reg_id = build_register_mapping_from_ucode(line, reg_field_info_ptr);
  if (reg_id == UINT_MAX) {
    /*printf("failed to insert chunk.\n");  */
  }
  db_virtual_reg_field_mapping_insert(&(api_info.reg_container.reg_info_arr[reg_id]), reg_field_info_ptr);
  api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr[reg_field_info_ptr->field_id].is_field_mapped = 1;       /*update that specific field is mapped*/
  free(reg_field_info_ptr);
}


/* Build result chunk of CAM based db. Modifies chunk_info_ptr and returns db_id*/
unsigned int build_register_mapping_from_ucode(const char* line, RegFieldMapper* reg_field_mapping_ptr) {
  char pem_mem_address[40];                                  /* this contain hex string address in format 40'hxxxx*/
  unsigned int                 field_id                             ; /* for debug - it is the location in the containing array*/
  unsigned int                 virtual_field_lsb                    ;
  unsigned int                 virtual_field_msb                    ;
  unsigned int                 pem_matrix_col                       ;
  unsigned int                 pem_mem_line                         ;
  unsigned int                 pem_mem_offset                       ;
  unsigned int                 pem_mem_total_width                  ;
  unsigned int                 pem_mem_ndx                          ;
  unsigned int                 reg_id                               ;
  unsigned int                 mapping_id                           ;

  unsigned int mapping_width;
  char key_word[MAX_NAME_LENGTH], db_type[MAX_NAME_LENGTH], db_name_debug[MAX_NAME_LENGTH], field_name_debug[MAX_NAME_LENGTH];
  char pem_mem_name[MAX_NAME_LENGTH];
  char colon;
  if (sscanf( line, "%s %s %d %d %s %d %d %d%c%d %s %d %d %s %s %d %d %d", 
    key_word, db_name_debug, (int *)&reg_id, (int *)&mapping_id, field_name_debug, (int *)&field_id, (int *)&mapping_width,
    (int *)&virtual_field_msb, &colon, (int *)&virtual_field_lsb, 
    db_type, (int *)&pem_matrix_col, (int *)&pem_mem_ndx, pem_mem_name, pem_mem_address, 
    (int *)&pem_mem_line, (int *)&pem_mem_offset, (int *)&pem_mem_total_width) != PEM_NOF_REGISTER_INFO_TOKEN) {
      /*printf("Bad line format. Skip and continue with next line.\n");*/
      return UINT_MAX;
  } else {
    reg_field_mapping_ptr->field_id                       = field_id              ;
    reg_field_mapping_ptr->cfg_mapper_width               = mapping_width         ;
    reg_field_mapping_ptr->virtual_field_lsb              = virtual_field_lsb     ;
    reg_field_mapping_ptr->virtual_field_msb              = virtual_field_msb     ;
    reg_field_mapping_ptr->pem_matrix_col                 = pem_matrix_col        ;
    reg_field_mapping_ptr->pem_mem_address                = hexstr2addr(pem_mem_address, &reg_field_mapping_ptr->pem_mem_block_id);
    reg_field_mapping_ptr->pem_mem_line                   = pem_mem_line          ;
    reg_field_mapping_ptr->pem_mem_offset                 = pem_mem_offset        ;
    reg_field_mapping_ptr->pem_mem_total_width            = pem_mem_total_width   ;
    reg_field_mapping_ptr->mapping_id                     = mapping_id            ;

    return reg_id;
  }
}

void db_chunk_insert(LogicalDbChunkMapperMatrix* logical_db_mapper, DbChunkMapper*const chunk_info_ptr) {
    logical_db_mapper[chunk_info_ptr->chunk_matrix_ndx].db_chunk_mapper[chunk_info_ptr->chunk_matrix_row_ndx][chunk_info_ptr->chunk_matrix_col_ndx] = chunk_info_ptr;
  }

 void db_virtual_reg_field_mapping_insert(LogicalRegInfo* reg_info_p, RegFieldMapper* reg_field_mapping_ptr) {
   reg_info_p->reg_field_info_arr[reg_field_mapping_ptr->field_id].reg_field_mapping_arr[reg_field_mapping_ptr->mapping_id] = *reg_field_mapping_ptr;
   reg_info_p->is_mapped = 1;
}


/* Init ChunkMApper mattrix*/
void init_logical_db_chunk_mapper(const char* line){
  int db_id, nof_key_chunk_rows, nof_key_chunk_cols, nof_result_chunk_rows, nof_result_chunk_cols, nof_implementations;
  char db_name[40];    
  /*in case of direct database*/
  if ((strncmp(line + KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO_SIZE + 1, KEYWORD_DB_DIRECT, KEYWORD_DB_DIRECT_SIZE) == 0)) { 
    if (sscanf( line, "%s %d %d %d %d ", db_name, &db_id, &nof_result_chunk_rows, &nof_result_chunk_cols, &nof_implementations) != DB_SINGLE_VIRT_DB_MAP_INFO_DIRECT_TOKEN) {
      /*printf("Bad line format. Skip and continue with next line.\n");*/
      return;
    }
    /*in case the result has sevral fields, same line is printed sevral times in dump -> avoiding second initialization*/
    if (api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr != NULL) {
      realloc_logical_db_mapper_matrix(&api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr, nof_result_chunk_cols, nof_implementations);
      return;
    }
    init_logical_db_mapper_matrix(&api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr, nof_result_chunk_rows, nof_result_chunk_cols, nof_implementations);
    api_info.db_direct_container.db_direct_info_arr[db_id].nof_chunk_matrices = nof_implementations;
  }
  /*in case of CAM based database*/
  else {
    if (sscanf( line, "%s %d %d %d %d %d %d ", db_name, &db_id, &nof_key_chunk_rows, &nof_key_chunk_cols, &nof_result_chunk_rows, &nof_result_chunk_cols, &nof_implementations) != DB_SINGLE_VIRT_DB_MAP_INFO_TOKEN) {
      /*printf("Bad line format. Skip and continue with next line.\n");*/
      return; 
    }
    if ( (strncmp(line + KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO_SIZE + 1, KEYWORD_DB_TCAM, KEYWORD_DB_TCAM_SIZE) == 0) ) {
      init_logical_db_mapper_matrix(&api_info.db_tcam_container.db_tcam_info_arr[db_id].key_chunk_mapper_matrix_arr, nof_key_chunk_rows, nof_key_chunk_cols, nof_implementations);
      init_logical_db_mapper_matrix(&api_info.db_tcam_container.db_tcam_info_arr[db_id].result_chunk_mapper_matrix_arr, nof_result_chunk_rows, nof_result_chunk_cols, nof_implementations);
      api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_chunk_matrices = nof_implementations;
    }
    else if ( (strncmp(line + KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO_SIZE + 1, KEYWORD_DB_EM, KEYWORD_DB_EM_SIZE) == 0) ) {
      init_logical_db_mapper_matrix(&api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_chunk_mapper_matrix_arr, nof_key_chunk_rows, nof_key_chunk_cols, nof_implementations);
      init_logical_db_mapper_matrix(&api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_chunk_mapper_matrix_arr, nof_result_chunk_rows, nof_result_chunk_cols, nof_implementations);
      init_em_cache(&api_info.db_em_container.db_em_info_arr[db_id].em_cache);
      api_info.db_em_container.db_em_info_arr[db_id].em_cache.next_free_index = 0;
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_chunk_matrices = nof_implementations;
    }
    else if ((strncmp(line + KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO_SIZE + 1, KEYWORD_DB_LPM, KEYWORD_DB_LPM_SIZE) == 0)) {
      init_logical_db_mapper_matrix(&api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_chunk_mapper_matrix_arr, nof_key_chunk_rows, nof_key_chunk_cols, nof_implementations);
      init_logical_db_mapper_matrix(&api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_chunk_mapper_matrix_arr, nof_result_chunk_rows, nof_result_chunk_cols, nof_implementations);
      init_lpm_cache(&api_info.db_lpm_container.db_lpm_info_arr[db_id].lpm_cache);
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_chunk_matrices = nof_implementations;
    }

  }
}

/* Init LogicalDbChunkMapperMatrix and chunkMapper to their proper size*/
void init_logical_db_mapper_matrix(LogicalDbChunkMapperMatrix** logical_db_mapper_matrix_ptr, const int nof_chunk_rows, const int nof_chunk_cols, const int nof_implementations) {
  int i, j;
  LogicalDbChunkMapperMatrix* logical_db_mapper_matrix;
  
  *logical_db_mapper_matrix_ptr = (LogicalDbChunkMapperMatrix*)calloc(nof_implementations, sizeof(LogicalDbChunkMapperMatrix));
  logical_db_mapper_matrix = *logical_db_mapper_matrix_ptr;

  for ( i = 0; i < nof_implementations; i = i + 1 ) {
    logical_db_mapper_matrix[i].nof_cols_in_chunk_matrix = nof_chunk_cols;
    logical_db_mapper_matrix[i].nof_rows_in_chunk_matrix= nof_chunk_rows;
    logical_db_mapper_matrix[i].db_chunk_mapper = (DbChunkMapper***)calloc(nof_chunk_rows, sizeof(DbChunkMapper**));
    for ( j = 0; j < nof_chunk_rows; j = j + 1 ) {
      logical_db_mapper_matrix[i].db_chunk_mapper[j] =  (DbChunkMapper**)calloc(nof_chunk_cols, sizeof(DbChunkMapper*));
    }
  } 
}

/* Realloc LogicalDbChunkMapperMatrix and chunkMapper to their new proper size*/
void realloc_logical_db_mapper_matrix(LogicalDbChunkMapperMatrix** logical_db_mapper_matrix_ptr, const int nof_chunk_cols_to_add, const int nof_implementations) {
  int i, j, nof_chunk_cols, nof_chunk_rows;
  LogicalDbChunkMapperMatrix* logical_db_mapper_matrix;
  logical_db_mapper_matrix = *logical_db_mapper_matrix_ptr;
  for( i = 0; i < nof_implementations; ++i) {
    nof_chunk_rows = logical_db_mapper_matrix[i].nof_rows_in_chunk_matrix;
    nof_chunk_cols = logical_db_mapper_matrix[i].nof_cols_in_chunk_matrix;
    nof_chunk_cols += nof_chunk_cols_to_add;
    logical_db_mapper_matrix[i].nof_cols_in_chunk_matrix = nof_chunk_cols;
    for(j = 0; j < nof_chunk_rows; ++j) {
      logical_db_mapper_matrix[i].db_chunk_mapper[j] = (DbChunkMapper**)realloc(logical_db_mapper_matrix[i].db_chunk_mapper[j], nof_chunk_cols * sizeof(DbChunkMapper*));
    }
  }
  return;
}

/* Init logical fields location */
void init_logical_fields_location(FieldBitRange** field_bit_range_arr, const int nof_fields) {
  (*field_bit_range_arr) = (FieldBitRange*)calloc(nof_fields, sizeof(FieldBitRange));
}

/* Init Logical_Reg_Field_Info*/
void init_reg_field_info(const char* line) {
  char key_word[MAX_NAME_LENGTH];
  unsigned int reg_id, nof_fields, register_total_width;
  if (sscanf( line, "%s %d %d %d", key_word, (int *)&reg_id, (int *)&nof_fields, (int *)&register_total_width) != VIRTUAL_REGISTER_NOF_FIELDS_TOKEN) {
    /*printf("Bad line format. Skip and continue with next line.\n");*/
    return;
  }
  else {
    api_info.reg_container.reg_info_arr[reg_id].nof_fields              = nof_fields;
    api_info.reg_container.reg_info_arr[reg_id].register_total_width    = register_total_width;
    api_info.reg_container.reg_info_arr[reg_id].reg_field_info_arr      = (LogicalRegFieldInfo*)calloc(nof_fields, sizeof(LogicalRegFieldInfo));
    api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr = (FieldBitRange*)      calloc(nof_fields, sizeof(FieldBitRange));
  }
}

/* Init Reg_Field_Mapper*/
void init_reg_field_mapper(const char* line) {
  char key_word[MAX_NAME_LENGTH];
  unsigned int reg_id, field_id, nof_mappings, lsb_bit, msb_bit;
  if (sscanf( line, "%s %d %d %d %d %d", key_word, (int *)&reg_id, (int *)&field_id, (int *)&lsb_bit, (int *)&msb_bit, (int *)&nof_mappings) != VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS_TOKEN) {
    /*printf("Bad line format. Skip and continue with next line.\n");*/
    return;
  }
  else {
    api_info.reg_container.reg_info_arr[reg_id].reg_field_info_arr[field_id].nof_mappings = nof_mappings;
    api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr[field_id].lsb = lsb_bit;
    api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr[field_id].msb = msb_bit;
    api_info.reg_container.reg_info_arr[reg_id].reg_field_info_arr[field_id].reg_field_mapping_arr = (RegFieldMapper*)calloc(nof_mappings, sizeof(RegFieldMapper));
  }
}


void init_meminfo_array_for_applets(const char* line) {
  char key_word[MAX_NAME_LENGTH];
  unsigned int last_stage_of_ingress, nof_mems;
  sscanf( line, "%s %u %u", key_word, &nof_mems, &last_stage_of_ingress);

  api_info.applet_info.currently_writing_applets_bit   = 0;
  api_info.applet_info.nof_pending_applets[0]          = 0;
  api_info.applet_info.nof_pending_applets[1]          = 0;
  api_info.applet_info.meminfo_curr_array_size         = 0;
  api_info.applet_info.last_stage_of_ingress           = last_stage_of_ingress;
  api_info.applet_info.meminfo_array_for_applet        = (PemMemInfoForApplet**)malloc(nof_mems*sizeof(PemMemInfoForApplet*));
}


void insert_meminfo_to_array_for_applets(const char* line) {  
  char key_word[MAX_NAME_LENGTH], mem_name[MAX_NAME_LENGTH];
  char pem_reg_address[40];
  unsigned int relevant_stage, u_mem_address, block_id;
  unsigned long long final_address;  
  int nof_entries_currently_in_array = api_info.applet_info.meminfo_curr_array_size;
  PemMemInfoForApplet* mem_info;
  sscanf( line, "%s %s %s %u", key_word, mem_name, pem_reg_address, &relevant_stage);
  u_mem_address = hexstr2addr(pem_reg_address, &block_id);
  /* calc final address - take 16 msbs of address and block id*/
  final_address = block_id;
  final_address <<= 32;
  final_address |= u_mem_address;
  final_address >>= 16;
  
  /*allocate struct, fill data and insert to array*/
  mem_info = (PemMemInfoForApplet*)malloc(sizeof(PemMemInfoForApplet));
  mem_info->address  = (unsigned int)final_address;
  sprintf(mem_info->name, "%s", mem_name);
  mem_info->stage_relevant_for = relevant_stage;
  api_info.applet_info.meminfo_array_for_applet[nof_entries_currently_in_array] = mem_info;
  api_info.applet_info.meminfo_curr_array_size++;
}

void init_pem_applet_reg(const char* line) {
  char key_word[MAX_NAME_LENGTH], name[MAX_NAME_LENGTH], address_s[MAX_NAME_LENGTH];
  unsigned int ingress0_egress_1, address, block_id, width_in_bits;
  sscanf( line, "%s %s %u %s %u", key_word, name, &ingress0_egress_1, address_s, &width_in_bits);

  address = hexstr2addr(address_s, &block_id);
  api_info.applet_info.applet_reg_info[ingress0_egress_1].address = address;
  api_info.applet_info.applet_reg_info[ingress0_egress_1].block_id = block_id;
  api_info.applet_info.applet_reg_info[ingress0_egress_1].is_ingress0_egress1 = ingress0_egress_1;
  api_info.applet_info.applet_reg_info[ingress0_egress_1].length_in_bits = width_in_bits;
  sprintf(api_info.applet_info.applet_reg_info[ingress0_egress_1].name, "%s", name);
}

void init_pem_applet_mem(const char* line) {
  char key_word[MAX_NAME_LENGTH], name[MAX_NAME_LENGTH], address_s[MAX_NAME_LENGTH];
  unsigned int ingress0_egress_1, address, block_id, width_in_bits;
  sscanf( line, "%s %s %u %s %u", key_word, name, &ingress0_egress_1, address_s, &width_in_bits);

  address = hexstr2addr(address_s, &block_id);
  api_info.applet_info.applet_mem_info[ingress0_egress_1].address = address;
  api_info.applet_info.applet_mem_info[ingress0_egress_1].block_id = block_id;
  api_info.applet_info.applet_mem_info[ingress0_egress_1].is_ingress0_egress1 = ingress0_egress_1;
  api_info.applet_info.applet_mem_info[ingress0_egress_1].length_in_bits = width_in_bits;
  sprintf(api_info.applet_info.applet_mem_info[ingress0_egress_1].name, "%s", name);
}






/* Init EM cache members*/
void init_em_cache(EmDbCache* em_cache_info){
  int i;
  em_cache_info->em_key_entry_arr = (unsigned int**)calloc(PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES, sizeof(unsigned int*));
  em_cache_info->em_result_entry_arr = (unsigned int**)calloc(PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES, sizeof(unsigned int*));
  for(i = 0; i < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++i) {
    em_cache_info->em_key_entry_arr[i] = (unsigned int*)calloc(1, sizeof(unsigned int));
    em_cache_info->em_result_entry_arr[i] = (unsigned int*)calloc(1, sizeof(unsigned int));
  }
  em_cache_info->physical_entry_occupation = (unsigned char*)calloc(PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES, sizeof(unsigned char));
}
/* Init LPM cache members*/
void init_lpm_cache(LpmDbCache* lpm_cache_info){
  int i;
  lpm_cache_info->lpm_key_entry_arr = (unsigned int**)calloc(PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES, sizeof(unsigned int*));
  lpm_cache_info->lpm_result_entry_arr = (unsigned int**)calloc(PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES, sizeof(unsigned int*));
  for(i = 0; i < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++i) {
    lpm_cache_info->lpm_key_entry_arr[i] = (unsigned int*)calloc(1, sizeof(unsigned int));
    lpm_cache_info->lpm_result_entry_arr[i] = (unsigned int*)calloc(1, sizeof(unsigned int));
  }
  lpm_cache_info->lpm_key_entry_prefix_length_arr = (unsigned int*)calloc(PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES, sizeof(unsigned int));
  lpm_cache_info->physical_entry_occupation = (unsigned char*)calloc(PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES, sizeof(unsigned char));
}


/**********************************************************************************************/
/* Init DBs content from write_commands file*/

int db_direct_write_command(const char* write_command);
int db_tcam_write_command(const char* write_command);
int db_lpm_write_command(const char* write_command);
int db_em_write_command(const char* write_command);
int reg_write_command(const char* write_command);
int program_select_tcam_write_command(const char* write_command);

int init_dbs_content_and_program_selection_tcams_from_write_commands_file(const char* write_commands_file_name) {
  FILE* fp = NULL;
  int ret_val = 0;
  char line[512];       /* input line */
  int line_no = 1;
  int in_comment = 0;

  if (NULL == (fp = fopen(write_commands_file_name, "r")))
    return -1;  /*file may not exist - not an error*/
  
  /* Read one line per iteration and parse it */
  while((fgets(line, sizeof(line), fp) != NULL) && (ret_val >= 0)) { 

    /* Skip empty lines */
    if (strlen(line) == 0) {line_no = line_no + 1; continue;} 

    /* Handle comments */
    if (strstr(line, END_COMMENT) != 0)    { in_comment = 0; ++line_no; continue;} /* Note that this match also one-line-comment */
    if (strstr(line, START_COMMENT) != 0)  { in_comment = 1; ++line_no; continue;}  
    if (in_comment) { ++line_no; continue;}


    /* Coverity: We shouldn't fix such strange defects */
    /* coverity[tainted_data] */
    if (strncmp(line, KEYWORD_DB_DIRECT_WRITE_COMMAND, KEYWORD_DB_DIRECT_WRITE_COMMAND_SIZE) == 0)          { db_direct_write_command(line);  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_TCAM_WRITE_COMMAND, KEYWORD_DB_TCAM_WRITE_COMMAND_SIZE) == 0)              { db_tcam_write_command(line);  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_LPM_WRITE_COMMAND, KEYWORD_DB_LPM_WRITE_COMMAND_SIZE) == 0)                { db_lpm_write_command(line);  ++line_no; continue; }
    if (strncmp(line, KEYWORD_DB_EM_WRITE_COMMAND, KEYWORD_DB_EM_WRITE_COMMAND_SIZE) == 0)                  { db_em_write_command(line);  ++line_no; continue; }
    if (strncmp(line, KEYWORD_REG_WRITE_COMMAND, KEYWORD_REG_WRITE_COMMAND_SIZE) == 0)                      { reg_write_command(line);  ++line_no; continue; }
    if (strncmp(line, KEYWORD_PROGRAM_SELECT_CONFIG_WRITE_COMMAND, KEYWORD_PROGRAM_SELECT_CONFIG_WRITE_COMMAND_SIZE) == 0)  { program_select_tcam_write_command(line);  ++line_no; continue; }

    ++line_no;

  } /* End of while loop over all lines in file*/

  fclose(fp);
  return ret_val;
}

int db_direct_write_command(const char* write_command) {             
  char key_word[MAX_NAME_LENGTH], res_value_s[MAX_NAME_LENGTH] ;  /* this contain hex string value in format n'hxxxx*/
  unsigned int db_id, row_ndx, res_len;
  unsigned int* res_value;
  pemladrv_mem_t* result_mem_access;
  unsigned int nof_fields_in_res;
  const FieldBitRange* bit_range;
  int ret_val = 0;

  /*DB_DIRECT_WRITE_COMMAND <db_id> <row_ndx> <result_value>*/
  if (sscanf( write_command, "%s %u %u %s", key_word, &db_id, &row_ndx, res_value_s) != 4) {
    /*printf("Bad ucode line format. Skip and continue with next line\n.");*/
    return -1;
  } else {
    res_value = hexstr2uints(res_value_s, &res_len);
  }

  /*retreive DB info*/
  nof_fields_in_res = api_info.db_direct_container.db_direct_info_arr[db_id].nof_fields;
  bit_range = api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr;
  /*allocate pem_mem_access struct, fill it and write to DB*/
  ret_val |= allocate_pemladrv_mem_struct(&result_mem_access, nof_fields_in_res, bit_range);
  set_pem_mem_accesss_fldbuf(0, res_value, bit_range, result_mem_access);
  ret_val |= pemladrv_direct_write(0, db_id, row_ndx, result_mem_access);

  free_pemladrv_mem_struct(&result_mem_access);
  free(res_value);

  return ret_val;
}

int db_tcam_write_command(const char* write_command) {
  char key_word[MAX_NAME_LENGTH], key_value_s[MAX_NAME_LENGTH], mask_value_s[MAX_NAME_LENGTH], res_value_s[MAX_NAME_LENGTH] ;  /* this contain hex string value in format n'hxxxx*/
  unsigned int db_id, row_ndx, len;
  unsigned int *key_value, *mask_value, *res_value;
  pemladrv_mem_t *key_mem_access, *mask_mem_access, *result_mem_access, *valid_mem_access;
  unsigned int nof_fields_in_res, nof_fields_in_key;
  const FieldBitRange* key_bit_range, *result_bit_range;
  FieldBitRange valid_bit_range;
  int ret_val = 0;
  const unsigned int valid_bit = 1;

  valid_bit_range.lsb = 0;
  valid_bit_range.msb = 0;
  valid_bit_range.is_field_mapped = 1;

  /*DB_TCAM_WRITE_COMMAND <db_id> <row_ndx> <key_value> <mask_value> <result_value>*/
  if (sscanf( write_command, "%s %u %u %s %s %s", key_word, &db_id, &row_ndx, key_value_s, mask_value_s, res_value_s) != 6) {
    /*printf("Bad ucode line format. Skip and continue with next line\n.");*/
    return -1;
  } else {
    key_value = hexstr2uints(key_value_s, &len);
    mask_value = hexstr2uints(mask_value_s, &len);
    res_value = hexstr2uints(res_value_s, &len);
  }

  /*retreive DB info*/
  nof_fields_in_key = api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_key;
  nof_fields_in_res = api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_result;
  key_bit_range =  api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr;
  result_bit_range =  api_info.db_tcam_container.db_tcam_info_arr[db_id].result_field_bit_range_arr;
  /*allocate pem_mem_access struct, fill it and write to DB*/
  ret_val |= allocate_pemladrv_mem_struct(&key_mem_access, nof_fields_in_key, key_bit_range);
  ret_val |= allocate_pemladrv_mem_struct(&mask_mem_access, nof_fields_in_key, key_bit_range);
  ret_val |= allocate_pemladrv_mem_struct(&result_mem_access, nof_fields_in_res, result_bit_range);
  ret_val |= allocate_pemladrv_mem_struct(&valid_mem_access, 1, &valid_bit_range);
  /*fill data and write to design*/
  set_pem_mem_accesss_fldbuf(0, key_value, key_bit_range, key_mem_access); 
  set_pem_mem_accesss_fldbuf(0, mask_value, key_bit_range, mask_mem_access);
  set_pem_mem_accesss_fldbuf(0, res_value, result_bit_range, result_mem_access);
  set_pem_mem_accesss_fldbuf(0, &valid_bit, &valid_bit_range, valid_mem_access);
  ret_val |= pemladrv_tcam_write(0, db_id, row_ndx, key_mem_access, mask_mem_access, valid_mem_access, result_mem_access);
  /*free allocated data*/
  free_pemladrv_mem_struct(&key_mem_access);
  free_pemladrv_mem_struct(&mask_mem_access);
  free_pemladrv_mem_struct(&result_mem_access);
  free_pemladrv_mem_struct(&valid_mem_access);
  free(key_value);
  free(mask_value);
  free(res_value);

  return ret_val;
}

int db_lpm_write_command(const char* write_command) {
  char key_word[MAX_NAME_LENGTH], key_value_s[MAX_NAME_LENGTH], res_value_s[MAX_NAME_LENGTH] ;  /* this contain hex string value in format n'hxxxx*/
  unsigned int db_id, nof_bits_in_prefix, len;
  unsigned int *key_value, *res_value;
  pemladrv_mem_t *key_mem_access, *result_mem_access;
  unsigned int nof_fields_in_res, nof_fields_in_key;
  const FieldBitRange* key_bit_range, *result_bit_range;
  int ret_val = 0;
  int index;

  /*DB_LPM_WRITE_COMMAND <db_id> <key_value> <prefix_len_in_bits> <result_value>*/
  if (sscanf( write_command, "%s %u %s %u %s", key_word, &db_id, key_value_s, &nof_bits_in_prefix, res_value_s) != 5) {
    /*printf("Bad ucode line format. Skip and continue with next line\n.");*/
    return -1;
  } else {
    key_value = hexstr2uints(key_value_s, &len);    
    res_value = hexstr2uints(res_value_s, &len);
  }

  /*retreive DB info*/
  nof_fields_in_key = api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_key;
  nof_fields_in_res = api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_result;
  key_bit_range =  api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_field_bit_range_arr;
  result_bit_range =  api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_field_bit_range_arr;
  /*allocate pem_mem_access struct, fill it and write to DB*/
  ret_val |= allocate_pemladrv_mem_struct(&key_mem_access, nof_fields_in_key, key_bit_range);
  ret_val |= allocate_pemladrv_mem_struct(&result_mem_access, nof_fields_in_res, result_bit_range);
  /*fill data and write to design*/
  set_pem_mem_accesss_fldbuf(0, key_value, key_bit_range, key_mem_access); 
  set_pem_mem_accesss_fldbuf(0, res_value, result_bit_range, result_mem_access);
  ret_val |= pemladrv_lpm_insert(0, db_id, key_mem_access, nof_bits_in_prefix, result_mem_access, &index);
  /*free allocated data*/
  free_pemladrv_mem_struct(&key_mem_access);
  free_pemladrv_mem_struct(&result_mem_access);
  free(key_value);
  free(res_value);

  return ret_val;
}

int db_em_write_command(const char* write_command) {
  char key_word[MAX_NAME_LENGTH], key_value_s[MAX_NAME_LENGTH], res_value_s[MAX_NAME_LENGTH] ;  /* this contain hex string value in format n'hxxxx*/
  unsigned int db_id, len;
  unsigned int *key_value, *res_value;
  pemladrv_mem_t *key_mem_access, *result_mem_access;
  unsigned int nof_fields_in_res, nof_fields_in_key;
  const FieldBitRange* key_bit_range, *result_bit_range;
  int ret_val = 0;
  int index;

  /*DB_EM_WRITE_COMMAND <db_id> <key_value> <result_value>*/
  if (sscanf( write_command, "%s %u %s %s", key_word, &db_id, key_value_s, res_value_s) != 4) {
    /*printf("Bad ucode line format. Skip and continue with next line\n.");*/
    return -1;
  } else {
    key_value = hexstr2uints(key_value_s, &len);    
    res_value = hexstr2uints(res_value_s, &len);
  }

  /*retreive DB info*/
  nof_fields_in_key = api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_key;
  nof_fields_in_res = api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_result;
  key_bit_range =  api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_field_bit_range_arr;
  result_bit_range =  api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_field_bit_range_arr;
  /*allocate pem_mem_access struct, fill it and write to DB*/
  ret_val |= allocate_pemladrv_mem_struct(&key_mem_access, nof_fields_in_key, key_bit_range);
  ret_val |= allocate_pemladrv_mem_struct(&result_mem_access, nof_fields_in_res, result_bit_range);
  /*fill data and write to design*/
  set_pem_mem_accesss_fldbuf(0, key_value, key_bit_range, key_mem_access);
  set_pem_mem_accesss_fldbuf(0, res_value, result_bit_range, result_mem_access);
  ret_val |= pemladrv_em_insert(0, db_id, key_mem_access, result_mem_access, &index);
  /*free allocated data*/
  free_pemladrv_mem_struct(&key_mem_access);
  free_pemladrv_mem_struct(&result_mem_access);
  free(key_value);
  free(res_value);

  return ret_val;
}

int reg_write_command(const char* write_command) {
  char key_word[MAX_NAME_LENGTH], reg_value_s[MAX_NAME_LENGTH] ;  /* this contain hex string value in format n'hxxxx*/
  unsigned int reg_id, len;
  unsigned int* reg_data;
  pemladrv_mem_t* data_mem_access;
  unsigned int nof_fields_in_res;
  const FieldBitRange* bit_range;
  int ret_val = 0;

  /*REG_WRITE_COMMAND <reg_id> <reg_value>*/
  if (sscanf( write_command, "%s %u %s", key_word, &reg_id, reg_value_s) != 3) {
    /*printf("Bad ucode line format. Skip and continue with next line\n.");*/
    return -1;
  } else {
    reg_data = hexstr2uints(reg_value_s, &len);
  }

  /*retreive DB info*/
  nof_fields_in_res = api_info.reg_container.reg_info_arr[reg_id].nof_fields;
  bit_range = api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr;
  /*allocate pem_mem_access struct, fill it and write to DB*/
  ret_val |= allocate_pemladrv_mem_struct(&data_mem_access, nof_fields_in_res, bit_range);
  set_pem_mem_accesss_fldbuf(0, reg_data, bit_range, data_mem_access);
  ret_val |= pemladrv_reg_write(0, reg_id, data_mem_access);

  free_pemladrv_mem_struct(&data_mem_access);
  free(reg_data);

  return ret_val;
}

int program_select_tcam_write_command(const char* write_command) {
  char key_word[MAX_NAME_LENGTH], mem_address_s[MAX_NAME_LENGTH], entry_val_s[MAX_NAME_LENGTH] ;  /* this contain hex string value in format n'hxxxx*/
  unsigned int block_id, value_length, pem_mem_address_val, row_ndx;
  unsigned int* data_value;
  int ret_val = 0;
  phy_mem_t mem_write;

  /*PROGRAM_SELECT_CONFIG_WRITE_COMMAND <address> <row_ndx> <value>*/
  if (sscanf( write_command, "%s %s %u %s", key_word, mem_address_s, &row_ndx, entry_val_s) != 4) {
    /*printf("Bad ucode line format. Skip and continue with next line\n.");*/
    return -1;
  } else {
    pem_mem_address_val = hexstr2addr(mem_address_s, &block_id);
    data_value = hexstr2uints(entry_val_s, &value_length);
  }

  /*fill write info and write to TCAM to config context*/
  mem_write.block_identifier = block_id;
  mem_write.mem_address = pem_mem_address_val;
  mem_write.mem_width_in_bits = value_length;
  mem_write.reserve = 0;
  mem_write.row_index = row_ndx;

#ifdef BCM_DNX_SUPPORT
  ret_val = phy_mem_write(0, &mem_write, data_value);
#endif
  free(data_value);
  return ret_val;
}

/**********************************************************************************************/

