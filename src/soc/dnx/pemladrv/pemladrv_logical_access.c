/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <shared/utilex/utilex_integer_arithmetic.h>

#include "pemladrv_physical_access.h"
#include "pemladrv_logical_access.h"
#include "pemladrv_meminfo_init.h"
#include "pemladrv_meminfo_access.h"
#include "pemladrv_cfg_api_defines.h"

#include "soc/register.h"
#include "soc/mem.h"
#include "soc/drv.h"
#include "soc/dnx/pemladrv/pemladrv.h"


#ifndef DB_OBJECT_ACCESS

extern ApiInfo api_info;
extern int cmodel_memreg_access(int unit,
                                int cmic_block,
                                uint32 addr,
                                uint32 dwc_read,
                                int is_mem,
                                int is_get,
                                uint32 *data);


#ifdef _MSC_VER
#if (_MSC_VER >= 1400) 
#pragma warning(push)
#pragma warning(disable: 4996) /* To avoid Microsoft compiler warning (4996 for _CRT_SECURE_NO_WARNINGS equivalent)*/
#endif
#endif

#define SRAM    0
#define TCAM    1

#define DOWN    0
#define UP      1


/* Returns number of cols in CAM chunk mattrix*/
unsigned int get_nof_cam_sram_chunk_cols(const unsigned int total_result_width);

/* Returns number of cols in MAP chunk mattrix*/
unsigned int get_nof_map_sram_chunk_cols(const unsigned int total_result_width);

/* Returns number of cols in MAP chunk mattrix*/
/* this function is for direct chunks only, and made to deal with DBs that      *
 * have sevral fields, have chunks (exept the last one) that occupy only part   *
 * of the physical sram and possibly have offset between chunks (meaning,       * 
 * chunks may be mapped to different adresses and will not follow each other.   *
 * for 'result' in CAM based DBs (where each field 'sits' next to its neighbour *
 * fields) you sould use: get_nof_cam_sram_chunk_cols.                              */
unsigned int get_nof_direct_chunk_cols(LogicalDirectInfo* direct_info);

/* Returns number of rows in MAP chunk mattrix*/
unsigned int get_nof_direct_chunk_rows(const unsigned int nof_entries);

/* Builds an unsigned int array that consists of ones, placed in the bits that needs modification, And an unsigned int array that consists of data from virtual db. used for key (included mask and valid bits)*/
void build_virtual_db_mask_and_data_from_key_fields(const int key_width, const unsigned int total_width, const FieldBitRange* field_bit_range_arr, const pemladrv_mem_t* key, const pemladrv_mem_t* mask, const pemladrv_mem_t* valid, unsigned int **virtual_key_mask, unsigned int **virtual_key_data);

/* Builds an unsigned int array that consists of ones, placed in the bits that needs modification, and an unsigned int array that consists of data from virtual db. Used for result*/
void build_virtual_db_mask_and_data_from_result_fields(const unsigned int total_width, const FieldBitRange* field_bit_range_arr, const pemladrv_mem_t* result,  const int is_init_mask_to_ones_at_field_location, unsigned int **virtual_result_mask, unsigned int **virtual_result_data);

/* Modify virtual_field_mask msb/lsb bits for each field in field_bit_range_arr*/
void build_virtual_db_mask(const unsigned int offset, const FieldBitRange* field_bit_range_arr, const pemladrv_mem_t *data , unsigned int *virtual_field_mask);

/* Build a cache array of a single line in virtual data*/
void build_virtual_db_data(const unsigned int offset, const FieldBitRange* field_bit_range_arr, const pemladrv_mem_t *data , unsigned int *virtual_field_data);

/* Modify only the relevent bits in virtual_field_mask to match msb/lsb offset of relevent field*/
void set_mask_with_ones(const unsigned int msb_bit, const unsigned int lsb_bit, unsigned int *virtual_field_mask);

/* Set the virtual mask in the correct index with ones with the width of msb-lsb. (starting from lsb bit)*/
void set_ones_in_chunk(const unsigned int lsb_bit, const unsigned int msb_bit, const unsigned int virtual_mask_arr_index, unsigned int* virtual_field_mask);

/* Check to see if specific chunk was modified and needs to be written */
int do_chunk_require_writing(const DbChunkMapper* chunk_mapper, const unsigned int *virtual_field_mask);

/*modify physical_memory_entry_data relevant bits from virtual_db_line_input_data_arr*/
void modify_entry_data(const unsigned char flag, 
                       const unsigned char last_chunk, 
                       const unsigned int  chunk_mapping_width, 
                       const unsigned int  chunk_virtual_mem_width_offset,
                       const unsigned int  chunk_physical_mem_width_offset,
                       const int           total_key_width, 
                       const unsigned int  *virtual_db_line_mask_arr, 
                       const unsigned int  *virtual_db_line_input_data_arr, 
                             unsigned int *physical_memory_entry_data);

/* read physical_memory_entry_data relevant bits to virtual_db_line_input_data_arr*/
void reg_read_entry_data(  const unsigned int  chunk_mapping_width, 
                           const unsigned int  chunk_virtual_mem_width_offset,
                           const unsigned int  chunk_physical_mem_width_offset,
                           const unsigned int *physical_memory_entry_data,
                                 unsigned int  *virtual_db_line_mask_arr, 
                                 unsigned int  *virtual_db_line_input_data_arr
                       );


/* Returns pointer to the new or found physical_memory_entry*/
PhysicalWriteInfo* find_or_allocate_and_read_physical_data(const unsigned int chunk_mem_block_id, 
                                                           const unsigned int chunk_phy_mem_addr, 
                                                           const unsigned int chunk_phy_mem_row_index, 
                                                           const unsigned int chunk_phy_mem_width, 
                                                           const unsigned int chunk_mapping_width,
                                                           const int          is_mem,
                                                           PhysicalWriteInfo** target_physical_memory_entry_data_list);


/*Write logical, cam based (TCAM, EM, LPM) DB, to physical*/
int logical_cam_based_db_write(const LogicalTcamInfo* cam_info, 
                               const int unit,  
                               const uint32 row_index, 
                               const pemladrv_mem_t *key, 
                               const pemladrv_mem_t *mask, 
                               const pemladrv_mem_t *valid, 
                               const pemladrv_mem_t *data);

/*Read logical, cam based (TCAM, EM, LPM) DB, from physical*/
int logical_cam_based_db_read(const LogicalTcamInfo* cam_info, const int unit, const uint32 row_index, pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid, pemladrv_mem_t *result);




/* Run over list and write to physical adress*/
void write_all_physical_data_from_list(PhysicalWriteInfo* target_physical_memory_entry_data_curr, int is_mem);

/* Build FieldBitRange for valid bit */
FieldBitRange* produce_valid_field_bit_range(); 

/* Runs over all relevant chunks and updates the physical_memory_data_list with adresses and entry_data*. returns pointer to the list.*/
PhysicalWriteInfo* run_over_all_chunks_read_physical_data_and_update_physical_memory_entry_data_list(const unsigned char flag, const unsigned int virtual_row_index, const unsigned int total_width, const unsigned int chunk_matrix_row, const unsigned int nof_implamentations, const unsigned int nof_chunk_matrix_cols, const LogicalDbChunkMapperMatrix* db_chunk_mapper_matrix, const unsigned int *virtual_mask, const unsigned int *virtual_data);

/* Run over all chunks in db, read physical data and modify the bits in virtual_db_data_arr*/
void build_physical_db_key_data_array(const LogicalDbChunkMapperMatrix* db_chunk_mapper_matrix, const unsigned int nof_chunk_cols, const unsigned int chunk_matrix_row_index, const unsigned int virtual_row_index, const unsigned int total_virtual_key_width, unsigned int *virtual_db_data_arr);

/* Run over all chunks in db, read physical data and modify the bits in virtual_db_data_arr*/
void build_physical_db_result_data_array(const LogicalDbChunkMapperMatrix* db_chunk_mapper_matrix, const unsigned int nof_chunk_cols, const unsigned int chunk_matrix_row_index, const unsigned int virtual_row_index, const FieldBitRange* field_bit_range_arr, pemladrv_mem_t *result, unsigned int *virtual_db_data_arr);

/*Builds (only) key_array from tcam data (including key, mask and valid bits) by key width in bits*/
unsigned int* convert_tcam_data_into_key_array(const int key_width_in_bits, unsigned int*  tcam_data);

/*Builds (only) mask_array from tcam data (including key, mask and valid bits) by key width in bits*/
unsigned int* convert_tcam_data_into_mask_array(const int total_key_width, const unsigned int* tcam_data);

/*calculate prefix length from mask array*/
unsigned int calculate_prefix_from_mask_array(const int total_key_width, const unsigned int* mask_arr);

/* Run over pem_mem_access fields and set the fldbuf by relevant bits in virtual_db_data_array*/
void set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(const unsigned int *virtual_db_data_array, const FieldBitRange* field_bit_range_arr, pemladrv_mem_t *result);

/* Run over pem_mem_access fields and insert the fldbuf from relevant bits in virtual_db_data_array*/
void set_pem_mem_accesss_fldbuf_from_physical_tcam_data_array(const unsigned int *virtual_db_data_array, const unsigned int total_key_width, const FieldBitRange* field_bit_range_arr , pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid);

/*Retrieve prefix from physical tcam entry data*/
unsigned int get_prefix_length_from_physical_data(const int total_key_width, const unsigned int* tcam_data);

/*Update LPM chace from physical memory*/
void update_lpm_cache_from_physical();

/*Update EM chace from physical memory*/
void update_em_cache_from_physical();

/*is db mapped to PEs*/
int is_db_mapped_to_pes(const LogicalDbChunkMapperMatrix*const db_chunk_mapper);


/*result_read_bits_mask - all the bits in the register which are mapped will be ones, unmapped bits will be zero*/
int pemladrv_reg_read_and_retreive_read_bits_mask(int unit,  reg_id_t reg_id, pemladrv_mem_t *result, pemladrv_mem_t *result_read_bits_mask);

static void set_pem_mem_access_with_value(pemladrv_mem_t* mem_access, unsigned int nof_fields, FieldBitRange* bit_range_arr, unsigned int field_val);
static int  compare_pem_mem_access(pemladrv_mem_t* mem_access_a, pemladrv_mem_t* mem_access_b, unsigned int nof_fields, FieldBitRange* bit_range_arr);

/***********************/
/* EM/LPM related func */
/***********************/

/* Check is key was alreay writen in DB. Returns index of key if exists and -1 if not. */
int does_key_exists_in_physical_db(const LogicalTcamInfo* tcam_info, const unsigned char* valids_arr, unsigned int** key_entry_arr, const unsigned int* key);

/* Check is key was alreay writen in DB. Returns index of key if exists and -1 if not. */
int does_key_exists_in_physical_lpm_db(const LogicalTcamInfo* tcam_info, const unsigned int prefix_length_in_bits,const LpmDbCache* lpm_cache, const unsigned int* key);

/*Build pemladrv_mem_t structure for mask, all bits set to ones, equivalent to key fields*/
pemladrv_mem_t* build_em_mask(const FieldBitRange* key_field_bit_range_arr, const pemladrv_mem_t *key);

/*Build pemladrv_mem_t structure for mask, bits in the number of prefix length are set to ones*/
pemladrv_mem_t* build_lpm_mask(const FieldBitRange* key_field_bit_range_arr, int prefix_length_in_bits, const int key_length_in_bits, pemladrv_mem_t *key);

/*Build pemladrv_mem_t structure for valid*/
pemladrv_mem_t* build_tcam_valid();

/*Returns the nof chunks in the fldbuf unsigned int array*/
unsigned int get_nof_chunks_in_fldbuf(const unsigned int field_index, const FieldBitRange* field_bit_range_arr);

/*Returns the nof chunks in the unsigned int array*/
unsigned int get_nof_unsigned_int_chunks(const int field_width);

/*Check to see if two keys are same. Returns TRUE/FALSE*/
unsigned int copmare_two_virtual_keys(const unsigned int key_width_in_bits, const unsigned int* first_key, const unsigned int* second_key);

/* After succsesful writing to db, updating the cache and the next free_index*/
void update_em_cache_and_next_free_index(unsigned int row_index, unsigned int *virtual_key_data, unsigned int *virtual_result_data, LogicalEmInfo* em_cache);

/* After succsesful writing to db, updating the lpm cache*/
void update_lpm_cache(unsigned int row_index, const int prefix_length, unsigned int *virtual_key_data, unsigned int *virtual_result_data, LogicalLpmInfo* lpm_cache);

/*Finds the next free entry in EM db (starting from 0) and returns its index. Returns -1 if DB is full*/
int find_next_em_free_entry(LogicalEmInfo* em_cache);

/*Finds the next free entry in EM db (starting from row_index) towards its direction and returns its index. Returns -1 if DB is full*/
int find_next_lpm_free_entry(const int index, const int direction, unsigned char* physical_entry_occupation);

/*Returns the index of key in physical DB. Returns -1 if key doesn't exists.*/
int find_physical_entry_index_by_key(const LogicalTcamInfo* tcam_info, const unsigned char* valids_arr, unsigned int** key_field_arr, const pemladrv_mem_t *key);

/*Returns the index of key in physical LPM DB. Returns -1 if key doesn't exists.*/
int find_physical_lpm_entry_index_by_key(const LogicalTcamInfo* tcam_info,const LpmDbCache* lpm_cache, const pemladrv_mem_t *key);

/*Copy the key and result field into pem_mem_access structures*/
int get_em_data_by_entry(const int db_id, const int index, pemladrv_mem_t *key, pemladrv_mem_t *result);

/*Copy the key and result field into pem_mem_access structures*/
int get_lpm_data_by_entry(const int db_id, const int index, pemladrv_mem_t *key, pemladrv_mem_t *result);

/*Return the index to insert key*/
int find_lpm_index_to_insert(const unsigned int prefix_length, const int key_length, LpmDbCache* lpm_cache);

/*Returns the best index that requires minimum writings to physical*/
int find_most_suitable_index_with_matching_prefix(const int index, LpmDbCache* lpm_cache);

/*Returns the last index with the same prefix*/
int find_last_index_with_same_prefix(const int index, unsigned int* lpm_key_entry_prefix_length_arr);

/*Check to see is lpm db is already full. Returns 1 if full and 0 otherwise*/
int is_lpm_full(const int nof_entries, unsigned char* physical_entry_occupation);

/*Return UP or DOWN, in the direction that minimum shifts is needed*/
int find_minimum_shifting_direction(const int index, const int table_id);

/*Move key to new entry in db*/
void move_physical_lpm_entry(int unit, const int old_index, const int new_index, const int table_id);

/*Shift all entries in physical by proper direction*/
int shift_physical_to_clear_entry(int unit, const int shift_direction, int *index, const int table_id);

/*Set zeros in key bits according to prefix length*/
void update_key_with_prefix_length(const int prefix_length, const int key_length, unsigned int* virtual_key_data);

/* get LPM entry key and result by index, 
 * Return '0' on success, or number indicating error:
 * - entry not valid 
 */
int pem_lpm_get_by_id (int unit, table_id_t table_id, int index,  pemladrv_mem_t* key, int *length_in_bits, pemladrv_mem_t* result);

/* set LPM entry key and result by index.
 * override existing entry. 
 * Return '0' on success, or number indicating error:
 */
int pem_lpm_set_by_id(int unit, table_id_t table_id, int index, pemladrv_mem_t* key, int *length_in_bits, pemladrv_mem_t* result);



/* Initiate internal structures */
void init_dbs(int nof_dbs);
void init(const char* line);

/*Allocate memory for fldbuf to pem_mem_access by bit range array*/
void pem_allocate_fldbuf_by_bit_range_array(const int nof_fields, const FieldBitRange* bit_range_array, pemladrv_mem_t* pem_mem_access);

/*
 * Return (in 'out_buff') 'nof_bits' bits taken from 'in_buff' starting at 'start_bit_indx'
 *                               ___________________________________>>>>____________________________
 *                              |                                                                   |               
 *                              |                                                                   |                
 *                    __________/\__________                                              __________V__________
 * n                 /                      \                          0                 /                      \  
 * --------------------------------------------------------------------                  ------------------------0
 * |                 |^^^^^^^^^^^^^^^^^^^^^^^|                         |                 |^^^^^^^^^^^^^^^^^^^^^^^|
 * --------------------------------------------------------------------                  -------------------------
 *                   <--  nof_bits --------> ^                                           <----  nof_bits ------>
 *                                           |                                                                     
 *                                    start_bit_indx                                                 
 */
static void get_bits(const unsigned int *in_buff, int start_bit_indx, int nof_bits, unsigned int *out_buff) {
  int out_uint_indx = 0;
  int in_uint_indx = start_bit_indx / 32;
  int n_to_shift = start_bit_indx % 32;
  int nof_written_bits = 0;


  for (; nof_written_bits < nof_bits; out_uint_indx++, in_uint_indx++) {
    /* first copy the '32 - n_to_shift' MSB */
    out_buff[out_uint_indx] = (in_buff[in_uint_indx] >> n_to_shift);
    if (n_to_shift) out_buff[out_uint_indx] = out_buff[out_uint_indx] | (in_buff[in_uint_indx + 1] << (32 - n_to_shift));
    nof_written_bits =  nof_written_bits + 32;
  }
}

/*
 * Modify the out_buff, starting at 'start_bit_indx' with 'nof_bits' from 'in_buff'
 *        
 *                               ___________________________________>>>>________________________________________
 *                               |                                                                              |               
 *                               |                                                                              |                
 *                    __________/\__________                                                          __________V__________
 * n                 /                      \                          0            t                /                      \                           0
 * --------------------------------------------------------------------             ---------------------------------------------------------------------
 * |                 |^^^^^^^^^^^^^^^^^^^^^^^|                         |            | NOT CHANGED    |^^^^^^^^^^^^^^^^^^^^^^^|   NOT CHANGED             |
 * --------------------------------------------------------------------             ---------------------------------------------------------------------
 *                   <----  nof_bits ------- ^                                                       <----  nof_bits ----->  ^
 *                                           |                                                                               |
 *                                 in_buff_start_bit_indx                                                            out_buff_start_bit_indx
 *              
 */
static void set_bits(const unsigned int *in_buff, int in_buff_start_bit_indx, int out_buff_start_bit_indx, int nof_bits, unsigned int *out_buff) {
  int in_uint_indx = 0;
  int out_uint_indx = out_buff_start_bit_indx / 32;
  int n_to_shift = out_buff_start_bit_indx % 32;
  int nof_written_bits = 0;
  unsigned int local_buff[1024] = {0};
  unsigned int out_buff_mask = 0xffffffff;
  /*unsigned int out_buff_mask2 = 0xffffffff;*/

  if (nof_bits == 0) /*in this case, no bits to set*/
    return;

  /* first copy the align in_buff (according to its start bit) to local_buff */

  get_bits(in_buff, in_buff_start_bit_indx, nof_bits, local_buff);

  /* Now the in-buff is in the local-buff shifted to start at the start-bit */

  if ((nof_bits + n_to_shift) < 32) {
    /* In case it is few bits (less then 31) that are in the middle of the 32 bits of an int, reset the relevant bit */

    /* first locate (in local_buff) the input bits (local_buff) in the place (index) they should be in the target */
    unsigned int mask = (1 << nof_bits) - 1;
    local_buff[0] = (local_buff[0] & mask) << n_to_shift;

    /* now reset the bits in the target (out_buff) that should be modified. Do it by using  a mask */
    out_buff_mask = (out_buff_mask << (32 - nof_bits)) >> (32 - (nof_bits + n_to_shift));
    out_buff[out_uint_indx] = out_buff[out_uint_indx] & (~out_buff_mask);

    /* Now combine (bit or) the input data and the target */
    out_buff[out_uint_indx] = out_buff[out_uint_indx] | local_buff[0];

    return;
  }


  /* Copy 'nof_bits' from local-buf (starting at 0) to the out buf starting at location 'out_buff_start_bit_indx' */
  for (; nof_written_bits < nof_bits; out_uint_indx++, in_uint_indx++) {
    /* first copy the 'n_to_shift' MSB */
    if (n_to_shift > 0) {
      out_buff[out_uint_indx] = (out_buff[out_uint_indx] & (out_buff_mask >> (32 - n_to_shift))) | (local_buff[in_uint_indx] << n_to_shift);
    } else {
      if (nof_bits - nof_written_bits < 32 ) {
        unsigned int mask = (1 << nof_bits) - 1;
        local_buff[in_uint_indx] = (local_buff[in_uint_indx] & mask) << n_to_shift;

        /* now reset the bits in the target (out_buff) that should be modified. Do it by using  a mask */
        out_buff_mask = (out_buff_mask << (32 - nof_bits)) >> (32 - (nof_bits + n_to_shift));
        out_buff[out_uint_indx] = out_buff[out_uint_indx] & (~out_buff_mask);

        /* Now combine (bit or) the input data and the target */
        out_buff[out_uint_indx] = out_buff[out_uint_indx] | local_buff[in_uint_indx];
        return;
      }
      out_buff[out_uint_indx] = local_buff[in_uint_indx];
    }
    nof_written_bits =  nof_written_bits + (32 - n_to_shift);

    if ((n_to_shift > 0) && (nof_written_bits < nof_bits)) {
      const unsigned int nof_remaining_bits = (nof_bits - nof_written_bits);
      unsigned int local_buff_mask;
      /* Now copy the '32 - n_to_shift' LSB*/
      /*const unsigned int prev_out_buff_mask2 = out_buff_mask2;*/
      out_buff_mask = (out_buff_mask << (nof_bits - (32 - n_to_shift)));
      local_buff_mask =  (nof_remaining_bits > 31) ? 0xffffffff : (~out_buff_mask) ;
      out_buff[out_uint_indx + 1] = (out_buff[out_uint_indx + 1] & (out_buff_mask)) | ((local_buff[in_uint_indx] >> (32 - n_to_shift)) & local_buff_mask);
      nof_written_bits = nof_written_bits + n_to_shift;
      out_buff_mask = 0xffffffff;
    }

  }
}


typedef struct {
  int           unit;
  phy_mem_t     chunk_info;
  unsigned int  data[32]; /* Assume memory max width less then 1024 bits */
} cache_entry_t;



