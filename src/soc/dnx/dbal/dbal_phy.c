/*
 * $Id: dpp_dbal.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DBALACCESSDNX
#include <shared/bsl.h>
#include "dbal_internal.h"
#include <soc/dnx/mdb.h>
#include <shared/utilex/utilex_bitstream.h>

static dbal_physical_mngr_info_t physical_table_mngr = {
    {
     {{0}},
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE TCAM",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_TCAM,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ NULL,
      /*
       * .entry_get               =
       */ NULL,
      /*
       * .entry_delete            =
       */ NULL,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE LPM PRIVATE",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_LPM_PRIVATE,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      /*
       * .entry_set               =
       */ mdb_lpm_entry_add,
      /*
       * .entry_get               =
       */ mdb_lpm_entry_get,
      /*
       * .entry_delete            =
       */ mdb_lpm_entry_delete,
#else
      /*
       * .entry_set               =
       */ NULL,
      /*
       * .entry_get               =
       */ NULL,
      /*
       * .entry_delete            =
       */ NULL,
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      /*
       * .iterator_init           =
       */ mdb_lpm_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_lpm_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_lpm_iterator_deinit
#else
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE LPM PUBLIC",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_LPM_PUBLIC,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      /*
       * .entry_set               =
       */ mdb_lpm_entry_add,
      /*
       * .entry_get               =
       */ mdb_lpm_entry_get,
      /*
       * .entry_delete            =
       */ mdb_lpm_entry_delete,
#else
      /*
       * .entry_set               =
       */ NULL,
      /*
       * .entry_get               =
       */ NULL,
      /*
       * .entry_delete            =
       */ NULL,
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      /*
       * .iterator_init           =
       */ mdb_lpm_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_lpm_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_lpm_iterator_deinit
#else
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE ISEM 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_ISEM_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE INLIF 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_INLIF_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations *******************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information *******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE IVSI",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_IVSI,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information *******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE ISEM 2",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_ISEM_2,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE INLIF 2",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_INLIF_2,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE ISEM 3",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_ISEM_3,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE INLIF 3",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_INLIF_3,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE LEM",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_LEM,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE IOEM 0",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_IOEM_0,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE IOEM 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_IOEM_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE MAP",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_MAP,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE FEC 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_FEC_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE FEC 2",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_FEC_2,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE FEC 3",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_FEC_3,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE MC ID",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_MC_ID,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE GLEM 0",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_GLEM_0,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE GLEM 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_GLEM_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EEDB 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EEDB_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EEDB 2",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EEDB_2,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EEDB 3",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EEDB_3,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EEDB 4",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EEDB_4,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EOEM 0",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EOEM_0,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EOEM 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EOEM_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE ESEM",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_ESEM,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EVSI",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EVSI,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_direct_table_entry_add,
      /*
       * .entry_get               =
       */ mdb_direct_table_entry_get,
      /*
       * .entry_delete            =
       */ mdb_direct_table_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ mdb_direct_table_iterator_init,
      /*
       * .iterator_get_next       =
       */ mdb_direct_table_iterator_get_next,
      /*
       * .iterator_deinit         =
       */ mdb_direct_table_iterator_deinit
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EXEM 1",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EXEM_1,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EXEM 2",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EXEM_2,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EXEM 3",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EXEM_3,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE EXEM 4",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_EXEM_4,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      },
     {
            /**************************************** Database information ******************************************/
      /*
       * .physical_name           =
       */ "MDB TABLE RMEP",
      /*
       * .physical_db_type        =
       */ DBAL_PHYSICAL_TABLE_RMEP,
      /*
       * .max_capacity            =
       */ 0,
      /*
       * .nof_entries             =
       */ 0,
            /**************************************** Database operations ********************************************/
      /*
       * .entry_set               =
       */ mdb_em_entry_add,
      /*
       * .entry_get               =
       */ mdb_em_entry_get,
      /*
       * .entry_delete            =
       */ mdb_em_entry_delete,
      /*
       * .table_clear             =
       */ NULL,
      /*
       * .table_default_values_set=
       */ NULL,
      /*
       * .table_init              =
       */ NULL,
      /*
       * .table_deinit            =
       */ NULL,
      /*
       * .iterator_init           =
       */ NULL,
      /*
       * .iterator_get_next       =
       */ NULL,
      /*
       * .iterator_deinit         =
       */ NULL
            /*********************************************************************************************************/
      }
     }
};

