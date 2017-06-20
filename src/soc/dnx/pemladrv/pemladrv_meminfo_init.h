/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _PEMLADRV_MEMINFO_INIT_H_
#define _PEMLADRV_MEMINFO_INIT_H_

#include <stdio.h>
#include "pemladrv_logical_access.h"
/*
 * The following function read (and parse) data-bases memory definition.
 * This memory definition is one of the PEM-compiler outputs.
 *
 * This function MUST be called prior to any db access.
 */


#define START_COMMENT              "/*"
#define END_COMMENT                "*/"
#define COMMENT_MARK_SIZE          2


/**********************************************
 *         Database initial information
 **********************************************/



#define KEYWORD_DB_INFO                                        "DB_INFO"
#define KEYWORD_DB_INFO_SIZE                                   sizeof("DB_INFO") - 1
#define KEYWORD_KEY_INFO                                       "KEY_FIELD"
#define KEYWORD_KEY_INFO_SIZE                                  sizeof("KEY_FIELD") - 1
#define KEYWORD_RESULT_INFO                                    "RESULT_FIELD"
#define KEYWORD_RESULT_INFO_SIZE                               sizeof("RESULT_FIELD") - 1
#define KEYWORD_VIRTUAL_REGISTER_MAPPING                       "VIRTUAL_REGISTER_MAPPING"
#define KEYWORD_REGISTER_INFO_SIZE                             sizeof("VIRTUAL_REGISTER_MAPPING") - 1  
#define KEYWORD_VIRTUAL_REGISTER_NOF_FIELDS                    "VIRTUAL_REGISTER_NOF_FIELDS"
#define KEYWORD_VIRTUAL_REGISTER_NOF_FIELDS_SIZE               sizeof("VIRTUAL_REGISTER_NOF_FIELDS") - 1
#define KEYWORD_VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS            "VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS"
#define KEYWORD_VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS_SIZE       sizeof("VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS") - 1
#define KEYWORD_PEM_APPLET_REG                                 "APPLET_REG_INFO"
#define KEYWORD_PEM_APPLET_REG_SIZE                            sizeof(KEYWORD_PEM_APPLET_REG) - 1
#define KEYWORD_PEM_APPLET_MEM                                 "APPLET_MEM_INFO"
#define KEYWORD_PEM_APPLET_MEM_SIZE                            sizeof(KEYWORD_PEM_APPLET_MEM) - 1
#define KEYWORD_GENERAL_INFO_FOR_APPLET                        "GENERAL_INFO_FOR_APPLET"
#define KEYWORD_GENERAL_INFO_FOR_APPLET_SIZE                   sizeof(KEYWORD_GENERAL_INFO_FOR_APPLET) - 1
#define KEYWORD_MEMINFO_FOR_APPLET                             "MEMINFO_FOR_APPLET"
#define KEYWORD_MEMINFO_FOR_APPLET_SIZE                        sizeof(KEYWORD_MEMINFO_FOR_APPLET) - 1 
#define KEYWORD_DB_DIRECT_INFO                                 "DB_DIRECT_MAPPING_INFO"
#define KEYWORD_DB_DIRECT_INFO_SIZE                            sizeof("DB_DIRECT_MAPPING_INFO") - 1
#define KEYWORD_DB_TCAM_KEY_INFO                               "DB_TCAM_KEY_MAPPING_INFO"
#define KEYWORD_DB_TCAM_KEY_INFO_SIZE                          sizeof("DB_TCAM_KEY_MAPPING_INFO") - 1
#define KEYWORD_DB_TCAM_RESULT_INFO                            "DB_TCAM_RESULT_MAPPING_INFO"
#define KEYWORD_DB_TCAM_RESULT_INFO_SIZE                       sizeof("DB_TCAM_RESULT_MAPPING_INFO") - 1
#define KEYWORD_DB_EXACT_MATCH_KEY_INFO                        "DB_EXACT_MATCH_KEY_MAPPING_INFO"
#define KEYWORD_DB_EXACT_MATCH_KEY_INFO_SIZE                   sizeof("DB_EXACT_MATCH_KEY_MAPPING_INFO") - 1
#define KEYWORD_DB_EXACT_MATCH_RESULT_INFO                     "DB_EXACT_MATCH_RESULT_MAPPING_INFO"
#define KEYWORD_DB_EXACT_MATCH_RESULT_INFO_SIZE                sizeof("DB_EXACT_MATCH_RESULT_MAPPING_INFO") - 1
#define KEYWORD_DB_LONGEST_PERFIX_MATCH_KEY_INFO               "DB_LONGEST_PREFIX_MATCH_KEY_MAPPING_INFO"
#define KEYWORD_DB_LONGEST_PERFIX_MATCH_KEY_INFO_SIZE          sizeof("DB_LONGEST_PREFIX_MATCH_KEY_MAPPING_INFO") - 1
#define KEYWORD_REG_AND_DBS_NUM_INFO                          "REG_AND_DBS_NUM_INFO" 
#define KEYWORD_REG_AND_DBS_NUM_INFO_SIZE                     sizeof("REG_AND_DBS_NUM_INFO") - 1
#define KEYWORD_DB_LONGEST_PERFIX_MATCH_RESULT_INFO            "DB_LONGEST_PREFIX_MATCH_RESULT_MAPPING_INFO"
#define KEYWORD_DB_LONGEST_PERFIX_MATCH_RESULT_INFO_SIZE       sizeof("DB_LONGEST_PREFIX_MATCH_RESULT_MAPPING_INFO") - 1


