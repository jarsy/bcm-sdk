/*
 * ! \file mdb_lpm_xpt.c Contains all of the MDB KAPS XPT callbacks provided to the KBPSDK.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/shrextend/shrextend_debug.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

#include <soc/dnx/mdb.h>
#include "mdb_internal.h"
#include <soc/dnx/dbal/dbal_structures.h>

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_MDBDNX
#include <shared/bsl.h>

#define MDB_LPM_KEY_BUFFER_NOF_BYTES           (20)

#define MDB_LPM_RPB_FIRST_BLK_ID  1
#define MDB_LPM_RPB_LAST_BLK_ID   4
#define MDB_LPM_BB_FIRST_BLK_ID   5
#define MDB_LPM_BB_LAST_BLK_ID    36

#define MDB_LPM_NOF_BLOCKS        4
#define MDB_LPM_BB_PER_BLOCK      (32/MDB_LPM_NOF_BLOCKS)

#define MDB_LPM_MAX_UINT32_WIDTH     16
#define MDB_LPM_RPB_MAX_UINT32_WIDTH 6
#define MDB_LPM_AD_MAX_UINT32_WIDTH  4

#define MDB_LPM_RPB_CAM_BIST_CONTROL_OFFSET   0x2a
#define MDB_LPM_RPB_CAM_BIST_STATUS_OFFSET    0x2b
#define MDB_LPM_RPB_GLOBAL_CONFIG_OFFSET      0x21

#define MDB_LPM_BB_GLOBAL_CONFIG_OFFSET       0x21
#define MDB_LPM_BB_MEM_CONFIG1_OFFSET         0x23

/*
 * Revision register
 */
#define KAPS_JERICHO_2_REVISION_REG_VALUE (0x00050000)

typedef enum
{
    MDB_LPM_CMD_WRITE,
    MDB_LPM_CMD_READ,
    MDB_LPM_CMD_COMPARE,
    MDB_LPM_CMD_SEARCH,

    MDB_NOF_LPM_CMD
} mdb_lpm_cmd_e;

typedef struct
{
    struct kaps_xpt mdb_lpm_xpt;
    int unit;
} MDB_LPM_XPT;

kbp_status
mdb_lpm_register_write(
    void *xpt,
    uint32_t offset,
    uint32_t nbytes,
    uint8_t * bytes)
{
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(((MDB_LPM_XPT *) xpt)->unit, "%s() is not supported.\n"), FUNCTION_NAME()));
    return KBP_FATAL_TRANSPORT_ERROR;
}

kbp_status
mdb_lpm_register_read(
    void *xpt,
    uint32_t offset,
    uint32_t nbytes,
    uint8_t * bytes)
{
    int rv = KBP_OK, i, unit;
    uint32 reg_val = 0;

    unit = ((MDB_LPM_XPT *) xpt)->unit;
    if ((offset == 0) && (nbytes == 4) && (bytes != NULL))
    {
        reg_val = KAPS_JERICHO_2_REVISION_REG_VALUE;
    }
    else
    {
        LOG_ERROR(BSL_LOG_MODULE,
                  (BSL_META_U(unit, "%s() only supports reading the KapsRevision register.\n"), FUNCTION_NAME()));
        return KBP_FATAL_TRANSPORT_ERROR;
    }
    for (i = 0; i < nbytes; i++)
    {
        bytes[i] = (reg_val >> ((nbytes - 1 - i) * 8)) & 0xff;
    }

    return rv;
}

kbp_status
mdb_lpm_search(
    void *xpt,
    uint8_t * key,
    enum kaps_search_interface search_interface,
    struct kaps_search_result * kaps_result)
{
    int rv = KBP_OK, unit;
    uint32 func = KAPS_FUNC3, i = 0, j, k;
    uint32 mem_array[MDB_LPM_MAX_UINT32_WIDTH];
    uint32 mem_index = MDB_LPM_RPB_MAX_UINT32_WIDTH - 2;
    bsl_severity_t severity;

