/*
 * $Id: memory.c,v 1.90 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Memory Tests implemented via S-Channel table read/write.
 * These are especially useful for memories without BIST (e.g. QVLAN).
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_LS_SOC_MEM

#include <shared/bsl.h>

#include <sal/types.h>
#include <sal/appl/io.h>
#include <shared/bsl.h>
#include <sal/appl/pci.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/progress.h>
#include <soc/mem.h>
#include <soc/l2x.h>
#include <bcm/field.h>
#ifdef BCM_TRIUMPH2_SUPPORT
#include <soc/triumph2.h>
#endif
#ifdef BCM_TRIDENT_SUPPORT
#include <soc/trident.h>
#endif
#ifdef BCM_TOMAHAWK_SUPPORT
#include <soc/tomahawk.h>
#endif
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif
#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/drv.h>
#include <bcm_int/dpp/counters.h>
#ifndef __KERNEL__
#include <time.h>
#else
#include <linux/time.h>
#endif

#endif
#ifdef BCM_DFE_SUPPORT
#include <soc/dfe/cmn/dfe_drv.h>
#endif
#include "testlist.h"

#if (defined (BCM_PETRA_SUPPORT)|| defined (BCM_DFE_SUPPORT))
extern cmd_result_t do_mem_test(int unit, args_t *a);
#if defined(BCM_PETRA_SUPPORT) ||defined (BCM_DFE_SUPPORT)
extern cmd_result_t do_mem_test_rw(int unit, tr7_dbase_t, args_t *a);
#endif
#ifdef BCM_PETRA_SUPPORT
extern cmd_result_t do_mem_test_flipflop(int , tr8_dbase_t, args_t *);
int8 tr8_open_dump_file(int unit, char *fname);
void tr8_close_dump_file(void);
void tr8_write_dump(const char * _Format);
int disable_ecc_disable_dynamic_mechanism(int );
#endif
extern cmd_result_t do_dump_memories(int unit, args_t *a);
extern int bcm_common_linkscan_enable_set(int,int);
#endif

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) || defined(BCM_POLAR_SUPPORT)

typedef struct {
    int            inited;
    int            was_debug_mode;
    int            orig_enable;
    int            saved_tcam_protect_write;
    int            mem_scan;
    int            scan_rate;
    sal_usecs_t    scan_interval;
    int            sram_scan;
    int            sram_scan_rate;
    sal_usecs_t    sram_scan_interval;            
    int            read_count;
    int            pat_zero, pat_one, pat_five, pat_a;
    int            pat_checker, pat_invckr, pat_linear, pat_random;
    int            progress_total;
    int            pat_hex;
    int            hex_byte;
    int            test_by_entry;
    int            reverify_count;
    int            reverify_delay;
    int            continue_on_error;
    int            error_count;
    int            error_max;
    soc_mem_test_t parm;
} mem_testdata_t;

static mem_testdata_t *mem_work[SOC_MAX_NUM_DEVICES];

#ifdef BCM_POLAR_SUPPORT
static int
_soc_mem_id_map(int unit, soc_mem_t mem, int *drv_mem){

    int table_name;
    
    switch(mem) {
        case GEN_MEMORYm:
            table_name = DRV_MEM_GEN;
            break;
        case L2_ARLm:
        case L2_MARLm:
        case L2_ARL_SWm:
        case L2_MARL_SWm:
            table_name = DRV_MEM_ARL_HW;
            break;
        case MARL_PBMPm:
            table_name = DRV_MEM_MCAST;
            break;
        case MSPT_TABm:
            table_name = DRV_MEM_MSTP;
            break;
        case VLAN_1Qm:
            table_name = DRV_MEM_VLAN;
            break;
        case VLAN2VLANm:
            table_name = DRV_MEM_VLANVLAN;
            break;
        case MAC2VLANm:
            table_name = DRV_MEM_MACVLAN;
            break;
        case PROTOCOL2VLANm:
            table_name = DRV_MEM_PROTOCOLVLAN;
            break;
        case FLOW2VLANm:
            table_name = DRV_MEM_FLOWVLAN;
            break;
        case CFP_TCAM_S0m:
        case CFP_TCAM_S1m:
        case CFP_TCAM_S2m:
        case CFP_TCAM_IPV4_SCm:
        case CFP_TCAM_IPV6_SCm:
        case CFP_TCAM_NONIP_SCm:
        case CFP_TCAM_CHAIN_SCm:
            table_name = DRV_MEM_TCAM_DATA;
            break;
        case CFP_TCAM_MASKm:
        case CFP_TCAM_IPV4_MASKm:
        case CFP_TCAM_IPV6_MASKm:
        case CFP_TCAM_NONIP_MASKm:
        case CFP_TCAM_CHAIN_MASKm:
            table_name = DRV_MEM_TCAM_MASK;
            break;
        case CFP_ACT_POLm:
        case CFP_ACTm:
            table_name = DRV_MEM_CFP_ACT;
            break;
        case CFP_METERm:
            table_name = DRV_MEM_CFP_METER;
            break;
        case CFP_STAT_IBm:
            table_name = DRV_MEM_CFP_STAT_IB;
            break;
        case CFP_STAT_OBm:
            table_name = DRV_MEM_CFP_STAT_OB;
            break;
        case EGRESS_VID_REMARKm:
            table_name = DRV_MEM_EGRVID_REMARK;
            break;
        default:
            cli_out("Unsupport memory table.\n");
            return -1;
    }
    *drv_mem = table_name;
    return SOC_E_NONE;
}
#endif /* BCM_POLAR_SUPPORT */

/*
 * Memory Test Support Routines
 */

void
mt_status_fn(soc_mem_test_t *parm, char *status_str)
{
    static char buf[120];

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(parm->unit) && SOC_IS_POLAR(parm->unit)) {
        sal_sprintf(buf,
            "%s on %s",
            status_str,
            SOC_ROBO_MEM_UFNAME(parm->unit, parm->mem));
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        sal_sprintf(buf,
            "%s on %s.%s",
            status_str,
            SOC_MEM_UFNAME(parm->unit, parm->mem),
            SOC_BLOCK_NAME(parm->unit, parm->copyno));
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    progress_status(buf);
}

int
mt_write_fn(soc_mem_test_t *parm, unsigned array_index,
        int copyno, int index, uint32 *entry_data)
{
    if (parm->report_progress) {
        progress_report(1);
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(parm->unit) && SOC_IS_POLAR(parm->unit)) {
        int table_name;
        if(_soc_mem_id_map(parm->unit, parm->mem, &table_name) == SOC_E_NONE) {
            if (DRV_MEM_WRITE(parm->unit, table_name, index, 1, entry_data) < 0) {
                cli_out("Write ERROR: table %s[%d]\n",
                        SOC_ROBO_MEM_UFNAME(parm->unit, parm->mem), index);
            }
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_array_write(parm->unit, parm->mem, array_index,
                  copyno, index, entry_data) < 0) {
            if (parm->array_index_start != 0 || parm->array_index_end != parm->array_index_start) {
                    cli_out("Write ERROR: table %s[%u].%s[%d]\n",
                            SOC_MEM_UFNAME(parm->unit, parm->mem),
                            array_index,
                            SOC_BLOCK_NAME(parm->unit, copyno), index);
            } else {
                    cli_out("Write ERROR: table %s.%s[%d]\n",
                            SOC_MEM_UFNAME(parm->unit, parm->mem),
                            SOC_BLOCK_NAME(parm->unit, copyno), index);
            }
            return -1;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    return 0;
}

#undef TEST_INJECT_READ_ERROR

int
mt_read_fn(soc_mem_test_t *parm, unsigned array_index,
       int copyno, int index, uint32 *entry_data)
{
    if (parm->report_progress) {
        progress_report(1);
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(parm->unit) && SOC_IS_POLAR(parm->unit)) {
        int table_name;
        if(_soc_mem_id_map(parm->unit, parm->mem, &table_name) == SOC_E_NONE) {
            if (DRV_MEM_READ(parm->unit, table_name, index, 1, entry_data) < 0) {
                cli_out("Read ERROR: table %s[%d]\n",
                        SOC_ROBO_MEM_UFNAME(parm->unit, parm->mem), index);
            }
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_array_read(parm->unit, parm->mem, array_index,
                 copyno, index, entry_data) < 0) {
            if (parm->array_index_start != 0 || parm->array_index_end != parm->array_index_start) {
                    cli_out("Read ERROR: table %s[%u].%s[%d]\n",
                            SOC_MEM_UFNAME(parm->unit, parm->mem),
                            array_index,
                            SOC_BLOCK_NAME(parm->unit, copyno), index);
            } else {
                    cli_out("Read ERROR: table %s.%s[%d]\n",
                            SOC_MEM_UFNAME(parm->unit, parm->mem),
                            SOC_BLOCK_NAME(parm->unit, copyno), index);
            }
            return -1;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

#ifdef TEST_INJECT_READ_ERROR
    if (index == 7)
    entry_data[0] ^= 0x00010000;
#endif

    return 0;
}

int
mt_miscompare_fn(soc_mem_test_t *parm, unsigned array_index,
         int copyno, int index,
         uint32 *read_data, uint32 *wrote_data, uint32 *mask_data)
{
    int            unit = parm->unit;
    int         i, dw;
    uint32        reread_data[SOC_MAX_MEM_WORDS];

    soc_pci_analyzer_trigger(unit);

    dw = soc_mem_entry_words(unit, parm->mem);

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (parm->array_index_start != 0 || parm->array_index_end != parm->array_index_start) {
        {
            cli_out("\n\nCompare ERROR: table %s[%u].%s[%d]\n",
                    SOC_MEM_UFNAME(unit, parm->mem), array_index,
                    SOC_BLOCK_NAME(unit, copyno), index);
        }
    } else 
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    {
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_ROBO(unit) && SOC_IS_POLAR(unit)) {
            cli_out("\n\nCompare ERROR: table %s[%d]\n",
                    SOC_ROBO_MEM_UFNAME(unit, parm->mem), index);
        } else
#endif /* BCM_POLAR_SUPPORT */
        {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
            cli_out("\n\nCompare ERROR: table %s.%s[%d]\n",
                    SOC_MEM_UFNAME(unit, parm->mem),
                    SOC_BLOCK_NAME(unit, copyno), index);
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
        }
    }
    cli_out("\n  Mask  :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", mask_data[i]);
    }


    cli_out("\n  Read  :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", read_data[i] & mask_data[i]);
    }
    cli_out("\n  Wrote :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", wrote_data[i] & mask_data[i]);
    }
    cli_out("\n  Diff  :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", (read_data[i] ^ wrote_data[i]) & mask_data[i]);
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(unit) && SOC_IS_POLAR(unit)) {
        int table_name;
        if(_soc_mem_id_map(unit, parm->mem, &table_name) == SOC_E_NONE) {
            if (DRV_MEM_READ(unit, table_name, index, 1, reread_data) >= 0) {
                cli_out("\n  Reread:");
                for (i = 0; i < dw; i++) {
                    cli_out(" 0x%08x", reread_data[i] & mask_data[i]);
                }
            }
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_array_read(unit, parm->mem, array_index,
                 copyno, index, reread_data) >= 0) {
            cli_out("\n  Reread:");
            for (i = 0; i < dw; i++) {
                cli_out(" 0x%08x", reread_data[i] & mask_data[i]);
            }
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }
    test_error(unit, "\n");

    return MT_MISCOMPARE_STOP;
}

#ifdef BCM_PETRA_SUPPORT
int
mt_show_compare_fn(soc_mem_test_t *parm, unsigned array_index,
         int copyno, int index,
         uint32 *read_data, uint32 *wrote_data, uint32 *mask_data)
{
    int            unit = parm->unit;
    int         i, dw;
    uint32        reread_data[SOC_MAX_MEM_WORDS];

   /* soc_pci_analyzer_trigger(unit); */

    dw = soc_mem_entry_words(unit, parm->mem);

    if (parm->array_index_start != 0 || parm->array_index_end != parm->array_index_start) {
        cli_out("\n\ntable %s[%u].%s[%d]\n",
                SOC_MEM_UFNAME(unit, parm->mem), array_index,
                SOC_BLOCK_NAME(unit, copyno), index);
    } else {
        cli_out("\n\ntable %s.%s[%d]\n",
                SOC_MEM_UFNAME(unit, parm->mem),
                SOC_BLOCK_NAME(unit, copyno), index);
    }
    cli_out("\n  Mask  :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", mask_data[i]);
    }


    cli_out("\n  Read  :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", read_data[i] & mask_data[i]);
    }
    cli_out("\n  Wrote :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", wrote_data[i] & mask_data[i]);
    }
    cli_out("\n  Diff  :");
    for (i = 0; i < dw; i++) {
    cli_out(" 0x%08x", (read_data[i] ^ wrote_data[i]) & mask_data[i]);
    }
    if (soc_mem_array_read(unit, parm->mem, array_index,
             copyno, index, reread_data) >= 0) {
    cli_out("\n  Reread:");
    for (i = 0; i < dw; i++) {
        cli_out(" 0x%08x", reread_data[i] & mask_data[i]);
    }
    }

    cli_out("\n");

    return 0;
}
#endif

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
STATIC int
mem_test_common_clear(int unit, soc_mem_t mem, int copyno)
{
#if defined(INCLUDE_MEM_SCAN) || defined(BCM_SRAM_SCAN_SUPPORT)
    mem_testdata_t *mw = mem_work[unit];
#endif
    int rv;
    
    SOC_MEM_TEST_SKIP_CACHE_SET(unit, 1);
    if ((rv = soc_mem_parity_control(unit, mem, copyno, FALSE)) < 0) {
        test_error(unit, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(u, mem));
        return -1;
    }
#ifdef INCLUDE_MEM_SCAN
    if ((mw->mem_scan = soc_mem_scan_running(unit, &mw->scan_rate,
                                             &mw->scan_interval)) > 0) {
        if (soc_mem_scan_stop(unit)) {
            return -1;
        }
    }
#endif /* INCLUDE_MEM_SCAN */
#ifdef BCM_SRAM_SCAN_SUPPORT
    if ((mw->sram_scan = soc_sram_scan_running(unit, &mw->sram_scan_rate,
                                               &mw->sram_scan_interval)) > 0) {
        if (soc_sram_scan_stop(unit)) {
            return -1;
        }
    }
#endif /* BCM_SRAM_SCAN_SUPPORT */
    return 0;
}

