/*
 * $Id: mem.c,v 1.39 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:       mem.c
 * Purpose:    Diag shell memory commands for DPP
 */

#include <shared/bsl.h>
#include <ibde.h>

#include <sal/appl/pci.h>

#include <soc/mem.h>
#include <soc/cm.h>

#include <soc/dcmn/dcmn_mem.h>

#include <soc/dcmn/dcmn_intr_corr_act_func.h>

#include <appl/diag/system.h>
#include <appl/diag/bslcons.h>
#include <appl/diag/bslfile.h>
#include <appl/dcmn/interrupts/dcmn_intr.h>

#include <appl/diag/sand/diag_sand_utils.h>
#include <appl/diag/sand/diag_sand_mem.h>
#include <appl/diag/sand/diag_sand_reg.h>

#define DUMP_TABLE_RAW          0x01
#define DUMP_TABLE_HEX          0x02
#define DUMP_TABLE_ALL          0x04
#define DUMP_TABLE_CHANGED      0x08
#define DUMP_DISABLE_CACHE      0x10

static void diag_sand_mem_entry_dump_field(
    int unit,
    soc_mem_t mem,
    void *buf,
    char *field_name);

static char *diag_sand_int_corr_act_name[] = {
    "Clear Check",
    "Config DRAM",
    "ECC 1b Correct",
    "EM Soft Recovery",
    "Force",
    "Handle CRC Delete Buffer FIFO",
    "Handle MACT Event FIFO",
    "Handle OAMP Event Fifo",
    "Handle OAMP Statistics Event Fifo",
    "Hard Reset",
    "Hard Reset without Fabric",
    "EM Soft Recovery",
    "EM Soft Recovery",
    "Ingress Hard Reset",
    "IPS QDESC Clear Unused",
    "None",
    "EM Soft Recovery",
    "Print",
    "Reprogram Resource",
    "RTP Link Mask Change",
    "RX LOS Handle",
    "According to SER algorithm",
    "Shadow and Soft Reset",
    "Shutdown link",
    "Shutdown Unreachable Destination",
    "Soft Reset",
    "TCAM Shadow from SW DB",
    "RTP SLSCT",
    "Shutdown links",
    "MC RTP Correct",
    "UC RTP Correct",
    "All Rechable fix",
    "Event Ready",
    "Unknown"
};

/*
 * TYPE DEFS
 */

/*
 * Utility routine to concatenate the first argument ("first"), with
 * the remaining arguments, with commas separating them.
 */
#define MAX_MEM_SIZE   100000

STATIC void
diag_sand_check_global(
    int unit,
    soc_mem_t mem,
    char *s,
    int *is_global)
{
    soc_field_info_t *fld;
    soc_mem_info_t *m = &SOC_MEM_INFO(unit, mem);
    char *eqpos;

    eqpos = strchr(s, '=');
    if (eqpos != NULL)
    {
        *eqpos++ = 0;
    }
    for (fld = &m->fields[0]; fld < &m->fields[m->nFields]; fld++)
    {
        if (!sal_strcasecmp(s, SOC_FIELD_NAME(unit, fld->field)) && (fld->flags & SOCF_GLOBAL))
        {
            break;
        }
    }
    if (fld == &m->fields[m->nFields])
    {
        *is_global = 0;
    }
    else
    {
        *is_global = 1;
    }
}

STATIC int
diag_sand_collect_args_with_view(
    args_t * a,
    char *valstr,
    char *first,
    char *view,
    int unit,
    soc_mem_t mem)
{
    char *s, *s_copy = NULL, *f_copy = NULL;
    int is_global, rv = 0;

    if ((f_copy = sal_alloc(strlen(first) + 1, "first")) == NULL)
    {
        LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "diag_sand_collect_args_with_view : Out of memory\n")));
        rv = -1;
        goto done;
    }
    memset(f_copy, 0, strlen(first) + 1);
    sal_strcpy(f_copy, first);

    /*
     * Check if field is global before applying view prefix 
     */
    diag_sand_check_global(unit, mem, f_copy, &is_global);
    if (!is_global)
    {
        sal_strcpy(valstr, view);
        strcat(valstr, first);
    }
    else
    {
        sal_strcpy(valstr, first);
    }

    while ((s = ARG_GET(a)) != 0)
    {
        if ((s_copy = sal_alloc(strlen(s) + 1, "s_copy")) == NULL)
        {
            LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "diag_sand_collect_args_with_view : Out of memory\n")));
            rv = -1;
            goto done;
        }
        memset(s_copy, 0, strlen(s) + 1);
        sal_strcpy(s_copy, s);
        diag_sand_check_global(unit, mem, s_copy, &is_global);
        sal_free(s_copy);
        strcat(valstr, ",");
        if (!is_global)
        {
            strcat(valstr, view);
            strcat(valstr, s);
        }
        else
        {
            strcat(valstr, s);
        }
    }
done:
    if (f_copy != NULL)
    {
        sal_free(f_copy);
    }
    return rv;
}

/*
 * diag_sand_mem_modify_fields
 *
 *   Verify similar to modify_reg_fields (see reg.c) but works on
 *   memory table entries instead of register values.  Handles fields
 *   of any length.
 *
 *   If mask is non-NULL, it receives an entry which is a mask of all
 *   fields modified.
 *
 *   Values may be specified with optional increment or decrement
 *   amounts; for example, a MAC address could be 0x1234+2 or 0x1234-1
 *   to specify an increment of +2 or -1, respectively.
 *
 *   If incr is FALSE, the increment is ignored and the plain value is
 *   stored in the field (e.g. 0x1234).
 *
 *   If incr is TRUE, the value portion is ignored.  Instead, the
 *   increment value is added to the existing value of the field.  The
 *   field value wraps around on overflow.
 *
 *   Returns -1 on failure, 0 on success.
 */