    unit = ((MDB_LPM_XPT *) xpt)->unit;

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U(unit, "%s():  search_interface: %d, key: 0x"), FUNCTION_NAME(), search_interface));
        for (j = 0; j < MDB_LPM_KEY_BUFFER_NOF_BYTES; j++)
        {
            LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%02X"), key[j]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
    }

    sal_memset(mem_array, 0, MDB_LPM_MAX_UINT32_WIDTH * sizeof(uint32));
    for (k = 0; k < MDB_LPM_KEY_BUFFER_NOF_BYTES / 4; k++)
    {
        for (j = 0; j < 4; j++)
        {
            mem_array[mem_index] |= key[i] << (3 - j) * 8;
            i++;
        }
        mem_index--;
    }
    mem_array[MDB_LPM_RPB_MAX_UINT32_WIDTH - 1] |= (func << 6);

    /*
     * rv = soc_mem_array_write(unit, KAPS_RPB_TCAM_CPU_COMMANDm, 0, KAPS_BLOCK(unit, search_interface), 0 ,
     * mem_array);
     */
    rv = soc_mem_array_write(unit, KAPS_RPB_TCAM_CPU_COMMANDm, 0, KAPS_BLOCK(unit), 0, mem_array);
    if (rv != _SHR_E_NONE)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), soc_mem_array_write failed.\n"), FUNCTION_NAME()));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    /*
     * rv = soc_mem_array_read(unit, KAPS_RPB_TCAM_CPU_COMMANDm, 0, KAPS_BLOCK(unit, search_interface), 0 , mem_array);
     */
    rv = soc_mem_array_read(unit, KAPS_RPB_TCAM_CPU_COMMANDm, 0, KAPS_BLOCK(unit), 0, mem_array);
    if (rv != _SHR_E_NONE)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), soc_mem_array_read failed.\n"), FUNCTION_NAME()));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    /*
     * The payload should be in [57:38] and the length in [37:30]
     */
    kaps_result->match_len = mem_array[0] >> 30 | ((mem_array[1] & 0x3F) << 2);
    kaps_result->ad_value[2] = (mem_array[1] >> 2);
    kaps_result->ad_value[1] = (mem_array[1] >> 10);
    kaps_result->ad_value[0] = (mem_array[1] >> 18);

    /*
     *  QAX format:
     *   kaps_result->match_len = mem_array[0] >> 24;
     *   kaps_result->ad_value[2]= (mem_array[1] << 4);
     *   kaps_result->ad_value[1]= (mem_array[1] >> 4);
     *   kaps_result->ad_value[0]= (mem_array[1] >> 12);
     */

    if (severity >= bslSeverityVerbose)
    {
        LOG_VERBOSE(BSL_LOG_MODULE,
                    (BSL_META_U(unit, "result_length: %d, result:0x%02x%02x%02x\n"), kaps_result->match_len,
                     kaps_result->ad_value[0], kaps_result->ad_value[1], kaps_result->ad_value[2]));
        for (j = 0; j < MDB_LPM_MAX_UINT32_WIDTH; j++)
        {
            LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mem_array[%d] = 0x%08X\n"), j, mem_array[j]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s()\n"), FUNCTION_NAME()));
    }

    return rv;
}

/*
 * This function always returns the core0 memory/register, to advance to the next core add 1 to *blk
 */