STATIC int
mem_test_common_restore(int unit, soc_mem_t mem, int copyno)
{
#if defined(INCLUDE_MEM_SCAN) || defined(BCM_SRAM_SCAN_SUPPORT)
    mem_testdata_t *mw = mem_work[unit];
#endif

    SOC_MEM_TEST_SKIP_CACHE_SET(unit, 0);
    if (soc_mem_parity_restore(unit, mem, copyno) < 0) {
        test_error(unit, "Could not enable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, mem));
        return -1;
    }
#ifdef INCLUDE_MEM_SCAN
    if (mw->mem_scan) {
        if (soc_mem_scan_start(unit, mw->scan_rate, mw->scan_interval)) {
            return -1;
        }
    }
#endif /* INCLUDE_MEM_SCAN */
#ifdef BCM_SRAM_SCAN_SUPPORT
    if (mw->sram_scan) {
        if (soc_sram_scan_start(unit, mw->sram_scan_rate, mw->sram_scan_interval)) {
            return -1;
        }
    }
#endif /* BCM_SRAM_SCAN_SUPPORT */

    return 0;
}
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT ||  BCM_CALADAN3_SUPPORT */

int
mem_test_default_init(int u, soc_mem_t mem, void **p)
{
    mem_testdata_t    *mw;
    soc_mem_test_t    *parm;    
    char        *idx_start_str, *idx_end_str;
    char        *array_idx_start_str, *array_idx_end_str;
    int            copies, blk;
    int            rv = -1;
    parse_table_t    pt;
#ifdef BCM_POLAR_SUPPORT
    uint32 cache_enable = FALSE;
#endif /* BCM_POLAR_SUPPORT */


    mw = mem_work[u];
    if (mw == NULL) {
        mw = sal_alloc(sizeof(mem_testdata_t), "memtest");
        if (mw == NULL) {
            cli_out("cannot allocate memory test data\n");
            return -1;
        }
        sal_memset(mw, 0, sizeof(mem_testdata_t));
        mem_work[u] = mw;
    }
    parm = &mw->parm;    

    parse_table_init(u, &pt); 
    

    parse_table_add(&pt,  "IndexStart",    PQ_STRING, "min",
                    &idx_start_str, NULL);
    parse_table_add(&pt,  "IndexEnd",    PQ_STRING, "max",
                    &idx_end_str, NULL);
    parse_table_add(&pt,  "ArrayIndexStart",    PQ_STRING, "min", &array_idx_start_str, NULL);
    parse_table_add(&pt,  "ArrayIndexEnd",    PQ_STRING, "max", &array_idx_end_str, NULL);
    parse_table_add(&pt,  "IndexInc",    PQ_INT, (void *) 1,
                    &parm->index_step, NULL);
    parse_table_add(&pt,  "PatZero",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_zero, NULL);
    parse_table_add(&pt,  "PatOne",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_one, NULL);
    parse_table_add(&pt,  "PatFive",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_five, NULL);
    parse_table_add(&pt,  "PatA",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_a, NULL);
    parse_table_add(&pt,  "PatChecker",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_checker, NULL);
    parse_table_add(&pt,  "PatInvckr",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_invckr, NULL);
    parse_table_add(&pt,  "PatLinear",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_linear, NULL);
    parse_table_add(&pt,  "PatRandom",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_random, NULL);
    parse_table_add(&pt,  "PatHex",  PQ_BOOL|PQ_DFL, 0,
                    &mw->pat_hex, NULL);
    parse_table_add(&pt,  "HexByte",  PQ_INT|PQ_DFL, 0,
                    &mw->hex_byte, NULL);
    parse_table_add(&pt,  "TestByEntry",  PQ_BOOL|PQ_DFL, 0,
                    &mw->test_by_entry, NULL);
    parse_table_add(&pt,  "ReVerifyCount",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_count, NULL);
    parse_table_add(&pt,  "ReVerifyDelay",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_delay, NULL);
    parse_table_add(&pt,  "IGnoreErrors",  PQ_BOOL|PQ_DFL, 0,
                    &mw->continue_on_error, NULL);
    parse_table_add(&pt,  "ErrorCount",  PQ_INT|PQ_DFL, 0,
                    &mw->error_count, NULL);
    parse_table_add(&pt,  "ErrorMax",  PQ_INT|PQ_DFL, (void *) 1,
                    &mw->error_max, NULL);
    parse_table_add(&pt,  "ReadCount",    PQ_INT|PQ_DFL,    (void *) 1,
            &parm->read_count, NULL);
    parse_table_add(&pt, "EccAsData",   PQ_BOOL|PQ_DFL,  0,
                    &parm->ecc_as_data, NULL);
    parse_table_add(&pt,  "ReportProgress", PQ_BOOL, (void *)1,
                    &parm->report_progress, NULL);

    mw->was_debug_mode = -1;

    if (!mw->inited) {
          mw->pat_zero = TRUE;
          mw->pat_one = TRUE;
          mw->pat_five = TRUE;
          mw->pat_a = TRUE;
          mw->pat_checker = TRUE;
          mw->pat_invckr = TRUE;
          mw->pat_linear = TRUE;
          mw->pat_random = TRUE;
          parm->unit = u;
          parm->status_cb = mt_status_fn;
          parm->write_cb = mt_write_fn;
          parm->read_cb = mt_read_fn;
          parm->miscompare_cb = mt_miscompare_fn;
          parm->read_count = 1;
          parm->ecc_as_data = 0;
          parm->report_progress = 1;
          mw->inited = 1;
    }
    
    if (parse_default_fill(&pt) < 0) {
      cli_out("Invalid default fill:\n");
      goto done;
    }

    parm->mem = mem;
    parm->copyno = COPYNO_ALL;

    if (!soc_mem_is_valid(u, parm->mem) ||
        soc_mem_is_readonly(u, parm->mem)) {
#ifdef BCM_POLAR_SUPPORT
            if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
                test_error(u, "Memory %s is invalid or readonly\n",
                           SOC_ROBO_MEM_UFNAME(u, parm->mem));
            } else
#endif /* BCM_POLAR_SUPPORT */
            {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
                test_error(u, "Memory %s is invalid or readonly\n",
                           SOC_MEM_UFNAME(u, parm->mem));
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
            }
            goto done;
        }

    if (parm->copyno != COPYNO_ALL &&
        !SOC_MEM_BLOCK_VALID(u, parm->mem, parm->copyno)) {
#ifdef BCM_POLAR_SUPPORT
            if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
                test_error(u,
                           "Copy number %d out of range for memory %s\n",
                           parm->copyno, SOC_ROBO_MEM_UFNAME(u, parm->mem));
            } else
#endif /* BCM_POLAR_SUPPORT */
            {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
                test_error(u,
                           "Copy number %d out of range for memory %s\n",
                           parm->copyno, SOC_MEM_UFNAME(u, parm->mem));
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
            }
            goto done;
        }

    parm->index_start =
      parse_memory_index(parm->unit, parm->mem, idx_start_str);
    parm->index_end =
      parse_memory_index(parm->unit, parm->mem, idx_end_str);
    parm->array_index_start = parse_memory_array_index(parm->unit, parm->mem, array_idx_start_str);
    parm->array_index_end = parse_memory_array_index(parm->unit, parm->mem, array_idx_end_str);
    if (parm->array_index_start > parm->array_index_end ) {
        unsigned temp_ai = parm->array_index_start;
        parm->array_index_start = parm->array_index_end;
        parm->array_index_end = temp_ai;
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        if (parm->index_start < soc_robo_mem_index_min(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust start index from %d to %d "
                    "for memory %s\n",
                    parm->index_start,
                    soc_robo_mem_index_min(parm->unit, parm->mem),
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
            parm->index_start = soc_robo_mem_index_min(parm->unit, parm->mem);
        }
        if (parm->index_end > soc_robo_mem_index_max(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust end index from %d to %d "
                    "for memory %s\n",
                    parm->index_end,
                    soc_robo_mem_index_max(parm->unit, parm->mem),
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
            parm->index_end = soc_robo_mem_index_max(parm->unit, parm->mem);
        }
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (parm->index_start < soc_mem_index_min(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust start index from %d to %d "
                    "for memory %s.%s\n",
                    parm->index_start,
                    soc_mem_index_min(parm->unit, parm->mem),
                    SOC_MEM_UFNAME(u, parm->mem),
                    SOC_BLOCK_NAME(u, parm->copyno));
            parm->index_start = soc_mem_index_min(parm->unit, parm->mem);
        }
        if (parm->index_end > soc_mem_index_max(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust end index from %d to %d "
                    "for memory %s.%s\n",
                    parm->index_end,
                    soc_mem_index_max(parm->unit, parm->mem),
                    SOC_MEM_UFNAME(u, parm->mem),
                    SOC_BLOCK_NAME(u, parm->copyno));
            parm->index_end = soc_mem_index_max(parm->unit, parm->mem);
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    if (parm->index_step == 0 ||
        (parm->index_start < parm->index_end && parm->index_step < 0) ||
        (parm->index_start > parm->index_end && parm->index_step > 0)) {
            test_error(u, "Illegal index step value\n");
            goto done;
    }

    parm->patterns = 0;
    if (mw->pat_zero)
        parm->patterns |= MT_PAT_ZEROES;
    if (mw->pat_one)
        parm->patterns |= MT_PAT_ONES;
    if (mw->pat_five)
        parm->patterns |= MT_PAT_FIVES;
    if (mw->pat_a)
        parm->patterns |= MT_PAT_AS;
    if (mw->pat_checker)
        parm->patterns |= MT_PAT_CHECKER;
    if (mw->pat_invckr)
        parm->patterns |= MT_PAT_ICHECKER;
    if (mw->pat_linear)
        parm->patterns |= MT_PAT_ADDR;
    if (mw->pat_random)
        parm->patterns |= MT_PAT_RANDOM;
    if (mw->pat_hex) {
        parm->patterns |= MT_PAT_HEX;
        parm->hex_byte = mw->hex_byte;
    }
    parm->test_by_entry = mw->test_by_entry;
    if (parm->test_by_entry) {
        /* Not supported unless in fill mode */
        mw->reverify_delay = 0;
        mw->reverify_count = 0;
    }
    parm->reverify_delay = mw->reverify_delay;
    parm->reverify_count = mw->reverify_count;
    parm->continue_on_error = mw->continue_on_error;
    parm->error_count = mw->error_count;
    parm->error_max = mw->error_max;

    /* Place MMU in debug mode if testing an MMU memory */
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (soc_mem_is_debug(u, parm->mem) &&
        (mw->was_debug_mode = soc_mem_debug_set(u, 0)) < 0) {
            test_error(u, "Could not put MMU in debug mode\n");
            goto done;
    }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */

#ifdef BCM_DFE_SUPPORT
    if(SOC_IS_DFE(u)) {
        parm->frag_index_start[0] = parm->index_start;
        parm->frag_index_end[0] = parm->index_end;
        parm->frag_count = 1;
        SOC_MEM_TEST_SKIP_CACHE_SET(u, 1);
        /* Disable any parity control */
        if ((rv = soc_mem_parity_control(u, parm->mem, parm->copyno, FALSE)) < 0) {
            test_error(u, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(u, parm->mem));
            goto done;
        }
    } else 
#endif /* BCM_DFE_SUPPORT */
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        /* Do nothing for parity control */
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (mem_test_common_clear(u, parm->mem, parm->copyno)) {
            goto done;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        DRV_MEM_CACHE_GET(u, parm->mem, &cache_enable);
        if (cache_enable) {
            cli_out("WARNING: Caching is enabled on memory %s\n",
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_cache_get(u, parm->mem,
                  parm->copyno == COPYNO_ALL ?
                  MEM_BLOCK_ALL : parm->copyno)) {
                cli_out("WARNING: Caching is enabled on memory %s.%s\n",
                        SOC_MEM_UFNAME(u, parm->mem),
                        SOC_BLOCK_NAME(u, parm->copyno));
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }
    
    if (parm->copyno != COPYNO_ALL) {
        copies = 1;
    } else {
        copies = 0;
        SOC_MEM_BLOCK_ITER(u, parm->mem, blk) {
            copies += 1;
        }
    }
    mw->progress_total =
      (_shr_popcount(parm->patterns) *
         (1 + (parm->read_count * (1 + parm->reverify_count))) *
       copies *
        ((parm->index_end > parm->index_start) ?
         (parm->index_end - parm->index_start + 1) / parm->index_step :
         (parm->index_end - parm->index_start - 1) / parm->index_step)) *
        (parm->array_index_end - parm->array_index_start + 1);

    *p = mw;

    rv = 0;

done:
    parse_arg_eq_done(&pt);
    return rv;
}

int
mem_test_rw_init(int u, soc_mem_t mem, void **p)
{
    mem_testdata_t    *mw;
    soc_mem_test_t    *parm;    
    char        *idx_start_str, *idx_end_str;
    char        *array_idx_start_str, *array_idx_end_str;
    int            copies, blk;
    int            rv = -1;
    parse_table_t    pt;
#ifdef BCM_POLAR_SUPPORT
    uint32 cache_enable = FALSE;
#endif /* BCM_POLAR_SUPPORT */

    mw = mem_work[u];
    if (mw == NULL) {
        mw = sal_alloc(sizeof(mem_testdata_t), "memtest");
        if (mw == NULL) {
            cli_out("cannot allocate memory test data\n");
            return -1;
        }
        sal_memset(mw, 0, sizeof(mem_testdata_t));
        mem_work[u] = mw;
    }
    parm = &mw->parm;    

    parse_table_init(u, &pt); 
    

    parse_table_add(&pt,  "IndexStart",    PQ_STRING, "min",
                    &idx_start_str, NULL);
    parse_table_add(&pt,  "IndexEnd",    PQ_STRING, "max",
                    &idx_end_str, NULL);
    parse_table_add(&pt,  "ArrayIndexStart",    PQ_STRING, "min", &array_idx_start_str, NULL);
    parse_table_add(&pt,  "ArrayIndexEnd",    PQ_STRING, "max", &array_idx_end_str, NULL);
    parse_table_add(&pt,  "IndexInc",    PQ_INT, (void *) 1,
                    &parm->index_step, NULL);
    parse_table_add(&pt,  "PatZero",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_zero, NULL);
    parse_table_add(&pt,  "PatOne",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_one, NULL);
    parse_table_add(&pt,  "PatFive",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_five, NULL);
    parse_table_add(&pt,  "PatA",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_a, NULL);
    parse_table_add(&pt,  "PatChecker",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_checker, NULL);
    parse_table_add(&pt,  "PatInvckr",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_invckr, NULL);
    parse_table_add(&pt,  "PatLinear",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_linear, NULL);
    parse_table_add(&pt,  "PatRandom",    PQ_BOOL|PQ_DFL,    0,
                    &mw->pat_random, NULL);
    parse_table_add(&pt,  "PatHex",  PQ_BOOL|PQ_DFL, 0,
                    &mw->pat_hex, NULL);
    parse_table_add(&pt,  "HexByte",  PQ_INT|PQ_DFL, 0,
                    &mw->hex_byte, NULL);
    parse_table_add(&pt,  "TestByEntry",  PQ_BOOL|PQ_DFL, 0,
                    &mw->test_by_entry, NULL);
    parse_table_add(&pt,  "ReVerifyCount",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_count, NULL);
    parse_table_add(&pt,  "ReVerifyDelay",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_delay, NULL);
    parse_table_add(&pt,  "IGnoreErrors",  PQ_BOOL|PQ_DFL, 0,
                    &mw->continue_on_error, NULL);
    parse_table_add(&pt,  "ErrorCount",  PQ_INT|PQ_DFL, 0,
                    &mw->error_count, NULL);
    parse_table_add(&pt,  "ErrorMax",  PQ_INT|PQ_DFL, (void *) 1,
                    &mw->error_max, NULL);
    parse_table_add(&pt,  "ReadCount",    PQ_INT|PQ_DFL,    (void *) 1,
            &parm->read_count, NULL);
    parse_table_add(&pt, "EccAsData",   PQ_BOOL|PQ_DFL,  0,
                    &parm->ecc_as_data, NULL);
    parse_table_add(&pt,  "ReportProgress", PQ_BOOL|PQ_DFL, (void *)1,
                    &parm->report_progress, NULL);
    if (SOC_IS_JERICHO(u)) {
        parse_table_add(&pt, "IncPm",   PQ_BOOL,  0,
                        &parm->inc_port_macros, NULL);
    }
    mw->was_debug_mode = -1;

    if (!mw->inited) {
          mw->pat_zero = TRUE;
          mw->pat_one = TRUE;
          mw->pat_five = TRUE;
          mw->pat_a = TRUE;
          mw->pat_checker = TRUE;
          mw->pat_invckr = TRUE;
          mw->pat_linear = TRUE;
          mw->pat_random = TRUE;
          parm->unit = u;
          parm->status_cb = mt_status_fn;
          parm->write_cb = mt_write_fn;
          parm->read_cb = mt_read_fn;
          parm->miscompare_cb = mt_miscompare_fn;
          parm->read_count = 1;
          parm->ecc_as_data = 0;
          parm->report_progress = 1;
          mw->inited = 1;
    }
   /* if (parse_arg_eq(*p, &pt) < 0) {
        parse_arg_eq_done(&pt);
        return CMD_USAGE;
    } */  
    if (parse_default_fill(&pt) < 0) {
      cli_out("Invalid default fill:\n");
      goto done;
    }

  
    parm->mem = mem;
    parm->copyno = COPYNO_ALL;

    if (!soc_mem_is_valid(u, parm->mem) ||
        soc_mem_is_readonly(u, parm->mem)) {
#ifdef BCM_POLAR_SUPPORT
            if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
                test_error(u, "Memory %s is invalid or readonly\n",
                           SOC_ROBO_MEM_UFNAME(u, parm->mem));
            } else
#endif /* BCM_POLAR_SUPPORT */
            {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
                test_error(u, "Memory %s is invalid or readonly\n",
                           SOC_MEM_UFNAME(u, parm->mem));
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
            }
            goto done;
        }

    if (parm->copyno != COPYNO_ALL &&
        !SOC_MEM_BLOCK_VALID(u, parm->mem, parm->copyno)) {
#ifdef BCM_POLAR_SUPPORT
            if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
                test_error(u,
                           "Copy number %d out of range for memory %s\n",
                           parm->copyno, SOC_ROBO_MEM_UFNAME(u, parm->mem));
            } else
#endif /* BCM_POLAR_SUPPORT */
            {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
                test_error(u,
                           "Copy number %d out of range for memory %s\n",
                           parm->copyno, SOC_MEM_UFNAME(u, parm->mem));
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
            }
            goto done;
        }

    parm->index_start =
      parse_memory_index(parm->unit, parm->mem, idx_start_str);
    parm->index_end =
      parse_memory_index(parm->unit, parm->mem, idx_end_str);
    parm->array_index_start = parse_memory_array_index(parm->unit, parm->mem, array_idx_start_str);
    parm->array_index_end = parse_memory_array_index(parm->unit, parm->mem, array_idx_end_str);
    if (parm->array_index_start > parm->array_index_end ) {
        unsigned temp_ai = parm->array_index_start;
        parm->array_index_start = parm->array_index_end;
        parm->array_index_end = temp_ai;
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        if (parm->index_start < soc_robo_mem_index_min(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust start index from %d to %d "
                    "for memory %s\n",
                    parm->index_start,
                    soc_robo_mem_index_min(parm->unit, parm->mem),
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
            parm->index_start = soc_robo_mem_index_min(parm->unit, parm->mem);
        }
        if (parm->index_end > soc_robo_mem_index_max(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust end index from %d to %d "
                    "for memory %s\n",
                    parm->index_end,
                    soc_robo_mem_index_max(parm->unit, parm->mem),
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
            parm->index_end = soc_robo_mem_index_max(parm->unit, parm->mem);
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (parm->index_start < soc_mem_index_min(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust start index from %d to %d "
                    "for memory %s.%s\n",
                    parm->index_start,
                    soc_mem_index_min(parm->unit, parm->mem),
                    SOC_MEM_UFNAME(u, parm->mem),
                    SOC_BLOCK_NAME(u, parm->copyno));
            parm->index_start = soc_mem_index_min(parm->unit, parm->mem);
        }
        if (parm->index_end > soc_mem_index_max(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust end index from %d to %d "
                    "for memory %s.%s\n",
                    parm->index_end,
                    soc_mem_index_max(parm->unit, parm->mem),
                    SOC_MEM_UFNAME(u, parm->mem),
                    SOC_BLOCK_NAME(u, parm->copyno));
            parm->index_end = soc_mem_index_max(parm->unit, parm->mem);
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    if (parm->index_step == 0 ||
        (parm->index_start < parm->index_end && parm->index_step < 0) ||
        (parm->index_start > parm->index_end && parm->index_step > 0)) {
            test_error(u, "Illegal index step value\n");
            goto done;
    }

#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(u)) {
        parm->frag_index_start[0] = parm->index_start;
        parm->frag_index_end[0] = parm->index_start;
        parm->frag_index_start[1] = parm->index_end;
        parm->frag_index_end[1] = parm->index_end;
        parm->frag_count = 2;

    }
#endif

    parm->patterns = 0;
    if (mw->pat_zero)
        parm->patterns |= MT_PAT_ZEROES;
    if (mw->pat_one)
        parm->patterns |= MT_PAT_ONES;
    if (mw->pat_five)
        parm->patterns |= MT_PAT_FIVES;
    if (mw->pat_a)
        parm->patterns |= MT_PAT_AS;
    if (mw->pat_checker)
        parm->patterns |= MT_PAT_CHECKER;
    if (mw->pat_invckr)
        parm->patterns |= MT_PAT_ICHECKER;
    if (mw->pat_linear)
        parm->patterns |= MT_PAT_ADDR;
    if (mw->pat_random)
        parm->patterns |= MT_PAT_RANDOM;
    if (mw->pat_hex) {
        parm->patterns |= MT_PAT_HEX;
        parm->hex_byte = mw->hex_byte;
    }
    parm->test_by_entry = mw->test_by_entry;
    if (parm->test_by_entry) {
        /* Not supported unless in fill mode */
        mw->reverify_delay = 0;
        mw->reverify_count = 0;
    }
    parm->reverify_delay = mw->reverify_delay;
    parm->reverify_count = mw->reverify_count;
    parm->continue_on_error = mw->continue_on_error;
    parm->error_count = mw->error_count;
    parm->error_max = mw->error_max;

    /* Place MMU in debug mode if testing an MMU memory */
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (soc_mem_is_debug(u, parm->mem) &&
        (mw->was_debug_mode = soc_mem_debug_set(u, 0)) < 0) {
            test_error(u, "Could not put MMU in debug mode\n");
            goto done;
    }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */

#ifdef BCM_DFE_SUPPORT
    if(SOC_IS_DFE(u) || SOC_IS_DNXF(u)) {
        SOC_MEM_TEST_SKIP_CACHE_SET(u, 1);
        /* Disable any parity control */
        if ((rv = soc_mem_parity_control(u, parm->mem, parm->copyno, FALSE)) < 0) {
            test_error(u, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(u, parm->mem));
            goto done;
        }
    } else 
#endif /* BCM_DFE_SUPPORT */    
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        /* Do nothing for parity control */
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (mem_test_common_clear(u, parm->mem, parm->copyno)) {
            goto done;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        DRV_MEM_CACHE_GET(u, parm->mem, &cache_enable);
        if (cache_enable) {
            cli_out("WARNING: Caching is enabled on memory %s\n",
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));        
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_cache_get(u, parm->mem,
                  parm->copyno == COPYNO_ALL ?
                  MEM_BLOCK_ALL : parm->copyno)) {
                cli_out("WARNING: Caching is enabled on memory %s.%s\n",
                        SOC_MEM_UFNAME(u, parm->mem),
                        SOC_BLOCK_NAME(u, parm->copyno));
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    
    if (parm->copyno != COPYNO_ALL) {
        copies = 1;
    } else {
        copies = 0;
        SOC_MEM_BLOCK_ITER(u, parm->mem, blk) {
            copies += 1;
        }
    }
    mw->progress_total =
      (_shr_popcount(parm->patterns) *
         (1 + (parm->read_count * (1 + parm->reverify_count))) *
       copies *
        ((parm->index_end > parm->index_start) ?
         (parm->index_end - parm->index_start + 1) / parm->index_step :
         (parm->index_end - parm->index_start - 1) / parm->index_step)) *
        (parm->array_index_end - parm->array_index_start + 1);

    *p = mw;
    /* adds on for tr 7*/
    if (parm->index_end > parm->index_start) {
        parm->index_step = parm->index_end - parm->index_start ;
    }

    rv = 0;

done:
    parse_arg_eq_done(&pt);
    return rv;
}

char tr50_test_usage[] = 
"TR50 (single memory write & read test with a sophisticated patterns) usage:\n"
" \n"
 #ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pedantic compiler\n";
 #else

  "M=<value>	Memory - The memory to be checked\n"
  "IS=<value>	IndexStart - Starting index\n"
  "IE=<value>	IndexEnd - Ending index\n"
  "AIS=<value>	ArrayIndexStart - Array starting index (in case of array memory)\n"
  "AIE=<value>	ArrayIndexEnd - Array ending index (in case of array memory)\n"
  "II=<value>	IndexInc - index increment value \n"
  "PZ=<value>	PatZero - fills memory with 0's\n"
  "PO=<value>	PatOne - fills memory with 1's\n"
  "PF=<value>	PatFive - fills memory with 5's\n"
  "PA=<value>	PatA - fills memory with a's\n"
  "PC=<value>	PatChecker - fills memory with alternating 1010... and 0101... This is useful to detecting horizontally or vertically shorted data lines (in cases where the RAM layout is plain)\n"
  "PI=<value>	PatInvckr - fills memory with alternating 0101... and 1010... This is useful to detecting horizontally or vertically shorted data lines (in cases where the RAM layout is plain)\n"
  "PL=<value>	PatLinear - Write a unique value in every location to make sure all locations are distinct (detect shorted address bits). The unique value used is the address so dumping the failed memory may indicate where the bad data came from.\n"
  "PR=<value>	PatRandom - Write a unique value in every location using pseudo-random data\n"
  "PH=<value>	PatHex - Write a value specified by the user\n"
  "HB=<value>	HexByte - the actual value used in ph option\n"
  "TBE=<value>	TestByEntry - Test each memory line\n"
  "RVC=<value>\n"
  "IGE=<value>	IGnoreErrors - Do not abort test due to errors\n"
  "EC=<value>	ErrorCount - Count of miss-compares so far\n"
  "Help=<1/0>                      -  Specifies if tr 7 help is on and exit or off (off by default)\n"
  "To enable printing values (write/read) of each memory proceed: debug +test +verb\n"
  "\n";
  #endif


int
mem_test_init(int u, args_t *a, void **p)
{
    mem_testdata_t    *mw;
    soc_mem_test_t    *parm;
    char        *mem_name;
    char        *idx_start_str, *idx_end_str;
    char        *array_idx_start_str, *array_idx_end_str;
    int            copies, blk;
    int            rv = -1;
    uint32 help_status_default = 0;
    uint32 help_status=0;
    parse_table_t    pt;
#ifdef BCM_POLAR_SUPPORT
    uint32 cache_enable = FALSE;
#endif /* BCM_POLAR_SUPPORT */

    mw = mem_work[u];
    if (mw == NULL) {
    mw = sal_alloc(sizeof(mem_testdata_t), "memtest");
    if (mw == NULL) {
        cli_out("%s: cannot allocate memory test data\n", ARG_CMD(a));
        return -1;
    }
    sal_memset(mw, 0, sizeof(mem_testdata_t));
    mem_work[u] = mw;
    }
    parm = &mw->parm;

    parse_table_init(u, &pt);
    parse_table_add(&pt,  "Memory",    PQ_STRING, "",
            &mem_name, NULL);
    parse_table_add(&pt,  "IndexStart",    PQ_STRING, "min",
            &idx_start_str, NULL);
    parse_table_add(&pt,  "IndexEnd",    PQ_STRING, "max",
            &idx_end_str, NULL);
    parse_table_add(&pt,  "ArrayIndexStart", PQ_STRING, "min",
            &array_idx_start_str, NULL);
    parse_table_add(&pt,  "ArrayIndexEnd", PQ_STRING, "max",
            &array_idx_end_str, NULL);
    parse_table_add(&pt,  "IndexInc",    PQ_INT, (void *) 1,
            &parm->index_step, NULL);

    parse_table_add(&pt,  "PatZero",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_zero, NULL);
    parse_table_add(&pt,  "PatOne",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_one, NULL);
    parse_table_add(&pt,  "PatFive",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_five, NULL);
    parse_table_add(&pt,  "PatA",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_a, NULL);
    parse_table_add(&pt,  "PatChecker",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_checker, NULL);
    parse_table_add(&pt,  "PatInvckr",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_invckr, NULL);
    parse_table_add(&pt,  "PatLinear",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_linear, NULL);
    parse_table_add(&pt,  "PatRandom",    PQ_BOOL|PQ_DFL,    0,
            &mw->pat_random, NULL);
    parse_table_add(&pt,  "PatHex",  PQ_BOOL|PQ_DFL, 0,
                    &mw->pat_hex, NULL);
    parse_table_add(&pt,  "HexByte",  PQ_INT|PQ_DFL, 0,
                    &mw->hex_byte, NULL);
    parse_table_add(&pt,  "TestByEntry",  PQ_BOOL|PQ_DFL, 0,
                    &mw->test_by_entry, NULL);
    parse_table_add(&pt,  "ReVerifyCount",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_count, NULL);
    parse_table_add(&pt,  "ReVerifyDelay",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_delay, NULL);
    parse_table_add(&pt,  "IGnoreErrors",  PQ_BOOL|PQ_DFL, 0,
                    &mw->continue_on_error, NULL);
    parse_table_add(&pt,  "ErrorCount",  PQ_INT|PQ_DFL, 0,
                    &mw->error_count, NULL);
    parse_table_add(&pt,  "ErrorMax",  PQ_INT|PQ_DFL, (void *) 1,
                    &mw->error_max, NULL);
    parse_table_add(&pt,  "ReadCount",    PQ_INT|PQ_DFL,    (void *) 1,
            &parm->read_count, NULL);
    parse_table_add(&pt, "EccAsData",   PQ_BOOL|PQ_DFL,  0,
                    &parm->ecc_as_data, NULL);
    parse_table_add(&pt,  "ReportProgress", PQ_BOOL|PQ_DFL, (void *)1,
                    &parm->report_progress, NULL);
    parse_table_add(&pt, "Help",    PQ_DFL|PQ_INT, &help_status_default, &help_status,           NULL);

    mw->was_debug_mode = -1;

    if (!mw->inited) {
        mw->pat_zero = TRUE;
        mw->pat_one = TRUE;
        mw->pat_five = TRUE;
        mw->pat_a = TRUE;
        mw->pat_checker = TRUE;
        mw->pat_invckr = TRUE;
        mw->pat_linear = TRUE;
        mw->pat_random = TRUE;
        parm->unit = u;
        parm->status_cb = mt_status_fn;
        parm->write_cb = mt_write_fn;
        parm->read_cb = mt_read_fn;
        parm->miscompare_cb = mt_miscompare_fn;
        parm->read_count = 1;
        parm->ecc_as_data = 0;
        parm->report_progress = 1;
        mw->inited = 1;
    }

    if (parse_arg_eq(a, &pt) < 0) {
    cli_out("%s: Invalid option: %s\n",
            ARG_CMD(a), ARG_CUR(a));
    goto done;
    }

    if (help_status == 1) {
        cli_out("%s\n",tr50_test_usage);
        goto done;
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(u)) {
      if (sal_strcasecmp(mem_name,"EXT_DDR") == 0) {
    return (sbx_mem_init(u,a,p));
      }
    }
#endif

    if (ARG_CNT(a) != 0) {
    cli_out("%s: extra options starting with \"%s\"\n",
            ARG_CMD(a), ARG_CUR(a));
    goto done;
    }

    if (parse_memory_name(u,
              &parm->mem,
              mem_name,
              &parm->copyno,
              0) < 0) {
    test_error(u, "Memory \"%s\" is invalid or Missing\n", mem_name);
    goto done;
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(u)) {
        if ( (parm->mem == EG_FD_PER_PORT_DROP_COUNT1m) ||
         (parm->mem == EG_FD_PER_PORT_DROP_COUNT2m) ||
         (parm->mem == GLOBAL_STATSm) ||
         (parm->mem == RT_BKm) ||
         (parm->mem == TC_FREE_POOLm) ||
         (parm->mem == TX_SFI_CFIFOm) ||
         (parm->mem == TX_SFI_DFIFOm) ) {
        cli_out("WARNING: TR 50 doesn't apply to memory %s\n",
                SOC_MEM_UFNAME(u, parm->mem));
        goto done;
    }
    }
#endif

    if (!soc_mem_is_valid(u, parm->mem) ||
        soc_mem_is_readonly(u, parm->mem) 
#if defined(BCM_DFE_SUPPORT) || defined(BCM_PETRA_SUPPORT)
        || 
        (SOC_IS_SAND(u) && (soc_mem_is_writeonly(u, parm->mem) || soc_mem_is_signal(u, parm->mem))) 
#endif

        ) {
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
            test_error(u, "Memory %s is invalid/readonly/writeonly/cannot be compared \n",
                       SOC_ROBO_MEM_UFNAME(u, parm->mem));
        } else
#endif /* BCM_POLAR_SUPPORT */
        {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
            test_error(u, "Memory %s is invalid/readonly/writeonly/cannot be compared \n",
                       SOC_MEM_UFNAME(u, parm->mem));
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
        }
        goto done;
    }

#ifdef BCM_PETRA_SUPPORT
    if(SOC_IS_ARAD(u)) {
        switch(parm->mem) {
            case BRDC_FSRD_FSRD_WL_EXT_MEMm:
            case IQM_MEM_8000000m:
            case NBI_TBINS_MEMm:
            case NBI_RBINS_MEMm:
            case PORT_WC_UCMEM_DATAm:
            case ECI_MBU_MEMm:
            case EGQ_CBMm:
            case EGQ_RDMUCm:
            case IHP_MEM_740000m:
            case IHP_MEM_9E0000m:
            case IHP_MEM_C50000m:
            
               test_error(u, "Memory %s is invalid/readonly/writeonly/signal \n",
                       SOC_MEM_UFNAME(u, parm->mem));

               goto done;
            default:
                break;
        }
    }
#endif

#if defined (BCM_DFE_SUPPORT)
    if(SOC_IS_DFE(u)) 
    { 
        int rv;
        int is_filtered = 0;
        
        rv = MBCM_DFE_DRIVER_CALL(u, mbcm_dfe_drv_test_mem_filter, (u, parm->mem, &is_filtered));
        if (rv != SOC_E_NONE)
        {
            return rv;
        }
        if (is_filtered)
        {
               test_error(u, "Memory %s is invalid/readonly/writeonly/signal \n",
                       SOC_MEM_UFNAME(u, parm->mem));
               goto done;
        }
    }
#endif /*BCM_DFE_SUPPORT*/

    if (parm->copyno != COPYNO_ALL &&
    !SOC_MEM_BLOCK_VALID(u, parm->mem, parm->copyno)) {
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
            test_error(u,
                   "Copy number %d out of range for memory %s\n",
                   parm->copyno, SOC_ROBO_MEM_UFNAME(u, parm->mem));
        } else
#endif /* BCM_POLAR_SUPPORT */
        {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
            test_error(u,
                   "Copy number %d out of range for memory %s\n",
                   parm->copyno, SOC_MEM_UFNAME(u, parm->mem));
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
        }
        goto done;
    }

    parm->index_start = parse_memory_index(parm->unit, parm->mem, idx_start_str);
    parm->index_end = parse_memory_index(parm->unit, parm->mem, idx_end_str);
    parm->array_index_start = parse_memory_array_index(parm->unit, parm->mem, array_idx_start_str);
    parm->array_index_end = parse_memory_array_index(parm->unit, parm->mem, array_idx_end_str);
    if (parm->array_index_start > parm->array_index_end ) {
        unsigned temp_ai = parm->array_index_start;
        parm->array_index_start = parm->array_index_end;
        parm->array_index_end = temp_ai;
        cli_out("WARNING: switching start and end array indices to %u-%u\n",
                parm->array_index_start, parm->array_index_end);
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        if (parm->index_start < soc_robo_mem_index_min(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust start index from %d to %d "
                    "for memory %s\n",
                    parm->index_start,
                    soc_robo_mem_index_min(parm->unit, parm->mem),
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
            parm->index_start = soc_robo_mem_index_min(parm->unit, parm->mem);
        }
        if (parm->index_end > soc_robo_mem_index_max(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust end index from %d to %d "
                    "for memory %s\n",
                    parm->index_end,
                    soc_robo_mem_index_max(parm->unit, parm->mem),
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
            parm->index_end = soc_robo_mem_index_max(parm->unit, parm->mem);            
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (parm->index_start < soc_mem_index_min(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust start index from %d to %d "
                    "for memory %s.%s\n",
                    parm->index_start,
                    soc_mem_index_min(parm->unit, parm->mem),
                    SOC_MEM_UFNAME(u, parm->mem),
                    SOC_BLOCK_NAME(u, parm->copyno));
            parm->index_start = soc_mem_index_min(parm->unit, parm->mem);
        }

        if (parm->index_end > soc_mem_index_max(parm->unit, parm->mem)) {
            cli_out("WARNING: Adjust end index from %d to %d "
                    "for memory %s.%s\n",
                    parm->index_end,
                    soc_mem_index_max(parm->unit, parm->mem),
                    SOC_MEM_UFNAME(u, parm->mem),
                    SOC_BLOCK_NAME(u, parm->copyno));
            parm->index_end = soc_mem_index_max(parm->unit, parm->mem);
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    if (parm->index_step == 0 ||
       (parm->index_start < parm->index_end && parm->index_step < 0) ||
       (parm->index_start > parm->index_end && parm->index_step > 0)) {
       if (soc_feature(parm->unit,soc_feature_esm_support)) {
#if defined(BCM_ESW_SUPPORT)
            if ((parm->mem == L3_DEFIPm && SOC_MEM_IS_ENABLED(u, L3_DEFIPm)) ||
                (parm->mem == L3_DEFIP_ONLYm && SOC_MEM_IS_ENABLED(u, L3_DEFIP_ONLYm)) ||
                (parm->mem == L3_DEFIP_DATA_ONLYm && SOC_MEM_IS_ENABLED(u, L3_DEFIP_DATA_ONLYm)) ||
                (parm->mem == L3_DEFIP_HIT_ONLYm && SOC_MEM_IS_ENABLED(u, L3_DEFIP_DATA_ONLYm)) ||
                (parm->mem == L3_DEFIP_PAIR_128_DATA_ONLYm && SOC_MEM_IS_ENABLED(u, L3_DEFIP_PAIR_128_DATA_ONLYm)) ||
                (parm->mem == L3_DEFIP_PAIR_128_HIT_ONLYm && SOC_MEM_IS_ENABLED(u, L3_DEFIP_PAIR_128_HIT_ONLYm))) {
                /* The above internal memories will not be supported if External TCAM is present.
                * These memories are set to 0 and hence can not run the test on these memories.*/
                return BCM_E_UNAVAIL;
            } else {
                test_error(u, "Illegal index step value\n");
                goto done;
             }
#endif /* BCM_ESW_SUPPORT */
       }
       else {
         test_error(u, "Illegal index step value\n");
         goto done;
       }
    }

    parm->patterns = 0;
    if (mw->pat_zero)
    parm->patterns |= MT_PAT_ZEROES;
    if (mw->pat_one)
    parm->patterns |= MT_PAT_ONES;
    if (mw->pat_five)
    parm->patterns |= MT_PAT_FIVES;
    if (mw->pat_a)
    parm->patterns |= MT_PAT_AS;
    if (mw->pat_checker)
    parm->patterns |= MT_PAT_CHECKER;
    if (mw->pat_invckr)
    parm->patterns |= MT_PAT_ICHECKER;
    if (mw->pat_linear)
    parm->patterns |= MT_PAT_ADDR;
    if (mw->pat_random)
    parm->patterns |= MT_PAT_RANDOM;
    if (mw->pat_hex) {
        parm->patterns |= MT_PAT_HEX;
        parm->hex_byte = mw->hex_byte;
    }
    parm->test_by_entry = mw->test_by_entry;
    if (parm->test_by_entry) {
        /* Not supported unless in fill mode */
        mw->reverify_delay = 0;
        mw->reverify_count = 0;
    }
    parm->reverify_delay = mw->reverify_delay;
    parm->reverify_count = mw->reverify_count;
    parm->continue_on_error = mw->continue_on_error;
    parm->error_count = mw->error_count;
    parm->error_max = mw->error_max;

    /* Place MMU in debug mode if testing an MMU memory */
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (soc_mem_is_debug(u, parm->mem) &&
    (mw->was_debug_mode = soc_mem_debug_set(u, 0)) < 0) {
    test_error(u, "Could not put MMU in debug mode\n");
    goto done;
    }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */

    /* Disable non-atomic TCAM write handling */
    mw->saved_tcam_protect_write = SOC_CONTROL(u)->tcam_protect_write;
    SOC_CONTROL(u)->tcam_protect_write = FALSE;

#ifdef BCM_DFE_SUPPORT
    if(SOC_IS_DFE(u)) {
        SOC_MEM_TEST_SKIP_CACHE_SET(u, 1);
        /* Disable any parity control */
        if ((rv = soc_mem_parity_control(u, parm->mem, parm->copyno, FALSE)) < 0) {
            test_error(u, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(u, parm->mem));
            goto done;
        }  
    } else 
#endif /* BCM_DFE_SUPPORT */
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        /* Do nothing for parity control */
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (mem_test_common_clear(u, parm->mem, parm->copyno)) {
            goto done;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_ROBO(u) && SOC_IS_POLAR(u)) {
        DRV_MEM_CACHE_GET(u, parm->mem, &cache_enable);
        if (cache_enable) {
            cli_out("WARNING: Caching is enabled on memory %s\n",
                    SOC_ROBO_MEM_UFNAME(u, parm->mem));
        }
    } else
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_cache_get(u, parm->mem,
                  parm->copyno == COPYNO_ALL ?
                  MEM_BLOCK_ALL : parm->copyno)) {
                cli_out("WARNING: Caching is enabled on memory %s.%s\n",
                        SOC_MEM_UFNAME(u, parm->mem),
                        SOC_BLOCK_NAME(u, parm->copyno));
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if ((rv = soc_mem_cpu_write_control(u, parm->mem, parm->copyno,
                                        TRUE, &mw->orig_enable)) < 0) {
        test_error(u, "Could not enable exclusive cpu write on memory %s\n",
                   SOC_MEM_UFNAME(u, parm->mem));
        goto done;
    }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */

    /*
     * Turn off L2 task to keep it from going crazy if L2 memory is
     * being tested.
     */

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (soc_feature(u, soc_feature_arl_hashed)) {
    (void)soc_l2x_stop(u);
    }
#endif /* BCM_XGS_SWITCH_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(u) && (parm->mem == EGR_PERQ_XMT_COUNTERSm)) {
        rv = soc_reg_field32_modify(u, EGR_EDB_HW_CONTROLr, REG_PORT_ANY,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
       if (rv != SOC_E_NONE) {
           goto done;
       }
    }
    if (SOC_IS_SHADOW(u) && SOC_BLOCK_IS_CMP(u,
        SOC_MEM_BLOCK_MIN(u, parm->mem), SOC_BLK_MS_ISEC)) {
        rv = soc_reg_field32_modify(u, ISEC_MASTER_CTRLr, 1,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
            return -1;
       }
        rv = soc_reg_field32_modify(u, ISEC_MASTER_CTRLr, 5,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
       if (rv != SOC_E_NONE) {
           return -1;
       }
    }
    /* Disable WRED */
    if (SOC_IS_SHADOW(u)) {
        uint32              rval;
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(u, &rval));
        soc_reg_field_set(u, MISCCONFIGr, &rval,
                      METERING_CLK_ENf, 0);
        soc_reg_field_set(u, MISCCONFIGr, &rval,
                      REFRESH_ENf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(u, rval));
    }
    if (SOC_IS_SHADOW(u) && SOC_BLOCK_IS_CMP(u,
        SOC_MEM_BLOCK_MIN(u, parm->mem), SOC_BLK_MS_ESEC)) {
        rv = soc_reg_field32_modify(u, ESEC_MASTER_CTRLr, 1,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
            return -1;
        }
        rv = soc_reg_field32_modify(u, ESEC_MASTER_CTRLr, 5,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
           return -1;
        }
    }
#endif

    /*
     * Turn off FP.
     * Required when testing FP and METERING memories or tests will fail.
     * We don't care about the return value as the BCM layer may not have
     * been initialized at all.
     */
    (void)bcm_field_detach(u);

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(u, soc_feature_esm_support)) {
        /* Some tables have dependency on the content of other table */
        switch (parm->mem) {
        case EXT_ACL360_TCAM_DATAm:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
            rv = soc_mem_clear(u, EXT_ACL360_TCAM_MASKm, MEM_BLOCK_ALL, TRUE);
            break;
        case EXT_ACL432_TCAM_DATAm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:
            rv = soc_mem_clear(u, EXT_ACL432_TCAM_MASKm, MEM_BLOCK_ALL, TRUE);
            break;
        default:
            break;
        }
    if (rv != SOC_E_NONE) {
        goto done;
    }
    }
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_TRIUMPH3_SUPPORT
    if (soc_feature(u, soc_feature_etu_support)) {
        /* Some tables have dependency on the content of other table */
        switch (parm->mem) {
        case EXT_ACL480_TCAM_DATAm:
            if (SOC_FAILURE(soc_mem_clear(u, EXT_ACL480_TCAM_MASKm,
                MEM_BLOCK_ALL, TRUE)))
            {
                test_error(u, "Failed to clear EXT_ACL360_TCAM_MASKm\n");
                parse_arg_eq_done(&pt);
                return -1;
            }
            break;
        default:
            break;
        }
    }
#endif /* BCM_TRIUMPH3_SUPPORT */
#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_CALADAN3(u)) {

        /* 
         * no clean up is done for this config, 
         * init soc will restore the driver config 
         */
        uint32    rval = 0;
        SOC_IF_ERROR_RETURN(READ_LRA_CONFIG0r(u, &rval));
        soc_reg_field_set(u, LRA_CONFIG0r, &rval, ENABLEf, 0);
        SOC_IF_ERROR_RETURN(WRITE_LRA_CONFIG0r(u, rval));
        
        if (parm->mem == OC_MEMORYm) {
            oc_lrp_port0_segment_entry_t entry;
            oc_lrp_port0_block_entry_t block_entry;
            int i;

            if (parm->index_end > 2*1024*1024) {
                /* 128Mbit total, 64bits per entry => 2M entries */
                parm->index_end = 2*1024*1024 - 1;
            }
            rval = 0;
            SOC_IF_ERROR_RETURN(READ_OC_CONFIGr(u, &rval));
            soc_reg_field_set(u, OC_CONFIGr, &rval, SOFT_RESET_Nf, 1);
            soc_reg_field_set(u, OC_CONFIGr, &rval, PROC_ACCESS_RAWf, 1);
            soc_reg_field_set(u, OC_CONFIGr, &rval, PROC_ACCESS_PHYSICALf, 1);
            soc_reg_field_set(u, OC_CONFIGr, &rval, PROC_PORT_IDf, 0);
            soc_reg_field_set(u, OC_CONFIGr, &rval, PROC_PORT_SEGMENTf, 0);
            soc_reg_field_set(u, OC_CONFIGr, &rval, DMA_ACCESS_PHYSICALf, 1);
            soc_reg_field_set(u, OC_CONFIGr, &rval, DMA_PORT_IDf, 0);
            soc_reg_field_set(u, OC_CONFIGr, &rval, DMA_PORT_SEGMENTf, 0);
            SOC_IF_ERROR_RETURN(WRITE_OC_CONFIGr(u, rval));

            SOC_IF_ERROR_RETURN(soc_mem_read(u, 
                                OC_LRP_PORT0_SEGMENTm, MEM_BLOCK_ANY, 0, &entry));
            rval = parm->index_end;
            soc_mem_field_set(u, OC_LRP_PORT0_SEGMENTm,
                          &entry.entry_data[0], SEGMENT_LIMITf, &rval);
            rval = 0;
            soc_mem_field_set(u, OC_LRP_PORT0_SEGMENTm,
                          &entry.entry_data[0], SEGMENT_BASEf, &rval);
            soc_mem_field_set(u, OC_LRP_PORT0_SEGMENTm,
                          &entry.entry_data[0], SEGMENT_WRITE_PROTf, &rval);
            soc_mem_field_set(u, OC_LRP_PORT0_SEGMENTm,
                          &entry.entry_data[0], SEGMENT_NULL_OFFSETf, &rval);
            rval = 2; /* ecc */
            soc_mem_field_set(u, OC_LRP_PORT0_SEGMENTm,
                          &entry.entry_data[0], SEGMENT_ERR_PROTf, &rval);
            rval = 6; /* 64bit access */
            soc_mem_field_set(u, OC_LRP_PORT0_SEGMENTm,
                          &entry.entry_data[0], SEGMENT_SIZEf, &rval);
            SOC_IF_ERROR_RETURN(soc_mem_write(u, 
                                OC_LRP_PORT0_SEGMENTm, MEM_BLOCK_ANY, 0, &entry));

            for (i=0; i < 128; i++) {
                sal_memset(&block_entry.entry_data[0], 0,
                              sizeof(oc_lrp_port0_block_entry_t));
                rval = 1; /* valid */
                soc_mem_field_set(u, OC_LRP_PORT0_BLOCKm,
                       &block_entry.entry_data[0], VALIDf, &rval);
                soc_mem_field_set(u, OC_LRP_PORT0_BLOCKm,
                       &block_entry.entry_data[0], PHYSICAL_BLOCKf, (uint32*)&i);
                SOC_IF_ERROR_RETURN(soc_mem_write(u,
                                OC_LRP_PORT0_BLOCKm, MEM_BLOCK_ANY, i, &block_entry));
            }
        }
        if (parm->mem == PORT_WC_UCMEM_DATAm) {
            int phy_port = 0, port = 0;
            rval = 0;
            PBMP_PORT_ITER(u, port) {
                if (soc_feature(u, soc_feature_logical_port_num)) {
                    phy_port = SOC_INFO(u).port_l2p_mapping[port];
                } else {
                    phy_port = port;
                }
                SOC_IF_ERROR_RETURN(READ_PORT_WC_UCMEM_CTRLr(u, phy_port, &rval));
                soc_reg_field_set(u, PORT_WC_UCMEM_CTRLr, &rval, ACCESS_MODEf, 1);
                SOC_IF_ERROR_RETURN(WRITE_PORT_WC_UCMEM_CTRLr(u, phy_port, rval));
            }
        }
    }
#endif

    if (parm->copyno != COPYNO_ALL) {
    copies = 1;
    } else {
    copies = 0;
    SOC_MEM_BLOCK_ITER(u, parm->mem, blk) {
        copies += 1;
    }
    }
    mw->progress_total =
    (_shr_popcount(parm->patterns) *
         (1 + (parm->read_count * (1 + parm->reverify_count))) *
     copies *
     ((parm->index_end > parm->index_start) ?
      (parm->index_end - parm->index_start + 1) / parm->index_step :
      (parm->index_end - parm->index_start - 1) / parm->index_step)) *
     (parm->array_index_end - parm->array_index_start + 1);

    parm->index_total = parm->index_start < parm->index_end ?
        parm->index_end - parm->index_start :
        parm->index_start - parm->index_end;
    parm->frag_index_start[0] = parm->index_start;
    parm->frag_index_end[0] = parm->index_end;
    parm->frag_count = 1;
    *p = mw;
#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(u))
    {

      bcm_common_linkscan_enable_set(u,0);
      soc_counter_stop(u);
    }
#endif
    rv = 0;

 done:
    parse_arg_eq_done(&pt);
    return rv;
}

/*
 * Memory Test
 */

int
mem_test(int u, args_t *a, void *p)
{
    mem_testdata_t    *mw = p;
    int            rv;

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(u)) {
      
      if (mw->inited == SBX_EXT_DDR_FLAG) {
      return (sbx_mem_test(u,a,p));
      }
    }
#endif

    progress_init(mw->progress_total, 3, 0);

    rv = soc_mem_test(&mw->parm);

    progress_done();

    return rv;
}

int
mem_test_done(int unit, void *p)
{
    mem_testdata_t    *mw = p;
#ifdef BCM_SHADOW_SUPPORT
    int rv = SOC_E_NONE;
#endif

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      
      if (mw->inited == SBX_EXT_DDR_FLAG) {
    return (sbx_test_done(unit,p));
      }
    }
#endif
#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_CALADAN3(unit)) {
        if (mw->parm.mem == OC_MEMORYm) {
            uint32    rval = 0;
            SOC_IF_ERROR_RETURN(READ_OC_CONFIGr(unit, &rval));
            soc_reg_field_set(unit, OC_CONFIGr, &rval,
                          PROC_ACCESS_RAWf, 0);
            soc_reg_field_set(unit, OC_CONFIGr, &rval,
                          PROC_ACCESS_PHYSICALf, 0);
            SOC_IF_ERROR_RETURN(WRITE_OC_CONFIGr(unit, rval));
        }
    }
#endif

    if (mw) {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        soc_mem_test_t    *parm = &mw->parm;
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT ||| BCM_SHADOW_SUPPORT */

        /* Take MMU out of debug mode if testing a CBP memory */

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_is_debug(parm->unit, parm->mem) &&
            mw->was_debug_mode >= 0 &&
            soc_mem_debug_set(unit, mw->was_debug_mode) < 0) {
            test_error(unit, "Could not restore previous MMU debug state\n");
            return -1;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */

        /* Restore non-atomic TCAM write handling status */
        SOC_CONTROL(unit)->tcam_protect_write = mw->saved_tcam_protect_write;

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (soc_mem_cpu_write_control(unit, parm->mem, parm->copyno,
                                      mw->orig_enable, &mw->orig_enable) < 0) {
            test_error(unit, "Could not disable exclusive cpu write on memory "
                       "%s\n",
                       SOC_MEM_UFNAME(unit, parm->mem));
            return -1;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT || BCM_CALADAN3_SUPPORT */

#ifdef BCM_DFE_SUPPORT
        if(SOC_IS_DFE(unit)) {
            SOC_MEM_TEST_SKIP_CACHE_SET(unit, 0);
            if (soc_mem_parity_restore(unit, parm->mem, parm->copyno) < 0) {
                test_error(unit, "Could not enable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, parm->mem));
                return -1;
            }
        } else 
#endif /* BCM_DFE_SUPPORT */
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_ROBO(unit) && SOC_IS_POLAR(unit)) {
            /* Do nothing for parity control */
        } else
#endif /* BCM_POLAR_SUPPORT */
        {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
            if (mem_test_common_restore(unit, parm->mem, parm->copyno)) {
                return -1;
            }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_PETRA_SUPPORT ||  BCM_CALADAN3_SUPPORT */
        }

#ifdef BCM_SHADOW_SUPPORT
        if (SOC_IS_SHADOW(unit) && (parm->mem == EGR_PERQ_XMT_COUNTERSm)) {
            rv = soc_reg_field32_modify(unit, EGR_EDB_HW_CONTROLr, REG_PORT_ANY,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
        }
        if (SOC_IS_SHADOW(unit)) {
            uint32              rval;
            SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &rval));
            soc_reg_field_set(unit, MISCCONFIGr, &rval,
                          METERING_CLK_ENf, 1);
            soc_reg_field_set(unit, MISCCONFIGr, &rval,
                          REFRESH_ENf, 1);
            SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, rval));
        }

        if (SOC_IS_SHADOW(unit) && SOC_BLOCK_IS_CMP(unit,
            SOC_MEM_BLOCK_MIN(unit, parm->mem), SOC_BLK_MS_ISEC)) {
            rv = soc_reg_field32_modify(unit, ISEC_MASTER_CTRLr, 1,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
            rv = soc_reg_field32_modify(unit, ISEC_MASTER_CTRLr, 5,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
        }
        if (SOC_IS_SHADOW(unit) && SOC_BLOCK_IS_CMP(unit,
            SOC_MEM_BLOCK_MIN(unit, parm->mem), SOC_BLK_MS_ESEC)) {
            rv = soc_reg_field32_modify(unit, ESEC_MASTER_CTRLr, 1,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
            rv = soc_reg_field32_modify(unit, ESEC_MASTER_CTRLr, 5,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
        }
#endif

    }
    return 0;
}

#if (defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT))
/*
 * Memories read/write test (tr 6)
 */
int
memories_rw_test(int unit, args_t *a, void *pa)
{
        int rv = 0;

        COMPILER_REFERENCE(pa);
    
        LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "Memories read test\n")));

        /*
         * If try_reg returns -1, there was a register access failure rather
         * than a bit error.
         *
         * If try_reg returns 0, then rd.error is -1 if there was a bit
         * error.
         */
        /*    soc_memories_bcm88650_a0 */
#ifdef BCM_PETRA_SUPPORT
        if (SOC_IS_ARAD(unit))
        {
          bcm_common_linkscan_enable_set(unit,0);
          soc_counter_stop(unit);
          if (SOC_IS_ARADPLUS_AND_BELOW(unit)){
              if((rv = bcm_dpp_counter_bg_enable_set(unit, FALSE)) < 0)
              {
                  LOG_WARN(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "WARNING: Unable to deactivate counter bg thread, unit %d \n"), unit));
              }
          }
          
          if ((rv = soc_dpp_device_reset(unit, SOC_DPP_RESET_MODE_REG_ACCESS, SOC_DPP_RESET_ACTION_INOUT_RESET)) < 0) {
              LOG_ERROR(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "ERROR: Unable to access regs, unit %d \n"), unit));
              test_error(unit, "Register read/write test failed\n");
              return rv;
          }
          
          rv = enable_dynamic_memories_access(unit); 
          if(SOC_FAILURE(rv)){
              return rv;
          } 
        }

#endif    
#if defined (BCM_DFE_SUPPORT) 
        if (SOC_IS_DFE(unit)) 
        {       
          rv = do_mem_test(unit,a);
        }
        else
#endif
        {
          rv = do_dump_memories(unit, a);
        }
      
        if (rv < 0) {
            test_error(unit, "Memory Read test failed (TR 6)\n");
        } else {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "Memories read test finished successfully\n")));
        }
    
        return rv;
}


