/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    access_pack.c
 * Purpose: Miscellaneous routine for device db access
 */

#include <appl/diag/system.h>

#include <sal/appl/io.h>
#include <sal/core/libc.h>
#include <sal/appl/sal.h>
#include <soc/defs.h>
#include <soc/drv.h>

#include <shared/bitop.h>

#ifdef BCM_SAND_SUPPORT
#include <shared/dbx/dbx_xml.h>
#include <shared/dbx/dbx_file.h>
#include <shared/utilex/utilex_rhlist.h>
#include <shared/utilex/utilex_str.h>

#include <soc/shared/access_pack.h>
#include <soc/shared/sand_signals.h>

#include <bcm/types.h>

#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/dpp_config_defs.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_diag.h>
#endif

#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/dnx_data/dnx_data_device.h>
#endif

#define BSL_LOG_MODULE BSL_LS_SOC_COMMON

/* Initialized by device_init */
static device_t *sand_device_array[SOC_MAX_NUM_DEVICES];

static shr_reg_data_t reg_data[NUM_SOC_REG] = {
        {0}
};

static shr_mem_data_t mem_data[NUM_SOC_MEM] = {
        {0}
};


int
shr_access_reg_no_read_get(
        int         unit,
        soc_reg_t   reg)
{
    if(reg < 0 || reg >= NUM_SOC_REG)
    {
        /* No such register in list - means no limitations on read */
        return 0;
    }

    return ((reg_data[reg].flags & ACC_NO_READ) ? 1 : 0);
}

int
shr_access_mem_no_read_get(
        int         unit,
        soc_mem_t   mem)
{
    if(mem < 0 || mem >= NUM_SOC_MEM)
    {
        /* No such register in list - means no limitations on read */
        return 0;
    }

    /* By default all explicitly uninitialized fields will be 0, so nothing happens, object may be read */
    return ((mem_data[mem].flags & ACC_NO_READ) ? 1 : 0);
}

int
shr_access_reg_no_wb_get(
        int         unit,
        soc_reg_t   reg)
{
    if(reg < 0 || reg >= NUM_SOC_REG)
    {
        /* No such register in list - means no limitations on read */
        return 0;
    }

    return ((reg_data[reg].flags & ACC_NO_WB) ? 1 : 0);
}

int
shr_access_mem_no_wb_get(
        int         unit,
        soc_mem_t   mem)
{
    if(mem < 0 || mem >= NUM_SOC_MEM)
    {
        /* No such register in list - means no limitations on read */
        return 0;
    }

    /* By default all explicitly uninitialized fields will be 0, so nothing happens, object may be read */
    return ((mem_data[mem].flags & ACC_NO_WB) ? 1 : 0);
}

#ifdef CMODEL_SERVER_MODE
#include <soc/dnx/cmodel/cmodel_reg_access.h>

typedef struct {
    char*  start; /* Don't modify - keep to use for sal_free */
    char*  data;
    uint32 length;
    int    nof;
} cmodel_info_t;

#define CMODEL_INFO_GET_UINT32(mc_var, mc_info)                                                             \
                if((mc_info)->length >= sizeof(uint32))                                                     \
                {                                                                                           \
                    mc_var = bcm_ntohl(*((uint32 *)(mc_info)->data));                                       \
                    (mc_info)->length -= sizeof(uint32); (mc_info)->data += sizeof(uint32);                 \
                }                                                                                           \
                else                                                                                        \
                {                                                                                           \
                    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "Request 4(uint32) vs %u\n", (mc_info)->length);         \
                }

#define CMODEL_INFO_GET_MEM(mc_mem, mc_mem_length, mc_info)                                                 \
                if(mc_mem_length > DSIG_MAX_SIZE_STR)                                                       \
                {                                                                                           \
                    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "Signal size:%u is more than maximum supported:%u\n",    \
                                                            mc_mem_length, DSIG_MAX_SIZE_STR);              \
                }                                                                                           \
                if((mc_info)->length >= mc_mem_length)                                                      \
                {                                                                                           \
                    sal_memcpy(mc_mem, (mc_info)->data, mc_mem_length);                                     \
                    (mc_info)->length -= mc_mem_length; (mc_info)->data += mc_mem_length;                   \
                }                                                                                           \
                else                                                                                        \
                {                                                                                           \
                    SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "Signal size:%u is more than buffer available:%u\n",     \
                                                                mc_mem_length, (mc_info)->length);          \
                }

static shr_error_e
sand_cmodel_get_signals(
        int unit,
        pp_block_t *cur_pp_block)
{
    cmodel_info_t *cmodel_info;
    int i_st;
    debug_signal_t *debug_signal;
    pp_stage_t *cur_pp_stage;
    uint32 name_length;

    SHR_FUNC_INIT_VARS(unit);

    if((cmodel_info  = utilex_alloc(sizeof(cmodel_info_t) * cur_pp_block->stage_num)) == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_MEMORY, "Memory allocation of:%d failed for cmodel info\n",
                                                         sizeof(char *) * cur_pp_block->stage_num);
    }
    cur_pp_block->signal_num = 0;
    for(i_st = 0; i_st < cur_pp_block->stage_num; i_st++)
    {
        cur_pp_stage = &cur_pp_block->stages[i_st];
        SHR_IF_ERR_EXIT(cmodel_get_signal(unit, cur_pp_stage->id, &cmodel_info[i_st].length, &cmodel_info[i_st].start));
        if((cmodel_info[i_st].length == 0) || (cmodel_info[i_st].start == NULL))
        {
            LOG_WARN(BSL_LS_SOC_COMMON, (BSL_META("No signal info received from cmodel for stage:%s\n"), cur_pp_stage->name));
            continue;
        }
        cmodel_info[i_st].data = cmodel_info[i_st].start;
        /*
         * First 4 bytes of buffer represent number of signals
         */
        CMODEL_INFO_GET_UINT32(cmodel_info[i_st].nof, &cmodel_info[i_st]);
        cur_pp_block->signal_num += cmodel_info[i_st].nof;
    }

    if(cur_pp_block->debug_signals != NULL)
    {
        utilex_free(cur_pp_block->debug_signals);
        cur_pp_block->debug_signals = NULL;
    }

    if(cur_pp_block->signal_num == 0)
    {
        SHR_ERR_EXIT(_SHR_E_NONE, "No signals obtained from cmodel\n");
    }

    if((debug_signal = utilex_alloc(sizeof(debug_signal_t) * cur_pp_block->signal_num)) == NULL)
    {
        SHR_ERR_EXIT(_SHR_E_MEMORY, "Memory allocation of:%d failed for debug signals\n",
                                                        sizeof(debug_signal_t) * cur_pp_block->signal_num);
    }

    cur_pp_block->debug_signals = debug_signal;
    /*
     * Now traverse the buffer and extract stage names and id
     */
    for(i_st = 0; i_st < cur_pp_block->stage_num; i_st++)
    {
        int i_sig, org_length;
        char signal_str[DSIG_MAX_SIZE_STR + 1];
        cur_pp_stage = &cur_pp_block->stages[i_st];
        org_length = cmodel_info[i_st].length;
        for(i_sig = 0;  i_sig < cmodel_info[i_st].nof; i_sig++)
        {
            CMODEL_INFO_GET_UINT32(name_length, &cmodel_info[i_st]);
            CMODEL_INFO_GET_MEM(&debug_signal->attribute, name_length, &cmodel_info[i_st]);
            CMODEL_INFO_GET_UINT32(debug_signal->size, &cmodel_info[i_st]);
            sal_memset(signal_str, 0, DSIG_MAX_SIZE_STR + 1);
            CMODEL_INFO_GET_MEM(signal_str, (debug_signal->size + 3) / 4 + 2, &cmodel_info[i_st]);

            parse_long_integer(debug_signal->value, DSIG_MAX_SIZE_UINT32, signal_str);
            if(i_st != cur_pp_block->stage_num - 1) {
                sal_strcpy(debug_signal->to, cur_pp_block->stages[i_st + 1].name);
            }
            sal_strcpy(debug_signal->from,    cur_pp_stage->name);
            sal_strcpy(debug_signal->block_n, cur_pp_block->name);
            sal_sprintf(debug_signal->hw, "%s_%s", cur_pp_stage->name, debug_signal->attribute);
            debug_signal++;
        }
        if(cmodel_info[i_st].length != 0)
        {
            LOG_WARN(BSL_LS_SOC_COMMON, (BSL_META("%d bytes left in %s buffer\n"), cmodel_info[i_st].length, cur_pp_stage->name));
        }
    }

exit:
    if(cmodel_info != NULL)
    {
        for(i_st = 0; i_st < cur_pp_block->stage_num; i_st++)
        {
            if(cmodel_info[i_st].start != NULL)
                sal_free(cmodel_info[i_st].start);
        }
        utilex_free(cmodel_info);
    }
    SHR_FUNC_EXIT;
}

static shr_error_e
sand_cmodel_get_stages(
        int unit,
        pp_block_t *cur_pp_block)
{
    cmodel_info_t stage_info_m;
    int i_ms, name_length;
    pp_stage_t *cur_pp_stage;

    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(cmodel_get_block_names(unit, (int *)&stage_info_m.length, &stage_info_m.start));
    if((stage_info_m.start == NULL) || (stage_info_m.length == 0))
    {
        SHR_ERR_EXIT(_SHR_E_NOT_FOUND, "CMODEL Returned NULL buffer\n");
    }
    stage_info_m.data = stage_info_m.start;
    /*
     * First 4 bytes of buffer represent number of ms
     */
    CMODEL_INFO_GET_UINT32(stage_info_m.nof, &stage_info_m);
    cur_pp_block->stages = utilex_alloc(sizeof(pp_stage_t) * stage_info_m.nof);
    /*
     * Now traverse the buffer and extract stage names and id
     */
    for(i_ms = 0; (i_ms < stage_info_m.nof) && (stage_info_m.length > sizeof(uint32) * 2); i_ms++)
    {
        if (cur_pp_block->stage_num == stage_info_m.nof)
            break;
        cur_pp_stage = &cur_pp_block->stages[cur_pp_block->stage_num++];
        CMODEL_INFO_GET_UINT32(cur_pp_stage->id,             &stage_info_m);
        CMODEL_INFO_GET_UINT32(name_length,                  &stage_info_m);
        CMODEL_INFO_GET_MEM(cur_pp_stage->name, name_length, &stage_info_m);
    }
    if(stage_info_m.length != 0)
    {
        LOG_WARN(BSL_LS_SOC_COMMON, (BSL_META("%d bytes left in stages buffer\n"), stage_info_m.length));
    }
exit:
    if(stage_info_m.start != NULL)
        sal_free(stage_info_m.start);
    SHR_FUNC_EXIT;
}

#endif /* CMODEL_SERVER_MODE */

static sigstruct_t *
sand_signal_struct_get(
    device_t * device,
    char *signal_n)
{
    int i_struct;
    sigstruct_t *cur_sigstruct;

    if (device->sigstructs == NULL)
    {  /* Not initialized yet */
        cli_out("Signal Struct DB was not initialized\n");
        return NULL;
    }

    for (i_struct = 0; i_struct < device->sigstruct_num; i_struct++)
    {
        cur_sigstruct = &device->sigstructs[i_struct];
        if (!sal_strcasecmp(cur_sigstruct->name, signal_n))
            return cur_sigstruct;
    }

    return NULL;
}

static sigparam_t *
sand_signal_resolve_get(
    device_t * device,
    char *signal_n)
{
    int i_par;
    sigparam_t *cur_sigparam;

    if (device->sigparams == NULL)
    {   /* Not initialized yet */
        cli_out("Signal Struct DB was not initialized\n");
        return NULL;
    }

    for (i_par = 0; i_par < device->sigparam_num; i_par++)
    {
        cur_sigparam = &device->sigparams[i_par];
        if (!sal_strcasecmp(cur_sigparam->name, signal_n))
            return cur_sigparam;
    }

    return NULL;
}

