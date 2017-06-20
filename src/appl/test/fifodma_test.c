/*
 * $Id: fifodma_test.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FIFO DMA Test
 *
 * The FIFO DMA test fills up the L2_MOD_FIFO with random values through SCHAN.
 * Then, it reads the L2_MOD_FIFO through FIFO DMA to an external memory. Data
 * integrity check is done by comparing the source and destination memories.
 * FIFO DMA read performance in bps is calculated.
 */

#include <shared/bsl.h>
#include <sal/appl/sal.h>
#include <sal/core/time.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <soc/debug.h>
#include <soc/dma.h>
#include <soc/l2x.h>
#include "testlist.h"
#if defined(BCM_CMICM_SUPPORT) || defined(BCM_CMICDV2_SUPPORT)
#include <soc/cmicm.h>
#endif

#define FIFODMA_PERFORMANCE_TEST 1
#define FIFODMA_TEST_TYPE_PARAM 1
#define FIFODMA_RATE_CALC_INTERVAL_PARAM 10
#define FIFODMA_CMC_PARAM 0
#define FIFODMA_HOSTMEM_THRESHOLD_PARAM 20
#define NUM_ENTRIES_READ_SW_PARAM 5

#if defined(BCM_ESW_SUPPORT) && defined(BCM_CMICM_SUPPORT)

typedef struct fdt_data_s {
    unsigned int fifo_depth;
    unsigned int fifo_width;
    unsigned int test_type;
    unsigned int rate_calc_interval;
    unsigned int vchan;
    unsigned int hostmem_threshold;
    unsigned int num_entries;
    unsigned int use_api;
    unsigned int stop_perf_test;
    sal_usecs_t td_acc;

    unsigned int* host_base;
    volatile int total_fifo_drains;
    uint32 test_fail;
} fd_test_data_t;

static fd_test_data_t fd_test_data;

char fd_test_usage[] =
    "FIFO DMA test usage:\n"
    " \n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pedantic compiler\n";
#else
    "VChan=<int>      -  Virtual CMC Channel; Only 0, 4, and 8 are valid for L2MOD\n"
    "                    default is 0\n"
    "HostMemTh=<int>  -  FIFO DMA host memory threshold; default is 65\n\n"
    "RateCalcIn=<int> -  Duration for rate calculation; default is 10 s\n"
    "NumEntRdSw=<int> -  Number of entries per FIFO DMA; default is 64\n"
    "TestType=<1/0>   -  0: feature test; 1: performance test; default is 1\n"
    "UseAPI=<1/0>     -  Switch between SOC APIs and manual coding; default is 1\n"
    ;
#endif