STATIC int
diag_sand_mem_modify_fields(
    int unit,
    soc_mem_t mem,
    uint32 * entry,
    uint32 * mask,
    char *mod,
    int incr)
{
    soc_field_info_t *fld;
    char *fmod, *fval, *s;
    char *modstr = NULL;
    uint32 fvalue[SOC_MAX_MEM_FIELD_WORDS];
    uint32 fincr[SOC_MAX_MEM_FIELD_WORDS];
    int i, entry_dw;
    soc_mem_info_t *m = &SOC_MEM_INFO(unit, mem);
    char *tokstr;

    entry_dw = BYTES2WORDS(m->bytes);
    if ((modstr = sal_alloc(ARGS_BUFFER, "modify_mem")) == NULL)
    {
        LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "diag_sand_mem_modify_fields : Out of memory\n")));
        return CMD_FAIL;
    }

    strncpy(modstr, mod, ARGS_BUFFER);  /* Don't destroy input string */
    modstr[ARGS_BUFFER - 1] = 0;
    mod = modstr;

    if (mask)
    {
        memset(mask, 0, entry_dw * 4);
    }

    while ((fmod = sal_strtok_r(mod, ",", &tokstr)) != 0)
    {
        mod = NULL;     /* Pass strtok NULL next time */
        fval = strchr(fmod, '=');
        if (fval != NULL)
        {       /* Point fval to arg, NULL if none */
            *fval++ = 0;        /* Now fmod holds only field name. */
        }
        if (fmod[0] == 0)
        {
            LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Null field name\n")));
            sal_free(modstr);
            return -1;
        }
        if (!sal_strcasecmp(fmod, "clear"))
        {
            memset(entry, 0, entry_dw * sizeof(*entry));
            if (mask)
            {
                memset(mask, 0xff, entry_dw * sizeof(*entry));
            }
            continue;
        }
        for (fld = &m->fields[0]; fld < &m->fields[m->nFields]; fld++)
        {
            if (!sal_strcasecmp(fmod, SOC_FIELD_NAME(unit, fld->field)))
            {
                break;
            }
        }
        if (fld == &m->fields[m->nFields])
        {
            LOG_INFO(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit, "No such field \"%s\" in memory \"%s\".\n"), fmod, SOC_MEM_UFNAME(unit, mem)));
            sal_free(modstr);
            return -1;
        }
        if (!fval)
        {
            LOG_INFO(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit, "Missing %d-bit value to assign to \"%s\" field \"%s\".\n"), fld->len,
                      SOC_MEM_UFNAME(unit, mem), SOC_FIELD_NAME(unit, fld->field)));
            sal_free(modstr);
            return -1;
        }
        s = strchr(fval, '+');
        if (s == NULL)
        {
            s = strchr(fval, '-');
        }
        if (s == fval)
        {
            s = NULL;
        }
        if (incr)
        {
            if (s != NULL)
            {
                parse_long_integer(fincr, SOC_MAX_MEM_FIELD_WORDS, s[1] ? &s[1] : "1");
                if (*s == '-')
                {
                    neg_long_integer(fincr, SOC_MAX_MEM_FIELD_WORDS);
                }
                if (fld->len & 31)
                {
                    /*
                     * Proper treatment of sign extension 
                     */
                    fincr[fld->len / 32] &= ~(0xffffffff << (fld->len & 31));
                }
                soc_mem_field_get(unit, mem, entry, fld->field, fvalue);
                add_long_integer(fvalue, fincr, SOC_MAX_MEM_FIELD_WORDS);
                if (fld->len & 31)
                {
                    /*
                     * Proper treatment of sign extension 
                     */
                    fvalue[fld->len / 32] &= ~(0xffffffff << (fld->len & 31));
                }
                soc_mem_field_set(unit, mem, entry, fld->field, fvalue);
            }
        }
        else
        {
            if (s != NULL)
            {
                *s = 0;
            }
            parse_long_integer(fvalue, SOC_MAX_MEM_FIELD_WORDS, fval);
            for (i = fld->len; i < SOC_MAX_MEM_FIELD_BITS; i++)
            {
                if (fvalue[i / 32] & 1 << (i & 31))
                {
                    LOG_INFO(BSL_LS_APPL_COMMON,
                             (BSL_META_U(unit, "Value \"%s\" too large for %d-bit field \"%s\".\n"), fval, fld->len,
                              SOC_FIELD_NAME(unit, fld->field)));
                    sal_free(modstr);
                    return -1;
                }
            }
            soc_mem_field_set(unit, mem, entry, fld->field, fvalue);
        }
        if (mask)
        {
            memset(fvalue, 0, sizeof(fvalue));
            for (i = 0; i < fld->len; i++)
            {
                fvalue[i / 32] |= 1 << (i & 31);
            }
            soc_mem_field_set(unit, mem, mask, fld->field, fvalue);
        }
    }

    sal_free(modstr);
    return 0;
}

STATIC int
diag_sand_parse_dwords(
    int unit,
    int count,
    uint32 * dw,
    args_t * a)
{
    char *s;
    int i;

    for (i = 0; i < count; i++)
    {
        if ((s = ARG_GET(a)) == NULL)
        {
            LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Not enough data values (have %d, need %d)\n"), i, count));
            return -1;
        }
        dw[i] = parse_integer(s);
    }

    if (ARG_CNT(a) > 0)
    {
        LOG_INFO(BSL_LS_APPL_COMMON,
                 (BSL_META_U(unit, "Ignoring extra data on command line (only %d words needed)\n"), count));
    }

    return 0;
}

/*
 * Print a one line summary for matching memories
 * If substr_match is NULL, match all memories.
 * If substr_match is non-NULL, match any memories whose name
 * or user-friendly name contains that substring.
 */
STATIC void
diag_sand_mem_list_summary(
    int unit,
    char *substr_match)
{
    soc_mem_t mem;
    int i, copies, dlen;
    int found = 0;
    char *dstr;

    bsl_log_start(BSL_APPL_SHELL, bslSeverityNormal, unit, "");
    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        if (!soc_mem_is_valid(unit, mem))
        {
            continue;
        }

        if (substr_match != NULL &&
            strcaseindex(SOC_MEM_NAME(unit, mem), substr_match) == NULL &&
            strcaseindex(SOC_MEM_UFNAME(unit, mem), substr_match) == NULL)
        {
            continue;
        }

        copies = 0;
        SOC_MEM_BLOCK_ITER(unit, mem, i)
        {
            copies += 1;
        }

        dlen = strlen(SOC_MEM_DESC(unit, mem));
        if (dlen > 38)
        {
            dlen = 34;
            dstr = "...";
        }
        else
        {
            dstr = "";
        }
        if (!found)
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                        " %-6s  %-22s%5s/%-4s %s\n", "Flags", "Name", "Entry", "Copy", "Description");
            found = 1;
        }

        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                    " %c%c%c%c%c%c  %-22s %5d",
                    soc_mem_is_readonly(unit, mem) ? 'r' : '-',
                    soc_mem_is_debug(unit, mem) ? 'd' : '-',
                    soc_mem_is_sorted(unit, mem) ? 's' :
                    soc_mem_is_hashed(unit, mem) ? 'h' :
                    soc_mem_is_cam(unit, mem) ? 'A' : '-',
                    soc_mem_is_cbp(unit, mem) ? 'c' : '-',
                    (soc_mem_is_bistepic(unit, mem) ||
                     soc_mem_is_bistffp(unit, mem) ||
                     soc_mem_is_bistcbp(unit, mem)) ? 'b' : '-',
                    soc_mem_is_cachable(unit, mem) ? 'C' : '-',
                    SOC_MEM_UFNAME(unit, mem), soc_mem_index_count(unit, mem));
        if (copies == 1)
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%5s %*.*s%s\n",
                        "", dlen, dlen, SOC_MEM_DESC(unit, mem), dstr);
        }
        else
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "/%-4d %*.*s%s\n",
                        copies, dlen, dlen, SOC_MEM_DESC(unit, mem), dstr);
        }
    }

    if (found)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                    "Flags: (r)eadonly, (d)ebug, (s)orted, (h)ashed\n"
                    "       C(A)M, (c)bp, (b)ist-able, (C)achable\n");
    }
    else if (substr_match != NULL)
    {
        LOG_INFO(BSL_LS_APPL_COMMON,
                 (BSL_META_U(unit, "No memory found with the substring '%s' in its name.\n"), substr_match));
    }
    bsl_log_end(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "");

}