STATIC int
enable_dynamic_memories_access_cb(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    uint32 value = *(uint32 *)data;
    soc_block_types_t blocks = SOC_REG_INFO(unit, ainfo->reg).block;
    soc_block_type_t block  = SOC_REG_FIRST_BLK_TYPE(blocks);
    int blk,inst=0;

    SOC_INIT_FUNC_DEFS;
    if ( SOC_IS_JERICHO(unit) ) {
        switch (block) {
            case SOC_BLK_BRDC_FMAC :
            case SOC_BLK_DRCA :
            case SOC_BLK_DRCB :
            case SOC_BLK_DRCBROADCAST :
            case SOC_BLK_DRCC :
            case SOC_BLK_DRCD :
            case SOC_BLK_DRCE :
            case SOC_BLK_DRCF :
            case SOC_BLK_DRCG :
            case SOC_BLK_DRCH :
            case SOC_BLK_FDR :
            case SOC_BLK_FMAC :
            case SOC_BLK_NBIH :
            case SOC_BLK_NBIL :
               SOC_EXIT; /* the enable dynamic registers of this blocks  is not accessable*/
            default:
                break;
        }
    }

    if (!SOC_REG_IS_VALID(unit,ainfo->reg) || 
        !SOC_REG_FIELD_VALID(unit,ainfo->reg,ENABLE_DYNAMIC_MEMORY_ACCESSf)) {
        SOC_EXIT;
    }
    SOC_BLOCK_ITER(unit, blk, SOC_REG_FIRST_BLK_TYPE(SOC_REG_INFO(unit, ainfo->reg).block)) {
        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, ainfo->reg, inst,ENABLE_DYNAMIC_MEMORY_ACCESSf, value));
        inst++;
    }


