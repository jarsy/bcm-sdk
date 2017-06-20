/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _PEMLADRV_LOGICAL_ACCESS_H_
#define _PEMLADRV_LOGICAL_ACCESS_H_

/* The following definition 'DB_OBJECT_ACCESS' is to compile code accessing Kidron DB objects (C++).
 * In case of using Kidron outputs (not the C++ environment) it should be commented out.
 */

/* #define DB_OBJECT_ACCESS */

#define MAX_NAME_LENGTH 192
#define MAX_MEM_DATA_LENGTH 640

#include <limits.h>
#include <sal/types.h>
#include <soc/dnx/pemladrv.h>
#include "pemladrv_physical_access.h"

#define GET_NOF_ENTRIES(nof_bits, entry_size_in_bits) ( ( (nof_bits) % (entry_size_in_bits) == 0) ? ( (nof_bits) / (entry_size_in_bits)) : ( (nof_bits) / (entry_size_in_bits) + 1))
#define APPLET_MEM_NOF_ENTRIES 63
#define APPLET_REG_AMOUNT_OF_PACKETS_SIZE_IN_BITS 7
#define ADDRESS_SIZE_IN_BITS  32
#define BLOCK_ID_SIZE_IN_BITS 8

/* Error codes */
typedef enum {
  PEM_ERR_NONE                                           =  0,
  PEM_ERR_INTERNAL                                       =  1,
  PEM_WARNING_READ_PARTIALLY_MAPPED_REGISTER             =  2,
  PEM_ERR_LIMIT                                          =  3
} pem_err_code_t;

/* PEM logical access layer */


typedef struct DbChunkMapper {
  unsigned int           phy_pe_matrix_col;
  unsigned int           phy_mem_index;
  char*                  phy_mem_name;
  unsigned int           phy_mem_addr;
  unsigned int           phy_mem_width;
  unsigned int           mem_block_id;   
  unsigned int           phy_mem_entry_offset;
  unsigned int           phy_mem_width_offset;
  unsigned int           virtual_mem_entry_offset;
  unsigned int           virtual_mem_width_offset;
  unsigned int           chunk_width;
  unsigned int           chunk_entries;
  unsigned int           chunk_matrix_row_ndx;
  unsigned int           chunk_matrix_col_ndx;
  unsigned int           chunk_matrix_ndx;
} DbChunkMapper;


typedef struct LogicalDbChunkMapperMatrix {
  DbChunkMapper*** db_chunk_mapper; /* matrix of pointers, NULL pointer means that the entry in the matrix is not initialized. common for TCAM key, TCAM result & PeMap Sram */
  unsigned int     nof_rows_in_chunk_matrix;
  unsigned int     nof_cols_in_chunk_matrix;
} LogicalDbChunkMapperMatrix;

typedef struct EmDbCache {
  unsigned int** em_key_entry_arr; /* array of entries, each entry contains an array that represent a single line (line width can be bigger than 32 bits, so it is possible that each entry requires few ints)*/
  unsigned int** em_result_entry_arr; /* array of entries, each entry contains an array that represent a single line (line width can be bigger than 32 bits, so it is possible that each entry requires few ints)*/
  int            next_free_index;   /*might be -1*/
  unsigned char* physical_entry_occupation;

}EmDbCache;

typedef struct LpmDbCache {
  unsigned int** lpm_key_entry_arr; /* array of entries, each entry contains an array that represent a single line (line width can be bigger than 32 bits, so it is possible that each entry requires few ints) */
  unsigned int** lpm_result_entry_arr; /* array of entries, each entry contains an array that represent a single line (line width can be bigger than 32 bits, so it is possible that each entry requires few ints) */
  unsigned int*  lpm_key_entry_prefix_length_arr;
  unsigned char* physical_entry_occupation;
}LpmDbCache;


typedef struct FieldBitRange {
  unsigned int          lsb;
  unsigned int          msb;
  int                   is_field_mapped;
}FieldBitRange;

typedef struct LogicalDirectInfo {
  unsigned int                         total_nof_entries;
  unsigned int                         total_result_width;
  unsigned int                         nof_fields;
  FieldBitRange*                       field_bit_range_arr;    /* array of fields, in each entry (lsb, msb) of the field*/
  LogicalDbChunkMapperMatrix*          result_chunk_mapper_matrix_arr;
  unsigned int                         nof_chunk_matrices;
} LogicalDirectInfo;

