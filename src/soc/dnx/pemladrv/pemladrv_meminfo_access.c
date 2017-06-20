/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include "pemladrv_physical_access.h"
#include "pemladrv_meminfo_access.h"
#include "pemladrv_meminfo_init.h"
#include "pemladrv_logical_access.h"


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

ApiInfo api_info;


/* The following are DBs structures initialization */
/***************************************************/

void db_mapping_info_init(int nof_dbs) {
  int db_ndx;
  api_info.db_direct_container.nof_direct_dbs = nof_dbs;
  api_info.db_direct_container.db_direct_info_arr = (LogicalDirectInfo*)malloc(nof_dbs * sizeof(LogicalDirectInfo));
  /*set mapping matrix to NULL*/
  for (db_ndx = 0; db_ndx < nof_dbs; ++db_ndx){
    api_info.db_direct_container.db_direct_info_arr[db_ndx].result_chunk_mapper_matrix_arr = NULL;
  }
}

void reg_mapping_info_init(int nof_reg) {
  int reg_ndx;
  api_info.reg_container.nof_registers = nof_reg;
  api_info.reg_container.reg_info_arr = (LogicalRegInfo*)malloc(nof_reg * sizeof(LogicalRegInfo));
  /*set reg as unmapped*/
  for (reg_ndx = 0; reg_ndx < nof_reg; ++reg_ndx){
    api_info.reg_container.reg_info_arr[reg_ndx].is_mapped = 0;
  }
}

void tcam_mapping_info_init(int nof_dbs) {
  int db_ndx;
  api_info.db_tcam_container.nof_tcam_dbs = nof_dbs;
  api_info.db_tcam_container.db_tcam_info_arr = (LogicalTcamInfo*)malloc(nof_dbs * sizeof(LogicalTcamInfo));
  /*set mapping matrix to NULL*/
  for (db_ndx = 0; db_ndx < nof_dbs; ++db_ndx){
    api_info.db_tcam_container.db_tcam_info_arr[db_ndx].key_chunk_mapper_matrix_arr = NULL;
    api_info.db_tcam_container.db_tcam_info_arr[db_ndx].result_chunk_mapper_matrix_arr = NULL;
  }
}

void em_mapping_info_init(int nof_dbs) {
  int db_ndx;
  api_info.db_em_container.nof_em_dbs = nof_dbs;
  api_info.db_em_container.db_em_info_arr = (LogicalEmInfo*)malloc(nof_dbs * sizeof(LogicalEmInfo));
  /*set mapping matrix to NULL*/
  for (db_ndx = 0; db_ndx < nof_dbs; ++db_ndx){
    api_info.db_em_container.db_em_info_arr[db_ndx].logical_em_info.key_chunk_mapper_matrix_arr = NULL;
    api_info.db_em_container.db_em_info_arr[db_ndx].logical_em_info.result_chunk_mapper_matrix_arr = NULL;
  }
}

void lpm_mapping_info_init(int nof_dbs) {
  int db_ndx;
  api_info.db_lpm_container.nof_lpm_dbs = nof_dbs;
  api_info.db_lpm_container.db_lpm_info_arr = (LogicalLpmInfo*)malloc(nof_dbs * sizeof(LogicalLpmInfo));
  /*set mapping matrix to NULL*/
  for (db_ndx = 0; db_ndx < nof_dbs; ++db_ndx){
    api_info.db_lpm_container.db_lpm_info_arr[db_ndx].logical_lpm_info.key_chunk_mapper_matrix_arr = NULL;
    api_info.db_lpm_container.db_lpm_info_arr[db_ndx].logical_lpm_info.result_chunk_mapper_matrix_arr = NULL;
  }
}


/**********************************************
*    uCode handling
**********************************************/
 
typedef struct {
  unsigned int  unit;
  unsigned int  mem_block_id;
  unsigned int  mem_addr;
  unsigned int  row_offset;
  unsigned int  *data;
  unsigned int  data_length;
  unsigned int  is_mem;
} ucode_write_instructions_t;
 
 
static ucode_write_instructions_t ucode[UCODE_MAX_NOF_MEM_LINE + UCODE_MAX_NOF_REG_LINE];
 
static unsigned int ucode_instruction_counter;
 
void ucode_init() {
  ucode_instruction_counter = 0;
}
 
void ucode_add(int unit, unsigned int mem_block_id, unsigned int mem_addr, unsigned int index, unsigned int *data, unsigned int data_length, int is_mem) {
 
  assert(data_length < MAX_MEM_DATA_LENGTH);
 
  ucode[ucode_instruction_counter].unit = unit;
  ucode[ucode_instruction_counter].mem_block_id = mem_block_id;
  ucode[ucode_instruction_counter].mem_addr = mem_addr;
  ucode[ucode_instruction_counter].row_offset = index;
  ucode[ucode_instruction_counter].data = data;
  ucode[ucode_instruction_counter].data_length = data_length;
  ucode[ucode_instruction_counter].is_mem = is_mem;
 
  assert(ucode_instruction_counter < UCODE_MAX_NOF_MEM_LINE + UCODE_MAX_NOF_REG_LINE);
  ucode_instruction_counter = ucode_instruction_counter + 1;
}
 
int  ucode_nof_instruction_get() {
  return ucode_instruction_counter;
}
 
int  ucode_instruction_get(unsigned int instruction_no, unsigned int *unit, unsigned int *mem_block_id, unsigned int *mem_addr, unsigned int *row_offset, unsigned int **data, unsigned int *data_length, int *is_mem) {
  assert(instruction_no < ucode_instruction_counter);
  *unit = ucode[instruction_no].unit;
  *mem_block_id = ucode[instruction_no].mem_block_id;
  *mem_addr = ucode[instruction_no].mem_addr;
  *row_offset = ucode[instruction_no].row_offset;
  *data_length = ucode[instruction_no].data_length;
  *data = ucode[instruction_no].data;
  *is_mem = ucode[instruction_no].is_mem;
 
  return 0;
}



void ucode_upload() {
  unsigned int inst;

  for (inst = 0; inst < ucode_instruction_counter; inst = inst + 1) {

  }

}