static int
sand_signal_param_get(
    device_t * device,
    char *signal_n,
    uint32 value,
    char *value_n,
    int show_value)
{
    int res = _SHR_E_NOT_FOUND;
    int i, k;
    sigparam_t *cur_sigparam;
    sigparam_value_t *cur_sigparam_value;

    if (device->sigstructs == NULL)
    {   /* Not initialized yet */
        cli_out("Signal Struct DB was not initialized\n");
        return res;
    }

    for (i = 0; i < device->sigparam_num; i++)
    {
        cur_sigparam = &device->sigparams[i];
        if (sal_strcasecmp(cur_sigparam->name, signal_n))
            continue;

        /*
         * Verify that value is suitable for signal resolution size
         */
        if (value >= (1 << cur_sigparam->size))
        {
            cli_out("Value:%d excess signal:%s size:%d\n", value, signal_n, cur_sigparam->size);
            break;
        }

        for (k = 0; k < cur_sigparam->value_num; k++)
        {
            cur_sigparam_value = &cur_sigparam->values[k];
            if (value == cur_sigparam_value->value)
            {
                if (show_value == 1)
                    sal_sprintf(value_n, "%s(%d)", cur_sigparam_value->name, value);
                else
                    sal_sprintf(value_n, "%s", cur_sigparam_value->name);
                res = _SHR_E_NONE;
                break;
            }
        }

        if ((res != _SHR_E_NONE) && !ISEMPTY(cur_sigparam->default_str))        /* Copy default param name if not found
                                                                                 */
        {
            sal_sprintf(value_n, "%s(%d)", cur_sigparam->default_str, value);
            res = _SHR_E_NONE;
        }

        break;
    }

    return res;
}

int
sand_signal_address_parse(
    char *full_address,
    signal_address_t * address)
{
    char **tokens;
    uint32 realtokens = 0;
    int i_tok;

    if ((tokens = utilex_str_split(full_address, "|", DSIG_MAX_ADDRESS_RANGE_NUM, &realtokens)) == NULL)
    {
        return 0;
    }

    for (i_tok = 0; i_tok < realtokens; i_tok++)
    {
        sscanf(tokens[i_tok], " {15'd%d,16'd%d} bits: [%d : %d]", &address->high, &address->low, &address->msb,
               &address->lsb);
        address++;
    }

    utilex_str_split_free(tokens, realtokens);

    return realtokens;
}

static int
sand_signal_range_parse(
    char *bit_range,
    int *startbit_p,
    int *endbit_p)
{
    int startbit = -1, endbit = -1;
    if (strstr(bit_range, ":") == NULL)
    {
        sscanf(bit_range, "%d", &endbit);
        startbit = endbit;
    }
    else
        sscanf(bit_range, "%d:%d", &endbit, &startbit);

    if ((endbit < startbit) || (startbit >= DSIG_MAX_SIZE_BITS) || (endbit >= DSIG_MAX_SIZE_BITS) || (startbit < 0)
        || (endbit < 0))
        return _SHR_E_INTERNAL;

    if (startbit <= endbit)
    {
        *startbit_p = startbit;
        *endbit_p = endbit;
    }
    else
    {
        *startbit_p = endbit;
        *endbit_p = startbit;
    }

    return _SHR_E_NONE;
}

void
sand_signal_struct_expand_init(
    device_t * device)
{
    int i, j;
    sigstruct_t *cur_sigstruct;
    sigstruct_field_t *cur_sigstruct_field;

    if (device->sigstructs == NULL)
    {
        /*
         * Not initialized yet
         */
        cli_out("Signal Struct DB was not initialized\n");
        return;
    }

    /*
     * Go through field and fill expansion were available
     */
    for (i = 0; i < device->sigstruct_num; i++)
    {
        cur_sigstruct = &device->sigstructs[i];
        for (j = 0; j < cur_sigstruct->field_num; j++)
        {
            cur_sigstruct_field = &(cur_sigstruct->fields[j]);
            if (!ISEMPTY(cur_sigstruct_field->expansion_m.name))
            {
                /*
                 * If expansion is not empty and not dynamic, find the expansion, if failed zero the field
                 */
                if (sal_strcasecmp(cur_sigstruct_field->expansion_m.name, "Dynamic"))
                    if (sand_signal_struct_get(device, cur_sigstruct_field->expansion_m.name) == NULL)
                    {
                        cli_out("No signal expansion:%s\n", cur_sigstruct_field->expansion_m.name);
                        SET_EMPTY(cur_sigstruct_field->expansion_m.name);
                    }
            }
            else
            {
                /*
                 * When expansion is empty first check may be it can be expanded by name
                 */
                if (sand_signal_struct_get(device, cur_sigstruct_field->name) != NULL)
                    strcpy(cur_sigstruct_field->expansion_m.name, cur_sigstruct_field->name);
                /*
                 * Not found check resolution
                 */
                else if (!ISEMPTY(cur_sigstruct_field->resolution))
                {
                    /*
                     * If resolution is not empty look if it exists
                     */
                    if (sand_signal_resolve_get(device, cur_sigstruct_field->resolution) == NULL)
                    {
                        cli_out("No signal resolution:%s\n", cur_sigstruct_field->resolution);
                        SET_EMPTY(cur_sigstruct_field->resolution);
                    }
                }
                else
                {
                    /*
                     * When resolution is empty check may be it can be expanded by name
                     */
                    if (sand_signal_resolve_get(device, cur_sigstruct_field->name) != NULL)
                        /*
                         * If there is match - attribute serves as resolution
                         */
                        strcpy(cur_sigstruct_field->resolution, cur_sigstruct_field->name);
                }
            }
        }
    }

    return;
}


void
sand_signal_list_free(
    rhlist_t * output_list)
{
    signal_output_t *signal_output;
    RHITERATOR(signal_output, output_list)
    {
        if (signal_output->field_list)
        {
            sand_signal_list_free(signal_output->field_list);
        }
    }
    utilex_rhlist_free_all(output_list);
}

static void
sand_signal_get_value(
    int unit,
    int core_id,
    debug_signal_t * debug_signal,
    uint32 * value)
{
#ifdef CMODEL_SERVER_MODE
    sal_memcpy(value, debug_signal->value, DSIG_MAX_SIZE_BYTES);
#else
#ifdef BCM_PETRA_SUPPORT
    int j;
    ARAD_PP_DIAG_REG_FIELD debug_field;
    signal_address_t *address;
    uint32 range_val[ARAD_PP_DIAG_DBG_VAL_LEN];
    int range_size;
    int last_pos;
    int uint_num;

    uint_num = BITS2WORDS(debug_signal->size);
    if (uint_num > DSIG_MAX_SIZE_UINT32)
    {
        cli_out("Bad signal_size:%d\n", debug_signal->size);
        uint_num = DSIG_MAX_SIZE_UINT32;
    }

    sal_memset(value, 0, sizeof(int) * uint_num);

    last_pos = 0;
    for (j = 0; j < debug_signal->range_num; j++)
    {
        address = &(debug_signal->address[j]);
        debug_field.base = (address->high << 16) + address->low;
        debug_field.lsb = address->lsb;
        debug_field.msb = address->msb;
        range_size = debug_field.msb + 1 - debug_field.lsb;
        arad_pp_diag_dbg_val_get_unsafe(unit, core_id, debug_signal->block_id, &debug_field, range_val);
        SHR_BITCOPY_RANGE(value, last_pos, range_val, 0, range_size);
        last_pos += range_size;
    }
#endif /* BCM_PETRA_SUPPORT */
#endif
    return;
}

static void
dsig_buffer_to_str(
    device_t * device,
    char *resolution_n,
    uint32 * org_source,
    char *dest,
    int bit_size,
    int byte_order)
{
    int i, j;
    int byte_size, long_size;
    int dest_byte_index, source_byte_index, real_byte_index;
    uint8 *source = (uint8 *) org_source;

    byte_size = BITS2BYTES(bit_size);
    long_size = BYTES2WORDS(byte_size);
    sal_memset(dest, 0, DSIG_MAX_SIZE_STR);

    if (!ISEMPTY(resolution_n) && (sand_signal_param_get(device, resolution_n, *org_source, dest, 1) == _SHR_E_NONE))
        return;

    dest_byte_index = 0;
#ifdef BE_HOST
    if (byte_order == PRINT_LITTLE_ENDIAN)
    {
        for (i = 0; i < long_size; i++)
        {
            for (j = 0; j < 4; j++)
            {
                source_byte_index = 4 * i + 3 - j;
                real_byte_index = 4 * i + j;
                if (real_byte_index >= byte_size)
                    continue;
                sal_sprintf(&dest[2 * dest_byte_index], "%02x", source[source_byte_index]);
                dest_byte_index++;
            }
        }
    }
    else
    {
        for (i = 0; i < long_size; i++)
        {
            for (j = 0; j < 4; j++)
            {
                source_byte_index = 4 * (long_size - 1 - i) + j;
                real_byte_index = 4 * (long_size - 1 - i) + 3 - j;
                if (real_byte_index >= byte_size)
                    continue;
                sal_sprintf(&dest[2 * dest_byte_index], "%02x", source[source_byte_index]);
                dest_byte_index++;
            }
        }
    }
#else
    if (byte_order == PRINT_LITTLE_ENDIAN)
    {
        for (i = 0; i < long_size; i++)
        {
            for (j = 0; j < 4; j++)
            {
                source_byte_index = 4 * i + 3 - j;
                real_byte_index = 4 * i + 3 - j;
                if (real_byte_index >= byte_size)
                    continue;
                sal_sprintf(&dest[2 * dest_byte_index], "%02x", source[source_byte_index]);
                dest_byte_index++;
            }
        }
    }
    else
    {
        for (i = 0; i < long_size; i++)
        {
            for (j = 0; j < 4; j++)
            {
                source_byte_index = 4 * (long_size - 1 - i) + 3 - j;
                real_byte_index = 4 * (long_size - 1 - i) + 3 - j;
                if (real_byte_index >= byte_size)
                    continue;
                sal_sprintf(&dest[2 * dest_byte_index], "%02x", source[source_byte_index]);
                dest_byte_index++;
            }
        }
    }
#endif
}

static void
dsig_print_description(
    match_t * match_p)
{
    cli_out("Unknown Signal ");
    if (!ISEMPTY(match_p->name))
        cli_out(":%s ", match_p->name);
    if (!ISEMPTY(match_p->block))
        cli_out("block:%s ", match_p->block);
    if (!ISEMPTY(match_p->from))
        cli_out("from:%s ", match_p->from);
    if (!ISEMPTY(match_p->to))
        cli_out("to:%s ", match_p->to);
    cli_out("\n");
}

/*
 * Copy field_size bits from "signal_value" with offset "field_offset" to field_value with size "field_size"
 */
static void
sand_signal_field_get_value(
    uint32 * signal_value,
    uint32 * field_value,
    int field_offset,
    int field_size)
{
    SHR_BITCOPY_RANGE(field_value, 0, signal_value, field_offset, field_size);
}

static int
sand_signal_field_get(
    device_t * device,
    char *sigstruct_n,
    uint32 * signal_value,
    char *field_name,
    char *field_str,
    uint32 * field_value)
{
    sigstruct_t *sigstruct;
    sigstruct_field_t *sigstruct_field;
    int i;
    int res = _SHR_E_NOT_FOUND;
    uint32 cur_field_value[DSIG_MAX_SIZE_UINT32];

    /*
     * Split by dot into struct and lower field, but only once, so maximum tokens = 2
     */
    char **tokens;
    uint32 realtokens = 0;

    if ((tokens = utilex_str_split(field_name, ".", 2, &realtokens)) == NULL)
    {
        return res;
    }

    if (!ISEMPTY(sigstruct_n) && (sigstruct = sand_signal_struct_get(device, sigstruct_n)) != NULL)
    {
        sigstruct_field = sigstruct->fields;
        for (i = 0; i < sigstruct->field_num; i++)
        {
            /*
             * We are looking for first token resolution
             */
            if (!sal_strcasecmp(tokens[0], sigstruct_field->name))
            {
                sal_memset(cur_field_value, 0, DSIG_MAX_SIZE_BYTES);
                /*
                 * Obtain the value
                 */
                sand_signal_field_get_value(signal_value, cur_field_value, sigstruct_field->start_bit, sigstruct_field->size);
                /*
                 * If this is the last token - obtain string value
                 */
                if (realtokens == 1)
                {
                    sal_memcpy(field_value, cur_field_value, DSIG_MAX_SIZE_BYTES);
                    if (field_str != NULL)
                    {
                        res = sand_signal_param_get(device, sigstruct_field->name, *cur_field_value, field_str, 0);
                        if (res != _SHR_E_NONE)
                            sal_sprintf(field_str, "%u", *cur_field_value);
                    }
                    res = _SHR_E_NONE;
                }
                else
                {
                    res = sand_signal_field_get(device, sigstruct_field->expansion_m.name, cur_field_value, tokens[1],
                                         field_str, field_value);
                }
                break;
            }
            sigstruct_field++;
        }
    }

    utilex_str_split_free(tokens, realtokens);

    return res;
}

