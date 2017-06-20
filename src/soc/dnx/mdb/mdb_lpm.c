/*
 * ! \file mdb_lpm.c Contains all of the MDB KAPS (LPM) initialization and API functions.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/shrextend/shrextend_debug.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

#include <soc/dnx/mdb.h>
#include <sal/appl/io.h>
#include "mdb_internal.h"
#include <soc/dnx/dbal/dbal_structures.h>
#include "../dbal/dbal_string_mgmt.h"
#include <shared/utilex/utilex_bitstream.h>
#include <shared/utilex/utilex_integer_arithmetic.h>

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_MDBDNX
#include <shared/bsl.h>

#ifdef CRASH_RECOVERY_SUPPORT
/*
 * crash recovery parameters
 */
#define MDB_KAPS_IS_CR_MODE(unit) 0
#define MDB_KAPS_NV_MEMORY_SIZE (25*1024*1024)
#endif

#define MDB_KAPS_ACCESS mdb_kaps_db

/*
 * KBPSDK related allocator/pointers.
 */
typedef struct
{
    struct kbp_allocator *dalloc_p;
    void *kaps_xpt_p;
    struct kbp_device *kaps_device_p;

} generic_kaps_app_data_t;

/*
 * kbp device warmboot info
 */
typedef struct mdb_kbp_warmboot_s
{
    FILE *kbp_file_fp;
    kbp_device_issu_read_fn kbp_file_read;
    kbp_device_issu_write_fn kbp_file_write;
} mdb_kbp_warmboot_t;

static mdb_kbp_warmboot_t Kaps_warmboot_data[SOC_MAX_NUM_DEVICES];

static FILE *Kaps_file_fp[SOC_MAX_NUM_DEVICES];

static generic_kaps_app_data_t *Lpm_app_data[SOC_MAX_NUM_DEVICES] = { NULL };

/*
 * Read file callback provided to the KBPSDK
 */
static int
mdb_lpm_file_read_func(
    void *handle,
    uint8_t * buffer,
    uint32_t size,
    uint32_t offset)
{
    size_t result;

    if (!handle)
    {
        return SOC_E_FAIL;
    }

    if (0 != fseek(handle, offset, SEEK_SET))
    {
        return SOC_E_FAIL;
    }

    result = fread(buffer, 1, size, handle);
    if (result < size)
    {
        return SOC_E_FAIL;
    }

    return SOC_E_NONE;
}

/*
 * Write file callback provided to the KBPSDK
 */
static int
mdb_lpm_file_write_func(
    void *handle,
    uint8_t * buffer,
    uint32_t size,
    uint32_t offset)
{
    size_t result;

    if (!handle)
    {
        return SOC_E_UNIT;
    }

    if (0 != fseek(handle, offset, SEEK_SET))
    {
        return SOC_E_FAIL;
    }

    result = fwrite(buffer, 1, size, handle);
    if (result != size)
    {
        return SOC_E_MEMORY;
    }
    fflush(handle);

    return SOC_E_NONE;
}

/*
 * Open a file to facilitate KBPSDK warmboot.
 */
static shr_error_e
mdb_lpm_kbp_file_open(
    int unit,
    char *filename,
    int device_type)
{
    int is_warmboot;
    char prefixed_file_name[SOC_PROPERTY_NAME_MAX + 256];
    char *stable_filename = NULL;

    FILE **file_fp = NULL;

    SHR_FUNC_INIT_VARS(unit);

    if (NULL == filename)
    {
        return 0;
    }

    if (device_type == KBP_DEVICE_KAPS)
    {
        file_fp = &Kaps_file_fp[unit];
    }

    if (*file_fp == NULL)
    {
        is_warmboot = SOC_WARM_BOOT(unit);

        prefixed_file_name[0] = '\0';

        stable_filename = soc_property_get_str(unit, spn_STABLE_FILENAME);

        /*
         * prefixing with unique file name to enable more than one parallel run from same SDK folder
         */
        /*
         * assuming stable_filename is unique for each separate run
         */
        if (NULL != stable_filename)
        {
            sal_strncat(prefixed_file_name, stable_filename, sizeof(prefixed_file_name) - 1);

            sal_strncat(prefixed_file_name, "_", sizeof(prefixed_file_name) - sal_strlen(prefixed_file_name) - 1);
        }
        sal_strncat(prefixed_file_name, filename, sizeof(prefixed_file_name) - sal_strlen(prefixed_file_name) - 1);

        if ((*file_fp = sal_fopen(prefixed_file_name, is_warmboot != 0 ? "r+" : "w+")) == 0)
        {
            SHR_ERR_EXIT(_SHR_E_INTERNAL, " Error:  sal_fopen() Failed\n");
        }
    }

    Kaps_warmboot_data[unit].kbp_file_fp = Kaps_file_fp[unit];
    Kaps_warmboot_data[unit].kbp_file_read = &mdb_lpm_file_read_func;
    Kaps_warmboot_data[unit].kbp_file_write = &mdb_lpm_file_write_func;

exit:
    SHR_FUNC_EXIT;
}

/*
 * This function performs Device Inits related to the KBPSDK.
 */
