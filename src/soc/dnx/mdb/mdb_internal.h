/*
 * ! \file mdb.h $Id$ Contains all of the internal MDB defines and globals. MDB Terminology Cluster - This is the
 * basic memory unit (each cluster is used for a single type of data base), has 16k/8k (MACROA/B) of 120b rows. Bucket
 * - A row of 4 clusters in indexes inside the MACRO 0-3 or 4-7. Macro - Contain 2 buckets (8 clusters). The MDB
 * consists of 12 Macro-A and 8 Macro-B The Macro-As are spread evenly between a single memory DHC_MACRO_ENTRY_BANK and
 * twelve blocks DHC0/../11. The Macro-Bs are spread evenly between two memories DDHB_MACRO_0/1_ENTRY_BANK and four
 * blocks DDHB0/1/2/3. Each Macro consists of 2 buckets, 4 clusters per bucket Macro-A cluster is 16k rows, Macro-B
 * cluster is 8k rows
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef MDB_H_INTERNAL_INCLUDED
/*
 * {
 */
#define MDB_H_INTERNAL_INCLUDED

/*
 * Include files.
 * {
 */
#include <soc/mem.h>
#include <soc/drv.h>
#include <soc/dnx/dbal/dbal_structures.h>
#include <shared/shrextend/shrextend_debug.h>
/*
 * }
 */

/*
 * DEFINEs
 * {
 */

/*
 * MDB general defines, for more information read the terminology section at the top of the file.
 */
#define MDB_ENTRY_DATA_SIZE_IN_BITS            128
/*
 * 8 ECC bits
 */
#define MDB_ENTRY_DATA_TOTAL_SIZE_IN_BITS      (MDB_ENTRY_DATA_SIZE_IN_BITS + 8)
#define MDB_ENTRY_BANK_SIZE_IN_BITS            544
#if (MDB_ENTRY_BANK_SIZE_IN_BITS % SAL_UINT32_NOF_BITS) != 0
#error "MDB_ENTRY_BANK_SIZE_IN_BITS assumes to be divided by SAL_UINT32_NOF_BITS"
#endif
#define MDB_ENTRY_BANK_SIZE_UINT32             (MDB_ENTRY_BANK_SIZE_IN_BITS/SAL_UINT32_NOF_BITS)
#if (MDB_ENTRY_DATA_SIZE_IN_BITS % SAL_UINT32_NOF_BITS) != 0
#error "MDB_ENTRY_DATA_SIZE_IN_BITS assumes to be divided by SAL_UINT32_NOF_BITS"
#endif
#define MDB_ENTRY_DATA_SIZE_UINT32             (MDB_ENTRY_DATA_SIZE_IN_BITS/SAL_UINT32_NOF_BITS)
#define MDB_ENTRY_DATA_PER_BANK                (MDB_ENTRY_BANK_SIZE_IN_BITS/MDB_ENTRY_DATA_SIZE_IN_BITS)

#define MDB_NOF_MACRO_A                         12
#define MDB_NOF_ROWS_IN_MACRO_A_CLUSTER        (16*1024)
#define MDB_NOF_MACRO_A_CLUSTERS                8

#define MDB_NOF_MACRO_B                         8
#define MDB_NOF_ROWS_IN_MACRO_B_CLUSTER        (8*1024)
#define MDB_NOF_MACRO_B_NOF_MEMS                2
#define MDB_NOF_MACRO_B_CLUSTERS                8
/*
 * Each direct entry size is a multiples of 30b
 */
#define MDB_DIRECT_BASIC_ENTRY_SIZE             30
#define MDB_NOF_CLUSTER_ROW_BITS                120
#define MDB_NOF_CLUSTER_IN_BUCKET               4


#define MDB_KAPS_PRIVATE_SIZE                   (64000)
#define MDB_KAPS_PUBLIC_SIZE                    (0)


#define MDB_EM_CMODEL_SIZE                      (40)

/*
 * Number of bits representing mdb_em_entry_encoding_e
 */
#define MDB_EM_ENTRY_ENCODING_NOF_BITS          (3)

/*
 * }
 */

/*
 * MACROs
 * {
 */
#define MDB_PAYLOAD_SIZE_TO_PAYLOAD_SIZE_TYPE(payload_size) ((payload_size / MDB_DIRECT_BASIC_ENTRY_SIZE) - 1)
#define MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(payload_type) ((payload_type + 1) * MDB_DIRECT_BASIC_ENTRY_SIZE)
/*
 * }
 */

/*
 * ENUMs
 * {
 */

typedef enum
{
    MDB_MACRO_A,
    MDB_MACRO_B,
    MDB_NOF_MACRO_TYPES
} mdb_macro_types_e;

typedef enum
{
    MDB_DIRECT_PAYLOAD_TYPE_30B,
    MDB_DIRECT_PAYLOAD_TYPE_60B,
    MDB_DIRECT_PAYLOAD_TYPE_90B,
    MDB_DIRECT_PAYLOAD_TYPE_120B,
    MDB_DIRECT_PAYLOAD_TYPE_150B,
    MDB_DIRECT_PAYLOAD_TYPE_180B,
    MDB_NOF_DIRECT_PAYLOAD_TYPES
} mdb_direct_payload_sizes_types_e;

typedef enum
{
    MDB_DIRECT_REMAINDER_TYPE_0B,
    MDB_DIRECT_REMAINDER_TYPE_30B,
    MDB_DIRECT_REMAINDER_TYPE_60B,
    MDB_DIRECT_REMAINDER_TYPE_90B,
    MDB_DIRECT_REMAINDER_TYPE_120B,
    MDB_DIRECT_REMAINDER_TYPE_150B,
    MDB_DIRECT_REMAINDER_TYPE_180B,
    MDB_DIRECT_REMAINDER_TYPE_210B,
    MDB_NOF_DIRECT_REMAINDER_TYPES
} mdb_direct_remainder_types_e;