static sigstruct_t *
sand_signal_expand_get(
    device_t * device,
    char *sigstruct_n,
    uint32 * value,
    expansion_t * expansion)
{
    int i, n;
    expansion_option_t *option;
    char field_str[RHNAME_MAX_SIZE];
    uint32 field_value[DSIG_MAX_SIZE_UINT32];
    int match_flag;
    sigstruct_t *sigstruct = NULL;
    uint32 rt_value, db_value;

    if (ISEMPTY(expansion->name))       /* No explicit expansion */
    {
        goto exit;
    }
    else if (sal_strcasecmp(expansion->name, "dynamic"))
    {
        /** static expansion */
        sigstruct = sand_signal_struct_get(device, expansion->name);
        goto exit;
    }

    /*
     * Dynamic expansion
     */
    option = expansion->options;
    for (i = 0; i < expansion->option_num; i++)
    {
        match_flag = TRUE;
        n = 0;
        while ((n < DSIG_OPTION_PARAM_MAX_NUM) && !ISEMPTY(option->param[n].name))
        {
            if (sand_signal_field_get(device, sigstruct_n, value, option->param[n].name, field_str, field_value) !=
                _SHR_E_NONE)
            {
                match_flag = FALSE;
                break;
            }
            if (sal_strcasecmp(option->param[n].value, field_str))
            {
                if ((utilex_str_stoul(field_str, &rt_value) != _SHR_E_NONE)
                    || (utilex_str_stoul(option->param[n].value, &db_value) != _SHR_E_NONE) || (rt_value != db_value))
                {
                    match_flag = FALSE;
                    break;
                }
            }

            n++;
        }
        if (match_flag == TRUE)
        {       /* Found our expansion leave option loop */
            sigstruct = sand_signal_struct_get(device, option->name);
            break;
        }
        option++;
    }
exit:
    return sigstruct;
}

static int
sand_signal_expand(
    device_t * device,
    uint32 * signal_value,
    char *sigstruct_n,
    int byte_order,
    char *match_n,
    int flags,
    rhlist_t ** field_list_p)
{
    int valid = 0;              /* By default no match - we yet need to find one */
    sigstruct_t *sigstruct, *sigstruct_exp, *sigstruct_exp4field;
    sigstruct_field_t *sigstruct_field;
    int i;
    uint32 field_value[DSIG_MAX_SIZE_UINT32];
    char *field_dyn_name;
    signal_output_t *field_output;
    int top_valid, child_valid;
    char *cur_match_n;
    rhhandle_t temp = NULL;

    rhlist_t *field_list = NULL, *child_list = NULL;

    char **tokens;
    uint32 realtokens = 0, maxtokens = 2;

    tokens = utilex_str_split(match_n, ".", maxtokens, &realtokens);
    if (realtokens && (tokens == NULL))
    {
        goto exit;
    }

    if (realtokens == maxtokens)
    {
        match_n = tokens[0];
    }

    /*
     * Check if there is an expansion at all
     */
    if (ISEMPTY(sigstruct_n) || (sigstruct = sand_signal_struct_get(device, sigstruct_n)) == NULL)
        goto exit;

    /*
     * In case of dynamic expansion find the matching expansion and switch to it
     */
    if ((sigstruct_exp = sand_signal_expand_get(device, sigstruct_n, signal_value, &sigstruct->expansion_m)) != NULL)
        sigstruct = sigstruct_exp;

    for (i = 0, sigstruct_field = sigstruct->fields; i < sigstruct->field_num; i++, sigstruct_field++)
    {
        child_valid = 0;
        child_list = NULL;
        /*
         * If there is a condition check that it is fulfilled
         */
        if (!(flags & SIGNALS_MATCH_NOCOND) && !ISEMPTY(sigstruct_field->cond_attribute))
        {
            sand_signal_field_get(device, sigstruct->name, signal_value, sigstruct_field->cond_attribute, NULL, field_value);
            if (0 == VALUE(field_value))
                continue;
        }
        /*
         * Check for further usage dynamic expansion for field
         * In case of dynamic expansion find the matching expansion and switch to it - in this case for field
         */
        sigstruct_exp4field = sand_signal_expand_get(device, sigstruct->name, signal_value, &sigstruct_field->expansion_m);
        if (sigstruct_exp4field)
            field_dyn_name = sigstruct_exp4field->name;
        else
            field_dyn_name = sigstruct_field->expansion_m.name;

        sal_memset(field_value, 0, DSIG_MAX_SIZE_BYTES);
        sand_signal_field_get_value(signal_value, field_value, sigstruct_field->start_bit, sigstruct_field->size);

        if ((match_n == NULL)   /* No match string on input */
            || ((flags & SIGNALS_MATCH_EXACT) && !sal_strcasecmp(sigstruct_field->name, match_n))
            || (!(flags & SIGNALS_MATCH_EXACT) && (sal_strcasestr(sigstruct_field->name, match_n) != NULL)))
            top_valid = 1;
        else
            top_valid = 0;

        if ((top_valid == 1) && (realtokens == maxtokens))
        {
            /*
             * Look into subfield for match, above match is not enough
             */
            top_valid = 0;
            cur_match_n = tokens[1];
        }
        else
        {
            cur_match_n = top_valid ? NULL : match_n;
        }

        if (sigstruct_exp4field)
        {
            child_valid = sand_signal_expand(device, field_value, sigstruct_exp4field->name, byte_order,
                                      cur_match_n, flags, &child_list);
        }

        if ((top_valid == 0) && (child_valid == 0))
        {
            continue;
        }

        valid = 1;      /* Once we have match return will be non zero */

        /*
         * Allocate structure for the child
         */
        if (field_list == NULL)
        {
            if ((field_list = utilex_rhlist_create("field_list", sizeof(signal_output_t), 0)) == NULL)
                goto exit;
        }

        /*
         * Now we can allocate output and fill it
         */
        if (utilex_rhlist_entry_add_tail(field_list, NULL, RHID_TO_BE_GENERATED, &temp) != _SHR_E_NONE)
        {
            /*
             * No more place any more - return with what you have until now
             */
            goto exit;
        }
        field_output = temp;

        strcpy(RHNAME(field_output), sigstruct_field->name);

        strcpy(field_output->expansion, field_dyn_name);

        memcpy(field_output->value, field_value, DSIG_MAX_SIZE_BYTES);

        dsig_buffer_to_str(device, sigstruct_field->resolution, field_value, field_output->print_value,
                           sigstruct_field->size, byte_order);
        field_output->field_list = child_list;
        field_output->size = sigstruct_field->size;
        if (flags & SIGNALS_MATCH_ONCE)
        {
            /*
             * No need to look anymore
             */
            goto exit;
        }
    }

exit:
    *field_list_p = field_list;
    utilex_str_split_free(tokens, realtokens);
    return valid;
}

shr_error_e
sand_signal_list_get(
    device_t * device,
    int unit,
    int core,
    match_t * match_p,
    rhlist_t * dsig_list)
{
    shr_error_e rv = _SHR_E_NONE;
    int i, j;
    char *match_n;

    pp_block_t *cur_pp_block;
    debug_signal_t *debug_signals;
    signal_output_t *signal_output = NULL;
    rhhandle_t temp = NULL;

    int top_valid, child_valid;
    /*
     * Dynamic Data
     */
    uint32 value[DSIG_MAX_SIZE_UINT32];
    char *cur_match_n;
    rhlist_t *field_list = NULL;

    char **tokens = NULL;
    uint32 realtokens = 0, maxtokens = 2;

    if (device->pp_blocks == NULL)
    {
        LOG_ERROR(_SHR_E_INIT, (BSL_META("Signal DB for:%s was not initialized\n"), SOC_CHIP_STRING(unit)));
        rv = _SHR_E_INTERNAL;
        goto exit;
    }
    /*
     * Zero the "value" just to be sure than in any case of problem it will be null string
     */
    memset(value, 0, sizeof(uint32) * DSIG_MAX_SIZE_UINT32);

    match_n = match_p->name;

    tokens = utilex_str_split(match_n, ".", maxtokens, &realtokens);
    if (realtokens && (tokens == NULL))
    {
        LOG_ERROR(_SHR_E_INIT, (BSL_META("Problem parsing match string\n")));
        rv = _SHR_E_INTERNAL;
        goto exit;
    }

    if (realtokens == maxtokens)
    {
        match_n = tokens[0];
    }

    for (i = 0; i < device->block_num; i++)
    {
        cur_pp_block = &device->pp_blocks[i];

        if (!ISEMPTY(match_p->block) && sal_strcasecmp(cur_pp_block->name, match_p->block))
            continue;

#ifdef CMODEL_SERVER_MODE
        if((rv = sand_cmodel_get_signals(unit, cur_pp_block)) != _SHR_E_NONE)
            goto exit;
#endif
        debug_signals = cur_pp_block->debug_signals;
        for (j = 0; j < cur_pp_block->signal_num; j++)
        {
            if ((match_p->flags & SIGNALS_MATCH_PERM) && (debug_signals[j].perm == 0))
                continue;

            if (!ISEMPTY(match_p->stage))
            {
                /** From == To means we want some stage either as from or to, so e one of conditions need to be true */
                if ((sal_strcasestr(debug_signals[j].from, match_p->stage) == NULL)
                    && (sal_strcasestr(debug_signals[j].to, match_p->stage) == NULL))
                    continue;
            }
            else
            {
                /** Here one of both conditions must be true to compare the attributes */
                if (!ISEMPTY(match_p->from) && (sal_strcasestr(debug_signals[j].from, match_p->from) == NULL))
                    continue;
                if (!ISEMPTY(match_p->to) && (sal_strcasestr(debug_signals[j].to, match_p->to) == NULL))
                    continue;
            }
            if ((match_n == NULL)       /* No match string on input */
                || ((match_p->flags & SIGNALS_MATCH_EXACT) &&   /* Match should be exact one */
                    (!sal_strcasecmp(debug_signals[j].attribute, match_n) || ((match_p->flags & SIGNALS_MATCH_HW) &&
                     !sal_strcasecmp(debug_signals[j].hw, match_n))))
                || (!(match_p->flags & SIGNALS_MATCH_EXACT) &&  /* Match may be any part of name */
                   ((sal_strcasestr(debug_signals[j].attribute, match_n) != NULL) || ((match_p->flags & SIGNALS_MATCH_HW) &&
                     (sal_strcasestr(debug_signals[j].hw, match_n) != NULL)))))
                top_valid = 1;
            else
                top_valid = 0;
            /*
             * If double flag set and no MATCH_DOUBLE skip this signal
             */
            if (!(match_p->flags & SIGNALS_MATCH_DOUBLE) && (debug_signals[j].double_flag == 1))
                continue;
            /*
             * If there is a condition check that it is fulfilled
             */
            if (!(match_p->flags & SIGNALS_MATCH_NOVALUE) &&
                (!(match_p->flags & SIGNALS_MATCH_NOCOND) && debug_signals[j].cond_signal))
            {
                sand_signal_get_value(unit, core, debug_signals[j].cond_signal, value);
                if (debug_signals[j].cond_value != VALUE(value))
                    continue;
            }

            if ((top_valid == 1) && (realtokens == maxtokens))
            {
                /*
                 * Look into subfield for match, above match is not enough
                 */
                top_valid = 0;
                cur_match_n = tokens[1];
            }
            else
            {
                /** If signal found bring all its sub-fields, meaning using NULL match string */
                cur_match_n = top_valid ? NULL : match_n;
            }
            /*
             * Step 1 - Fill signal value
             */
            if (!(match_p->flags & SIGNALS_MATCH_NOVALUE))
                sand_signal_get_value(unit, core, &debug_signals[j], value);
            /*
             * Step 2 - Now start expansion
             */
            if (match_p->flags & SIGNALS_MATCH_EXPAND)
            {
                child_valid = sand_signal_expand(device, value, debug_signals[j].expansion, match_p->output_order,
                                          cur_match_n, match_p->flags, &field_list);
            }
            else
            {
                child_valid = 0;
                field_list = NULL;
            }

            if ((top_valid == 0) && (child_valid == 0)) /* No match in any place */
                continue;
            /*
             * Now we can allocate output and fill it
             */
            if((rv = utilex_rhlist_entry_add_tail(dsig_list, debug_signals[j].attribute, RHID_TO_BE_GENERATED,  &temp)) != _SHR_E_NONE)
            {   /** free already allocated child list, that we have no way to preserve */
                sand_signal_list_free(field_list);
                goto exit;
            }
            signal_output = temp;
            /*
             * Copy debug signal into output_signal
             */
            signal_output->debug_signal = &debug_signals[j];
            /*
             * Create print value from raw one
             */
            if (!(match_p->flags & SIGNALS_MATCH_NOVALUE))
            {
                char *resolution_n;
                if(match_p->flags & SIGNALS_MATCH_EXPAND)
                    resolution_n = debug_signals[j].resolution;
                else
                    resolution_n = NULL;
                dsig_buffer_to_str(device, resolution_n, value, signal_output->print_value,
                                                                        debug_signals[j].size, match_p->output_order);
                memcpy(signal_output->value, value, DSIG_MAX_SIZE_BYTES);
            }
            /*
             * Fill pointer to structure fields
             */
            signal_output->size = debug_signals[j].size;
            signal_output->core = core;
            signal_output->field_list = field_list;
            signal_output->device = device;
            if (match_p->flags & SIGNALS_MATCH_ONCE)
            {  /* No need to look anymore */
                goto exit;
            }
        }
    }   /* Dynamic Data */

exit:
    utilex_str_split_free(tokens, realtokens);
    return rv;
}

