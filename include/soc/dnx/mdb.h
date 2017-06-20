/*
 * $Id$ Contains all of the MDB access functions provided to the DBAL.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef MDB_H_INCLUDED
/*
 * {
 */
#define MDB_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <soc/mem.h>
#include <soc/drv.h>
#include <soc/dnx/dbal/dbal_structures.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dnx/swstate/access/mdb_kaps_access.h>

#include <soc/kbp/alg_kbp/include/db.h>
#include <soc/kbp/alg_kbp/include/default_allocator.h>
#include <soc/kbp/alg_kbp/include/device.h>
#include <soc/kbp/alg_kbp/include/key.h>
#include <soc/kbp/alg_kbp/include/instruction.h>
#include <soc/kbp/alg_kbp/include/errors.h>
#include <soc/kbp/alg_kbp/include/ad.h>
#include <soc/kbp/alg_kbp/include/kbp_legacy.h>
#include <soc/kbp/alg_kbp/include/init.h>
#include <soc/kbp/alg_kbp/include/kbp_portable.h>
#include <soc/kbp/alg_kbp/include/dma.h>
#include <soc/kbp/alg_kbp/include/xpt_kaps.h>
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

#include <shared/shrextend/shrextend_debug.h>
#include <shared/swstate/sw_state.h>

/*
 * Maximum number of clusters associated with a single physical DB
 */
#define MDB_MAX_NOF_CLUSTERS     40

/*
 * Current is runtime calculation, depending on the device
 */
#define MDB_CURRENT_NOF_CORES    2
/*
 * The ID of the single search instruction in MDB KAPS
 */
#define MDB_LPM_INSTRUCTIONS_ID  0

/*
 * The KAPS(LPM) AD(payload) size
 */
#define MDB_KAPS_AD_WIDTH_IN_BITS               (20)
#define MDB_KAPS_AD_WIDTH_IN_BYTES              (3)

/*
 * The KAPS(LPM) key size
 */
#define MDB_KAPS_KEY_WIDTH_IN_BITS              (160)
#define MDB_KAPS_KEY_WIDTH_IN_BYTES             (MDB_KAPS_KEY_WIDTH_IN_BITS/SAL_UINT8_NOF_BITS)
#define MDB_KAPS_KEY_WIDTH_IN_UINT32            (MDB_KAPS_KEY_WIDTH_IN_BITS/SAL_UINT32_NOF_BITS)

/*
 * The KAPS DB prefix length
 */
#define MDB_KAPS_KEY_PREFIX_LENGTH              (6)

/*
 * The KAPS DB prefixes, the prefix length is MDB_KAPS_KEY_PREFIX_LENGTH
 */
typedef enum
{
    MDB_KAPS_KEY_PREFIX_IPV4_UC = 0,
    MDB_KAPS_KEY_PREFIX_IPV4_MC = 2,
    MDB_KAPS_KEY_PREFIX_IPV6_UC = 3,
    MDB_KAPS_KEY_PREFIX_IPV6_MC = 4,
    MDB_KAPS_KEY_PREFIX_FCOE = 6,

    MDB_NOF_KAPS_KEY_PREFIX
} mdb_kaps_key_prefix_e;

/*
 * The largest payload size associated with a direct table is 180 bits (IN_LIF)
 */
#define MDB_MAX_DIRECT_PAYLOAD_SIZE_IN_UINT32   (6)

/*
 * The largest payload size associated with an EM table is 160 bits(LEM)
 */
#define MDB_MAX_EM_KEY_SIZE_IN_UINT32           (5)
/*
 * The largest payload size associated with an EM table is 180 bits
 */
#define MDB_MAX_EM_KEY_PAYLOAD_IN_UINT32        (6)

/*
 * The number of different APP IDs
 */
#define MDB_NOF_APP_IDS                         (64)

/*
 * The number of entries tested is capacity/MDB_TEST_BRIEF_FACTOR
 */
#define MDB_TEST_BRIEF_FACTOR                   (10)