#define KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO                     "DB_SINGLE_VIRT_DB_MAP_INFO"
#define KEYWORD_DB_SINGLE_VIRT_DB_MAP_INFO_SIZE                sizeof("DB_SINGLE_VIRT_DB_MAP_INFO") - 1
#define KEYWORD_DB_DIRECT                                      "DIRECT"
#define KEYWORD_DB_DIRECT_SIZE                                 sizeof("DIRECT") - 1
#define KEYWORD_DB_TCAM                                        "TCAM"
#define KEYWORD_DB_TCAM_SIZE                                   sizeof("TCAM") - 1
#define KEYWORD_DB_EM                                          "EM"
#define KEYWORD_DB_EM_SIZE                                     sizeof("EM") - 1
#define KEYWORD_DB_LPM                                         "LPM"
#define KEYWORD_DB_LPM_SIZE                                    sizeof("LPM") - 1

#define DBS_WRITE_COMMNADS_FILE                                "lab_commands.txt"
#define KEYWORD_DB_DIRECT_WRITE_COMMAND                        "DB_DIRECT_WRITE_COMMAND"
#define KEYWORD_DB_DIRECT_WRITE_COMMAND_SIZE                    sizeof(KEYWORD_DB_DIRECT_WRITE_COMMAND) - 1
#define KEYWORD_DB_TCAM_WRITE_COMMAND                          "DB_TCAM_WRITE_COMMAND"
#define KEYWORD_DB_TCAM_WRITE_COMMAND_SIZE                      sizeof(KEYWORD_DB_TCAM_WRITE_COMMAND) - 1
#define KEYWORD_DB_LPM_WRITE_COMMAND                            "DB_LPM_WRITE_COMMAND"
#define KEYWORD_DB_LPM_WRITE_COMMAND_SIZE                       sizeof(KEYWORD_DB_LPM_WRITE_COMMAND) - 1
#define KEYWORD_DB_EM_WRITE_COMMAND                             "DB_EM_WRITE_COMMAND"
#define KEYWORD_DB_EM_WRITE_COMMAND_SIZE                        sizeof(KEYWORD_DB_EM_WRITE_COMMAND) - 1
#define KEYWORD_REG_WRITE_COMMAND                               "REG_WRITE_COMMAND"
#define KEYWORD_REG_WRITE_COMMAND_SIZE                          sizeof(KEYWORD_REG_WRITE_COMMAND) - 1
#define KEYWORD_PROGRAM_SELECT_CONFIG_WRITE_COMMAND             "PROGRAM_SELECT_CONFIG_WRITE_COMMAND"
#define KEYWORD_PROGRAM_SELECT_CONFIG_WRITE_COMMAND_SIZE        sizeof(KEYWORD_PROGRAM_SELECT_CONFIG_WRITE_COMMAND) - 1