static void
parse_fifodma_test_params(int unit, args_t* a)
{
    parse_table_t parse_table_fifodma_test;

    parse_table_init(unit, &parse_table_fifodma_test);
    parse_table_add(&parse_table_fifodma_test, "VChan",       PQ_INT|PQ_DFL, 0,
                    &(fd_test_data.vchan),                    NULL);
    parse_table_add(&parse_table_fifodma_test, "HostMemTh",   PQ_INT|PQ_DFL, 0,
                    &(fd_test_data.hostmem_threshold),        NULL);
    parse_table_add(&parse_table_fifodma_test, "RateCalcInt", PQ_INT|PQ_DFL, 0,
                    &(fd_test_data.rate_calc_interval),       NULL);
    parse_table_add(&parse_table_fifodma_test, "NumEntRdSw",  PQ_INT|PQ_DFL, 0,
                    &(fd_test_data.num_entries),              NULL);
    parse_table_add(&parse_table_fifodma_test, "TestType",    PQ_INT|PQ_DFL, 0,
                    &(fd_test_data.test_type),                NULL);
    parse_table_add(&parse_table_fifodma_test, "UseAPI",      PQ_BOOL|PQ_DFL, 0,
                    &(fd_test_data.use_api),                  NULL);

    if (parse_arg_eq(a, &parse_table_fifodma_test) < 0 || ARG_CNT(a) != 0) {
        test_error(unit,
                   "%s: Invalid option: %s\n", ARG_CMD(a),
                   ARG_CUR(a) ? ARG_CUR(a) : "*");
        fd_test_data.test_fail = 1;
        test_msg(fd_test_usage);
        parse_arg_eq_done(&parse_table_fifodma_test);
    }

    if (fd_test_data.vchan >= SOC_DCHAN_NUM(unit)) {
        fd_test_data.vchan = FIFODMA_CMC_PARAM;
    }
    cli_out("Testing CMC %0d channel %0d\n", fd_test_data.vchan / N_DMA_CHAN,
            fd_test_data.vchan % N_DMA_CHAN);

    if (fd_test_data.use_api) {
        /* round up the fd_test_data.num_entries to the nearest value
          supported by the _soc_mem_sbus_fifo_dma_start_memreg API */
        if (fd_test_data.num_entries < 64          ||
            fd_test_data.fifo_depth == 128) {
            fd_test_data.num_entries = 64;
        } else if (fd_test_data.num_entries < 128  ||
                   fd_test_data.fifo_depth == 256) {
            fd_test_data.num_entries = 128;
        } else if (fd_test_data.num_entries < 256  ||
                   fd_test_data.fifo_depth == 512) {
            fd_test_data.num_entries = 256;
        } else if (fd_test_data.num_entries < 512  ||
                   fd_test_data.fifo_depth == 1024) {
            fd_test_data.num_entries = 512;
        } else if (fd_test_data.num_entries < 1024 ||
                   fd_test_data.fifo_depth == 2048) {
            fd_test_data.num_entries = 1024;
        } else if (fd_test_data.num_entries < 2048 ||
                   fd_test_data.fifo_depth == 4096) {
            fd_test_data.num_entries = 2048;
        } else if (fd_test_data.num_entries < 4096 ||
                   fd_test_data.fifo_depth == 8192) {
            fd_test_data.num_entries = 4096;
        } else if (fd_test_data.num_entries < 8192 ||
                   fd_test_data.fifo_depth == 16384) {
            fd_test_data.num_entries = 8192;
        } else {
            fd_test_data.num_entries = 16384;
        }
    }
    /* fd_test_data.hostmem_threshold must be greater than fd_test_data.num_entries */
    if (fd_test_data.hostmem_threshold < fd_test_data.num_entries) {
        fd_test_data.hostmem_threshold = fd_test_data.num_entries + 1;
    }

    cli_out("\n ------------- PRINTING TEST PARAMS ------------------");
    cli_out("\nVChan       = %0d", fd_test_data.vchan);
    cli_out("\nHostMemTh   = %0d", fd_test_data.hostmem_threshold);
    cli_out("\nRateCalcInt = %0d", fd_test_data.rate_calc_interval);
    cli_out("\nNumEntRdSw  = %0d", fd_test_data.num_entries);
    cli_out("\nTestType    = %0d", fd_test_data.test_type);
    cli_out("\nUseAPI      = %0d", fd_test_data.use_api);
    cli_out("\n -----------------------------------------------------");
}


static unsigned int
custom_power(unsigned int base, unsigned int exponent)
{
    unsigned int result = 1;
    unsigned int i;
    for (i = 0; i < exponent; i++) {
        result *= base;
    }
    return(result);
}


static unsigned int
check_l2_mod_fifo_empty(int unit)
{
    uint64 rd_ptr, wr_ptr;
    unsigned int empty = 0x0;
    (void) soc_reg_get(unit, L2_MOD_FIFO_WR_PTRr, 0, 0, (&wr_ptr));
    (void) soc_reg_get(unit, L2_MOD_FIFO_RD_PTRr, 0, 0, (&rd_ptr));
    if (COMPILER_64_EQ(wr_ptr, rd_ptr)) {
        empty = 0x1;
    }

    return(empty);
}


static void
kick_off_fifo_dma_l2_mod_fifo(int unit, unsigned int* hostmem_start_addr,
                              unsigned int hostmem_num_entries_sel,
                              unsigned int hostmem_threshold,
                              unsigned int vchan)
{
    int cmc = vchan / N_DMA_CHAN;
    int ch  = vchan % N_DMA_CHAN;
    int copyno, big_pio, big_packet, big_other;
    uint8 at;
    unsigned int rdata = 0;
    unsigned int fifo_addr;
    schan_msg_t msg;

    soc_endian_get(unit, &big_pio, &big_packet, &big_other);
    copyno = SOC_MEM_BLOCK_ANY(unit, L2_MOD_FIFOm);
    fifo_addr = soc_mem_addr_get(unit, L2_MOD_FIFOm, 0, copyno, 0, &at);
    soc_schan_header_cmd_set(unit, &msg.header, FIFO_POP_CMD_MSG,
                             SOC_BLOCK2SCH(unit, copyno), 0,
                             SOC_MEM_ACC_TYPE(unit, L2_MOD_FIFOm), 4, 0, 0);
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, ENDIANESSf,
                      (unsigned int) big_other);
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata,
                      HOST_NUM_ENTRIES_SELf, hostmem_num_entries_sel);
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, BEAT_COUNTf,
                      fd_test_data.fifo_width);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch), rdata);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_OPCODE_OFFSET(cmc, ch),
                  msg.dwords[0]);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_SBUS_START_ADDRESS_OFFSET(cmc, ch),
                  fifo_addr);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_HOSTMEM_START_ADDRESS_OFFSET(cmc, ch),
                  soc_cm_l2p(unit, (unsigned int*)(hostmem_start_addr)));
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_HOSTMEM_THRESHOLD_OFFSET(cmc, ch),
                  hostmem_threshold);
    rdata = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch));
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, ENABLEf, 1);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch), rdata);
}