int pemladrv_direct_write(int unit, table_id_t db_id, uint32 row_index, pemladrv_mem_t *data) {
  PhysicalWriteInfo* target_result_physical_memory_entry_data_list = NULL;
  unsigned int chunk_matrix_row            ;
  unsigned int nof_implamentations         ;
  unsigned int nof_result_chunk_matrix_cols;
    /*VIRTUAL MASK*/
  unsigned int *virtual_result_mask = NULL; 
  /*VIRTUAL ENTRY DATA*/
  unsigned int *virtual_result_data = NULL; 

  /*check id db was mapped in the application*/
  if (api_info.db_direct_container.db_direct_info_arr == NULL || api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr == NULL) {
    /*printf("Warning: PEM Db-id %d was NOT mapped during the application run. Unable to write.\n", db_id);*/
    return 0;
}
  
  chunk_matrix_row             = row_index / PEM_CFG_API_MAP_CHUNK_N_ENTRIES;
  nof_implamentations          = api_info.db_direct_container.db_direct_info_arr[db_id].nof_chunk_matrices;
  nof_result_chunk_matrix_cols = get_nof_direct_chunk_cols(&api_info.db_direct_container.db_direct_info_arr[db_id]);


  build_virtual_db_mask_and_data_from_result_fields(api_info.db_direct_container.db_direct_info_arr[db_id].total_result_width, api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr, data, 1, &virtual_result_mask, &virtual_result_data);

  target_result_physical_memory_entry_data_list = run_over_all_chunks_read_physical_data_and_update_physical_memory_entry_data_list(0, row_index, 
                                                                                                                                       api_info.db_direct_container.db_direct_info_arr[db_id].total_result_width,
                                                                                                                                       chunk_matrix_row,
                                                                                                                                       nof_implamentations,
                                                                                                                                       nof_result_chunk_matrix_cols, 
                                                                                                                                       api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr, 
                                                                                                                                       virtual_result_mask,
                                                                                                                                       virtual_result_data);
  write_all_physical_data_from_list(target_result_physical_memory_entry_data_list, 1);
  free(virtual_result_mask);
  free(virtual_result_data);

  return 0;
}


int pemladrv_direct_read(int unit, table_id_t db_id, uint32 row_index, pemladrv_mem_t *result) {

  const unsigned int total_result_width_in_bits = api_info.db_direct_container.db_direct_info_arr[db_id].total_result_width; 
  unsigned int       *virtual_db_data_array     = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_result_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));
  unsigned int       chunk_matrix_row_index     = row_index / PEM_CFG_API_MAP_CHUNK_N_ENTRIES;
  
  build_physical_db_result_data_array(api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr, get_nof_direct_chunk_cols(&api_info.db_direct_container.db_direct_info_arr[db_id]), chunk_matrix_row_index, row_index, api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr , result, virtual_db_data_array);
  
  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(virtual_db_data_array, api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr, result);

  free(virtual_db_data_array);
  return 0;
}


int pemladrv_tcam_write(int unit, tcam_id_t tcam_id, uint32 row_index, pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid, pemladrv_mem_t *data) {
  return logical_cam_based_db_write(&api_info.db_tcam_container.db_tcam_info_arr[tcam_id], unit, row_index, key, mask, valid, data);
}


int logical_cam_based_db_write(const LogicalTcamInfo* cam_info, const int unit, const uint32 row_index, const pemladrv_mem_t *key, const pemladrv_mem_t *mask, const pemladrv_mem_t *valid, const pemladrv_mem_t *data) {
  PhysicalWriteInfo* target_key_physical_memory_entry_data_list    = NULL;
  PhysicalWriteInfo* target_result_physical_memory_entry_data_list = NULL;

  unsigned int tcam_key_width;

  unsigned int chunk_matrix_row;
  unsigned int nof_implamentations;

  unsigned int nof_key_chunk_matrix_cols    ;
  unsigned int nof_result_chunk_matrix_cols ;

  /*VIRTUAL MASK*/
  unsigned int *virtual_key_mask    = NULL;
  unsigned int *virtual_result_mask = NULL; 

  /*VIRTUAL ENTRY DATA*/
  unsigned int *virtual_key_data    = NULL;
  unsigned int *virtual_result_data = NULL; 
  
   /*check if db was mapped in the application*/
  if (cam_info->result_chunk_mapper_matrix_arr == NULL || cam_info->result_chunk_mapper_matrix_arr->db_chunk_mapper == NULL) {
    /*printf("Warning: PEM Db was NOT mapped during the application run. Unable to write.\n");*/
    return 0;
  }

  target_key_physical_memory_entry_data_list    = NULL;
  target_result_physical_memory_entry_data_list = NULL;

  tcam_key_width        = cam_info->total_key_width;

  chunk_matrix_row      = row_index / PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES;          /*Should always be 0*/
  nof_implamentations   = cam_info->nof_chunk_matrices;

  nof_key_chunk_matrix_cols    = get_nof_tcam_chunk_cols(cam_info->total_key_width);
  nof_result_chunk_matrix_cols = get_nof_cam_sram_chunk_cols(cam_info->total_result_width);


  build_virtual_db_mask_and_data_from_key_fields(tcam_key_width, 2*tcam_key_width + 1,  cam_info->key_field_bit_range_arr, key, mask, valid, &virtual_key_mask, &virtual_key_data);
  build_virtual_db_mask_and_data_from_result_fields(cam_info->total_result_width, cam_info->result_field_bit_range_arr, data, 1, &virtual_result_mask, &virtual_result_data);

  /*KEY/MASK/VALID*/
  target_key_physical_memory_entry_data_list =  run_over_all_chunks_read_physical_data_and_update_physical_memory_entry_data_list(1, row_index, 
                                                                                                                                     cam_info->total_key_width,
                                                                                                                                     chunk_matrix_row,
                                                                                                                                     nof_implamentations,
                                                                                                                                     nof_key_chunk_matrix_cols, 
                                                                                                                                     cam_info->key_chunk_mapper_matrix_arr, 
                                                                                                                                     virtual_key_mask,
                                                                                                                                     virtual_key_data);

  /*RESULT*/
  target_result_physical_memory_entry_data_list = run_over_all_chunks_read_physical_data_and_update_physical_memory_entry_data_list(0, row_index, 
                                                                                                                                       cam_info->total_result_width,
                                                                                                                                       chunk_matrix_row,
                                                                                                                                       nof_implamentations,
                                                                                                                                       nof_result_chunk_matrix_cols, 
                                                                                                                                       cam_info->result_chunk_mapper_matrix_arr, 
                                                                                                                                       virtual_result_mask,
                                                                                                                                       virtual_result_data);

  write_all_physical_data_from_list(target_key_physical_memory_entry_data_list, 1);
  write_all_physical_data_from_list(target_result_physical_memory_entry_data_list, 1);
  free(virtual_result_mask);
  free(virtual_result_data);
  free(virtual_key_mask);
  free(virtual_key_data);
  
  return 0;
}


int pemladrv_tcam_read(int unit,  tcam_id_t tcam_id, uint32 row_index, pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid, pemladrv_mem_t *result) {
  return logical_cam_based_db_read(&api_info.db_tcam_container.db_tcam_info_arr[tcam_id], unit, row_index, key, mask, valid, result);
}

int logical_cam_based_db_read(const LogicalTcamInfo* cam_info, const int unit, const uint32 row_index, pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid, pemladrv_mem_t *result) {

  unsigned int total_key_width_in_bits    = 2 * cam_info->total_key_width + 1;  /*Key + Mask + Valid*/
  unsigned int total_result_width_in_bits = cam_info->total_result_width;
  unsigned int total_key_width            = cam_info->total_key_width;
  unsigned int chunk_matrix_row_index     = row_index / PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES;

  unsigned int *virtual_db_key_data_array    = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));
  unsigned int *virtual_db_result_data_array = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_result_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));

  build_physical_db_key_data_array(cam_info->key_chunk_mapper_matrix_arr, get_nof_tcam_chunk_cols(total_key_width), chunk_matrix_row_index, row_index, total_key_width, virtual_db_key_data_array);
  build_physical_db_result_data_array(cam_info->result_chunk_mapper_matrix_arr, get_nof_cam_sram_chunk_cols(total_result_width_in_bits) ,chunk_matrix_row_index, row_index, cam_info->result_field_bit_range_arr, result, virtual_db_result_data_array);
  

  set_pem_mem_accesss_fldbuf_from_physical_tcam_data_array(virtual_db_key_data_array, cam_info->total_key_width, cam_info->key_field_bit_range_arr, key, mask, valid);
  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(virtual_db_result_data_array, cam_info->result_field_bit_range_arr, result);

  free(virtual_db_key_data_array);
  free(virtual_db_result_data_array);

  return 0;
}


/* Runs over all relevant chunks and updates the physical_memory_data_list with adresses and entry_data*. returns pointer to the list.*/
PhysicalWriteInfo* reg_field_run_over_all_chunks_read_physical_data_and_update_physical_memory_entry_data_list(
            LogicalRegFieldInfo*  reg_field_info_pnt,
            FieldBitRange*        reg_field_bit_range,
            const unsigned int    *virtual_mask, 
            const unsigned int    *virtual_data,
            PhysicalWriteInfo*    target_physical_memory_entry_data_list) {

            PhysicalWriteInfo* target_physical_memory_entry_data_list_curr_element = NULL;
            unsigned int mapping_ndx;
   
            for (mapping_ndx=0;mapping_ndx<reg_field_info_pnt->nof_mappings;++mapping_ndx) {
              RegFieldMapper* reg_field_mapper = &reg_field_info_pnt->reg_field_mapping_arr[mapping_ndx];
   
   
               target_physical_memory_entry_data_list_curr_element =   find_or_allocate_and_read_physical_data(PEM_BLOCK_ID,
                                                      reg_field_mapper->pem_mem_address,
                                                      reg_field_mapper->pem_mem_line,
                                                      reg_field_mapper->pem_mem_total_width,
                                                      reg_field_mapper->cfg_mapper_width,
                                                      0,
                                                      &target_physical_memory_entry_data_list);
                  
                modify_entry_data(0, 0, /* first 0 - writing to SRAM, second 0 - last chunk is relevant only for tcam*/
                                  reg_field_mapper->cfg_mapper_width, 
                                  reg_field_bit_range->lsb + reg_field_mapper->virtual_field_lsb, /*offset in the structure plus internal offset int the field*/
                                  reg_field_mapper->pem_mem_offset,
                                  0,  /* full virtual reg width is irrelevant here (only for tcam)*/
                                  virtual_mask, 
                                  virtual_data, 
                                  target_physical_memory_entry_data_list_curr_element->entry_data);                                                     
   
              }
   
              return target_physical_memory_entry_data_list;
}


/* Runs over all relevant chunks and updates the physical_memory_data_list with adresses and entry_data*. returns pointer to the list.*/
PhysicalWriteInfo* reg_field_run_over_all_chunks_read_physical_data_to_virtual_data(
            LogicalRegFieldInfo*  reg_field_info_pnt,
            FieldBitRange*        reg_field_bit_range,
            unsigned int    *virtual_mask, 
            unsigned int    *virtual_data,
            PhysicalWriteInfo*    target_physical_memory_entry_data_list) {

            PhysicalWriteInfo* target_physical_memory_entry_data_list_curr_element = NULL;
            unsigned int mapping_ndx;
   
            for (mapping_ndx=0;mapping_ndx<reg_field_info_pnt->nof_mappings;++mapping_ndx) {
              RegFieldMapper* reg_field_mapper = &reg_field_info_pnt->reg_field_mapping_arr[mapping_ndx];
   
   
               target_physical_memory_entry_data_list_curr_element =   find_or_allocate_and_read_physical_data(PEM_BLOCK_ID,
                                                      reg_field_mapper->pem_mem_address,
                                                      reg_field_mapper->pem_mem_line,
                                                      reg_field_mapper->pem_mem_total_width,
                                                      reg_field_mapper->cfg_mapper_width,
                                                      0,
                                                      &target_physical_memory_entry_data_list);
                  
                reg_read_entry_data(reg_field_mapper->cfg_mapper_width,
                                    reg_field_bit_range->lsb + reg_field_mapper->virtual_field_lsb,
                                    reg_field_mapper->pem_mem_offset,
                                    target_physical_memory_entry_data_list_curr_element->entry_data,
                                    virtual_mask, 
                                    virtual_data);                                                ;
   
              }
   
              return target_physical_memory_entry_data_list;
}


int pemladrv_reg_write(int unit, reg_id_t reg_id, pemladrv_mem_t *data) {
  unsigned int field_ndx;

  PhysicalWriteInfo* target_reg_physical_memory_entry_data_list    = NULL;

  const unsigned int register_total_width        = api_info.reg_container.reg_info_arr[reg_id].register_total_width;
  const unsigned int nof_fields                  = api_info.reg_container.reg_info_arr[reg_id].nof_fields;

  /*VIRTUAL MASK & DATA*/
  unsigned int *virtual_reg_mask    = NULL;
  unsigned int *virtual_reg_data    = NULL; 
  
  build_virtual_db_mask_and_data_from_result_fields(register_total_width, api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr, data, 1, &virtual_reg_mask, &virtual_reg_data);


  for(field_ndx=0; field_ndx < nof_fields;++field_ndx) {
    LogicalRegFieldInfo* reg_field_info_pnt     = &(api_info.reg_container.reg_info_arr[reg_id].reg_field_info_arr[field_ndx]);
    FieldBitRange*       reg_field_bit_range    = &(api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr[field_ndx]);
    target_reg_physical_memory_entry_data_list 
        = reg_field_run_over_all_chunks_read_physical_data_and_update_physical_memory_entry_data_list(reg_field_info_pnt,
                                                                                                      reg_field_bit_range,
                                                                                                      virtual_reg_mask, 
                                                                                                      virtual_reg_data,
                                                                                                      target_reg_physical_memory_entry_data_list);
  }

  write_all_physical_data_from_list(target_reg_physical_memory_entry_data_list, 0);

  if(virtual_reg_mask != NULL)
    free(virtual_reg_mask);
  if(virtual_reg_data != NULL)
    free(virtual_reg_data);
  return 0;
}


int pemladrv_reg_read(int unit,  reg_id_t reg_id, pemladrv_mem_t *result) {
  pemladrv_mem_t *result_read_bits_mask;
  pemladrv_mem_t *expected_result_read_bits_mask_when_all_mapped;
  int is_result_mask_equal_to_expected_when_all_mapped;
  int nof_fields                         = api_info.reg_container.reg_info_arr[reg_id].nof_fields;
  FieldBitRange* reg_field_bit_range_arr = api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr;
  allocate_pemladrv_mem_struct(&result_read_bits_mask, nof_fields, api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr);
  pemladrv_reg_read_and_retreive_read_bits_mask(unit,  reg_id, result, result_read_bits_mask);

  /*init expected value to all Ones*/
  allocate_pemladrv_mem_struct(&expected_result_read_bits_mask_when_all_mapped, nof_fields, reg_field_bit_range_arr);
  set_pem_mem_access_with_value(expected_result_read_bits_mask_when_all_mapped, nof_fields, reg_field_bit_range_arr, 0xffffffff);

  is_result_mask_equal_to_expected_when_all_mapped = compare_pem_mem_access(result_read_bits_mask, expected_result_read_bits_mask_when_all_mapped, nof_fields, reg_field_bit_range_arr);
  free(result_read_bits_mask);
  free(expected_result_read_bits_mask_when_all_mapped);
  return is_result_mask_equal_to_expected_when_all_mapped ? 0 : PEM_WARNING_READ_PARTIALLY_MAPPED_REGISTER;
}


/*result_read_bits_mask - all the bits in the register which are mapped will be ones, unmapped bits will be zero*/
int pemladrv_reg_read_and_retreive_read_bits_mask(int unit,  reg_id_t reg_id, pemladrv_mem_t *result, pemladrv_mem_t *result_read_bits_mask) {
  unsigned int field_ndx;

  PhysicalWriteInfo* target_reg_physical_memory_entry_data_list    = NULL;

  const unsigned int register_total_width        = api_info.reg_container.reg_info_arr[reg_id].register_total_width;
  const unsigned int nof_fields                  = api_info.reg_container.reg_info_arr[reg_id].nof_fields;

  /*VIRTUAL MASK & DATA*/
  unsigned int *virtual_reg_mask    = NULL;
  unsigned int *virtual_reg_data    = NULL; 
  
  build_virtual_db_mask_and_data_from_result_fields(register_total_width, api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr, result, 0, &virtual_reg_mask, &virtual_reg_data);


  for(field_ndx=0; field_ndx < nof_fields;++field_ndx) {
    LogicalRegFieldInfo* reg_field_info_pnt     = &(api_info.reg_container.reg_info_arr[reg_id].reg_field_info_arr[field_ndx]);
    FieldBitRange*       reg_field_bit_range    = &(api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr[field_ndx]);
    target_reg_physical_memory_entry_data_list 
        = reg_field_run_over_all_chunks_read_physical_data_to_virtual_data(reg_field_info_pnt,
                                                                           reg_field_bit_range,
                                                                           virtual_reg_mask, 
                                                                           virtual_reg_data,
                                                                           target_reg_physical_memory_entry_data_list);
  }

   set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(virtual_reg_data, api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr, result);
   if (result_read_bits_mask != 0) {
    set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(virtual_reg_mask, api_info.reg_container.reg_info_arr[reg_id].reg_field_bit_range_arr, result_read_bits_mask);
   }

  if(virtual_reg_mask != NULL)
    free(virtual_reg_mask);
  if(virtual_reg_data != NULL)
    free(virtual_reg_data);
  return 0;
}

extern int parse_meminfo_definition_file(const int restore_after_reset, const char *file_name); /* This one is implemented in pemladrv_meminfo_init.c */

int pemladrv_init(const int restore_after_reset, const char *file_name) {

  const char *fname;
  unsigned int ret_val = 0;
  unsigned int ucode_inst;
  unsigned int ucode_nof_inst;
  unsigned int unit = 0;
  unsigned int *data = NULL;
  phy_mem_t phy_mem = {0};
  int is_mem;
  unsigned int read_config_for_debug[32+1] = {0};
  int entry_ndx = 0, nof_entries;
  unsigned int nof_bits_in_last_entry, mask;

  if (file_name != NULL) {
    fname = file_name;
  } else {
    fname = PEM_DEFAULT_DB_MEMORY_MAP_FILE;
  }

  /*init pem_applets databse - and free old stuff from previous application*/
  init_pem_applets_db();
  /* Load (and parse) virtual DB definitions and uCode */
  ret_val = parse_meminfo_definition_file(restore_after_reset, fname);
  if (UINT_MAX == ret_val) {
    return UINT_MAX;
  } 

  /*in case init was needed after CPU reset*/
  if (restore_after_reset)
    return 0;

  /* Upload uCode to the device */
  ucode_nof_inst = ucode_nof_instruction_get();
  /*printf("nof UCODE instructions is %d\n", ucode_nof_inst);*/
  for (ucode_inst = 0; ucode_inst < ucode_nof_inst; ucode_inst = ucode_inst + 1) {
    ucode_instruction_get(ucode_inst, &unit, &phy_mem.block_identifier, &phy_mem.mem_address, &phy_mem.row_index, &data, &phy_mem.mem_width_in_bits, &is_mem);
    if (pem_write(unit, &phy_mem, is_mem, data)) {
      /*printf("phy_mem_write() ERROR!\n");   */
      ret_val = 1;
    }
    /*PEMA registers and LOAD/UPDATE memories are problematic anyway. Verify only writes to PEM block*/
    if (phy_mem.block_identifier != PEM_BLOCK_ID) 
      continue;
    /*check if written data is equal to read data*/
    nof_entries = ( phy_mem.mem_width_in_bits % (8*sizeof(unsigned)) == 0) ? (phy_mem.mem_width_in_bits / (8*sizeof(unsigned))) : (phy_mem.mem_width_in_bits / (8*sizeof(unsigned)) + 1);
    /*last entry of read may contain previous bits and thus seem like an error. Therefore zero it*/
    nof_bits_in_last_entry = phy_mem.mem_width_in_bits % (8*sizeof(unsigned));
    mask = (1 << nof_bits_in_last_entry) - 1;   
    pem_read(unit, &phy_mem, is_mem, read_config_for_debug);
    /*mask bits over phy_mem.mem_width_in_bits*/
    if (nof_entries*(8*sizeof(unsigned)) != phy_mem.mem_width_in_bits)
      read_config_for_debug[nof_entries-1] &= mask;
    for (entry_ndx=0; entry_ndx < nof_entries; ++entry_ndx){
      if (read_config_for_debug[entry_ndx] != data[entry_ndx]){        
        ret_val = 1;
        /*printf("phy_mem_write() ERROR! block_id=%x, address=%x, row_ndx=%d\n", phy_mem.block_identifier, phy_mem.mem_address, phy_mem.row_index);   */
        print_debug_pem_read_write("\tWriting: ", phy_mem.mem_address, phy_mem.mem_width_in_bits, phy_mem.row_index, data);
        print_debug_pem_read_write("\treading: ", phy_mem.mem_address, phy_mem.mem_width_in_bits, phy_mem.row_index, read_config_for_debug);
        break; 
      }     
    }    
       
  }


  if (!ret_val){
    /*printf("pem init completed without errors.\n");*/
  }
  else{
    /*printf("pem init FAILED!\n");*/
  }
  return ret_val;

} /* end of pemladrv_meminfo_init() */

int init_pem_applets_db() {  
  int mem_ndx, cache_ndx;
  unsigned int cache_entry;  
  int nof_previously_allocated_mems = api_info.applet_info.meminfo_curr_array_size;
  
  for (mem_ndx = 0; mem_ndx < nof_previously_allocated_mems; ++mem_ndx) {    
    if (api_info.applet_info.meminfo_array_for_applet[mem_ndx] == NULL)
      continue;
    free(api_info.applet_info.meminfo_array_for_applet[mem_ndx]);
    api_info.applet_info.meminfo_array_for_applet[mem_ndx] = NULL;
  }
  if (api_info.applet_info.meminfo_array_for_applet != NULL)
    free(api_info.applet_info.meminfo_array_for_applet);

  for (cache_ndx = 0; cache_ndx < 2; ++cache_ndx) {
    for (cache_entry = 0; cache_entry < api_info.applet_info.nof_pending_applets[cache_ndx]; ++cache_entry){
      if (api_info.applet_info.applet_mem_cache[cache_ndx][cache_entry] != NULL) {
        free(api_info.applet_info.applet_mem_cache[cache_ndx][cache_entry]);
        api_info.applet_info.applet_mem_cache[cache_ndx][cache_entry] = NULL;
      }
    }
    api_info.applet_info.nof_pending_applets[cache_ndx] = 0;
  }  
  api_info.applet_info.meminfo_curr_array_size = 0;
  api_info.applet_info.currently_writing_applets_bit = 0;

  return 0;
}

/* Initiate internal structures */
void init_all_db_arr_by_size(const char* line) {
  int nof_reg, nof_direct_dbs, nof_tcam_dbs, nof_em_dbs, nof_lpm_dbs;
  char key_word[MAX_NAME_LENGTH];
  sscanf( line, "%s %d %d %d %d %d",key_word, &nof_direct_dbs, &nof_tcam_dbs, &nof_em_dbs, &nof_lpm_dbs, &nof_reg);

  db_mapping_info_init(nof_direct_dbs);
  reg_mapping_info_init(nof_reg);
  tcam_mapping_info_init(nof_tcam_dbs);
  em_mapping_info_init(nof_em_dbs);
  lpm_mapping_info_init(nof_lpm_dbs);
  return;
}


