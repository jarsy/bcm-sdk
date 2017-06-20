/*
 * ! \file mdb_direct.c $Id$ Contains all of the MDB direct table access functions provided to the DBAL.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/*
 * Includes
 * {
 */

#include <soc/dnx/mdb.h>
#include <soc/dnx/dbal/dbal_structures.h>
#include <shared/utilex/utilex_bitstream.h>
#include <shared/utilex/utilex_framework.h>
#include "../dbal/dbal_internal.h"
#include "mdb_internal.h"
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif

/*
 * }
 */

/*
 * Defines
 * {
 */

#define BSL_LOG_MODULE BSL_LS_SOCDNX_MDBDNX
#define MDB_ENTRY_EMPTY_CONTROL           0

/*
 * Cluster 128 bits row structure -> <8 bits ECC><120 bits data><5 bits format><3 bits command>
 */
#define MDB_ENTRY_DATA_OFFSET             8
#define MDB_ENTRY_FORMAT_OFFSET           3
#define MDB_MAX_CLUSTER_PER_DB            2

/** The minimal number of UINT32 that can hold the largest entry */
#define MDB_MAX_DIRECT_PAYLOAD_SIZE_32    MDB_CEILING(SAL_UINT32_NOF_BITS,\
                                                  MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_NOF_DIRECT_PAYLOAD_TYPES-1))

/*
 * }
 */

/*
 * MACROs
 * {
 */

/*
 *  Command size is no more than 3 bits and the format size it no more than 5 bits.
 *  When using this MACRO make sure that the command and the format don't exceed their sizes.
 */
#define MDB_ENTRY_SET_CONTROL_BITS(command, format) (command | (format << MDB_ENTRY_FORMAT_OFFSET))

/*
 * Calculate the entry offset inside a row
 */
#define MDB_ENTRY_REMAINDER_CALC(entry_index,entries_size,row_width) ((entry_index * entries_size) % row_width)
/*
 * Get the type (mdb_direct_remainder_types_e enum) of the remainder
 */
#define MDB_GET_REMAINDER_TYPE(entry_index,entries_size,row_width)\
                             (MDB_ENTRY_REMAINDER_CALC(entry_index,entries_size,row_width)/MDB_DIRECT_BASIC_ENTRY_SIZE)

/*
 * Return the ceiling value of y/x
 */
#define MDB_CEILING(x,y) ((x -1 + y)/x)

/*
 * }
 */

/*
 * ENUMs
 * {
 */

/**
 * \brief List of the instructions that are used for handling one cluster row data \n
 */
typedef enum
{
    /** Read the cluster content */
    MDB_CLUSTER_READ_INST = 1,
    /** Write content to the cluster */
    MDB_CLUSTER_WRITE_INST = 2,
    /** Read the cluster content from the next row */
    MDB_CLUSTER_READ_NEXT_ROW_INST = 3,
    /** Write content to the next row of the cluster */
    MDB_CLUSTER_WRITE_NEXT_ROW_INST = 4,
} mdb_cluster_instructions_e;


/**
 * \brief List of the formats that are used for handling one cluster row data \n
 */
typedef enum
{
    /** Default format - no format */
    MDB_CLUSTER_FORMAT_NONE,
    /** Use one row below the specified row in the cluster   */
    MDB_CLUSTER_FORMAT_NEXT_ROW = 1
} mdb_cluster_format_e;

/*
 * \brief List of the NOF clusters that are used for a single direct entry
 */
typedef enum
{
    /** Don't use any cluster in this case (invalid case) */
    MDB_CLUSTER_USE_NONE,
    /** Only one cluster is use for writing this entry to the bucket */
    MDB_CLUSTER_USE_ONE = 1,
    /** Two clusters are used for writing this entry to the bucket */
    MDB_CLUSTER_USE_TWO = 2
} mdb_nof_clusters_to_use_e;

/**
 * \brief When writing a direct entry, up to two clusters can be used. This ENUM display all the two clusters
 * combinations in a bitmap for a single entry use.  \n
 */
typedef enum
{
    /** This entry doesn't use any clusters (For invalid cases) */
    MDB_CLUSTER_BITMAP_NONE = 0x0,
    /** This entry uses only the first cluster*/
    MDB_CLUSTER_BITMAP_FIRST = 0x1,
    /** This entry uses only the second cluster*/
    MDB_CLUSTER_BITMAP_SECOND = 0x2,
    /** This entry uses both the first and the second clusters*/
    MDB_CLUSTER_BITMAP_BOTH = 0x3
} mdb_cluster_use_bitmap_e;

/*
 * \brief Each DB can use one or two rows (in adjacent clusters), this ENUM is the offset of the cluster to use from
 * the DB basic cluster (0-1).
 */
typedef enum
{
    /** Use the basic cluster of the DB */
    MDB_CLUSTER_OFFSET_NONE = 0,
    /** Use the next cluster to basic cluster of the DB*/
    MDB_CLUSTER_OFFSET_ONE = 1
} mdb_cluster_offset_e;

/*
 * \brief The data in a cluster row starts after 8 bits of control, the offsets inside the 120b data itself can be in
 *  multiples of 30b (e.g if we want to writ to the cluster data after 60b the row offset will be 60+8).
 *  This is a list of all the possible offsets in a cluster row to start a new entry.
 */
typedef enum
{
    /** This offset doesn't in use */
    MDB_CLUSTER_DATA_OFFSET_NA = 0,
    /** Write to the cluster row after 8 bits of control and 0 bits of data  */
    MDB_CLUSTER_DATA_OFFSET_0b = MDB_ENTRY_DATA_OFFSET + (MDB_DIRECT_BASIC_ENTRY_SIZE * 0),
    /** Write to the cluster row after 8 bits of control and 30 bits of data  */
    MDB_CLUSTER_DATA_OFFSET_30b = MDB_ENTRY_DATA_OFFSET + (MDB_DIRECT_BASIC_ENTRY_SIZE * 1),
    /** Write to the cluster row after 8 bits of control and 60 bits of data  */
    MDB_CLUSTER_DATA_OFFSET_60b = MDB_ENTRY_DATA_OFFSET + (MDB_DIRECT_BASIC_ENTRY_SIZE * 2),
    /** Write to the cluster row after 8 bits of control and 90 bits of data  */
    MDB_CLUSTER_DATA_OFFSET_90b = MDB_ENTRY_DATA_OFFSET + (MDB_DIRECT_BASIC_ENTRY_SIZE * 3)
} mdb_cluster_row_offset_e;

/*
 * \brief Sometime a single entry is placed inside two clusters, the first cluster will always use the entry at the
 * start of it while the second cluster will use the entry from the end point of the first.
 * This is a list of possible entry offsets (multiples of 30b).
 */