shr_error_e
dbal_physical_table_init(
    int unit)
{
    int ii;
    dbal_physical_table_def_t *PhysicalTable;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(mdb_init(unit));

    for (ii = 0; ii < DBAL_NOF_PHYSICAL_TABLES; ii++)
    {

        SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, ii, &PhysicalTable));

        if (physical_table_mngr.physical_tables[ii].physical_db_type != ii)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "ERROR dbal_physical_table_init %d\n", ii);
        }

        SHR_IF_ERR_EXIT(mdb_get_capacity(unit, PhysicalTable, &physical_table_mngr.physical_tables[ii].max_capacity));

        if (physical_table_mngr.physical_tables[ii].table_init)
        {
            SHR_IF_ERR_EXIT(physical_table_mngr.physical_tables[ii].table_init(unit));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_physical_table_deinit(
    int unit)
{
    int ii;

    SHR_FUNC_INIT_VARS(unit);

    for (ii = 0; ii < DBAL_NOF_PHYSICAL_TABLES; ii++)
    {
        if (physical_table_mngr.physical_tables[ii].table_deinit)
        {
            SHR_IF_ERR_EXIT(physical_table_mngr.physical_tables[ii].table_deinit(unit));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_physical_table_get(
    int unit,
    dbal_physical_tables_e physical_table_id,
    dbal_physical_table_def_t ** physical_table)
{
    SHR_FUNC_INIT_VARS(unit);

    if (physical_table_id > DBAL_NOF_PHYSICAL_TABLES)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "ERROR dbal_physical_table_get %d\n", physical_table_id);
    }
    *physical_table = &(physical_table_mngr.physical_tables[physical_table_id]);

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_phy_table_iterator_init(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    dbal_physical_table_def_t *PhysicalTable;
    dbal_physical_tables_e physical_db_id = iterator_info->entry_handle->table->physical_db_id;
    uint32 app_id = iterator_info->entry_handle->table->app_id;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&iterator_info->mdb_iterator, 0x0, sizeof(iterator_info->mdb_iterator));

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, physical_db_id, &PhysicalTable));

    if (PhysicalTable->iterator_init == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "ERROR missing implementation for %s\n", PhysicalTable->physical_name);
    }

    SHR_IF_ERR_EXIT(PhysicalTable->iterator_init(unit, physical_db_id, app_id, &iterator_info->mdb_iterator));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_phy_table_entry_get_next(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    dbal_physical_table_def_t *PhysicalTable;
    dbal_physical_tables_e physical_db_id = iterator_info->entry_handle->table->physical_db_id;
    uint32 app_id = iterator_info->entry_handle->table->app_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, physical_db_id, &PhysicalTable));

    if (PhysicalTable->iterator_get_next == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "ERROR missing implementation for %s\n", PhysicalTable->physical_name);
    }

    SHR_IF_ERR_EXIT(PhysicalTable->iterator_get_next(unit, physical_db_id, app_id, &iterator_info->mdb_iterator,
                                                     &iterator_info->entry_handle->phy_entry, &iterator_info->is_end));
    if (!iterator_info->is_end)
    {
        uint8 is_valid_entry;
        SHR_IF_ERR_EXIT(dbal_set_key_info_from_buffer(unit, iterator_info->entry_handle, iterator_info->keys_info,
                                                      &is_valid_entry));
        if (!is_valid_entry)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, "MDB iterator found invalid entry\n");
        }

        SHR_IF_ERR_EXIT(dbal_phy_entry_print(unit, &iterator_info->entry_handle->phy_entry, FALSE, -1));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_phy_table_iterator_deinit(
    int unit,
    dbal_iterator_info_t * iterator_info)
{
    dbal_physical_table_def_t *PhysicalTable;
    dbal_physical_tables_e physical_db_id = iterator_info->entry_handle->table->physical_db_id;
    uint32 app_id = iterator_info->entry_handle->table->app_id;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, physical_db_id, &PhysicalTable));

    if (PhysicalTable->iterator_init == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "ERROR missing implementation for %s\n", PhysicalTable->physical_name);
    }

    SHR_IF_ERR_EXIT(PhysicalTable->iterator_deinit(unit, physical_db_id, app_id, &iterator_info->mdb_iterator));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_phy_table_entry_get(
    int unit,
    dbal_physical_tables_e physical_db_id,
    uint32 app_id,
    dbal_physical_entry_t * phy_entry)
{
    dbal_physical_table_def_t *PhysicalTable;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, physical_db_id, &PhysicalTable));

    if (PhysicalTable->entry_get == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "ERROR missing implementation for %s\n", PhysicalTable->physical_name);
    }

    SHR_IF_ERR_EXIT(PhysicalTable->entry_get(unit, physical_db_id, app_id, phy_entry));
    SHR_IF_ERR_EXIT(dbal_phy_entry_print(unit, phy_entry, FALSE, -1));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_phy_table_entry_delete(
    int unit,
    dbal_physical_tables_e physical_db_id,
    uint32 app_id,
    dbal_physical_entry_t * phy_entry)
{
    dbal_physical_table_def_t *PhysicalTable;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, physical_db_id, &PhysicalTable));

    if (PhysicalTable->entry_delete == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "missing implementation %s\n", PhysicalTable->physical_name);
    }

    SHR_IF_ERR_EXIT(dbal_phy_entry_print(unit, phy_entry, TRUE, -1));
    SHR_IF_ERR_EXIT(PhysicalTable->entry_delete(unit, physical_db_id, app_id, phy_entry));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_phy_table_entry_add(
    int unit,
    dbal_physical_tables_e physical_db_id,
    uint32 app_id,
    dbal_physical_entry_t * phy_entry)
{
    dbal_physical_table_def_t *PhysicalTable;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, physical_db_id, &PhysicalTable));

    if (PhysicalTable->entry_add == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, "missing implementation %s\n", PhysicalTable->physical_name);
    }

    SHR_IF_ERR_EXIT(dbal_phy_entry_print(unit, phy_entry, FALSE, -1));
    SHR_IF_ERR_EXIT(PhysicalTable->entry_add(unit, physical_db_id, app_id, phy_entry));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dbal_physical_db_info_print(
    int unit,
    dbal_physical_tables_e physical_db_id)
{

    dbal_physical_table_def_t *PhysicalTable;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, physical_db_id, &PhysicalTable));

    LOG_CLI((BSL_META("Physical Table %-20s\n"), PhysicalTable->physical_name));
    LOG_CLI((BSL_META("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n")));
    LOG_CLI((BSL_META("\tMax capacity: %d\n"), PhysicalTable->max_capacity));

exit:
    SHR_FUNC_EXIT;
}
