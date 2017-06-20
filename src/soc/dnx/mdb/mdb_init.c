/*
 * ! \file mdb_init.c $Id$ Contains all of the MDB initialization sequences.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/*
 * Include files.
 * {
 */
#include <soc/dnx/mdb.h>
#include <soc/dnx/dbal/dbal.h>
#include "mdb_internal.h"

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_MDBDNX

/*
 * }
 */

/*
 * Globals
 * {
 */
/**
 * defualt unknown string
 */
static char *mdb_unknown_string = "Unknown ID";

/**
 * An array that holds for each physical table the addresses of its physical resources.
 */
mdb_db_info_t mdb_db_infos[MDB_NOF_PHYSICAL_TABLES];

/*
 *  mdb_default_connection_array:
 * Physical A0  A1  A2  A3  A4  A5  A6  A7  A8  A9  A10 A11 B0  B1  B2  B3  B4  B5  B6  B7
 *  ISEM_1  1   1   1   1   1           1                                   1           1
 *  INLIF_1                                                     1
 *  IVSI                    1
 *  ISEM_2                                  1   1   1                   1   1
 *  INLIF_2             1   1
 *  ISEM_3              1                                   1   1   1
 *  INLIF_3     1   1
 *  LEM 1   1   1   1       1   1   1   1   1   1   1
 *  ADS_1                       1
 *  ADS_2                           1
 *  KAPS_1      1   1   1   1           1   1   1   1                   1   1           1
 *  KAPS_2                  1                               1       1       1   1   1   1
 *  IOEM_0                      1                                                   1   1
 *  IOEM_1                          1                                       1   1
 *  MAP                                 1   1
 *  FEC_1                                                           1
 *  FEC_2                                                               1
 *  FEC_3                                               1
 *  MC_ID                                                                   1   1   1   1
 *  GLEM_0                      1   1
 *  GLEM_1  1                                           1
 *  EEDB_1  1
 *  EEDB_2                                          1
 *  EEDB_3                                                  1
 *  EEDB_4                              1
 *  EOEM_0                                                  1       1   1               1
 *  EOEM_1                                                  1       1   1               1
 *  ESEM                                1   1   1   1
 *  EVSI                                            1
 *  EXEM_1                                  1   1           1   1
 *  EXEM_2                                                  1   1               1   1
 *  EXEM_3                                                      1               1   1
 *  EXEM_4                                                      1               1   1
 *  RMEP                                                            1   1
 *
 * An array that holds (MDB_NOF_MACRO_A + MDB_NOF_MACRO_B) entries for each physical DB.
 * If a specific MACRO is valid for the DB, the corresponding array cell will describe how many clusters are allocated to it.
 * The DBs appearance in the array must be aligned to physical_tables_e.
 * The clusters within each macro are allocated sequentially on a first-come first-serve basis.
 */
static uint8 mdb_default_connection_array[MDB_NOF_PHYSICAL_TABLES * (MDB_NOF_MACRO_A + MDB_NOF_MACRO_B)] = {
    0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1,
    0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0,
};
/*
 * }
 */

/**
 *  initializes an array that translates from DBAL physical table enum to MDB.
 */