static shr_error_e
mdb_lpm_init_device(
    int unit)
{
    uint32 flags;
    mdb_kbp_warmboot_t *warmboot_data;
    uint8 is_allocated;

#ifdef CRASH_RECOVERY_SUPPORT
    uint8 *nv_mem_ptr = NULL;
    unsigned int nv_mem_size = JER_KAPS_NV_MEMORY_SIZE;
#endif

    SHR_FUNC_INIT_VARS(unit);

    Lpm_app_data[unit]->kaps_xpt_p = NULL;
#ifdef USE_MODEL
    /*
     * Initialize the C-Model
     */
    KBP_TRY(kbp_sw_model_init(Lpm_app_data[unit]->dalloc_p, KBP_DEVICE_KAPS, KBP_DEVICE_DEFAULT,
                              NULL, &(Lpm_app_data[unit]->kaps_xpt_p)));
#else
#ifndef BLACKHOLE_MODE
    SHR_IF_ERR_EXIT(mdb_lpm_xpt_init(unit, &(Lpm_app_data[unit]->kaps_xpt_p)));
#endif
#endif

    warmboot_data = &Kaps_warmboot_data[unit];

    flags = KBP_DEVICE_DEFAULT | KBP_DEVICE_ISSU;
    if (SOC_WARM_BOOT(unit))
    {
        flags |= KBP_DEVICE_SKIP_INIT;
#ifdef CRASH_RECOVERY_SUPPORT
        if (MDB_KAPS_IS_CR_MODE(unit) == 1)
        {
            flags &= ~KBP_DEVICE_ISSU;
        }
#endif
    }

    KBP_TRY(kbp_device_init(Lpm_app_data[unit]->dalloc_p,
                            KBP_DEVICE_KAPS,
                            flags,
                            (struct kaps_xpt *) Lpm_app_data[unit]->kaps_xpt_p,
                            NULL, &Lpm_app_data[unit]->kaps_device_p));

#ifdef CRASH_RECOVERY_SUPPORT
    if (MDB_KAPS_IS_CR_MODE(unit) == 1)
    {
        /*
         * Get allocation HA
         */
        nv_mem_ptr =
            ha_mem_alloc(unit, HA_KAPS_Mem_Pool, HA_KAPS_SUB_ID_0, KAPS_VERSION_1_0, KAPS_STRUCT_SIG, &nv_mem_size);
        if (nv_mem_ptr == NULL)
        {
            SHR_ERR_EXIT(_SHR_E_MEMORY, " Error:  ha_mem_alloc for nv_mem_ptr Failed\n");
        }

        if (SOC_WARM_BOOT(unit))
        {
            KBP_TRY(kbp_device_set_property
                    (Lpm_app_data[unit]->kaps_device_p, KBP_DEVICE_PROP_CRASH_RECOVERY, 0, nv_mem_ptr, nv_mem_size));
            KBP_TRY(kbp_device_restore_state(Lpm_app_data[unit]->kaps_device_p, NULL, NULL, NULL));
        }
        else
        {
            KBP_TRY(kbp_device_set_property
                    (Lpm_app_data[unit]->kaps_device_p, KBP_DEVICE_PROP_CRASH_RECOVERY, 1, nv_mem_ptr, nv_mem_size));
            SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.is_init(unit, &is_allocated));
            if (!is_allocated)
            {
                SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.init(unit));
            }
        }
    }
    else
#endif
    {
        if (SOC_WARM_BOOT(unit))
        {
            KBP_TRY(kbp_device_restore_state
                    (Lpm_app_data[unit]->kaps_device_p, warmboot_data->kbp_file_read, warmboot_data->kbp_file_write,
                     warmboot_data->kbp_file_fp));
        }
        else
        {
            SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.is_init(unit, &is_allocated));
            if (!is_allocated)
            {
                SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.init(unit));
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * This function initializes the private and public KAPS DBs.
 * It also initializes clone DBs per core/pipeline if necessary.
 * Assume first MDB_CURRENT_NOF_CORES DBs are private,                                                 .
 * and the next MDB_CURRENT_NOF_CORES DBs are public.
 */
static shr_error_e
mdb_lpm_db_init(
    int unit,
    uint32 db_id)
{
    mdb_kaps_db_handles_t db_handles_info;

    SHR_FUNC_INIT_VARS(unit);

    if (!SOC_WARM_BOOT(unit))
    {
        uint32 db_size;
        memset(&db_handles_info, 0, sizeof(db_handles_info));

        if (db_id == MDB_KAPS_IP_PRIVATE_DB_ID)
        {       /* Private DB (or a clone) */
            db_size = MDB_KAPS_PRIVATE_SIZE;
        }
        else if (db_id == MDB_KAPS_IP_PUBLIC_DB_ID)
        {       /* Public DB (or a clone) */
            db_size = MDB_KAPS_PUBLIC_SIZE;
        }
        else
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, " Error: unrecognized db_id: %d\n", db_id);
        }

        if (db_size != 0)
        {
            db_handles_info.is_valid = 1;
        }
        else
        {
            db_handles_info.is_valid = 0;
        }

        if (db_handles_info.is_valid == 1)
        {       /* Create the DB */
            KBP_TRY(kbp_db_init(Lpm_app_data[unit]->kaps_device_p, KBP_DB_LPM, db_id, db_size, &db_handles_info.db_p));

            /*
             * Associate DB with AD table
             */
            KBP_TRY(kbp_ad_db_init(Lpm_app_data[unit]->kaps_device_p,
                                   db_id, db_size, MDB_KAPS_AD_WIDTH_IN_BITS, &db_handles_info.ad_db_p));

            KBP_TRY(kbp_db_set_ad(db_handles_info.db_p, db_handles_info.ad_db_p));
        }

        /*
         * KBP_TRY(kbp_db_set_property(db_config_info->db_p, KBP_PROP_DEFER_DELETES, 1));
         */

        /*
         * save DB handles
         */
        SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.set(unit, db_id, &db_handles_info));
    }
    else
    {
        SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_id, &db_handles_info));

        KBP_TRY(kbp_db_refresh_handle(Lpm_app_data[unit]->kaps_device_p, db_handles_info.db_p, &db_handles_info.db_p));

        KBP_TRY(kbp_ad_db_refresh_handle
                (Lpm_app_data[unit]->kaps_device_p, db_handles_info.ad_db_p, &db_handles_info.ad_db_p));

        KBP_TRY(MDB_KAPS_ACCESS.db_info.set(unit, db_id, &db_handles_info));
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * This function initializes the KBPSDK search.
 * To simplify the implementation, it only configures a single search.
 */