STATIC int
diag_sand_mem_list_cache(
    int unit,
    soc_mem_t mem,
    void *en)
{
    if (soc_mem_cache_get(unit, mem, COPYNO_ALL))
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "  %s \n", SOC_MEM_NAME(unit, mem));
    }

    return BCM_E_NONE;
}

STATIC int
diag_sand_mem_list_dynamic(
    int unit,
    soc_mem_t mem,
    void *en)
{
    if (dcmn_tbl_is_dynamic(unit, mem))
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "  %s \n", SOC_MEM_NAME(unit, mem));
    }

    return BCM_E_NONE;
}

STATIC int
diag_sand_active_intr_check(
    int unit,
    int inter)
{
#if defined(INCLUDE_INTR)
    int i;
    uint32 *int_active_on;

    int_active_on = interrupt_active_on_intr_get(unit);
    if (int_active_on == NULL)
    {
        return 0;
    }

    for (i = 0; int_active_on[i] != INVALIDr; i++)
    {
        if (int_active_on[i] == inter)
        {
            return 1;
        }
    }
#endif

    return 0;
}

STATIC int
diag_sand_mem_list_ser_action(
    int unit)
{
    int nof_interrupts;
    int inter = 0;
    soc_interrupt_db_t *interrupts;
    int rc = BCM_E_NONE;
    int i, act_action, recur_action;
    dcmn_intr_action_t *dcmn_intr_action_info = NULL;
    uint32 is_enable = 0;
    bcm_switch_event_control_t bcm_switch_event_control;

    dcmn_intr_action_info = dcmn_intr_action_info_get(unit);
    if (dcmn_intr_action_info == NULL)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                    "This command does not support on this device.\n");
        return CMD_OK;
    }

    interrupts = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info;
    if (NULL == interrupts)
    {
        LOG_ERROR(BSL_LS_APPL_SOCMEM, (BSL_META_U(unit, "No interrupts for device.\n")));
        return CMD_FAIL;
    }

    rc = soc_nof_interrupts(unit, &nof_interrupts);
    if (rc != SOC_E_NONE)
    {
        return CMD_FAIL;
    }

    bcm_switch_event_control.action = bcmSwitchEventMask;
    bcm_switch_event_control.index = 0;

#if defined(SOC_NO_NAMES)
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                "Interrupt Id, Enable, SER Action                         , Cycle Time, Cycle Count, Recurring Action \n");
#else
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                "Interrupt Name                                                   , Enable, SER Action                         , Cycle Time, Cycle Count, Recurring Action \n");
#endif

    for (inter = 0; inter < nof_interrupts; inter++)
    {
        is_enable = 0;
        act_action = DCMN_INT_CORR_ACT_NONE;
        recur_action = DCMN_INT_CORR_ACT_NONE;

        if (diag_sand_active_intr_check(unit, inter))
        {
            bcm_switch_event_control.event_id = inter;
            rc = bcm_switch_event_control_get(unit, BCM_SWITCH_EVENT_DEVICE_INTERRUPT, bcm_switch_event_control,
                                              &is_enable);
            if (rc != BCM_E_NONE)
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                            " Get Interrupt %s status failed.\n", interrupts[inter].name);
                continue;
            }
        }

        if (interrupts[inter].func_arr)
        {
            for (i = 0; dcmn_intr_action_info[i].func_arr != NULL; i++)
            {
                if (interrupts[inter].func_arr == dcmn_intr_action_info[i].func_arr)
                {
                    act_action = dcmn_intr_action_info[i].corr_action;
                    break;
                }
            }
        }

        if (interrupts[inter].func_arr_recurring_action)
        {
            for (i = 0; dcmn_intr_action_info[i].func_arr != NULL; i++)
            {
                if (interrupts[inter].func_arr_recurring_action == dcmn_intr_action_info[i].func_arr)
                {
                    recur_action = dcmn_intr_action_info[i].corr_action;
                    break;
                }
            }
        }

#if defined(SOC_NO_NAMES)
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%-12d, %-6d, %-35s, %-10d, %-11d, %s\n",
                    inter, is_enable,
                    diag_sand_int_corr_act_name[act_action],
                    interrupts[inter].recurring_action_cycle_time,
                    interrupts[inter].recurring_action_cycle_counting, dcmn_int_corr_act_name[recur_action]);
#else
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%-65s, %-6d, %-35s, %-10d, %-11d, %s\n",
                    interrupts[inter].name, is_enable,
                    diag_sand_int_corr_act_name[act_action],
                    interrupts[inter].recurring_action_cycle_time,
                    interrupts[inter].recurring_action_cycle_counting, diag_sand_int_corr_act_name[recur_action]);
#endif
    }

    return CMD_OK;
}

/*
 * List the tables, or fields of a table entry
 */
char cmd_sand_mem_list_usage[] =
    "Parameters: [<TABLE> [<DATA> ...]]\n\t"
    "If no parameters are given, displays a reference list of all\n\t"
    "memories and their attributes.\n\t"
    "If TABLE is given, displays the entry fields for that table.\n\t"
    "If TABLE is cache-table, displays all cached tables.\n\t"
    "If TABLE is ser-action, displays all SER action.\n\t"
    "If TABLE is dynamic-table, displays all dynamic tables.\n\t"
    "If DATA is given, decodes the data into entry fields.\n";