exit:
if (_rv!= SOC_E_NONE) {
    cli_out("Skip  enable %s\n",SOC_REG_NAME(unit,ainfo->reg));
    _rv=SOC_E_NONE;

}
SOC_FUNC_RETURN; 

}


/*
 * Enables dynamic memory access 
 */
soc_error_t 
enable_dynamic_memories_access(int unit)
{
  uint32 write_val = 1;
  uint64 write_val64; 
  soc_reg_above_64_val_t write_val_above64;

    SOC_INIT_FUNC_DEFS;

    if (SOC_IS_DFE(unit)) {
        SOC_EXIT;
    }

    _SOC_IF_ERR_EXIT(soc_reg_iterate(unit, enable_dynamic_memories_access_cb, &write_val)) ;


    if ( SOC_IS_JERICHO(unit) ) {

        _SOC_IF_ERR_EXIT(soc_reg32_set(unit, EGQ_EGRESS_SHAPER_ENABLE_SETTINGSr, REG_PORT_ANY, 0, 0));

        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, SCH_CIR_SHAPERS_CONFIGURATIONr, REG_PORT_ANY,
                                                 CIR_SHAPERS_CAL_ACCESS_PERIODf, 0x0));
        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, SCH_CIR_SHAPERS_CONFIGURATION_1r, REG_PORT_ANY,
                                                 CIR_SHAPERS_CAL_ACCESS_PERIOD_1f, 0x0));
        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, SCH_PIR_SHAPERS_CONFIGURATIONr, REG_PORT_ANY,
                                                 PIR_SHAPERS_CAL_ACCESS_PERIODf, 0x0));
        _SOC_IF_ERR_EXIT(soc_reg_field32_modify(unit, SCH_PIR_SHAPERS_CONFIGURATION_1r, REG_PORT_ANY,
                                                 PIR_SHAPERS_CAL_ACCESS_PERIOD_1f, 0x0));
        COMPILER_64_SET(write_val64, 0x0, 0x0);
        _SOC_IF_ERR_EXIT(WRITE_IPS_CREDIT_WATCHDOG_CONFIGURATIONr(unit, REG_PORT_ANY, write_val64));
    }


    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {

        COMPILER_64_SET(write_val64, 0xffffffff, 0xffffffff);
        if (!SOC_IS_ARDON(unit)) {
            _SOC_IF_ERR_EXIT(WRITE_NBI_INDIRECT_WR_MASKr(unit,write_val64));/* 64 */

            _SOC_IF_ERR_EXIT(READ_IDR_GLBL_CFGr(unit, &write_val));
            soc_reg_field_set(unit, IDR_GLBL_CFGr, &write_val, MCDB_INITf, 0);
            soc_reg_field_set(unit, IDR_GLBL_CFGr, &write_val, MCDA_INITf, 0);
            _SOC_IF_ERR_EXIT(WRITE_IDR_GLBL_CFGr(unit, write_val));
        }

        SOC_REG_ABOVE_64_ALLONES(write_val_above64);
        _SOC_IF_ERR_EXIT(WRITE_IDR_INDIRECT_WR_MASKr(unit,write_val_above64));  /* 640 */

        _SOC_IF_ERR_EXIT(WRITE_CMIC_SBUS_TIMEOUTr(unit, 0x10000));
    }