typedef struct LogicalTcamInfo {
  unsigned int                           total_nof_entries;
  unsigned int                           total_key_width;
  unsigned int                           total_result_width;
  unsigned int                           nof_fields_in_key;
  unsigned int                           nof_fields_in_result;
  FieldBitRange*                         key_field_bit_range_arr;
  FieldBitRange*                         result_field_bit_range_arr;
  LogicalDbChunkMapperMatrix*            key_chunk_mapper_matrix_arr;
  LogicalDbChunkMapperMatrix*            result_chunk_mapper_matrix_arr;
  unsigned int                           nof_chunk_matrices;
} LogicalTcamInfo;

typedef struct LogicalEmInfo {
  LogicalTcamInfo               logical_em_info;
  EmDbCache                     em_cache;
} LogicalEmInfo;

typedef struct LogicalLpmInfo {
  LogicalTcamInfo               logical_lpm_info;
  LpmDbCache                    lpm_cache;
} LogicalLpmInfo;


/* The following map a piece of a logical field to a memory chunk (or part of it) of PEM-memory */
typedef struct RegFieldMapper {
  /* virtual field mapping info*/
  unsigned int                 mapping_id                           ; /* per reg-field there are N mappings with ids 0..N-1*/
  unsigned int                 field_id                             ; /* for debug - it is the location in the containing array*/
  unsigned int                 cfg_mapper_width                     ; /* the width of m_wire_cfg_p that this cCfgMapper is implementing*/
  unsigned int                 virtual_field_lsb                    ;
  unsigned int                 virtual_field_msb                    ;
  /* physical field mapping info*/
  unsigned int                 pem_matrix_col                       ;
  unsigned int                 pem_mem_block_id                     ;
  unsigned int                 pem_mem_address                      ;
  unsigned int                 pem_mem_line                         ;
  unsigned int                 pem_mem_offset                       ;
  unsigned int                 pem_mem_total_width                  ;
} RegFieldMapper; 

typedef struct LogicalRegFieldInfo {
  unsigned int       nof_mappings;
  RegFieldMapper*    reg_field_mapping_arr;
} LogicalRegFieldInfo;


typedef struct LogicalRegInfo {
  unsigned int         register_total_width; 
  unsigned int         nof_fields; 
  LogicalRegFieldInfo* reg_field_info_arr;
  FieldBitRange*       reg_field_bit_range_arr;
  int                  is_mapped;
} LogicalRegInfo;


typedef struct LogicalRegContainer {
  unsigned int nof_registers;
  LogicalRegInfo* reg_info_arr;
} LogicalRegContainer;


typedef struct LogicalDirectContainer {
  unsigned int         nof_direct_dbs;
  LogicalDirectInfo*   db_direct_info_arr;
} LogicalDirectContainer;

typedef struct LogicalTcamContainer {
  unsigned int         nof_tcam_dbs;
  LogicalTcamInfo*     db_tcam_info_arr;
} LogicalTcamContainer;

typedef struct LogicalEmContainer { 
  unsigned int          nof_em_dbs;
  LogicalEmInfo*        db_em_info_arr;
} LogicalEmContainer;

typedef struct LogicalLpmContainer { 
  unsigned int           nof_lpm_dbs;
  LogicalLpmInfo*        db_lpm_info_arr;
} LogicalLpmContainer;

/*this struct holds information about the memory where applet data is written to in order to modify the configuraion.
  it holds the address of the memory, its name and size
  */
typedef struct AppletRegMemInfo {
  char                 name[MAX_NAME_LENGTH];
  unsigned int         is_ingress0_egress1;
  unsigned int         block_id;
  unsigned int         address;
  unsigned int         length_in_bits;  
} AppletRegMemInfo;

/*this struct holds info about all pem/pema memories in use for the current application.
  when modifying it via applet mechanism, must know if the memory is reachable through ingress or egress
  */
typedef struct PemMemInfoForApplet {
  char              name[MAX_NAME_LENGTH];
  unsigned int      address; 
  unsigned int      stage_relevant_for;
} PemMemInfoForApplet;

typedef struct AppletInfo {
  unsigned int            currently_writing_applets_bit;    
  unsigned int            last_stage_of_ingress;
  int                     meminfo_curr_array_size;
  PemMemInfoForApplet**   meminfo_array_for_applet; /*array of all physical memories, applets may write to*/
  /*all of the following come in pairs - ingress and egress*/
  AppletRegMemInfo        applet_reg_info[2];   /*register info for trigerring applets writg*/
  AppletRegMemInfo        applet_mem_info[2];   /*memory to write applets to*/  
  unsigned int            nof_pending_applets[2];    /*how many */
  unsigned int*           applet_mem_cache[2][APPLET_MEM_NOF_ENTRIES]; /*2 memories as one for ingress and one for ingress*/
  int                     entry_found_for_read_lately[2];     /*usually doing read-modify-write. maybe entry found in applets mem. if so keep the entry for faster access when writing*/
} AppletInfo;