static void
wait_for_fifo_dma(int unit, unsigned int hostmem_num_entries_sel,
                  unsigned int hostmem_threshold, unsigned int vchan,
                  unsigned int break_on_empty_fifo)
{
    unsigned int stat, num_valid_entries;
    int cmc = vchan / N_DMA_CHAN;
    int ch  = vchan % N_DMA_CHAN;
    while(1) {
        stat = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_OFFSET(cmc, ch));
        num_valid_entries = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_NUM_OF_ENTRIES_VALID_IN_HOSTMEM_OFFSET(cmc, ch));
        if ((((stat >> 4) & 0x00000001) == 0x1) ||
            (num_valid_entries >= custom_power(2, (6 + hostmem_num_entries_sel))) ||
            (break_on_empty_fifo !=0 && check_l2_mod_fifo_empty(unit)!=0))
            break;
    }
}


static void
populate_l2_mod_fifo_sbus(int unit, unsigned int num_entries)
{
    uint64 wr_ptr, rd_ptr;
    unsigned int i, j;
    unsigned int *wr_data = NULL;
    uint64 zero_64;
    uint64 depth_64;
    uint64 max_entries;

    wr_data = (unsigned int *)soc_cm_salloc(unit,
                    fd_test_data.fifo_width, "fd_test_data");

    if(!wr_data) {
        cli_out("\nError in allocating memory:");
        return;
    }

    COMPILER_64_ZERO(zero_64);
    COMPILER_64_SET(depth_64, 0x0, fd_test_data.fifo_depth);

    (void) soc_reg_get(unit, L2_MOD_FIFO_WR_PTRr, 0, 0, (&wr_ptr));
    (void) soc_reg_get(unit, L2_MOD_FIFO_RD_PTRr, 0, 0, (&rd_ptr));

    COMPILER_64_SET(max_entries, COMPILER_64_HI(wr_ptr),
                    COMPILER_64_LO(wr_ptr));
    COMPILER_64_SUB_64(max_entries, rd_ptr);
    COMPILER_64_ADD_32(max_entries, num_entries);

    if (COMPILER_64_GT(max_entries,depth_64)) {
        (void) soc_reg_set(unit, L2_MOD_FIFO_WR_PTRr, 0, 0, zero_64);
        (void) soc_reg_set(unit, L2_MOD_FIFO_RD_PTRr, 0, 0, zero_64);
        COMPILER_64_ZERO(wr_ptr);
        COMPILER_64_ZERO(rd_ptr);
    } else {
        for (i = 0; i < num_entries; i++) {
            for (j = 0; j < fd_test_data.fifo_width; j++) {
                wr_data[j] = sal_rand();
            }
            (void) soc_mem_write(unit, L2_MOD_FIFOm, COPYNO_ALL, i, wr_data);
            COMPILER_64_ADD_32(wr_ptr, 0x1);
        }
        (void) soc_reg_set(unit, L2_MOD_FIFO_WR_PTRr, 0, 0, wr_ptr);
    }
    soc_cm_sfree(unit, (void *)wr_data);
}