cmd_result_t
cmd_sand_mem_list(
    int unit,
    args_t * a)
{
    soc_mem_info_t *m;
    soc_field_info_t *fld;
    char *tab, *s;
    soc_mem_t mem;
    uint32 entry[SOC_MAX_MEM_WORDS];
    uint32 mask[SOC_MAX_MEM_WORDS];
    int have_entry, i, dw, copyno;
    int copies, disabled, dmaable;
    char *dmastr;
    uint32 flags;
    int minidx, maxidx;
    uint8 acc_type;

    if (!sh_check_attached(ARG_CMD(a), unit))
    {
        return CMD_FAIL;
    }

    if (!soc_property_get(unit, spn_MEMLIST_ENABLE, 1))
    {
        return CMD_OK;
    }

    tab = ARG_GET(a);

    if ((tab != NULL) && (sal_strcasecmp(tab, "cache-table") == 0))
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Current enabled cached memories: \n");
        if (soc_mem_iterate(unit, diag_sand_mem_list_cache, NULL) < 0)
        {
            LOG_ERROR(BSL_LS_APPL_SOCMEM, (BSL_META_U(unit, "ERROR: Get all cache tables error.\n")));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if ((tab != NULL) && (sal_strcasecmp(tab, "ser-action") == 0))
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Current all SER action: \n");
        if (diag_sand_mem_list_ser_action(unit) < 0)
        {
            LOG_ERROR(BSL_LS_APPL_SOCMEM, (BSL_META_U(unit, "ERROR: List SER action error.\n")));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if ((tab != NULL) && (sal_strcasecmp(tab, "dynamic-table") == 0))
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Current dynamic memories: \n");
        if (soc_mem_iterate(unit, diag_sand_mem_list_dynamic, NULL) < 0)
        {
            LOG_ERROR(BSL_LS_APPL_SOCMEM, (BSL_META_U(unit, "ERROR: Get all dynamic tables error.\n")));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (!tab)
    {
        diag_sand_mem_list_summary(unit, NULL);
        return CMD_OK;
    }

    if (parse_memory_name(unit, &mem, tab, &copyno, 0) < 0)
    {
        if ((s = strchr(tab, '.')) != NULL)
        {
            *s = 0;
        }
        diag_sand_mem_list_summary(unit, tab);
        return CMD_OK;
    }

    bsl_log_start(BSL_APPL_SHELL, bslSeverityNormal, unit, "");
    if (!SOC_MEM_IS_VALID(unit, mem))
    {
        LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "ERROR: Memory \"%s\" not valid for this unit\n"), tab));
        return CMD_FAIL;
    }

    if (copyno < 0)
    {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    else if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno))
    {
        LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "ERROR: Invalid copy number %d for memory %s\n"), copyno, tab));
        return CMD_FAIL;
    }

    m = &SOC_MEM_INFO(unit, mem);
    flags = m->flags;

    dw = BYTES2WORDS(m->bytes);

    if ((s = ARG_GET(a)) == 0)
    {
        have_entry = 0;
    }
    else
    {
        for (i = 0; i < dw; i++)
        {
            if (s == 0)
            {
                LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Not enough data specified (%d words needed)\n"), dw));
                return CMD_FAIL;
            }
            entry[i] = parse_integer(s);
            s = ARG_GET(a);
        }
        if (s)
        {
            LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Extra data specified (ignored)\n")));
        }
        have_entry = 1;
    }

    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Memory: %s.%s", SOC_MEM_UFNAME(unit, mem),
                SOC_BLOCK_NAME(unit, copyno));
    s = SOC_MEM_UFALIAS(unit, mem);
    if (s && *s && strcmp(SOC_MEM_UFNAME(unit, mem), s) != 0)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " alias %s \n", s);
    }
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " address 0x%08x\n",
                soc_mem_addr_get(unit, mem, 0, copyno, 0, &acc_type));

    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Flags:");
    if (flags & SOC_MEM_FLAG_READONLY)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " read-only");
    }

    if (flags & SOC_MEM_FLAG_WRITEONLY)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " write-only");
    }
    if (flags & SOC_MEM_FLAG_SIGNAL)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " signal/dynamic");
    }

    if (flags & SOC_MEM_FLAG_VALID)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " valid");
    }
    if (flags & SOC_MEM_FLAG_DEBUG)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " debug");
    }
    if (flags & SOC_MEM_FLAG_SORTED)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " sorted");
    }
    if (flags & SOC_MEM_FLAG_CBP)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " cbp");
    }
    if (flags & SOC_MEM_FLAG_CACHABLE)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " cachable");
    }
    if (flags & SOC_MEM_FLAG_BISTCBP)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " bist-cbp");
    }
    if (flags & SOC_MEM_FLAG_BISTEPIC)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " bist-epic");
    }
    if (flags & SOC_MEM_FLAG_BISTFFP)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " bist-ffp");
    }
    if (flags & SOC_MEM_FLAG_UNIFIED)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " unified");
    }
    if (flags & SOC_MEM_FLAG_HASHED)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " hashed");
    }
    if (flags & SOC_MEM_FLAG_WORDADR)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " word-addressed");
    }
    if (flags & SOC_MEM_FLAG_BE)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " big-endian");
    }
    if (flags & SOC_MEM_FLAG_IS_ARRAY)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "  array[0-%u]",
                    SOC_MEM_NUMELS(unit, mem) - 1);

    }
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "\n");

    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Blocks: ");
    copies = disabled = dmaable = 0;
    SOC_MEM_BLOCK_ITER(unit, mem, i)
    {
        if (SOC_INFO(unit).block_valid[i])
        {
            dmastr = "";
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " %s%s", SOC_BLOCK_NAME(unit, i),
                        dmastr);
        }
        else
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " [%s]", SOC_BLOCK_NAME(unit, i));
            disabled += 1;
        }
        copies += 1;
    }
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " (%d cop%s", copies,
                copies == 1 ? "y" : "ies");
    if (disabled)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, ", %d disabled", disabled);
    }
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, ")\n");

    minidx = soc_mem_index_min(unit, mem);
    maxidx = soc_mem_index_max(unit, mem);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Entries: %d with indices %d-%d (0x%x-0x%x)",
                maxidx - minidx + 1, minidx, maxidx, minidx, maxidx);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, ", each %d bytes %d words\n", m->bytes, dw);

    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Entry mask:");
    soc_mem_datamask_get(unit, mem, mask);
    for (i = 0; i < dw; i++)
    {
        if (mask[i] == 0xffffffff)
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " -1");
        }
        else if (mask[i] == 0)
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " 0");
        }
        else
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " 0x%08x", mask[i]);
        }
    }
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "\n");

    s = SOC_MEM_DESC(unit, mem);
    if (s && *s)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "Description: %s\n", s);
    }

    for (fld = &m->fields[m->nFields - 1]; fld >= &m->fields[0]; fld--)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "  %s<%d",
                    SOC_FIELD_NAME(unit, fld->field), fld->bp + fld->len - 1);
        if (fld->len > 1)
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, ":%d", fld->bp);
        }
        if (have_entry)
        {
            uint32 fval[SOC_MAX_MEM_FIELD_WORDS];
            char tmp[132];

            memset(fval, 0, sizeof(fval));
            soc_mem_field_get(unit, mem, entry, fld->field, fval);
            format_long_integer(tmp, fval, SOC_MAX_MEM_FIELD_WORDS);
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "> = %s\n", tmp);
        }
        else
        {
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, ">\n");
        }
    }
    bsl_log_end(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "");
    return CMD_OK;
}