typedef enum
{
    /** Use the entry starting from bit 0    */
    MDB_ENTRY_OFFSET_0b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 0),
    /** Use the entry starting from bit 30   */
    MDB_ENTRY_OFFSET_30b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 1),
    /** Use the entry starting from bit 60    */
    MDB_ENTRY_OFFSET_60b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 2),
    /** Use the entry starting from bit 90    */
    MDB_ENTRY_OFFSET_90b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 3),
    /** Use the entry starting from bit 120    */
    MDB_ENTRY_OFFSET_120b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 4)
} mdb_entry_offset_e;

/*
 * \brief The part of the entry to use (given the offset to start with inside the entry), the cluster can use
 * only 120b at most from a given entry (multiples of 30b).
 * This is a list of  all entry part sizes that can be used by a cluster.
 */
typedef enum
{
    /** Use the entry starting from bit 0    */
    MDB_ENTRY_SIZE_0b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 0),
    /** Use the entry starting from bit 30   */
    MDB_ENTRY_SIZE_30b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 1),
    /** Use the entry starting from bit 60    */
    MDB_ENTRY_SIZE_60b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 2),
    /** Use the entry starting from bit 90    */
    MDB_ENTRY_SIZE_90b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 3),
    /** Use the entry starting from bit 120    */
    MDB_ENTRY_SIZE_120b = (MDB_DIRECT_BASIC_ENTRY_SIZE * 4)
} mdb_entry_size_e;

/*
 * }
 */

/*
 * Structures
 * {
 */

/*
 * This structure holds all the needed information to add a partial/full entry payload into a single cluster.
 * That is payload's and cluster's row positioning info.
 */

typedef struct mdb_cluster_direct_entry_access_info
{
    /*
     * The cluster offset from the DB basic cluster number (0-1).
     */
    mdb_cluster_offset_e cluster_offset;
    /*
     * The offset (bits) inside the cluster row to accesses (will always be in multiples of 30b).
     */
    mdb_cluster_row_offset_e cluster_row_offset;
    /*
     * The size (bits) of the entry payload that relevant for this cluster access (will always be in multiples of 30b).
     */
    mdb_entry_size_e entry_size;
    /*
     * The offset (bits) inside the entry payload.
     */
    mdb_entry_offset_e entry_offset;
} mdb_cluster_direct_entry_access_info_t;

/*
 * A single direct entry can use up to two clusters, as the clusters data parts are not consecutive adding an entry
 * sometimes demands the split the entry into two parts and handle each part in a different cluster.
 * This structure is used to describe where each part of the entry goes to.
 */
typedef struct mdb_cluster_direct_entry_info
{
    /*
     * bitmap of the clusters (used by this DB) that are relevant for a single direct entry (only 2 bits).
     */
    mdb_cluster_use_bitmap_e active_bitmap;
    /*
     * Number of clusters used for the direct entry (1-2)
     */
    mdb_nof_clusters_to_use_e nof_used_clusters;
    /*
     * The format of each cluster
     */
    mdb_cluster_format_e clusters_format[MDB_MAX_CLUSTER_PER_DB];
    /*
     * access information for the relevant clusters
     */
    mdb_cluster_direct_entry_access_info_t access_info[MDB_MAX_CLUSTER_PER_DB];
} mdb_cluster_direct_entry_info_t;

/*
 * }
 */

/*
 * globals
 * {
 */

mdb_physical_tables_e mdb_direct_dbal_to_mdb[DBAL_NOF_PHYSICAL_TABLES];

/*
 * The offset of each of the 4 clusters inside a bucket row.
 */
const STATIC uint32 mdb_direct_cluster_offset_in_bucket[] = {
    0 * MDB_ENTRY_DATA_TOTAL_SIZE_IN_BITS,
    1 * MDB_ENTRY_DATA_TOTAL_SIZE_IN_BITS,
    2 * MDB_ENTRY_DATA_TOTAL_SIZE_IN_BITS,
    3 * MDB_ENTRY_DATA_TOTAL_SIZE_IN_BITS
};

/*
 * This array contain the number of entries that a single DB instance can contain, a single DB instance will be a
 * cluster in 120 mode or a pair of adjacent clusters in 240 mode.
 *
 * NOF_entries = ceiling( (MACRO_ROWS * row_size) / (entry size))
 *
 * The ceiling is because there are an additional 120bit to complete the last entry if there is
 * still some space at the end of a DB that is smaller than an entry.
 */
const uint32 mdb_nof_entries_in_db[MDB_NOF_DIRECT_ROW_WIDTH_TYPES][MDB_NOF_MACRO_TYPES][MDB_NOF_DIRECT_PAYLOAD_TYPES] = {
    {{
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_30B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_60B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_90B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_120B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_150B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_180B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS)},
     {
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_30B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_60B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_90B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_120B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_150B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_180B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS)}},
    {{
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_30B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_60B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_90B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_120B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_150B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_180B),
                  MDB_NOF_ROWS_IN_MACRO_A_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2)},
     {
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_30B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_60B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_90B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_120B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_150B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2),
      MDB_CEILING(MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_DIRECT_PAYLOAD_TYPE_180B),
                  MDB_NOF_ROWS_IN_MACRO_B_CLUSTER * MDB_NOF_CLUSTER_ROW_BITS * 2)}}
};

/*
 * This structure contain the access information for each payload size and remainder value combination.
 */
const STATIC
    mdb_cluster_direct_entry_info_t
    mdb_entry_info[MDB_NOF_DIRECT_ROW_WIDTH_TYPES][MDB_NOF_DIRECT_PAYLOAD_TYPES][MDB_NOF_DIRECT_REMAINDER_TYPES] = {
    {
     {
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 30 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 60 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_30b}}},
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 90 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 120 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_120b}}},
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_90b}}},
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_30b}}},
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 150 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_120b}}},
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 120 and entry size of 180 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     },
    {
     {
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 30 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 60 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_30b}}},
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 240 and entry size of 90 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_30b}}}
      },
     {
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_FIRST, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_SECOND, MDB_CLUSTER_USE_ONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 120 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      },
     {
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_120b}}},
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_90b}}},
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_30b}}},
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_120b}}},
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_30b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_90b}}},
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_90b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 240 and entry size of 150 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_90b, MDB_ENTRY_SIZE_30b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_30b}}}
      },
     {
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 0.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_120b}}},
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 30.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 60.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 90.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 120.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_120b}}},
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 150.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}},
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 180.
       */
      {MDB_CLUSTER_BITMAP_BOTH, MDB_CLUSTER_USE_TWO, {MDB_CLUSTER_FORMAT_NEXT_ROW, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_ONE, MDB_CLUSTER_DATA_OFFSET_60b, MDB_ENTRY_SIZE_60b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_0b, MDB_ENTRY_SIZE_120b, MDB_ENTRY_OFFSET_60b}}},
      /*
       * Row width of 240 and entry size of 180 and an entry remainder of 210.
       */
      {MDB_CLUSTER_BITMAP_NONE, MDB_CLUSTER_USE_NONE, {MDB_CLUSTER_FORMAT_NONE, MDB_CLUSTER_FORMAT_NONE},
       {{MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b},
        {MDB_CLUSTER_OFFSET_NONE, MDB_CLUSTER_DATA_OFFSET_NA, MDB_ENTRY_SIZE_0b, MDB_ENTRY_OFFSET_0b}}}
      }
     }
};