static int
sand_signal_value_read(
    rhlist_t * field_list,
    char *full_name,
    uint32 * value,
    int size)
{
    signal_output_t *cur_signal_output;
    int status;
    char *name;

    char **tokens;
    uint32 realtokens = 0, maxtokens = 2;

    tokens = utilex_str_split(full_name, ".", maxtokens, &realtokens);
    if (realtokens && (tokens == NULL))
    {
        goto exit;
    }

    if (realtokens == maxtokens)
        name = tokens[0];
    else
        name = full_name;

    RHITERATOR(cur_signal_output, field_list)
    {
        status = sal_strcasecmp(RHNAME(cur_signal_output), name);
        /*
         * If no more tokens and strings match we found our target
         */
        if ((realtokens == 1) && (status == 0))
        {
            if (size < BITS2WORDS(cur_signal_output->size))
            {
                cli_out("Not enough memory for signal:%s %d vs %d\n", name, size, cur_signal_output->size);
                return 0;
            }
            else
                memcpy(value, cur_signal_output->value, BITS2WORDS(cur_signal_output->size) * 4);
            return 1;
        }
        else
        {
            /*
             * If we had 2 tokens and strings were match we need to move to new token
             */
            if ((realtokens == maxtokens) && (status == 0))
                name = tokens[1];
            else
                name = full_name;

            /*
             * If we have regular string or strings were not match keep original name
             */
            if (sand_signal_value_read(cur_signal_output->field_list, name, value, size) == 1)
            {
                return 1;
            }
        }
    }

exit:
    utilex_str_split_free(tokens, realtokens);
    return 0;
}

int
dpp_dsig_read(
    int unit,
    int core,
    char *block,
    char *from,
    char *to,
    char *name,
    uint32 * value,
    int size)
{
    int res = _SHR_E_NONE;
    rhlist_t *dsig_list;
    match_t match;
    device_t *device;

    if ((value == NULL) || (size == 0))
    {
        res = _SHR_E_PARAM;
        goto exit;
    }

    if ((device = sand_signal_device_get(unit)) == NULL)
    {
      return res;
    }

    memset(&match, 0, sizeof(match_t));
    match.block = block;
    match.from = from;
    match.to = to;
    match.name = name;
    match.flags = SIGNALS_MATCH_EXPAND | SIGNALS_MATCH_ONCE | SIGNALS_MATCH_EXACT;

    if ((dsig_list = utilex_rhlist_create("signals", sizeof(signal_output_t), 0)) == NULL)
    {
        res = _SHR_E_MEMORY;
        goto exit;
    }

    if ((sand_signal_list_get(device, unit, core, &match, dsig_list) != _SHR_E_NONE) || (RHLNUM(dsig_list) == 0))
    {
        dsig_print_description(&match);
        res = _SHR_E_NOT_FOUND;
    }
    else
    {
        if (sand_signal_value_read(dsig_list, name, value, size) == 0)
        {
            res = _SHR_E_FAIL;
        }
    }

    sand_signal_list_free(dsig_list);

exit:
    return res;
}

int
sand_signal_handle_get(
    device_t * device,
    char *block,
    char *from,
    char *to,
    char *name,
    debug_signal_t ** signal_p)
{
    int signal_num = 0;
    rhlist_t *dsig_list;
    match_t match;
    signal_output_t *signal_output = NULL;
    int unit = 0;               /* Assuming there will be always unit 0 */
    int core = 0;               /* Assuming there will be always core 0 */

    if ((dsig_list = utilex_rhlist_create("signals", sizeof(signal_output_t), 0)) == NULL)
    {
        goto exit;
    }

    memset(&match, 0, sizeof(match_t));
    match.block = block;
    match.from = from;
    match.to = to;
    match.name = name;
    match.flags = SIGNALS_MATCH_EXACT | SIGNALS_MATCH_NOCOND | SIGNALS_MATCH_ONCE | SIGNALS_MATCH_NOVALUE;

    if (sand_signal_list_get(device, unit, core, &match, dsig_list) != _SHR_E_NONE)
    {
        cli_out("Signal:%s was not found\n", name);
    }
    else
    {
        if ((signal_output = utilex_rhlist_entry_get_first(dsig_list)) != NULL)
        {
            *signal_p = signal_output->debug_signal;
            signal_num = 1;
        }
    }

    sand_signal_list_free(dsig_list);

exit:
    return signal_num;
}