/*************************************/
/* EM (Exact Match) access functions */
/*************************************/

int pemladrv_em_insert(int unit, table_id_t table_id,  pemladrv_mem_t* key,  pemladrv_mem_t* result, int *index) {
            
  /*unsigned int              row_index                   = (*index);*/
  unsigned int              nof_fields_in_key           = api_info.db_em_container.db_em_info_arr[table_id].logical_em_info.nof_fields_in_key; 
  const unsigned int        total_key_width_in_bits     = api_info.db_em_container.db_em_info_arr[table_id].logical_em_info.total_key_width;
  const FieldBitRange*const key_fields_arr              = api_info.db_em_container.db_em_info_arr[table_id].logical_em_info.key_field_bit_range_arr;
  int                       next_free_index             = api_info.db_em_container.db_em_info_arr[table_id].em_cache.next_free_index;

  unsigned int *virtual_key_data = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));
 
  /*Check to see if all key fields are available*/
  if (key->nof_fields != nof_fields_in_key) {
    /*printf("ERROR. One or More fields are missing while trying to write key. Exact-Match - #%d id\n", table_id); */
    free(virtual_key_data);
    return 1;/*return error of bad key fields*/
  }

  /*Check to see if EM is already full*/
  if (next_free_index == -1) {
    /*printf("The DB is already full, Can't write or add keys.\n");*/
    free(virtual_key_data);
    return 1;/*return error of full db*/
  }

  build_virtual_db_data(0, key_fields_arr, key, virtual_key_data);    /*needed only to search for existing key in db*/

  *index = does_key_exists_in_physical_db(&api_info.db_em_container.db_em_info_arr[table_id].logical_em_info, api_info.db_em_container.db_em_info_arr[table_id].em_cache.physical_entry_occupation, api_info.db_em_container.db_em_info_arr[table_id].em_cache.em_key_entry_arr, virtual_key_data);
  if (*index != -1) {
    /*printf("Key already exists. Key index is %d\n", *index);*/
    free(virtual_key_data);
    return 1;/*return error of key already existing*/
  }
  free(virtual_key_data);
  /*writing the data to 'next_free_index'*/
  *index = next_free_index;
  return pemladrv_em_entry_set_by_id(unit, table_id, next_free_index, key, result);
}


int pemladrv_em_remove(int unit, table_id_t table_id,  pemladrv_mem_t* key, int *index) {
  
  *index = find_physical_entry_index_by_key(&api_info.db_em_container.db_em_info_arr[table_id].logical_em_info, api_info.db_em_container.db_em_info_arr[table_id].em_cache.physical_entry_occupation, api_info.db_em_container.db_em_info_arr[table_id].em_cache.em_key_entry_arr, key);
  if (*index == -1) {
    /*printf("Key doesn't exist in DB\n");*/
    return 1; /*RETURN KEY NOT EXIST ERROR*/
  }
  return pemladrv_em_remove_by_index(unit, table_id, *index);
}


int pemladrv_em_lookup(int unit, table_id_t table_id,  pemladrv_mem_t* key, pemladrv_mem_t* result, int *index) {

  *index = find_physical_entry_index_by_key(&api_info.db_em_container.db_em_info_arr[table_id].logical_em_info, api_info.db_em_container.db_em_info_arr[table_id].em_cache.physical_entry_occupation, api_info.db_em_container.db_em_info_arr[table_id].em_cache.em_key_entry_arr, key);
  if (*index == -1) {
    /*printf("Key doesn't exist in DB\n");*/
    return 1; /*RETURN KEY NOT EXIST ERROR*/
  }
  return get_em_data_by_entry(table_id, *index, key, result);
}


int pemladrv_em_get_next_index(int unit, table_id_t table_id, int* index) {
  *index = api_info.db_em_container.db_em_info_arr[table_id].em_cache.next_free_index;
  return 0;
}

int pemladrv_em_entry_get_by_id(int unit, table_id_t table_id,  int index,  pemladrv_mem_t* key, pemladrv_mem_t* result) {

  if (index == -1) {
    /*printf("Invalid entry\n");*/
    return 1; /*RETURN INVALID ENTRY ERROR*/
  }
  return get_em_data_by_entry(table_id, index, key, result);
}


int pemladrv_em_entry_set_by_id(int unit, table_id_t table_id,  int index, pemladrv_mem_t* key, pemladrv_mem_t* result) {
  const FieldBitRange* key_fields_arr;
  const FieldBitRange* result_fields_arr;
  unsigned int         total_key_width_in_bits;
  unsigned int         total_result_width_in_bits;
  unsigned int*        virtual_key_data;
  unsigned int*        virtual_result_data ;
  pemladrv_mem_t     *mask, *valid;

  if (index <0 || index > 31) {
    /*printf("Invalid entry\n");*/
    return 1;/*RETURN INVALID ENTRY ERROR*/
  }

  if (api_info.db_em_container.db_em_info_arr[table_id].em_cache.physical_entry_occupation[index] == 1) {
    /*printf("Error. Entry is already occupied.\n");*/
    return 1;/*RETURN ENTRY OCCUPIED ERROR*/
  }

  key_fields_arr              = api_info.db_em_container.db_em_info_arr[table_id].logical_em_info.key_field_bit_range_arr;
  result_fields_arr           = api_info.db_em_container.db_em_info_arr[table_id].logical_em_info.result_field_bit_range_arr;
  total_key_width_in_bits     = api_info.db_em_container.db_em_info_arr[table_id].logical_em_info.total_key_width;
  total_result_width_in_bits  = api_info.db_em_container.db_em_info_arr[table_id].logical_em_info.total_result_width;

  virtual_key_data    = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));
  virtual_result_data = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_result_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));

  build_virtual_db_data(0, key_fields_arr, key, virtual_key_data);
  build_virtual_db_data(0, result_fields_arr, result, virtual_result_data);

  /*mask and valid bits set to zeros*/
  mask  = build_em_mask(key_fields_arr, key);
  valid = build_tcam_valid();

  logical_cam_based_db_write(&api_info.db_em_container.db_em_info_arr[table_id].logical_em_info, unit, index, key, mask, valid, result); 
  /*if(api_info.db_em_container.db_em_info_arr[table_id].em_cache.next_free_index == index)*/
  update_em_cache_and_next_free_index(index, virtual_key_data, virtual_result_data, &api_info.db_em_container.db_em_info_arr[table_id]);
  free_pemladrv_mem_struct(&mask);
  free_pemladrv_mem_struct(&valid);
  return 0;
}


int pemladrv_em_remove_all(int unit, table_id_t table_id) {
  unsigned int nof_entries = PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES;
  unsigned int index;

  for(index = 0; index < nof_entries; ++index) {
    if(api_info.db_em_container.db_em_info_arr[table_id].em_cache.physical_entry_occupation[index] == 1)
      pemladrv_em_remove_by_index(unit, table_id, index);
  }
  return 0;
}


int pemladrv_em_remove_by_index(int unit, table_id_t table_id, int index) {
  set_valid_bit_to_zero(unit, index, &api_info.db_em_container.db_em_info_arr[table_id].logical_em_info);

  /*Update in physical_entry_occupation that the entry is now free*/
  api_info.db_em_container.db_em_info_arr[table_id].em_cache.physical_entry_occupation[index] = 0;
  /*Update next_free_index by need*/
  if ( index < api_info.db_em_container.db_em_info_arr[table_id].em_cache.next_free_index || api_info.db_em_container.db_em_info_arr[table_id].em_cache.next_free_index == -1)
    api_info.db_em_container.db_em_info_arr[table_id].em_cache.next_free_index = index;
  return 0;
}

/*******************************/
/* End of EM access functions  */
/*******************************/


/****************************************************
 **  LPM (Longest Prefix Match) access functions
 ****************************************************/
int pemladrv_lpm_insert(int unit, table_id_t table_id,  pemladrv_mem_t* key, int length_in_bits,  pemladrv_mem_t* result, int *index) {
  int                       shift_direction;
  unsigned int              nof_fields_in_key           = api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.nof_fields_in_key; 
  const unsigned int        total_key_width_in_bits     = api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.total_key_width;
  const FieldBitRange*const key_fields_arr              = api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.key_field_bit_range_arr;
  unsigned int uint_len_in_bits = length_in_bits;
  unsigned int *virtual_key_data = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));
  
  /*prefix length sanity check*/
  if(uint_len_in_bits > api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.total_key_width) {
    /*printf("ERROR. Wrong prefix in: Exact-Match - #%d id\n", table_id);*/
    free(virtual_key_data);
    return 1;/*return error of bad prefix*/
  }

  /*Check to see if all key fields are available*/
  if (key->nof_fields != nof_fields_in_key) {
    /*printf("ERROR. One or More fields are missing while trying to write key. Exact-Match - #%d id\n", table_id); */
    free(virtual_key_data);
    return 1;/*return error of bad key fields*/
  }

  /*Check to see if key exist - for overwriting*/
  build_virtual_db_data(0, key_fields_arr, key, virtual_key_data);    /*needed only to search for existing key in db*/
  update_key_with_prefix_length(length_in_bits, total_key_width_in_bits, virtual_key_data);

  *index = does_key_exists_in_physical_lpm_db(&api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info, uint_len_in_bits, &api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache, virtual_key_data);
  if (*index != -1) {
    /*printf("Warning - Key already exists. Key index is %d. OVERWRITING.\n", *index);*/
    free(virtual_key_data);
    return pem_lpm_set_by_id(unit, table_id, *index, key, &length_in_bits, result);/*return error of key already existing*/
  }
  free(virtual_key_data);

  /*Check to see if LPM is already full*/
  if (is_lpm_full(api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.total_nof_entries ,api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation)) {
    /*printf("The DB is already full, Can't write or add keys.\n");*/
    return 1;/*return error of full db*/
  }

  /*writing the data to 'next_free_index'*/
  *index = find_lpm_index_to_insert(length_in_bits, total_key_width_in_bits, &api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache);

  /*if free entry, write to physical*/
  if( api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation[*index] == 0)
    return pem_lpm_set_by_id(unit, table_id, *index, key, &length_in_bits, result);


  shift_direction = find_minimum_shifting_direction(*index, table_id);

  shift_physical_to_clear_entry(unit, shift_direction, index, table_id);
  return pem_lpm_set_by_id(unit, table_id, *index, key, &length_in_bits, result); 

}


int pemladrv_lpm_remove(int unit, table_id_t table_id,  pemladrv_mem_t* key,  int length_in_bits, int *index){
  *index = find_physical_entry_index_by_key(&api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info, api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation, api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.lpm_key_entry_arr, key);
  if (*index == -1) {
    /*printf("Key doesn't exist in DB\n");*/
    return 1; /*RETURN KEY NOT EXIST ERROR*/
  }
  return pemladrv_lpm_remove_by_index(unit, table_id, *index);
}

int pemladrv_lpm_lookup(int unit, table_id_t table_id,  pemladrv_mem_t* key,  pemladrv_mem_t* result, int *index){
  *index = find_physical_lpm_entry_index_by_key(&api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info, &api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache, key);
  if (*index == -1) {
    /*printf("Key doesn't exist in DB\n");*/
    return 1; /*RETURN KEY NOT EXIST ERROR*/
  }
  return get_lpm_data_by_entry(table_id, *index, key, result);
}


int pem_lpm_get_by_id(int unit, table_id_t table_id, int index,  pemladrv_mem_t* key, int *length_in_bits, pemladrv_mem_t* result) {

  if (index == -1) {
    /*printf("Invalid entry\n");*/
    return 1; /*RETURN INVALID ENTRY ERROR*/
  }
  *length_in_bits = api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.lpm_key_entry_prefix_length_arr[index];
  return get_lpm_data_by_entry(table_id, index, key, result);
}

int pem_lpm_set_by_id(int unit, table_id_t table_id, int index, pemladrv_mem_t* key, int *length_in_bits, pemladrv_mem_t* result) {

  const FieldBitRange* key_fields_arr;
  const FieldBitRange* result_fields_arr;
  unsigned int         total_key_width_in_bits;
  unsigned int         total_result_width_in_bits;
  unsigned int*        virtual_key_data;
  unsigned int*        virtual_result_data ;
  pemladrv_mem_t     *mask, *valid;
  unsigned int len_in_bits_uint = *length_in_bits;

  /*prefix length sanity check*/
  if(len_in_bits_uint > api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.total_key_width) {
    /*printf("ERROR. Wrong prefix in: Exact-Match - #%d id\n", table_id);*/
    return 1;/*return error of bad prefix*/
  }

  if (index < 0 || index > 31) {
    /*printf("Invalid entry\n");*/
    return 1; /*RETURN INVALID ENTRY ERROR*/
  }

  /*if (api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation[index] == 1) {
   *  printf("Error. Entry is already occupied.\n");                                                          
   *  return 1; RETURN ENTRY OCCUPIED ERROR 
   *}                                                                                                         
   */

  key_fields_arr              = api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.key_field_bit_range_arr;
  result_fields_arr           = api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.result_field_bit_range_arr;
  total_key_width_in_bits     = api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.total_key_width;
  total_result_width_in_bits  = api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.total_result_width;

  virtual_key_data    = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));
  virtual_result_data = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_result_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));

  build_virtual_db_data(0, key_fields_arr, key, virtual_key_data);
  build_virtual_db_data(0, result_fields_arr, result, virtual_result_data);
  update_key_with_prefix_length(*length_in_bits, total_key_width_in_bits, virtual_key_data);

  /*mask and valid bits set to zeros*/

  mask  = build_lpm_mask(key_fields_arr, *length_in_bits, total_key_width_in_bits, key);
  valid = build_tcam_valid();
  set_pem_mem_accesss_fldbuf(0, virtual_key_data, key_fields_arr, key);

  logical_cam_based_db_write(&api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info, unit, index, key, mask, valid, result); 
  update_lpm_cache(index, *length_in_bits, virtual_key_data, virtual_result_data, &api_info.db_lpm_container.db_lpm_info_arr[table_id]);

  free_pemladrv_mem_struct(&mask);
  free_pemladrv_mem_struct(&valid);
  return 0;
}

int pemladrv_lpm_remove_all(int unit, table_id_t table_id){
  unsigned int nof_entries = PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES;
  unsigned int index;

  for(index = 0; index < nof_entries; ++index) {
    if(api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation[index] == 1)
      pemladrv_lpm_remove_by_index(unit, table_id, index);
  }
  return 0;
}

int pemladrv_lpm_remove_by_index (int unit, table_id_t table_id, int index){

  set_valid_bit_to_zero(unit, index, &api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info);

  /*Update in physical_entry_occupation that the entry is now free*/
  api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation[index] = 0;

  return 0;
}


/********************************/
/* End of LPM access functions  */
/********************************/


/* Returns number of cols in CAM chunk mattrix*/
unsigned int get_nof_cam_sram_chunk_cols(const unsigned int total_result_width) {
  if (total_result_width % PEM_CFG_API_CAM_RESULT_CHUNK_WIDTH == 0)
    return (total_result_width / PEM_CFG_API_CAM_RESULT_CHUNK_WIDTH);
  else
    return (total_result_width / PEM_CFG_API_CAM_RESULT_CHUNK_WIDTH) + 1;
}

/* Returns number of cols in MAP chunk mattrix*/
unsigned int get_nof_map_sram_chunk_cols(const unsigned int total_result_width) {
  if (total_result_width % PEM_CFG_API_MAP_CHUNK_WIDTH == 0)
    return (total_result_width / PEM_CFG_API_MAP_CHUNK_WIDTH);
  else
    return (total_result_width / PEM_CFG_API_MAP_CHUNK_WIDTH) + 1;
}


/* Returns number of cols in MAP chunk mattrix*/
/* this function is for direct chunks only, and made to deal with DBs that      *
 * have sevral fields, have chunks (exept the last one) that occupy only part   *
 * of the physical sram and possibly have offset between chunks (meaning,       * 
 * chunks may be mapped to different adresses and will not follow each other.   *
 * for 'result' in CAM based DBs (where each field 'sits' next to its neighbour *
 * fields) you sould use: get_nof_cam_sram_chunk_cols.                              */
unsigned int get_nof_direct_chunk_cols(LogicalDirectInfo* direct_info) {
  unsigned int chunks_num = 0;
  unsigned int field_num, curr_lsb_bit, curr_msb_bit, field_width;


  for (field_num = 0; field_num < direct_info->nof_fields; ++field_num) {
    curr_lsb_bit = direct_info->field_bit_range_arr[field_num].lsb;
    curr_msb_bit = direct_info->field_bit_range_arr[field_num].msb;
    field_width = curr_msb_bit - curr_lsb_bit + 1;
    chunks_num += get_nof_map_sram_chunk_cols(field_width);
  }
  return chunks_num;
}


/* Returns number of rows in MAP chunk mattrix*/
unsigned int get_nof_direct_chunk_rows(const unsigned int nof_entries) {
  if (nof_entries % PEM_CFG_API_MAP_CHUNK_N_ENTRIES == 0)
    return (nof_entries / PEM_CFG_API_MAP_CHUNK_N_ENTRIES);
  else
    return (nof_entries / PEM_CFG_API_MAP_CHUNK_N_ENTRIES) + 1;
}


/* Builds an unsigned int array that consists of ones, placed in the bits that needs modification, and an unsigned int array that consists of data from virtual db. used for key (included mask and valid bits)*/
void build_virtual_db_mask_and_data_from_key_fields(const int key_width, const unsigned int total_width, const FieldBitRange* field_bit_range_arr, const pemladrv_mem_t* key, const pemladrv_mem_t* mask, const pemladrv_mem_t* valid, unsigned int **virtual_key_mask, unsigned int **virtual_key_data) {
  FieldBitRange* valid_field_bit_range_arr = produce_valid_field_bit_range();
  /*MASK*/
  *virtual_key_mask = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
  
  build_virtual_db_mask(0,             field_bit_range_arr,              key,       *virtual_key_mask);  /*key - offset is 0 */                                                                                                                                                   
  build_virtual_db_mask(key_width,     field_bit_range_arr,              mask,      *virtual_key_mask);  /*Done for Mask. includes the offset of the key*/
  build_virtual_db_mask(2 * key_width, valid_field_bit_range_arr,        valid,     *virtual_key_mask);  /*Done for Valid. includes the offset of the key and the mask*/

  /*DATA*/
  *virtual_key_data = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
                                                                                                                                                                                      
  build_virtual_db_data(          0, field_bit_range_arr,              key,       *virtual_key_data);  /*key - offset is 0 */                                           
  build_virtual_db_data(  key_width, field_bit_range_arr,              mask,      *virtual_key_data);  /*Done for Mask. includes the offset of the key*/
  build_virtual_db_data(2*key_width, valid_field_bit_range_arr,        valid,     *virtual_key_data);  /*Done for Valid. includes the offset of the key and the mask*/
  free(valid_field_bit_range_arr);
  return;
}


/* Builds an unsigned int array that consists of ones, placed in the bits that needs modification, and an unsigned int array that consists of data from virtual db. Used for result*/
void build_virtual_db_mask_and_data_from_result_fields(const unsigned int         total_width, 
                                                       const FieldBitRange*       field_bit_range_arr, 
                                                       const pemladrv_mem_t*    result, 
                                                       const int                  is_init_mask_to_ones_at_field_location,
                                                       unsigned int **            virtual_result_mask, 
                                                       unsigned int **            virtual_result_data) {

  /*MASK*/
  *virtual_result_mask = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_width, SAL_UINT32_NOF_BITS), sizeof(uint32));

  /*set ones to relevant fields*/
  if (is_init_mask_to_ones_at_field_location) {
    build_virtual_db_mask(0, field_bit_range_arr, result, *virtual_result_mask);       /*offset for RESULT is 0 */
  }

  /*DATA*/
  *virtual_result_data = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
                                                                                                                                                                                      
  build_virtual_db_data(0, field_bit_range_arr, result,  *virtual_result_data);     /*offset for RESULT is 0 */
  return;
}


/* Modify virtual_field_mask msb/lsb bits for each field in field_bit_range_arr*/
void build_virtual_db_mask(const unsigned int offset, const FieldBitRange* field_bit_range_arr, const pemladrv_mem_t *data , unsigned int *virtual_field_mask) {
  unsigned int nof_fields = data->nof_fields;
  unsigned int msb_bit, lsb_bit;
  unsigned int field_index;

  for (field_index = 0; field_index < nof_fields; ++field_index) {
    msb_bit = field_bit_range_arr[data->fields[field_index]->field_id].msb;
    lsb_bit = field_bit_range_arr[data->fields[field_index]->field_id].lsb;

    set_mask_with_ones(msb_bit + offset, lsb_bit + offset, virtual_field_mask);
  }
}


/* Build a cache array of a single line in virtual data*/
void build_virtual_db_data(const unsigned int offset, const FieldBitRange* field_bit_range_arr, const pemladrv_mem_t *data , unsigned int *virtual_field_data) {
  unsigned int nof_fields = data->nof_fields;
  unsigned int msb_bit, lsb_bit;
  unsigned int field_index;
  unsigned int field_width;

  for (field_index = 0; field_index < nof_fields; ++field_index) {
    msb_bit = field_bit_range_arr[data->fields[field_index]->field_id].msb;
    lsb_bit = field_bit_range_arr[data->fields[field_index]->field_id].lsb;
    field_width = msb_bit - lsb_bit + 1;

    set_bits(data->fields[field_index]->fldbuf, 0, lsb_bit + offset, field_width, virtual_field_data);
  }
}


/* Modify only the relevent bits in virtual_field_mask to match msb/lsb offset of relevent field*/
void set_mask_with_ones(const unsigned int msb_bit, const unsigned int lsb_bit, unsigned int *virtual_field_mask) {
  unsigned int curr_lsb = lsb_bit;
  unsigned int field_width, virtual_mask_arr_index ;
  unsigned int lsb_offset;
  unsigned int msb_offset = msb_bit % UINT_WIDTH_IN_BITS;


  while (curr_lsb <= msb_bit ) {
    virtual_mask_arr_index = curr_lsb / UINT_WIDTH_IN_BITS;
    lsb_offset = curr_lsb % UINT_WIDTH_IN_BITS;
    field_width = msb_bit - curr_lsb + 1;
    if (lsb_offset + field_width > UINT_WIDTH_IN_BITS ) {
      set_ones_in_chunk(lsb_offset, UINT_WIDTH_IN_BITS - 1, virtual_mask_arr_index, virtual_field_mask);
      curr_lsb += (UINT_WIDTH_IN_BITS - lsb_offset);
      continue;
    }
    set_ones_in_chunk(lsb_offset, msb_offset, virtual_mask_arr_index, virtual_field_mask);
    curr_lsb += (msb_offset - lsb_offset + 1);
  }
}