static unsigned int
compare_entries(int unit, unsigned int* base_addr, unsigned int start_index,
                unsigned int num_entries)
{
    unsigned int i, j;
    unsigned int match = 1;
    unsigned int *rd_data;
    volatile unsigned int* exp_data;
    exp_data = base_addr;

    rd_data = (unsigned int *)soc_cm_salloc(unit,
                    fd_test_data.fifo_width, "fd_test_data");

    if(!rd_data) {
        cli_out("\nError in allocating memory:");
        return 0;
    }

    for (i = 0; i < num_entries; i++) {
        (void) soc_mem_read(unit, L2_MOD_FIFOm, COPYNO_ALL, start_index + i, rd_data);
        for (j = 0; j < fd_test_data.fifo_width; j++) {
            if (exp_data[fd_test_data.fifo_width * i + j] != rd_data[j]) {
                cli_out("\nIndex %0d mismatched: Exp: 0x%x, Act: 0x%x",
                        start_index + i,
                        exp_data[fd_test_data.fifo_width * i + j],
                        rd_data[j]);
                match = 0;
            }
        }
    }
    soc_cm_sfree(unit, (void *)rd_data);
    return(match);
}


static void
feature_test_fifo_dma(int unit)
{
    unsigned int match, rdata;
    unsigned int cmc = fd_test_data.vchan / N_DMA_CHAN;
    unsigned int ch  = fd_test_data.vchan % N_DMA_CHAN;

    cli_out("\nhost_base = %p", fd_test_data.host_base);

    cli_out("\nFilling up L2_MOD_FIFO through SBUS");
    populate_l2_mod_fifo_sbus(unit, fd_test_data.fifo_depth);
    if (fd_test_data.use_api) {
        (void) soc_mem_fifo_dma_start(unit, fd_test_data.vchan, L2_MOD_FIFOm,
                               COPYNO_ALL, fd_test_data.num_entries, fd_test_data.host_base);
    } else {
        kick_off_fifo_dma_l2_mod_fifo(unit, fd_test_data.host_base, 0x0,
                                      fd_test_data.hostmem_threshold,
                                      fd_test_data.vchan);
        wait_for_fifo_dma(unit, 0x0, fd_test_data.hostmem_threshold,
                          fd_test_data.vchan, 0x1);
    }

    rdata = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_OFFSET(cmc, ch));

    if (fd_test_data.hostmem_threshold <= 0x40) {
        if (((rdata >> 2) & 0x00000001) == 0x00000001) {
            cli_out("\nHostmem overflow set as expected");
        } else  {
            fd_test_data.test_fail = 1;
            test_error(unit, "Hostmem overflow not set even though number of words popped exceeds threshold");
        }
    }
    match = compare_entries(unit, fd_test_data.host_base, 0x0, 0x40);

    if (match == 1) {
        cli_out("\nFirst round of data integrity checks passed");
    } else {
        fd_test_data.test_fail = 1;
        test_error(unit, "First round of data integrity checks failed");
    }

    cli_out("\nMarking 1st %0d valid entries as having been read by SW",
            fd_test_data.num_entries);

    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEM_OFFSET(cmc, ch),
                  fd_test_data.num_entries);

    sal_usleep(1000000);

    match = compare_entries(unit, fd_test_data.host_base, 0x40,
                            fd_test_data.num_entries);

    if (match == 1) {
        cli_out("\nSecond round of data integrity checks passed");
    } else {
        fd_test_data.test_fail = 1;
        test_error(unit, "Second round of data integrity checks failed");
    }

    cli_out("\nMarking remaining valid entries as having been read by SW");
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEM_OFFSET(cmc, ch),
                  0x40);

    wait_for_fifo_dma(unit, 0x0, fd_test_data.hostmem_threshold,
                      fd_test_data.vchan, 0x1);
    match = compare_entries(unit, (fd_test_data.host_base +
                            (fd_test_data.num_entries * fd_test_data.fifo_width *
                             4)), (0x40 + fd_test_data.num_entries), (0x40 -
                             fd_test_data.num_entries));
    if (match == 1) {
        cli_out("\nFinal round of data integrity checks passed");
    } else {
        fd_test_data.test_fail = 1;
        test_error(unit, "Final round of data integrity checks failed");
    }

    cli_out("\nAborting FIFO DMA");
    rdata = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch));
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, ABORTf, 1);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch), rdata);
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, ABORTf, 0);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch), rdata);
    wait_for_fifo_dma(unit, 0x0, fd_test_data.hostmem_threshold,
                      fd_test_data.vchan, 0x1);

    rdata = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_STAT_OFFSET(cmc, ch));
    if ((((rdata >> 4) & 0x00000001) == 0x00000001)) {
        cli_out("\nDone bit set as expected after FIFO DMA abort");
    } else {
        fd_test_data.test_fail = 1;
        test_error(unit, "Done bit not set after FIFO DMA abort");
    }
}