static cmd_result_t
diag_sand_do_dump_table(
    int unit,
    soc_mem_t mem,
    unsigned array_index,
    int copyno,
    int index,
    int count,
    int flags)
{
    int k, i;
    uint32 entry[SOC_MAX_MEM_WORDS];
    char lineprefix[256];
    int entry_dw;
    int rv = CMD_FAIL;

    assert(copyno >= 0);

    entry_dw = soc_mem_entry_words(unit, mem);

    bsl_log_start(BSL_APPL_SHELL, bslSeverityNormal, unit, "");

    for (k = index; k < index + count; k++)
    {

        if (!(flags & DUMP_DISABLE_CACHE))
        {
            i = soc_mem_array_read(unit, mem, array_index, copyno, k, entry);
        }
        else
        {
            i = soc_mem_array_read_flags(unit, mem, array_index, copyno, k, entry, SOC_MEM_DONT_USE_CACHE);
        }
        if (i < 0)
        {
            LOG_INFO(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit, "Read ERROR: table %s.%s[%d]: %s\n"), SOC_MEM_UFNAME(unit, mem),
                      SOC_BLOCK_NAME(unit, copyno), k, soc_errmsg(i)));
            goto done;
        }

        if (!(flags & DUMP_TABLE_ALL))
        {
            
        }

        if (flags & DUMP_TABLE_HEX)
        {
            for (i = 0; i < entry_dw; i++)
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%08x\n", entry[i]);
            }
        }
        else if (flags & DUMP_TABLE_CHANGED)
        {
            if (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_IS_ARRAY)
            {
                sal_sprintf(lineprefix, "%s[%d].%s[%d]: ", SOC_MEM_UFNAME(unit, mem), array_index,
                            SOC_BLOCK_NAME(unit, copyno), k);
            }
            else
            {
                sal_sprintf(lineprefix, "%s.%s[%d]: ", SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno), k);
            }
            soc_mem_entry_dump_if_changed(unit, mem, entry, lineprefix);
        }
        else
        {
            if (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_IS_ARRAY)
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%s[%d].%s[%d]: ",
                            SOC_MEM_UFNAME(unit, mem), array_index, SOC_BLOCK_NAME(unit, copyno), k);
            }
            else
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%s.%s[%d]: ",
                            SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno), k);
            }
            if (flags & DUMP_TABLE_RAW)
            {
                for (i = 0; i < entry_dw; i++)
                {
                    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "0x%08x ", entry[i]);
                }
            }
            else
            {
                soc_mem_entry_dump(unit, mem, entry);
            }
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "\n");
        }
    }
    bsl_log_end(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "");

    rv = CMD_OK;

done:
    return rv;
}
STATIC cmd_result_t
diag_sand_do_dump_table_field(
    int unit,
    soc_mem_t mem,
    unsigned array_index,
    int copyno,
    int index,
    int count,
    int flags,
    char *field)
{
    int k, i;
    uint32 entry[SOC_MAX_MEM_WORDS];
    char lineprefix[256];
    int entry_dw;
    int rv = CMD_FAIL;
    assert(copyno >= 0);
    entry_dw = soc_mem_entry_words(unit, mem);
    bsl_log_start(BSL_APPL_SHELL, bslSeverityNormal, unit, "");
    for (k = index; k < index + count; k++)
    {
        if (!(flags & DUMP_DISABLE_CACHE))
        {
            i = soc_mem_array_read(unit, mem, array_index, copyno, k, entry);
        }
        else
        {
            i = soc_mem_array_read_flags(unit, mem, array_index, copyno, k, entry, SOC_MEM_DONT_USE_CACHE);
        }
        if (i < 0)
        {
            LOG_INFO(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit, "Read ERROR: table %s.%s[%d]: %s\n"), SOC_MEM_UFNAME(unit, mem),
                      SOC_BLOCK_NAME(unit, copyno), k, soc_errmsg(i)));
            goto done;
        }
        if (!(flags & DUMP_TABLE_ALL))
        {
        }
        if (flags & DUMP_TABLE_HEX)
        {
            for (i = 0; i < entry_dw; i++)
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%08x\n", entry[i]);
            }
        }
        else if (flags & DUMP_TABLE_CHANGED)
        {
            if (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_IS_ARRAY)
            {
                sal_sprintf(lineprefix, "%s[%d].%s[%d]: ", SOC_MEM_UFNAME(unit, mem), array_index,
                            SOC_BLOCK_NAME(unit, copyno), k);
            }
            else
            {
                sal_sprintf(lineprefix, "%s.%s[%d]: ", SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno), k);
            }
            soc_mem_entry_dump_if_changed(unit, mem, entry, lineprefix);
        }
        else
        {
            if (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_IS_ARRAY)
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%s[%d].%s[%d]: ",
                            SOC_MEM_UFNAME(unit, mem), array_index, SOC_BLOCK_NAME(unit, copyno), k);
            }
            else
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%s.%s[%d]: ",
                            SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno), k);
            }
            if (flags & DUMP_TABLE_RAW)
            {
                for (i = 0; i < entry_dw; i++)
                {
                    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "0x%08x ", entry[i]);
                }
            }
            else
            {
                diag_sand_mem_entry_dump_field(unit, mem, entry, field);
            }
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "\n");
        }
    }
    bsl_log_end(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "");
    rv = CMD_OK;

done:
    return rv;
}

/*
 * Function:
 *    soc_mem_entry_dump
 * Purpose:
 *    Debug routine to dump a formatted table entry.
 *
 *    Note:  Prefix != NULL : Dump chg command
 *           Prefix == NULL : Dump     command
 *             (Actually should pass dump_chg flag but keeping for simplicity)
 */
static void
diag_sand_mem_entry_dump_field(
    int unit,
    soc_mem_t mem,
    void *buf,
    char *field_name)
{
    soc_field_info_t *fieldp;
    soc_mem_info_t *memp;
    int f;
    int field_found = 0;
#if !defined(SOC_NO_NAMES)
    uint32 fval[SOC_MAX_MEM_FIELD_WORDS];
    char tmp[(SOC_MAX_MEM_FIELD_WORDS * 8) + 3];
#endif
    /*
     * Max nybbles + "0x" + null terminator 
     */

    memp = &SOC_MEM_INFO(unit, mem);

    if (!SOC_MEM_IS_VALID(unit, mem))
    {
        LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Memory not valid for unit\n")));
    }
    else
    {
        for (f = memp->nFields - 1; f >= 0; f--)
        {
            fieldp = &memp->fields[f];
            if (sal_strcasecmp(SOC_FIELD_NAME(unit, fieldp->field), field_name))
            {
                continue;
            }
            else if (field_found++ == 0)
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "<");
            }

#if !defined(SOC_NO_NAMES)
            {
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%s=",
                            soc_fieldnames[fieldp->field]);
                sal_memset(fval, 0, sizeof(fval));
                soc_mem_field_get(unit, mem, buf, fieldp->field, fval);
                _shr_format_long_integer(tmp, fval, BITS2BYTES(fieldp->len));
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%s%s", tmp, f > 0 ? "," : "");
            }