/* Set the virtual mask in the correct index with ones with the width of msb-lsb. (starting from lsb bit)*/
void set_ones_in_chunk(const unsigned int lsb_bit, const unsigned int msb_bit, const unsigned int virtual_mask_arr_index, unsigned int* virtual_mask_arr) {
  unsigned int ones_mask = 0xffffffff;
  ones_mask = ones_mask >> (UINT_WIDTH_IN_BITS - (msb_bit - lsb_bit+1));
  ones_mask = ones_mask << lsb_bit;
  virtual_mask_arr[virtual_mask_arr_index] = virtual_mask_arr[virtual_mask_arr_index] | ones_mask;
}


/* Check to see if specific chunk was modified and needs to be written */
int do_chunk_require_writing(const DbChunkMapper* chunk_mapper, const unsigned int *virtual_field_mask) {
  unsigned int temp_mask[MAX_MEM_DATA_LENGTH / UINT_WIDTH_IN_BITS] = {0};
  unsigned int lsb = chunk_mapper->virtual_mem_width_offset;
  unsigned int msb = lsb + chunk_mapper->chunk_width;
  int i;
  int do_write = 0;
  int array_size = chunk_mapper->chunk_width / UINT_WIDTH_IN_BITS + 1;

  set_mask_with_ones(msb, lsb, temp_mask);
  for (i = lsb / UINT_WIDTH_IN_BITS; i < array_size + lsb / UINT_WIDTH_IN_BITS; ++i) {
    temp_mask[i] = temp_mask[i] & virtual_field_mask[i];
    if (temp_mask[i] != 0)
      do_write = 1;
  }
  return do_write;
}


/* Modify physical_memory_entry_data relevant bits from virtual_db_line_input_data_arr*/
/* flag distinguishes between tcam and sram type dbs. 
 * flag = 0: TCAM
 * flag = 1: SRAM
 */
void modify_entry_data(const unsigned char flag, 
                       const unsigned char last_chunk, 
                       const unsigned int  chunk_mapping_width, 
                       const unsigned int  chunk_virtual_mem_width_offset,
                       const unsigned int  chunk_physical_mem_width_offset,
                       const int           total_key_width, 
                       const unsigned int  *virtual_db_line_mask_arr, 
                       const unsigned int  *virtual_db_line_input_data_arr, 
                             unsigned int *physical_memory_entry_data) {

  unsigned int *one_chunk_virt_mask;
  unsigned int *one_chunk_virt_data;
  int last_physical_data_arr_entry_to_update;

  int nof_entries_to_update = -1;
  int i;  
  int physical_data_arr_entry_offset = 0;

  if (flag == TCAM) {
    physical_data_arr_entry_offset = 0;
    one_chunk_virt_mask = (unsigned int*)calloc(1 + UTILEX_DIV_ROUND_UP(chunk_mapping_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
    one_chunk_virt_data = (unsigned int*)calloc(1 + UTILEX_DIV_ROUND_UP(chunk_mapping_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
    nof_entries_to_update = 2;
    /*KEY*/
    /* taking from the virtual_db_line_mask_arr       the truncated mask of the tcam key*/
    set_bits(virtual_db_line_mask_arr,        chunk_virtual_mem_width_offset,                         0,                                  chunk_mapping_width, one_chunk_virt_mask);
    /* taking from the virtual_db_line_input_data_arr the truncated data of the tcam key*/                                                      
    set_bits(virtual_db_line_input_data_arr,  chunk_virtual_mem_width_offset,                         0,                                  chunk_mapping_width, one_chunk_virt_data);
    /*MASK*/                                                                                                                                    
    /* taking from the virtual_db_line_mask_arr       the truncated mask of the tcam mask*/                                                     
    set_bits(virtual_db_line_mask_arr,        chunk_virtual_mem_width_offset + total_key_width,       PEM_CFG_API_CAM_TCAM_KEY_WIDTH,     chunk_mapping_width, one_chunk_virt_mask);
    /* taking from the virtual_db_line_input_data_arr the truncated data of the tcam mask*/                                                     
    set_bits(virtual_db_line_input_data_arr,  chunk_virtual_mem_width_offset + total_key_width,       PEM_CFG_API_CAM_TCAM_KEY_WIDTH,     chunk_mapping_width, one_chunk_virt_data);
    /*VALID*/
    /* taking from the virtual_db_line_mask_arr       the truncated mask of the tcam mask*/
    set_bits(virtual_db_line_mask_arr,       (2 * total_key_width),                                         2 * PEM_CFG_API_CAM_TCAM_KEY_WIDTH, chunk_mapping_width, one_chunk_virt_mask);
    /* taking from the virtual_db_line_input_data_arr the truncated data of the tcam mask*/
    set_bits(virtual_db_line_input_data_arr, (2 * total_key_width),                                         2 * PEM_CFG_API_CAM_TCAM_KEY_WIDTH, chunk_mapping_width, one_chunk_virt_data);
    /*In case last chunk, pad mask with ones*/
    if (last_chunk) {
      unsigned int ones[1];
                   ones[0] = 0xFFFFFFFF;
      /*Padding mask with ones*/                                                     
      set_bits(ones, 0, PEM_CFG_API_CAM_TCAM_KEY_WIDTH + chunk_mapping_width, PEM_CFG_API_CAM_TCAM_KEY_WIDTH - chunk_mapping_width, one_chunk_virt_mask);
      /*Padding data with ones */                                                     
      set_bits(ones, 0, PEM_CFG_API_CAM_TCAM_KEY_WIDTH + chunk_mapping_width, PEM_CFG_API_CAM_TCAM_KEY_WIDTH - chunk_mapping_width, one_chunk_virt_data);
    }
  }
  else {
    assert(flag == SRAM);
    last_physical_data_arr_entry_to_update     = (chunk_physical_mem_width_offset+chunk_mapping_width) / UINT_WIDTH_IN_BITS;
    physical_data_arr_entry_offset             = chunk_physical_mem_width_offset / UINT_WIDTH_IN_BITS;
    nof_entries_to_update                      = last_physical_data_arr_entry_to_update - physical_data_arr_entry_offset + 1;
    one_chunk_virt_mask = (unsigned int*)calloc(1 + UTILEX_DIV_ROUND_UP(chunk_physical_mem_width_offset+chunk_mapping_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
    one_chunk_virt_data = (unsigned int*)calloc(1 + UTILEX_DIV_ROUND_UP(chunk_physical_mem_width_offset+chunk_mapping_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
    /* taking from the virtual_db_line_mask_arr       the truncated mask of the sram*/
    set_bits(virtual_db_line_mask_arr,       chunk_virtual_mem_width_offset, chunk_physical_mem_width_offset, chunk_mapping_width, one_chunk_virt_mask);
    /* taking from the virtual_db_line_input_data_arr the truncated data of the sram*/
    set_bits(virtual_db_line_input_data_arr, chunk_virtual_mem_width_offset, chunk_physical_mem_width_offset, chunk_mapping_width, one_chunk_virt_data);
  }

  for (i = physical_data_arr_entry_offset; i < physical_data_arr_entry_offset+nof_entries_to_update; ++i) {
    physical_memory_entry_data[i] = (physical_memory_entry_data[i] & ~one_chunk_virt_mask[i]);
    one_chunk_virt_data[i] = (one_chunk_virt_data[i] & one_chunk_virt_mask[i]);
    physical_memory_entry_data[i] = (physical_memory_entry_data[i] | one_chunk_virt_data[i]);
  }
  free(one_chunk_virt_mask);
  free(one_chunk_virt_data);
}


/* read physical_memory_entry_data relevant bits to virtual_db_line_input_data_arr*/
void reg_read_entry_data(  const unsigned int  chunk_mapping_width, 
                           const unsigned int  chunk_virtual_mem_width_offset,
                           const unsigned int  chunk_physical_mem_width_offset,
                           const unsigned int *physical_memory_entry_data,
                                 unsigned int  *virtual_db_line_mask_arr, 
                                 unsigned int  *virtual_db_line_input_data_arr
                       ) {

  /*unsigned int *one_chunk_virt_mask;
   *unsigned int *one_chunk_virt_data;

   *int nof_entries_to_update = -1;
   *int i;
  int physical_data_arr_entry_offset = 0;*/

  /*int last_physical_data_arr_entry_to_update = (chunk_physical_mem_width_offset+chunk_mapping_width) / UINT_WIDTH_IN_BITS;*/
  /*physical_data_arr_entry_offset             = chunk_physical_mem_width_offset / UINT_WIDTH_IN_BITS;*/

  set_bits          (physical_memory_entry_data, chunk_physical_mem_width_offset, chunk_virtual_mem_width_offset, chunk_mapping_width, virtual_db_line_input_data_arr);
  set_mask_with_ones(chunk_virtual_mem_width_offset + chunk_mapping_width-1, chunk_virtual_mem_width_offset, virtual_db_line_mask_arr);
}


int calc_nof_arr_entries_from_width(const unsigned int width_in_bits)
{
  const int nof_bits_in_int = sizeof(int) * 8;
  unsigned int nof_entries =  (width_in_bits % nof_bits_in_int == 0) ? width_in_bits / nof_bits_in_int : width_in_bits / nof_bits_in_int + 1;
  return nof_entries;
}

/* returns pointer to the new or found physical_memory_entry*/
PhysicalWriteInfo* find_or_allocate_and_read_physical_data(const unsigned int chunk_mem_block_id, 
                                                           const unsigned int chunk_phy_mem_addr, 
                                                           const unsigned int chunk_phy_mem_row_index, 
                                                           const unsigned int chunk_phy_mem_width, 
                                                           const unsigned int chunk_mapping_width,
                                                           const int          is_mem,
                                                           PhysicalWriteInfo** target_physical_memory_entry_data_list) {
  PhysicalWriteInfo* target_physical_memory_entry_data_list_curr_element = *target_physical_memory_entry_data_list;
  /*const unsigned int physical_address = chunk_mapper->phy_mem_addr;*/
  PhysicalWriteInfo* target_physical_memory_entry_data_new_element = NULL;
  int entry_data_arr_nof_entries;


  /*maybe physical adress is already in the list and thus we dont need to read it*/
  while(target_physical_memory_entry_data_list_curr_element) {
    if (chunk_phy_mem_addr          == target_physical_memory_entry_data_list_curr_element->mem.mem_address &&
        chunk_phy_mem_row_index     == target_physical_memory_entry_data_list_curr_element->mem.row_index) {
        return target_physical_memory_entry_data_list_curr_element;
    }
    if (target_physical_memory_entry_data_list_curr_element->next) {
      target_physical_memory_entry_data_list_curr_element = target_physical_memory_entry_data_list_curr_element->next;
      continue;
    }
    else {
      break;
    }
  }
  /* next is null -> lets allocate it and read the physical memory entry and return it*/
  target_physical_memory_entry_data_new_element = (PhysicalWriteInfo*)calloc(1, sizeof(PhysicalWriteInfo));
  entry_data_arr_nof_entries = calc_nof_arr_entries_from_width(chunk_phy_mem_width);
  target_physical_memory_entry_data_new_element->entry_data = (unsigned int*)calloc(entry_data_arr_nof_entries, sizeof(unsigned int)); 
  init_phy_mem_t_from_chunk_mapper(chunk_mem_block_id, 
                                   chunk_phy_mem_addr, 
                                   chunk_phy_mem_row_index, 
                                   chunk_phy_mem_width, 
                                   &target_physical_memory_entry_data_new_element->mem);
  /*Optimization - if rewriting all chunk, no reading is needed.*/ 
  if (chunk_mapping_width != chunk_phy_mem_width)
    pem_read(0, &target_physical_memory_entry_data_new_element->mem, is_mem, target_physical_memory_entry_data_new_element->entry_data);
  /* append the new physical entry to the list*/
  if (target_physical_memory_entry_data_list_curr_element)
    target_physical_memory_entry_data_list_curr_element->next  = target_physical_memory_entry_data_new_element;
  else
    *target_physical_memory_entry_data_list = target_physical_memory_entry_data_new_element;
  return target_physical_memory_entry_data_new_element;
}


/* Run over list and write to physical adress*/
void write_all_physical_data_from_list(PhysicalWriteInfo* target_physical_memory_entry_data_curr, int is_mem) {

    while(target_physical_memory_entry_data_curr) {
      pem_write(0, &target_physical_memory_entry_data_curr->mem, is_mem, target_physical_memory_entry_data_curr->entry_data);
      target_physical_memory_entry_data_curr = target_physical_memory_entry_data_curr->next;
    }
}


/* Build FieldBitRange for valid bit*/
FieldBitRange* produce_valid_field_bit_range() {
  FieldBitRange* valid_bit_array = (FieldBitRange*)calloc(1, sizeof(FieldBitRange));
  valid_bit_array->lsb = 0;
  valid_bit_array->msb = 0;  
  valid_bit_array->is_field_mapped = 1;
  return valid_bit_array;
}


/* Runs over all relevant chunks and updates the physical_memory_data_list with adresses and entry_data*. returns pointer to the list.*/
PhysicalWriteInfo* run_over_all_chunks_read_physical_data_and_update_physical_memory_entry_data_list(const unsigned char flag, const unsigned int virtual_row_index, const unsigned int total_width, const unsigned int chunk_matrix_row, const unsigned int nof_implamentations, const unsigned int nof_chunk_matrix_cols, const LogicalDbChunkMapperMatrix* db_chunk_mapper_matrix, const unsigned int *virtual_mask, const unsigned int *virtual_data) {

  int do_write = 0;
  unsigned int chunk_index, implamentation_index;
  unsigned char last_chunk = 0;

  PhysicalWriteInfo* target_physical_memory_entry_data_list = NULL;
  PhysicalWriteInfo* target_physical_memory_entry_data_list_curr_element = NULL;

  for (chunk_index = 0; chunk_index < nof_chunk_matrix_cols; ++chunk_index) {
    if ( chunk_index == nof_chunk_matrix_cols - 1 ) { last_chunk = 1; }                                  /*Writing the Valid Bit only for the last chunk*/
    for (implamentation_index = 0; implamentation_index < nof_implamentations; ++implamentation_index) {

      /*chunk_mapper_matrix_arr[0] - only one READ is needed from any chunk_mapper_matrix_arr*/
      do_write = do_chunk_require_writing(db_chunk_mapper_matrix[implamentation_index].db_chunk_mapper[chunk_matrix_row][chunk_index], virtual_mask);
      if (do_write) {
        const DbChunkMapper* chunk_mapper = db_chunk_mapper_matrix[implamentation_index].db_chunk_mapper[chunk_matrix_row][chunk_index];
        const unsigned int physical_row_index = calculate_physical_row_index_from_chunk_mapper(chunk_mapper, virtual_row_index);
        target_physical_memory_entry_data_list_curr_element = find_or_allocate_and_read_physical_data(chunk_mapper->mem_block_id, 
                                                                                                      chunk_mapper->phy_mem_addr, 
                                                                                                      physical_row_index,
                                                                                                      chunk_mapper->phy_mem_width, 
                                                                                                      chunk_mapper->chunk_width,
                                                                                                      1, 
                                                                                                      &target_physical_memory_entry_data_list);

        modify_entry_data(flag, 
                          last_chunk,
                          chunk_mapper->chunk_width,
                          chunk_mapper->virtual_mem_width_offset,
                          chunk_mapper->phy_mem_width_offset,
                          total_width, 
                          virtual_mask, 
                          virtual_data, 
                          target_physical_memory_entry_data_list_curr_element->entry_data);
      }
    }
  }
  return target_physical_memory_entry_data_list;
}


/* Run over all chunks in db, read physical data and modify the bits in virtual_db_data_arr*/
void build_physical_db_key_data_array(const LogicalDbChunkMapperMatrix* db_chunk_mapper_matrix, const unsigned int nof_chunk_cols, const unsigned int chunk_matrix_row_index, const unsigned int virtual_row_index, const unsigned int total_virtual_key_width, unsigned int *virtual_db_data_arr) {
  unsigned int chunk_matrix_col_index;
  unsigned int chunk_width, phy_mem_entry_offset;
  unsigned int virtual_data_array_offset = 0;
  DbChunkMapper* chunk_mapper;
  unsigned int* read_data = (unsigned int*)calloc(2, sizeof(unsigned int));
  phy_mem_t*    phy_mem   = (phy_mem_t*)calloc(1, sizeof(phy_mem_t)); 
  unsigned int physical_row_index;

  /* Run over one row of chunk_matrix_mapper. Implamentation index is not relevant.*/
  for (chunk_matrix_col_index = 0; chunk_matrix_col_index < nof_chunk_cols; ++chunk_matrix_col_index) {
    chunk_width = db_chunk_mapper_matrix[0].db_chunk_mapper[chunk_matrix_row_index][chunk_matrix_col_index]->chunk_width;
    phy_mem_entry_offset = db_chunk_mapper_matrix[0].db_chunk_mapper[chunk_matrix_row_index][chunk_matrix_col_index]->phy_mem_entry_offset;
    /* Init phy_mem_t for each chunk*/
    chunk_mapper           = db_chunk_mapper_matrix[0].db_chunk_mapper[chunk_matrix_row_index][chunk_matrix_col_index];
    physical_row_index = calculate_physical_row_index_from_chunk_mapper(chunk_mapper, virtual_row_index);
    init_phy_mem_t_from_chunk_mapper(chunk_mapper->mem_block_id, chunk_mapper->phy_mem_addr, physical_row_index, chunk_mapper->phy_mem_width, phy_mem); 
    /* Read all physical data*/
    pem_read(0, phy_mem, 1, read_data);
    /*Set bits in virtual_db_data_arr taken from read_data, with the offset and width indicated in chunk_info*/
    /*key*/
    set_bits(read_data, phy_mem_entry_offset, virtual_data_array_offset, chunk_width, virtual_db_data_arr);
    /*mask*/
    set_bits(read_data, phy_mem_entry_offset + PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH, virtual_data_array_offset + total_virtual_key_width, chunk_width, virtual_db_data_arr);
    /*valid*/
    if ( chunk_matrix_col_index == nof_chunk_cols - 1)
      set_bits(read_data, 2 * PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH, 2 * total_virtual_key_width, 1, virtual_db_data_arr);
    /*new offset*/
    virtual_data_array_offset += chunk_width;
  }
  free(read_data);
  free(phy_mem);
  return;
}


/* Run over all chunks in db, read physical data and modify the bits in virtual_db_data_arr*/
void build_physical_db_result_data_array(const LogicalDbChunkMapperMatrix* db_chunk_mapper_matrix, const unsigned int nof_chunk_cols, const unsigned int chunk_matrix_row_index, const unsigned int virtual_row_index, const FieldBitRange* field_bit_range_arr, pemladrv_mem_t *result, unsigned int *virtual_db_data_arr) {
  unsigned int chunk_matrix_col_index, chunk_width, chunk_lsb_bit, chunk_msb_bit, field_idx, do_read;
  unsigned int virtual_data_array_offset = 0;
  DbChunkMapper* chunk_mapper;
  unsigned int physical_row_index;
  unsigned int* read_data = (unsigned int*)calloc(2, sizeof(unsigned int));
  phy_mem_t*    phy_mem   = (phy_mem_t*)calloc(1, sizeof(phy_mem_t)); 

  /* Run over one row of chunk_matrix_mapper. Implamentation index is not relevant.*/
  for (chunk_matrix_col_index = 0; chunk_matrix_col_index < nof_chunk_cols; ++chunk_matrix_col_index) {
    chunk_width   = db_chunk_mapper_matrix[0].db_chunk_mapper[chunk_matrix_row_index][chunk_matrix_col_index]->chunk_width;
    /*optimization to read only relevant chunks that required to be read by result pem_mem_access fields ID.*/
    chunk_lsb_bit = db_chunk_mapper_matrix[0].db_chunk_mapper[chunk_matrix_row_index][chunk_matrix_col_index]->virtual_mem_width_offset;
    chunk_msb_bit = chunk_lsb_bit + chunk_width - 1;
    do_read = 0;
    for(field_idx = 0; field_idx < result->nof_fields; ++field_idx) {
      if(field_bit_range_arr[result->fields[field_idx]->field_id].lsb > chunk_msb_bit && field_bit_range_arr[result->fields[field_idx]->field_id].msb < chunk_lsb_bit)  /*in case field is not located in specific chunk */
        continue;
      else {                                             /*field bits (or part of them) are located in this chunk*/
        do_read = 1;
        break;
      }
    }
    if(do_read) {
      /* Init phy_mem_t for each chunk*/
      chunk_mapper           = db_chunk_mapper_matrix[0].db_chunk_mapper[chunk_matrix_row_index][chunk_matrix_col_index];
      physical_row_index     = calculate_physical_row_index_from_chunk_mapper(chunk_mapper, virtual_row_index);
      init_phy_mem_t_from_chunk_mapper(chunk_mapper->mem_block_id, chunk_mapper->phy_mem_addr, physical_row_index, chunk_mapper->phy_mem_width, phy_mem); 

      /* Read all physical data*/
      pem_read(0, phy_mem, 1, read_data);
      /*Set bits in virtual_db_data_arr taken from read_data, with the offset and width indicated in chunk_info*/
      set_bits(read_data, db_chunk_mapper_matrix[0].db_chunk_mapper[chunk_matrix_row_index][chunk_matrix_col_index]->phy_mem_width_offset, virtual_data_array_offset, chunk_width, virtual_db_data_arr);
    }
    virtual_data_array_offset += chunk_width;
  }
  free(read_data);
  free(phy_mem);
}


/*Builds (only) key_array from tcam data (including key, mask and valid bits) by key width in bits*/
unsigned int* convert_tcam_data_into_key_array(const int key_width_in_bits, unsigned int*  tcam_data) {
  unsigned int* key_arr = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(key_width_in_bits, SAL_UINT32_NOF_BITS), sizeof(uint32));
  unsigned int  nof_bits_to_write;
  int           nof_bits_written, index;

  for (index = 0, nof_bits_written = 0; nof_bits_written < key_width_in_bits; nof_bits_written += nof_bits_to_write, ++index) {
    if ((key_width_in_bits - nof_bits_written) > PEM_CFG_API_CAM_TCAM_KEY_WIDTH ) 
      nof_bits_to_write = PEM_CFG_API_CAM_TCAM_KEY_WIDTH;
    else 
      nof_bits_to_write = (key_width_in_bits - nof_bits_written) % PEM_CFG_API_CAM_TCAM_KEY_WIDTH;

    set_bits(tcam_data, nof_bits_written, index * PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH, nof_bits_to_write, key_arr);
  }
  return key_arr;
}


/*Builds (only) mask_array from tcam data (including key, mask and valid bits) by key width in bits*/
unsigned int* convert_tcam_data_into_mask_array(const int total_key_width, const unsigned int* tcam_data) {
  
  unsigned int* mask_arr = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
  unsigned int  nof_bits_to_write;
  int           nof_bits_written, index;
  for (index = 0, nof_bits_written = 0; nof_bits_written < total_key_width; nof_bits_written += nof_bits_to_write, ++index) {
    if ((total_key_width - nof_bits_written) > PEM_CFG_API_CAM_TCAM_KEY_WIDTH ) 
      nof_bits_to_write = PEM_CFG_API_CAM_TCAM_KEY_WIDTH;
    else 
      nof_bits_to_write = (total_key_width - nof_bits_written) % PEM_CFG_API_CAM_TCAM_KEY_WIDTH;

    set_bits(tcam_data, nof_bits_written + total_key_width, index * PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH, nof_bits_to_write, mask_arr);
  }
  return mask_arr;
}


/*calculate prefix length from mask array*/
unsigned int calculate_prefix_from_mask_array(const int total_key_width, const unsigned int* mask_arr) {
  int j;
  unsigned int i;
  unsigned int nof_int_chunks_in_mask_arr, temp;
  unsigned int prefix_length = total_key_width;
  nof_int_chunks_in_mask_arr = get_nof_unsigned_int_chunks(total_key_width);

  for(i = 0; i < nof_int_chunks_in_mask_arr; ++i) {
    if( mask_arr[i] == -1) {
      prefix_length -= UINT_WIDTH_IN_BITS;
      continue;
    }
    break;
  }
  temp = mask_arr[i];
  for(j = 0; j < UINT_WIDTH_IN_BITS; ++j) {
    if(temp == 0)
      break;
    temp = temp >> 1;
    prefix_length -= 1;
  }
  return prefix_length;
}


/* Run over pem_mem_access fields and set the fldbuf by relevant bits in virtual_db_data_array*/
void set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(const unsigned int *virtual_db_data_array, const FieldBitRange* field_bit_range_arr, pemladrv_mem_t *result) {
  set_pem_mem_accesss_fldbuf(0, virtual_db_data_array, field_bit_range_arr, result); 
}


/* Run over pem_mem_access fields and insert the fldbuf from relevant bits in virtual_db_data_array*/
void set_pem_mem_accesss_fldbuf_from_physical_tcam_data_array(const unsigned int *virtual_db_data_array, const unsigned int total_key_width, const FieldBitRange* field_bit_range_arr , pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid) {

  FieldBitRange* valid_field_bit_range_arr = produce_valid_field_bit_range();

  /*Key and Mask may have different field_id's to read */
  /*KEY*/
  set_pem_mem_accesss_fldbuf(0, virtual_db_data_array, field_bit_range_arr, key); 
  /*MASK*/
  set_pem_mem_accesss_fldbuf(total_key_width, virtual_db_data_array, field_bit_range_arr, mask); 
  /*VALID*/
  set_pem_mem_accesss_fldbuf(2 * total_key_width, virtual_db_data_array, valid_field_bit_range_arr, valid);

  free(valid_field_bit_range_arr);
}


/*Retrieve prefix from physical tcam entry data*/
unsigned int get_prefix_length_from_physical_data(const int total_key_width, const unsigned int* tcam_data) {
  unsigned int ret;
  unsigned int* mask_arr = convert_tcam_data_into_mask_array(total_key_width, tcam_data);
  ret = calculate_prefix_from_mask_array(total_key_width, mask_arr);
  free(mask_arr);
  return ret;
}


/*Update LPM chace from physical memory*/
void update_lpm_cache_from_physical() {
  unsigned int index, entry, total_key_width, total_result_width, chunk_matrix_row_index, valid, prefix_length;
  unsigned int *tcam_data, *key_arr, *result_arr;
  pemladrv_mem_t *key, *result; /*result is needed to figure if each field was mapped, to optimize physical readings*/


  for (index = 0; index < api_info.db_lpm_container.nof_lpm_dbs; ++index) {
    if (NULL == api_info.db_lpm_container.db_lpm_info_arr[index].logical_lpm_info.result_chunk_mapper_matrix_arr)     /*In case db was not mapped*/
      continue;
    /*finding relevant fields and allocating memory*/
    total_key_width    = api_info.db_lpm_container.db_lpm_info_arr[index].logical_lpm_info.total_key_width;
    total_result_width = api_info.db_lpm_container.db_lpm_info_arr[index].logical_lpm_info.total_result_width;
    tcam_data          = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(2 * total_key_width + 1, SAL_UINT32_NOF_BITS), sizeof(uint32));  /*KEY + MASK + VALID*/
    pemladrv_mem_alloc_lpm(index, &key, &result);
    for (entry = 0; entry < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES ; ++entry) {
      chunk_matrix_row_index = entry / PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES;
      result_arr = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_result_width, SAL_UINT32_NOF_BITS), sizeof(uint32));

      build_physical_db_key_data_array(api_info.db_lpm_container.db_lpm_info_arr[index].logical_lpm_info.key_chunk_mapper_matrix_arr,  
                                       get_nof_tcam_chunk_cols(total_key_width), 
                                       chunk_matrix_row_index, 
                                       entry, 
                                       total_key_width, 
                                       tcam_data);
      
      valid = (tcam_data[UTILEX_DIV_ROUND_UP(2*total_key_width + 1, SAL_UINT32_NOF_BITS) - 1]) >> ((2 * total_key_width + 1) % UINT_WIDTH_IN_BITS - 1);
      /*in case valid is '0', no need to update cache.*/
      if(!valid) {
        free(result_arr);
        continue;
      }

      build_physical_db_result_data_array(api_info.db_lpm_container.db_lpm_info_arr[index].logical_lpm_info.result_chunk_mapper_matrix_arr
                                        , get_nof_cam_sram_chunk_cols(total_result_width)
                                        , chunk_matrix_row_index
                                        , entry
                                        , api_info.db_lpm_container.db_lpm_info_arr[index].logical_lpm_info.result_field_bit_range_arr
                                        , result
                                        , result_arr);

      key_arr = convert_tcam_data_into_key_array(total_key_width, tcam_data);
      prefix_length = get_prefix_length_from_physical_data(total_key_width, tcam_data);
      update_lpm_cache(entry, prefix_length, key_arr, result_arr, &api_info.db_lpm_container.db_lpm_info_arr[index]);

    }
    free(tcam_data);
    pemladrv_mem_free(key);
    pemladrv_mem_free(result);
  }
}


/*Update EM chace from physical memory*/
void update_em_cache_from_physical() {
  unsigned int index, entry, total_key_width, total_result_width, chunk_matrix_row_index, valid;
  unsigned int *tcam_data, *key_arr, *result_arr;
  pemladrv_mem_t *key, *result; /*result is needed to figure if each field was mapped, to optimize physical readings*/


  for (index = 0; index < api_info.db_em_container.nof_em_dbs; ++index) {
    if (NULL == api_info.db_em_container.db_em_info_arr[index].logical_em_info.result_chunk_mapper_matrix_arr)     /*In case db was not mapped*/
      continue;
    /*finding relevant fields and allocating memory*/
    total_key_width    = api_info.db_em_container.db_em_info_arr[index].logical_em_info.total_key_width;
    total_result_width = api_info.db_em_container.db_em_info_arr[index].logical_em_info.total_result_width;
    tcam_data          = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(2 * total_key_width + 1, SAL_UINT32_NOF_BITS), sizeof(uint32));  /*KEY + MASK + VALID*/
    pemladrv_mem_alloc_em(index, &key, &result);
    for (entry = 0; entry < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES ; ++entry) {
      chunk_matrix_row_index = entry / PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES;
      result_arr = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_result_width, SAL_UINT32_NOF_BITS), sizeof(uint32));

      build_physical_db_key_data_array(api_info.db_em_container.db_em_info_arr[index].logical_em_info.key_chunk_mapper_matrix_arr,  
                                       get_nof_tcam_chunk_cols(total_key_width), 
                                       chunk_matrix_row_index, 
                                       entry, 
                                       total_key_width, 
                                       tcam_data);
      
      valid = (tcam_data[UTILEX_DIV_ROUND_UP(2*total_key_width + 1, SAL_UINT32_NOF_BITS) - 1]) >> ((2 * total_key_width + 1) % UINT_WIDTH_IN_BITS - 1);
      /*in case valid is '0', no need to update cache.*/
      if(!valid) {
        free(result_arr);
        continue;
      }

      build_physical_db_result_data_array(api_info.db_em_container.db_em_info_arr[index].logical_em_info.result_chunk_mapper_matrix_arr
                                        , get_nof_cam_sram_chunk_cols(total_result_width)
                                        , chunk_matrix_row_index
                                        , entry
                                        , api_info.db_em_container.db_em_info_arr[index].logical_em_info.result_field_bit_range_arr
                                        , result
                                        , result_arr);

      key_arr = convert_tcam_data_into_key_array(total_key_width, tcam_data);
      update_em_cache_and_next_free_index(entry, key_arr, result_arr, &api_info.db_em_container.db_em_info_arr[index]);

    }
    free(tcam_data);
    pemladrv_mem_free(key);
    pemladrv_mem_free(result);
  }
}


/* Set all fldbufs per field in mem_access*/
void set_pem_mem_accesss_fldbuf(const unsigned int offset, const unsigned int* virtual_db_data_array,const FieldBitRange* field_bit_range_arr, pemladrv_mem_t* mem_access) {

  unsigned int field_index, field_width, field_id;

  for (field_index = 0; field_index < mem_access->nof_fields; ++field_index) {
    field_id = mem_access->fields[field_index]->field_id;
    field_width = field_bit_range_arr[field_id].msb - field_bit_range_arr[field_id].lsb + 1;
    set_bits(virtual_db_data_array, field_bit_range_arr[field_id].lsb + offset,                   0, field_width, mem_access->fields[field_index]->fldbuf);    /*KEY*/
  }
}


/*is db mapped to PEs*/
int is_db_mapped_to_pes(const LogicalDbChunkMapperMatrix*const db_chunk_mapper){
  return (db_chunk_mapper != NULL);
}


/* Check is key was alreay writen in DB. Returns index of key if exists and -1 if not. */
int does_key_exists_in_physical_db(const LogicalTcamInfo* tcam_info, const unsigned char* valids_arr, unsigned int** key_entry_arr, const unsigned int* key) {

  unsigned int entry_index;
  /*unsigned int total_nof_entries = tcam_info->total_nof_entries;*/
  unsigned int total_key_width = tcam_info->total_key_width;

  for (entry_index = 0; entry_index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++entry_index) {
    if ( (1 == valids_arr[entry_index]) && (copmare_two_virtual_keys(total_key_width, key, key_entry_arr[entry_index])) )
      return entry_index;
  }
  return -1;
}

/* Check is key was alreay writen in DB. Returns index of key if exists and -1 if not. */
int does_key_exists_in_physical_lpm_db(const LogicalTcamInfo* tcam_info, const unsigned int prefix_length_in_bits, const LpmDbCache* lpm_cache, const unsigned int* key) {
  unsigned int entry_index, i;
  unsigned int key_width = tcam_info->total_key_width;
  unsigned int key_chunks = calc_nof_arr_entries_from_width(key_width);
  unsigned int *temp_key_data = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(key_width, SAL_UINT32_NOF_BITS), sizeof(uint32));

  for (entry_index = 0; entry_index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++entry_index) {
    if ( 1 == lpm_cache->physical_entry_occupation[entry_index] ) {
      for (i = 0; i < key_chunks; ++i)
        temp_key_data[i] = key[i];
      update_key_with_prefix_length(lpm_cache->lpm_key_entry_prefix_length_arr[entry_index], key_width, temp_key_data);
      if (copmare_two_virtual_keys(key_width, lpm_cache->lpm_key_entry_arr[entry_index], temp_key_data) && (lpm_cache->lpm_key_entry_prefix_length_arr[entry_index] == prefix_length_in_bits) )
      {
        free(temp_key_data);
        return entry_index;
      }
    }
  }
  free(temp_key_data);
  return -1;
}