/* DB_DIRECT_MAPPING_INFO - line format
 *
 * KEYWORD_DB_DIRECT_INFO                                                  
 * <virtual-mem-name for debug>                                            
 * <db_id>                                                                 
 * <field-name for debug>                                                  
 * <field_id>                                                              
 * <result_field_width_for_debug>                                          
 * <mem-dimensions for debug>     (3 tokens 'rows' 'x' 'columns)           
 * <vc_st_row:vc_end_row>         (3 tokens 'vc_st_col' ':' 'vc_end_row')  
 * <vc_st_col:vc_end_col>         (3 tokens 'vc_st_col' ':' 'vc_end_col')  
 * {MAP|CAM}                      pe-type keyword                          
 * <pe_matrix_col>                                                         
 * <phy_mem_index>                                                         
 * <pem_mem_name> 
 * <pem_mem_address>              ( "40'hxxxx" where the first byte contains the block-id and the 32 LSb are the address ) 
 * <mem_st_row>                                                            
 * <mem_st_col> 
 * <mem_width_in_bits>
 * <implementation_index>
 */                                                                        
#define   PEM_NOF_DB_DIRECT_INFO_TOKEN  24 

/* KEYWORD_DB_INFO - line format
 *
 * DB_INFO
 * <db_type>
 * <Database-name> 
 * <db_id>
 * <nof_entries>
 * <key-width> 
 * <result-width> 
 *<key_field_num>
 * <result_field_num>
 *
 */
#define   PEM_NOF_DB_INFO_TOKEN   9


/* KEYWORD_KEY_INFO - line format
 *
 * KEY_INFO
 * <Database-name for debug> 
 * <db_type>
 * <db id>
 * <key-field-name for debug>
 * <key-field-id>
 * <lsb_bit>
 * <msb_bit>
 */
#define   PEM_NOF_KEY_FIELD_TOKEN  8


/* KEYWORD_RESULT_INFO - line format
 *
 * RESULT_FIELD 
 * <Database-name for debug> 
 * <db_type>
 * <db id>
 * <result-field-name for debug>
 * <result-field-id>
 * <lsb_bit>
 * <msb_bit>
 */
#define   PEM_NOF_RESULT_FIELD_TOKEN  8 


/* KEYWORD_VIRTUAL_REGISTER_MAPPING - line format
 *
 * VIRTUAL_REGISTER_MAPPING  
 * <Register-name for debug>
 * <reg-id>
 * <mapping_id>
 * <register field name for debug>
 * <register-field-id>
 * <register-total-width>
 * <register-field-msb:register-field-lsb>   (3 tokens 'register-field-msb' ':' 'register-field-lsb')
 * <pem-type>
 * <pe-matrix-col>
 * <program-mem-index>
 * <program-mem-name>
 * <pem_mem_addr>                             ( "40'hxxxx" where the first byte contains the block-id and the 32 LSb are the address ) 
 * <prog_mem_line> 
 * <pem_mem_offset> 
 * <pem_mem_width_in_bits>
 */
#define   PEM_NOF_REGISTER_INFO_TOKEN  18


/* KEYWORD_VIRTUAL_REGISTER_NOF_FIELDS - line format
 *
 * KEYWORD_VIRTUAL_REGISTER_NOF_FIELDS 
 * <reg_id>
 * <nof_fields>
 */
#define   VIRTUAL_REGISTER_NOF_FIELDS_TOKEN  4


/* KEYWORD_VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS - line format
 *
 * KEYWORD_VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS
 * <reg_id>
 * <field_id>
 * <nof_mappings>
 */
#define   VIRTUAL_REGISTER_FIELD_NOF_MAPPINGS_TOKEN  6