STATIC
    kbp_status mdb_lpm_translate_blk_func_offset_to_mem_reg(int unit,
                                                            uint8 blk_id,
                                                            uint32 func,
                                                            uint32 offset,
                                                            soc_mem_t * mem,
                                                            soc_reg_t * reg, uint32 * array_index, int *blk)
{
    uint32 rv = KBP_OK;

    *mem = INVALIDm;
    *reg = INVALIDr;
    *array_index = 0;

    if (blk_id >= MDB_LPM_RPB_FIRST_BLK_ID && blk_id <= MDB_LPM_RPB_LAST_BLK_ID)
    {
        /*
         * blk = KAPS_BLOCK(unit, (blk_id - MDB_LPM_RPB_FIRST_BLK_ID));
         */
        *blk = KAPS_BLOCK(unit);
        switch (func)
        {
            case KAPS_FUNC0:
                if (offset == MDB_LPM_RPB_CAM_BIST_CONTROL_OFFSET)
                {
                    *reg = KAPS_RPB_CAM_BIST_CONTROLr;
                }
                else if (offset == MDB_LPM_RPB_CAM_BIST_STATUS_OFFSET)
                {
                    *reg = KAPS_RPB_CAM_BIST_STATUSr;
                }
                else if (offset == MDB_LPM_RPB_GLOBAL_CONFIG_OFFSET)
                {
                    *reg = KAPS_RPB_GLOBAL_CONFIGr;
                }
                else
                {
                    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s():  unsupported RPB register offset: %d\n"),
                                               FUNCTION_NAME(), offset));
                    rv = KBP_FATAL_TRANSPORT_ERROR;
                }
                break;

            case KAPS_FUNC1:
                *mem = KAPS_RPB_TCAM_CPU_COMMANDm;
                break;

            case KAPS_FUNC4:
                *mem = KAPS_RPB_ADSm;
                break;

            default:
                LOG_ERROR(BSL_LOG_MODULE,
                          (BSL_META_U(unit, "%s():  RPB, unsupported func: %d\n"), FUNCTION_NAME(), func));
                rv = KBP_FATAL_TRANSPORT_ERROR;
                break;
        }
    }
    else if (blk_id >= MDB_LPM_BB_FIRST_BLK_ID && blk_id <= MDB_LPM_BB_LAST_BLK_ID)
    {
        /*
         * blk = KAPS_BLOCK(unit, ((blk_id - MDB_LPM_BB_FIRST_BLK_ID) / MDB_LPM_BB_PER_BLOCK));
         */
        *blk = KAPS_BLOCK(unit);
        *array_index = (blk_id - MDB_LPM_BB_FIRST_BLK_ID) % MDB_LPM_BB_PER_BLOCK;
        switch (func)
        {
            case KAPS_FUNC0:
                if (offset == MDB_LPM_BB_GLOBAL_CONFIG_OFFSET)
                {
                    *reg = KAPS_BB_GLOBAL_CONFIGr;
                }
                else
                {
                    LOG_ERROR(BSL_LOG_MODULE,
                              (BSL_META_U(unit, "%s():  unsupported BB register offset: %d\n"), FUNCTION_NAME(),
                               offset));
                    rv = KBP_FATAL_TRANSPORT_ERROR;
                }
                break;

            case KAPS_FUNC2:
            case KAPS_FUNC5:
                *mem = KAPS_BUCKET_MEMORYm;
                break;

            default:
                LOG_ERROR(BSL_LOG_MODULE,
                          (BSL_META_U(unit, "%s():  BB, unsupported func: %d\n"), FUNCTION_NAME(), func));
                rv = KBP_FATAL_TRANSPORT_ERROR;
                break;
        }
    }
    else
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), unrecognized blk_id = %d.\n"), FUNCTION_NAME(), blk_id));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    return rv;
}