static void
performance_test_fifo_dma(void* up)
{
    unsigned int rdata;
    uint64 wr_ptr;
    int unit;
    int cmc = fd_test_data.vchan / N_DMA_CHAN;
    int ch  = fd_test_data.vchan % N_DMA_CHAN;
    sal_usecs_t time0, time1;
    uint64 new_wr_ptr;
    unit = PTR_TO_INT(up);
    fd_test_data.td_acc = 0;

    populate_l2_mod_fifo_sbus(unit, fd_test_data.fifo_depth);
    kick_off_fifo_dma_l2_mod_fifo(unit, fd_test_data.host_base, 0x1,
                                  fd_test_data.hostmem_threshold,
                                  fd_test_data.vchan);
    time1 = sal_time_usecs();
    while (fd_test_data.stop_perf_test == 0)
    {
        wait_for_fifo_dma(unit, 0x1, fd_test_data.hostmem_threshold,
                          fd_test_data.vchan, 0x1);
        time0 = sal_time_usecs();
        if (fd_test_data.total_fifo_drains > 0) {
            fd_test_data.td_acc = SAL_USECS_ADD(fd_test_data.td_acc,
                                                SAL_USECS_SUB(time0, time1));
        }
        rdata = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_NUM_OF_ENTRIES_VALID_IN_HOSTMEM_OFFSET(cmc, ch));
        fd_test_data.total_fifo_drains++;
        if (rdata != fd_test_data.fifo_depth) {
            cli_out("\nERROR : Expected to pop all entries but valid entry %0d != fifo depth %0d", rdata, fd_test_data.fifo_depth);
            break;
        }
        soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEM_OFFSET(cmc, ch),
                      fd_test_data.fifo_depth);
        (void) soc_reg_get(unit, L2_MOD_FIFO_WR_PTRr, 0, 0, &wr_ptr);
        COMPILER_64_SET(new_wr_ptr, COMPILER_64_HI(wr_ptr),
                                    COMPILER_64_LO(wr_ptr));
        COMPILER_64_ADD_32(new_wr_ptr, fd_test_data.fifo_depth);
        (void) soc_reg_set(unit, L2_MOD_FIFO_WR_PTRr, 0, 0, new_wr_ptr);
        time1 = sal_time_usecs();
    }

    sal_thread_exit(0);
}


static void
fifodma_measure_rate(unsigned int interval_in_seconds)
{
    int cmc = fd_test_data.vchan / N_DMA_CHAN;
    int ch  = fd_test_data.vchan % N_DMA_CHAN;
    uint64 fifodma_total_fifo_drains_start;
    uint64 fifodma_total_fifo_drains_end;
    uint64 byte_transferred, rate_calc_int, transfer_rate;
    sal_usecs_t td_acc_start, td_acc_end, actual_drain_time;
    char byte_transferred_str[32], transfer_rate_str[32];

    COMPILER_64_SET(fifodma_total_fifo_drains_start, 0,
                    fd_test_data.total_fifo_drains);
    td_acc_start = fd_test_data.td_acc;

    cli_out("\nMeasuring data transfer rate over an interval of %0d seconds",
            interval_in_seconds);
    sal_sleep(interval_in_seconds);

    COMPILER_64_SET(fifodma_total_fifo_drains_end, 0,
                    fd_test_data.total_fifo_drains);
    td_acc_end = fd_test_data.td_acc;

    actual_drain_time = SAL_USECS_SUB(td_acc_end, td_acc_start);
    COMPILER_64_DELTA(byte_transferred, fifodma_total_fifo_drains_start,
                                        fifodma_total_fifo_drains_end);
    COMPILER_64_UMUL_32(byte_transferred, fd_test_data.fifo_depth);
    COMPILER_64_UMUL_32(byte_transferred, fd_test_data.fifo_width);
    COMPILER_64_SET(transfer_rate, COMPILER_64_HI(byte_transferred),
                                   COMPILER_64_LO(byte_transferred));
    COMPILER_64_SET(rate_calc_int, 0, actual_drain_time);
    COMPILER_64_UDIV_64(transfer_rate, rate_calc_int);
    format_uint64_decimal(byte_transferred_str, byte_transferred, 0);
    format_uint64_decimal(transfer_rate_str, transfer_rate, 0);
    cli_out("\n***FIFO DMA transfer rate on CMC %0d channel %0d = %s byte/s",
            cmc, ch, transfer_rate_str);
    cli_out("\n***Total byte transferred = %s", byte_transferred_str);
    cli_out("\n***Actual Draining time = %d us",
            actual_drain_time);
}