/* DB_TCAM_KEY_MAPPING_INFO   - line format
 *
 * DB_TCAM_KEY_MAPPING_INFO                                                
 * <virtual-mem-name for debug> 
 * <db_id> 
 * <key-name for debug>  
 * <virt_KEY_mem-total-dimensions for debug>  (3 tokens 'rows' 'x' 'columns')  
 * <vc_end_row:vc_st_row>                     (3 tokens 'end-rows' ':' 'start-rows')
 * <vc_valid_col>  
 * <vc_mask_end_col: vc_mask_st_col >         (3 tokens 'mask-end-col' ':' 'mask-st-col')
 * <vc_key_end_col: vc_key_st_col >           (3 tokens 'key-end-col' ':' 'key-st-col')
 * <pe_type> 
 * <pe_matrix_col> 
 * <phy_mem_ndx> 
 * <pem_mem_name>
 * <pem_mem_addr>                             ( "40'hxxxx" where the first byte contains the block-id and the 32 LSb are the address )  
 * <mem_st_row> 
 * <mem_valid_col> 
 * <mem_mask_st_col> 
 * <mem_key_st_col>
 * <mem_width_in_bits>
 * <implementation_index>
 */                                                                        
#define   PEM_NOF_DB_CAM_KEY_MAPPING_INFO_TOKEN  28 

/* DB_TCAM_RESULT_MAPPING_INFO    - line format
 *
 * DB_TCAM_RESULT_MAPPING_INFO
 * <virtual-mem-name for debug> 
 * <db_id> 
 * <result-name for debug>  
 * <virt_RESULT_mem-dimensions for debug>      (3 tokens 'rows' 'x' 'columns') 
 * <vc_end_row:vc_st_row>                      (3 tokens 'end-rows' ':' 'st-rows')
 * <vc_end_col: vc_st_col >                    (3 tokens 'end-columns' ':' 'st-columns') 
 * <pe_type> 
 * <pe_matrix_col>
 * <phy_total_width_in_bits>
 * <phy_mem_ndx> 
 * <pem_mem_name> 
 * <pem_mem_addr>                             ( "40'hxxxx" where the first byte contains the block-id and the 32 LSb are the address )
 * <mem_st_row> 
 * <mem_st_col> 
 * <implementation_index>
 *                                           
 */                                                                        
#define   PEM_NOF_DB_CAM_RESULT_MAPPING_INFO_TOKEN  22 
 

/* DB_SINGLE_VIRT_DB_MAP_INFO               -line format
*
* DB_SINGLE_VIRT_DB_MAP_INFO
* <db_id>
* <nof_chunk_rows>
* <nof_chunk_cols>
* <nof_implementations>
*
*/
#define   DB_SINGLE_VIRT_DB_MAP_INFO_DIRECT_TOKEN 5


/* DB_SINGLE_VIRT_DB_MAP                    -line format
*
* DB_SINGLE_VIRT_DB_MAP_INFO
* <db_id>
* <nof_key_chunk_rows>
* <nof_key_chunk_cols>
* <nof_result_chunk_rows>
* <nof_result_chunk_cols>
* <nof_implementations>S
*
*/
#define   DB_SINGLE_VIRT_DB_MAP_INFO_TOKEN 7





/**********************************************
 *         uCode definition
 **********************************************/

#define KEYWORD_UCODE_PEM_START                   "PEM_START"
#define KEYWORD_UCODE_PEM_START_SIZE              sizeof("PEM_START") - 1
#define KEYWORD_UCODE_PEM_END                     "PEM_END"
#define KEYWORD_UCODE_PEM_END_SIZE                sizeof("PEM_END") - 1
#define KEYWORD_UCODE_REG_LINE_START_INFO         "REG_LINE_START"
#define KEYWORD_UCODE_REG_LINE_START_INFO_SIZE    sizeof("REG_LINE_START") - 1
#define KEYWORD_UCODE_REG_LINE_END                "REG_LINE_END"
#define KEYWORD_UCODE_REG_LINE_END_SIZE           sizeof("REG_LINE_END") - 1 
#define KEYWORD_UCODE_FIELD_INFO                  "FIELD"
#define KEYWORD_UCODE_FIELD_INFO_SIZE             sizeof("FIELD") - 1
#define KEYWORD_UCODE_MEM_LINE_START_INFO         "MEM_LINE_START"
#define KEYWORD_UCODE_MEM_LINE_START_INFO_SIZE    sizeof("MEM_LINE_START") - 1
#define KEYWORD_UCODE_MEM_LINE_END                "MEM_LINE_END"
#define KEYWORD_UCODE_MEM_LINE_END_SIZE           sizeof("MEM_LINE_END") - 1