static static_signal_t irpp_jericho_static_signals[] = {
 /* Signal Name                  From                To                  Size  Block_ID Address */
    {"Port_Termination_Context",      "NIF",              "Port Termination", 8,    4, "{15'd1,16'd6} bits: [218 : 211]"},
    {"Packet_Header_Size",            "NIF",              "Port Termination", 7,    4, "{15'd1,16'd2} bits: [88 : 82]"},
    {"Packet_Header",                 "NIF",              "Port Termination", 1024, 4, "{15'd1,16'd2} bits: [255 : 89] | {15'd1,16'd3} bits: [255 : 0] | {15'd1,16'd4} bits: [255 : 0] | {15'd1,16'd5} bits: [255 : 0] | {15'd1,16'd6} bits: [88 : 0]"},
    {"Parser_Program_Ptr",            "Port Termination", "Parser",           4,    4, "{15'd2,16'd0} bits: [174 : 171]"},
    {"Src_System_Port",               "Port Termination", "Parser",           16,   4, "{15'd1,16'd6} bits: [206 : 191]"},
    {"In_PP_Port",                    "Port Termination", "Parser",           8,    4, "{15'd1,16'd6} bits: [226 : 219]"},
    {"Packet_Format_Qualifier",       "Parser",           "LL",               55,   4, "{15'd1,16'd6} bits: [146 : 92]"},
    {"Header_Offset",                 "Parser",           "LL",               42,   4, "{15'd1,16'd6} bits: [190 : 149]"},
    {"LL_LEM_Result",                 "LEM",              "LL",               45,   4, "{15'd7,16'd0} bits: [44 : 0]"},
    {"Snoop_Strength",                "LL",               "VT", 2, 4, "{15'd2,16'd0} bits: [55 : 54]"},
    {"Fwd_Action_Strength",           "LL",               "VT", 3, 4, "{15'd2,16'd0} bits: [120 : 118]"},
    {"Fwd_Action_CPU_Trap_Code",      "LL",               "VT", 8, 4, "{15'd2,16'd0} bits: [146 : 139]"},
    {"Fwd_Action_CPU_Trap_Qualifier", "LL",               "VT", 16, 4, "{15'd2,16'd0} bits: [138 : 123]"},
    {"Outer_VID",                     "LL",               "VT",               12,   4, "{15'd2,16'd4} bits: [255 : 251] | {15'd2,16'd5} bits: [6 : 0]"},
    {"Inner_VID",                     "LL",               "VT",               12,   4, "{15'd2,16'd5} bits: [19 : 8]"},
    {"Initial_VID",                   "LL",               "VT",               12,   4, "{15'd2,16'd0} bits: [24 : 13]"},
    {"Snoop_Strength",                "TT", "FLP", 2, 4, "{15'd3,16'd1} bits: [74 : 73]"},
    {"Fwd_Action_Strength",           "TT", "FLP", 3, 4, "{15'd3,16'd2} bits: [67 : 65]"},
    {"Fwd_Action_CPU_Trap_Code",      "TT", "FLP", 8, 4, "{15'd3,16'd2} bits: [110 : 103]"},
    {"Fwd_Action_CPU_Trap_Qualifier", "TT", "FLP", 16, 4, "{15'd4,16'd8} bits: [143 : 128]"},
    {"Fwd_Action_Dst",                "TT", "FLP", 19, 4, "{15'd3,16'd2} bits: [86 : 68]"},
    {"EEI",                           "TT", "FLP", 24, 4, "{15'd3,16'd2} bits: [135 : 112]"},
    {"Out_LIF",                       "TT", "FLP", 18, 4, "{15'd3,16'd1} bits: [122 : 105]"},
    {"Incoming_Tag_Structure",        "TT",               "FLP",              4,    4, "{15'd3,16'd1} bits: [194 : 191]"},
    {"Packet_Format_Code",            "TT",               "FLP",              6,    4, "{15'd3,16'd1} bits: [104 : 99]"},
    {"VSI",                           "TT",               "FLP",              19,   4, "{15'd3,16'd0} bits: [18 : 0]"},
    {"TT_Code",                       "TT",               "FLP",              4,    4, "{15'd3,16'd0} bits: [229 : 226]"},
    {"Fwd_Code",                      "TT",               "FLP",              4,    4, "{15'd3,16'd7} bits: [73 : 70]"},
    {"Fwd_Offset_Index",              "TT",               "FLP",              3,    4, "{15'd3,16'd7} bits: [67 : 65]"},
    {"Fwd_Offset_Extension",          "TT",               "FLP",              2,    4, "{15'd3,16'd7} bits: [69 : 68]"},
    {"VRF",                           "TT",               "FLP",              14,   4, "{15'd3,16'd0} bits: [222 : 209]"},
    {"TRILL_MC",                      "TT",               "FLP",              1,    4, "{15'd3,16'd1} bits: [54 : 54]"},
    {"TCAM_1st_Key",                  "FLP",              "TCAM",             160,  4, "{15'd28,16'd0} bits: [255 : 166] | {15'd28,16'd1} bits: [69 : 0]"},
    {"TCAM_1st_Result",               "TCAM",             "FLP",              48,   4, "{15'd29,16'd0} bits: [98 : 51]"},
    {"TCAM_1st_Match",                "TCAM",             "FLP",              1,    4, "{15'd29,16'd0} bits: [99 : 99]"},
    {"LEM_2nd_Key",                   "FLP",              "LEM",              80,   4, "{15'd26,16'd0} bits: [79 : 0]"},
    {"LPM_2nd_Lookup_Found",          "FLP",              "PMF",              1,    4, "{15'd4,16'd6} bits: [184 : 184]"},
    {"KBP_Key",                       "FLP",              "KBP",              1024, 4, "{15'd32,16'd0} bits: [255 : 8] | {15'd32,16'd1} bits: [255 : 0] | {15'd32,16'd2} bits: [255 : 0] | {15'd32,16'd3} bits: [255 : 0] | {15'd32,16'd4} bits: [7 : 0]"},
    {"KBP_Result_LSB",                "KBP",              "FLP",              128,  4, "{15'd33,16'd0} bits: [128 : 1]"},
    {"Fwd_Action_Dst",                "FLP",              "PMF",              19,   4, "{15'd4,16'd8} bits: [127 : 109]"},
    {"Snoop_Strength",                "FLP", "PMF", 2, 4, "{15'd4,16'd1} bits: [251 : 250]"},
    {"Fwd_Action_Strength",           "FLP", "PMF", 3, 4, "{15'd4,16'd8} bits: [104 : 102]"},
    {"Fwd_Action_CPU_Trap_Code",      "FLP", "PMF", 8, 4, "{15'd4,16'd8} bits: [151 : 144]"},
    {"Fwd_Action_CPU_Trap_Qualifier", "FLP", "PMF", 16, 4, "{15'd4,16'd8} bits: [143 : 128]"},
    {"EEI",                           "FLP", "PMF", 24, 4, "{15'd4,16'd9} bits: [192 : 169]"},
    {"Out_LIF",                       "FLP", "PMF", 18, 4, "{15'd4,16'd9} bits: [255 : 247] | {15'd4,16'd10} bits: [8 : 0]"},
    {"LPM_1st_A_Lookup_Key",          "FLP",              "LPM",              160,  4, "{15'd30,16'd1} bits: [255 : 252] | {15'd30,16'd2} bits: [155 : 0]"},
    {"LPM_1st_B_Lookup_Key",          "FLP",              "LPM",              160,  4, "{15'd30,16'd0} bits: [255 : 174] | {15'd30,16'd1} bits: [77 : 0]"},
    {"LPM_2nd_A_Lookup_Key",          "FLP",              "LPM",              160,  4, "{15'd30,16'd1} bits: [244 : 85]"},
    {"LPM_2nd_B_Lookup_Key",          "FLP",              "LPM",              160,  4, "{15'd30,16'd0} bits: [166 : 7]"},
    {"LPM_1st_A_Lookup_Result",       "LPM",              "FLP",              20,   4, "{15'd31,16'd0} bits: [119 : 100]"},
    {"LPM_1st_B_Lookup_Result",       "LPM",              "FLP",              20,   4, "{15'd31,16'd0} bits: [59 : 40]"},
    {"LPM_2nd_A_Lookup_Result",       "LPM",              "FLP",              20,   4, "{15'd31,16'd0} bits: [89 : 70]"},
    {"LPM_2nd_B_Lookup_Result",       "LPM",              "FLP",              20,   4, "{15'd31,16'd0} bits: [29 : 10]"},
    {"LPM_1st_A_Lookup_Status",       "LPM",              "FLP",              2,    4, "{15'd31,16'd0} bits: [99 : 98]"},
    {"LPM_1st_B_Lookup_Status",       "LPM",              "FLP",              2,    4, "{15'd31,16'd0} bits: [39 : 38]"},
    {"LPM_2nd_A_Lookup_Status",       "LPM",              "FLP",              2,    4, "{15'd31,16'd0} bits: [69 : 68]"},
    {"LPM_2nd_B_Lookup_Status",       "LPM",              "FLP",              2,    4, "{15'd31,16'd0} bits: [9 : 8]"},
    {"LPM_1st_A_Lookup_Length",       "LPM",              "FLP",              8,    4, "{15'd31,16'd0} bits: [97 : 90]"},
    {"LPM_1st_B_Lookup_Length",       "LPM",              "FLP",              8,    4, "{15'd31,16'd0} bits: [37 : 30]"},
    {"LPM_2nd_A_Lookup_Length",       "LPM",              "FLP",              8,    4, "{15'd31,16'd0} bits: [67 : 60]"},
    {"LPM_2nd_B_Lookup_Length",       "LPM",              "FLP",              8,    4, "{15'd31,16'd0} bits: [7 : 0]"},
    {"Fwd_Action",                    "PMF",              "FER",              133, 21, "{15'd0,16'd10} bits: [255 : 241] | {15'd0,16'd11} bits: [117 : 0]"},
    {"EEI",                           "PMF",              "FER",              24,  21, "{15'd0,16'd11} bits: [161 : 138]"},
    {"Out_LIF",                       "PMF",              "FER",              18,  21, "{15'd0,16'd9} bits: [219 : 202]"},
    {"Snoop_Strength",                "PMF",              "FER",              2,   21, "{15'd0,16'd9} bits: [136 : 135]"},
    {"Snoop_Code",                    "PMF",              "FER",              8,   21, "{15'd0,16'd9} bits: [144 : 137]"},
    {"In_PP_Port",                    "PMF",              "FER",              8,   21, "{15'd0,16'd12} bits: [32 : 25]"},
    {"Statistics_Tag",                "PMF",              "FER",              8,   21, "{15'd0,16'd9} bits: [93 : 86]"},
    {"St_VSQ_Ptr",                    "PMF",              "FER",              8,   21, "{15'd0,16'd9} bits: [118 : 111]"},
    {"Mirror_Action",                 "PMF",              "FER",              4,   21, "{15'd0,16'd9} bits: [255 : 254] | {15'd0,16'd10} bits: [1 : 0]"},
    {"VLAN_Edit_Cmd",                 "PMF",              "FER",              34,  21, "{15'd0,16'd8} bits: [253 : 220]"},
    {"Packet_Format_Qualifier",       "PMF",              "FER",              55,  21, "{15'd0,16'd4} bits: [71 : 17]"},
    {"Out_Mirror_Disable",            "PMF",              "FER",              1,   21, "{15'd0,16'd9} bits: [201 : 201]"},
    {"Exclude_Src_Action",            "PMF",              "FER",              1,   21, "{15'd0,16'd11} bits: [136 : 136]"},
    {"Ingress_Shaping_Dst",           "PMF",              "FER",              19,  21, "{15'd0,16'd10} bits: [145 : 127]"},
    {"Src_System_Port",               "PMF",              "FER",              16,  21, "{15'd0,16'd9} bits: [134 : 119]"},
    {"Fwd_Code",                      "PMF",              "FER",              4,   21, "{15'd0,16'd10} bits: [240 : 237]"},
    {"Fwd_Offset_Index",              "PMF",              "FER",              3,   21, "{15'd0,16'd8} bits: [187 : 185]"},
    {"Fwd_Offset_Fix",                "PMF",              "FER",              6,   21, "{15'd0,16'd10} bits: [234 : 229]"},
    {"Bytes_to_Remove_Index",         "PMF",              "FER",              2,   21, "{15'd0,16'd12} bits: [8 : 7]"},
    {"Bytes_to_Remove_Fix",           "PMF",              "FER",              6,   21, "{15'd0,16'd12} bits: [14 : 9]"},
    {"System_Header_Profile_Index",   "PMF",              "FER",              4,   21, "{15'd0,16'd9} bits: [85 : 82]"},
    {"VSI_ACL",                       "PMF",              "FER",              16,  21, "{15'd0,16'd8} bits: [205 : 190]"},
    {"Orientation_is_Hub",            "PMF",              "FER",              1,   21, "{15'd0,16'd9} bits: [220 : 220]"},
    {"In_RIF",                        "PMF",              "FER",              15,  21, "{15'd0,16'd10} bits: [175 : 161]"},
    {"VRF",                           "PMF",              "FER",              14,  21, "{15'd0,16'd8} bits: [219 : 206]"},
    {"In_TTL",                        "PMF",              "FER",              8,   21, "{15'd0,16'd10} bits: [154 : 147]"},
    {"In_DSCP_EXP",                   "PMF",              "FER",              8,   21, "{15'd0,16'd10} bits: [213 : 206]"},
    {"RPF_Dst_Valid",                 "PMF",              "FER",              1,   21, "{15'd0,16'd9} bits: [177 : 177]"},
    {"RPF_Dst",                       "PMF",              "FER",              19,  21, "{15'd0,16'd9} bits: [196 : 178]"},
    {"Ingress_Learn_Enable",          "PMF",              "FER",              1,   21, "{15'd0,16'd10} bits: [146 : 146]"},
    {"Egress_Learn_Enable",           "PMF",              "FER",              1,   21, "{15'd0,16'd11} bits: [137 : 137]"},
    {"Learn_Key",                     "PMF",              "FER",              63,  21, "{15'd0,16'd10} bits: [66 : 4]"},
    {"Learn_Data",                    "PMF",              "FER",              40,  21, "{15'd0,16'd10} bits: [106 : 67]"},
    {"Learn_or_Transplant",           "PMF",              "FER",              1,   21, "{15'd0,16'd10} bits: [3 : 3]"},
    {"In_LIF",                        "PMF",              "FER",              18,  21, "{15'd0,16'd10} bits: [205 : 188]"},
    {"ECMP_LB_Key_Packet_Data",       "PMF",              "FER",              20,  21, "{15'd0,16'd11} bits: [181 : 162]"},
    {"LAG_LB_Key_Packet_Data",        "PMF",              "FER",              20,  21, "{15'd0,16'd10} bits: [126 : 107]"},
    {"Stacking_Route_History_Bitmap", "PMF",              "FER",              16,  21, "{15'd0,16'd9} bits: [110 : 95]"},
    {"Ignore_CP",                     "PMF",              "FER",              1,   21, "{15'd0,16'd10} bits: [214 : 214]"},
    {"PPH_Type",                      "PMF",              "FER",              2,   21, "{15'd0,16'd9} bits: [198 : 197]"},
    {"Packet_is_BOOTP_DHCP",          "PMF",              "FER",              1,   21, "{15'd0,16'd9} bits: [200 : 200]"},
    {"Unknown_Addr",                  "PMF",              "FER",              1,   21, "{15'd0,16'd9} bits: [65 : 65]"},
    {"Fwd_Header_Enc",                "PMF",              "FER",              2,   21, "{15'd0,16'd10} bits: [236 : 235]"},
    {"IEEE1588_Enc",                  "PMF",              "FER",              1,   21, "{15'd0,16'd10} bits: [222 : 222]"},
    {"OAM_Up_MEP",                    "PMF",              "FER",              1,   21, "{15'd0,16'd9} bits: [221 : 221]"},
    {"OAM_Sub_Type",                  "PMF",              "FER",              3,   21, "{15'd0,16'd9} bits: [224 : 222]"},
    {"OAM_Stamp_Offset",              "PMF",              "FER",              7,   21, "{15'd0,16'd9} bits: [231 : 225]"},
    {"OAM_Offset",                    "PMF",              "FER",              7,   21, "{15'd0,16'd9} bits: [238 : 232]"},
    {"In_LIF_Profile",                "PMF",              "FER",              4,   21, "{15'd0,16'd10} bits: [187 : 184]"},
    {"In_RIF_Profile",                "PMF",              "FER",              6,   21, "{15'd0,16'd10} bits: [160 : 155]"},
    {"Fwd_Action",                    "FER",              "LBP",              133, 21, "{15'd2,16'd2} bits: [255 : 158] | {15'd2,16'd3} bits: [34 : 0]"},
    {"EEI",                           "FER",              "LBP",              24,  21, "{15'd2,16'd3} bits: [86 : 63]"},
    {"Out_LIF",                       "FER",              "LBP",              18,  21, "{15'd2,16'd1} bits: [62 : 45]"},
    {"VLAN_Edit_Cmd",                 "FER",              "LBP",              34,  21, "{15'd2,16'd0} bits: [149 : 116]"},
    {"Ingress_Learn_Enable",          "FER",                "LBP", 1, 4, "{15'd2,16'd1} bits: [194 : 194]"},
    {"Egress_Learn_Enable",           "FER",              "LBP", 1, 21, "{15'd2,16'd3} bits: [62 : 62]"},
    {"Learn_or_Transplant",           "FER", "LBP", 1, 21, "{15'd2,16'd1} bits: [95 : 95]"},
    {"Learn_Data",                    "FER", "LBP", 40, 21, "{15'd2,16'd0} bits: [59 : 20]"},
    {"Learn_Key",                     "FER", "LBP",  63, 21, "{15'd2,16'd1} bits: [158 : 96]"},
    {"TM_Cmd",                        "LBP",              "ITM",              323, 21, "{15'd4,16'd0} bits: [255 : 0] | {15'd4,16'd1} bits: [66 : 0]"},
    {NULL}
};