typedef struct kbp_key *mdb_kaps_key_t_p;
typedef struct kbp_dma_db *mdb_kaps_dma_db_t_p;

/**
 * \brief An enum that represents the two DB types of KAPS(LPM).
 */

/**
 * \brief An enum that holds the different MDB Physical tables.
 */
typedef enum
{
    MDB_PHYSICAL_TABLE_ISEM_1,
    MDB_PHYSICAL_TABLE_INLIF_1,
    MDB_PHYSICAL_TABLE_IVSI,
    MDB_PHYSICAL_TABLE_ISEM_2,
    MDB_PHYSICAL_TABLE_INLIF_2,
    MDB_PHYSICAL_TABLE_ISEM_3,
    MDB_PHYSICAL_TABLE_INLIF_3,
    MDB_PHYSICAL_TABLE_LEM,
    MDB_PHYSICAL_TABLE_ADS_1,
    MDB_PHYSICAL_TABLE_ADS_2,
    MDB_PHYSICAL_TABLE_KAPS_1,
    MDB_PHYSICAL_TABLE_KAPS_2,
    MDB_PHYSICAL_TABLE_IOEM_0,
    MDB_PHYSICAL_TABLE_IOEM_1,
    MDB_PHYSICAL_TABLE_MAP,
    MDB_PHYSICAL_TABLE_FEC_1,
    MDB_PHYSICAL_TABLE_FEC_2,
    MDB_PHYSICAL_TABLE_FEC_3,
    MDB_PHYSICAL_TABLE_MC_ID,
    MDB_PHYSICAL_TABLE_GLEM_0,
    MDB_PHYSICAL_TABLE_GLEM_1,
    MDB_PHYSICAL_TABLE_EEDB_1,
    MDB_PHYSICAL_TABLE_EEDB_2,
    MDB_PHYSICAL_TABLE_EEDB_3,
    MDB_PHYSICAL_TABLE_EEDB_4,
    MDB_PHYSICAL_TABLE_EOEM_0,
    MDB_PHYSICAL_TABLE_EOEM_1,
    MDB_PHYSICAL_TABLE_ESEM,
    MDB_PHYSICAL_TABLE_EVSI,
    MDB_PHYSICAL_TABLE_EXEM_1,
    MDB_PHYSICAL_TABLE_EXEM_2,
    MDB_PHYSICAL_TABLE_EXEM_3,
    MDB_PHYSICAL_TABLE_EXEM_4,
    MDB_PHYSICAL_TABLE_RMEP,

    MDB_NOF_PHYSICAL_TABLES
} mdb_physical_tables_e;

/**
 * \brief List of the different data bases types that the MDB supports
 */
typedef enum
{
    /*
     * Exact Match
     */
    MDB_DB_EM,
    /*
     * Direct Access
     */
    MDB_DB_DIRECT,
    /*
     * Ternary Content-Addressable Memory
     */
    MDB_DB_TCAM,
    /*
     * Kbp Assisted Prefix Search (LPM)
     * KBP - Knowledge Base Processor
     */
    MDB_DB_KAPS,

    MDB_NOF_DB_TYPES
} mdb_db_type_e;

/**
 * \brief List of the different test modes
 */
typedef enum
{
    /*
     * Quick test, randomly tests capacity/MDB_TEST_BRIEF_FACTOR entries
     */
    MDB_TEST_BRIEF,
    /*
     * Full test, tests the full capacity
     */
    MDB_TEST_FULL,

    MDB_NOF_TEST_MODES
} mdb_test_mode_e;

/**
 * \brief A struct used to pass cluster information for prints.
 */