typedef struct ApiInfo { 
  LogicalDirectContainer db_direct_container;
  LogicalTcamContainer   db_tcam_container;
  LogicalEmContainer     db_em_container;
  LogicalLpmContainer    db_lpm_container;
  LogicalRegContainer    reg_container;
  AppletInfo             applet_info;
} ApiInfo;


/*serves as an element in a list that maintains write data*/
typedef struct PhysicalWriteInfo {
  phy_mem_t                   mem;
  unsigned int*               entry_data;
  struct PhysicalWriteInfo*   next;
} PhysicalWriteInfo;

/* Init 
 * This function is responsible of linking between logical names to physical memory location.
 * Note that this function MUST be called before any memory access.
 *
 * Parameters:
 * 1. file-name - file name (including the path) of the Kidron output file containing the PEM memory definitions.
 *                In case of null the define 'PEM_DEFAULT_DB_MEMORY_MAP_FILE' is taken
 */
#define PEM_DEFAULT_DB_MEMORY_MAP_FILE  "pem_memory_map.txt"


int init_pem_applets_db();



/* MBIST */ 
/***********************************************/
/*perform virtual MBIST to pem dbs - write then read content to verify correctness*/
int pem_mem_virtual_bist();

int  allocate_pemladrv_mem_struct(pemladrv_mem_t** mem_access, unsigned int nof_fields, const FieldBitRange* bit_range_arr);
void free_pemladrv_mem_struct(pemladrv_mem_t** mem_access);

/* Set all fldbufs per field in mem_access*/
void set_pem_mem_accesss_fldbuf(const unsigned int offset, const unsigned int* virtual_db_data_array, const FieldBitRange* field_bit_range_arr, pemladrv_mem_t* mem_access);

/***********************************************/
/*print write value for debug*/
void print_debug_pem_read_write(const char* prefix, const unsigned int address, const unsigned int length_in_bits, const unsigned int row_ndx, const unsigned int* value);

/***********************************************/
/*Applets*/
/*those functions should be called to also handle applets*/
int pem_read(int          unit,
             phy_mem_t*   mem,
             int          is_mem,
             void*        entry_data);

int pem_write(int         unit,
              phy_mem_t*  mem,
              int         is_mem,
              void*       entry_data);
/*******/
int  start_writing_applets();                                           /*call when configuration through applets is desired*/
void flush_applets_to_single_mem(const int is_ingress0_egress1);
int  flush_applets();                                                   /*call to flush the applets memory and write to PEM*/
int  is_mem_belong_to_ingress0_or_egress1(phy_mem_t* mem, int min_ndx, int max_ndx);              /*deetrmine to which memory to write the applet*/



/******************/
/* Utile functions*/
/******************/
/* The following are utile functions, used both in pem_logical_access and pemladrv_meminfo_init*/

/*Writing '0' to valid bit in physical DB. Also, Sets the mask to ones*/
void set_valid_bit_to_zero(int unit, const int virtual_row_index, LogicalTcamInfo* tcam_info);

/* Returns number of rows in TCAM chunk mattrix*/
unsigned int get_nof_tcam_chunk_cols(const unsigned int total_key_width);

/* Returns the physical row index*/
int calculate_physical_row_index_from_chunk_mapper(const DbChunkMapper* chunk_mapper, const int virtual_row_index);

/* Init phy_mem_t from chunk_mapper info*/
void init_phy_mem_t_from_chunk_mapper(const unsigned int chunk_mem_block_id, 
                                      const unsigned int chunk_phy_mem_addr, 
                                      const unsigned int chunk_phy_mem_row_index, 
                                      const unsigned int chunk_phy_mem_width,
                                      /*int                is_mem,*/
                                      phy_mem_t* phy_mem);



/* remove LPM entry by index (invalidate).
 * Return '0' on success, or number indicating error:
 * - not exists.
 */
int pemladrv_lpm_remove_by_index (int unit, table_id_t table_id,  int index);


/* Calibrate last chunk of cam based dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_cam_based_dbs();
/*Calibrate last chunk of tcam dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_tcam_dbs();
/*Calibrate last chunk of lpm dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_lpm_dbs();
/*Calibrate last chunk of wm dbs to avoid writing junk to physical (setting ones to mask and zeroing valid bit)*/
void zero_last_chunk_of_em_dbs();
/*Reads physical memory and updates LPM and EM cache*/
void read_physical_and_update_cache();


#endif /* _PEMLADRV_LOGICAL_ACCESS_H_ */