static static_signal_t etpp_jericho_static_signals[] = {
    {"System_Headers_Record",       "Prp",      "Term", 89,  18, "{15'd4,16'd0} bits: [88 : 0]", "SHR_ETPP_PRP"},
    {"EES_Action_0_Valid",          "Prp",      "Term", 1,   18, "{15'd4,16'd2} bits: [228 : 228]"},
    {"EES_Action_0",                "Prp",      "Term", 28,  18, "{15'd4,16'd2} bits: [255 : 229] | {15'd4,16'd3} bits: [0 : 0]", "EES_Action_Short"},
    {"EES_Action_1_Valid",          "Prp",      "Term", 1,   18, "{15'd4,16'd2} bits: [96 : 96]"},
    {"EES_Action_1",                "Prp",      "Term", 131, 18, "{15'd4,16'd2} bits: [227 : 97]"},
    {"EES_Action_2_Valid",          "Prp",      "Term", 1,   18, "{15'd4,16'd1} bits: [220 : 220]"},
    {"EES_Action_2",                "Prp",      "Term", 131, 18, "{15'd4,16'd1} bits: [255 : 221] | {15'd4,16'd2} bits: [95 : 0]"},
    {"EES_Action_3_Valid",          "Prp",      "Term", 1,   18, "{15'd4,16'd1} bits: [88 : 88]"},
    {"EES_Action_3",                "Prp",      "Term", 131, 18, "{15'd4,16'd1} bits: [219 : 89]"},
    {"Out_LIF",                     "Encap",    "LL",   18,  18, "{15'd8,16'd0} bits: [77 : 60]"},
    {"FHEI_Code",                   "LL",       "PRGE", 2,   18, "{15'd9,16'd2} bits: [183 : 182]"},
    {"PRGE_Instruction_1_Index",    "PRGE",     NULL, 7, 18, "{15'd11,16'd0} bits: [86 : 80]"},
    {NULL}
};

static static_field_t fwd_action_fields[] = {
    {"Strength",            "2:0"},
    {"Dst",                 "21:3"},
    {"TC",                  "24:22"},
    {"DP",                  "26:25"},
    {"Meter_A_Update",      "27"},
    {"Meter_A_Ptr",         "44:28"},
    {"Meter_B_Update",      "45"},
    {"Meter_B_Ptr",         "62:46"},
    {"DP_Meter_Cmd",        "64:63"},
    {"Counter_A_Update",    "65"},
    {"Counter_A_Ptr",       "86:66"},
    {"Counter_B_Update",    "87"},
    {"Counter_B_Ptr",       "108:88"},
    {"CPU_Trap_Code",       "116:109"},
    {"CPU_Trap_Qualifier",  "132:117"},
    {NULL},
};

static static_field_t vlan_edit_cmd_fields[] = {
    {"VID_1",   "11:0"},
    {"VID_2",   "23:12"},
    {"DEI",     "24:24"},
    {"PCP",     "27:25"},
    {"Cmd",     "33:28"},
    {NULL},
};

static static_field_t pfq_fields[] = {
    {"Header_1", "10:00"},
    {"Header_2", "21:11"},
    {"Header_3", "32:22"},
    {"Header_4", "43:33"},
    {"Header_5", "54:44"},
    {NULL},
};

static static_field_t tm_cmd_fields[] = {
    {"Eth_Meter_Ptr",                   "10:0"},
    {"Learn_Info_Valid",                "11"},
    {"Learn_Info",                      "115:12"},
    {"Learn_Info_Reserved",             "117:116"},
    {"Dst_Valid",                       "118"},
    {"Snoop_Cmd",                       "122:119"},
    {"Mirror_Cmd",                      "126:123"},
    {"DP",                              "128:127"},
    {"TC",                              "131:129"},
    {"Dst",                             "150:132"},
    {"Ingress_is_Shaped",               "151"},
    {"Ingress_Shaping_Dst",             "168:152"},
    {"Eth_Enc",                         "170:169"},
    {"Statistics_Tag",                  "178:171"},
    {"St_VSQ_Ptr",                      "186:179"},
    {"Meter_B_Ptr",                     "203:187"},
    {"Meter_B_Update",                  "204"},
    {"Meter_A_Ptr",                     "221:205"},
    {"Meter_A_Update",                  "222"},
    {"Counter_B_Ptr",                   "243:223"},
    {"Counter_B_Update",                "244"},
    {"Counter_A_Ptr",                   "265:245"},
    {"Counter_A_Update",                "266"},
    {"LAG_LB_Key",                      "282:267"},
    {"DP_Meter_Cmd",                    "284:283"},
    {"Ignore_CP",                       "285"},
    {"LAG_Member_Valid",                "286"},
    {"In_PP_Port",                      "294:287"},
    {"Network_Header_Truncate_Size",    "302:295"},
    {"Network_Header_Append_Size_Ptr",  "310:303"},
    {"Snoop_Code",                      "318:311"},
    {"Reserved",                        "322:319"},
    {NULL},
};

static static_field_t shr_ettp_prp_fields[] = {
    {"Drop",            "0:0"},
    {"Fwd_Code",        "04:01"},
    {"Out_LIF",         "22:05"},
    {"FHEI",            "62:23"},
    {"FHEI_Upper_3B",   "86:63"},
    {"FHEI_Code",       "88:87"},
    {NULL}
};

static static_field_t ees_action_short_fields[] = {
    {"EEI",             "23:0"},
    {"Type",            "27:24"},
    {NULL}
};

static static_sigstruct_t static_sigstructs[] = {
    {"Fwd_Action",              133,   fwd_action_fields},
    {"VLAN_Edit_Cmd",           34,    vlan_edit_cmd_fields},
    {"Packet_Format_Qualifier", 55,    pfq_fields},
    {"TM_Cmd",                  323,   tm_cmd_fields},
    {"SHR_ETPP_PRP",            89,    shr_ettp_prp_fields},
    {"EES_Action_Short",        28,    ees_action_short_fields},
    {NULL},
};

static static_block_t jericho_static_blocks[] = {
    {"IRPP", irpp_jericho_static_signals},
    {"ETPP", etpp_jericho_static_signals},
    {NULL}
};

static static_device_t static_devices[] = {
    {"jericho", jericho_static_blocks},
    {"qmx",     jericho_static_blocks},
    {NULL}
};

static int
dsig_struct_static_init(
    device_t * device)
{
    int res = _SHR_E_NONE;
    static_sigstruct_t *static_sigstruct;
    static_field_t *static_field;
    sigstruct_t *cur_sigstruct;
    sigstruct_field_t *cur_sigstruct_field;

    if (device->sigstructs != NULL)
        /*
         * ALready initialized
         */
        return res;

    device->sigstruct_num = sizeof(static_sigstructs) / sizeof(static_sigstruct_t);
    cur_sigstruct = device->sigstructs = utilex_alloc(device->sigstruct_num * sizeof(sigstruct_t));

    /*
     * loop through entries
     */
    for(static_sigstruct = static_sigstructs; static_sigstruct->name != NULL; static_sigstruct++)
    {
        sal_strncpy(cur_sigstruct->name, static_sigstruct->name, (RHNAME_MAX_SIZE - 1));
        cur_sigstruct->size = static_sigstruct->size;

        for(static_field = static_sigstruct->fields; static_field->name != NULL; static_field++)
            cur_sigstruct->field_num++;

        cur_sigstruct_field = cur_sigstruct->fields =
                                    utilex_alloc(cur_sigstruct->field_num * sizeof(sigstruct_field_t));

        /*
         * loop through entries
         */
        for(static_field = static_sigstruct->fields; static_field->name != NULL; static_field++)
        {
            sal_strncpy(cur_sigstruct_field->name, static_field->name, (RHNAME_MAX_SIZE - 1));
            if (sand_signal_range_parse(static_field->bitstr, &cur_sigstruct_field->start_bit,
                                                                        &cur_sigstruct_field->end_bit) != _SHR_E_NONE)
            {
                cli_out("Field:%s.%s, has bad bites range:%s\n", cur_sigstruct->name, cur_sigstruct_field->name,
                                                                                                static_field->bitstr);
                continue;
            }

            cur_sigstruct_field->size = cur_sigstruct_field->end_bit + 1 - cur_sigstruct_field->start_bit;
            cur_sigstruct_field++;
        }
        cur_sigstruct++;
    }

    return res;
}

device_t *
sand_signal_static_init(
    int unit)
{
    device_t *device = NULL;
    static_device_t *static_device;
    static_block_t *static_block;
    static_signal_t *static_signal;
    pp_block_t *cur_pp_block;

    for(static_device = static_devices; static_device->name != NULL; static_device++)
    {
        if(!sal_strcasecmp(SOC_CHIP_STRING(unit), static_device->name))
            break;
    }

    /* Check if we have found the match */
    if(static_device->name == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META_U(unit, "Signal Data for %s was not found\n"), SOC_CHIP_STRING(unit)));
        goto exit;
    }

    /* We have a match - now allocate device */
    if((device = utilex_alloc(sizeof(device_t))) == NULL)
        goto exit;

    /* Copy device name */
    sal_strncpy(RHNAME(device), static_device->name, (RHNAME_MAX_SIZE - 1));

    /* Initialize parsing structures for the device */
    if ((dsig_struct_static_init(device)) != _SHR_E_NONE)
        goto exit;

    /* Figure out how many blocks are in the device */
    for(static_block = static_device->blocks; static_block->name != NULL; static_block++)
        device->block_num++;

    if(device->block_num == 0)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META_U(unit, "No blocks %s Data\n"), SOC_CHIP_STRING(unit)));
        goto exit;
    }

    /* Allocate space for blocks */
    if((cur_pp_block = device->pp_blocks = utilex_alloc(sizeof(pp_block_t) * device->block_num)) == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META_U(unit, "No memory for %s Blocks Data\n"), SOC_CHIP_STRING(unit)));
        goto exit;
    }
    /*
     * loop through entries
     */
    for(static_block = static_device->blocks; static_block->name != NULL; static_block++)
    {
        debug_signal_t *debug_signal;

        sal_strncpy(cur_pp_block->name, static_block->name, (RHNAME_MAX_SIZE - 1));

        for(static_signal = static_block->signals; static_signal->name != NULL; static_signal++)
            cur_pp_block->signal_num++;
        /*
         * There are no debug signals for this block
         */
        if (cur_pp_block->signal_num == 0)
            continue;

        if((debug_signal = cur_pp_block->debug_signals = utilex_alloc(sizeof(debug_signal_t) * cur_pp_block->signal_num)) == NULL)
        {
            LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META_U(unit, "No memory for %s Signal Data\n"), SOC_CHIP_STRING(unit)));
            goto exit;
        }

        /*
         * loop through entries
         */
        for(static_signal = static_block->signals; static_signal->name != NULL; static_signal++)
        {
            int size, i;
            sal_strncpy(debug_signal->attribute, static_signal->name, (RHNAME_MAX_SIZE - 1));
            if(static_signal->from != NULL)
                sal_strncpy(debug_signal->from, static_signal->from, (RHNAME_MAX_SIZE - 1));

            if(static_signal->to != NULL)
                sal_strncpy(debug_signal->to, static_signal->to, (RHNAME_MAX_SIZE - 1));

            debug_signal->size = static_signal->size;
            debug_signal->block_id = static_signal->block_id;
            sal_strncpy(debug_signal->block_n, cur_pp_block->name, (RHNAME_MAX_SIZE - 1));

            if(static_signal->addr_str != NULL)
                debug_signal->range_num = sand_signal_address_parse(static_signal->addr_str, debug_signal->address);
            else {
                LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META_U(0, "Signal %s has no address\n"), debug_signal->attribute));
                continue;
            }

            if (static_signal->expansion)
            {
                if (sand_signal_struct_get(device, static_signal->expansion) != NULL)
                    sal_strncpy(debug_signal->expansion, static_signal->expansion, (RHNAME_MAX_SIZE - 1));
                else
                    LOG_WARN(BSL_LS_SOC_COMMON, (BSL_META_U(0, "No signal expansion:%s\n"), debug_signal->expansion));
            }
            else
            {
                /*
                 * No explicit expansion - look for implicit one
                 */
                if ((sand_signal_struct_get(device, debug_signal->attribute)) != NULL)
                    /*
                     * If there is match - attribute serves as expansion
                     */
                    sal_strcpy(debug_signal->expansion, debug_signal->attribute);
            }

            /*
             * Verify consistency between size and sum of all bits in range
             */
            size = 0;
            for (i = 0; i < debug_signal->range_num; i++)
                size += (debug_signal->address[i].msb + 1 - debug_signal->address[i].lsb);

            if (size != debug_signal->size)
            {
                LOG_WARN(BSL_LS_SOC_COMMON, (BSL_META_U(0, "Correcting size for:%s from:%d to %d\n"), debug_signal->attribute, debug_signal->size, size));
                debug_signal->size = size;
            }

            debug_signal++;
        }
        cur_pp_block++;
    }

exit:
    return device;
}

#if !defined(NO_FILEIO)
/*
 * {
 */