typedef struct
{
    /*
     * Cluster information
     */

    /*
     * The cluster global address
     */
    uint8 cluster_address;
    /*
     * The memory, either DDHB_MACRO_0/1_ENTRY_BANKm or DHC_MACRO_ENTRY_BANKm
     */
    soc_mem_t macro_mem;
    /*
     * The block, either DDHB_BLOCK[0..7] or DHC_BLOCK[0..11]
     */
    int blk;
    int blk_id;
    /*
     * The array(bucket) index in the memory, either 0 or 1
     */
    uint32 bucket_index;
    /*
     * The offset within the bucket
     */
    uint32 bucket_offset;

    /*
     * Specific entry information
     */

    /*
     * The row within the cluster
     */
    int cluster_row;
    /*
     * The offset within the cluster row
     */
    int cluster_row_offset;
} mdb_cluster_info_t;

/**
 * \brief A struct used to pass DBAL physical table information for prints.
 */
typedef struct
{
    /*
     * The access resolution for direct access in bits, 30/60/90/120/150/180 bits
     */
    int direct_access_resolution;
    /*
     * The row size, 120/240/480 bits
     */
    int row_width;

    /*
     * An array holding the information for all clusters associated with this table
     */
    mdb_cluster_info_t cluster_info_array[MDB_MAX_NOF_CLUSTERS];
    int nof_clusters;
} mdb_dbal_physical_table_access_info_t;

extern const char cmd_mdb_usage[];

/**
 * \brief
 *   Initialize the MDB databases based on the connection array retrieved from the XML.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 * \par DIRECT OUTPUT:
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT:
 *   mdb_db_infos -\n
 *      An array that holds for each physical table the addresses of its physical resources.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *   mdb_direct_dbal_to_mdb_init - initializes an array that translates from DBAL physical table enum to MDB.
 *   mdb_em_dbal_to_mdb_init - initializes an array that associates DBAL physical tables with MDB EM registers.
 */
shr_error_e mdb_init(
    int unit);

/**
 * \brief
 *   Retrieve the allocated capacity for the specified physical table in 120bit rows.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] dbal_physical_table -\n
 *      The physical table in which we set the entry.
 *   \param [out] capacity -\n
 *      The capacity allocated for the physical_table in 120bit rows.
 * \par DIRECT OUTPUT:
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 * \see
 *   shr_error_e
 */
shr_error_e mdb_get_capacity(
    int unit,
    dbal_physical_table_def_t * dbal_physical_table,
    int *capacity);

/**
 * \brief
 *   Retrieve the MDB direct table associated with the dbal_physical_table_id.
 * \par DIRECT INPUT:
 *   \param [in] dbal_physical_table_id -\n
 *      The DBAL physical table id
 * \par DIRECT OUTPUT:
 *   See \ref mdb_physical_tables_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 */
mdb_physical_tables_e mdb_get_physical_table(
    dbal_physical_tables_e dbal_physical_table_id);

/**
 * \brief
 *   Get the EM key size associated with the DBAL physical table
 *   and app_id.\n Note that this function retrieves the
 *   information from HW.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] dbal_physical_table_id -\n
 *      The physical table id.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] key_size -\n
 *      The retrieved key entry size will be returned by this value.
 * \par INDIRECT INPUT
 *  See entry above
 * \par DIRECT OUTPUT:
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected, for example if the table is not associated with a direct table. See \ref shr_error_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \see
 *   shr_error_e
 */
shr_error_e mdb_em_get_key_size(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    uint32 * key_size);

/**
 * \brief
 *   Get the EM entry size associated with the DBAL
 *   physical table and app_id.\n Note that this function
 *   retrieves the information from SW.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] dbal_physical_table_id -\n
 *      The physical table id.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry_size -\n
 *      The retrieved entry size will be returned by this value.
 * \par INDIRECT INPUT
 *  See entry above
 * \par DIRECT OUTPUT:
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected, for example if the table is not associated with a direct table. See \ref shr_error_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \see
 *   shr_error_e
 */
shr_error_e mdb_em_get_entry_size(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    uint32 * entry_size);

/**
 * \brief
 *   Retrieve the DB type (Direct/EM/LPM/TCAM) of the dbal_physical_table_id.
 * \par DIRECT INPUT:
 *   \param [in] dbal_physical_table_id -\n
 *      The DBAL physical table id
 * \par DIRECT OUTPUT:
 *   See \ref mdb_db_type_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 */