exit:
  SOC_FUNC_RETURN;
}



/* following macros used to replace usage of wrong macros in disable_ecc_disable_dynamic_mechanism() */
#define DISABLE_ECC_CHECK_FUNC_RESULT(res, err_num,op, err_val) \
    res = op; \
    if (res != SOC_E_NONE) { \
        res = err_val; \
        exit_place = err_num; \
        soc_sand_set_error_code_into_error_word(res, &rv); \
        LOG_ERROR(BSL_LS_APPL_COMMON, \
                  (BSL_META("error in function disable_ecc_dynamic_mechanism()\n"))); \
        goto exit;  \
    }
    

#define DISABLE_ECC_SOC_SAND_IF_ERROR_RETURN(res, err_num,op) \
    DISABLE_ECC_CHECK_FUNC_RESULT(res,err_num,op,SOC_SAND_SOC_ERR)



#define DISABLE_ECC_ARAD_FIELD_SET(reg_desc, fld_desc, fld_value, err_num) \
  DISABLE_ECC_CHECK_FUNC_RESULT(res,err_num,soc_reg_above_64_field32_modify(unit,reg_desc,REG_PORT_ANY,0,fld_desc,fld_value) ,ARAD_REG_ACCESS_ERR);




/* set a field of a register array element */
#define DISABLE_ECC_ARAD_FIELD_ASET(reg_desc, index,fld_desc, fld_value, err_num) \
    DISABLE_ECC_CHECK_FUNC_RESULT(res,err_num,soc_reg_above_64_field32_modify(unit,reg_desc,REG_PORT_ANY,index,fld_desc,fld_value) ,ARAD_REG_ACCESS_ERR);