static
int
shr_access_reg_init(xml_node curTop)
{
    int res = _SHR_E_NONE;
    xml_node curSubTop, cur;

    if ((curSubTop = dbx_xml_child_get_first(curTop, "registers")) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    RHDATA_ITERATOR(cur, curSubTop, "reg")
    {
        char reg_name[RHNAME_MAX_SIZE];
        soc_reg_t reg;

        RHDATA_GET_STR_CONT(cur, "name", reg_name);

        for(reg = 0; reg < NUM_SOC_REG; reg++)
        {
            if(!sal_strcmp(SOC_REG_NAME(unit, reg), reg_name))
                break;
        }
        if(reg == NUM_SOC_REG)
        {
            LOG_WARN(BSL_LS_SOC_COMMON, (BSL_META_U(0, "Register:%s does not exist\n"), reg_name));
        }
        else
        {
            int tmp;
            if(reg_data[reg].flags != 0)
            {
                LOG_VERBOSE(BSL_LS_SOC_COMMON, (BSL_META_U(0, "Register:%s appeared twice in auxiliary list\n"), reg_name));
            }
            RHDATA_GET_INT_DEF(cur, "no_read", tmp, 0);
            /* Registers that cannot be read cannot participate in wb test, no need to mention it explicitly */
            if(tmp == 1)
            {
                reg_data[reg].flags = ACC_NO_READ | ACC_NO_WB;
            }
            else
            {
                RHDATA_GET_INT_DEF(cur, "no_wb", tmp, 0);
                reg_data[reg].flags = ACC_NO_WB;
            }
        }
    }

exit:
    return res;
}

static
int
shr_access_mem_init(xml_node curTop)
{
    int res = _SHR_E_NONE;
    xml_node curSubTop, cur;

    if ((curSubTop = dbx_xml_child_get_first(curTop, "memories")) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    RHDATA_ITERATOR(cur, curSubTop, "mem")
    {
        char mem_name[RHNAME_MAX_SIZE];
        soc_mem_t mem;

        RHDATA_GET_STR_CONT(cur, "name", mem_name);

        for(mem = 0; mem < NUM_SOC_MEM; mem++)
        {
            if(!sal_strcmp(SOC_MEM_NAME(unit, mem), mem_name))
                break;
        }
        if(mem == NUM_SOC_MEM)
        {
            LOG_WARN(BSL_LS_SOC_COMMON, (BSL_META_U(0, "Memory:%s does not exist\n"), mem_name));
        }
        else
        {
            int tmp;
            if(mem_data[mem].flags)
            {
                LOG_VERBOSE(BSL_LS_SOC_COMMON, (BSL_META_U(0, "Memory:%s appeared twice in auxilary list\n"), mem_name));
            }
            RHDATA_GET_INT_DEF(cur, "no_read", tmp, 0);
            /* Memories that cannot be read cannot participate in wb test, no need to mention it explicitly */
            if(tmp == 1)
            {
                mem_data[mem].flags = ACC_NO_READ | ACC_NO_WB;
            }
            else
            {
                RHDATA_GET_INT_DEF(cur, "no_wb", tmp, 0);
                mem_data[mem].flags = ACC_NO_WB;
            }
        }
    }
exit:
    return res;
}

int
shr_access_obj_init(device_t *device)
{
    int res = _SHR_E_NONE;
    xml_node curTop;

    /* Look for global file first */
    if ((curTop = dbx_file_get_xml_top(NULL, "AccessObjects.xml", "top", 0)) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    if((res = shr_access_reg_init(curTop)) != _SHR_E_NONE)
    {
        goto exit;
    }

    if((res = shr_access_mem_init(curTop)) != _SHR_E_NONE)
    {
        goto exit;
    }
#ifdef USE_DEVICE_SPECIFIC
    /* Then for per device add-ons if any */
    if ((curTop = dbx_file_get_xml_top(RHNAME(device), "AccessObjects.xml", "top", 0)) == NULL)
    {
        goto exit;
    }

    if((res = shr_access_reg_init(curTop)) != _SHR_E_NONE)
    {
        goto exit;
    }

    if((res = shr_access_mem_init(curTop)) != _SHR_E_NONE)
    {
        goto exit;
    }
#endif /* USE_DEVICE_SPECIFIC */
exit:
    return res;
}

static rhlist_t    *sand_db_list = NULL;

shr_error_e
shr_access_global_init(void)
{
    shr_error_e rv = _SHR_E_NONE;
    xml_node curTop, curSubTop, cur;
    char device_n[RHNAME_MAX_SIZE];
    char db_name[RHNAME_MAX_SIZE];
    device_t *device;
    rhhandle_t temp = NULL;

    /* If it is not first device global device list may be already initialized */
    if (sand_db_list != NULL)
        goto  exit;

    memset(sand_device_array, 0, sizeof(device_t *) * SOC_MAX_NUM_DEVICES);

    if ((curTop = dbx_file_get_xml_top(NULL, "DNX-Devices.xml", "top", 0)) == NULL)
    {
        rv = _SHR_E_NOT_FOUND;
        goto exit;
    }

    if ((curSubTop = dbx_xml_child_get_first(curTop, "devices")) == NULL)
    {
        rv = _SHR_E_NOT_FOUND;
        goto exit;
    }

    if ((sand_db_list = utilex_rhlist_create("devices", sizeof(device_t), 0)) == NULL)
    {
        rv = _SHR_E_MEMORY;
        goto exit;
    }

    RHDATA_ITERATOR(cur, curSubTop, "device")
    {
        RHDATA_GET_STR_CONT(cur, "chip", device_n);
        RHDATA_GET_STR_CONT(cur, "db_name", db_name);
        if ((device = utilex_rhlist_entry_get_by_name(sand_db_list, db_name)) == NULL)
        {
            /*
             * No such db in the list
             */
            if ((rv = utilex_rhlist_entry_add_tail(sand_db_list, db_name, RHID_TO_BE_GENERATED, &temp)) != _SHR_E_NONE)
            {
                /*
                 * No more place any more - return with what you have until now
                 */
                goto exit;
            }
            device = temp;
            /*
             * Create list of chips supported by this device
             */
            if ((device->chip_list = utilex_rhlist_create("devices", sizeof(rhentry_t), 0)) == NULL)
            {
                /*
                 * Couldn't create new list - still we can leave with what we already have
                 */
                goto exit;
            }
        }

        if ((rv = utilex_rhlist_entry_add_tail(device->chip_list, device_n, RHID_TO_BE_GENERATED, &temp)) != _SHR_E_NONE)
        {
            /*
             * No more place any more - return with what you have until now
             */
            goto exit;
        }
    }

exit:
    return rv;
}

static void
sand_signal_expand_init(
    xml_node cur,
    expansion_t * expansion)
{
    xml_node curOption;
    expansion_option_t *option;

    RHDATA_GET_STR_DEF_NULL(cur, "expansion", expansion->name);

    if (!sal_strcasecmp(expansion->name, "Dynamic"))
    {
        RHDATA_GET_NODE_NUM(expansion->option_num, cur, "option");
        option = expansion->options = utilex_alloc(expansion->option_num * sizeof(expansion_option_t));
        RHDATA_ITERATOR(curOption, cur, "option")
        {
            RHDATA_GET_STR_CONT(curOption, "expansion", option->name);
            dbx_xml_property_get_all(curOption, option->param, "expansion", DSIG_OPTION_PARAM_MAX_NUM);
            option++;
        }
    }
    return;
}

static int
sand_signal_struct_init(
    device_t * device)
{
    xml_node curTop, curSubTop, cur, curSub;
    int res = _SHR_E_NONE;
    sigstruct_t *cur_sigstruct;
    sigstruct_field_t *cur_sigstruct_field;
    sigparam_t *cur_sigparam;
    sigparam_value_t *cur_sigparam_value;

    char temp[RHNAME_MAX_SIZE];

    if (device->sigstructs != NULL)
        /*
         * ALready initialized
         */
        return res;

    if ((curTop = dbx_file_get_xml_top(RHNAME(device), "PP.xml", "top", 0)) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    RHDATA_GET_STR_STOP(curTop, "expansion", temp);
    dbx_xml_top_close(curTop);

    if ((curTop = dbx_file_get_xml_top(RHNAME(device), temp, "top", 0)) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    /*
     * Read all available parameters with their possible values
     */
    if ((curSubTop = dbx_xml_child_get_first(curTop, "signal-params")) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    RHDATA_GET_NODE_NUM(device->sigparam_num, curSubTop, "signal");
    cur_sigparam = device->sigparams = utilex_alloc(device->sigparam_num * sizeof(sigparam_t));

    RHDATA_ITERATOR(cur, curSubTop, "signal")
    {
        RHDATA_GET_STR_CONT(cur, "name", cur_sigparam->name);
        RHDATA_GET_INT_CONT(cur, "size", cur_sigparam->size);
        RHDATA_GET_STR_DEF_NULL(cur, "default", cur_sigparam->default_str);

        RHDATA_GET_NODE_NUM(cur_sigparam->value_num, cur, "entry");
        cur_sigparam_value = cur_sigparam->values = utilex_alloc(cur_sigparam->value_num * sizeof(sigparam_value_t));

        RHDATA_ITERATOR(curSub, cur, "entry")
        {
            RHDATA_GET_STR_CONT(curSub, "name", cur_sigparam_value->name);
            RHDATA_GET_INT_CONT(curSub, "value", cur_sigparam_value->value);
            cur_sigparam_value++;
        }

        cur_sigparam++;
    }

    if ((curSubTop = dbx_xml_child_get_first(curTop, "signal-structures")) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    RHDATA_GET_NODE_NUM(device->sigstruct_num, curSubTop, "structure");
    cur_sigstruct = device->sigstructs = utilex_alloc(device->sigstruct_num * sizeof(sigstruct_t));

    /*
     * loop through entries
     */
    RHDATA_ITERATOR(cur, curSubTop, "structure")
    {
        RHDATA_GET_STR_CONT(cur, "name", cur_sigstruct->name);
        RHDATA_GET_INT_CONT(cur, "size", cur_sigstruct->size);
        sand_signal_expand_init(cur, &cur_sigstruct->expansion_m);

        RHDATA_GET_NODE_NUM(cur_sigstruct->field_num, cur, "field");
        cur_sigstruct_field = cur_sigstruct->fields =
            utilex_alloc(cur_sigstruct->field_num * sizeof(sigstruct_field_t));

        /*
         * loop through entries
         */
        RHDATA_ITERATOR(curSub, cur, "field")
        {
            RHDATA_GET_STR_CONT(curSub, "name", cur_sigstruct_field->name);
            RHDATA_GET_STR_CONT(curSub, "bits", temp);
            RHDATA_GET_STR_DEF_NULL(curSub, "condition", cur_sigstruct_field->cond_attribute);

            if (sand_signal_range_parse(temp, &cur_sigstruct_field->start_bit, &cur_sigstruct_field->end_bit) !=
                _SHR_E_NONE)
            {
                cli_out("Field:%s.%s, has bad bites range:%s\n", cur_sigstruct->name, cur_sigstruct_field->name, temp);
                continue;
            }

            cur_sigstruct_field->size = cur_sigstruct_field->end_bit + 1 - cur_sigstruct_field->start_bit;

            RHDATA_GET_STR_DEF_NULL(curSub, "resolution", cur_sigstruct_field->resolution);

            sand_signal_expand_init(curSub, &cur_sigstruct_field->expansion_m);

            cur_sigstruct_field++;
        }
        cur_sigstruct++;
    }

    sand_signal_struct_expand_init(device);

    dbx_xml_top_close(curTop);
exit:
    return res;
}

shr_error_e
sand_qual_signal_init(
    char *db_name,
    char *qual_signals_db,
    pp_stage_t * pp_stage)
{
    shr_error_e res = _SHR_E_NONE;
    void *curTop, *cur;
    internal_signal_t *cur_internal_signal;

    if ((curTop = dbx_file_get_xml_top(db_name, qual_signals_db, "SignalInfo", 0)) == NULL)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    RHDATA_GET_INT_STOP(curTop, "size0", pp_stage->buffer0_size);
    RHDATA_GET_INT_DEF(curTop,  "size1", pp_stage->buffer1_size, 0);
    RHDATA_GET_NODE_NUM(pp_stage->number, curTop, "signal")

    if (pp_stage->number == 0)
    {
        res = _SHR_E_NOT_FOUND;
        goto exit;
    }

    cur_internal_signal = pp_stage->signals = utilex_alloc(sizeof(internal_signal_t) * pp_stage->number);

    /*
     * loop through entries
     */
    RHDATA_ITERATOR(cur, curTop, "signal")
    {
        dbx_xml_child_get_content_str(cur, "name",    cur_internal_signal->name, RHNAME_MAX_SIZE);
        dbx_xml_child_get_content_int(cur, "offset",  &cur_internal_signal->offset);
        dbx_xml_child_get_content_int(cur, "size",    &cur_internal_signal->size);
        dbx_xml_child_get_content_int(cur, "buffer",  &cur_internal_signal->buffer);
        dbx_xml_child_get_content_str(cur, "verilog", cur_internal_signal->hw, RHFILE_MAX_SIZE);
        cur_internal_signal++;
    }

    dbx_xml_top_close(curTop);

exit:
    return res;
}

shr_error_e
sand_signal_init(
    int        unit,
    device_t * device)
{
    shr_error_e rv = _SHR_E_NONE;
    void *curTop, *curSubTop, *curBlock;
    debug_signal_t *debug_signal;
    int i, j;

    pp_block_t *cur_pp_block;

    if (device == NULL)
    {
        rv = _SHR_E_NOT_FOUND;
        goto exit;
    }

    if (device->pp_blocks != NULL)
    { /* Already initialized, success */
        goto exit;
    }

    if((rv = sand_signal_struct_init(device)) != _SHR_E_NONE)
        goto exit;;

    if ((curTop = dbx_file_get_xml_top(RHNAME(device), "PP.xml", "top", 0)) == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META("No Pipeline Scheme found\n")));
        rv = _SHR_E_NOT_FOUND;
        goto exit;
    }

    if ((curSubTop = dbx_xml_child_get_first(curTop, "block-list")) == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META("No block-list in Pipeline Scheme\n")));
        rv = _SHR_E_INTERNAL;
        goto exit;
    }

    RHDATA_GET_NODE_NUM(device->block_num, curSubTop, "block");

    if((cur_pp_block = device->pp_blocks = utilex_alloc(sizeof(pp_block_t) * device->block_num)) == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META_U(unit, "No memory for %s Blocks Data\n"), SOC_CHIP_STRING(unit)));
        rv = _SHR_E_MEMORY;
        goto exit;
    }
    /*
     * loop through entries
     */
    RHDATA_ITERATOR(curBlock, curSubTop, "block")
    {
        /*
         * Verify that we are inside the limitations MAX_NUM
         */
        RHDATA_GET_STR_CONT(curBlock, "name", cur_pp_block->name);
        RHDATA_GET_STR_DEF_NULL(curBlock, "debug-signals", cur_pp_block->debug_signals_n);

        /*
         * In case of c-model obtain the list of stages and add here
         */
#ifdef CMODEL_SERVER_MODE
        if((rv = sand_cmodel_get_stages(unit, cur_pp_block)) != _SHR_E_NONE)
            goto exit;
#else
        {
            void *curSignalTop, *curSignal, *cur, *curStage;
            int size;
            char full_address[DSIG_ADDRESS_MAX_SIZE];
            pp_stage_t *cur_pp_stage;
            char temp[RHNAME_MAX_SIZE];

            RHDATA_GET_NODE_NUM(cur_pp_block->stage_num, curBlock, "stage")
            if(cur_pp_block->stage_num == 0)
                continue;

            if((cur_pp_block->stages = utilex_alloc(sizeof(pp_stage_t) * cur_pp_block->stage_num)) == NULL)
            {
                LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META("No memory for stages\n")));
                rv = _SHR_E_MEMORY;
                goto exit;
            }
            cur_pp_stage = cur_pp_block->stages;
            /*
             * loop through entries
             */
            RHDATA_ITERATOR(curStage, curBlock, "stage")
            {
                RHDATA_GET_STR_CONT(curStage,     "name",             cur_pp_stage->name);
                RHDATA_GET_STR_DEF_NULL(curStage, "programmable",     cur_pp_stage->programmable);
                RHDATA_GET_STR_DEF_NULL(curStage, "internal-signals", temp);
                if (!ISEMPTY(temp))
                {
                    sand_qual_signal_init(RHNAME(device), temp, cur_pp_stage);
                }
                cur_pp_stage++;
            }

            if (ISEMPTY(cur_pp_block->debug_signals_n))     /* No debug signals for this block */
                continue;

            if ((curSignalTop =
                 dbx_file_get_xml_top(RHNAME(device), cur_pp_block->debug_signals_n, "SignalInfo", 0)) == NULL)
            {
                LOG_ERROR(BSL_LS_SOC_COMMON, (BSL_META("No Signal Info found in:%s\n"), cur_pp_block->debug_signals_n));
                rv = _SHR_E_NOT_FOUND;
                goto exit;
            }

            RHDATA_GET_NODE_NUM(cur_pp_block->signal_num, curSignalTop, "Signal");
            /*
             * It is like there are no debug signals for this block
             */
            if (cur_pp_block->signal_num == 0)
                continue;

            debug_signal = utilex_alloc(sizeof(debug_signal_t) * cur_pp_block->signal_num);

            cur_pp_block->debug_signals = debug_signal;

            /*
             * loop through entries
             */
            RHDATA_ITERATOR(curSignal, curSignalTop, "Signal")
            {
                dbx_xml_child_get_content_str(curSignal, "Name", debug_signal->hw, RHFILE_MAX_SIZE);
                dbx_xml_child_get_content_int(curSignal, "Size", &debug_signal->size);
                dbx_xml_child_get_content_int(curSignal, "BlockID", &debug_signal->block_id);
                dbx_xml_child_get_content_str(curSignal, "Changeable", temp, RHNAME_MAX_SIZE);
                dbx_xml_child_get_content_str(curSignal, "Attribute", debug_signal->attribute, RHNAME_MAX_SIZE);
                dbx_xml_child_get_content_int(curSignal, "Perm", &debug_signal->perm);
                cur = dbx_xml_child_get_content_str(curSignal, "Condition", debug_signal->cond_attribute, RHNAME_MAX_SIZE);
                if ((cur != NULL) && !ISEMPTY(debug_signal->cond_attribute))
                {
                    RHDATA_GET_INT_DEF(cur, "Value", debug_signal->cond_value, 1);
                }

                debug_signal->changeable = (sal_strcasecmp(temp, "Yes") ? 0 : 1);

                dbx_xml_child_get_content_str(curSignal, "Expansion", debug_signal->expansion, RHNAME_MAX_SIZE);
                /*
                 * No explicit expansion - look for implicit one
                 */
                if (!ISEMPTY(debug_signal->expansion))
                {
                    if ((sand_signal_struct_get(device, debug_signal->expansion)) == NULL)
                    {
                        LOG_INFO(BSL_LS_SOC_COMMON, (BSL_META("No signal expansion:%s\n"), debug_signal->expansion));
                        SET_EMPTY(debug_signal->expansion);
                    }
                }
                else
                {
                    if ((sand_signal_struct_get(device, debug_signal->attribute)) != NULL)
                        /*
                         * If there is match - attribute serves as expansion
                         */
                        strcpy(debug_signal->expansion, debug_signal->attribute);
                }

                dbx_xml_child_get_content_str(curSignal, "Resolution", debug_signal->resolution, RHNAME_MAX_SIZE);
                if (!ISEMPTY(debug_signal->resolution))
                {
                    if ((sand_signal_resolve_get(device, debug_signal->resolution)) == NULL)
                    {
                        LOG_INFO(BSL_LS_SOC_COMMON, (BSL_META("Signal resolution:%s does not exist\n"),  debug_signal->resolution));
                        SET_EMPTY(debug_signal->resolution);
                    }
                }
                else
                {
                    if ((sand_signal_resolve_get(device, debug_signal->attribute)) != NULL)
                        /*
                         * If there is match - attribute serves as resolution param name
                         */
                        strcpy(debug_signal->resolution, debug_signal->attribute);
                }

                cur = dbx_xml_child_get_content_str(curSignal, "Double", temp, RHNAME_MAX_SIZE);
                if ((cur != NULL) && !ISEMPTY(temp))
                    debug_signal->double_flag = 1;

                dbx_xml_child_get_content_str(curSignal, "From", debug_signal->from, RHNAME_MAX_SIZE);
                dbx_xml_child_get_content_str(curSignal, "To", debug_signal->to, RHNAME_MAX_SIZE);
                strcpy(debug_signal->block_n, cur_pp_block->name);
                dbx_xml_child_get_content_str(curSignal, "Address", full_address, DSIG_ADDRESS_MAX_SIZE);
                debug_signal->range_num = sand_signal_address_parse(full_address, debug_signal->address);
                /*
                 * Verify consistency between size and sum of all bits in range
                 */
                size = 0;
                for (i = 0; i < debug_signal->range_num; i++)
                    size += (debug_signal->address[i].msb + 1 - debug_signal->address[i].lsb);

                if (size != debug_signal->size)
                {
                    cli_out("Correcting size for:%s from:%d to %d\n", debug_signal->attribute, debug_signal->size, size);
                    debug_signal->size = size;
                }

                debug_signal++;
            }

            dbx_xml_top_close(curSignalTop);
        }
#endif /* CMODEL_SERVER_MODE */
        cur_pp_block++;
    }
    /*
     * Now we need to extract condition attribute
     */
    for (i = 0; i < device->block_num; i++)
    {
        cur_pp_block = &device->pp_blocks[i];
        debug_signal = cur_pp_block->debug_signals;
        for (j = 0; j < cur_pp_block->signal_num; j++)
        {
            if (!ISEMPTY(debug_signal->cond_attribute))
            {
                if(sand_signal_handle_get(device, debug_signal->block_n,
                                             debug_signal->from, debug_signal->to,
                                             debug_signal->cond_attribute, &debug_signal->cond_signal) == 0)
                {
                    cli_out("Condition Attribute:%s does not exist for:%s:%s -> %s\n",
                            debug_signal->cond_attribute, debug_signal->block_n, debug_signal->from, debug_signal->to);
                }
            }
            debug_signal++;
        }
    }

    dbx_xml_top_close(curTop);