mdb_db_type_e mdb_get_db_type(
    dbal_physical_tables_e dbal_physical_table_id);

/**
 * \brief
 *   Retrieve the string describing the enum.
 * \par DIRECT INPUT:
 *   \param [in] mdb_db_type -\n
 *      The MDB db type
 * \par DIRECT OUTPUT:
 *    char * - \n
 *      output string
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 */
char *mdb_get_db_type_str(
    mdb_db_type_e mdb_db_type);

/**
 * \brief
 *   Retrieve DBAL physical table access info.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] mdb_physical_table -\n
 *      The MDB physical table id
 *   \param [in] table_access_info -\n
 *      The retrieved table info
 * \par DIRECT OUTPUT:
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 */
shr_error_e mdb_get_db_access_info(
    int unit,
    mdb_physical_tables_e mdb_physical_table,
    mdb_dbal_physical_table_access_info_t * table_access_info);

/**
 * \brief
 *   Get the basic direct entry size associated with the DBAL physical table.\n
 *   This entry size is used for the indexation of entries. \n
 *   For example: The EEDB basic entry size is 30 bits,\n
 *   meaning if we have 120bit entries, the direct table key must increment by 4 for each entry.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] dbal_physical_table_id -\n
 *      The physical table id.
 *   \param [in] basic_size -\n
 *      The retrieved basic entry size will be returned by this value.
 * \par INDIRECT INPUT
 *  See entry above
 *   mdb_direct_table_to_row_width - mdb table to basic size translation.
 * \par DIRECT OUTPUT:
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected, for example if the table is not associated with a direct table. See \ref shr_error_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \see
 *   shr_error_e
 */
shr_error_e mdb_direct_table_get_basic_size(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    int *basic_size);

/**
 * \brief
 *   Set a direct access entry in the specified table at the specified index.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table in which we set the entry.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to set. \n
 *      * entry.key - the entry index. \n
 *      * entry.payload - the entry content. \n
 *      * entry.payload_size - the entry size in bytes.\n
 *      * entry.p_mask - the entry mask, if not complete, the function will perform read-modify-write.
 * \par INDIRECT INPUT
 *  See entry above
 *   mdb_payload_type_to_row_size - entry size into row size (e.g. entry size of 150b will use a 240b row).
 *   mdb_entry_info - All the access information into the clusters for a given entry size and a remainder.
 * \par DIRECT OUTPUT:
 *   \retval Zero if no error was detected
 *   \retval Negative if error was detected. See \ref shr_error_e
 * \par INDIRECT OUTPUT:
 *   * None.
 * \see
 *   shr_error_e
 */
