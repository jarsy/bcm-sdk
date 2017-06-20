/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _PEMLADRV_H_
#define _PEMLADRV_H_

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

typedef unsigned int table_id_t;   
typedef unsigned int field_id_t; 
typedef unsigned int reg_id_t;  
typedef unsigned int tcam_id_t;

typedef struct pemladrv_field_s {
	  field_id_t	        field_id;
    unsigned int*       fldbuf;
    unsigned int  	    flags;    
} pemladrv_field_t;

typedef struct pemladrv_mem_s {	
    unsigned short          nof_fields;
    pemladrv_field_t        **fields; 
    unsigned int            flags;    
} pemladrv_mem_t;


/* Read/Write function to (logical) access the PEM Logical Db Direct Data-Bases */
int pemladrv_direct_write(int unit, table_id_t db_id, unsigned int row_index, pemladrv_mem_t *data);
int pemladrv_direct_read(int unit, table_id_t db_id, unsigned int row_index, pemladrv_mem_t *result);

/* Read/Write function to (logical) access the PEM Logical Registers */
int pemladrv_reg_write(int unit, reg_id_t reg_id, pemladrv_mem_t *data);
int pemladrv_reg_read(int unit,  reg_id_t reg_id, pemladrv_mem_t *result);

/* Read/Write function to (logical) access the PEM Logical DB TCAM  */
int pemladrv_tcam_write(int unit, tcam_id_t tcam_id, unsigned int row_index, pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid, pemladrv_mem_t *data);
int pemladrv_tcam_read(int unit,  tcam_id_t tcam_id, unsigned int row_index, pemladrv_mem_t *key, pemladrv_mem_t *mask, pemladrv_mem_t *valid, pemladrv_mem_t *result);


/*
 * The following functions are higher logical access to logical Databases.
 * Which means that user do not know how actually this memory are implemented (sram/tcam,...)
 * Implementation of those function are using the above memories access.
 *
 * All the following function return '0' on success or negative number indicating error.
 */

/* EM (Exact Match) access functions */
/*************************************/

/* add entry, 
 * returns the index (in 'index' parameter) in the EM table
 * The function return '0' on success, or  number indicating error:
 * - key already exists returns error and the index of existed entry, 
 * - table is full 
 */
int pemladrv_em_insert(int unit, table_id_t table_id,  pemladrv_mem_t* key,  pemladrv_mem_t* result, int *index);

/* remove entry, 
 * returns the index (in 'index'  parameter) in the EM table that was cleared. 
 * The function return '0' on success or  number indicating error:
 * - entry not exists.
 */
int pemladrv_em_remove(int unit, table_id_t table_id,  pemladrv_mem_t* key, int *index); 

/* lookup entry, 
 * returns the index (in 'index' parameter) in the EM table that was found, the key and the result.
 * Function return '0' on success or number indicating error:
 */
int pemladrv_em_lookup(int unit, table_id_t table_id,  pemladrv_mem_t* key, pemladrv_mem_t* result, int *index);

/* get next EM entry, 
 * returns the index (in 'index' parameter) in the EM table of the next valid entry, 
 * to receive the first entry use index=0, if no more entries exists returns index=-1.
 * Return '0' on success, or number indicating error:
 */
int pemladrv_em_get_next_index(int unit, table_id_t table_id, int* index);

/* get EM entry key and result by index, 
 * Return '0' on success, or number indicating error:
 * - entry not valid 
 */
int pemladrv_em_entry_get_by_id(int unit, table_id_t table_id,  int index,  pemladrv_mem_t* key, pemladrv_mem_t* result);

/* set EM entry key and result by index, override existing entry. 
 * Return '0' on success, or number indicating error:
 */
int pemladrv_em_entry_set_by_id(int unit, table_id_t table_id,  int index, pemladrv_mem_t* key, pemladrv_mem_t* result);

/* remove all EM entries in DB */
int pemladrv_em_remove_all(int unit, table_id_t table_id);

/* remove EM entry by index (invalidate).
 * Return '0' on success, or number indicating error:
 * - not exists.
 */
int pemladrv_em_remove_by_index (int unit, table_id_t table_id, int index);


/* LPM (Longest Prefix Match) access functions */ 
/***********************************************/

/* add entry, 
 * returns the index (in 'index' parameter) in the LPM table.
 * The function return '0' on success, or  number indicating error:
 * - key already exists returns error and the index of existed entry, 
 * - table is full 
 */
int pemladrv_lpm_insert(int unit, table_id_t table_id,  pemladrv_mem_t* key, int length_in_bits,  pemladrv_mem_t* result, int *index);

/* remove entry, 
 * returns the index (in 'index'  parameter) in the LPM table that was cleared. 
 * The function return '0' on success or  number indicating error:
 * - entry not exists.
 */
int pemladrv_lpm_remove(int unit, table_id_t table_id,  pemladrv_mem_t* key,  int length_in_bits, int *index);

/* lookup entry, 
 * returns the index (in 'index' parameter) in the LPM table that was found, the key and the result.
 * Function return '0' on success or number indicating error:
 */
int pemladrv_lpm_lookup(int unit, table_id_t table_id,  pemladrv_mem_t* key,  pemladrv_mem_t* result, int *index);

/*remove all LPM entries in DB */
int pemladrv_lpm_remove_all(int unit, table_id_t table_id);


/******************************/
/* Memory Management functions*/
/******************************/

/*allocating pem_mem_access struct with pem_field_access** arr with size of nof_fields*/
pemladrv_mem_t* pemladrv_mem_alloc(const unsigned int nof_fields);

/*The function receives  a pointer to specific field and the size of the field buffer in bits and allocates memory for the field buffer*/
void              pemladrv_mem_alloc_field(pemladrv_field_t* pem_field_access, const unsigned int field_width_in_bits);

/*allocating pem_mem_access struct for result with nof_fields and fldbuf size matching the fields_num and nof_bits per field in direct db with db_id. Return '0' on success, or number indicating error*/
unsigned int      pemladrv_mem_alloc_direct(const int db_id, pemladrv_mem_t** result);
                   
/*allocating pem_mem_access struct for key, mask, valid and result with nof_fields and fldbuf size matching the fields_num and nof_bits per field in tcam db with db_id. Return '0' on success, or number indicating error*/
unsigned int      pemladrv_mem_alloc_tcam(const int db_id, pemladrv_mem_t** key, pemladrv_mem_t** mask, pemladrv_mem_t** valid, pemladrv_mem_t** result);
                   
/*allocating pem_mem_access struct for result with nof_fields and fldbuf size matching the fields_num and nof_bits per field in lpm db with db_id. Return '0' on success, or number indicating error*/
unsigned int      pemladrv_mem_alloc_lpm(const int db_id, pemladrv_mem_t** key, pemladrv_mem_t** result);
                   
/*allocating pem_mem_access struct for result with nof_fields and fldbuf size matching the fields_num and nof_bits per field in em db with db_id. Return '0' on success, or number indicating error*/
unsigned int      pemladrv_mem_alloc_em(const int db_id, pemladrv_mem_t** key, pemladrv_mem_t** result);

/*freeing memory allocated in pem_mem_access struct*/
void              pemladrv_mem_free(pemladrv_mem_t* pem_mem_access);

int pemladrv_init(const int restore_after_reset, const char *vmem_definition_file_name /* in case of null, PEM_DEFAULT_DB_MEMORY_MAP_FILE is taken */);

#endif /* _PEMLADRV_H_ */