/*
#define WRITE_BIT_DISABLE_DYNAMIC(unit,reg,field) \
  {                                             \
      write_val = 0;                            \
      READ_##reg (unit,&write_val);             \
      soc_reg_field_set(unit, reg , &write_val, (field),1);    \
      WRITE_##reg (unit,write_val);                         \
   }
*/
/*
 * Enables dynamic memory access 
 */
int 
disable_ecc_disable_dynamic_mechanism(int unit)
{
#ifdef BCM_PETRA_SUPPORT
  uint32 res;
  uint32 write_val = 1, rv =0, exit_place =0;

    soc_sand_initialize_error_word(0,0,&rv);

  DISABLE_ECC_SOC_SAND_IF_ERROR_RETURN(res, 10,soc_reg_iterate(unit, enable_dynamic_memories_access_cb, &write_val)) ;

  if (!SOC_IS_ARDON(unit)) {
      DISABLE_ECC_ARAD_FIELD_SET(SCH_SCHEDULER_CONFIGURATION_REGISTERr, DISABLE_FABRIC_MSGSf, 1 ,580);
  }

  if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      if (!SOC_IS_ARDON(unit)) {
          DISABLE_ECC_ARAD_FIELD_SET(NBI_STATISTICSr, STAT_USE_GLOBAL_INDIRECTf, 1, 110); 

          DISABLE_ECC_ARAD_FIELD_SET(IDR_GLBL_CFGr, MCDA_INITf, 1 ,540);
          DISABLE_ECC_ARAD_FIELD_SET(IDR_GLBL_CFGr, MCDB_INITf, 1 ,550);
          DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_0102r, FIELD_6_6f, 1 ,600);
          DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_0102r, FIELD_5_5f, 1 ,610);
          DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_0102r, FIELD_4_4f, 1 ,620);
          DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_0102r, FIELD_3_3f, 1 ,630);
          DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_0102r, FIELD_2_2f, 1 ,640);
          DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_0102r, FIELD_1_1f, 1 ,650);
          DISABLE_ECC_ARAD_FIELD_SET(SCH_SYSTEM_RED_CONFIGURATIONr, AGING_TIMER_CFGf, 0 ,560);
          DISABLE_ECC_ARAD_FIELD_SET(SCH_DVS_CONFIG_0r, NIF_FORCE_PAUSEf, 0xffffffff ,570);
      }
      DISABLE_ECC_ARAD_FIELD_SET(CFC_REG_00CFr,FIELD_16_16f, 1 ,120);
      DISABLE_ECC_ARAD_FIELD_SET(CRPS_REG_00CEr,FIELD_16_16f, 1 ,130);
      DISABLE_ECC_ARAD_FIELD_SET(ECI_GLOBALFr,CPU_BYPASS_ECC_PARf, 1 ,140);
      DISABLE_ECC_ARAD_FIELD_SET(EGQ_REG_00CFr,FIELD_16_16f, 1 ,150);
      DISABLE_ECC_ARAD_FIELD_SET(EPNI_REG_00CFr,FIELD_16_16f, 1 ,160);
      DISABLE_ECC_ARAD_FIELD_SET(FCR_REG_00CFr,FIELD_16_16f, 1 ,170);
      DISABLE_ECC_ARAD_FIELD_SET(FDT_REG_00CFr,FIELD_16_16f, 1 ,180);
      DISABLE_ECC_ARAD_FIELD_SET(IDR_REG_00CFr,FIELD_16_16f, 1 ,190);
    /* WRITE_BIT_DISABLE_DYNAMIC(unit , IDR_REG_00CFr , FIELD_16_16f);*/
      DISABLE_ECC_ARAD_FIELD_SET(IHB_REG_008Ar,FIELD_16_16f, 1 ,200);
      DISABLE_ECC_ARAD_FIELD_SET(IHP_REG_00CFr,FIELD_16_16f, 1 ,210);
      DISABLE_ECC_ARAD_FIELD_SET(IPS_REG_00C6r,FIELD_16_16f, 1 ,220);
      DISABLE_ECC_ARAD_FIELD_SET(IPT_REG_00CFr,FIELD_16_16f, 1 ,230);
      DISABLE_ECC_ARAD_FIELD_SET(IQM_REG_00CDr,FIELD_16_16f, 1 ,240);
      DISABLE_ECC_ARAD_FIELD_SET(IRE_REG_00CFr,FIELD_16_16f, 1 ,250);
      DISABLE_ECC_ARAD_FIELD_SET(IRR_REG_00CFr,FIELD_16_16f, 1 ,260);
      DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_00CFr,FIELD_16_16f, 1 ,270);
      DISABLE_ECC_ARAD_FIELD_SET(NBI_REG_00CFr,FIELD_16_16f, 1 ,280);
      if(SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
          if (!SOC_IS_ARDON(unit)) {
              DISABLE_ECC_ARAD_FIELD_SET(OAMP_GLOBAL_Fr, CPU_BYPASS_ECC_PARf, 1 ,290);
              DISABLE_ECC_ARAD_FIELD_SET(OLP_OLP_GLOBAL_REGISTERS_0r, FIELD_16_16f, 1 ,300);
          }
       } else {
        DISABLE_ECC_ARAD_FIELD_SET(OAMP_REG_00CFr,FIELD_16_16f, 1 ,290);
        DISABLE_ECC_ARAD_FIELD_SET(OLP_REG_00F0r,FIELD_16_16f, 1 ,300);
      }
      DISABLE_ECC_ARAD_FIELD_SET(CFC_REG_00CFr,FIELD_15_15f, 1 ,310);
      DISABLE_ECC_ARAD_FIELD_SET(CRPS_REG_00CEr,FIELD_15_15f, 1 ,320);
      DISABLE_ECC_ARAD_FIELD_SET(ECI_GLOBALFr, DIS_ECCf, 1 ,330);
      DISABLE_ECC_ARAD_FIELD_SET(EGQ_REG_00CFr, FIELD_15_15f, 1 ,340);
      DISABLE_ECC_ARAD_FIELD_SET(EPNI_REG_00CFr, FIELD_15_15f, 1 ,350);
      DISABLE_ECC_ARAD_FIELD_SET(FCR_REG_00CFr, FIELD_15_15f, 1 ,360);
      DISABLE_ECC_ARAD_FIELD_SET(FDT_REG_00CFr, FIELD_15_15f, 1 ,370);
      DISABLE_ECC_ARAD_FIELD_SET(IDR_REG_00CFr, FIELD_15_15f, 1 ,380);
      DISABLE_ECC_ARAD_FIELD_SET(IHB_REG_008Ar, FIELD_15_15f, 1 ,390);
      DISABLE_ECC_ARAD_FIELD_SET(IHP_REG_00CFr, FIELD_15_15f, 1 ,400);
      DISABLE_ECC_ARAD_FIELD_SET(IPS_REG_00C6r, FIELD_15_15f, 1 ,410);
      DISABLE_ECC_ARAD_FIELD_SET(IPT_REG_00CFr, FIELD_15_15f, 1 ,420);
      DISABLE_ECC_ARAD_FIELD_SET(IQM_REG_00CDr, FIELD_15_15f, 1 ,430);
      DISABLE_ECC_ARAD_FIELD_SET(IRE_REG_00CFr, FIELD_15_15f, 1 ,440);
      DISABLE_ECC_ARAD_FIELD_SET(IRR_REG_00CFr, FIELD_15_15f, 1 ,450);
      DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_00CFr, FIELD_15_15f, 1 ,460);
      DISABLE_ECC_ARAD_FIELD_SET(NBI_REG_00CFr, FIELD_15_15f, 1 ,470);
      if(SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
          if (!SOC_IS_ARDON(unit)) {
              DISABLE_ECC_ARAD_FIELD_SET(OAMP_GLOBAL_Fr, DIS_ECCf, 1, 290); 
              DISABLE_ECC_ARAD_FIELD_SET(OLP_OLP_GLOBAL_REGISTERS_0r, FIELD_15_15f, 1 ,300);
          }
      } else {
        DISABLE_ECC_ARAD_FIELD_SET(OAMP_REG_00CFr, FIELD_15_15f, 1 ,480);   
        DISABLE_ECC_ARAD_FIELD_SET(OLP_REG_00F0r, FIELD_15_15f, 1 ,490);
      }
      DISABLE_ECC_ARAD_FIELD_SET(IPS_IPS_GENERAL_CONFIGURATIONSr, DISCARD_ALL_IQM_MSGf, 1 ,500);
      DISABLE_ECC_ARAD_FIELD_SET(IPS_IPS_GENERAL_CONFIGURATIONSr, DISCARD_ALL_CRDTf, 1 ,510);
      DISABLE_ECC_ARAD_FIELD_SET(IPS_IPS_GENERAL_CONFIGURATIONSr, DISABLE_STATUS_MSG_GENf, 1 ,520);
      DISABLE_ECC_ARAD_FIELD_SET(IPS_CREDIT_WATCHDOG_CONFIGURATIONr, CR_WD_MIN_SCAN_CYCLE_PERIODf, 0 ,530);
      DISABLE_ECC_ARAD_FIELD_SET(MMU_REG_00C1r, FIELD_2_2f, 0 ,590);
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_CNT_SRC_CFGr, 0, CRPS_N_CNT_SRCf, 0x12, 660 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_CNT_SRC_CFGr, 1, CRPS_N_CNT_SRCf, 0x12, 670 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_CNT_SRC_CFGr, 2, CRPS_N_CNT_SRCf, 0x12, 680 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_CNT_SRC_CFGr, 3, CRPS_N_CNT_SRCf, 0x12, 690 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 0, CRPS_N_CPU_CLEAR_READ_ENf, 0, 700 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 0, CRPS_N_BUBBLE_ENf, 0, 710 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 1, CRPS_N_CPU_CLEAR_READ_ENf, 0, 720 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 1, CRPS_N_BUBBLE_ENf, 0, 730 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 2, CRPS_N_CPU_CLEAR_READ_ENf, 0, 740 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 2, CRPS_N_BUBBLE_ENf, 0, 750 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 3, CRPS_N_CPU_CLEAR_READ_ENf, 0, 760 );
      DISABLE_ECC_ARAD_FIELD_ASET(CRPS_CRPS_GENERAL_CFGr, 3, CRPS_N_BUBBLE_ENf, 0, 770 );

      DISABLE_ECC_SOC_SAND_IF_ERROR_RETURN(res, 780, WRITE_CMIC_SBUS_TIMEOUTr(unit, 0x30000));
  } else {
      DISABLE_ECC_ARAD_FIELD_SET(SCH_DVS_CONFIGr, FORCE_PAUSEf, 1 ,570);
  }