kbp_status
mdb_lpm_write_command(
    void *xpt,
    uint8 blk_id,
    uint32 cmd,
    uint32 func,
    uint32 offset,
    uint32 nbytes,
    uint8 * bytes)
{
    int rv = KBP_OK, unit = ((MDB_LPM_XPT *) xpt)->unit;
    soc_mem_t mem;
    soc_reg_t reg;
    uint32 reg_val;
    uint32 array_index;
    uint32 mem_array[MDB_LPM_MAX_UINT32_WIDTH];
    uint32 mem_index = 0;
    uint32 i = 0, k;
    int blk;
    bsl_severity_t severity;
    uint32 core;

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s() start\n nbytes: %d, bytes: 0x"), FUNCTION_NAME(), nbytes));
        for (i = 0; i < nbytes; i++)
        {
            LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%02X "), bytes[i]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s():  blk_id: %d, cmd: %d, func: %d, offset: %d, nbytes: %d\n"),
                                     FUNCTION_NAME(), blk_id, cmd, func, offset, nbytes));
    }

    rv = mdb_lpm_translate_blk_func_offset_to_mem_reg(unit, blk_id, func, offset, &mem, &reg, &array_index, &blk);
    if (rv != KBP_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE,
                  (BSL_META_U(unit, "%s(), mdb_lpm_translate_blk_func_offset_to_mem_reg failed.\n"), FUNCTION_NAME()));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(): mem: %s, reg: %s, array_index: %d. \n"),
                                 FUNCTION_NAME(), SOC_MEM_NAME(unit, mem), SOC_REG_NAME(unit, reg), array_index));

    if (mem != INVALIDm)
    {
        /*
         * Write to memory
         */
        sal_memset(mem_array, 0, MDB_LPM_MAX_UINT32_WIDTH * sizeof(uint32));

        /*
         * Convert from uint8 to uint32
         */
        if (mem == KAPS_RPB_TCAM_CPU_COMMANDm)
        {
            mem_index = MDB_LPM_RPB_MAX_UINT32_WIDTH - 1;
        }
        else if (mem == KAPS_RPB_ADSm)
        {
            mem_index = MDB_LPM_AD_MAX_UINT32_WIDTH - 1;
        }
        else if ((mem == KAPS_BUCKET_MEMORYm) || (mem == KAPS_BBS_BUCKET_MEMORYm))
        {
            mem_index = MDB_LPM_MAX_UINT32_WIDTH - 2;
        }

        i = 0;
        if (nbytes % 4 > 0)
        {
            for (i = 0; i < nbytes % 4; i++)
            {
                mem_array[mem_index] |= bytes[i] << (nbytes % 4 - 1 - i) * 8;
            }
            mem_index--;
        }
        for (k = 0; k < nbytes / 4; k++)
        {
            mem_array[mem_index] = bytes[i] << 24 | bytes[i + 1] << 16 | bytes[i + 2] << 8 | bytes[i + 3];
            i += 4;
            mem_index--;
        }

        /*
         * Assume the different cores are in sequential blks
         */
        for (core = 0; core < MDB_CURRENT_NOF_CORES; core++)
        {
            rv = soc_mem_array_write(unit, mem, array_index, blk + core, offset, mem_array);
            if (rv != _SHR_E_NONE)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), soc_mem_array_write failed.\n"), FUNCTION_NAME()));
                return KBP_FATAL_TRANSPORT_ERROR;
            }
        }
    }
    else if (reg != INVALIDr)
    {
        /*
         * Write to a register
         */
        /*
         * Assuming only registers up to 32 bits
         */
        reg_val = (uint32) bytes[0] << 24 | (uint32) bytes[1] << 16 | (uint32) bytes[2] << 8 | (uint32) bytes[3];

        /*
         * Assume the different cores are in sequential blks
         */
        for (core = 0; core < MDB_CURRENT_NOF_CORES; core++)
        {
            rv = soc_reg32_set(unit, reg, blk, array_index, reg_val);
            if (rv != _SHR_E_NONE)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), soc_reg32_set failed.\n"), FUNCTION_NAME()));
                return KBP_FATAL_TRANSPORT_ERROR;
            }
        }

    }
    else
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), both mem and reg are invalid.\n"), FUNCTION_NAME()));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s() end\n"), FUNCTION_NAME()));

    return KBP_OK;
}

/*
 * This function always reads from core 0
 */