typedef enum
{
    MDB_DIRECT_ROW_WIDTH_TYPE_120B,
    MDB_DIRECT_ROW_WIDTH_TYPE_240B,
    MDB_NOF_DIRECT_ROW_WIDTH_TYPES
} mdb_direct_row_width_types_e;

/*
 * These enum values are written as the MDB_EM_ENTRY_ENCODING_NOF_BITS-bit value in EMNAME_ENTRY_ENCODING registers.
 * They represent the payload size out of 240bits.
 */
typedef enum
{
    MDB_EM_ENTRY_ENCODING_ONE = 0,
    MDB_EM_ENTRY_ENCODING_HALF = 1,
    MDB_EM_ENTRY_ENCODING_QUARTER = 2,
    MDB_EM_ENTRY_ENCODING_EIGHTH = 3,
    MDB_EM_ENTRY_ENCODING_EMPTY = 7
} mdb_em_entry_encoding_e;

/*
 * }
 */

/*
 * Structures
 * {
 */
typedef struct mdb_db_info
{
    /*
     * mdb_db_type_e db_type;
     */

    /*
     * Number of type A clusters associated with this DB
     */
    uint8 macro_a_clusters_num;
    /*
     * Addressing is in clusters from 0 to MDB_NOF_MACRO_A * MDB_NOF_MACRO_A_CLUSTERS\n
     * This identifies a single cluster based on its index counting from the first cluster in its MACRO
     */
    uint8 macro_a_clusters_addr[MDB_NOF_MACRO_A];

    /*
     * Number of type B clusters associated with this DB
     */
    uint8 macro_b_clusters_num;
    /*
     * Addressing is in clusters from 0 to MDB_NOF_MACRO_B * MDB_NOF_MACRO_B_CLUSTERS\n
     * This identifies a single cluster based on its index counting from the first cluster in its MACRO
     */
    uint8 macro_b_clusters_addr[MDB_NOF_MACRO_B];

} mdb_db_info_t;
/*
 * }
 */
/*
 * Globals
 * {
 */

/*
 * }
 */
/*
 * Externs
 * {
 */

extern mdb_db_info_t mdb_db_infos[MDB_NOF_PHYSICAL_TABLES];
/*
 * Mapping between DBAL physical tables and MDB physical tables.
 * The DBAL:MDB mapping is 1:1 except for the KAPS (LPM), that has several MDB enums, but only a single DBAL enum (LPM).
 */
extern mdb_physical_tables_e mdb_direct_dbal_to_mdb[DBAL_NOF_PHYSICAL_TABLES];
/*
 * Mapping between DBAL EM tables and MDB EM registers.
 */
extern soc_mem_t mdb_em_dbal_to_mdb[DBAL_NOF_PHYSICAL_TABLES];
/*
 * Mapping between DBAL EM tables and MDB EM TID ATR registers.
 */
extern soc_mem_t mdb_em_dbal_to_mdb_tid_atr[DBAL_NOF_PHYSICAL_TABLES];
/*
 * Mapping between DBAL EM tables and MDB EM Entry Encoding registers.
 */
extern soc_reg_t mdb_em_dbal_to_mdb_entry_encoding[DBAL_NOF_PHYSICAL_TABLES];
/*
 * Mapping between MDB EM tables and APP_ID to entry_encoding.
 */
extern mdb_em_entry_encoding_e mdb_em_app_id_to_entry_encoding[MDB_NOF_PHYSICAL_TABLES][MDB_NOF_APP_IDS];
/*
 * This array contain the number of entries that a single DB instance can contain, a single DB instance will be a
 * cluster in 120 mode or a pair of adjacent clusters in 240 mode.
 */
extern const uint32
    mdb_nof_entries_in_db[MDB_NOF_DIRECT_ROW_WIDTH_TYPES][MDB_NOF_MACRO_TYPES][MDB_NOF_DIRECT_PAYLOAD_TYPES];


extern const mdb_direct_payload_sizes_types_e mdb_direct_table_to_payload_type[MDB_NOF_PHYSICAL_TABLES];

/*
 * The row width for each direct table.
 */
const mdb_direct_payload_sizes_types_e mdb_direct_table_to_row_width[MDB_NOF_PHYSICAL_TABLES];

/*
 * }
 */

/*
 * Function headers
 * {
 */

/**
 * \brief
 *  test - returns whether or not the the payload is fully or partially masked.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] entry - The received entry payload that is about to enter the MDB
 *  \param [in] is_masked - Indicate if the entry is masked or not.
 * \par INDIRECT INPUT
 *  See entry above.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *  See is_masked above
 */
shr_error_e mdb_direct_is_payload_masked(
    int unit,
    dbal_physical_entry_t * entry,
    int *is_masked);

/*
 * Initialize the DBs and searches in the KAPS.
 */

/**
 * \brief
 *  Initialize the DBs and searches in the KAPS.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_lpm_init(
    int unit);

/**
 * \brief
 *  Initialize the DBs and searches in the KAPS.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_lpm_init(
    int unit);

/**
 * \brief
 *  Initialize the XPT struct passed to the KBPSDK.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] xpt - KBPSDK XPT handler.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_lpm_xpt_init(
    int unit,
    void **xpt);

/*
 * }
 */
#endif /* !MDB_H_INTERNAL_INCLUDED */