#endif
        }
    }

    if (field_found)
    {
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, ">");
    }
    else
    {
        LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "field: \"%s\" didn't found\n"), field_name));
    }

}

STATIC int
diag_sand_filter_mems(
    int unit,
    int mem)
{
    switch (mem)
    {
            /*
             * Not a real table but the actual external DRAM and its huge and slow 
             */
        case MMU_DRAM_ADDRESS_SPACEm:

            return 1;
    }
    return 0;
}

STATIC int
diag_sand_mem_get_all(
    int unit,
    int flags)
{
    soc_mem_t mem;
    int i, rv;

    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        if (!soc_mem_is_valid(unit, mem) || diag_sand_filter_mems(unit, mem))
        {
            continue;
        }
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "unit %d, mem %d\n", unit, mem);
        SOC_MEM_BLOCK_ITER(unit, mem, i)
        {
            int index = SOC_MEM_INFO(unit, mem).index_min;
            int count = SOC_MEM_INFO(unit, mem).index_max - SOC_MEM_INFO(unit, mem).index_min + 1;
            if (soc_mem_entry_words(unit, mem) + 1 > CMIC_SCHAN_WORDS(unit))
            {
                LOG_ERROR(BSL_LS_APPL_SHELL,
                          (BSL_META_U(unit, "Error: can't read memory %s\n"), SOC_MEM_UFNAME(unit, mem)));
                rv = CMD_FAIL;
            }
            else
            {
                rv = diag_sand_do_dump_table(unit, mem, 0, i, index, count, flags);
            }
            if (rv != CMD_OK)
            {
                LOG_INFO(BSL_LS_APPL_COMMON,
                         (BSL_META_U(unit, "failed to dump table %d copy %d indexd %d unit %d\n"), mem, i, index,
                          unit));
            }
        }
    }

    return CMD_OK;
}

static void
diag_sand_pci_print_config(
    int unit)
{
    uint32 data;
    int cap_len, cap_base, next_cap_base, i;

    if ((soc_cm_get_dev_type(unit) & BDE_PCI_DEV_TYPE) == 0)
    {
        LOG_ERROR(BSL_LS_APPL_SHELL,
                  (BSL_META_U(unit, "Error in %s(): Device does not support PCI interface."), FUNCTION_NAME()));
        return;
    }

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_VENDOR_ID);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  DeviceID=%04x  VendorID=%04x\n",
                PCI_CONF_VENDOR_ID, data, (data & 0xffff0000) >> 16, (data & 0x0000ffff) >> 0);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_COMMAND);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  Status=%04x  Command=%04x\n",
                PCI_CONF_COMMAND, data, (data & 0xffff0000) >> 16, (data & 0x0000ffff) >> 0);
    cap_len = (data >> 16) & 0x10 ? 4 : 0;

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_REVISION_ID);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  ClassCode=%06x  RevisionID=%02x\n",
                PCI_CONF_REVISION_ID, data, (data & 0xffffff00) >> 8, (data & 0x000000ff) >> 0);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_CACHE_LINE_SIZE);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  BIST=%02x  HeaderType=%02x  "
                "LatencyTimer=%02x  CacheLineSize=%02x\n",
                PCI_CONF_CACHE_LINE_SIZE, data,
                (data & 0xff000000) >> 24,
                (data & 0x00ff0000) >> 16, (data & 0x0000ff00) >> 8, (data & 0x000000ff) >> 0);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_BAR0);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  BaseAddress0=%08x\n",
                PCI_CONF_BAR0, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_BAR1);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  BaseAddress1=%08x\n",
                PCI_CONF_BAR1, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_BAR2);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  BaseAddress2=%08x\n",
                PCI_CONF_BAR2, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_BAR3);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  BaseAddress3=%08x\n",
                PCI_CONF_BAR3, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_BAR4);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  BaseAddress4=%08x\n",
                PCI_CONF_BAR4, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_BAR5);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  BaseAddress5=%08x\n",
                PCI_CONF_BAR5, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_CB_CIS_PTR);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  CardbusCISPointer=%08x\n",
                PCI_CONF_CB_CIS_PTR, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_SUBSYS_VENDOR_ID);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                "%04x: %08x  SubsystemID=%02x  SubsystemVendorID=%02x\n", PCI_CONF_SUBSYS_VENDOR_ID, data,
                (data & 0xffff0000) >> 16, (data & 0x0000ffff) >> 0);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_EXP_ROM);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  ExpansionROMBaseAddress=%08x\n",
                PCI_CONF_EXP_ROM, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, 0x34);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit,
                "%04x: %08x  Reserved=%06x  CapabilitiesPointer=%02x\n", 0x34, data, (data & 0xffffff00) >> 8,
                (data & 0x000000ff) >> 0);
    cap_base = cap_len ? data & 0xff : 0;

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, 0x38);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  Reserved=%08x\n",
                0x38, data, data);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, PCI_CONF_INTERRUPT_LINE);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  Max_Lat=%02x  Min_Gnt=%02x  "
                "InterruptPin=%02x  InterruptLine=%02x\n",
                PCI_CONF_INTERRUPT_LINE, data,
                (data & 0xff000000) >> 24,
                (data & 0x00ff0000) >> 16, (data & 0x0000ff00) >> 8, (data & 0x000000ff) >> 0);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, 0x40);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  Reserved=%02x  "
                "RetryTimeoutValue=%02x  TRDYTimeoutValue=%02x\n",
                0x40, data, (data & 0xffff0000) >> 16, (data & 0x0000ff00) >> 8, (data & 0x000000ff) >> 0);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, 0x44);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  PLLConf=%01x\n",
                0x44, data, (data & 0x000000ff) >> 0);

    data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, 0x48);
    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  -\n", 0x48, data);

    while (cap_base)
    {
        data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, cap_base);
        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  CapabilityID=%02x "
                    "CapabilitiesPointer=%02x ", cap_base, data, data & 0xff, (data >> 8) & 0xff);
        next_cap_base = (data >> 8) & 0xff;
        switch (data & 0xff)
        {
            case 0x01:
                cap_len = 2 * 4;
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "PWR-MGMT\n");
                break;
            case 0x03:
                cap_len = 2 * 4;
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "VPD\n");
                break;
            case 0x05:
                cap_len = 6 * 4;        /* 3 to 6 DWORDS */
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "MSI\n");
                break;
            case 0x10:
                cap_len = 3 * 4;
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "PCIE\n");
                break;
            case 0x11:
                cap_len = 3 * 4;
                bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "MSI-X\n");
                break;
            default:
                break;
        }
        for (i = 4; i < cap_len; i += 4)
        {
            data = CMVEC(unit).pci_conf_read(&CMDEV(unit).dev, cap_base + i);
            bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "%04x: %08x  -\n", cap_base + i, data);
        }
        cap_base = next_cap_base;
    }
}