kbp_status
mdb_lpm_read_command(
    void *xpt,
    uint32 blk_id,
    uint32 cmd,
    uint32 func,
    uint32 offset,
    uint32 nbytes,
    uint8 * bytes)
{
    int rv = KBP_OK, unit = ((MDB_LPM_XPT *) xpt)->unit;
    soc_mem_t mem;
    soc_reg_t reg;
    uint32 reg_val;
    uint32 array_index;
    uint32 mem_array[MDB_LPM_MAX_UINT32_WIDTH];
    uint32 mem_index = 0;
    uint32 i = 0, j, k;
    int blk;
    bsl_severity_t severity;

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s() start\n nbytes: %d, bytes: 0x"), FUNCTION_NAME(), nbytes));
        for (j = 0; j < nbytes; j++)
        {
            LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%02X "), bytes[j]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "\n")));
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s():  blk_id: %d, cmd: %d, func: %d, offset: %d, nbytes: %d\n"),
                                     FUNCTION_NAME(), blk_id, cmd, func, offset, nbytes));
    }

    rv = mdb_lpm_translate_blk_func_offset_to_mem_reg(unit, blk_id, func, offset, &mem, &reg, &array_index, &blk);
    if (rv != KBP_OK)
    {
        LOG_ERROR(BSL_LOG_MODULE,
                  (BSL_META_U(unit, "%s(), mdb_lpm_translate_blk_func_offset_to_mem_reg failed.\n"), FUNCTION_NAME()));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(): mem: %s, reg: %s, array_index: %d. \n"),
                                 FUNCTION_NAME(), SOC_MEM_NAME(unit, mem), SOC_REG_NAME(unit, reg), array_index));

    if (mem != INVALIDm)
    {
        sal_memset(mem_array, 0, MDB_LPM_MAX_UINT32_WIDTH * sizeof(uint32));

        if (mem == KAPS_RPB_TCAM_CPU_COMMANDm)
        {
            /*
             * Need to pass a read command to the TCAM before we are able to read from it
             */
            mem_array[MDB_LPM_RPB_MAX_UINT32_WIDTH - 1] |= (func << 6);
            rv = soc_mem_array_write(unit, mem, array_index, blk, offset, mem_array);
            if (rv != _SHR_E_NONE)
            {
                LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), soc_mem_array_write failed.\n"), FUNCTION_NAME()));
                return KBP_FATAL_TRANSPORT_ERROR;
            }
        }

        /*
         * Read from memory
         */
        rv = soc_mem_array_read(unit, mem, array_index, blk, offset, mem_array);
        if (rv != _SHR_E_NONE)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), soc_mem_array_read failed.\n"), FUNCTION_NAME()));
            return KBP_FATAL_TRANSPORT_ERROR;
        }

        /*
         * Convert from uint32 to uint8
         */
        if (mem == KAPS_RPB_TCAM_CPU_COMMANDm)
        {
            mem_index = MDB_LPM_RPB_MAX_UINT32_WIDTH - 1;
        }
        else if (mem == KAPS_RPB_ADSm)
        {
            mem_index = MDB_LPM_AD_MAX_UINT32_WIDTH - 1;
        }
        else if ((mem == KAPS_BUCKET_MEMORYm) || (mem == KAPS_BBS_BUCKET_MEMORYm))
        {
            mem_index = MDB_LPM_MAX_UINT32_WIDTH - 2;
        }

        i = 0;
        if (nbytes % 4 > 0)
        {
            for (i = 0; i < nbytes % 4; i++)
            {
                bytes[i] = mem_array[mem_index] >> ((nbytes % 4 - 1 - i) * 8);
            }
            mem_index--;
        }
        for (k = 0; k < nbytes / 4; k++)
        {
            for (j = 0; j < 4; j++)
            {
                bytes[i] = mem_array[mem_index] >> (3 - j) * 8;
                i++;
            }
            mem_index--;
        }

    }
    else if (reg != INVALIDr)
    {
        /*
         * Read from a register
         */
        /*
         * Assuming only registers up to 32 bits
         */
        rv = soc_reg32_get(unit, reg, blk, array_index, &reg_val);
        if (rv != _SHR_E_NONE)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), soc_reg32_get failed.\n"), FUNCTION_NAME()));
            return KBP_FATAL_TRANSPORT_ERROR;
        }
        bytes[0] = reg_val >> 24;
        bytes[1] = reg_val >> 16;
        bytes[2] = reg_val >> 8;
        bytes[3] = reg_val;

    }
    else
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s(), both mem and reg are invalid.\n"), FUNCTION_NAME()));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "%s() end\n"), FUNCTION_NAME()));

    return KBP_OK;
}

kbp_status
mdb_lpm_command(
    void *xpt,
    enum kaps_cmd cmd,
    enum kaps_func func,
    uint32_t blk_nr,
    uint32_t row_nr,
    uint32_t nbytes,
    uint8_t * bytes)
{
    int rv = KBP_OK;
    int unit;

    unit = ((MDB_LPM_XPT *) xpt)->unit;

    switch (cmd)
    {
        case KAPS_CMD_READ:
            rv = mdb_lpm_read_command(xpt, blk_nr, cmd, func, row_nr, nbytes, bytes);
            break;

        case KAPS_CMD_WRITE:
            rv = mdb_lpm_write_command(xpt, blk_nr, cmd, func, row_nr, nbytes, bytes);
            break;

        case KAPS_CMD_EXTENDED:
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "%s():  IBC interface disabled, redundant command: %d\n"), FUNCTION_NAME(),
                       cmd));
            break;

        default:
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "%s():  unsupported cmd: %d\n"), FUNCTION_NAME(), cmd));
            rv = KBP_FATAL_TRANSPORT_ERROR;
            break;
    }

    return rv;
}