/*Build pemladrv_mem_t structure for mask, all bits set to zero, equivalent to key fields*/
pemladrv_mem_t* build_em_mask(const FieldBitRange* key_field_bit_range_arr, const pemladrv_mem_t *key) {
  int field_index;
  unsigned int num_of_chunks_in_fldbuf;

  pemladrv_mem_t* mask = (pemladrv_mem_t*)calloc(1, sizeof(pemladrv_mem_t));

  mask->nof_fields = key->nof_fields;
  mask->flags   = key->flags; 
  mask->fields  = (pemladrv_field_t**)calloc(mask->nof_fields, sizeof(pemladrv_field_t*));
  for (field_index = 0; field_index < mask->nof_fields; ++field_index) {
    mask->fields[field_index]         = (pemladrv_field_t*)calloc(1, sizeof(pemladrv_field_t));
    mask->fields[field_index]->field_id  = key->fields[field_index]->field_id;  /*in case all fields are written: mask->fields[field_index].field = field_index*/
    mask->fields[field_index]->flags  = key->fields[field_index]->flags;

    num_of_chunks_in_fldbuf           = get_nof_chunks_in_fldbuf(field_index, key_field_bit_range_arr);
    mask->fields[field_index]->fldbuf = (unsigned int*)calloc(num_of_chunks_in_fldbuf, sizeof(unsigned int));
  }
  return mask;
}


/*Build pemladrv_mem_t structure for mask, bits in the number of prefix length are set to zero*/
pemladrv_mem_t* build_lpm_mask(const FieldBitRange* key_field_bit_range_arr, int prefix_length_in_bits, const int key_length_in_bits, pemladrv_mem_t *key) {
  int field_index, nof_bits_to_mask;
  unsigned int i;
  unsigned int *virtual_mask;         
  unsigned int num_of_chunks_in_fldbuf;
  unsigned int num_of_uint_chunks_in_mask;

  pemladrv_mem_t* mask = (pemladrv_mem_t*)calloc(1, sizeof(pemladrv_mem_t));
  /*building virtual mask with zeros in prefix length*/
  num_of_uint_chunks_in_mask =  get_nof_unsigned_int_chunks(key_length_in_bits);
  virtual_mask = (unsigned int*)calloc(num_of_uint_chunks_in_mask, sizeof(unsigned int));
  for(i = 0; i < num_of_uint_chunks_in_mask; ++i)
    virtual_mask[i] = 0;
  i = 0;
  nof_bits_to_mask = key_length_in_bits - prefix_length_in_bits;
  while (nof_bits_to_mask > 0) {
    if (nof_bits_to_mask >= 32) {
      virtual_mask[i] = (unsigned)-1;
      nof_bits_to_mask -= 32;
      ++i;
    }
    else {
      virtual_mask[i] =(( 1 << nof_bits_to_mask ) - 1);
      break;
    }
  }
  /*allocating memories for field buffer*/
  mask->nof_fields = key->nof_fields;
  mask->flags   = key->flags;
  mask->fields  = (pemladrv_field_t**)calloc(mask->nof_fields, sizeof(pemladrv_field_t*));
  for (field_index = 0; field_index < mask->nof_fields; ++field_index) {
    mask->fields[field_index]         = (pemladrv_field_t*)calloc(1, sizeof(pemladrv_field_t));
    mask->fields[field_index]->field_id  = key->fields[field_index]->field_id;  /*in case all fields are written: mask->fields[field_index].field = field_index*/
    mask->fields[field_index]->flags  = key->fields[field_index]->flags;

    num_of_chunks_in_fldbuf           = get_nof_chunks_in_fldbuf(field_index, key_field_bit_range_arr);
    mask->fields[field_index]->fldbuf = (unsigned int*)calloc(num_of_chunks_in_fldbuf, sizeof(unsigned int));
  }

  set_pem_mem_accesss_fldbuf(0, virtual_mask, key_field_bit_range_arr, mask);
  free(virtual_mask);
  return mask;
}


/*Build pemladrv_mem_t structure for valid*/
pemladrv_mem_t* build_tcam_valid() {

  pemladrv_mem_t* valid     = (pemladrv_mem_t*)calloc(1, sizeof(pemladrv_mem_t));
  valid->fields               = (pemladrv_field_t**)calloc(1, sizeof(pemladrv_field_t*));
  valid->fields[0]            = (pemladrv_field_t*)calloc(1, sizeof(pemladrv_field_t));
  valid->flags = 0;           
  valid->nof_fields = 1;         
  valid->fields[0]->flags     = 0;
  valid->fields[0]->field_id     = 0;
  valid->fields[0]->fldbuf    = (unsigned int*)calloc(1, sizeof(unsigned int));
  valid->fields[0]->fldbuf[0] = 1;
  return valid;
}


/*Returns the nof chunks in the fldbuf unsigned int array*/
unsigned int get_nof_chunks_in_fldbuf(const unsigned int field_index, const FieldBitRange* field_bit_range_arr) {
  int field_width = field_bit_range_arr[field_index].msb - field_bit_range_arr[field_index].lsb + 1;
  return get_nof_unsigned_int_chunks(field_width);
}


/*Returns the nof chunks in the unsigned int array*/
unsigned int get_nof_unsigned_int_chunks(const int field_width) {
  if ((field_width % UINT_WIDTH_IN_BITS) == 0)
    return field_width / UINT_WIDTH_IN_BITS;
  else
    return (field_width / UINT_WIDTH_IN_BITS + 1);
}


/*Check to see if two keys are same. Returns TRUE/FALSE*/
unsigned int copmare_two_virtual_keys(const unsigned int key_width_in_bits, const unsigned int* first_key, const unsigned int* second_key) {
  int nof_chunks = get_nof_unsigned_int_chunks(key_width_in_bits);
  unsigned int last_chunk_of_first_key, last_chunk_of_second_key ;
  int chunk_index;
  /*checking all chunks exept the last one*/
  for (chunk_index = 0; chunk_index < nof_chunks - 1 ; ++chunk_index) {     
    if (first_key[chunk_index] != second_key[chunk_index])
      return 0;
  }
  /*cheking the last chunk (msb bits)*/
  last_chunk_of_first_key  = first_key[chunk_index]  << (32 - (key_width_in_bits%32));
  last_chunk_of_second_key = second_key[chunk_index] << (32 - (key_width_in_bits%32));
  if (last_chunk_of_first_key  != last_chunk_of_second_key)
    return 0;
  return 1;
}


/* After succsesful writing to db, updating the cache and the next free_index*/
void update_em_cache_and_next_free_index(unsigned int row_index, unsigned int *virtual_key_data, unsigned int *virtual_result_data, LogicalEmInfo* em_cache) {
  em_cache->em_cache.em_key_entry_arr[row_index]          = virtual_key_data;
  em_cache->em_cache.em_result_entry_arr[row_index]       = virtual_result_data;
  em_cache->em_cache.physical_entry_occupation[row_index] = 1;
  em_cache->em_cache.next_free_index                      = find_next_em_free_entry(em_cache);
}


/* After succsesful writing to db, updating the lpm cache*/
void update_lpm_cache(unsigned int row_index, const int prefix_length, unsigned int *virtual_key_data, unsigned int *virtual_result_data, LogicalLpmInfo* lpm_cache) {
  lpm_cache->lpm_cache.lpm_key_entry_arr[row_index]               = virtual_key_data;
  lpm_cache->lpm_cache.lpm_result_entry_arr[row_index]            = virtual_result_data;
  lpm_cache->lpm_cache.physical_entry_occupation[row_index]       = 1;
  lpm_cache->lpm_cache.lpm_key_entry_prefix_length_arr[row_index] = prefix_length; 
}


/*Finds the next free entry in EM db (starting from row_index) and returns its index. Returns -1 if DB is full*/
int find_next_em_free_entry(LogicalEmInfo* em_cache) {
  unsigned int next_free_index;
  for (next_free_index = 0; next_free_index < em_cache->logical_em_info.total_nof_entries; ++next_free_index) { /*starting for loop from 0 instead of row_index becouse of set_bi_id()*/
    if (!em_cache->em_cache.physical_entry_occupation[next_free_index]) {
      em_cache->em_cache.next_free_index = next_free_index;
      return next_free_index;
    }
  }
  return -1;
}


/*Finds the next free entry in EM db (starting from row_index) towards its direction and returns its index. Returns -1 if DB is full*/
int find_next_lpm_free_entry(const int index, const int direction, unsigned char* physical_entry_occupation) {
  
  int next_free_index;
  if(direction == UP) {
    for(next_free_index = index - 1; next_free_index >= 0; --next_free_index) {
      if(physical_entry_occupation[next_free_index] == 0)
        return next_free_index;
    }
   return -1;/*means no free entries found. SHOULD NOT REACH THIS LINE*/
  }

  else if(direction == DOWN) {
    for(next_free_index = index + 1; next_free_index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++next_free_index) {
      if(physical_entry_occupation[next_free_index] == 0)
        return next_free_index;
    }
    return -1;/*means no free entries found. SHOULD NOT REACH THIS LINE*/
  }
  return -1; /*SHOULD NOT REACH THIS LINE*/
}

/*Returns the index of key in physical DB. Returns -1 if key doesn't exists.*/
int find_physical_entry_index_by_key(const LogicalTcamInfo* tcam_info,  const unsigned char* valids_arr, unsigned int** key_field_arr,const pemladrv_mem_t *key) {
  int ret;
  FieldBitRange* key_field_bit_rang_arr = tcam_info->key_field_bit_range_arr;
  unsigned int   total_key_width        = tcam_info->total_key_width;
  unsigned int   *virtual_key_data      = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width, SAL_UINT32_NOF_BITS), sizeof(uint32));

  build_virtual_db_data(0, key_field_bit_rang_arr, key, virtual_key_data);
  ret = does_key_exists_in_physical_db(tcam_info, valids_arr, key_field_arr, virtual_key_data);
  free(virtual_key_data);
  return ret;
}


/*Returns the index of key in physical DB. Returns -1 if key doesn't exists.*/
int find_physical_lpm_entry_index_by_key(const LogicalTcamInfo* tcam_info, const LpmDbCache* lpm_cache, const pemladrv_mem_t *key) {
  unsigned int entry_index, i;
  FieldBitRange* key_field_bit_rang_arr = tcam_info->key_field_bit_range_arr;
  unsigned int   total_key_width        = tcam_info->total_key_width;
  unsigned int   *virtual_key_data      = (unsigned int*)calloc(UTILEX_DIV_ROUND_UP(total_key_width, SAL_UINT32_NOF_BITS), sizeof(uint32));
  unsigned int   key_chunks             = calc_nof_arr_entries_from_width(total_key_width);


  build_virtual_db_data(0, key_field_bit_rang_arr, key, virtual_key_data);

  for (entry_index = 0; entry_index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++entry_index) {
    if ( 1 == lpm_cache->physical_entry_occupation[entry_index] ) {
      for (i = 0; i < key_chunks; ++i)
        virtual_key_data[i] = lpm_cache->lpm_key_entry_arr[entry_index][i];
      update_key_with_prefix_length(lpm_cache->lpm_key_entry_prefix_length_arr[entry_index], total_key_width, virtual_key_data);
      if (copmare_two_virtual_keys(total_key_width, lpm_cache->lpm_key_entry_arr[entry_index], virtual_key_data))
      {
        free(virtual_key_data);
        return entry_index;
      }
    }
  }
  free(virtual_key_data);
  return -1;
}