const mdb_direct_payload_sizes_types_e mdb_direct_table_to_payload_type[MDB_NOF_PHYSICAL_TABLES] = {
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_ISEM_1 */
    MDB_DIRECT_PAYLOAD_TYPE_60B,        /* MDB_PHYSICAL_TABLE_INLIF_1 */
    MDB_DIRECT_PAYLOAD_TYPE_90B,        /* MDB_PHYSICAL_TABLE_IVSI */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_ISEM_2 */
    MDB_DIRECT_PAYLOAD_TYPE_60B,        /* MDB_PHYSICAL_TABLE_INLIF_2 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_ISEM_3 */
    MDB_DIRECT_PAYLOAD_TYPE_60B,        /* MDB_PHYSICAL_TABLE_INLIF_3 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_LEM */
    MDB_DIRECT_PAYLOAD_TYPE_120B,       /* MDB_PHYSICAL_TABLE_ADS_1 */
    MDB_DIRECT_PAYLOAD_TYPE_120B,       /* MDB_PHYSICAL_TABLE_ADS_2 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_KAPS_1 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_KAPS_2 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_IOEM_0 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_IOEM_1 */
    MDB_DIRECT_PAYLOAD_TYPE_60B,        /* MDB_PHYSICAL_TABLE_MAP */
    MDB_DIRECT_PAYLOAD_TYPE_150B,       /* MDB_PHYSICAL_TABLE_FEC_1 */
    MDB_DIRECT_PAYLOAD_TYPE_150B,       /* MDB_PHYSICAL_TABLE_FEC_2 */
    MDB_DIRECT_PAYLOAD_TYPE_150B,       /* MDB_PHYSICAL_TABLE_FEC_3 */
    MDB_DIRECT_PAYLOAD_TYPE_120B,       /* MDB_PHYSICAL_TABLE_MC_ID */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_GLEM_0 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_GLEM_1 */
    MDB_DIRECT_PAYLOAD_TYPE_30B,        /* MDB_PHYSICAL_TABLE_EEDB_1 */
    MDB_DIRECT_PAYLOAD_TYPE_30B,        /* MDB_PHYSICAL_TABLE_EEDB_2 */
    MDB_DIRECT_PAYLOAD_TYPE_30B,        /* MDB_PHYSICAL_TABLE_EEDB_3 */
    MDB_DIRECT_PAYLOAD_TYPE_30B,        /* MDB_PHYSICAL_TABLE_EEDB_4 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_EOEM_0 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_EOEM_1 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_ESEM */
    MDB_DIRECT_PAYLOAD_TYPE_30B,        /* MDB_PHYSICAL_TABLE_EVSI */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_EXEM_1 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_EXEM_2 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_EXEM_3 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_EXEM_4 */
    MDB_NOF_DIRECT_PAYLOAD_TYPES,       /* MDB_PHYSICAL_TABLE_RMEP */
};

const mdb_direct_payload_sizes_types_e mdb_direct_table_to_row_width[MDB_NOF_PHYSICAL_TABLES] = {
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_ISEM_1 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_INLIF_1 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_IVSI */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_ISEM_2 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_INLIF_2 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_ISEM_3 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_INLIF_3 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_LEM */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_ADS_1 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_ADS_2 */
    MDB_NOF_CLUSTER_ROW_BITS * 4,       /* MDB_PHYSICAL_TABLE_KAPS_1 */
    MDB_NOF_CLUSTER_ROW_BITS * 4,       /* MDB_PHYSICAL_TABLE_KAPS_2 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_IOEM_0 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_IOEM_1 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_MAP */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_FEC_1 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_FEC_2 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_FEC_3 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_MC_ID */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_GLEM_0 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_GLEM_1 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EEDB_1 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EEDB_2 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EEDB_3 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EEDB_4 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EOEM_0 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EOEM_1 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_ESEM */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EVSI */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EXEM_1 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EXEM_2 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_EXEM_3 */
    MDB_NOF_CLUSTER_ROW_BITS * 2,       /* MDB_PHYSICAL_TABLE_EXEM_4 */
    MDB_NOF_CLUSTER_ROW_BITS,   /* MDB_PHYSICAL_TABLE_RMEP */
};

/*
 * }
 */
/**
 * \brief
 * This function returns the block ID, bucket index and the cluster index inside the specified memory.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] mem - The memory, in this case the MACRO entry bank.
 *  \param [in] cluster - The cluster index inside the memory (mem).
 *  \param [in] blk - The block to read from.
 *  \param [in] bucket_index - The bucket index inside the MACRO (0-1)
 *  \param [in] cluster_index - The cluster index inside the bucket (0-3).
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *  See blk above \n
 *  See bucket_index above \n
 *  See cluster_index above
 */
static shr_error_e
mdb_direct_table_calc_cluster_address(
    int unit,
    soc_mem_t mem,
    int cluster,
    int *blk,
    uint32 * bucket_index,
    uint32 * cluster_index)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Each bucket holds 4 clusters, bucket_index is 0 or 1
     */
    if (mem == DHC_MACRO_ENTRY_BANKm)
    {
        *blk = DHC_BLOCK(unit, (cluster / MDB_NOF_MACRO_A_CLUSTERS));
        *cluster_index = cluster % MDB_NOF_CLUSTER_IN_BUCKET;
        *bucket_index = (cluster % MDB_NOF_MACRO_A_CLUSTERS) / MDB_NOF_CLUSTER_IN_BUCKET;
    }
    else if ((mem == DDHB_MACRO_0_ENTRY_BANKm) || (mem == DDHB_MACRO_1_ENTRY_BANKm))
    {
        *blk = DDHB_BLOCK(unit, (cluster / MDB_NOF_MACRO_B_CLUSTERS));
        *cluster_index = cluster % MDB_NOF_CLUSTER_IN_BUCKET;
        *bucket_index = (cluster % MDB_NOF_MACRO_B_CLUSTERS) / MDB_NOF_CLUSTER_IN_BUCKET;
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Error. Mem %d is not supported.\n", mem);
    }

exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief
 * This function issues a read command and then reads the result into row_data
 *
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] mem - The memory, in this case the MACRO entry bank (DHC_MACRO_ENTRY_BANK,
 *                    DDHB_MACRO_0_ENTRY_BANK or DDHB_MACRO_1_ENTRY_BANK).
 *  \param [in] bucket_index - The index of the bucket to read from.
 *  \param [in] blk - The block ID (DHC or DDHB)
 *  \param [in] offset - The offset (BITS) inside to read from (the row number)
 *  \param [in] row_data - Contains the control information for the read.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 *  \par INDIRECT OUTPUT
 *   See row_data above.
 */
static shr_error_e
mdb_direct_table_read_entry_from_cluster(
    int unit,
    soc_mem_t mem,
    uint32 bucket_index,
    int blk,
    int offset,
    uint32 * row_data)
{
    bsl_severity_t severity;

    SHR_FUNC_INIT_VARS(unit);
    /*
     * Update the relevant bucket clusters control for the read process
     */
    SHR_IF_ERR_EXIT(soc_mem_array_write(unit, mem, bucket_index, blk, offset, row_data));

    SHR_IF_ERR_EXIT(soc_mem_array_read(unit, mem, bucket_index, blk, offset, row_data));

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        int data_traverse = 0;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_direct_table_read_entry_from_cluster: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "Mem: %s. bucket_index: %d. blk: %d. offset: %d.\n"),
                                     SOC_MEM_NAME(unit, mem), bucket_index, blk, offset));
        for (data_traverse = 0; data_traverse < MDB_ENTRY_BANK_SIZE_UINT32; data_traverse++)
        {
            if ((data_traverse % MDB_ENTRY_DATA_PER_BANK == 0) && (data_traverse != 0))
            {
                LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
            }
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "row_data[%d]: 0x%08X."), data_traverse, row_data[data_traverse]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\nmdb_direct_table_read_entry_from_cluster: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}
/**
 * \brief
 *  This function calculates from the entry_index: The MACRO type (DHA/DHB) The cluster in which the entry
 *  resides (indexation is within MACRO_0/1) the offset within that cluster
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] physical_table - MDB physical table type.
 *  \param [in] entry_index - The index of the entry.
 *  \param [in] entry_size - The size of the entry 30,60,90,120 or 150
 *  \param [in] row_width - The width of the row that this pysical table use (120 or 240)
 * \par INDIRECT INPUT
 *  mdb_nof_entries_in_db - Array that contains the number of entries each DB
 *     (cluster or a pair of cluster in 240 mode) can hold.
 *  mdb_db_infos - Provide cluster distribution information for each physical database type
 *  \param *mem - Memory - the MACRO entry bank
 *  \param *cluster - The cluster position inside the memory
 *  \param *row_in_cluster - relevant row in the cluster.
 *  \param *entry_index_inside_the_cluster - The index of the entry relative to the cluster that stores this entry.
 * \par DIRECT OUTPUT:
 *  Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *  See *mem above \n
 *  See *cluster above \n
 *  See *row_in_cluster above \n
 *  See *entry_index_inside_the_cluster above
 */