shr_error_e
mdb_lpm_xpt_init(
    int unit,
    void **xpt)
{
    MDB_LPM_XPT *xpt_p;

    SHR_FUNC_INIT_VARS(unit);

    SHR_ALLOC(*xpt, sizeof(MDB_LPM_XPT), "kaps_xpt", "%s%s%s\r\n", EMPTY, EMPTY, EMPTY);
    if (*xpt == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_MEMORY, " Error:  SHR_ALLOC for xpt Failed\n");
    }

    xpt_p = (MDB_LPM_XPT *) * xpt;

    xpt_p->mdb_lpm_xpt.device_type = KBP_DEVICE_KAPS;

    xpt_p->mdb_lpm_xpt.kaps_search = mdb_lpm_search;
    xpt_p->mdb_lpm_xpt.kaps_register_read = mdb_lpm_register_read;
    xpt_p->mdb_lpm_xpt.kaps_command = mdb_lpm_command;
    xpt_p->mdb_lpm_xpt.kaps_register_write = mdb_lpm_register_write;

    xpt_p->unit = unit;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
mdb_lpm_entry_search(
    int unit,
    int core,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_t * entry)
{
    uint32 mem_cmd[MDB_LPM_MAX_UINT32_WIDTH], mem_reply[MDB_LPM_MAX_UINT32_WIDTH];
    bsl_severity_t severity;
    mdb_kaps_ip_db_id_e mdb_db_idx;
    uint32 mem_field;
    int prefix_length;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * mdb_db_idx translates directly to the array_index
     */
    SHR_IF_ERR_EXIT(mdb_lpm_dbal_to_db(unit, dbal_physical_table_id, &mdb_db_idx));

    sal_memset(mem_cmd, 0, MDB_LPM_MAX_UINT32_WIDTH * sizeof(uint32));
    sal_memset(mem_reply, 0, MDB_LPM_MAX_UINT32_WIDTH * sizeof(uint32));

    sal_memcpy(mem_cmd, entry->key, WORDS2BYTES(MDB_LPM_RPB_MAX_UINT32_WIDTH - 1));
    /*
     * stamp the app_id to the msb
     */
    mem_cmd[MDB_LPM_RPB_MAX_UINT32_WIDTH - 2] |= app_id << (SAL_UINT32_NOF_BITS - MDB_KAPS_KEY_PREFIX_LENGTH);

    mem_field = MDB_LPM_CMD_SEARCH;
    soc_mem_field_set(unit, KAPS_RPB_TCAM_CPU_CMDm, mem_cmd, TCAM_CPU_CMD_FUNCf, &mem_field);

    SHR_IF_ERR_EXIT(WRITE_KAPS_RPB_TCAM_CPU_COMMANDm(unit, mdb_db_idx, KAPS_MI_BLOCK(unit, core), 0, mem_cmd));

    SHR_IF_ERR_EXIT(READ_KAPS_RPB_TCAM_CPU_COMMANDm(unit, mdb_db_idx, KAPS_MI_BLOCK(unit, core), 0, mem_reply));

    /*
     * The payload should be in [57:38] and the length in [37:30]
     */
    entry->payload[0] = (mem_reply[1] >> 6) & ((1 << MDB_KAPS_AD_WIDTH_IN_BITS) - 1);
    prefix_length = (int) (mem_reply[0] >> 30 | ((mem_reply[1] & 0x3F) << 2));
    SHR_IF_ERR_EXIT(mdb_lpm_prefix_len_to_mask(unit, prefix_length, entry));

    SHR_GET_SEVERITY_FOR_MODULE(severity);
    if (severity >= bslSeverityVerbose)
    {
        int print_index;
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_search: start\n")));
        for (print_index = MDB_LPM_RPB_MAX_UINT32_WIDTH - 1; print_index >= 0; print_index--)
        {
            LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mem_cmd[%d]: %08X.\n"), print_index, mem_cmd[print_index]));
        }
        for (print_index = MDB_LPM_RPB_MAX_UINT32_WIDTH - 1; print_index >= 0; print_index--)
        {
            LOG_VERBOSE(BSL_LOG_MODULE,
                        (BSL_META_U(unit, "mem_reply[%d]: %08X.\n"), print_index, mem_reply[print_index]));
        }
        LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "mdb_lpm_entry_search: end\n")));
    }

exit:
    SHR_FUNC_EXIT;
}

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