/*Copy the key and result field into pem_mem_access structures*/
int get_em_data_by_entry(const int db_id, const int index, pemladrv_mem_t *key, pemladrv_mem_t *result) {

  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(api_info.db_em_container.db_em_info_arr[db_id].em_cache.em_result_entry_arr[index], 
                                                           api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_field_bit_range_arr, 
                                                           result);
  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(api_info.db_em_container.db_em_info_arr[db_id].em_cache.em_key_entry_arr[index],    
                                                           api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_field_bit_range_arr,    /*Sram - because only key is needed, without mask and valid*/
                                                           key);
  return 0;
}


/*Copy the key and result field into pem_mem_access structures*/
int get_lpm_data_by_entry(const int db_id, const int index, pemladrv_mem_t *key, pemladrv_mem_t *result) {

  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(api_info.db_lpm_container.db_lpm_info_arr[db_id].lpm_cache.lpm_result_entry_arr[index], 
                                                           api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_field_bit_range_arr, 
                                                           result);
  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(api_info.db_lpm_container.db_lpm_info_arr[db_id].lpm_cache.lpm_key_entry_arr[index],    
                                                           api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_field_bit_range_arr,    /*Sram - because only key is needed, without mask and valid*/
                                                           key);
  return 0;
}


/*
This is how we find the index we will want to insert the new key. We do it with 'Similar triangles' Geometry:

                  _______________
                 |              /    
Index:  a --->   |@@@@@@@@@@@@@/  <----  prefix length Max_p
                 |            /
                 |           /
                 |          /
Index to find--->|@@@@@@@@@/      <----  prefix we want to insert. Prefix length: curr_p
                 |        /
                 |       / 
Index:  b --->   |@@@@@@/         <----  prefix length Min_p
                 |     /
                 |    /
                 |   /
                 |  /
                 | /
                 |/

                              curr_p - Min_p
              X = b - (b-a)* ---------------
                               Max_p - Min_p

*/
/*Return the index to insert key*/
int find_lpm_index_to_insert(const unsigned int prefix_length, const int key_length, LpmDbCache* lpm_cache) {
  int index;
  int a, b, max_p, min_p, curr_p;

  curr_p = prefix_length;
  a = 0;
  b = PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES - 1;
  min_p = 0;
  max_p = key_length;
  
  for(index = 0; index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES - 1; ++index) {
    if (lpm_cache->physical_entry_occupation[index] == 1) {
      if(lpm_cache->lpm_key_entry_prefix_length_arr[index] == prefix_length) {
        return find_most_suitable_index_with_matching_prefix(index, lpm_cache);
      }
      if(lpm_cache->lpm_key_entry_prefix_length_arr[index] > prefix_length) {
        a = index + 1;
        max_p = lpm_cache->lpm_key_entry_prefix_length_arr[index];
      }
      if(lpm_cache->lpm_key_entry_prefix_length_arr[index] < prefix_length) {
        b = index;
        min_p = lpm_cache->lpm_key_entry_prefix_length_arr[index];
        break;
      }
    }

  }
/*  return (int)(b - (size_t)ceil((b-a)*(curr_p - min_p)/(max_p - min_p)));*/ /*ceil!!!!*/
    return b - (b - a) * UTILEX_DIV_ROUND_UP((curr_p - min_p), (max_p - min_p));
}


/*Returns the best index that requires minimum writings to physical*/
int find_most_suitable_index_with_matching_prefix(const int index, LpmDbCache* lpm_cache) {

  int i;
  int distance_to_free_apace_above = 0;
  int distance_to_free_apace_below = 0;
  int last_index_of_same_prefix_length;
  int same_last_prefix = 1;

  unsigned int prefix_length = lpm_cache->lpm_key_entry_prefix_length_arr[index];

  last_index_of_same_prefix_length = find_last_index_with_same_prefix(index, lpm_cache->lpm_key_entry_prefix_length_arr);
  /*only one entry of same prefix
  if( index == last_index_of_same_prefix_length)
    return index;*/

  /*check distance to free entry from above*/
  for(i = index - 1; i >= 0 ; --i) {
    ++distance_to_free_apace_above;
    if (lpm_cache->physical_entry_occupation[i] == 0) {  /*Found free space*/
      if (same_last_prefix) /*in this case, no shifting is needed*/
        return i;
      break;
    }
    if ((lpm_cache->lpm_key_entry_prefix_length_arr[i] != prefix_length))
      same_last_prefix = 0;
  }
  /*in case no free space above index*/
  if(i == -1)
    return last_index_of_same_prefix_length + 1;
  /*check distance to free entry from below*/
  same_last_prefix = 1;
  for(i = last_index_of_same_prefix_length + 1; i < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES ; ++i) {
    ++distance_to_free_apace_below;
    if (lpm_cache->physical_entry_occupation[i] == 0) { /*Found free space*/
      if (same_last_prefix) /*in this case, no shifting is needed*/
        return i;
      break;
    }
    if (lpm_cache->lpm_key_entry_prefix_length_arr[i] != prefix_length)
      same_last_prefix = 0;
  }
  /*in case no free space below last index*/
  if(i == PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES)
    return index - 1;

  /*return the most suitable index*/
  if ( distance_to_free_apace_above <= distance_to_free_apace_below)
    return index;
  return last_index_of_same_prefix_length + 1;
}


/*Returns the last index with the same prefix*/
int find_last_index_with_same_prefix(const int index, unsigned int* lpm_key_entry_prefix_length_arr) {
  int i;
  for(i = index + 1; i < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++i) {
    if (lpm_key_entry_prefix_length_arr[i] != lpm_key_entry_prefix_length_arr[index])
      return i - 1;
  }
  return PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES - 1;
}


/*Check to see is lpm db is already full. Returns 1 if full and 0 otherwise*/
int is_lpm_full(const int nof_entries, unsigned char* physical_entry_occupation) {
  int is_full = 1;
  int nof_occupied_entries = 0;
  int i;

  for(i = 0; i < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++i) {
    if (physical_entry_occupation[i] == 1)
      ++nof_occupied_entries;
  }
  if (nof_occupied_entries == nof_entries)
  	return is_full;
	else
	return !is_full;
}


/*Return UP or DOWN, in the direction that minimum shifts is needed*/
int find_minimum_shifting_direction(const int index, const int table_id) {
  int i;
  int distance_to_free_apace_above = 0;
  int distance_to_free_apace_below = 0;
  int last_index_of_same_prefix_length = find_last_index_with_same_prefix(index, api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.lpm_key_entry_prefix_length_arr); 


  /*check distance to free entry from above*/
  for(i = index - 1; i >= 0 ; --i) {
    ++distance_to_free_apace_above;
    if (api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation[i] == 0)
      break;
  }
  /*in case no free space above index*/
  if(i == -1)
    return DOWN;
  /*check distance to free entry from below*/
  for(i = last_index_of_same_prefix_length + 1; i < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES ; ++i) {
    ++distance_to_free_apace_below;
    if (api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation[i] == 0)
      break;
  }
  /*in case no free space below last index*/
  if(i == PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES)
    return UP;

  /*return the most suitable index*/
  if ( distance_to_free_apace_above <= distance_to_free_apace_below)
    return UP;
  return DOWN;
}


/*Move key to new entry in db*/
void move_physical_lpm_entry(int unit, const int old_index, const int new_index, const int table_id) {
  int length_in_bits;
  pemladrv_mem_t* key     = (pemladrv_mem_t*)calloc(1, sizeof(pemladrv_mem_t));
  pemladrv_mem_t* result  = (pemladrv_mem_t*)calloc(1, sizeof(pemladrv_mem_t));
  key->nof_fields              = 1;         
  key->fields               = (pemladrv_field_t**)calloc(1,sizeof(pemladrv_field_t*));
  key->fields[0]            = (pemladrv_field_t*)calloc(1,sizeof(pemladrv_field_t));
  key->fields[0]->field_id     = 0;
  key->fields[0]->fldbuf    = (unsigned int*)calloc(2,sizeof(unsigned int));
  result->nof_fields           = 1;
  result->fields            = (pemladrv_field_t**)calloc(1,sizeof(pemladrv_field_t*));
  result->fields[0]         = (pemladrv_field_t*)calloc(1,sizeof(pemladrv_field_t));
  result->fields[0]->field_id  = 0;
  result->fields[0]->fldbuf = (unsigned int*)calloc(2,sizeof(unsigned int));


  length_in_bits = api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.lpm_key_entry_prefix_length_arr[old_index];
  

  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.lpm_key_entry_arr[old_index], 
                                                           api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.key_field_bit_range_arr,
                                                           key);


  set_pem_mem_accesss_fldbuf_from_physical_sram_data_array(api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.lpm_result_entry_arr[old_index], 
                                                           api_info.db_lpm_container.db_lpm_info_arr[table_id].logical_lpm_info.result_field_bit_range_arr,
                                                           result);

  pemladrv_lpm_remove_by_index(0, table_id, old_index);
  pem_lpm_set_by_id(unit, table_id, new_index, key, &length_in_bits, result);
  free_pemladrv_mem_struct(&key);
  free_pemladrv_mem_struct(&result);
}

/*Shift all entries in physical by proper direction*/
int shift_physical_to_clear_entry(int unit, const int shift_direction, int *index, const int table_id) {
  int next_free_entry, index_to_write_to;
  if (shift_direction == UP) {
    *index = *index - 1;                   /*Moving index up to entry that will be free after this iterations*/
    next_free_entry = find_next_lpm_free_entry(*index, UP, api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation);
    for(index_to_write_to = next_free_entry; index_to_write_to != *index; ++index_to_write_to)
      move_physical_lpm_entry(unit, index_to_write_to + 1, index_to_write_to, table_id); 
    return 0;
  }

  else if (shift_direction == DOWN) {
    next_free_entry = find_next_lpm_free_entry(*index, DOWN, api_info.db_lpm_container.db_lpm_info_arr[table_id].lpm_cache.physical_entry_occupation);
    for(index_to_write_to = next_free_entry; index_to_write_to != *index; --index_to_write_to)     /*index_to_write_to - 1 is acctually the entry index that i read from*/
      move_physical_lpm_entry(unit, index_to_write_to - 1, index_to_write_to, table_id); 
    return 0;
  }
  return 1; /*SHOULD NOT REACH THIS LINE*/
}


/*Set zeros in key bits according to prefix length*/
void update_key_with_prefix_length(const int prefix_length, const int key_length, unsigned int* virtual_key_data) {
  unsigned int zeros_arr[10] = {0};
  set_bits(zeros_arr, 0, 0, key_length - prefix_length, virtual_key_data);
}


/******************************/
/* Memory Management functions*/
/******************************/

void pem_allocate_fldbuf_by_bit_range_array(const int nof_fields, const FieldBitRange* bit_range_array, pemladrv_mem_t* pem_mem_access) {
  int index, msb_bit, lsb_bit, field_width_in_bits;

  for (index = 0; index < nof_fields; ++index) {
    if (bit_range_array[index].is_field_mapped) {
      msb_bit = bit_range_array[index].msb;
      lsb_bit = bit_range_array[index].lsb;
      field_width_in_bits = msb_bit - lsb_bit + 1;
      pemladrv_mem_alloc_field(pem_mem_access->fields[index], field_width_in_bits);
      pem_mem_access->fields[index]->field_id = index;
    }
  }
}

pemladrv_mem_t* pemladrv_mem_alloc(const unsigned int nof_fields){
  unsigned int field_index;
  pemladrv_mem_t* pem_mem_access;
  pem_mem_access = (pemladrv_mem_t*)calloc(1, sizeof(pemladrv_mem_t));

  pem_mem_access->nof_fields = (uint16)nof_fields;
  pem_mem_access->fields              = (pemladrv_field_t**)calloc(nof_fields, sizeof(pemladrv_field_t*));
  for (field_index = 0; field_index < nof_fields; ++field_index)
    pem_mem_access->fields[field_index] = (pemladrv_field_t*)calloc(1, sizeof(pemladrv_field_t));
  return pem_mem_access;
}

void pemladrv_mem_alloc_field(pemladrv_field_t* pem_field_access, const unsigned int field_width_in_bits) {
  unsigned int nof_entries = (field_width_in_bits % UINT_WIDTH_IN_BITS == 0) ? field_width_in_bits / UINT_WIDTH_IN_BITS : field_width_in_bits / UINT_WIDTH_IN_BITS + 1;
  pem_field_access->fldbuf = (unsigned int*)calloc(nof_entries, sizeof(unsigned int));
  return;
}

unsigned int pemladrv_mem_alloc_direct(const int db_id, pemladrv_mem_t** result) {
  int nof_fields;

  /*check id db was mapped in the application*/
  if (api_info.db_direct_container.db_direct_info_arr == NULL || api_info.db_direct_container.db_direct_info_arr[db_id].result_chunk_mapper_matrix_arr == NULL) {
    /*printf("Warning: PEM Db-id %d was NOT mapped during the application run.\n", db_id);*/
    return 1;
  }

  nof_fields = api_info.db_direct_container.db_direct_info_arr[db_id].nof_fields;
  *result = pemladrv_mem_alloc(nof_fields);

  pem_allocate_fldbuf_by_bit_range_array(nof_fields, api_info.db_direct_container.db_direct_info_arr[db_id].field_bit_range_arr, *result);

  return 0;
}

unsigned int pemladrv_mem_alloc_tcam(const int db_id, pemladrv_mem_t** key, pemladrv_mem_t** mask, pemladrv_mem_t** valid, pemladrv_mem_t** result) {
  int nof_fields_in_key, nof_fields_in_result;

  /*check if db was mapped in the application*/
  if (api_info.db_tcam_container.db_tcam_info_arr[db_id].result_chunk_mapper_matrix_arr == NULL || 
      api_info.db_tcam_container.db_tcam_info_arr[db_id].result_chunk_mapper_matrix_arr->db_chunk_mapper == NULL) {
    /*printf("Warning: PEM Db was NOT mapped during the application run\n")*/;
    return 1;
  }

  nof_fields_in_key    = api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_key;
  nof_fields_in_result = api_info.db_tcam_container.db_tcam_info_arr[db_id].nof_fields_in_result;
  *key    = pemladrv_mem_alloc(nof_fields_in_key);
  *mask   = pemladrv_mem_alloc(nof_fields_in_key);
  *valid  = pemladrv_mem_alloc(1);
  *result = pemladrv_mem_alloc(nof_fields_in_result);

  /*Key + Mask allocations*/
  pem_allocate_fldbuf_by_bit_range_array(nof_fields_in_key, api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr, *key);
  pem_allocate_fldbuf_by_bit_range_array(nof_fields_in_key, api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr, *mask);
  /*Valid allocating*/
  pemladrv_mem_alloc_field((*valid)->fields[0], 1);
  (*valid)->fields[0]->field_id = 0;
  /*Result allocations*/
  pem_allocate_fldbuf_by_bit_range_array(nof_fields_in_result, api_info.db_tcam_container.db_tcam_info_arr[db_id].key_field_bit_range_arr, *result);

  return 0;
}

unsigned int pemladrv_mem_alloc_lpm(const int db_id, pemladrv_mem_t** key, pemladrv_mem_t** result) {
  int nof_fields_in_key, nof_fields_in_result;

  /*check if db was mapped in the application*/
  if (api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_chunk_mapper_matrix_arr == NULL || 
      api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_chunk_mapper_matrix_arr->db_chunk_mapper == NULL) {
      /*printf("Warning: PEM Db was NOT mapped during the application run\n")*/;
      return 1;
  }
  nof_fields_in_key    = api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_key;
  nof_fields_in_result = api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.nof_fields_in_result;
  *key    = pemladrv_mem_alloc(nof_fields_in_key);
  *result = pemladrv_mem_alloc(nof_fields_in_result);

  /*Key allocations*/
  pem_allocate_fldbuf_by_bit_range_array(nof_fields_in_key,    api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.key_field_bit_range_arr, *key);
  /*Result allocations*/
  pem_allocate_fldbuf_by_bit_range_array(nof_fields_in_result, api_info.db_lpm_container.db_lpm_info_arr[db_id].logical_lpm_info.result_field_bit_range_arr, *result);

  return 0;
}

unsigned int pemladrv_mem_alloc_em(const int db_id, pemladrv_mem_t** key, pemladrv_mem_t** result) {
  int nof_fields_in_key, nof_fields_in_result;

  /*check if db was mapped in the application*/
  if (api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_chunk_mapper_matrix_arr == NULL || 
      api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_chunk_mapper_matrix_arr->db_chunk_mapper == NULL) {
      /*printf("Warning: PEM Db was NOT mapped during the application run\n")*/;
      return 1;
  }
  nof_fields_in_key    = api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_key;
  nof_fields_in_result = api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.nof_fields_in_result;
  *key    = pemladrv_mem_alloc(nof_fields_in_key);
  *result = pemladrv_mem_alloc(nof_fields_in_result);

  /*Key allocations*/
  pem_allocate_fldbuf_by_bit_range_array(nof_fields_in_key,    api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.key_field_bit_range_arr, *key);
  /*Result allocations*/                                                                                              
  pem_allocate_fldbuf_by_bit_range_array(nof_fields_in_result, api_info.db_em_container.db_em_info_arr[db_id].logical_em_info.result_field_bit_range_arr, *result);

  return 0;
}

void pemladrv_mem_free(pemladrv_mem_t* pem_mem_access) {
  unsigned int field_ndx;
  unsigned int nof_fields = pem_mem_access->nof_fields;
  for (field_ndx = 0; field_ndx < nof_fields; ++field_ndx){
    free(pem_mem_access->fields[field_ndx]->fldbuf);
  }
  free(pem_mem_access->fields);
  free(pem_mem_access);
}

/* The following are utile functions, used both in pem_logical_access and pemladrv_meminfo_init*/
/******************************************************************************************/

/*Writing '0' to valid bit in physical DB. Also, Sets the mask to ones*/
void set_valid_bit_to_zero(int unit, const int virtual_row_index, LogicalTcamInfo* tcam_info){
  int           physical_row_index;
  unsigned int  nof_tcam_chunk_cols;
  unsigned int  implementation_index, nof_implementations;
  unsigned int  entry_data[2] = {0xFFFFFFFF, 0};                 /*writing only last chunk of physical - 33 bits*/
  DbChunkMapper *chunk_mapper;
  phy_mem_t      phy_mem;

  nof_tcam_chunk_cols = get_nof_tcam_chunk_cols(tcam_info->total_key_width);
  nof_implementations = tcam_info->nof_chunk_matrices;

  /*Writing 0 to valid bit for last chunk of each implementation*/
  for (implementation_index = 0; implementation_index < nof_implementations; ++implementation_index) {
    chunk_mapper = tcam_info->key_chunk_mapper_matrix_arr[implementation_index].db_chunk_mapper[0][nof_tcam_chunk_cols-1];
    physical_row_index = calculate_physical_row_index_from_chunk_mapper(chunk_mapper, virtual_row_index);
    init_phy_mem_t_from_chunk_mapper(chunk_mapper->mem_block_id, 
      chunk_mapper->phy_mem_addr, 
      physical_row_index, 
      chunk_mapper->phy_mem_width,
      &phy_mem);
    pem_write(unit, &phy_mem, 1, entry_data);
  }
}

/* Returns number of rows in TCAM chunk mattrix*/
unsigned int get_nof_tcam_chunk_cols(const unsigned int total_key_width) {
  if (total_key_width % PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH == 0)
    return (total_key_width / PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH);
  else
    return (total_key_width / PEM_CFG_API_CAM_TCAM_CHUNK_WIDTH) + 1;
}

/* Returns the physical row index*/
int calculate_physical_row_index_from_chunk_mapper(const DbChunkMapper* chunk_mapper, const int virtual_row_index) {
  const unsigned int internal_chunk_offset = (virtual_row_index % PEM_CFG_API_MAP_CHUNK_N_ENTRIES);
  return  chunk_mapper->phy_mem_entry_offset + internal_chunk_offset;
}


/* init phy_mem_t from chunk_mapper info*/
void init_phy_mem_t_from_chunk_mapper(const unsigned int chunk_mem_block_id, 
                                      const unsigned int chunk_phy_mem_addr, 
                                      const unsigned int chunk_phy_mem_row_index, 
                                      const unsigned int chunk_phy_mem_width,
                                      phy_mem_t* phy_mem) {
  phy_mem->block_identifier   = chunk_mem_block_id;
  phy_mem->mem_address        = chunk_phy_mem_addr;
  phy_mem->mem_width_in_bits  = chunk_phy_mem_width ;
  phy_mem->reserve            = 0;
  phy_mem->row_index          = chunk_phy_mem_row_index;/*calculate_physical_row_index_from_chunk_mapper(chunk_mapper, virtual_row_index);*/
  /*phy_mem->is_mem             = is_mem;*/
}

/* Calibrate last chunk of cam based dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_cam_based_dbs() {
  zero_last_chunk_of_tcam_dbs();
  zero_last_chunk_of_lpm_dbs();
  zero_last_chunk_of_em_dbs();
}

/*Calibrate last chunk of tcam dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_tcam_dbs() {
  int i, entry_index, nof_dbs;
  nof_dbs = api_info.db_tcam_container.nof_tcam_dbs;
  for (i = 0; i < nof_dbs; ++i) {
    if (NULL == api_info.db_tcam_container.db_tcam_info_arr[i].result_chunk_mapper_matrix_arr)     /*In case db was not mapped*/
      continue;
    for (entry_index = 0; entry_index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++entry_index) 
      set_valid_bit_to_zero(0, entry_index, &api_info.db_tcam_container.db_tcam_info_arr[i]);
  }
}

/*Calibrate last chunk of lpm dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_lpm_dbs() {
  int i, entry_index, nof_dbs;
  nof_dbs = api_info.db_lpm_container.nof_lpm_dbs;
  for (i = 0; i < nof_dbs; ++i) {
    if (NULL == api_info.db_lpm_container.db_lpm_info_arr[i].logical_lpm_info.result_chunk_mapper_matrix_arr)      /*In case db was not mapped*/
      continue;
    for (entry_index = 0; entry_index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++entry_index) 
      set_valid_bit_to_zero(0, entry_index, &api_info.db_lpm_container.db_lpm_info_arr[i].logical_lpm_info);
  }
}

/*Calibrate last chunk of wm dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_em_dbs() {
  int i, entry_index, nof_dbs;
  nof_dbs = api_info.db_em_container.nof_em_dbs;
  for (i = 0; i < nof_dbs; ++i) {
    if (NULL == api_info.db_em_container.db_em_info_arr[i].logical_em_info.result_chunk_mapper_matrix_arr)      /*In case db was not mapped*/
      continue;
    for (entry_index = 0; entry_index < PEM_CFG_API_CAM_TCAM_CHUNK_N_ENTRIES; ++entry_index) 
      set_valid_bit_to_zero(0, entry_index, &api_info.db_em_container.db_em_info_arr[i].logical_em_info);
  }
}