static shr_error_e
mdb_kaps_search_init(
    int unit)
{
    uint32 db_idx;

    mdb_kaps_key_t_p master_key;
    mdb_kaps_key_t_p key;

    mdb_kaps_db_handles_t db_handles_info;

    mdb_kaps_instruction_handles_t instruction_handles_info;

    char *master_key_field_names[MDB_KAPS_IP_NOF_DB] = { "PRIVATE", "PUBLIC" };

    SHR_FUNC_INIT_VARS(unit);

    if (!SOC_WARM_BOOT(unit))
    {
        memset(&instruction_handles_info, 0, sizeof(instruction_handles_info));

        /*
         * create a new instruction
         */
        KBP_TRY(kbp_instruction_init(Lpm_app_data[unit]->kaps_device_p, 0 /* search_id */ ,
                                     0 /* ltr */ ,
                                     &(instruction_handles_info.inst_p)));

        /*
         * Create a key associated to each DB
         */
        for (db_idx = 0; db_idx < MDB_KAPS_IP_NOF_DB; db_idx++)
        {
            SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

            if (db_handles_info.is_valid == 1)
            {
                KBP_TRY(kbp_key_init(Lpm_app_data[unit]->kaps_device_p, &key));
                /*
                 * Add a single 160bit LPM field to key
                 */
                KBP_TRY(kbp_key_add_field(key,
                                          master_key_field_names[db_idx],
                                          MDB_KAPS_KEY_WIDTH_IN_BITS, KBP_KEY_FIELD_PREFIX));

                KBP_TRY(kbp_db_set_key(db_handles_info.db_p, key));
            }
        }

        /*
         * Master key creation
         */
        KBP_TRY(kbp_key_init(Lpm_app_data[unit]->kaps_device_p, &master_key));

        /*
         * Add fields to the master key
         * Even if a DB is disabled, we still need to build the full master key
         */
        for (db_idx = 0; db_idx < MDB_KAPS_IP_NOF_DB; db_idx++)
        {
            SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

            if (db_handles_info.is_valid == 1)
            {
                /*
                 * Add a single 160bit LPM field to key
                 */
                KBP_TRY(kbp_key_add_field(master_key,
                                          master_key_field_names[db_idx],
                                          MDB_KAPS_KEY_WIDTH_IN_BITS, KBP_KEY_FIELD_PREFIX));
            }
        }

        KBP_TRY(kbp_instruction_set_key(instruction_handles_info.inst_p, master_key));

        for (db_idx = 0; db_idx < MDB_KAPS_IP_NOF_DB; db_idx++)
        {
            SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

            if (db_handles_info.is_valid == 1)
            {
                KBP_TRY(kbp_instruction_add_db(instruction_handles_info.inst_p, db_handles_info.db_p, db_idx));
            }
        }

        KBP_TRY(kbp_instruction_install(instruction_handles_info.inst_p));

    }
    else
    {
        SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.
                        search_instruction_info.get(unit, MDB_LPM_INSTRUCTIONS_ID, &instruction_handles_info));

        KBP_TRY(kbp_instruction_refresh_handle
                (Lpm_app_data[unit]->kaps_device_p, instruction_handles_info.inst_p, &instruction_handles_info.inst_p));
    }

    SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.
                    search_instruction_info.set(unit, MDB_LPM_INSTRUCTIONS_ID, &instruction_handles_info));

exit:
    SHR_FUNC_EXIT;
}

/*
 * This function initializes the DBs and locks the device configuration.
 */
static shr_error_e
mdb_kaps_init_db_set(
    int unit)
{
    uint32 db_idx;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Configure DBs
     */
    for (db_idx = 0; db_idx < MDB_KAPS_IP_NOF_DB; db_idx++)
    {
        SHR_IF_ERR_EXIT(mdb_lpm_db_init(unit, db_idx));
    }

    /*
     * Configure search keys in KAPS
     */
    SHR_IF_ERR_EXIT(mdb_kaps_search_init(unit));

    KBP_TRY(kbp_device_lock(Lpm_app_data[unit]->kaps_device_p));

exit:
    SHR_FUNC_EXIT;
}

/*
 * This function performs the KBPSDK init sequence.
 */
static shr_error_e
mdb_lpm_init_app(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    if (Lpm_app_data[unit] == NULL)
    {
        SHR_ALLOC(Lpm_app_data[unit], sizeof(generic_kaps_app_data_t), "lpm_app_data[unit]", "%s%s%s\r\n", EMPTY, EMPTY,
                  EMPTY);
        if (Lpm_app_data[unit] == NULL)
        {
            SHR_ERR_EXIT(_SHR_E_MEMORY, " Error:  SHR_ALLOC for lpm_app_data Failed\n");
        }
        sal_memset(Lpm_app_data[unit], 0x0, sizeof(generic_kaps_app_data_t));
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_INIT, " Error: kaps is already initialized.\n");
    }

    /*
     * Create the default allocator
     */
    KBP_TRY(default_allocator_create(&Lpm_app_data[unit]->dalloc_p));

    /*
     * Initialize Device now
     */
    SHR_IF_ERR_EXIT(mdb_lpm_init_device(unit));

    /*
     * Create KAPS DB, config searches
     */
    SHR_IF_ERR_EXIT(mdb_kaps_init_db_set(unit));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Initialize the KBPSDK
 */
shr_error_e
mdb_lpm_init(
    int unit)
{

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Open a file used for the KBPSDK warmboot
     */
    SHR_IF_ERR_EXIT(mdb_lpm_kbp_file_open(unit, "kaps", KBP_DEVICE_KAPS));

    SHR_IF_ERR_EXIT(mdb_lpm_init_app(unit));

exit:
    SHR_FUNC_EXIT;
}

#ifdef CRASH_RECOVERY_SUPPORT
static shr_error_e
mdb_lpm_cr_transaction_cmd(
    int unit,
    uint8 is_start)
{
    SHR_FUNC_INIT_VARS(unit);

    if (Lpm_app_data[unit]->kaps_device_p != NULL)
    {
        if (is_start != 0)
        {
            KBP_TRY(kbp_device_start_transaction(Lpm_app_data[unit]->kaps_device_p));
        }
        else
        {
            KBP_TRY(kbp_device_end_transaction(Lpm_app_data[unit]->kaps_device_p));
        }
    }

    SHR_FUNC_EXIT;
}