char cmd_sand_mem_get_usage[] = "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "DUMP [options]\n"
#else
    "DUMP [File=<name>] [Append=true|false] [raw] [hex] [all] [chg] [disable_cache]\n\t"
    "        <TABLE[[ARRAYINDEX]]>[.<COPYNO>][<INDEX>] [<COUNT>]\n\t"
    "        [-filter <FIELD>=<VALUE>[,...]]\n\t"
    "      If raw is specified, show raw memory words instead of fields.\n\t"
    "      If hex is specified, show hex data only (for Expect parsing).\n\t"
    "      If all is specified, show even empty or invalid entries\n\t"
    "      If chg is specified, show only fields changed from defaults\n\t"
    "      If disable_cache is specified - dosn't read from cached memory, if exist\n\t "
    "      (Use \"listmem\" command to show a list of valid tables)\n"
#endif
    ;

cmd_result_t
cmd_sand_mem_get(
    int unit,
    args_t * a)
{
    soc_mem_t mem;
    char *arg1, *arg2, *arg3;
    volatile int flags = 0;
    int copyno, blk;
    volatile int rv = CMD_FAIL;
    parse_table_t pt;
    volatile char *fname = "";
    int append = FALSE;
    unsigned array_index;
    char *next_name;
    volatile int console_was_on = 0, console_disabled = 0, pushed_ctrl_c = 0;
    jmp_buf ctrl_c;
    uint32 is_debug = 0;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "File", PQ_STRING, 0, &fname, 0);
    parse_table_add(&pt, "Append", PQ_BOOL, 0, &append, FALSE);

    if (!sh_check_attached(ARG_CMD(a), unit))
    {
        goto done;
    }
    next_name = NULL;
    if (parse_arg_eq(a, &pt) < 0)
    {
        rv = CMD_USAGE;
        goto done;
    }

    console_was_on = bslcons_is_enabled();

    if (fname[0] != 0)
    {
        /*
         * Catch control-C in case if using file output option. 
         */
#ifndef NO_CTRL_C
        if (setjmp(ctrl_c))
        {
            rv = CMD_INTR;
            goto done;
        }
#endif
        sh_push_ctrl_c(&ctrl_c);
        pushed_ctrl_c = TRUE;

        if (bslfile_is_enabled())
        {
            LOG_INFO(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit, "%s: Can't dump to file while logging is enabled\n"), ARG_CMD(a)));
            rv = CMD_FAIL;
            goto done;
        }

        if (bslfile_open((char *) fname, append) < 0)
        {
            LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "%s: Could not start log file\n"), ARG_CMD(a)));
            rv = CMD_FAIL;
            goto done;
        }

        bslcons_enable(FALSE);
        console_disabled = 1;
    }

    arg1 = ARG_GET(a);
    for (;;)
    {
        if (arg1 != NULL && !sal_strcasecmp(arg1, "raw"))
        {
            flags |= DUMP_TABLE_RAW;
            arg1 = ARG_GET(a);
        }
        else if (arg1 != NULL && !sal_strcasecmp(arg1, "hex"))
        {
            flags |= DUMP_TABLE_HEX;
            arg1 = ARG_GET(a);
        }
        else if (arg1 != NULL && !sal_strcasecmp(arg1, "all"))
        {
            flags |= DUMP_TABLE_ALL;
            arg1 = ARG_GET(a);
        }
        else if (arg1 != NULL && !sal_strcasecmp(arg1, "chg"))
        {
            flags |= DUMP_TABLE_CHANGED;
            arg1 = ARG_GET(a);
        }
        else if (arg1 != NULL && !sal_strcasecmp(arg1, "debug"))
        {
            is_debug = 1;
            flags |= REG_PRINT_ADDR;
            arg1 = ARG_GET(a);
        }
        else if (arg1 != NULL && !sal_strcasecmp(arg1, "disable_cache"))
        {
            flags |= DUMP_DISABLE_CACHE;
            arg1 = ARG_GET(a);
        }
        else
        {
            break;
        }
    }

    if (arg1 == NULL)
    {
        rv = CMD_USAGE;
        goto done;
    }

    if (!sal_strcasecmp(arg1, "soc"))
    {
        rv = diag_sand_reg_get_all(unit, is_debug);
        goto done;
    }

    if (!sal_strcasecmp(arg1, "socmem"))
    {
        rv = diag_sand_mem_get_all(unit, flags);
        goto done;
    }

    if (!sal_strcasecmp(arg1, "pcic"))
    {
        diag_sand_pci_print_config(unit);
        goto done;
    }

    /*
     * See if dumping a memory table 
     */
    if (parse_memory_name(unit, &mem, arg1, &copyno, &array_index) >= 0)
    {
        int index, count;

        arg2 = ARG_GET(a);
        arg3 = ARG_GET(a);
        if (!SOC_MEM_IS_VALID(unit, mem))
        {
            LOG_INFO(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit, "Error: Memory %s not valid for chip %s.\n"), SOC_MEM_UFNAME(unit, mem),
                      SOC_UNIT_NAME(unit)));
            goto done;
        }
        if (arg2)
        {
            index = parse_memory_index(unit, mem, arg2);
            count = (arg3 ? parse_integer(arg3) : 1);
        }
        else
        {
            index = soc_mem_index_min(unit, mem);
            if (soc_mem_is_sorted(unit, mem) && !(flags & DUMP_TABLE_ALL))
            {
                count = soc_mem_entries(unit, mem, copyno);
            }
            else
            {
                count = soc_mem_index_max(unit, mem) - index + 1;
            }
        }

        next_name = ARG_GET(a);

        SOC_MEM_BLOCK_ITER(unit, mem, blk)
        {

            if (copyno != COPYNO_ALL && copyno != blk)
            {
                continue;
            }

            if (next_name != NULL)
            {
                rv = diag_sand_do_dump_table_field(unit, mem, array_index, blk, index, count, flags, next_name);
            }
            else
            {
                rv = diag_sand_do_dump_table(unit, mem, array_index, blk, index, count, flags);
            }
        }
        goto done;
    }

    LOG_INFO(BSL_LS_APPL_COMMON,
             (BSL_META_U(unit, "Unknown option or memory to dump (use 'help dump' for more info)\n")));

    rv = CMD_FAIL;

done:

    if (fname[0] != 0)
    {
        bslfile_close();
    }

    if (console_disabled && console_was_on)
    {
        bslcons_enable(TRUE);
    }

    if (pushed_ctrl_c)
    {
        sh_pop_ctrl_c();
    }

    parse_arg_eq_done(&pt);
    return rv;
}