/*Reads physical memory and updates LPM and EM cache*/
void read_physical_and_update_cache() {
     update_lpm_cache_from_physical();
     update_em_cache_from_physical();
}



/* MBIST */ 
/***********************************************/

/*global functions to use for MBIST*/
static const unsigned int nof_mbist_values = 4;
static const unsigned int gMbistVal[] = {0xFFFFFFFF, 0x0, 0xAAAAAAAA, 0x55555555};
static unsigned int get_mbist_val_by_ndx(const unsigned int mbist_val_ndx) { return gMbistVal[mbist_val_ndx]; }

static unsigned int get_nof_mapped_fields(unsigned int nof_fields, const FieldBitRange* bit_range_arr){
  unsigned int field_ndx = 0, nof_mapped_fields = 0;
  for (field_ndx = 0;field_ndx < nof_fields; ++field_ndx) {
    if (bit_range_arr[field_ndx].is_field_mapped)
      ++nof_mapped_fields;
  }

  return nof_mapped_fields;
}

int  allocate_pemladrv_mem_struct(pemladrv_mem_t** mem_access, unsigned int nof_fields, const FieldBitRange* bit_range_arr){
  unsigned int field_ndx, mapped_field_ndx, nof_bits_in_field, nof_entries_in_fldbuf;
  unsigned int nof_bits_in_uint32 = 8*sizeof(uint32);
  unsigned int fld_buf_entry_ndx;
  unsigned int nof_mapped_fields = get_nof_mapped_fields(nof_fields, bit_range_arr);

  /*allocate struct*/
  *mem_access = (pemladrv_mem_t*)malloc(sizeof(pemladrv_mem_t));
  if (*mem_access == NULL)
    return -1;
  (*mem_access)->nof_fields = (uint16)nof_mapped_fields;
  (*mem_access)->flags = 0;
  (*mem_access)->fields = (pemladrv_field_t**)malloc(nof_mapped_fields * sizeof(pemladrv_field_t*));
  if ((*mem_access)->fields == NULL) 
    return -1;
  for (field_ndx = 0, mapped_field_ndx = 0; field_ndx < nof_fields; ++field_ndx){
    if (!bit_range_arr[field_ndx].is_field_mapped) {      
      continue;
    }
    (*mem_access)->fields[mapped_field_ndx] = (pemladrv_field_t*)malloc(sizeof(pemladrv_field_t));
    if ((*mem_access)->fields[mapped_field_ndx] == NULL)
      return -1;
    /*calculate nof entries in field array*/
    nof_bits_in_field = bit_range_arr[field_ndx].msb - bit_range_arr[field_ndx].lsb + 1;
    nof_entries_in_fldbuf = (nof_bits_in_field % nof_bits_in_uint32 == 0 && nof_bits_in_field > 0) ? nof_bits_in_field / nof_bits_in_uint32 : nof_bits_in_field / nof_bits_in_uint32 + 1;
    
    /*allocate array*/
    (*mem_access)->fields[mapped_field_ndx]->field_id = field_ndx;
    (*mem_access)->fields[mapped_field_ndx]->flags = 0;
    (*mem_access)->fields[mapped_field_ndx]->fldbuf = (uint32*)malloc(sizeof(uint32) * nof_entries_in_fldbuf);
    if (NULL == (*mem_access)->fields[mapped_field_ndx]->fldbuf)
      return -1;
    for (fld_buf_entry_ndx = 0; fld_buf_entry_ndx < nof_entries_in_fldbuf; ++fld_buf_entry_ndx)
      (*mem_access)->fields[mapped_field_ndx]->fldbuf[fld_buf_entry_ndx] = 0;

    ++mapped_field_ndx;
  }

  return 0;
}
void free_pemladrv_mem_struct(pemladrv_mem_t** mem_access){
  unsigned int field_ndx;
  unsigned int nof_fields = (*mem_access)->nof_fields;
  for (field_ndx = 0; field_ndx < nof_fields; ++field_ndx){
    free((*mem_access)->fields[field_ndx]->fldbuf);
    free((*mem_access)->fields[field_ndx]);
  }
  free((*mem_access)->fields);
  free(*mem_access);
}
static void set_pem_mem_access_with_value(pemladrv_mem_t* mem_access, unsigned int nof_fields, FieldBitRange* bit_range_arr, unsigned int field_val){
  unsigned int field_ndx, mapped_field_ndx, nof_bits_in_field, nof_bits_in_last_entry, nof_entries_in_fldbuf, entry_ndx_in_field;
  unsigned int nof_bits_in_uint32 = 8 * sizeof(uint32);
  unsigned int bit_mask;
  
  for (field_ndx = 0, mapped_field_ndx = 0; field_ndx < nof_fields; ++field_ndx){
    if (!bit_range_arr[field_ndx].is_field_mapped)
      continue;
    /*calculate nof entries in field array*/
    nof_bits_in_field = bit_range_arr[field_ndx].msb - bit_range_arr[field_ndx].lsb + 1;
    nof_bits_in_last_entry = (nof_bits_in_field % nof_bits_in_uint32 == 0 && nof_bits_in_field > 0) ? nof_bits_in_uint32 : nof_bits_in_field % nof_bits_in_uint32;
    nof_entries_in_fldbuf = (nof_bits_in_field % nof_bits_in_uint32 == 0 && nof_bits_in_field > 0) ? nof_bits_in_field / nof_bits_in_uint32 : nof_bits_in_field / nof_bits_in_uint32 + 1;
    
    for (entry_ndx_in_field = 0; entry_ndx_in_field < nof_entries_in_fldbuf; ++entry_ndx_in_field){
      mem_access->fields[mapped_field_ndx]->fldbuf[entry_ndx_in_field] = field_val;     
    }
    /*zero upper bits of last entry*/
    bit_mask = 0xffffffff;
    bit_mask >>= (nof_bits_in_uint32 - nof_bits_in_last_entry);
    mem_access->fields[mapped_field_ndx]->fldbuf[entry_ndx_in_field-1] &= bit_mask;

    ++mapped_field_ndx;
  }
}
static int  compare_pem_mem_access(pemladrv_mem_t* mem_access_a, pemladrv_mem_t* mem_access_b, unsigned int nof_fields, FieldBitRange* bit_range_arr){
  unsigned int field_ndx, entry_ndx_in_field, nof_bits_in_field, nof_entries_in_fldbuf;
  unsigned int nof_bits_in_uint32 = 8*sizeof(uint32), error = 0;
  unsigned int mapped_field_ndx = 0;
 
  for (field_ndx = 0, mapped_field_ndx = 0; field_ndx < nof_fields; ++field_ndx){
    if (!bit_range_arr[field_ndx].is_field_mapped)
      continue;
    /*calculate nof entries in field array*/
    nof_bits_in_field = bit_range_arr[field_ndx].msb - bit_range_arr[field_ndx].lsb + 1;
    nof_entries_in_fldbuf = (nof_bits_in_field % nof_bits_in_uint32 == 0 && nof_bits_in_field > 0) ? nof_bits_in_field / nof_bits_in_uint32 : nof_bits_in_field / nof_bits_in_uint32 + 1;
    
    for (entry_ndx_in_field = 0; entry_ndx_in_field < nof_entries_in_fldbuf; ++entry_ndx_in_field){
      error |= (mem_access_a->fields[mapped_field_ndx]->fldbuf[entry_ndx_in_field] != mem_access_b->fields[mapped_field_ndx]->fldbuf[entry_ndx_in_field]);
    }

    ++mapped_field_ndx;
  }

  return error;
}


/*Direct DB MBIST*/
static int create_pemladrv_mem_struct_db_direct(pemladrv_mem_t** mem_access, LogicalDirectInfo* db_info){
  return allocate_pemladrv_mem_struct(mem_access, db_info->nof_fields, db_info->field_bit_range_arr);
}
static void set_mbist_value_to_field_db_direct(pemladrv_mem_t* mem_access, LogicalDirectInfo* db_info, unsigned int field_val){
  unsigned int nof_fields = db_info->nof_fields;

  set_pem_mem_access_with_value(mem_access, nof_fields, db_info->field_bit_range_arr, field_val);
}
static int compare_pemladrv_mem_structs_db_direct(pemladrv_mem_t* mem_access_a, pemladrv_mem_t* mem_access_b, LogicalDirectInfo* db_info){
  return compare_pem_mem_access(mem_access_a, mem_access_b, db_info->nof_fields, db_info->field_bit_range_arr);
}
static int pem_mem_direct_db_virtual_bist(){
  unsigned int db_ndx, entry_ndx, nof_entries, mbist_val_ndx;
  unsigned int nof_direct_dbs = api_info.db_direct_container.nof_direct_dbs;
  LogicalDirectInfo* db_info;
  pemladrv_mem_t* data_to_write_to_design;
  pemladrv_mem_t* data_read_from_design;
  int error = 0;

  /*iterate over all direct dbs*/
  for (db_ndx = 0; db_ndx < nof_direct_dbs; ++db_ndx){
    db_info = &(api_info.db_direct_container.db_direct_info_arr[db_ndx]);
    nof_entries = db_info->total_nof_entries;
    if (!is_db_mapped_to_pes(db_info->result_chunk_mapper_matrix_arr))
      continue;
    /*write and then read content to all db entries
      create pemladrv_mem_structs*/
    create_pemladrv_mem_struct_db_direct(&data_to_write_to_design, db_info);
    create_pemladrv_mem_struct_db_direct(&data_read_from_design,   db_info);
    set_mbist_value_to_field_db_direct(data_read_from_design, db_info, 0x0);
    for (entry_ndx = 0; entry_ndx < nof_entries; ++entry_ndx){
      /*write 4 mbist values, then read and compare*/
      for (mbist_val_ndx = 0; mbist_val_ndx < nof_mbist_values; ++mbist_val_ndx){
      set_mbist_value_to_field_db_direct(data_to_write_to_design, db_info, get_mbist_val_by_ndx(mbist_val_ndx));
      pemladrv_direct_write(0, db_ndx, entry_ndx, data_to_write_to_design);
      pemladrv_direct_read(0,  db_ndx, entry_ndx, data_read_from_design);
      if (compare_pemladrv_mem_structs_db_direct(data_to_write_to_design, data_read_from_design, db_info))
        error = 1;
      }
    }
    free_pemladrv_mem_struct(&data_to_write_to_design);
    free_pemladrv_mem_struct(&data_read_from_design);   
  }

  return error;
}

/*TCAM DB MBIST*/
static int create_pemladrv_mem_struct_db_tcam(pemladrv_mem_t** key, pemladrv_mem_t** mask, pemladrv_mem_t** valid, pemladrv_mem_t** result ,LogicalTcamInfo* db_info){
  unsigned int nof_fields_in_key = db_info->nof_fields_in_key;
  unsigned int nof_fields_in_result = db_info->nof_fields_in_result;
  unsigned int ret_val = 0;
  FieldBitRange valid_bit_range;

  valid_bit_range.lsb = 0;
  valid_bit_range.msb = 0;
  valid_bit_range.is_field_mapped = 1;
 
  /*set key and mask fields*/
  ret_val = allocate_pemladrv_mem_struct(key,    nof_fields_in_key,    db_info->key_field_bit_range_arr);
  ret_val = allocate_pemladrv_mem_struct(mask,   nof_fields_in_key,    db_info->key_field_bit_range_arr);
  ret_val = allocate_pemladrv_mem_struct(result, nof_fields_in_result, db_info->result_field_bit_range_arr);
  ret_val = allocate_pemladrv_mem_struct(valid,  1, &valid_bit_range);

  return ret_val;
}
static void set_mbist_value_to_field_db_tcam(pemladrv_mem_t* key, pemladrv_mem_t* mask, pemladrv_mem_t* valid,pemladrv_mem_t* result ,LogicalTcamInfo* db_info, unsigned int field_val){
  unsigned int nof_fields_in_key = db_info->nof_fields_in_key;
  unsigned int nof_fields_in_result = db_info->nof_fields_in_result;
  FieldBitRange valid_bit_range;

  valid_bit_range.lsb = 0;
  valid_bit_range.msb = 0;
  valid_bit_range.is_field_mapped = 1;

  /*set key and mask fields*/
  set_pem_mem_access_with_value(key,    nof_fields_in_key,    db_info->key_field_bit_range_arr, field_val);
  set_pem_mem_access_with_value(mask,   nof_fields_in_key,    db_info->key_field_bit_range_arr, field_val);
  set_pem_mem_access_with_value(result, nof_fields_in_result, db_info->result_field_bit_range_arr, field_val);
  set_pem_mem_access_with_value(valid,  1, &valid_bit_range, 1);
}
static int compare_pemladrv_mem_structs_db_tcam(pemladrv_mem_t* key_write,    pemladrv_mem_t* key_read,
                                                  pemladrv_mem_t* mask_write,   pemladrv_mem_t* mask_read,
                                                  pemladrv_mem_t* result_write, pemladrv_mem_t* result_read,
                                                  pemladrv_mem_t* valid_write,  pemladrv_mem_t* valid_read, LogicalTcamInfo* db_info){
  unsigned int nof_fields_in_key = db_info->nof_fields_in_key;
  unsigned int nof_fields_in_result = db_info->nof_fields_in_result;
  unsigned int ret_val = 0;
  FieldBitRange valid_bit_range;

  valid_bit_range.lsb = 0;
  valid_bit_range.msb = 0;
  valid_bit_range.is_field_mapped = 1;

  /*set key and mask fields*/
  ret_val |= compare_pem_mem_access(key_write,    key_read,    nof_fields_in_key,    db_info->key_field_bit_range_arr);
  ret_val |= compare_pem_mem_access(mask_write,   mask_read,   nof_fields_in_key,    db_info->key_field_bit_range_arr);
  ret_val |= compare_pem_mem_access(result_write, result_read, nof_fields_in_result, db_info->result_field_bit_range_arr);
  ret_val |= compare_pem_mem_access(valid_write,  valid_read,  1,                    &valid_bit_range);

  return ret_val;
}
static void free_pemladrv_mem_structs_db_tcam(pemladrv_mem_t** key, pemladrv_mem_t** mask, pemladrv_mem_t** valid, pemladrv_mem_t** result)
{
  free_pemladrv_mem_struct(key);
  free_pemladrv_mem_struct(mask);
  free_pemladrv_mem_struct(valid);
  free_pemladrv_mem_struct(result);
}
static int pem_mem_tcam_db_virtual_bist(){
  unsigned int db_ndx, entry_ndx, nof_entries, mbist_val_ndx;
  unsigned int nof_dbs = api_info.db_tcam_container.nof_tcam_dbs;
  LogicalTcamInfo* db_info;
  pemladrv_mem_t *key_write, *mask_write, *result_write, *valid_write;
  pemladrv_mem_t *key_read, *mask_read, *result_read, *valid_read;
  int error = 0;

  /*iterate over all dbs*/
  for (db_ndx = 0; db_ndx < nof_dbs; ++db_ndx){
    db_info = &(api_info.db_tcam_container.db_tcam_info_arr[db_ndx]);
    nof_entries = db_info->total_nof_entries;
    if (!is_db_mapped_to_pes(db_info->result_chunk_mapper_matrix_arr))
      continue;
    /*write and then read content to all db entries
      create pemladrv_mem_structs*/
    create_pemladrv_mem_struct_db_tcam(&key_write, &mask_write, &valid_write, &result_write, db_info);
    create_pemladrv_mem_struct_db_tcam(&key_read, &mask_read, &valid_read, &result_read, db_info);
    set_mbist_value_to_field_db_tcam(key_read, mask_read, valid_read, result_read, db_info, 0x0);
    for (entry_ndx = 0; entry_ndx < nof_entries; ++entry_ndx){
      /*write 4 mbist values, then read and compare*/
      for (mbist_val_ndx = 0; mbist_val_ndx < nof_mbist_values; ++mbist_val_ndx){
      set_mbist_value_to_field_db_tcam(key_write, mask_write, valid_write, result_write, db_info, get_mbist_val_by_ndx(mbist_val_ndx));
      valid_read->fields[0]->fldbuf[0] = 0;
      pemladrv_tcam_write(0, db_ndx, entry_ndx, key_write, mask_write, valid_write, result_write);  
      pemladrv_tcam_read(0, db_ndx, entry_ndx, key_read, mask_read, valid_read, result_read); 
      if (compare_pemladrv_mem_structs_db_tcam(key_write, key_read, mask_write, mask_read, result_write, result_read, valid_write, valid_read, db_info))
        error = 1;      
      }
      /*disable the tcam line*/
      valid_write->fields[0]->fldbuf[0] = 0;
      pemladrv_tcam_write(0, db_ndx, entry_ndx, key_write, mask_write, valid_write, result_write);  
    }
    free_pemladrv_mem_structs_db_tcam(&key_write, &mask_write, &valid_write, &result_write);
    free_pemladrv_mem_structs_db_tcam(&key_read, &mask_read, &valid_read, &result_read);
  }

  return error;
}


/*EM DB MBIST + LPM DB MBIST*/
static int create_pemladrv_mem_struct_db_em_lpm(pemladrv_mem_t** key, pemladrv_mem_t** result ,LogicalTcamInfo* db_info){
  unsigned int nof_fields_in_key = db_info->nof_fields_in_key;
  unsigned int nof_fields_in_result = db_info->nof_fields_in_result;
  unsigned int ret_val = 0;

  /*set key and mask fields*/
  ret_val = allocate_pemladrv_mem_struct(key,    nof_fields_in_key,    db_info->key_field_bit_range_arr);
  ret_val = allocate_pemladrv_mem_struct(result, nof_fields_in_result, db_info->result_field_bit_range_arr);

  return ret_val;
}
static void set_mbist_value_to_field_db_em_lpm(pemladrv_mem_t* key, pemladrv_mem_t* result ,LogicalTcamInfo* db_info, unsigned int field_val){
  unsigned int nof_fields_in_key = db_info->nof_fields_in_key;
  unsigned int nof_fields_in_result = db_info->nof_fields_in_result;
 
  /*set key and mask fields*/
  set_pem_mem_access_with_value(key,    nof_fields_in_key,    db_info->key_field_bit_range_arr, field_val);
  set_pem_mem_access_with_value(result, nof_fields_in_result, db_info->result_field_bit_range_arr, field_val);
}
static int compare_pemladrv_mem_structs_db_em_lpm(pemladrv_mem_t* key_write,    pemladrv_mem_t* key_read,                                                  
                                                  pemladrv_mem_t* result_write, pemladrv_mem_t* result_read, LogicalTcamInfo* db_info){
  unsigned int nof_fields_in_key = db_info->nof_fields_in_key;
  unsigned int nof_fields_in_result = db_info->nof_fields_in_result;
  unsigned int ret_val = 0;

  /*set key and mask fields*/
  ret_val = compare_pem_mem_access(key_write,    key_read,    nof_fields_in_key,    db_info->key_field_bit_range_arr);  
  ret_val = compare_pem_mem_access(result_write, result_read, nof_fields_in_result, db_info->result_field_bit_range_arr);

  return ret_val;
}
static void free_pemladrv_mem_structs_db_em_lpm(pemladrv_mem_t** key, pemladrv_mem_t** result)
{
  free_pemladrv_mem_struct(key);
  free_pemladrv_mem_struct(result);
}
static int pem_mem_em_db_virtual_bist(){
  unsigned int db_ndx, entry_ndx, nof_entries, mbist_val_ndx;
  unsigned int nof_dbs = api_info.db_em_container.nof_em_dbs;
  LogicalTcamInfo* db_info;
  pemladrv_mem_t *key_write, *result_write;
  pemladrv_mem_t *key_read, *result_read;
  int error = 0;

  /*iterate over all dbs*/
  for (db_ndx = 0; db_ndx < nof_dbs; ++db_ndx){
    db_info = &(api_info.db_em_container.db_em_info_arr[db_ndx].logical_em_info);
    nof_entries = db_info->total_nof_entries;
    if (!is_db_mapped_to_pes(db_info->result_chunk_mapper_matrix_arr))
      continue;
    /*write and then read content to all db entries
      create pemladrv_mem_structs*/
    create_pemladrv_mem_struct_db_em_lpm(&key_write, &result_write, db_info);
    create_pemladrv_mem_struct_db_em_lpm(&key_read, &result_read, db_info);
    set_mbist_value_to_field_db_em_lpm(key_read, result_read, db_info, 0x0);
    for (entry_ndx = 0; entry_ndx < nof_entries; ++entry_ndx){
      /*write 4 mbist values, then read and compare*/
      for (mbist_val_ndx = 0; mbist_val_ndx < nof_mbist_values; ++mbist_val_ndx){
      set_mbist_value_to_field_db_em_lpm(key_write, result_write, db_info, get_mbist_val_by_ndx(mbist_val_ndx));
      pemladrv_em_entry_set_by_id(0, db_ndx, entry_ndx, key_write, result_write);  
      pemladrv_em_entry_get_by_id(0, db_ndx, entry_ndx, key_read, result_read); 
      if (compare_pemladrv_mem_structs_db_em_lpm(key_write, key_read, result_write, result_read, db_info))
        error = 1;

      pemladrv_em_remove_by_index(0, db_ndx, entry_ndx);
      }      
    }
    free_pemladrv_mem_structs_db_em_lpm(&key_write, &result_write);
    free_pemladrv_mem_structs_db_em_lpm(&key_read, &result_read);    
  }

  return error;
}
static int pem_mem_lpm_db_virtual_bist(){
  unsigned int db_ndx, entry_ndx, nof_entries, mbist_val_ndx;
  unsigned int nof_dbs = api_info.db_lpm_container.nof_lpm_dbs;
  LogicalTcamInfo* db_info;
  pemladrv_mem_t *key_write, *result_write;
  pemladrv_mem_t *key_read, *result_read;
  int length_in_bits_write, length_in_bits_read;
  int error = 0;

  /*iterate over all dbs*/
  for (db_ndx = 0; db_ndx < nof_dbs; ++db_ndx){
    db_info = &(api_info.db_lpm_container.db_lpm_info_arr[db_ndx].logical_lpm_info);
    nof_entries = db_info->total_nof_entries;
    length_in_bits_write = db_info->total_key_width;
    if (!is_db_mapped_to_pes(db_info->result_chunk_mapper_matrix_arr))
      continue;
    /*write and then read content to all db entries
      create pemladrv_mem_structs*/
    create_pemladrv_mem_struct_db_em_lpm(&key_write, &result_write, db_info);
    create_pemladrv_mem_struct_db_em_lpm(&key_read, &result_read, db_info);
    set_mbist_value_to_field_db_em_lpm(key_read, result_read, db_info, 0x0);
    for (entry_ndx = 0; entry_ndx < nof_entries; ++entry_ndx){
      /*write 4 mbist values, then read and compare*/
      for (mbist_val_ndx = 0; mbist_val_ndx < nof_mbist_values; ++mbist_val_ndx){
      set_mbist_value_to_field_db_em_lpm(key_write, result_write, db_info, get_mbist_val_by_ndx(mbist_val_ndx));
      pem_lpm_set_by_id(0, db_ndx, entry_ndx, key_write, &length_in_bits_write, result_write);  
      pem_lpm_get_by_id(0, db_ndx, entry_ndx, key_read, &length_in_bits_read, result_read); 
      if (compare_pemladrv_mem_structs_db_em_lpm(key_write, key_read, result_write, result_read, db_info))
        error = 1;

      pemladrv_lpm_remove_by_index(0, db_ndx, entry_ndx);
      }
    }
    free_pemladrv_mem_structs_db_em_lpm(&key_write, &result_write);
    free_pemladrv_mem_structs_db_em_lpm(&key_read, &result_read);
  }

  return error;
}