exit:
    if (rv !=0) {
                soc_sand_error_handler(res, "error in disable_ecc_disable_dynamic_mechanism()", exit_place,0,0,0,0,0 );
    }
    return rv;
#else
  return 0;
#endif
}


/*
 * Memories read/write for first and last memory index test (tr 7)
 */
#if defined(BCM_PETRA_SUPPORT) ||  defined(BCM_DFE_SUPPORT)
char tr7_test_usage[] = 
"TR7 memory write & read (first & last) usage:\n"
" \n"
 #ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pedantic compiler\n";
 #else

  "EnableReset=<1/0>               -  Specifies if proceed reset (1, default) or not\n"
  "EnableSkip=<1/0>                -  Specifies if to skip registers cannot be compared (1, default) or do not skip(0)\n"
  "IncPm=<1/0>                -  Specifies if to include port macro blocks in tr (1-include 0-exclude(default))\n"
  "StartFrom=<integer>                -  Specifies from what mem id to start the tr(default 0)\n"
  "Help=<1/0>                      -  Specifies if tr 7 help is on and exit or off (off by default)\n"
  "To enable printing values (write/read) of each memory proceed: debug +test +verb\n"
  "\n";
  #endif

#endif

int
memories_rw_first_last_test(int unit, args_t *a, void *pa)
{

    int         rv = 0;
    int num_soc_mem = NUM_SOC_MEM;
#if defined(BCM_PETRA_SUPPORT) ||  defined(BCM_DFE_SUPPORT) || defined(BCM_DNXF_SUPPORT)
    parse_table_t    pt;
    uint32 help_status = 0;
    uint32 help_status_default = 0;
    uint32 enablereset = 1;
    uint32 default_enablereset = 1;
    tr7_dbase_t tr7_data;

    tr7_data.enable_skip = 1;
    tr7_data.show_compare = 0;
    tr7_data.include_port_macros = 0;
    tr7_data.start_from = 0;
    tr7_data.count = NUM_SOC_MEM;
#endif
    COMPILER_REFERENCE(pa);
#if defined(BCM_PETRA_SUPPORT) ||  defined(BCM_DFE_SUPPORT) || defined(BCM_DNXF_SUPPORT)
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "EnableReset",  PQ_DFL|PQ_INT, &default_enablereset, &enablereset, NULL); 
    parse_table_add(&pt, "EnableSkip", PQ_DFL|PQ_INT,  0, &tr7_data.enable_skip, NULL);
    parse_table_add(&pt, "IncPm", PQ_DFL|PQ_INT,  0, &tr7_data.include_port_macros, NULL);
    parse_table_add(&pt, "StartFrom", PQ_DFL|PQ_INT,  0, &tr7_data.start_from, NULL);
    parse_table_add(&pt, "MemCount", PQ_DFL|PQ_INT,  &num_soc_mem, &tr7_data.count, NULL);
    parse_table_add(&pt, "Help",    PQ_DFL|PQ_INT, &help_status_default, &help_status,           NULL);

    if (parse_arg_eq(a, &pt) < 0) {
        cli_out("%s: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        goto done;
    }
    parse_arg_eq_done(&pt);

    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "Memories read/write first and last test\n")));

    if (help_status == 1) {
        cli_out("%s\n",tr7_test_usage);
        goto done;
    }
#endif
           
    COMPILER_REFERENCE(pa);

    /*
     * If try_reg returns -1, there was a register access failure rather
     * than a bit error.
     *
     * If try_reg returns 0, then rd.error is -1 if there was a bit
     * error.
     */
#if defined(BCM_PETRA_SUPPORT) ||  defined(BCM_DFE_SUPPORT)
#if defined(BCM_PETRA_SUPPORT)
    if (SOC_IS_ARAD(unit)) {
        bcm_common_linkscan_enable_set(unit,0);
        soc_counter_stop(unit);
        if ((enablereset == 1) && !SOC_IS_JERICHO(unit)) {
            if ((rv = soc_dpp_device_reset(unit, SOC_DPP_RESET_MODE_REG_ACCESS,SOC_DPP_RESET_ACTION_INOUT_RESET)) < 0) {
                LOG_ERROR(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "ERROR: Unable to reinit unit %d \n"), unit));
                goto done;
            }
        } else {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "Skip device reset\n")));
        }

        rv = enable_dynamic_memories_access(unit);
        if(SOC_FAILURE(rv)) {
            goto done;
        }
    }

#endif
    rv = do_mem_test_rw(unit, tr7_data, a);

done:  
#endif
    if (rv < 0) {
        test_error(unit, "Register read/write test failed\n");
    }

    return rv;
}



#ifdef BCM_PETRA_SUPPORT
char tr8_test_usage[] = 
"TR8 memory write & periodic read usage:\n"
" \n"
  "RunCount=<value>                -  Specifies number of read all memory cycles after one write (0xffffffff is like while(1)), 0xffffffff by default\n"
  "Value=<value>                   -  Specifies write values (32 bits), 0 by default\n"
  "Period=<value>                  -  Specifies period of read cycle in seconds, 5 by default\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pedantic compiler\n";
#else
  "Help=<1/0>                      -  Specifies if tr 8 help is on and exit or off (off by default)\n"
  "File=<file name>                -  Specifies file name for dump (relevant when UseFile=1)\n"
  "UseFile=<1/0>                   -  Specifies if to use file for dump (1- use, 0 - do not use), 0 by default\n"
  "faultInject=<1/0>               -  Specifies if to activate fault inject(non correct write, test should show compare errors(1-enabled, 0-disabled), 0 by default\n"
  "SkipResetAndWrite=<1/0>         -  Specifies if to skip chip reset and memories write(1-skip, 0-do not skip), 0 by default\n"
  "IncPm=<1/0>                     -  Specifies if to include port macro blocks in tr (1-include 0-exclude(default))\n"
  "StartFrom=<integer>             -  Specifies from what mem id to start the tr(default 0)\n"
  "MemCount=<integer>             -   Specifies how many memories to test)\n"
  "MaxSize=<integer|-1>            -  table with size in bytes above MaxSize will excluded from the test(default -1 all tables included)\n"
  "\n"
;
#endif

/*
 * Memories flip flop test tr 8
 */