shr_error_e
mdb_direct_dbal_to_mdb_init(
    int unit)
{
    int db_index;

    SHR_FUNC_INIT_VARS(unit);
    /*
     * Initialize the mapping between DBAL physical tables and MDB physical tables
     */
    for (db_index = DBAL_PHYSICAL_TABLE_NONE; db_index < DBAL_NOF_PHYSICAL_TABLES; db_index++)
    {
        switch (db_index)
        {
            case DBAL_PHYSICAL_TABLE_TCAM:
                mdb_direct_dbal_to_mdb[db_index] = MDB_NOF_PHYSICAL_TABLES;
                break;

            case DBAL_PHYSICAL_TABLE_LPM_PRIVATE:
                mdb_direct_dbal_to_mdb[db_index] = MDB_NOF_PHYSICAL_TABLES;
                break;

            case DBAL_PHYSICAL_TABLE_LPM_PUBLIC:
                mdb_direct_dbal_to_mdb[db_index] = MDB_NOF_PHYSICAL_TABLES;
                break;

            case DBAL_PHYSICAL_TABLE_ISEM_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_ISEM_1;
                break;

            case DBAL_PHYSICAL_TABLE_INLIF_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_INLIF_1;
                break;

            case DBAL_PHYSICAL_TABLE_IVSI:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_IVSI;
                break;

            case DBAL_PHYSICAL_TABLE_ISEM_2:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_ISEM_2;
                break;

            case DBAL_PHYSICAL_TABLE_INLIF_2:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_INLIF_2;
                break;

            case DBAL_PHYSICAL_TABLE_ISEM_3:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_ISEM_3;
                break;

            case DBAL_PHYSICAL_TABLE_INLIF_3:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_INLIF_3;
                break;

            case DBAL_PHYSICAL_TABLE_LEM:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_LEM;
                break;

            case DBAL_PHYSICAL_TABLE_IOEM_0:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_IOEM_0;
                break;

            case DBAL_PHYSICAL_TABLE_IOEM_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_IOEM_1;
                break;

            case DBAL_PHYSICAL_TABLE_MAP:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_MAP;
                break;

            case DBAL_PHYSICAL_TABLE_FEC_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_FEC_1;
                break;

            case DBAL_PHYSICAL_TABLE_FEC_2:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_FEC_2;
                break;

            case DBAL_PHYSICAL_TABLE_FEC_3:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_FEC_3;
                break;

            case DBAL_PHYSICAL_TABLE_MC_ID:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_MC_ID;
                break;

            case DBAL_PHYSICAL_TABLE_GLEM_0:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_GLEM_0;
                break;

            case DBAL_PHYSICAL_TABLE_GLEM_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_GLEM_1;
                break;

            case DBAL_PHYSICAL_TABLE_EEDB_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EEDB_1;
                break;

            case DBAL_PHYSICAL_TABLE_EEDB_2:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EEDB_2;
                break;

            case DBAL_PHYSICAL_TABLE_EEDB_3:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EEDB_3;
                break;

            case DBAL_PHYSICAL_TABLE_EEDB_4:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EEDB_4;
                break;

            case DBAL_PHYSICAL_TABLE_EOEM_0:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EOEM_0;
                break;

            case DBAL_PHYSICAL_TABLE_EOEM_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EOEM_1;
                break;

            case DBAL_PHYSICAL_TABLE_ESEM:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_ESEM;
                break;

            case DBAL_PHYSICAL_TABLE_EVSI:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EVSI;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_1:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EXEM_1;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_2:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EXEM_2;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_3:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EXEM_3;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_4:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_EXEM_4;
                break;

            case DBAL_PHYSICAL_TABLE_RMEP:
                mdb_direct_dbal_to_mdb[db_index] = MDB_PHYSICAL_TABLE_RMEP;
                break;

            default:
                mdb_direct_dbal_to_mdb[db_index] = MDB_NOF_PHYSICAL_TABLES;
        }
    }

    SHR_FUNC_EXIT;
}

/**
 *  initializes an array that associates DBAL physical tables with MDB EM registers.
 *  Depends on direct being initialized.
 */