/*Registers MBIST*/

static void mask_reg_read(pemladrv_mem_t *result, const pemladrv_mem_t *result_read_bits_mask, unsigned int nof_fields, FieldBitRange* bit_range_arr) {
  unsigned int field_ndx, mapped_field_ndx, nof_bits_in_field, nof_entries_in_fldbuf, entry_ndx_in_field;
  unsigned int nof_bits_in_uint32 = 8 * sizeof(uint32);
    
  for (field_ndx = 0, mapped_field_ndx = 0; field_ndx < nof_fields; ++field_ndx){
    if (!bit_range_arr[field_ndx].is_field_mapped)
      continue;
    /*calculate nof entries in field array*/
    nof_bits_in_field = bit_range_arr[field_ndx].msb - bit_range_arr[field_ndx].lsb + 1;    
    nof_entries_in_fldbuf = (nof_bits_in_field % nof_bits_in_uint32 == 0 && nof_bits_in_field > 0) ? nof_bits_in_field / nof_bits_in_uint32 : nof_bits_in_field / nof_bits_in_uint32 + 1;
    
    for (entry_ndx_in_field = 0; entry_ndx_in_field < nof_entries_in_fldbuf; ++entry_ndx_in_field){
      result->fields[mapped_field_ndx]->fldbuf[entry_ndx_in_field] &= result_read_bits_mask->fields[mapped_field_ndx]->fldbuf[entry_ndx_in_field];     
    }   
    ++mapped_field_ndx;
  }
}

static int pem_mem_reg_virtual_bist(){
  unsigned int reg_ndx, mbist_val_ndx, nof_fields;
  unsigned int nof_regs = api_info.reg_container.nof_registers;
  LogicalRegInfo* reg_info;
  FieldBitRange* bit_range_arr;
  pemladrv_mem_t *data_write, *data_read, *mask;
  int error = 0;

  /*iterate over all registers*/
  for (reg_ndx = 0; reg_ndx < nof_regs; ++reg_ndx){
    /*get next register*/
    reg_info = &(api_info.reg_container.reg_info_arr[reg_ndx]);
    /*skip unmapped registers*/
    if (!reg_info->is_mapped)
      continue;
    nof_fields = reg_info->nof_fields;
    /*create BitRange array describing register's fields*/
    bit_range_arr = reg_info->reg_field_bit_range_arr;    
    /*allocate read and write mem_access_t*/
    allocate_pemladrv_mem_struct(&data_write, nof_fields, bit_range_arr);
    allocate_pemladrv_mem_struct(&data_read, nof_fields, bit_range_arr);
    allocate_pemladrv_mem_struct(&mask, nof_fields, bit_range_arr);
    set_pem_mem_access_with_value(data_read, nof_fields, bit_range_arr, 0x0);
    /*write 4 mbist values, then read and compare*/
    for (mbist_val_ndx = 0; mbist_val_ndx < nof_mbist_values; ++mbist_val_ndx){
      set_pem_mem_access_with_value(data_write, nof_fields, bit_range_arr, get_mbist_val_by_ndx(mbist_val_ndx));
      set_pem_mem_access_with_value(mask, nof_fields, bit_range_arr, 0x0);
      pemladrv_reg_write(0, reg_ndx, data_write);  
      pemladrv_reg_read_and_retreive_read_bits_mask(0, reg_ndx, data_read, mask);
      mask_reg_read(data_write, mask, nof_fields, bit_range_arr);
      if (compare_pem_mem_access(data_write, data_read, nof_fields, bit_range_arr))
        error = 1;
    }
    free_pemladrv_mem_struct(&data_write);
    free_pemladrv_mem_struct(&data_read);
    free_pemladrv_mem_struct(&mask);
  }

  return error;
}




int pem_mem_virtual_bist(){
  int ret_val = 0;
  ret_val |= pem_mem_direct_db_virtual_bist();
  ret_val |= pem_mem_tcam_db_virtual_bist();
  ret_val |= pem_mem_em_db_virtual_bist();
  ret_val |= pem_mem_lpm_db_virtual_bist();
  ret_val |= pem_mem_reg_virtual_bist();   

  if (ret_val){
    /*printf("PEM virtual MBIST FAILED!\n");*/
  }
  return ret_val;
}


void print_debug_pem_read_write(const char* prefix, const unsigned int address, const unsigned int length_in_bits, const unsigned int row_ndx, const unsigned int* value) {
  unsigned int nof_hex_chars;
  unsigned int mask, char_index, curr_char_loc, curr_value_ndx, curr_char_value;
  unsigned int last_char_mask = (length_in_bits % 4 == 0) ? 0xf : ((1 << (length_in_bits % 4)) - 1); /*mask for msb hex char in case too much bits are given in field_value*/
  char* hex_value = NULL;
  char curr_char;
  
 
  nof_hex_chars = (length_in_bits + 3) / 4;
  hex_value = (char*)malloc(sizeof(char)*nof_hex_chars+1);
  mask = 0xf;
  curr_char_loc = nof_hex_chars-1;
  curr_value_ndx = 0;


  /*convert to hex str */
  for (char_index = 0; char_index < nof_hex_chars; ++char_index, --curr_char_loc){
    /*ndx in value_arr is */
    curr_value_ndx = char_index / 8;
    /*reset mask for next array entry*/
    if (char_index % 8 == 0) 
      mask = 0xf;
    /*take 4 next bits and shift right to 4 lsbs*/
    curr_char_value = (value[curr_value_ndx] & mask) >> (char_index%8 * 4);
    /*for last char, mask lower bits in case not zeros*/
    if (char_index == nof_hex_chars-1)
      curr_char_value &= last_char_mask;
    /*map 4 bits to hexa char*/
    curr_char = (curr_char_value < 10) ? (char)curr_char_value + '0' :
                                         (char)curr_char_value + 'a' - 10;
    hex_value[curr_char_loc] = curr_char;
    mask = mask << 4;
  }
  hex_value[nof_hex_chars] = '\0';
  /*printf value and free mem*/
  /*printf("%s: address=32'h%X, value=%d'h%s, row_index=%d\n", prefix, address, length_in_bits, hex_value, row_ndx);*/
  free (hex_value);
}

/****************************************************************/
/*PEM Applets*/
static int compare_address_to_cache_entry(phy_mem_t* mem, const uint32* cache_data) {
  /*address is always first word in cache and then the block_id (8bits)*/
  unsigned int address = cache_data[0]; /*address is 32bits - as uint32*/
  unsigned int block_id_mask = (1<<BLOCK_ID_SIZE_IN_BITS) - 1;
  unsigned int block_id = cache_data[1] & block_id_mask;

  return (address == mem->mem_address + mem->row_index) && (block_id == mem->block_identifier);
}

static int find_entry_in_cache(phy_mem_t* mem, int start_ndx, int is_ingress0_egress1) {
  int entry_ndx, count;
  unsigned int** cache = (unsigned int**)api_info.applet_info.applet_mem_cache[is_ingress0_egress1];
  const unsigned int* cache_line;
  int nof_entries_in_mem = api_info.applet_info.nof_pending_applets[is_ingress0_egress1];   
  for (count = 0; count < nof_entries_in_mem; ++count) {
    entry_ndx = (start_ndx + count) % nof_entries_in_mem;  /*start looking from start_ndx until start_ndx-1 is reached - it is cyclic*/
    cache_line = cache[entry_ndx];
    if ( compare_address_to_cache_entry(mem, cache_line) )
      return entry_ndx;
  }

  /*if not found return -1*/
  return -1;
}
/*in cache the entry is {data,block_id,address} where address is 32 bits and block_id is 8 bits*/

static void read_data_from_applets_cache(int entry, int is_ingress0_egress1, uint32* data, int data_size_in_bits) {   
  uint32* entry_to_copy = api_info.applet_info.applet_mem_cache[is_ingress0_egress1][entry];
  uint32 temp_dest_data = 0;
  int first_entry_of_data = (ADDRESS_SIZE_IN_BITS+BLOCK_ID_SIZE_IN_BITS) / (8*sizeof(uint32));    /*first entry where data starts - after the full address*/
  int shift_left_amount = 8*sizeof(uint32) - BLOCK_ID_SIZE_IN_BITS;                             /*for next entry*/
  int shift_right_amount = (ADDRESS_SIZE_IN_BITS+BLOCK_ID_SIZE_IN_BITS) % (8*sizeof(uint32));   /*data is not alligned*/
  int source_ndx, dest_ndx, nof_bits_left_to_copy = data_size_in_bits;  
    
  /*second entry is block id (8bits) then the data starts*/
  for (source_ndx = first_entry_of_data, dest_ndx = 0; nof_bits_left_to_copy > 0; ++dest_ndx, ++source_ndx, temp_dest_data=0) {    
    temp_dest_data = entry_to_copy[source_ndx];   /*take entry, shift right by one byte and take LSB(yte) from next entry*/
    temp_dest_data >>= shift_right_amount;
    nof_bits_left_to_copy -= shift_left_amount;   /*took 24 bits from src*/
    if (nof_bits_left_to_copy > 0)    /*while not done copying, take lsb from next entry*/
      temp_dest_data |= (entry_to_copy[source_ndx+1] << shift_left_amount);
    data[dest_ndx] = temp_dest_data;  
    nof_bits_left_to_copy -= shift_right_amount;    /*took 8 more bits from next entry*/
  }  
}

static void get_address_and_block_id(unsigned long long full_address, uint32* address, uint32* block_id) {  
  *address = (uint32)full_address; /*take 32 bits*/
  full_address >>= ADDRESS_SIZE_IN_BITS;  /*shift till block_id is in the lsbs*/
  *block_id = (uint32)full_address;               /*take lsbs*/
}

static void write_to_applets_cache(int entry, int is_ingress0_egress1, uint32* data, int data_size_in_bits, unsigned long long full_address) {  
  uint32 address, block_id;
  uint32* entry_to_write = api_info.applet_info.applet_mem_cache[is_ingress0_egress1][entry];
  uint32 input_msbs_for_next_entry = 0, input_lsbs_for_curr_entry = 0;
  /*address is 40 bits. 32bits in entry #0 and 8 bits in entry #1.
   *therefore each entry in the data array has to be shifted left by 8 bits, then 24 lsbs are ored with previous entry.
   *8 msbs are set in the next entry (of 'entry_to_write') with 24 data bits from previous entry.*/
  int shift_right_amount = 8*sizeof(uint32) - BLOCK_ID_SIZE_IN_BITS;                             
  int shift_left_amount = (ADDRESS_SIZE_IN_BITS+BLOCK_ID_SIZE_IN_BITS) % (8*sizeof(uint32));   
  int source_ndx, dest_ndx, nof_uint32_entries_to_write_for_data;

  get_address_and_block_id(full_address, &address, &block_id);
  /*copy address to first entry and block id to second*/
  entry_to_write[0] = address;  /*address and uint32 are both 32 bits*/
  entry_to_write[1] = block_id;
  
  /*first entry is address. then nof entries needed is length of data + block_id_length(=8bits)*/
  nof_uint32_entries_to_write_for_data = GET_NOF_ENTRIES((data_size_in_bits+BLOCK_ID_SIZE_IN_BITS), 8*sizeof(uint32));
  /*write data after block_id*/ 
  for (source_ndx = 0, dest_ndx = 1; source_ndx < nof_uint32_entries_to_write_for_data; ++source_ndx, ++dest_ndx) {
    /*those are the msbs of prev entry. should sit in lsbs of current entry*/
    entry_to_write[dest_ndx] |= input_msbs_for_next_entry;  /*note that first iteration does nothing here.*/
    /*take 8 msbs of src current entry for next entry lsbs*/
    input_msbs_for_next_entry = data[source_ndx] >> shift_right_amount;
    /*take 24 lsbs of src current entry. those are the 24 msbs for the destination*/
    input_lsbs_for_curr_entry = data[source_ndx] << shift_left_amount;    
    entry_to_write[dest_ndx] |= input_lsbs_for_curr_entry;  /*set the 24 bits as the msb of the current entry*/
  }
}
static void build_phy_mem_struct_for_applets_flush(phy_mem_t* mem, const int is_ingress0_egress1, const int is_mem0_reg1) {
  mem->mem_address = is_mem0_reg1 ? (api_info.applet_info.applet_reg_info[is_ingress0_egress1].address) : (api_info.applet_info.applet_mem_info[is_ingress0_egress1].address);
  mem->block_identifier = is_mem0_reg1 ? (api_info.applet_info.applet_reg_info[is_ingress0_egress1].block_id) : (api_info.applet_info.applet_mem_info[is_ingress0_egress1].block_id);
  mem->mem_width_in_bits = is_mem0_reg1 ? (api_info.applet_info.applet_reg_info[is_ingress0_egress1].length_in_bits) : (api_info.applet_info.applet_mem_info[is_ingress0_egress1].length_in_bits);
  mem->row_index = 0; /*for reg it's fine. for mem, caller will update later if needed*/
}
static void wait_for_trigger_reg(const int is_ingress0_egress1) {
  unsigned int reg_val = 0;
  phy_mem_t reg_info;
  build_phy_mem_struct_for_applets_flush(&reg_info, is_ingress0_egress1, 1);
  do {
    /*read reg val until trigger bit (bit #6) is 0*/
#ifdef BCM_DNX_SUPPORT
    phy_mem_read(0, &reg_info, &reg_val);
#endif
    reg_val >>= APPLET_REG_AMOUNT_OF_PACKETS_SIZE_IN_BITS;
  } while(reg_val != 0);
}

int pem_read(int unit, phy_mem_t* mem, int is_mem, void *entry_data) {
  int is_ingress0_egress1;
  int entry_in_cache = -1;
#ifdef CMODEL_SERVER_MODE
  unsigned int data_length_in_bytes = (mem->mem_width_in_bits % 8 == 0) ? (mem->mem_width_in_bits / 8) : ((mem->mem_width_in_bits / 8) + 1 );
  int currently_writing_applets = api_info.applet_info.currently_writing_applets_bit;

  if (!currently_writing_applets) {  /*if not currently writing applets then just read from the design*/
    return cmodel_memreg_access(unit, (int)mem->block_identifier, mem->mem_address, data_length_in_bytes, is_mem, 1, entry_data);
  }
#endif

  /*getting here means the cache should be read instead of the design*/  
  is_ingress0_egress1 = is_mem_belong_to_ingress0_or_egress1(mem, 0, api_info.applet_info.meminfo_curr_array_size-1);
  entry_in_cache = find_entry_in_cache(mem, 0, is_ingress0_egress1);
  /*if not found in cache then read from design*/
  if (-1 == entry_in_cache) { 
    return phy_mem_read(unit, mem, entry_data);
  }

  /*getting here means the entry was found in the cache. read it from there.*/
  api_info.applet_info.entry_found_for_read_lately[is_ingress0_egress1] = entry_in_cache;
  read_data_from_applets_cache(entry_in_cache, is_ingress0_egress1, (uint32*)entry_data, mem->mem_width_in_bits);
  return 0;
}




int pem_write(int unit, phy_mem_t* mem, int is_mem, void *entry_data) {
  int is_ingress0_egress1;
  int entry_in_cache = -1;
  int entry_found_in_cache_lately, nof_entries_in_cache;
  unsigned long long address;
  int applet_mem_size_in_bits = api_info.applet_info.applet_mem_info[0].length_in_bits;
#ifdef CMODEL_SERVER_MODE  
  int currently_writing_applets = api_info.applet_info.currently_writing_applets_bit;
  unsigned int data_length_in_bytes = (mem->mem_width_in_bits % 8 == 0) ? (mem->mem_width_in_bits / 8) : ((mem->mem_width_in_bits / 8) + 1 );
  if (!currently_writing_applets) {  /*if not currently writing applets then just read from the design*/
    return cmodel_memreg_access(unit, (int)mem->block_identifier, mem->mem_address, data_length_in_bytes, is_mem, 0, entry_data);
  }
#endif


  /*getting here means currently writing applets. write to the cache*/
  is_ingress0_egress1 = is_mem_belong_to_ingress0_or_egress1(mem, 0, api_info.applet_info.meminfo_curr_array_size-1);
  nof_entries_in_cache = api_info.applet_info.nof_pending_applets[is_ingress0_egress1];
  entry_found_in_cache_lately = api_info.applet_info.entry_found_for_read_lately[is_ingress0_egress1];
  /*assume just read from the entry if found. so start looking from same entry. if too big, start from 0.*/
  entry_found_in_cache_lately = (entry_found_in_cache_lately > nof_entries_in_cache-1) ? 0 : entry_found_in_cache_lately;  
  entry_in_cache = find_entry_in_cache(mem, entry_found_in_cache_lately, is_ingress0_egress1);
    
  /*if not found in cache then allocate a new line*/
  if (-1 == entry_in_cache) {
    assert(nof_entries_in_cache < APPLET_MEM_NOF_ENTRIES);  /*make sure not writing too many entries*/    
    api_info.applet_info.applet_mem_cache[is_ingress0_egress1][nof_entries_in_cache] = (uint32*)calloc(applet_mem_size_in_bits/(sizeof(uint32)+1), sizeof(uint32)+1);
    entry_in_cache = api_info.applet_info.nof_pending_applets[is_ingress0_egress1]++;
  }
  /*write to the line - either found in cache or allocated a new line*/
  address = mem->block_identifier;
  address <<= ADDRESS_SIZE_IN_BITS;
  address |= (unsigned long long)(mem->mem_address);
  address += mem->row_index;
  write_to_applets_cache(entry_in_cache, is_ingress0_egress1, (uint32*)entry_data, mem->mem_width_in_bits, address);

  return 0;
}

int start_writing_applets() {
  int applets_bit_state = api_info.applet_info.currently_writing_applets_bit;
  /*wait for both triggers (ingress and ingress) to be idle(=0)*/ 
  wait_for_trigger_reg(0);
  wait_for_trigger_reg(1);

  /*make sure not calling the function twice before calling 'flush_applets()*/
  assert(0 == applets_bit_state);
  api_info.applet_info.currently_writing_applets_bit = 1;
  
  return (!applets_bit_state);  /*error returned if already on*/
}

void flush_applets_to_single_mem(const int is_ingress0_egress1) {
  unsigned int nof_entries_to_write = api_info.applet_info.nof_pending_applets[is_ingress0_egress1];
  unsigned int entry_ndx;
  unsigned int applet_reg_val;
  phy_mem_t reg_info, mem_info;  
  
  if (nof_entries_to_write == 0)  /*nothing to write for this mem*/
    return;
 
  /*build structures for physical access*/
  build_phy_mem_struct_for_applets_flush(&reg_info, is_ingress0_egress1, 1);
  build_phy_mem_struct_for_applets_flush(&mem_info, is_ingress0_egress1, 0);

/*write first entry twice due to bug in PeMap*/
#ifdef BCM_DNX_SUPPORT
    phy_mem_write(0, &mem_info, api_info.applet_info.applet_mem_cache[is_ingress0_egress1][0]);
#endif /*BCM_DNX_SUPPORT*/
    mem_info.row_index++;

  /*write all entries from buffer to applets memory*/
  for (entry_ndx = 0; entry_ndx < nof_entries_to_write; ++entry_ndx) {
    /*write entry to mem and increment row_index*/
#ifdef BCM_DNX_SUPPORT
    phy_mem_write(0, &mem_info, api_info.applet_info.applet_mem_cache[is_ingress0_egress1][entry_ndx]);
#endif /*BCM_DNX_SUPPORT*/
    mem_info.row_index++;
    free(api_info.applet_info.applet_mem_cache[is_ingress0_egress1][entry_ndx]); /*free cache*/
  }
  /*write reg to trigger applets write to PEM*/  
  /*nof entries + 1 --> because first entry is written twice*/
  applet_reg_val = ((1<<7) | (nof_entries_to_write+1));   /*{trigger_bit,nof_entries(6b)}*/
#ifdef BCM_DNX_SUPPORT
  phy_mem_write(0, &reg_info, &applet_reg_val);
#endif /*BCM_DNX_SUPPORT*/
  api_info.applet_info.nof_pending_applets[is_ingress0_egress1] = 0;
}

int flush_applets() {
  int applets_bit_state = api_info.applet_info.currently_writing_applets_bit;

  /*make sure not calling the function twice before calling 'flush_applets()*/
  assert(1 == applets_bit_state);
  if (applets_bit_state == 0)
    return 1; /*cannot flush applets as none written*/

  /*write the applets to the physical memories if needed*/
  flush_applets_to_single_mem(0);
  flush_applets_to_single_mem(1);
  /*turn bit indicating writing applets off*/
  api_info.applet_info.currently_writing_applets_bit = 0;   

  return 0;
}
 
int is_mem_belong_to_ingress0_or_egress1(phy_mem_t* mem, int min_ndx, int max_ndx) {
  int middle;
  unsigned int search_address; 
  unsigned int last_stage_of_ingress = api_info.applet_info.last_stage_of_ingress;
  /*address to compare is {block_id, 16msbs of address}*/
  search_address = (mem->mem_address >> 16);
  search_address |= (mem->block_identifier << 16);

  assert(max_ndx >= min_ndx);

  middle = min_ndx + ((max_ndx - min_ndx) / 2);
  if (api_info.applet_info.meminfo_array_for_applet[middle]->address > search_address)
    return is_mem_belong_to_ingress0_or_egress1(mem, min_ndx, middle - 1);   /*look in lower half*/
  else if (api_info.applet_info.meminfo_array_for_applet[middle]->address < search_address)
    return is_mem_belong_to_ingress0_or_egress1(mem, middle + 1, max_ndx);   /*look in upper half*/
  else   /*found. check if ingress or egress an return - 0 for ingress, 1 for egress*/
    return (api_info.applet_info.meminfo_array_for_applet[middle]->stage_relevant_for > last_stage_of_ingress);
}


#endif /* DB_OBJECT_ACCESS */