int
mem_flipflop_test(int unit, args_t *a, void *pa)
{

   int         rv = 0;
   parse_table_t    pt;
   tr8_dbase_t tr8_data;
   uint32 default_val = 5;
   uint32 default_run_count = 0x1;
   uint32 help_status;
   uint32 help_status_default = 0;
   int xd_usefile = 0;
   char *file_name=NULL;
   int default_include_port_macros = 0;
   int default_max_size=-1;
   char print_str[250];
   bcm_switch_event_control_t bcm_switch_event_control;
#ifndef __KERNEL__
   time_t current_time;
#endif
   int default_skip_reset_write = 0;
   tr8_data.write_value = 0;
   tr8_data.period = 5;
   tr8_data.run_count = 0x1;
   tr8_data.skip_reset_and_write = 0;
   tr8_data.include_port_macros = 0;
   tr8_data.start_from = 0;
   tr8_data.max_size = -1;
   tr8_data.total_count=0;
   tr8_data.error_count=0;
   tr8_data.unclear_skipped_count=0;
   tr8_data.count=NUM_SOC_MEM;
   help_status = 0;
  
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "RunCount",         PQ_DFL|PQ_INT,    &default_run_count,         &tr8_data.run_count,            NULL); 
        parse_table_add(&pt, "Value",            PQ_DFL|PQ_INT,    0,                          &tr8_data.write_value,          NULL);
        parse_table_add(&pt, "Period",           PQ_DFL|PQ_INT,    &default_val,               &tr8_data.period,               NULL);
        parse_table_add(&pt, "Help",             PQ_DFL|PQ_INT,    &help_status_default,       &help_status,                   NULL);
        parse_table_add(&pt, "UseFile",          PQ_DFL|PQ_BOOL ,  0,                          &xd_usefile,                    NULL);
        parse_table_add(&pt, "File",             PQ_STRING,        0,                          &file_name,                     NULL);
        parse_table_add(&pt, "faultInject",      PQ_DFL|PQ_INT,    0,      &tr8_data.fault_inject,         NULL); 
        parse_table_add(&pt, "SkipResetAndWrite",PQ_DFL|PQ_INT,    &default_skip_reset_write,  &tr8_data.skip_reset_and_write, NULL); 
        parse_table_add(&pt, "IncPm",            PQ_DFL|PQ_INT,    &default_include_port_macros,                          &tr8_data.start_from,           NULL); 
        parse_table_add(&pt, "StartFrom",        PQ_DFL|PQ_INT,    0,                          &tr8_data.start_from,           NULL); 
        parse_table_add(&pt, "MemsCount",        PQ_DFL|PQ_INT,    0,                          &tr8_data.count,           NULL); 
        parse_table_add(&pt, "MaxSize",          PQ_DFL|PQ_INT,    &default_max_size,                         &tr8_data.max_size,             NULL); 

        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid option: %s\n",
                    ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            goto done;
        }
        if (xd_usefile > 0) {
            /*  = sal_strdup(xd_file); */
            if (tr8_open_dump_file(unit,file_name ) == -1) {
                cli_out("Error open dump file %s, dump file disabled\n", file_name);
            }
        } 

        parse_arg_eq_done(&pt);
#ifndef __KERNEL__
        time(&current_time);
        
        /* COMPILER_REFERENCE(pa); */
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "Memories read/write first and last test, %s\n"),ctime((const time_t *)&current_time)));

        sal_sprintf(print_str,"Memories read/write first and last test, %s\n",ctime((const time_t *)&current_time)); 
#endif
        tr8_write_dump(print_str);

        
        sal_sprintf(print_str,"write pattern =0x%x, period =%d, total iterations =%d, skip reset & write= %d\n",
               tr8_data.write_value, 
               tr8_data.period,
               tr8_data.run_count,
               tr8_data.skip_reset_and_write); 
        cli_out(print_str);
        tr8_write_dump(print_str);
    
        if (help_status == 1) {
            cli_out("%s\n",tr8_test_usage);
            goto done;
        }
        
    
        /*
         * If try_reg returns -1, there was a register access failure rather
         * than a bit error.
         *
         * If try_reg returns 0, then rd.error is -1 if there was a bit
         * error.
         */
        /*    soc_memories_bcm88650_a0 */
        if (SOC_IS_ARAD(unit))
        {
            if (tr8_data.skip_reset_and_write  != 1) {     
              bcm_common_linkscan_enable_set(unit,0);
              soc_counter_stop(unit);
              /* Disable all interrupts*/
              bcm_switch_event_control.action = bcmSwitchEventMask;
              bcm_switch_event_control.event_id = BCM_SWITCH_EVENT_CONTROL_ALL;
              if ((rv = bcm_switch_event_control_set(unit, BCM_SWITCH_EVENT_DEVICE_INTERRUPT, bcm_switch_event_control, 0x1)) < 0) {
                  LOG_ERROR(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "ERROR: Unable to Disable interrupt on unit %d \n"), unit));
                  goto done;
              }         
              /* if (BCM_E_NONE == bcm_dpp_counter_bg_enable_set(unit, FALSE)) {
                  cli_out("unit %d counter processor background accesses suspended\n", unit);
              } else {
                  cli_out("unit %d counter processor background access suspend failed: %d (%s)\n",
                          unit,
                          result,
                          _SHR_ERRMSG(result));
              } */

              if ((rv = soc_dpp_device_reset(unit, SOC_DPP_RESET_MODE_REG_ACCESS,SOC_DPP_RESET_ACTION_INOUT_RESET)) < 0) {
                  LOG_ERROR(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "ERROR: Unable to reinit unit %d \n"), unit));
                  goto done;
              }
              disable_ecc_disable_dynamic_mechanism(unit);

              rv = enable_dynamic_memories_access(unit); 
              if(SOC_FAILURE(rv)){
                  return rv;
              } 
          }

          rv = do_mem_test_flipflop(unit, tr8_data, a);
          tr8_close_dump_file();
        }
 
done:
        if (rv < 0) {
            test_error(unit, "Register read/write test failed\n");
        }
        return rv;
}

#ifndef __KERNEL__
static FILE * pFileTr8;
#endif

int8 tr8_open_dump_file(int unit, char *fname)
{

#ifndef __KERNEL__
  pFileTr8 = sal_fopen(fname,"wt");
  if (pFileTr8==NULL) {
      return -1;
  }
#endif /* __KERNEL__ */
  return 0;
}

void tr8_close_dump_file(void)
{
#ifndef __KERNEL__
    if (pFileTr8 != NULL) {
        fclose (pFileTr8);
    }
#endif
}

void tr8_write_dump(const char * _Format)
{
#ifndef __KERNEL__
    if (pFileTr8 != NULL) {
        fprintf(pFileTr8,_Format);
    }
#endif
}


static uint32 total_processed_bits;
void tr8_reset_bits_counter(void)
{
    total_processed_bits = 0;
}

void tr8_increment_bits_counter(uint32 bits_num)
{
    total_processed_bits += bits_num;
}

uint32 tr8_bits_counter_get(void)
{
    return total_processed_bits;
}
uint32 tr8_get_bits_num(uint32 number)
{
  uint32 counter = 0;
  uint32 i;
  for (i=0;i<32;i++) {
      if ((number & (1<<i)) != 0) {
          counter++;
      }
  }
  return counter;
}

#endif

#endif /* defined (BCM_PETRA_SUPPORT)|| defined (BCM_DFE_SUPPORT) */

#if defined (SER_TR_TEST_SUPPORT)
typedef struct ser_test_s {
    _soc_ser_test_t test_type;
    int             verbose_prints;
    int             error_prints;
    int             cmd_print;
    int             inject_only;
    soc_mem_t       mem;
} ser_test_param_t;

static ser_test_param_t *ser_test_parameters[SOC_MAX_NUM_DEVICES];

int ser_test_init(int unit, args_t *arg, void **p) {
    ser_test_param_t    *ser_params;
    parse_table_t        pt;
    soc_block_t          copyno;
    char                *test_type_name;
    char                *mem_name;
    if(ser_test_parameters[unit] == NULL) {
        ser_test_parameters[unit] = sal_alloc(sizeof(ser_test_param_t), "ser_test");
        if (ser_test_parameters[unit] == NULL) {
            test_error(unit, "ERROR: cannot allocate memory\n");
            return -1;
        }
        sal_memset(ser_test_parameters[unit],0,sizeof(ser_test_param_t));
    }
    ser_params = ser_test_parameters[unit];
    parse_table_init(unit, &pt);
    parse_table_add(&pt,  "Memory",    PQ_STRING, "all",
            &(mem_name), NULL);
    parse_table_add(&pt,  "Cmd",    PQ_BOOL, FALSE,
            &(ser_params->cmd_print), NULL);
    parse_table_add(&pt,  "Error",    PQ_BOOL, FALSE,
            &(ser_params->error_prints), NULL);
    parse_table_add(&pt,  "Verbose",    PQ_BOOL, FALSE,
            &(ser_params->verbose_prints), NULL);
    parse_table_add(&pt,  "testType",    PQ_STRING, "single",
            &(test_type_name), NULL);
    parse_table_add(&pt, "injectOnly", PQ_BOOL, FALSE,
            &(ser_params->inject_only),NULL);
    if (parse_arg_eq(arg, &pt) < 0) {
        cli_out("%s: Invalid option: %s\n",
                ARG_CMD(arg), ARG_CUR(arg));
        parse_arg_eq_done(&pt);
        return -1;
    }

    if (!sal_strcasecmp(test_type_name, "single")) {
        ser_params->test_type = SER_SINGLE_INDEX;
    } else if (!sal_strcasecmp(test_type_name, "spread")) {
        ser_params->test_type = SER_FIRST_MID_LAST_INDEX;
    } else if (!sal_strcasecmp(test_type_name, "all")) {
        ser_params->test_type = SER_ALL_INDEXES;
    } else {
        cli_out("Invalid test type selected.\n");
        parse_arg_eq_done(&pt);
        return -1;
    }

    if (!sal_strcasecmp(mem_name, "all")) {
        ser_params->mem = INVALIDm;
    } else if (!(parse_memory_name(unit,&(ser_params->mem),mem_name,&copyno,0) >=0)) {
        cli_out("Invalid memory selected.\n");
        parse_arg_eq_done(&pt);
        return -1;
    }
    *p = ser_params;
    parse_arg_eq_done(&pt);
    return 0;
}

int ser_test_done(int unit, void *p) 
{
    return 0;
}

/*
 * Function:
 *      ser_test
 * Purpose:
 *      Entry point and argument parser for SER TR tests.
 * Parameters:
 *      unit    - (IN) Device Number
 *      arg     - (IN) A set of passed arguments to this TR test
 *      p       - (IN) Unused. *Required for TR test format.
 */
int ser_test(int unit, args_t *arg, void *p) {
    ser_test_param_t *test_params = p;
    int rv = SOC_E_NONE;
    int force_read_through = SOC_MEM_FORCE_READ_THROUGH(unit);
#ifdef INCLUDE_MEM_SCAN
    sal_usecs_t mem_scan_interval = 0;
    int mem_scan_rate = 0, mem_scan_running;
#endif
#ifdef BCM_SRAM_SCAN_SUPPORT
    sal_usecs_t sram_scan_interval = 0;
    int sram_scan_rate = 0, sram_scan_running;
#endif
    
    
    
    if (!force_read_through) {
       SOC_MEM_FORCE_READ_THROUGH_SET(unit, 1);
    }
    /*init soc*/
    rv = soc_reset_init(unit);
    if (rv != SOC_E_NONE) {
        cli_out("Error during soc reset.  Aborting.\n");
        return rv;
    }
    /*init misc*/
    rv = soc_misc_init(unit);
    if (rv != SOC_E_NONE) {
        cli_out("Error during misc init.  Aborting.\n");
        return rv;
    }
#if defined(BCM_SABER2_SUPPORT) || defined(BCM_KATANA2_SUPPORT)
    if (SOC_IS_SABER2(unit) || SOC_IS_KATANA2(unit)) { 
        /*init mmu */
        rv = soc_mmu_init(unit);
        if (rv != SOC_E_NONE) {
            cli_out("Error during mmu init.  Aborting.\n");
            return rv;
        }
    }
#endif
    
#ifdef INCLUDE_MEM_SCAN
    mem_scan_running = soc_mem_scan_running (unit, &mem_scan_rate,
                                             &mem_scan_interval);
    if( mem_scan_running > 0){
        if( soc_mem_scan_stop(unit) ) {
            return -1;
        }
    }
#endif
#ifdef BCM_SRAM_SCAN_SUPPORT
    sram_scan_running = soc_sram_scan_running (unit, &sram_scan_rate,
                                               &sram_scan_interval);
    if (sram_scan_running > 0){
        if (soc_sram_scan_stop(unit)) {
            return -1;
        }
    }
#endif

    if (test_params->verbose_prints) {
        
    }
    if (test_params->error_prints) {
        
    }

    /*Test individual memories and inject individual errors.*/
    if (test_params->mem != INVALIDm) {
        if (test_params->inject_only == TRUE) {
            soc_ser_inject_error(unit, FALSE, test_params->mem, 0, -1, 0);
        } else {
            rv = soc_ser_test_mem(unit, test_params->mem, 
                                  test_params->test_type, 
                                  test_params->cmd_print);
            if (SOC_FAILURE(rv)) {
                test_error(unit,
                           "Error during ser test: %s\n",
                           soc_errmsg(rv));
            }
        }
    } else {
        if (test_params->inject_only == TRUE) {
            cli_out("Parameter 'injectOnly' may only be used "
                    "when 'memory' is also specified.\n");
        } else {
            rv = soc_ser_test(unit, test_params->test_type);
            if (SOC_FAILURE(rv)) {
                test_error(unit,
                           "Error during ser test: %s\n",
                           soc_errmsg(rv));
            }
        }
    }
    
    if (!force_read_through) {
        SOC_MEM_FORCE_READ_THROUGH_SET(unit, 0);
    }
    rv = soc_reset_init(unit);
    if (SOC_FAILURE(rv)) {
        cli_out("Error during soc reset.  Aborting.\n");
        return rv;
    }
    /*init misc*/
    rv = soc_misc_init(unit);
    if (SOC_FAILURE(rv)) {
        cli_out("Error during misc init.  Aborting.\n");
        return rv;
    }

#if defined(BCM_SABER2_SUPPORT)
    if (SOC_IS_SABER2(unit)) { 
        /*init mmu */
        rv = soc_mmu_init(unit);
        if (rv != SOC_E_NONE) {
            cli_out("Error during mmu init.  Aborting.\n");
            return rv;
        }
    }
#endif
    
#ifdef INCLUDE_MEM_SCAN
    if (mem_scan_running) {
        if(soc_mem_scan_start(unit, mem_scan_rate, mem_scan_interval)){
            return -1;
        }
    }
#endif
#ifdef BCM_SRAM_SCAN_SUPPORT
    if (sram_scan_running) {
        if(soc_sram_scan_start(unit, sram_scan_rate, sram_scan_interval)){
            return -1;
        }
    }
#endif
    return rv;
}
#endif /* defined (SER_TR_TEST_SUPPORT)*/

#endif /* defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_PETRA_SUPPORT) || defined (BCM_DFE_SUPPORT)  || defined (BCM_POLAR_SUPPORT) */

#undef _ERR_MSG_MODULE_NAME