static shr_error_e
mdb_direct_table_calc_index_address(
    int unit,
    mdb_physical_tables_e physical_table,
    uint32 entry_index,
    int entry_size,
    uint32 row_width,
    soc_mem_t * mem,
    int *cluster,
    int *row_in_cluster,
    uint32 * entry_index_inside_the_cluster)
{
    uint32 cluster_index;
    uint32 macro_index;
    uint32 macro_a_nof_entries;
    uint32 macro_b_nof_entries;
    mdb_direct_payload_sizes_types_e payload_type;
    /*
     * Holds 2 if the clusters are paired or 1 otherwise
     */
    uint32 row_factor;
    SHR_FUNC_INIT_VARS(unit);

    row_factor = row_width / MDB_NOF_CLUSTER_ROW_BITS;

    payload_type = MDB_PAYLOAD_SIZE_TO_PAYLOAD_SIZE_TYPE(entry_size);
    macro_a_nof_entries =
        (mdb_db_infos[physical_table].macro_a_clusters_num / row_factor) * mdb_nof_entries_in_db[row_factor -
                                                                                                 1][MDB_MACRO_A]
        [payload_type];
    macro_b_nof_entries =
        (mdb_db_infos[physical_table].macro_b_clusters_num / row_factor) * mdb_nof_entries_in_db[row_factor -
                                                                                                 1][MDB_MACRO_B]
        [payload_type];

    if (entry_index < macro_a_nof_entries)
    {
        /*
         * Write to MACRO_A
         */
        *mem = DHC_MACRO_ENTRY_BANKm;

        macro_index = entry_index;
        cluster_index = macro_index / mdb_nof_entries_in_db[row_factor - 1][MDB_MACRO_A][payload_type];
        *entry_index_inside_the_cluster =
            macro_index - (cluster_index * mdb_nof_entries_in_db[row_factor - 1][MDB_MACRO_A][payload_type]);
        *row_in_cluster = ((*entry_index_inside_the_cluster) * entry_size) / row_width;
        *cluster = mdb_db_infos[physical_table].macro_a_clusters_addr[cluster_index];
    }
    else if (entry_index < (macro_a_nof_entries + macro_b_nof_entries))
    {
        /*
         * Write to MACRO_B
         */

        /*
         * The index minus all of the indices stored in MACRO A
         */
        macro_index = (entry_index - (macro_a_nof_entries));
        cluster_index = macro_index / mdb_nof_entries_in_db[row_factor - 1][MDB_MACRO_B][payload_type];
        *entry_index_inside_the_cluster =
            macro_index - (cluster_index * mdb_nof_entries_in_db[row_factor - 1][MDB_MACRO_B][payload_type]);
        *row_in_cluster = ((*entry_index_inside_the_cluster) * entry_size) / row_width;
        /*
         * Determine whether this cluster is located in MACRO_0 or MACRO_1
         */
        if (mdb_db_infos[physical_table].macro_b_clusters_addr[cluster_index] <
            (MDB_NOF_MACRO_B * MDB_NOF_MACRO_B_CLUSTERS / MDB_NOF_MACRO_B_NOF_MEMS))
        {
            *mem = DDHB_MACRO_0_ENTRY_BANKm;
            *cluster = mdb_db_infos[physical_table].macro_b_clusters_addr[cluster_index];
        }
        else
        {
            *mem = DDHB_MACRO_1_ENTRY_BANKm;
            *cluster =
                mdb_db_infos[physical_table].macro_b_clusters_addr[cluster_index] -
                (MDB_NOF_MACRO_B * MDB_NOF_MACRO_B_CLUSTERS / MDB_NOF_MACRO_B_NOF_MEMS);
        }
    }
    else
    {
        /*
         * Return error
         */
        SHR_ERR_EXIT(_SHR_E_RESOURCE, "Error. Index %d out of range in physical table %d.\n", entry_index,
                     physical_table);
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_is_payload_masked(
    int unit,
    dbal_physical_entry_t * entry,
    int *is_masked)
{

    int valid_payload_bits;

    SHR_FUNC_INIT_VARS(unit);

    shr_bitop_range_count(entry->p_mask, 0 /* first */ , entry->payload_size, &valid_payload_bits);

    if (valid_payload_bits == entry->payload_size)
    {
        *is_masked = 0;
    }
    else
    {
        *is_masked = 1;
    }

    SHR_FUNC_EXIT;
}

/**
 * \brief
 * This function adds commands (Read, Write etc...) and formats to each cluster in a bucket.
 * \par DIRECT INPUT
 *  \param [in] unit - The unit number.
 *  \param [in] command - The command to update (Read, Write etc...).
 *  \param [in] clusters_in_use_bitmap - A bitmap, one bit per cluster. Only 4 bits are relevant.
 *      Bitmap indicates which of the four cluster is actually in use. Only 'control info' structures of active
 *      clusters are updated.
 *  \param [in] clusters_format - An array of formats to update the clusters control.
 *  \param row_data - The bucket row, only the control information is modified.
 * \par INDIRECT OUTPUT
 *  See clusters_format above
 * \par DIRECT OUTPUT:
 *    Non-zero in case of an error
 * \par INDIRECT OUTPUT
 *  See row_data above
 */
static shr_error_e
mdb_direct_update_bucket_row_control(
    int unit,
    uint32 command,
    uint32 clusters_in_use_bitmap,
    const uint32 * clusters_format,
    uint32 * row_data)
{
    uint32 control;
    int cluster_index;
    int format_index = 0;
    mdb_cluster_instructions_e updated_command;

    SHR_FUNC_INIT_VARS(unit);

    for (cluster_index = 0; cluster_index < MDB_NOF_CLUSTER_IN_BUCKET; cluster_index++)
    {
        /*
         * Change the command to the next row if necessary
         */
        if (clusters_format[format_index++] == MDB_CLUSTER_FORMAT_NEXT_ROW)
        {
            if (command == MDB_CLUSTER_READ_INST)
            {
                updated_command = MDB_CLUSTER_READ_NEXT_ROW_INST;
            }
            else if (command == MDB_CLUSTER_WRITE_INST)
            {
                updated_command = MDB_CLUSTER_WRITE_NEXT_ROW_INST;
            }
            else
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "Error. Unexpected command: %d.\n", command);
            }
        }
        else
        {
            updated_command = command;
        }
        control = ((clusters_in_use_bitmap & (1 << cluster_index)) == 0) ? MDB_ENTRY_EMPTY_CONTROL :
            MDB_ENTRY_SET_CONTROL_BITS(updated_command, 0);
        SHR_BITCOPY_RANGE(row_data, (cluster_index * MDB_ENTRY_DATA_TOTAL_SIZE_IN_BITS), &control, 0,
                          MDB_ENTRY_DATA_OFFSET);
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_iterator_init(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator)
{
    int basic_size;
    dbal_physical_table_def_t *dbal_physical_table;
    SHR_FUNC_INIT_VARS(unit);

    physical_entry_iterator->mdb_entry_index = 0;
    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, dbal_physical_table_id, &dbal_physical_table));
    SHR_IF_ERR_EXIT(mdb_get_capacity(unit, dbal_physical_table, &physical_entry_iterator->mdb_entry_capacity));
    SHR_IF_ERR_EXIT(mdb_direct_table_get_basic_size(unit, dbal_physical_table_id, &basic_size));
    physical_entry_iterator->payload_basic_size = (uint32) basic_size;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_iterator_get_next(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator,
    dbal_physical_entry_t * entry,
    uint8 * is_end)
{
    SHR_FUNC_INIT_VARS(unit);

    sal_memset(entry, 0x0, sizeof(*entry));

    entry->payload_size = physical_entry_iterator->payload_basic_size;

    /*
     * Only return non-default entries
     */
    while (physical_entry_iterator->mdb_entry_index < physical_entry_iterator->mdb_entry_capacity
           && !utilex_bitstream_have_one(entry->payload, BITS2WORDS(entry->payload_size)))
    {
        entry->key[0] = physical_entry_iterator->mdb_entry_index;
        SHR_IF_ERR_EXIT(mdb_direct_table_entry_get(unit, dbal_physical_table_id, app_id, entry));
        physical_entry_iterator->mdb_entry_index++;
    }

    /*
     * If the capacity has been reached and the last entry retrieved is zero
     */
    if (physical_entry_iterator->mdb_entry_index >= physical_entry_iterator->mdb_entry_capacity
        && !utilex_bitstream_have_one(entry->payload, BITS2WORDS(entry->payload_size)))
    {
        *is_end = TRUE;
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_iterator_deinit(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_entry_add(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    mdb_physical_tables_e mdb_physical_table = mdb_direct_dbal_to_mdb[dbal_physical_table_id];
    const mdb_cluster_direct_entry_info_t *ci;
    mdb_direct_payload_sizes_types_e payload_type;
    mdb_direct_remainder_types_e remainder_type;
    soc_mem_t mem;
    uint32 row_data[MDB_ENTRY_BANK_SIZE_IN_BITS / SAL_UINT32_NOF_BITS];
    /** Use to save the original data of an entry that needs to be partially updated **/
    uint32 orig_entry_masked[MDB_MAX_DIRECT_PAYLOAD_SIZE_32];
    /** In case the payload is masked, the cluster_mask will be used to hold the mask part of a single cluster  **/
    uint32 cluster_mask[MDB_MAX_DIRECT_PAYLOAD_SIZE_32];
    /** The part of the payload that relevant for the currently update cluster in the bucket **/
    uint32 cluster_payload[MDB_MAX_DIRECT_PAYLOAD_SIZE_32];
    /** The new entered payload padded in zeroes at the end if the payload size is smaller than the entry size **/
    uint32 entry_payload[MDB_MAX_DIRECT_PAYLOAD_SIZE_32];
    uint32 entry_index = entry->key[0];
    uint32 entry_index_inside_the_cluster;
    uint32 bucket_index;
    uint32 cluster_index_in_bucket;
    uint32 row_width;
    uint32 index;
    uint32 row_offset;
    bsl_severity_t severity;
    /*
     * The table entry size used for the table indexation
     */
    uint32 table_entry_size;
    /*
     * The entry size used for the table insertion
     */
    uint32 entry_size_type = MDB_CEILING(MDB_DIRECT_BASIC_ENTRY_SIZE, entry->payload_size) - 1;
    uint32 cluster_index;
    int is_masked;
    int cluster_pos_in_mem;
    int row_in_cluster;
    int blk;

    SHR_FUNC_INIT_VARS(unit);

    if (mdb_physical_table == MDB_NOF_PHYSICAL_TABLES)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM,
                     "Error. DBAL physical table %d is not associated with an MDB direct table.\n",
                     dbal_physical_table_id);
    }

    payload_type = mdb_direct_table_to_payload_type[mdb_physical_table];

    table_entry_size = MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(payload_type);

    row_width = mdb_direct_table_to_row_width[mdb_physical_table];

    /** Get entry position info  */
    SHR_IF_ERR_EXIT(mdb_direct_table_calc_index_address
                    (unit, mdb_physical_table, entry_index, table_entry_size, row_width,
                     &mem, &cluster_pos_in_mem, &row_in_cluster, &entry_index_inside_the_cluster));

    SHR_IF_ERR_EXIT(mdb_direct_table_calc_cluster_address
                    (unit, mem, cluster_pos_in_mem, &blk, &bucket_index, &cluster_index_in_bucket));

    SHR_IF_ERR_EXIT(mdb_direct_is_payload_masked(unit, entry, &is_masked));

    remainder_type = MDB_GET_REMAINDER_TYPE(entry_index_inside_the_cluster, table_entry_size, row_width);

    /** Get the cluster access information for adding the entry */
    if (entry_size_type >= MDB_NOF_DIRECT_PAYLOAD_TYPES)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM,
                     "Error. Direct tables currently does not support entries larger than %d.\n",
                     MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(MDB_NOF_DIRECT_PAYLOAD_TYPES - 1));
    }
    ci = &mdb_entry_info[row_width / MDB_NOF_CLUSTER_ROW_BITS - 1][entry_size_type][remainder_type];

    sal_memset(row_data, 0x0, (MDB_ENTRY_BANK_SIZE_IN_BITS / SAL_UINT32_NOF_BITS) * sizeof(row_data[0]));

    /*
     * Unless the update is a full (not masked) cluster row, a read modify write is needed
     * This is an optimization for a 120 mode with a 120b size row
     */
    if ((ci->nof_used_clusters > MDB_CLUSTER_USE_ONE) || (table_entry_size != MDB_NOF_CLUSTER_ROW_BITS) || is_masked)
    {
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_direct_table_entry_add: Performing read-modify-write.\n")));
        /*
         * Update control bits in the row
         */
        SHR_IF_ERR_EXIT(mdb_direct_update_bucket_row_control
                        (unit, MDB_CLUSTER_READ_INST, ci->active_bitmap << cluster_index_in_bucket, ci->clusters_format,
                         row_data));

        /*
         * Read the entry from the HW
         */
        SHR_IF_ERR_EXIT(mdb_direct_table_read_entry_from_cluster
                        (unit, mem, bucket_index, blk, row_in_cluster, row_data));
    }

    sal_memset(entry_payload, 0x0, MDB_MAX_DIRECT_PAYLOAD_SIZE_32 * sizeof(entry_payload[0]));
    SHR_BITCOPY_RANGE(entry_payload, 0 /* src_offset */ , entry->payload, 0, entry->payload_size);

    for (index = 0; index < ci->nof_used_clusters; index++)
    {
        cluster_index = cluster_index_in_bucket + ci->access_info[index].cluster_offset;

        row_offset = mdb_direct_cluster_offset_in_bucket[cluster_index] + ci->access_info[index].cluster_row_offset;
        /*
         * Update the cluster current payload information and the mask part
         */
        SHR_BITCOPY_RANGE(cluster_payload, 0 /* src_offset */ , entry_payload, ci->access_info[index].entry_offset,
                          ci->access_info[index].entry_size);

        if (is_masked != 0)
        {
            uint32 reverse_masked[MDB_MAX_DIRECT_PAYLOAD_SIZE_32];

            /*
             * if the payload is partially masked we will need to read the entry, zero the valid payload bits and use OR
             * to copy over only the valid bits
             */
            SHR_BITCOPY_RANGE(orig_entry_masked, 0 /* src_offset */ , row_data, row_offset,
                              ci->access_info[index].entry_size);
            SHR_BITCOPY_RANGE(cluster_mask, 0 /* src_offset */ , entry->p_mask, ci->access_info[index].entry_offset,
                              ci->access_info[index].entry_size);

            /*
             * zero the valid payload bits
             */
            SHR_BITREMOVE_RANGE(orig_entry_masked, cluster_mask, 0, ci->access_info[index].entry_size,
                                orig_entry_masked);
            /*
             * Clear all the excess bits from the payload
             */
            SHR_BITNEGATE_RANGE(cluster_mask, 0, ci->access_info[index].entry_size, reverse_masked);
            SHR_BITREMOVE_RANGE(cluster_payload, reverse_masked, 0, ci->access_info[index].entry_size, cluster_payload);

            /*
             * Use OR to insert the masked payload
             */
            SHR_BITOR_RANGE(orig_entry_masked, cluster_payload, 0, ci->access_info[index].entry_size, cluster_payload);
        }

        SHR_BITCOPY_RANGE(row_data, row_offset, cluster_payload, 0 /* src_offset */ ,
                          ci->access_info[index].entry_size);
    }

    /*
     * Update control bits in the row
     */
    SHR_IF_ERR_EXIT(mdb_direct_update_bucket_row_control
                    (unit, MDB_CLUSTER_WRITE_INST, ci->active_bitmap << cluster_index_in_bucket, ci->clusters_format,
                     row_data));
    SHR_IF_ERR_EXIT(soc_mem_array_write(unit, mem, bucket_index, blk, row_in_cluster, row_data));

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_direct_table_entry_add: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "entry.key: %d. entry.payload_size: %d, physical_table: %s.\n"),
                                     entry->key[0], entry->payload_size, dbal_physical_table_to_string(unit,
                                                                                                       dbal_physical_table_id)));
        for (row_offset = 0; row_offset < BITS2WORDS(entry->payload_size); row_offset++)
        {
            uint32 print_index = BITS2WORDS(entry->payload_size) - 1 - row_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry.payload[%d]: 0x%08x.\n"), print_index, entry->payload[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U(unit, "Mem: %s. bucket_index: %d. blk: %d. row_in_cluster: %d. data_index: %d.\n"),
                     SOC_MEM_NAME(unit, mem), bucket_index, blk, row_in_cluster, cluster_index_in_bucket));
        for (index = 0; index < MDB_ENTRY_BANK_SIZE_IN_BITS / SAL_UINT32_NOF_BITS; index++)
        {
            if (index % MDB_ENTRY_DATA_PER_BANK == 0)
            {
                LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
            }
            LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "row_data[%d]: 0x%08x. "), index, row_data[index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));

        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_direct_table_entry_add: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_entry_get(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    mdb_physical_tables_e mdb_physical_table = mdb_direct_dbal_to_mdb[dbal_physical_table_id];
    const mdb_cluster_direct_entry_info_t *ci;
    mdb_direct_payload_sizes_types_e payload_type;
    mdb_direct_remainder_types_e remainder_type;
    soc_mem_t mem;
    uint32 row_data[MDB_ENTRY_BANK_SIZE_IN_BITS / SAL_UINT32_NOF_BITS];
    uint32 entry_index = entry->key[0];
    uint32 bucket_index;
    uint32 row_width;
    uint32 entry_index_inside_the_cluster;
    uint32 cluster_index_in_bucket;
    uint32 index;
    uint32 row_offset;
    /*
     * The table entry size used for the table indexation
     */
    uint32 table_entry_size;
    /*
     * The entry size used for the table insertion
     */
    uint32 entry_size = MDB_CEILING(MDB_DIRECT_BASIC_ENTRY_SIZE, entry->payload_size) - 1;
    uint32 cluster_index;
    int blk;
    int cluster_pos_in_mem;
    int row_in_cluster;
    bsl_severity_t severity;

    SHR_FUNC_INIT_VARS(unit);

    if (mdb_direct_dbal_to_mdb[dbal_physical_table_id] == MDB_NOF_PHYSICAL_TABLES)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM,
                     "Error. DBAL physical table %d is not associated with an MDB direct table.\n",
                     dbal_physical_table_id);
    }

    payload_type = mdb_direct_table_to_payload_type[mdb_physical_table];

    table_entry_size = MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(payload_type);

    row_width = mdb_direct_table_to_row_width[mdb_physical_table];

    SHR_IF_ERR_EXIT(mdb_direct_table_calc_index_address
                    (unit, mdb_physical_table, entry_index, table_entry_size, row_width,
                     &mem, &cluster_pos_in_mem, &row_in_cluster, &entry_index_inside_the_cluster));

    SHR_IF_ERR_EXIT(mdb_direct_table_calc_cluster_address
                    (unit, mem, cluster_pos_in_mem, &blk, &bucket_index, &cluster_index_in_bucket));

    remainder_type = MDB_GET_REMAINDER_TYPE(entry_index_inside_the_cluster, table_entry_size, row_width);

    ci = &mdb_entry_info[row_width / MDB_NOF_CLUSTER_ROW_BITS - 1][entry_size][remainder_type];

    sal_memset(row_data, 0x0, (MDB_ENTRY_BANK_SIZE_IN_BITS / SAL_UINT32_NOF_BITS) * sizeof(row_data[0]));
    /*
     * Update control bits in the row
     */
    SHR_IF_ERR_EXIT(mdb_direct_update_bucket_row_control
                    (unit, MDB_CLUSTER_READ_INST, ci->active_bitmap << cluster_index_in_bucket, ci->clusters_format,
                     row_data));

    SHR_IF_ERR_EXIT(mdb_direct_table_read_entry_from_cluster(unit, mem, bucket_index, blk, row_in_cluster, row_data));

    for (index = 0; index < ci->nof_used_clusters; index++)
    {
        cluster_index = cluster_index_in_bucket + ci->access_info[index].cluster_offset;
        row_offset = mdb_direct_cluster_offset_in_bucket[cluster_index] + ci->access_info[index].cluster_row_offset;
        SHR_BITCOPY_RANGE(entry->payload, ci->access_info[index].entry_offset, row_data, row_offset,
                          ci->access_info[index].entry_size);
    }

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_direct_table_entry_get: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "entry.key: %d. entry.payload_size: %d, physical_table: %s.\n"),
                                     entry->key[0], entry->payload_size, dbal_physical_table_to_string(unit,
                                                                                                       dbal_physical_table_id)));
        for (index = 0; index < BITS2WORDS(entry->payload_size); index++)
        {
            uint32 print_index = BITS2WORDS(entry->payload_size) - 1 - index;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry.payload[%d]: 0x%08X.\n"), print_index, entry->payload[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_direct_table_entry_get: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_entry_delete(
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    dbal_physical_entry_t temp_entry;

    SHR_FUNC_INIT_VARS(unit);

    sal_memcpy(&temp_entry, entry, sizeof(dbal_physical_entry_t));

    sal_memset(temp_entry.payload, 0x0, sizeof(temp_entry.payload));
    sal_memset(temp_entry.p_mask, 0xFF, sizeof(temp_entry.p_mask));

    SHR_IF_ERR_EXIT(mdb_direct_table_entry_add(unit, physical_table, app_id, &temp_entry));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_test(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    mdb_test_mode_e mode)
{
    dbal_physical_table_def_t *dbal_physical_table;
    dbal_physical_entry_t entry;
    uint32 row_data_write[MDB_MAX_DIRECT_PAYLOAD_SIZE_32];
    uint32 row_data_read[MDB_MAX_DIRECT_PAYLOAD_SIZE_32];
    int entry_counter;
    int uint32_counter;
    mdb_physical_tables_e mdb_physical_table = mdb_direct_dbal_to_mdb[dbal_physical_table_id];
    mdb_direct_payload_sizes_types_e payload_type;
    uint32 entry_size;

    SHR_FUNC_INIT_VARS(unit);

    sal_srand(55555);

    if (mdb_physical_table == MDB_NOF_PHYSICAL_TABLES)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM,
                     "Error. DBAL physical table %d is not associated with an MDB direct table.\n",
                     dbal_physical_table_id);
    }

    sal_memset(row_data_write, 0x0, (MDB_MAX_DIRECT_PAYLOAD_SIZE_32) * sizeof(row_data_write[0]));
    sal_memset(row_data_read, 0x0, (MDB_MAX_DIRECT_PAYLOAD_SIZE_32) * sizeof(row_data_read[0]));

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, dbal_physical_table_id, &dbal_physical_table));

    payload_type = mdb_direct_table_to_payload_type[mdb_physical_table];

    entry_size = MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(payload_type);

    /*
     * Iterate on all possible indices
     */
    entry_counter = 0;
    while (entry_counter < dbal_physical_table->max_capacity)
    {
        /*
         * Fill the data array with random content
         */
        for (uint32_counter = 0; uint32_counter < (MDB_CEILING(SAL_UINT32_NOF_BITS, entry_size)); uint32_counter++)
        {
            row_data_write[uint32_counter] = sal_rand();
        }

        /*
         * Zero redundant bits
         */
        if (entry_size != MDB_MAX_DIRECT_PAYLOAD_SIZE_32)
        {
            SHR_IF_ERR_EXIT(utilex_bitstream_reset_bit_range
                            (row_data_write, entry_size, (MDB_CEILING(SAL_UINT32_NOF_BITS, entry_size)) - 1));
        }

        entry.key[0] = entry_counter;
        entry.payload_size = entry_size;
        SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(entry.p_mask, 0, entry.payload_size));
        sal_memcpy(entry.payload, row_data_write, (MDB_MAX_DIRECT_PAYLOAD_SIZE_32) * sizeof(entry.payload[0]));

        SHR_IF_ERR_EXIT(mdb_direct_table_entry_add(unit, dbal_physical_table_id, 0 /* app_id */ , &entry));

        sal_memset(entry.payload, 0x0, (MDB_MAX_DIRECT_PAYLOAD_SIZE_32) * sizeof(entry.payload[0]));

        SHR_IF_ERR_EXIT(mdb_direct_table_entry_get(unit, dbal_physical_table_id, 0 /* app_id */ , &entry));

        /*
         * Save the read data
         */
        sal_memcpy(row_data_read, entry.payload, (MDB_MAX_DIRECT_PAYLOAD_SIZE_32) * sizeof(row_data_read[0]));

        /*
         * Xor between the read data and the written data, expect the output to be all zeros
         */
        SHR_IF_ERR_EXIT(utilex_bitstream_xor(entry.payload, row_data_write, MDB_MAX_DIRECT_PAYLOAD_SIZE_32));

        if (utilex_bitstream_have_one_in_range(entry.payload, 0 /* start_place */ , entry.payload_size - 1))
        {
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Written data:\n 0x")));
            for (uint32_counter = MDB_MAX_DIRECT_PAYLOAD_SIZE_32 - 1; uint32_counter >= 0; uint32_counter--)
            {
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%08X"), row_data_write[uint32_counter]));
            }
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));

            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Read data:\n 0x")));
            for (uint32_counter = MDB_MAX_DIRECT_PAYLOAD_SIZE_32 - 1; uint32_counter >= 0; uint32_counter--)
            {
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%08X"), row_data_read[uint32_counter]));
            }
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Payload size: %d\n"), entry.payload_size));
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "entry_counter: %d\n"), entry_counter));

            SHR_ERR_EXIT(_SHR_E_FAIL, "Test failed, read data is not equal to written data.\n");
        }

        /*
         * Delete the entry
         */
        SHR_IF_ERR_EXIT(mdb_direct_table_entry_delete(unit, dbal_physical_table_id, 0 /* app_id */ , &entry));

        SHR_IF_ERR_EXIT(mdb_direct_table_entry_get(unit, dbal_physical_table_id, 0 /* app_id */ , &entry));

        if (utilex_bitstream_have_one_in_range(entry.payload, 0 /* start_place */ , entry.payload_size - 1))
        {
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Payload size: %d\n"), entry.payload_size));
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "entry_counter: %d\n"), entry_counter));

            SHR_ERR_EXIT(_SHR_E_FAIL, "Test failed, data different from 0 after delete.\n");
        }

        if (mode == MDB_TEST_FULL)
        {
            entry_counter++;
        }
        else
        {
            entry_counter += sal_rand() % MDB_TEST_BRIEF_FACTOR;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_direct_table_get_basic_size(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    int *basic_size)
{

    SHR_FUNC_INIT_VARS(unit);

    if (mdb_get_db_type(dbal_physical_table_id) != MDB_DB_DIRECT)
    {
        SHR_ERR_EXIT(_SHR_E_BADID, "%s is not associated with an MDB direct table.\n",
                     dbal_physical_table_to_string(unit, dbal_physical_table_id));
    }

    *basic_size =
        MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(mdb_direct_table_to_payload_type
                                              [mdb_direct_dbal_to_mdb[dbal_physical_table_id]]);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_get_db_access_info(
    int unit,
    mdb_physical_tables_e mdb_physical_table,
    mdb_dbal_physical_table_access_info_t * table_access_info)
{
    int cluster_index = 0;
    int i;
    int blk;
    uint32 bucket_index;
    uint32 cluster_array_index;
    uint8 cluster_address;
    soc_mem_t mem;

    SHR_FUNC_INIT_VARS(unit);

    table_access_info->direct_access_resolution =
        MDB_PAYLOAD_SIZE_TYPE_TO_PAYLOAD_SIZE(mdb_direct_table_to_payload_type[mdb_physical_table]);
    table_access_info->row_width = mdb_direct_table_to_row_width[mdb_physical_table];

    table_access_info->nof_clusters =
        mdb_db_infos[mdb_physical_table].macro_a_clusters_num + mdb_db_infos[mdb_physical_table].macro_b_clusters_num;

    for (i = 0; i < table_access_info->nof_clusters; i++)
    {
        if (i < mdb_db_infos[mdb_physical_table].macro_a_clusters_num)
        {
            table_access_info->cluster_info_array[cluster_index].cluster_address =
                mdb_db_infos[mdb_physical_table].macro_a_clusters_addr[i];
            cluster_address = mdb_db_infos[mdb_physical_table].macro_a_clusters_addr[i];
            mem = DHC_MACRO_ENTRY_BANKm;
        }
        else
        {
            table_access_info->cluster_info_array[cluster_index].cluster_address =
                mdb_db_infos[mdb_physical_table].macro_b_clusters_addr[i];
            /*
             * Determine whether this cluster is located in MACRO_0 or MACRO_1
             */
            if (mdb_db_infos[mdb_physical_table].macro_b_clusters_addr[i] <
                (MDB_NOF_MACRO_B * MDB_NOF_MACRO_B_CLUSTERS / MDB_NOF_MACRO_B_NOF_MEMS))
            {
                mem = DDHB_MACRO_0_ENTRY_BANKm;
                cluster_address = mdb_db_infos[mdb_physical_table].macro_b_clusters_addr[i];
            }
            else
            {
                mem = DDHB_MACRO_1_ENTRY_BANKm;
                cluster_address =
                    mdb_db_infos[mdb_physical_table].macro_b_clusters_addr[i] -
                    (MDB_NOF_MACRO_B * MDB_NOF_MACRO_B_CLUSTERS / MDB_NOF_MACRO_B_NOF_MEMS);
            }
        }

        SHR_IF_ERR_EXIT(mdb_direct_table_calc_cluster_address(unit, mem, cluster_address,
                                                              &blk, &bucket_index, &cluster_array_index));

        table_access_info->cluster_info_array[cluster_index].macro_mem = mem;
        if (mem == DHC_MACRO_ENTRY_BANKm)
        {
            table_access_info->cluster_info_array[cluster_index].blk = DHC_BLOCK(unit, 0);
            table_access_info->cluster_info_array[cluster_index].blk_id = blk - DHC_BLOCK(unit, 0);
        }
        else
        {
            table_access_info->cluster_info_array[cluster_index].blk = DDHB_BLOCK(unit, 0);
            table_access_info->cluster_info_array[cluster_index].blk_id = blk - DDHB_BLOCK(unit, 0);
        }

        table_access_info->cluster_info_array[cluster_index].bucket_index = bucket_index;
        table_access_info->cluster_info_array[cluster_index].bucket_offset = cluster_array_index;

        cluster_index++;
    }

exit:
    SHR_FUNC_EXIT;
}