/* PEM_START - line format
 * 
 * PEM_START
 * <pe_type_name>
 * <pem_row> 
 * <pem_col>
 */

#define PEM_NOF_PEM_START_TOKEN 4

/* PEM_END - line format
 *
 * PEM_END
 */
#define PEM_NOF_PEM_END_TOKEN 1

/* REG_LINE_START - line format
 * 
 * REG_LINE_START 
 * <reg_name> 
 * <reg_addr>                     ( "40'hxxxx" where the first byte contains the block-id and the 32 LSb are the address )
 * <reg_val>                      ( "n'hxxxxxxx" format )
 */

#define PEM_NOF_REG_LINE_START_TOKEN 4  /*changed from 9*/

/* REG_LINE_END - line format
 *
 * REG_LINE_END
 */
#define PEM_NOF_REG_LINE_END_TOKEN 1

/* FIELD - line format
 * 
 * FIELD 
 * <field_id_0> 
 * <field_name_0> 
 * <field_val_0>             ( "n'hxxxxxxx" format )
 */
#define PEM_NOF_FIELD_TOKEN 4

/* MEM_LINE_START - line format
 * 
 * MEM_LINE_START 
 * <pem_mem_name> 
 * <pem_mem_addr>                     ( "40'hxxxx" where the first byte contains the block-id and the 32 LSb are the address )
 * <prog_mem_address>   
 * <all_line_bits_val>                ( "n'hxxxxxxx" format )
 */
#define PEM_NOF_MEM_LINE_START_TOKEN 5

/* MEM_LINE_END - line format
 *
 * MEM_LINE_END
 */
#define PEM_NOF_MEM_LINE_END_TOKEN 1



#define phy_mem_len 6    /* 7 = pe_type (1 bit, MAP or CAM) + pe_index (4 bits, max of 16 PEs) + sram_index (2 bits, max of 4 SRAM) */
#define phy_mem2mem_index(phy_mem)  (phy_mem & (0xFFFFFFFF >> (32 - phy_mem_len)))
#define build_mem_index(db_type, pe_index, sram_index) (db_type | (pe_index << 1) | (sram_index << 5))

/*Initialize api_info struct*/
void init_api_info();
/*Free all prior alocated memory in api_info*/
void free_api_info();
/*free all direct container*/
void free_db_direct_container();
/*free all tcam container*/
void free_db_tcam_container();
/*free all lpmcontainer*/
void free_db_lpm_container();
/*free all em container*/
void free_db_em_container();
/*free all reg container*/
void free_reg_container();
/*free logical direct info*/
void free_logical_direct_info(LogicalDirectInfo* logical_direct_info);
/*free logical tcam info*/
void free_logical_tcam_info(LogicalTcamInfo* logical_tcam_info);
/*free logical lpm info*/
void free_logical_lpm_info(LogicalLpmInfo* logical_lpm_info);
/*free logical em info*/
void free_logical_em_info(LogicalEmInfo* logical_em_info);
/*free logical reg info*/
void free_logical_reg_info(LogicalRegInfo* logical_reg_info);
/*free chunk mapper matrix*/
void free_chunk_mapper_matrix(const int nof_implamentations, LogicalDbChunkMapperMatrix* chunk_mapper_matrix);
/*free lpm cache*/
void free_lpm_cache(LpmDbCache* lpm_cache);
/*free em cache*/
void free_em_cache(EmDbCache* em_cache);

/*Insert memory line*/
void mem_line_insert(const char* line);
/*Insert register line*/
void reg_line_insert(const char* line);

/* Inserts info of Db by ID*/
void db_info_insert(const char* line);
/* Insert msb/lsb mapping*/
void db_field_info_insert(const char* line);
/* Insert msb/lsb mapping to logical_key_fields_location*/
void db_key_field_info_insert(const char* db_type, const int db_id, const int field_id, const int lsb_bit, const int msb_bit);
/* Insert msb/lsb mapping to logical_result_fields_location*/
void db_result_field_info_insert(const char* db_type, const int db_id, const int field_id, const int lsb_bit, const int msb_bit);
/* Updates is_mapped per each field in key or result. Writes 1 if field was mapped to physical and 0 else.*/
void is_field_mapped_update(const int num_of_fields, FieldBitRange* field_bit_range_arr , DbChunkMapper*const chunk_info_ptr);