STATIC cmd_result_t
diag_sand_mem_write(
    int unit,
    args_t * a,
    int mod)
{
    int i, index, start, count, copyno, blk;
    unsigned array_index;
    char *tab, *idx, *cnt, *s, *memname;
    soc_mem_t mem;
    uint32 entry[SOC_MAX_MEM_WORDS];
    int entry_dw, view_len;
    char copyno_str[8];
    int r, update;
    int rv = CMD_FAIL;
    char *valstr = NULL, *view = NULL;
    int no_cache = 0;

    if (!sh_check_attached(ARG_CMD(a), unit))
    {
        goto done;
    }

    tab = ARG_GET(a);
    if (tab != NULL && sal_strcasecmp(tab, "nocache") == 0)
    {
        no_cache = 1;
        tab = ARG_GET(a);
    }
    idx = ARG_GET(a);
    cnt = ARG_GET(a);
    s = ARG_GET(a);

    /*
     * you will need at least one value and all the args .. 
     */
    if (!tab || !idx || !cnt || !s || !isint(cnt))
    {
        return CMD_USAGE;
    }

    /*
     * Deal with VIEW:MEMORY if applicable 
     */
    memname = strstr(tab, ":");
    view_len = 0;
    if (memname != NULL)
    {
        memname++;
        view_len = memname - tab;
    }
    else
    {
        memname = tab;
    }

    if (parse_memory_name(unit, &mem, memname, &copyno, &array_index) < 0)
    {
        LOG_ERROR(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "ERROR: unknown table \"%s\"\n"), tab));
        goto done;
    }

    if (!SOC_MEM_IS_VALID(unit, mem))
    {
        LOG_ERROR(BSL_LS_APPL_SHELL,
                  (BSL_META_U(unit, "Error: Memory %s not valid for chip %s.\n"), SOC_MEM_UFNAME(unit, mem),
                   SOC_UNIT_NAME(unit)));
        goto done;
    }

    if (soc_mem_is_readonly(unit, mem))
    {
        LOG_ERROR(BSL_LS_APPL_SHELL, (BSL_META_U(unit, "ERROR: Table %s is read-only\n"), SOC_MEM_UFNAME(unit, mem)));
        goto done;
    }

    start = parse_memory_index(unit, mem, idx);
    count = parse_integer(cnt);

    if (copyno == COPYNO_ALL)
    {
        copyno_str[0] = 0;
    }
    else
    {
        sal_sprintf(copyno_str, ".%d", copyno);
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    if ((valstr = sal_alloc(ARGS_BUFFER, "reg_set")) == NULL)
    {
        LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "cmd_esw_mem_write : Out of memory\n")));
        goto done;
    }

    /*
     * If a list of fields were specified, generate the entry from them.
     * Otherwise, generate it by reading raw dwords from command line.
     */
    if (!isint(s))
    {
        /*
         * List of fields 
         */
        if (view_len == 0)
        {
            diag_sand_utils_collect_comma_args(a, valstr, s);
        }
        else
        {
            if ((view = sal_alloc(view_len + 1, "view_name")) == NULL)
            {
                LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "cmd_esw_mem_write : Out of memory\n")));
                goto done;
            }
            memset(view, 0, view_len + 1);
            memcpy(view, tab, view_len);
            if (diag_sand_collect_args_with_view(a, valstr, s, view, unit, mem) < 0)
            {
                LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Out of memory: aborted\n")));
                goto done;
            }
        }

        memset(entry, 0, sizeof(entry));

        if (0 == mod)
        {
            if (diag_sand_mem_modify_fields(unit, mem, entry, NULL, valstr, FALSE) < 0)
            {
                LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Syntax error: aborted\n")));
                goto done;
            }
        }

        update = TRUE;
    }
    else
    {
        /*
         * List of numeric values 
         */
        ARG_PREV(a);
        if (diag_sand_parse_dwords(unit, entry_dw, entry, a) < 0)
        {
            goto done;
        }
        update = FALSE;
    }

    SOC_MEM_BLOCK_ITER(unit, mem, blk)
    {

        if (copyno != COPYNO_ALL && copyno != blk)
        {
            continue;
        }

        for (index = start; index < start + count; index++)
        {

            if (1 == mod)
            {
                r = soc_mem_array_read(unit, mem, array_index, blk, index, entry);
                if (r < 0)
                {
                    LOG_INFO(BSL_LS_APPL_COMMON,
                             (BSL_META_U(unit, "READ ERROR: table %s.%s[%d]: %s\n"), SOC_MEM_UFNAME(unit, mem),
                              copyno_str, index, soc_errmsg(r)));
                }

                if (diag_sand_mem_modify_fields(unit, mem, entry, NULL, valstr, FALSE) < 0)
                {
                    LOG_INFO(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "Syntax error: aborted\n")));
                    goto done;
                }
            }

            if (index == start)
            {
                if (bsl_check(bslLayerAppl, bslSourceSocmem, bslSeverityNormal, unit))
                {
                    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "WRITE[%s%s], DATA:",
                                SOC_MEM_UFNAME(unit, mem), copyno_str);
                    for (i = 0; i < entry_dw; i++)
                    {
                        bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, " 0x%x", entry[i]);
                    }
                    bsl_log_add(BSL_APPL_SHELL, bslSeverityNormal, bslSinkIgnore, unit, "\n");
                }
            }

            r = soc_mem_array_write_extended(unit, no_cache ? SOC_MEM_DONT_USE_CACHE : SOC_MEM_NO_FLAGS, mem,
                                             array_index, blk, index, entry);
            if (r < 0)
            {
                LOG_INFO(BSL_LS_APPL_COMMON,
                         (BSL_META_U(unit, "Write ERROR: table %s.%s[%d]: %s\n"), SOC_MEM_UFNAME(unit, mem), copyno_str,
                          index, soc_errmsg(r)));
                goto done;
            }

            if (mod && update)
            {
                diag_sand_mem_modify_fields(unit, mem, entry, NULL, valstr, TRUE);
            }
        }
    }
    rv = CMD_OK;

done:
    if (valstr != NULL)
    {
        sal_free(valstr);
    }
    if (view != NULL)
    {
        sal_free(view);
    }
    return rv;
}

char cmd_sand_mem_write_usage[] =
    "Parameters: <TABLE>[.<COPY>] <ENTRY> <ENTRYCOUNT>\n\t"
    "        { <DW0> .. <DWN> | <FIELD>=<VALUE>[,...] }\n\t"
    "Number of <DW> must be a multiple of table entry size.\n\t" "Writes entry(s) into table index(es).\n";

cmd_result_t
cmd_sand_mem_write(
    int unit,
    args_t * a)
{
    return diag_sand_mem_write(unit, a, 0);
}

char cmd_sand_mem_modify_usage[] =
    "Parameters: <TABLE>[.<COPY>] <ENTRY> <ENTRYCOUNT>\n\t"
    "        <FIELD>=<VALUE>[,...]\n\t" "Read/modify/write field(s) of a table entry(s).\n";

cmd_result_t
cmd_sand_mem_modify(
    int unit,
    args_t * a)
{
    return diag_sand_mem_write(unit, a, 1);
}