exit:
    return rv;
}
/*
 * }
 */
#endif /* defined(NO_FILEIO) */

device_t *
sand_signal_device_get(
    int unit)
{
    if(sand_device_array[unit] == NULL) {
        LOG_WARN(BSL_LS_SOC_COMMON,
              (BSL_META_U(0, "Signal Data for %s(%s) was not initialized\n"), soc_dev_name(unit), SOC_CHIP_STRING(unit)));
    }
    return sand_device_array[unit];
}

shr_error_e
shr_access_device_init(
    int unit)
{
    shr_error_e rv = _SHR_E_NONE;
#if defined(NO_FILEIO)
    if((sand_device_array[unit] = sand_signal_static_init(unit)) == NULL)
    {
        rv = _SHR_E_NOT_FOUND;
        goto exit;
    }
#else
    device_t *device;
    char *chip_n = (char *) soc_dev_name(unit);

    /* Initialize global DB list with supported chips */
    if((rv = shr_access_global_init()) != _SHR_E_NONE)
        goto exit;

    /* Look in the list by specific device ID */
    RHITERATOR(device, sand_db_list)
    {
        if ((utilex_rhlist_entry_get_by_name(device->chip_list, chip_n)) != NULL)
        {
            sand_device_array[unit] = device;
            break;
        }
    }

    /* Look in the list by family name */
    if (sand_device_array[unit] == NULL)
    {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(0, "%s was not found in Device DB - looking by:%s\n"), chip_n, SOC_CHIP_STRING(unit)));
        RHITERATOR(device, sand_db_list)
        {
            if ((utilex_rhlist_entry_get_by_name(device->chip_list, SOC_CHIP_STRING(unit))) != NULL)
            {
                sand_device_array[unit] = device;
                break;
            }
        }
    }

    if (sand_device_array[unit] == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(0, "No %s neither %s was found in Device DB\n"), chip_n, SOC_CHIP_STRING(unit)));
        rv = _SHR_E_NOT_FOUND;
        goto exit;
    }

    /* Initialize Access Objects */
    if ((rv = shr_access_obj_init(sand_device_array[unit])) != _SHR_E_NONE)
        goto exit;

    /* Initialize Signals Data */
    if ((rv = sand_signal_init(unit, sand_device_array[unit])) != _SHR_E_NONE)
        goto exit;

#endif /* NO_FILEIO */
exit:
    return rv;
}

#endif /* BCM_SAND_SUPPORT */