/* Insert one chunk into LogicalDirectMapper*/
void direct_result_chunk_insert(const char* line);
/* Insert one CAM based key chunk into api_info.db_tcam_container.db_tcam_info_arr*/
void tcam_key_chunk_insert(const char* line);
/* Insert one CAM based result chunk into api_info.db_tcam_container.db_tcam_info_arr*/
void tcam_result_chunk_insert(const char* line);
/*Insert one key chunk of EM database into api_info.db_em_container.db_em_info_arr*/
void em_key_chunk_insert(const char* line);
/*Insert one key chunk of EM database into api_info.db_em_container.db_em_info_arr*/
void em_result_chunk_insert(const char* line);
/*Insert one key chunk of EM database into api_info.db_lpm_container.db_lpm_info_arr*/
void lpm_key_chunk_insert(const char* line);
/*Insert one key chunk of EM database into api_info.db_lpm_container.db_lpm_info_arr*/
void lpm_result_chunk_insert(const char* line);

/*Insert register*/
void register_insert(const char* line);

/* Build result chunk of DIRECT db. Modifies chunk_info_ptr and returns db_id*/
unsigned int build_direct_chunk_from_ucode(const char* line, DbChunkMapper* chunk_info_ptr);
/* Build key chunk of CAM based db. Modifies chunk_info_ptr and returns db_id*/
unsigned int build_cam_key_chunk_from_ucode(const char* line, DbChunkMapper* chunk_info_ptr);
/* Build result chunk of CAM based db. Modifies chunk_info_ptr and returns db_id*/
unsigned int build_cam_result_chunk_from_ucode(const char* line, DbChunkMapper* chunk_info_ptr);
/* Build register field mapping info*/
unsigned int build_register_mapping_from_ucode(const char* line, struct RegFieldMapper* reg_field_mapping_ptr);

/* Insert the chunk into right place in the list*/
void db_chunk_insert(LogicalDbChunkMapperMatrix* logical_db_mapper, DbChunkMapper*const chunk_info_ptr);

void db_virtual_reg_field_mapping_insert(struct LogicalRegInfo* reg_info_p, struct RegFieldMapper* reg_field_mapping_ptr);

/* Gets size of each DB from line and initialize it*/
void init_all_db_arr_by_size(const char* line);
/* Init DbChunkMapper for a single Db_id*/
void init_logical_db_chunk_mapper(const char* line);
/* Init LogicalDbChunkMapperMatrix mattrix*/
void init_logical_db_mapper_matrix(LogicalDbChunkMapperMatrix** logical_db_mapper_matrix_ptr, const int nof_chunk_rows, const int nof_chunk_cols, const int nof_implementations);
/* Realloc LogicalDbChunkMapperMatrix mattrix*/
void realloc_logical_db_mapper_matrix(LogicalDbChunkMapperMatrix** logical_db_mapper_matrix_ptr, const int nof_chunk_cols_to_add, const int nof_implementations);
/* Init msb/lsb bits mattrix*/
void init_logical_fields_location(FieldBitRange** field_bit_range_arr, const int nof_fields);
/* Init Logical_Reg_Field_Info*/
void init_reg_field_info(const char* line);
/* Init Reg_Field_Mapper*/
void init_reg_field_mapper(const char* line);
/* Init pem applet regs and mems*/
void init_pem_applet_reg(const char* line);
void init_pem_applet_mem(const char* line);
void init_meminfo_array_for_applets(const char* line);
void insert_meminfo_to_array_for_applets(const char* line);
/* Init EM cache members*/
void init_em_cache(EmDbCache* em_cache_info);
/* Init LPM cache members*/
void init_lpm_cache(LpmDbCache* lpm_cache_info);


/* Init DBs content from write_commands file*/
int init_dbs_content_and_program_selection_tcams_from_write_commands_file(const char* write_commands_file_name);

unsigned int hexstr2addr(char *str, unsigned int *block_id);



#endif /* _PEMLADRV_MEMINFO_INIT_H_ */