static uint8
mdb_lpm_cr_query_restore_status(
    int unit)
{
    uint8 result = TRUE;

    enum kbp_restore_status res_status;
    if (Lpm_app_data[unit]->kaps_device_p != NULL)
    {
        KBP_TRY(kbp_device_query_restore_status(Lpm_app_data[unit]->kaps_device_p, &res_status));
    }

    if (res_status == KBP_RESTORE_CHANGES_ABORTED)
    {
        result = FALSE;
    }

    return result;
}

static uint32
mdb_lpm_cr_clear_restore_status(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    if (Lpm_app_data[unit]->kaps_device_p != NULL)
    {
        KBP_TRY(kbp_device_clear_restore_status(Lpm_app_data[unit]->kaps_device_p));
    }

    SHR_FUNC_EXIT;
}

static shr_error_e
mdb_lpm_cr_db_commit(
    int unit,
    uint32 db_idx)
{
    mdb_kaps_db_handles_t db_handles_info;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

    if (db_handles_info.db_p != NULL)
    {
        KBP_TRY(kbp_db_install(db_handles_info.db_p));
    }

exit:
    SHR_FUNC_EXIT;
}
#endif

/*
 * Translate from DBAL physical table to KAPS LPM DB ID.
 */
shr_error_e
mdb_lpm_dbal_to_db(
    int unit,
    dbal_physical_tables_e physical_table,
    mdb_kaps_ip_db_id_e * db_idx)
{
    SHR_FUNC_INIT_VARS(unit);

    if (physical_table == DBAL_PHYSICAL_TABLE_LPM_PRIVATE)
    {
        *db_idx = MDB_KAPS_IP_PRIVATE_DB_ID;
    }
    else if (physical_table == DBAL_PHYSICAL_TABLE_LPM_PUBLIC)
    {
        *db_idx = MDB_KAPS_IP_PUBLIC_DB_ID;
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, " Error: unrecognized LPM physical_table.\n");
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_prefix_len_to_mask(
    int unit,
    int prefix_length,
    dbal_physical_entry_t * entry)
{
    SHR_FUNC_INIT_VARS(unit);

    sal_memset(entry->k_mask, 0x0, DBAL_PHYSICAL_KEY_SIZE_IN_WORDS * sizeof(entry->k_mask[0]));

    if ((prefix_length >= 0) && (prefix_length <= MDB_KAPS_KEY_WIDTH_IN_BITS))
    {
        int ii;
        for (ii = 0; ii < MDB_KAPS_KEY_WIDTH_IN_UINT32; ii++)
        {
            if (prefix_length > ii * SAL_UINT32_NOF_BITS)
            {
                int local_prefix_length = prefix_length - ii * SAL_UINT32_NOF_BITS;
                entry->k_mask[MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - ii] =
                    _shr_ip_mask_create(local_prefix_length >
                                        SAL_UINT32_NOF_BITS ? SAL_UINT32_NOF_BITS : local_prefix_length);
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, " Error: Prefix must be between 0 and %d, prefix given: %d.\n",
                     MDB_KAPS_KEY_WIDTH_IN_BITS, prefix_length);
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Arrange bytes in the order the KBPSDK expects them
 */
static shr_error_e
mdb_lpm_uint32_to_uint8(
    int unit,
    int nof_bytes,
    uint32 * uint32_data,
    uint8 * uint8_data)
{
    int uint32_index;

    SHR_FUNC_INIT_VARS(unit);

    if (nof_bytes == MDB_KAPS_KEY_WIDTH_IN_BYTES)
    {
        /*
         * uint8_data[0] is the msb
         */
        for (uint32_index = 0; uint32_index < MDB_KAPS_KEY_WIDTH_IN_UINT32; uint32_index++)
        {
            uint8_data[(uint32_index * 4) + 0] =
                (uint32_data[MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - uint32_index] >> 24) & 0xFF;
            uint8_data[(uint32_index * 4) + 1] =
                (uint32_data[MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - uint32_index] >> 16) & 0xFF;
            uint8_data[(uint32_index * 4) + 2] =
                (uint32_data[MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - uint32_index] >> 8) & 0xFF;
            uint8_data[(uint32_index * 4) + 3] = (uint32_data[MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - uint32_index]) & 0xFF;
        }
    }
    else if (nof_bytes == MDB_KAPS_AD_WIDTH_IN_BYTES)
    {
        /*
         * index 0: bits 19-12, index 1: bits 11-4, index 2: bits 3-0 (shifted to the msb)
         */
        uint8_data[0] = (uint32_data[0] >> 12) & 0xFF;
        uint8_data[1] = (uint32_data[0] >> 4) & 0xFF;
        uint8_data[2] = (uint32_data[0] << 4) & 0xFF;
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, " Error:  unexpected nof_bytes to mdb_lpm_uint32_to_uint8.\n");
    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * Arrange bytes in the order the DBAL expects them
 */
static shr_error_e
mdb_lpm_uint8_to_uint32(
    int unit,
    int nof_bytes,
    uint8 * uint8_data,
    uint32 * uint32_data)
{
    SHR_FUNC_INIT_VARS(unit);

    if (nof_bytes == MDB_KAPS_KEY_WIDTH_IN_BYTES)
    {
        int uint32_index;
        /*
         * uint8_data[0] is the msb
         */
        for (uint32_index = 0; uint32_index < MDB_KAPS_KEY_WIDTH_IN_UINT32; uint32_index++)
        {
            uint32_data[MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - uint32_index] = (uint8_data[(uint32_index * 4) + 0] << 24) |
                (uint8_data[(uint32_index * 4) + 1] << 16) |
                (uint8_data[(uint32_index * 4) + 2] << 8) | uint8_data[(uint32_index * 4) + 3];
        }
    }
    else if (nof_bytes == MDB_KAPS_AD_WIDTH_IN_BYTES)
    {
        uint32_data[0] = uint8_data[0] << 12 | uint8_data[1] << 4 | uint8_data[2] >> 4;
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, " Error:  unexpected nof_bytes to mdb_lpm_uint32_to_uint8.\n");
    }

exit:
    SHR_FUNC_EXIT;
}

uint32
mdb_lpm_calculate_prefix_length(
    uint32 * payload_mask)
{
    int uint32_index;
    uint32 prefix_length = 0;

    for (uint32_index = MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1; uint32_index >= 0; uint32_index--)
    {
        /*
         * If the prefix is full or if this is the MSB, ignore the prefix MSB masking
         */
        if ((payload_mask[uint32_index] == 0xFFFFFFFF) ||
            ((uint32_index == MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1)
             && (payload_mask[uint32_index] == (0xFFFFFFFF >> MDB_KAPS_KEY_PREFIX_LENGTH))))
        {
            prefix_length += SAL_UINT32_NOF_BITS;
        }
        else
        {
            prefix_length += SAL_UINT32_NOF_BITS - (utilex_msb_bit_on(~payload_mask[uint32_index]) + 1);
            break;
        }
    }

    return prefix_length;
}

shr_error_e
mdb_lpm_entry_add(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    int32 res;
    mdb_kaps_db_handles_t db_handles_info;
    struct kbp_entry *db_entry = NULL;
    struct kbp_ad *ad_entry = NULL;
    int is_update = 0;
    mdb_kaps_ip_db_id_e db_idx;
    uint8 data[MDB_KAPS_KEY_WIDTH_IN_BYTES];
    uint8 asso_data[MDB_KAPS_AD_WIDTH_IN_BYTES];
    uint32 prefix_length;
    bsl_severity_t severity;

    SHR_FUNC_INIT_VARS(unit);

#ifdef CRASH_RECOVERY_SUPPORT
    if (MDB_KAPS_IS_CR_MODE(unit) == 1)
    {
        SHR_IF_ERR_EXIT(mdb_lpm_cr_transaction_cmd(unit, TRUE));
    }
    else
    {
        SHR_IF_ERR_EXIT(soc_dcmn_cr_suppress(unit, dcmn_cr_no_support_kaps_kbp));
    }
#endif

    SHR_IF_ERR_EXIT(mdb_lpm_dbal_to_db(unit, dbal_physical_table_id, &db_idx));

    SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

    if (db_handles_info.is_valid != 1)
    {
        SHR_ERR_EXIT(_SHR_E_RESOURCE, " Error: Physical table %s not allocated\n",
                     dbal_physical_table_to_string(unit, dbal_physical_table_id));
    }

    SHR_IF_ERR_EXIT(mdb_lpm_uint32_to_uint8(unit, MDB_KAPS_KEY_WIDTH_IN_BYTES, entry->key, data));
    /*
     * stamp the app_id to the msb
     */
    data[0] |= app_id << (SAL_UINT8_NOF_BITS - MDB_KAPS_KEY_PREFIX_LENGTH);

    SHR_IF_ERR_EXIT(mdb_lpm_uint32_to_uint8(unit, MDB_KAPS_AD_WIDTH_IN_BYTES, entry->payload, asso_data));

    /*
     * Verify the table_id bits are valid
     */
    SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range
                    (entry->k_mask, MDB_KAPS_KEY_WIDTH_IN_BITS - MDB_KAPS_KEY_PREFIX_LENGTH,
                     MDB_KAPS_KEY_WIDTH_IN_BITS - 1));
    prefix_length = mdb_lpm_calculate_prefix_length(entry->k_mask);

    /*
     * Check if the db_entry exists
     */
    kbp_db_get_prefix_handle(db_handles_info.db_p, data, prefix_length, &db_entry);
    if (db_entry != NULL)
    {
        is_update = 1;
    }

    if (!is_update)
    {
        KBP_TRY(kbp_db_add_prefix(db_handles_info.db_p, data, prefix_length, &db_entry));

        res = kbp_ad_db_add_entry(db_handles_info.ad_db_p, asso_data, &ad_entry);

        if (res != KBP_OK)
        {       /* rollback */
            kbp_db_delete_entry(db_handles_info.db_p, db_entry);
            SHR_ERR_EXIT(_SHR_E_INTERNAL, " Error:  kbp_ad_db_add_entry Failed\n");
        }

        res = kbp_entry_add_ad(db_handles_info.db_p, db_entry, ad_entry);

        if (res != KBP_OK)
        {       /* rollback */
            kbp_db_delete_entry(db_handles_info.db_p, db_entry);
            kbp_ad_db_delete_entry(db_handles_info.ad_db_p, ad_entry);
            SHR_ERR_EXIT(_SHR_E_INTERNAL, " Error:  kbp_entry_add_ad Failed\n");
        }
    }
    else
    {
        KBP_TRY(kbp_entry_get_ad(db_handles_info.db_p, db_entry, &ad_entry));

        KBP_TRY(kbp_ad_db_update_entry(db_handles_info.ad_db_p, ad_entry, asso_data));
    }

#ifdef CRASH_RECOVERY_SUPPORT
    if (MDB_KAPS_IS_CR_MODE(unit) == 1)
    {
        dcmn_cr_info[unit]->kaps_dirty = 1;
        dcmn_cr_info[unit]->kaps_tbl_id = table_id;
    }
    else
#endif
    {
        res = kbp_db_install(db_handles_info.db_p);
        if (res != KBP_OK)
        {       /* rollback */
            kbp_db_delete_entry(db_handles_info.db_p, db_entry);
            kbp_ad_db_delete_entry(db_handles_info.ad_db_p, ad_entry);

            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit,
                                  "Error in %s(): Entry add : kbp_db_install failed with: %s!\n"),
                       FUNCTION_NAME(), kbp_get_status_string(res)));

            if (res == KBP_OUT_OF_DBA || res == KBP_OUT_OF_UIT || res == KBP_OUT_OF_UDA || res == KBP_OUT_OF_AD)
            {
                SHR_ERR_EXIT(_SHR_E_FULL, " Error:  KAPS is full\n");
            }
            else
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, " Error:  kbp_db_install failed with: %s.\n", kbp_get_status_string(res));
            }
        }
    }

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        uint32 data_offset;
        uint32 print_index;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_add: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "prefix_length: %d. dbal_physical_table: %s, app_id: %d.\n"),
                                     prefix_length, dbal_physical_table_to_string(unit, dbal_physical_table_id),
                                     app_id));
        for (data_offset = 0; data_offset < MDB_KAPS_KEY_WIDTH_IN_UINT32; data_offset++)
        {
            print_index = MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->key[%d]: %08X.\n"), print_index, entry->key[print_index]));
        }
        for (data_offset = 0; data_offset < BITS2WORDS(MDB_KAPS_AD_WIDTH_IN_BITS); data_offset++)
        {
            print_index = BITS2WORDS(MDB_KAPS_AD_WIDTH_IN_BITS) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->payload[%d]: %08X.\n"), print_index, entry->payload[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_add: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_entry_get(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    mdb_kaps_db_handles_t db_handles_info;
    struct kbp_ad *ad_entry = NULL;
    struct kbp_entry *db_entry = NULL;
    mdb_kaps_ip_db_id_e db_idx;
    uint8 data[MDB_KAPS_KEY_WIDTH_IN_BYTES];
    uint8 asso_data[MDB_KAPS_AD_WIDTH_IN_BYTES];
    uint32 prefix_length;
    bsl_severity_t severity;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(mdb_lpm_dbal_to_db(unit, dbal_physical_table_id, &db_idx));

    SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

    if (db_handles_info.is_valid != 1)
    {
        SHR_ERR_EXIT(_SHR_E_RESOURCE, " Error: Physical table %s not allocated\n",
                     dbal_physical_table_to_string(unit, dbal_physical_table_id));
    }

    SHR_IF_ERR_EXIT(mdb_lpm_uint32_to_uint8(unit, MDB_KAPS_KEY_WIDTH_IN_BYTES, entry->key, data));
    /*
     * stamp the app_id to the msb
     */
    data[0] |= app_id << (SAL_UINT8_NOF_BITS - MDB_KAPS_KEY_PREFIX_LENGTH);

    /*
     * Verify the table_id bits are valid
     */
    SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range
                    (entry->k_mask, MDB_KAPS_KEY_WIDTH_IN_BITS - MDB_KAPS_KEY_PREFIX_LENGTH,
                     MDB_KAPS_KEY_WIDTH_IN_BITS - 1));
    prefix_length = mdb_lpm_calculate_prefix_length(entry->k_mask);

    /*
     * Retrieve the db_entry
     */
    kbp_db_get_prefix_handle(db_handles_info.db_p, data, prefix_length, &db_entry);
    if (db_entry == NULL)
    {
        /*
         * Exit without error print
         */
        SHR_SET_CURRENT_ERR(_SHR_E_NOT_FOUND);
        SHR_EXIT();
    }

    KBP_TRY(kbp_entry_get_ad(db_handles_info.db_p, db_entry, &ad_entry));
    if (ad_entry == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, " Error:  kbp_entry_get_ad() Failed\n");
    }

    sal_memset(asso_data, 0x0, sizeof(asso_data[0]) * MDB_KAPS_AD_WIDTH_IN_BYTES);

    KBP_TRY(kbp_ad_db_get(db_handles_info.ad_db_p, ad_entry, asso_data));

    SHR_IF_ERR_EXIT(mdb_lpm_uint8_to_uint32(unit, MDB_KAPS_AD_WIDTH_IN_BYTES, asso_data, entry->payload));

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        uint32 data_offset;
        uint32 print_index;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_get: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "prefix_length: %d. dbal_physical_table: %s, app_id: %d.\n"),
                                     prefix_length, dbal_physical_table_to_string(unit, dbal_physical_table_id),
                                     app_id));
        for (data_offset = 0; data_offset < MDB_KAPS_KEY_WIDTH_IN_UINT32; data_offset++)
        {
            print_index = MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->key[%d]: %08X.\n"), print_index, entry->key[print_index]));
        }
        for (data_offset = 0; data_offset < BITS2WORDS(MDB_KAPS_AD_WIDTH_IN_BITS); data_offset++)
        {
            print_index = BITS2WORDS(MDB_KAPS_AD_WIDTH_IN_BITS) - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->payload[%d]: %08X.\n"), print_index, entry->payload[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_get: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_entry_delete(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    mdb_kaps_db_handles_t db_handles_info;
    struct kbp_ad *ad_entry = NULL;
    struct kbp_entry *db_entry = NULL;
    mdb_kaps_ip_db_id_e db_idx;
    uint32 prefix_length;
    bsl_severity_t severity;

    uint8 data[MDB_KAPS_KEY_WIDTH_IN_BYTES];

    SHR_FUNC_INIT_VARS(unit);

#ifdef CRASH_RECOVERY_SUPPORT
    if (MDB_KAPS_IS_CR_MODE(unit) == 1)
    {
        SOC_SAND_IF_ERR_EXIT(mdb_lpm_cr_transaction_cmd(unit, TRUE));
    }
    else
    {
        SOC_SAND_IF_ERR_EXIT(soc_dcmn_cr_suppress(unit, dcmn_cr_no_support_kaps_kbp));
    }
#endif

    SHR_IF_ERR_EXIT(mdb_lpm_dbal_to_db(unit, dbal_physical_table_id, &db_idx));

    SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

    if (db_handles_info.is_valid != 1)
    {
        SHR_ERR_EXIT(_SHR_E_RESOURCE, " Error: Physical table %s not allocated\n",
                     dbal_physical_table_to_string(unit, dbal_physical_table_id));
    }

    SHR_IF_ERR_EXIT(mdb_lpm_uint32_to_uint8(unit, MDB_KAPS_KEY_WIDTH_IN_BYTES, entry->key, data));
    /*
     * stamp the app_id to the msb
     */
    data[0] |= app_id << (SAL_UINT8_NOF_BITS - MDB_KAPS_KEY_PREFIX_LENGTH);

    /*
     * Verify the table_id bits are valid
     */
    SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range
                    (entry->k_mask, MDB_KAPS_KEY_WIDTH_IN_BITS - MDB_KAPS_KEY_PREFIX_LENGTH,
                     MDB_KAPS_KEY_WIDTH_IN_BITS - 1));
    prefix_length = mdb_lpm_calculate_prefix_length(entry->k_mask);

    /*
     * Retrieve the db_entry
     */
    KBP_TRY(kbp_db_get_prefix_handle(db_handles_info.db_p, data, prefix_length, &db_entry));
    if (db_entry == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "mdb_lpm_entry_get, entry not found.");
    }

    /*
     * Retrieve the ad_entry
     */
    KBP_TRY(kbp_entry_get_ad(db_handles_info.db_p, db_entry, &ad_entry));
    if (ad_entry == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL, " Error:  kbp_entry_get_ad() Failed\n");
    }

    KBP_TRY(kbp_db_delete_entry(db_handles_info.db_p, db_entry));

    KBP_TRY(kbp_ad_db_delete_entry(db_handles_info.ad_db_p, ad_entry));