shr_error_e
mdb_em_dbal_to_mdb_init(
    int unit)
{
    int db_index;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * First initialize the mapping between DBAL EM tables and MDB EM registers to be invalid
     */
    for (db_index = DBAL_PHYSICAL_TABLE_NONE; db_index < DBAL_NOF_PHYSICAL_TABLES; db_index++)
    {
        mdb_em_dbal_to_mdb[db_index] = INVALIDm;
    }

    /*
     * Traverse the logical tables:
     * a) Update the mapping between DBAL EM tables and MDB EM registers
     * b) Update the EM memory APP ID with the associated entry size
     */
    for (db_index = 0; db_index < DBAL_NOF_TABLES; db_index++)
    {
        uint32 key_size;
        int payload_size;
        const dbal_logical_table_t *dbal_logical_table;

        SHR_IF_ERR_EXIT(dbal_logical_table_get(unit, db_index, &dbal_logical_table));

        key_size = dbal_logical_table->key_size;
        payload_size = dbal_logical_table->max_payload_size;

        switch (dbal_logical_table->physical_db_id)
        {
            case DBAL_PHYSICAL_TABLE_ISEM_1:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_ISEM_1m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_ISEM_1_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_ISEM_1_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_ISEM_2:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_ISEM_2m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_ISEM_2_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_ISEM_2_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_ISEM_3:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_ISEM_3m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_ISEM_3_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_ISEM_3_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_LEM:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_LEMm;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_LEM_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_LEM_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_IOEM_0:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_IOEM_0m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_IOEM_TID_ATR_0m;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_IOEM_ENTRY_ENCODING_0r;
                break;

            case DBAL_PHYSICAL_TABLE_IOEM_1:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_IOEM_1m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_IOEM_TID_ATR_1m;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_IOEM_ENTRY_ENCODING_1r;
                break;

            case DBAL_PHYSICAL_TABLE_GLEM_0:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_GLEM_0m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_GLEM_TID_ATR_0m;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_GLEM_ENTRY_ENCODING_0r;
                break;

            case DBAL_PHYSICAL_TABLE_GLEM_1:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_GLEM_1m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_GLEM_TID_ATR_1m;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_GLEM_ENTRY_ENCODING_1r;
                break;

            case DBAL_PHYSICAL_TABLE_EOEM_0:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_EOEM_0m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_EOEM_TID_ATR_0m;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_EOEM_ENTRY_ENCODING_0r;
                break;

            case DBAL_PHYSICAL_TABLE_EOEM_1:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_EOEM_1m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_EOEM_TID_ATR_1m;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_EOEM_ENTRY_ENCODING_1r;
                break;

            case DBAL_PHYSICAL_TABLE_ESEM:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_ESEMm;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_ESEM_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_ESEM_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_1:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_EXEM_1m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_EXEM_1_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_EXEM_1_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_2:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_EXEM_2m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_EXEM_2_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_EXEM_2_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_3:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_EXEM_3m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_EXEM_3_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_EXEM_3_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_EXEM_4:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_EXEM_4m;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_EXEM_4_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_EXEM_4_ENTRY_ENCODINGr;
                break;

            case DBAL_PHYSICAL_TABLE_RMEP:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = MDB_RMEPm;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = MDB_RMEP_TID_ATRm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = MDB_RMEP_ENTRY_ENCODINGr;
                break;

            default:
                mdb_em_dbal_to_mdb[dbal_logical_table->physical_db_id] = INVALIDm;
                mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] = INVALIDm;
                mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] = INVALIDr;
        }

        /*
         * Fill the key size in the TID ATR memory per APP ID
         */
        if (mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id] != INVALIDm)
        {
            uint32 data[SOC_REG_ABOVE_64_MAX_SIZE_U32];

            SHR_IF_ERR_EXIT(soc_mem_read
                            (unit, mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id], SOC_BLOCK_ALL,
                             dbal_logical_table->app_id, data));

            soc_mem_field_set(unit, mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id], data, KEY_SIZEf,
                              &key_size);

            SHR_IF_ERR_EXIT(soc_mem_write
                            (unit, mdb_em_dbal_to_mdb_tid_atr[dbal_logical_table->physical_db_id], SOC_BLOCK_ALL,
                             dbal_logical_table->app_id, data));
        }

        /*
         * Fill the payload size in the ENTRY ENCODING register per APP ID
         */
        if (mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id] != INVALIDr)
        {
            soc_reg_above_64_val_t data;
            mdb_physical_tables_e mdb_physical_table = mdb_direct_dbal_to_mdb[dbal_logical_table->physical_db_id];
            int temp_total_size = mdb_direct_table_to_row_width[mdb_physical_table];
            mdb_em_entry_encoding_e entry_encoding = 0;

            SHR_IF_ERR_EXIT(soc_reg_above_64_get
                            (unit, mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id], REG_PORT_ANY,
                             0, data));

            /*
             * Calculate the entry_encoding as one/half/quarter/eighth of 240bits
             */
            while (temp_total_size >= payload_size + key_size)
            {
                entry_encoding++;
                temp_total_size /= 2;
            }
            entry_encoding--;
            /*
             * Only support payloads as small as 30bits
             */
            if (entry_encoding > MDB_EM_ENTRY_ENCODING_EIGHTH)
            {
                entry_encoding = MDB_EM_ENTRY_ENCODING_EIGHTH;
            }

            SHR_BITCOPY_RANGE(data, MDB_EM_ENTRY_ENCODING_NOF_BITS * dbal_logical_table->app_id, &entry_encoding, 0,
                              MDB_EM_ENTRY_ENCODING_NOF_BITS);

            mdb_em_app_id_to_entry_encoding[mdb_physical_table][dbal_logical_table->app_id] = entry_encoding;

            SHR_IF_ERR_EXIT(soc_reg_above_64_set
                            (unit, mdb_em_dbal_to_mdb_entry_encoding[dbal_logical_table->physical_db_id], REG_PORT_ANY,
                             0, data));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_init(
    int unit)
{
    int db_index, macro_index, cluster_counter;

    /*
     * The counters are used to allocate the clusters in each macro to specific DBs
     */
    uint8 macro_a_counter[MDB_NOF_MACRO_A];
    uint8 macro_b_counter[MDB_NOF_MACRO_B];

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(macro_a_counter, 0x0, MDB_NOF_MACRO_A * sizeof(macro_a_counter[0]));
    sal_memset(macro_b_counter, 0x0, MDB_NOF_MACRO_B * sizeof(macro_b_counter[0]));

    for (db_index = 0; db_index < MDB_NOF_PHYSICAL_TABLES; db_index++)
    {
        for (macro_index = 0; macro_index < (MDB_NOF_MACRO_A + MDB_NOF_MACRO_B); macro_index++)
        {
            /*
             * the counter value is the amount of associated clusters in the macro
             */
            for (cluster_counter = 0;
                 cluster_counter <
                 mdb_default_connection_array[(db_index * (MDB_NOF_MACRO_A + MDB_NOF_MACRO_B)) + macro_index];
                 cluster_counter++)
            {
                /*
                 * Update the appropriate address in the DB array, MACRO B array is right after MACRO A
                 */
                if (macro_index < MDB_NOF_MACRO_A)
                {
                    /*
                     * Update Macro A addresses
                     */
                    mdb_db_infos[db_index].macro_a_clusters_addr[mdb_db_infos[db_index].macro_a_clusters_num] =
                        (macro_index * MDB_NOF_MACRO_A_CLUSTERS) + macro_a_counter[macro_index];
                    macro_a_counter[macro_index]++;
                    mdb_db_infos[db_index].macro_a_clusters_num++;
                }
                else
                {
                    /*
                     * Update Macro B addresses
                     */
                    mdb_db_infos[db_index].macro_b_clusters_addr[mdb_db_infos[db_index].macro_b_clusters_num] =
                        ((macro_index - MDB_NOF_MACRO_A) * MDB_NOF_MACRO_B_CLUSTERS) +
                        macro_b_counter[(macro_index - MDB_NOF_MACRO_A)];
                    macro_b_counter[(macro_index - MDB_NOF_MACRO_A)]++;
                    mdb_db_infos[db_index].macro_b_clusters_num++;
                }
            }
        }
    }

    SHR_IF_ERR_EXIT(mdb_direct_dbal_to_mdb_init(unit));

    /*
     * Depends on direct being initialized
     */
    SHR_IF_ERR_EXIT(mdb_em_dbal_to_mdb_init(unit));

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    SHR_IF_ERR_EXIT(mdb_lpm_init(unit));
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_get_capacity(
    int unit,
    dbal_physical_table_def_t * dbal_physical_table,
    int *capacity)
{
    mdb_physical_tables_e mdb_physical_table = mdb_direct_dbal_to_mdb[dbal_physical_table->physical_db_type];
    mdb_physical_tables_e dbal_physical_table_id;
    mdb_direct_payload_sizes_types_e payload_type;
    uint32 row_width;
    /*
     * Holds 2 if the clusters are paired or 1 otherwise
     */
    uint32 row_factor;
    SHR_FUNC_INIT_VARS(unit);

    if (dbal_physical_table->physical_db_type == DBAL_PHYSICAL_TABLE_LPM_PRIVATE)
    {
        /*
         * Currently only support 64k entries (small KAPS)
         */
        *capacity = MDB_KAPS_PRIVATE_SIZE;
    }
    else if (dbal_physical_table->physical_db_type == DBAL_PHYSICAL_TABLE_LPM_PUBLIC)
    {
        /*
         * Currently only support 64k entries (small KAPS)
         */
        *capacity = MDB_KAPS_PUBLIC_SIZE;
    }
    else if (dbal_physical_table->physical_db_type == DBAL_PHYSICAL_TABLE_TCAM)
    {
        
        /*
         * SHR_IF_ERR_EXIT(SHR_E_PARAM);
         */
        *capacity = 100000;
    }
    else if (mdb_em_dbal_to_mdb[dbal_physical_table->physical_db_type] != INVALIDm)
    {
        /*
         * EM table
         */
        *capacity = MDB_EM_CMODEL_SIZE;
    }
    else if (mdb_direct_dbal_to_mdb[dbal_physical_table->physical_db_type] != MDB_NOF_PHYSICAL_TABLES)
    {
        /*
         * Direct table
         */
        dbal_physical_table_id = mdb_direct_dbal_to_mdb[dbal_physical_table->physical_db_type];
        payload_type = mdb_direct_table_to_payload_type[mdb_physical_table];
        row_width = mdb_direct_table_to_row_width[mdb_physical_table];
        row_factor = row_width / MDB_NOF_CLUSTER_ROW_BITS;

        if (payload_type != MDB_NOF_DIRECT_PAYLOAD_TYPES)
        {
            *capacity =
                ((mdb_db_infos[dbal_physical_table_id].macro_a_clusters_num / row_factor *
                  mdb_nof_entries_in_db[row_factor - 1][MDB_MACRO_A][payload_type]) +
                 (mdb_db_infos[dbal_physical_table_id].macro_b_clusters_num / row_factor *
                  mdb_nof_entries_in_db[row_factor - 1][MDB_MACRO_B][payload_type]));
        }
        else
        {
            *capacity = 0;
        }
    }
    else if (dbal_physical_table->physical_db_type == DBAL_PHYSICAL_TABLE_NONE)
    {
        *capacity = 0;
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_BADID, "Error. dbal_physical_table %d is not associated with an exact match memory.\n",
                     dbal_physical_table->physical_db_type);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * Strings mapping array for MDB access types:
 */
static char *mdb_db_types_strings[MDB_NOF_DB_TYPES] = {
    "Exact Match",
    "Direct Access",
    "TCAM",
    "KAPS (LPM)"
};

char *
mdb_get_db_type_str(
    mdb_db_type_e mdb_db_type)
{
    if (mdb_db_type >= MDB_NOF_DB_TYPES)
    {
        return mdb_unknown_string;
    }
    else
    {
        return mdb_db_types_strings[mdb_db_type];
    }
}

mdb_physical_tables_e
mdb_get_physical_table(
    dbal_physical_tables_e dbal_physical_table_id)
{
    if (dbal_physical_table_id >= DBAL_NOF_PHYSICAL_TABLES)
    {
        return MDB_NOF_PHYSICAL_TABLES;
    }
    else
    {
        return mdb_direct_dbal_to_mdb[dbal_physical_table_id];
    }
}

mdb_db_type_e
mdb_get_db_type(
    dbal_physical_tables_e dbal_physical_table_id)
{
    switch (dbal_physical_table_id)
    {
        case DBAL_PHYSICAL_TABLE_TCAM:
            return MDB_DB_TCAM;

        case DBAL_PHYSICAL_TABLE_LPM_PRIVATE:
            return MDB_DB_KAPS;

        case DBAL_PHYSICAL_TABLE_LPM_PUBLIC:
            return MDB_DB_KAPS;

        case DBAL_PHYSICAL_TABLE_ISEM_1:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_INLIF_1:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_IVSI:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_ISEM_2:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_INLIF_2:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_ISEM_3:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_INLIF_3:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_LEM:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_IOEM_0:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_IOEM_1:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_MAP:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_FEC_1:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_FEC_2:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_FEC_3:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_MC_ID:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_GLEM_0:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_GLEM_1:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_EEDB_1:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_EEDB_2:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_EEDB_3:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_EEDB_4:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_EOEM_0:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_EOEM_1:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_ESEM:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_EVSI:
            return MDB_DB_DIRECT;

        case DBAL_PHYSICAL_TABLE_EXEM_1:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_EXEM_2:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_EXEM_3:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_EXEM_4:
            return MDB_DB_EM;

        case DBAL_PHYSICAL_TABLE_RMEP:
            return MDB_DB_EM;

        default:
            return MDB_NOF_DB_TYPES;
    }
}