int fifodma_test_init(int unit, args_t *a, void **pa)
{
    int i;
    int cmc;
    uint32 addr_remap;

    cli_out("\nCalling fifodma_test_init\n");

    fd_test_data.test_fail = 0;

    if (soc_feature(unit, soc_feature_arl_hashed)) {
        SOC_IF_ERROR_RETURN(soc_l2x_stop(unit));
    }

    fd_test_data.test_type = FIFODMA_TEST_TYPE_PARAM;
    fd_test_data.rate_calc_interval = FIFODMA_RATE_CALC_INTERVAL_PARAM;
    fd_test_data.vchan = FIFODMA_CMC_PARAM;
    fd_test_data.hostmem_threshold = FIFODMA_HOSTMEM_THRESHOLD_PARAM;
    fd_test_data.num_entries = NUM_ENTRIES_READ_SW_PARAM;
    fd_test_data.fifo_depth = soc_mem_index_count(unit, L2_MOD_FIFOm);
    fd_test_data.fifo_width = soc_mem_entry_words(unit, L2_MOD_FIFOm);
    fd_test_data.use_api = TRUE;
    fd_test_data.stop_perf_test = 0;
    fd_test_data.total_fifo_drains = 0;

    parse_fifodma_test_params(unit, a);

    cli_out("\nInitializing host memory");
    fd_test_data.host_base = (unsigned int*)sal_dma_alloc(fd_test_data.fifo_depth *
                                                          fd_test_data.fifo_width *
                                                          sizeof(unsigned int), "HOST_BASE");
    for (i = 0; i < (fd_test_data.fifo_depth * fd_test_data.fifo_width); i++) {
        fd_test_data.host_base[i] = 0x0;
    }
    cmc = fd_test_data.vchan / N_DMA_CHAN;
    if (cmc != 0) {
        addr_remap = soc_pci_read(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_0_OFFSET(0));
        soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_0_OFFSET(cmc), addr_remap);
        addr_remap = soc_pci_read(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_1_OFFSET(0));
        soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_1_OFFSET(cmc), addr_remap);
        addr_remap = soc_pci_read(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_2_OFFSET(0));
        soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_2_OFFSET(cmc), addr_remap);
        if (SOC_REG_IS_VALID(unit, CMIC_CMC0_HOSTMEM_ADDR_REMAP_3r)) {
            addr_remap = soc_pci_read(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_3_OFFSET(0));
            soc_pci_write(unit, CMIC_CMCx_HOSTMEM_ADDR_REMAP_3_OFFSET(cmc), addr_remap);
        }
    }
    return 0;
}


int fifodma_test_cleanup(int unit, void *pa)
{
    int cmc = fd_test_data.vchan / N_DMA_CHAN;
    int ch  = fd_test_data.vchan % N_DMA_CHAN;
    unsigned int rdata;

    cli_out("\nCalling fifodma_fifoma_test_cleanup\n");
    sal_dma_free(fd_test_data.host_base);
    rdata = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch));
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, ABORTf, 1);
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, ENABLEf, 0);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch), rdata);
    soc_reg_field_set(unit, CMIC_CMC0_FIFO_CH0_RD_DMA_CFGr, &rdata, ABORTf, 0);
    soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc, ch), rdata);
    cli_out("\n");
    return 0;
}


int fifodma_test(int unit, args_t *a, void *pa)
{
    sal_thread_t pid_fifodma;
    int rv = 0;

    if (fd_test_data.test_type == FIFODMA_PERFORMANCE_TEST) {
        cli_out("\nDoing FIFO DMA performance test");
        pid_fifodma = sal_thread_create("FIFO DMA", 32 * 1024 * 1024, 200,
                                        performance_test_fifo_dma, INT_TO_PTR(unit));
        sal_usleep(1000000);
        fifodma_measure_rate(fd_test_data.rate_calc_interval);
        cli_out("\npid_fifodma = %p", pid_fifodma);
        fd_test_data.stop_perf_test = 1;
        sal_usleep(100000);
    } else {
        cli_out("\nDoing FIFO DMA feature test");
        feature_test_fifo_dma(unit);
    }
    if (fd_test_data.test_fail == 1) {
        rv = BCM_E_FAIL;
    } else {
        rv = BCM_E_NONE;
    }
    return rv;
}

#endif /* BCM_ESW_SUPPORT && BCM_CMICM_SUPPORT */