#ifdef CRASH_RECOVERY_SUPPORT
    if (MDB_KAPS_IS_CR_MODE(unit) == 1)
    {
        dcmn_cr_info[unit]->kaps_dirty = 1;
        dcmn_cr_info[unit]->kaps_tbl_id = table_id;
    }
    else
#endif
    {
        KBP_TRY(kbp_db_install(db_handles_info.db_p));
    }

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        uint32 data_offset;
        uint32 print_index;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_delete: start\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "prefix_length: %d. dbal_physical_table: %s, app_id: %d.\n"),
                                     prefix_length, dbal_physical_table_to_string(unit, dbal_physical_table_id),
                                     app_id));
        for (data_offset = 0; data_offset < MDB_KAPS_KEY_WIDTH_IN_UINT32; data_offset++)
        {
            print_index = MDB_KAPS_KEY_WIDTH_IN_UINT32 - 1 - data_offset;
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "entry->key[%d]: %08X.\n"), print_index, entry->key[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_delete: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_iterator_init(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator)
{
    mdb_kaps_db_handles_t db_handles_info;
    mdb_kaps_ip_db_id_e db_idx;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(mdb_lpm_dbal_to_db(unit, dbal_physical_table_id, &db_idx));

    SHR_IF_ERR_EXIT(MDB_KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info));

    if (db_handles_info.is_valid != 1)
    {
        SHR_ERR_EXIT(_SHR_E_RESOURCE, " Error: Physical table %s not allocated\n",
                     dbal_physical_table_to_string(unit, dbal_physical_table_id));
    }

    physical_entry_iterator->mdb_lpm_db_p = db_handles_info.db_p;
    physical_entry_iterator->mdb_lpm_ad_db_p = db_handles_info.ad_db_p;

    KBP_TRY(kbp_db_entry_iter_init(physical_entry_iterator->mdb_lpm_db_p, &physical_entry_iterator->mdb_lpm_iter));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_iterator_get_next(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator,
    dbal_physical_entry_t * entry,
    uint8 * is_end)
{
    struct kbp_entry *kpb_e;
    struct kbp_entry_info kpb_e_info;

    SHR_FUNC_INIT_VARS(unit);

    KBP_TRY(kbp_db_entry_iter_next
            (physical_entry_iterator->mdb_lpm_db_p, physical_entry_iterator->mdb_lpm_iter, &kpb_e));

    if (kpb_e == NULL)
    {
        *is_end = TRUE;
    }
    else
    {
        KBP_TRY(kbp_entry_get_info(physical_entry_iterator->mdb_lpm_db_p, kpb_e, &kpb_e_info));

        /*
         * Verify that the table prefix is equal to app id, retrieve next if not equal
         */
        while ((kpb_e_info.data[0] >> (SAL_UINT8_NOF_BITS - MDB_KAPS_KEY_PREFIX_LENGTH)) != app_id)
        {
            KBP_TRY(kbp_db_entry_iter_next
                    (physical_entry_iterator->mdb_lpm_db_p, physical_entry_iterator->mdb_lpm_iter, &kpb_e));

            if (kpb_e == NULL)
            {
                *is_end = TRUE;
                break;
            }
            else
            {
                KBP_TRY(kbp_entry_get_info(physical_entry_iterator->mdb_lpm_db_p, kpb_e, &kpb_e_info));
            }
        }

        if (kpb_e != NULL)
        {
            if (kpb_e_info.ad_handle != NULL)
            {
                uint8 ad_8[MDB_KAPS_AD_WIDTH_IN_BYTES];

                KBP_TRY(kbp_ad_db_get(physical_entry_iterator->mdb_lpm_ad_db_p, kpb_e_info.ad_handle, ad_8));

                sal_memset(entry, 0x0, sizeof(*entry));

                /*
                 * Set key fields
                 */
                entry->key_size = MDB_KAPS_KEY_WIDTH_IN_BITS;
                SHR_IF_ERR_EXIT(mdb_lpm_uint8_to_uint32
                                (unit, MDB_KAPS_KEY_WIDTH_IN_BYTES, kpb_e_info.data, entry->key));
                SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range
                                (entry->k_mask, MDB_KAPS_KEY_WIDTH_IN_BITS - kpb_e_info.prio_len,
                                 MDB_KAPS_KEY_WIDTH_IN_BITS - 1));

                /*
                 * Set payload fields
                 */
                entry->payload_size = MDB_KAPS_AD_WIDTH_IN_BITS;
                SHR_IF_ERR_EXIT(mdb_lpm_uint8_to_uint32(unit, MDB_KAPS_AD_WIDTH_IN_BYTES, ad_8, entry->payload));
                SHR_IF_ERR_EXIT(utilex_bitstream_set_bit_range(entry->p_mask, 0, MDB_KAPS_AD_WIDTH_IN_BITS - 1));
            }
            else
            {
                SHR_ERR_EXIT(_SHR_E_INTERNAL, "KBPSDK entry AD handle is null.\n");
                *is_end = TRUE;
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_iterator_deinit(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator)
{
    SHR_FUNC_INIT_VARS(unit);

    KBP_TRY(kbp_db_entry_iter_destroy(physical_entry_iterator->mdb_lpm_db_p, physical_entry_iterator->mdb_lpm_iter));

    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_test(
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    mdb_test_mode_e mode)
{
    dbal_physical_table_def_t *dbal_physical_table;
    dbal_physical_entry_t entry, entry_duplicate;
    int max_entries;
    int entry_counter;
    int uint32_counter;
    int prefix_length;
    shr_error_e res;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&entry, 0x0, sizeof(entry));

    SHR_IF_ERR_EXIT(dbal_physical_table_get(unit, dbal_physical_table_id, &dbal_physical_table));

    max_entries = dbal_physical_table->max_capacity;

    /*
     * Iterate on max number of entries, multiplied by 2 since half the entries are deleted to verify delete functionality
     */
    entry_counter = 0;
    while (entry_counter < max_entries * 2)
    {
        /*
         * Fill the entry key and mask with random content
         */
        prefix_length = sal_rand() % MDB_KAPS_KEY_WIDTH_IN_BITS;
        for (uint32_counter = 0;
             uint32_counter < ((MDB_KAPS_KEY_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS);
             uint32_counter++)
        {
            entry.key[uint32_counter] = sal_rand();
            if (prefix_length > uint32_counter * SAL_UINT32_NOF_BITS)
            {
                int local_prefix_length = prefix_length - uint32_counter * SAL_UINT32_NOF_BITS;
                entry.k_mask[uint32_counter] =
                    _shr_ip_mask_create(local_prefix_length >
                                        SAL_UINT32_NOF_BITS ? SAL_UINT32_NOF_BITS : local_prefix_length);
            }

        }

        /*
         * Zero redundant bits
         */
        if ((((MDB_KAPS_KEY_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) !=
            MDB_KAPS_KEY_WIDTH_IN_BITS)
        {
            SHR_IF_ERR_EXIT(utilex_bitstream_reset_bit_range
                            (entry.key, MDB_KAPS_KEY_WIDTH_IN_BITS,
                             (((MDB_KAPS_KEY_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS -
                                1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) - 1));
        }

        /*
         * Copy the entry to the duplicate entry, to be used later to verify the get function
         */
        sal_memcpy(&entry_duplicate, &entry, sizeof(entry));

        /*
         * Fill the entry payload with random content
         */
        for (uint32_counter = 0;
             uint32_counter < ((MDB_KAPS_AD_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS);
             uint32_counter++)
        {
            entry.payload[uint32_counter] = sal_rand();
        }

        /*
         * Zero redundant bits
         */
        if ((((MDB_KAPS_AD_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) !=
            MDB_KAPS_AD_WIDTH_IN_BITS)
        {
            SHR_IF_ERR_EXIT(utilex_bitstream_reset_bit_range
                            (entry.payload, MDB_KAPS_AD_WIDTH_IN_BITS,
                             (((MDB_KAPS_AD_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS -
                                1) / SAL_UINT32_NOF_BITS) * SAL_UINT32_NOF_BITS) - 1));
        }

        SHR_IF_ERR_EXIT(mdb_lpm_entry_add(unit, dbal_physical_table_id, 0, &entry));

        SHR_IF_ERR_EXIT(mdb_lpm_entry_get(unit, dbal_physical_table_id, 0, &entry_duplicate));

        /*
         * Xor between the read data and the written data, expect the output to be all zeros
         */
        SHR_IF_ERR_EXIT(utilex_bitstream_xor
                        (entry.payload, entry_duplicate.payload,
                         (MDB_KAPS_AD_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS));

        if (utilex_bitstream_have_one_in_range(entry.payload, 0 /* start_place */ , MDB_KAPS_AD_WIDTH_IN_BITS - 1))
        {
            SHR_IF_ERR_EXIT(utilex_bitstream_xor
                            (entry.payload, entry_duplicate.payload,
                             (MDB_KAPS_AD_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS));

            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Written data:\n 0x")));
            for (uint32_counter = ((MDB_KAPS_AD_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) - 1;
                 uint32_counter >= 0; uint32_counter--)
            {
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%08X"), entry.payload[uint32_counter]));
            }
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));

            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Read data:\n 0x")));
            for (uint32_counter = ((MDB_KAPS_AD_WIDTH_IN_BITS + SAL_UINT32_NOF_BITS - 1) / SAL_UINT32_NOF_BITS) - 1;
                 uint32_counter >= 0; uint32_counter--)
            {
                LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "%08X"), entry_duplicate.payload[uint32_counter]));
            }
            LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));

            SHR_ERR_EXIT(_SHR_E_FAIL, "Test failed, read data is not equal to written data.\n");
        }

        /*
         * Only delete half the entries, to also allow max capacity tests
         */
        if (entry_counter % 2 == 0)
        {
            /*
             * Delete the entry
             */
            SHR_IF_ERR_EXIT(mdb_lpm_entry_delete(unit, dbal_physical_table_id, 0, &entry_duplicate));

            res = mdb_lpm_entry_get(unit, dbal_physical_table_id, 0, &entry_duplicate);
            if (res != _SHR_E_NOT_FOUND)
            {
                SHR_ERR_EXIT(_SHR_E_FAIL, "Test failed, read data after delete should return NOT_FOUND.\n");
            }
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

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