shr_error_e mdb_direct_table_entry_add(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/** ****************************************************
 * \brief
 *   Get a direct access entry in the specified table at the specified index.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table from which we get the entry.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.key - the entry index. \n
 *      entry.payload_size - the entry size in bytes.
 * \par INDIRECT INPUT
 *  See entry above
 *   mdb_payload_type_to_row_size - entry size into row size (e.g. entry size of 150b will use a 240b row).
 *   mdb_entry_info - All the access information into the clusters for a given entry size and a remainder.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error.
 * \par INDIRECT OUTPUT:
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.payload - the entry content.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_direct_table_entry_get(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/** ****************************************************
 * \brief
 *   Delete (set entry to 0) a direct access entry in the specified table at the specified index.\n
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table from which we delete the entry.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to delete. \n
 *      entry.key - the entry index. \n
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error.
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_direct_table_entry_delete(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/**
 * \brief
 *  Test direct tables by adding and deleting entries to all indices and verifying the content on read.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] mode - The test mode, 0(brief) or 1(full).
 * \par INDIRECT INPUT
 *  mdb_direct_dbal_to_mdb - convert from DBAL DB type to MDB DB type
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_direct_table_test(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    mdb_test_mode_e mode);

/**
 * \brief
 *  Init dbal iterator for mdb direct table, called by
 *  dbal_iterator_init() .
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] app_id - The application id.
 *  \param [in] physical_entry_iterator - dbal physical iterator
 *         structure.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *  iterator_info - updated with the required parameters
 */
shr_error_e mdb_direct_table_iterator_init(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator);

/**
 * \brief
 *  get next entry in mdb direct table, acccording to
 *  information stored in iterator_info
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] app_id - The application id.
 *  \param [in] physical_entry_iterator - dbal physical iterator
 *         structure.
 *  \param [in] entry - The next entry retrieved by the
 *         iterator.
 *  \param [in] is_end - has the iterator reached the end.
 *         retrieved entry is invalid.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *   \param [in] entry - the next entry fetched.
 */
shr_error_e mdb_direct_table_iterator_get_next(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator,
    dbal_physical_entry_t * entry,
    uint8 * is_end);

/**
 * \brief
 *  de-init iterator API for mdb direct tables. This function
 *  has no effect for direct table.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] app_id - The application id.
 *  \param [in] physical_entry_iterator - dbal physical iterator
 *         structure.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_direct_table_iterator_deinit(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator);

/** ****************************************************
 * \brief
 *   Add an exact match entry in the specified table.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table in which we set the entry.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to set. \n
 *      entry.key - the entry key. \n
 *      entry.key_size - the entry key size in bytes. \n
 *      entry.payload - the entry content. \n
 *      entry.payload_size - the entry size in bytes.\n
 *      entry.p_mask - the entry mask, if not complete, the function will perform read-modify-write.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error.
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_em_entry_add(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/** ****************************************************
 * \brief
 *   Get an exact match entry from the specified table.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table from which we get the entry.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.key - the entry key. \n
 *      entry.key_size - the entry key size in bytes. \n
 *      entry.payload_size - the entry size in bytes.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error.
 * \par INDIRECT OUTPUT:
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.payload - the entry content.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_em_entry_get(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/** ****************************************************
 * \brief
 *   Delete an exact match entry in the specified table.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table from which we delete the entry.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to set. \n
 *      entry.key - the entry key. \n
 *      entry.key_size - the entry key size in bytes.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error.
 * \par INDIRECT OUTPUT:
 *   * None.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_em_entry_delete(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/**
 * \brief
 *  Test exact match tables by adding and reading entries, until max capacity has been reached.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] mode - The test mode, 0(brief) or 1(full).
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_em_test(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    mdb_test_mode_e mode);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

/** ****************************************************
 * \brief
 *   Add a longest prefix match entry to the private/public database.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table in which we set the entry, private/public.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to set. \n
 *      entry.key - the entry key, up to 160bits. \n
 *      entry.k_mask - the entry key mask, up to 160bits. \n
 *      entry.payload - the entry payload, 20 bits.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error, can return _SHR_E_FULL if full or _SHR_E_INTERNAL for KBPSDK internal error.
 * \par INDIRECT OUTPUT:
 *   * Adds the entry through the KBPSDK.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_lpm_entry_add(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/** ****************************************************
 * \brief
 *   Get the data associated with a longest prefix match entry
 *   from the private/public SW database.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table in which we set the entry, private/public.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.key - the entry key, up to 160bits. \n
 *      entry.k_mask - the entry key mask, up to 160bits.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error, can return _SHR_E_NOT_FOUND if entry not found or _SHR_E_INTERNAL for KBPSDK internal error.
 * \par INDIRECT OUTPUT:
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.payload - the entry content.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_lpm_entry_get(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/** ****************************************************
 * \brief
 *   Get the data associated with a longest prefix match entry
 *   from the private/public HW database.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] core -\n
 *      The core number.
 *   \param [in] physical_table -\n
 *      The physical table in which we set the entry, private/public.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.key - the entry key, up to 160bits.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error, can return _SHR_E_NOT_FOUND if entry not found or _SHR_E_INTERNAL for KBPSDK internal error.
 * \par INDIRECT OUTPUT:
 *   \param [in] entry -\n
 *      The entry we would like to get. \n
 *      entry.payload - the entry content.\n
 *      entry.k_mask - the entry key mask, up to 160bits.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_lpm_entry_search(
    int unit,
    int core,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/** ****************************************************
 * \brief
 *   Delete a longest prefix match entry from the private/public database.
 * \par DIRECT INPUT:
 *   \param [in] unit -\n
 *      The unit number.
 *   \param [in] physical_table -\n
 *      The physical table in which we set the entry, private/public.
 *   \param [in] app_id -\n
 *      The application id.
 *   \param [in] entry -\n
 *      The entry we would like to delete. \n
 *      entry.key - the entry key, up to 160bits. \n
 *      entry.k_mask - the entry key mask, up to 160bits.
 * \par DIRECT OUTPUT:
 *   shr_error_e -\n
 *     Value of corresponding BCM API error, can return _SHR_E_NOT_FOUND if entry not found or _SHR_E_INTERNAL for KBPSDK internal error.
 * \par INDIRECT OUTPUT:
 *   * Deletes the entry through the KBPSDK.
 * \remark
 *   None
 * \see
 *   shr_error_e
 *****************************************************/
shr_error_e mdb_lpm_entry_delete(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

/**
 * \brief
 *  Test LPM tables by adding and reading entries, until max capacity has been reached.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] mode - The test mode, 0(brief) or 1(full).
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_lpm_test(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    mdb_test_mode_e mode);

/**
 * \brief
 *  Init dbal iterator for mdb lpm table, called by
 *  dbal_iterator_init() .  Important: Deinit must be called to
 *  avoid memory leak.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] app_id - The application id.
 *  \param [in] physical_entry_iterator - dbal physical iterator
 *         structure.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *  iterator_info - updated with the required parameters
 */
shr_error_e mdb_lpm_iterator_init(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator);

/**
 * \brief
 *  get next entry in mdb lpm table, based on the information
 *  stored in physical_entry_iterator
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] app_id - The application id.
 *  \param [in] physical_entry_iterator - dbal physical iterator
 *         structure.
 *  \param [in] entry - The next entry retrieved by the
 *         iterator.
 *  \param [in] is_end - has the iterator reached the end.
 *         retrieved entry is invalid.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *   \param [in] entry - the next entry fetched.
 *  payload
 */
shr_error_e mdb_lpm_iterator_get_next(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator,
    dbal_physical_entry_t * entry,
    uint8 * is_end);

/**
 * \brief
 *  de-init iterator API for mdb LPM tables.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] dbal_physical_table_id - Physical table ID that the entry belongs to.
 *  \param [in] app_id - The application id.
 *  \param [in] physical_entry_iterator - dbal physical iterator
 *         structure.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 */
shr_error_e mdb_lpm_iterator_deinit(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator);

/**
 * \brief
 * This function calculates and returns the prefix length of the provided payload_mask,
 * it assumes the array is at least MDB_KAPS_KEY_WIDTH_IN_UINT32 long.
 * It also assumes the on bits are aligned to the msb.
 */
uint32 mdb_lpm_calculate_prefix_length(
    uint32 * payload_mask);

/**
 * \brief
 * Translate from dbal_physical_tables_e to mdb_kaps_ip_db_id_e
 */
shr_error_e mdb_lpm_dbal_to_db(
    int unit,
    dbal_physical_tables_e physical_table,
    mdb_kaps_ip_db_id_e * db_idx);

/**
 * \brief
 *  get next entry in mdb lpm table, based on the information
 *  stored in physical_entry_iterator
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] prefix_length - The prefix length 0 - 160.
 *  \param [in] entry - The entry with k_mask field to fill.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *   \param [in] entry.k_mask - Filled with prefix_length.
 *  payload
 */
shr_error_e mdb_lpm_prefix_len_to_mask(
    int unit,
    int prefix_length,
    dbal_physical_entry_t * entry);

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

/*
 * }
 */
#endif /* !MDB_H_INCLUDED */
