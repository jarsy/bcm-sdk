/*
 * $Id: testlist.c,v 1.131 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        testlist.c
 * Purpose:     Defines list of tests and their associated
 *              functions.
 *
 * Notes:       Fields defined as follows:
 *
 *      Test # - Indicates the test number, all commands hat operate on tests
 *              can use either the test name or #. The # is intended to be the
 *              DV Test number, but is arbitrary. If the test number is
 *              negative, then the test is not selected by default; otherwise
 *              it is.
 *      Flags - See appl/diag/test.h for explanation
 *              TS88xxx- Test is supported on the specified chip.
 *      Test Name - The name of the test.
 *      Init Func - A routine called before the test is run to perform any
 *              setup required, may be NULL if no initialization required.
 *      Cleanup Func - Function called after test has run to reset any state
 *              etc, may be NULL if not required.
 *      Test Function - actual test function, called "Loop Count" times.
 *      Loop Count - Number of times to call "Test Function".
 *      Default Args - Default argument string passed into test.
 *
 * Order of a normal test execution is:
 *
 *      [1] Load TEST_RC file (if required)
 *      [2] Call "Init Func"
 *      [3] Call "Test Func" Loop times
 *      [4] Call "Cleanup Func"
 *
 * All external declarations for functions are found in testlist.h.
 */

#include <appl/diag/system.h>
#include <appl/diag/test.h>
#include <shared/bsl.h>
#include "testlist.h"

test_t sand_test_list[] = { /*

Test #      "Test Name"                 Flags/Chips             Init Func               Cleanup Func            Test Func          Loop Cnt  Default Args
----------------------------------------------------------------------------------------------------------------------------------------------------------*/
#if defined (BCM_DFE_SUPPORT) || defined (BCM_PETRA_SUPPORT)
TEST(1,   "Register reset defaults",  TS_DPP | TS_DFE,        0,                       0,              rval_test,       1,    0)
TEST(2,   "PCI Compliance",           TSEL | TS_DPP | TS_DFE, pci_test_init,           pci_test_done,  pci_test,        100,  0)
#endif
#if defined (BCM_SAND_SUPPORT)
TEST(3,   "Register read/write",      TS_SAND, 0,             0,                       reg_test,        1,    0)
#endif
#if defined (BCM_DFE_SUPPORT) || defined (BCM_PETRA_SUPPORT)
TEST(4,   "PCI S-Channel Buf",        TSEL | TS_DPP | TS_DFE, 0,                       0,              pci_sch_test,     100, 0)
TEST(5,   "BIST",                     TSEL | TS_DPP | TS_DFE, bist_test_init,          bist_test_done, bist_test,        1,   "ALL")
TEST(6,   "Memories Test",            TS_DPP | TS_DFE,        0,                       0,              memories_rw_test, 1,   0)
#endif
#if defined (BCM_DFE_SUPPORT) || defined (BCM_PETRA_SUPPORT)
TEST(7,   "Memories WR First Last Test", TS_SAND,             0,                       0,   memories_rw_first_last_test, 1,    0)
#endif
#ifdef BCM_PETRA_SUPPORT
TEST(8,   "Memories Flip/Flop Test",  TS_DPP,                 0,                       0,             mem_flipflop_test, 1,    0)
#endif
#if defined(BCM_DFE_SUPPORT)
TEST(9,   "Broadcast Blocks",         TS_DFE,                 0,                       0,                 brdc_blk_test, 1,    0)
#endif
#if defined (BCM_PETRA_SUPPORT)
TEST(17,  "CPU Loopback",             TRC | TSEL | TS_DPP,    lb_dma_init,             lb_dma_done,         lb_dma_test, 1,    0)
TEST(18,  "MAC Loopback",             TRC | TSEL | TS_DPP,    lb_mac_init,             lb_mac_done,         lb_mac_test, 1,    0)
TEST(19,  "PHY Loopback",             TRC | TSEL | TS_DPP,    lb_mii_init,             lb_mii_done,         lb_mii_test, 1,    0)
TEST(20,  "EXT Loopback",             TRC | TSEL | TS_DPP,    lb_ext_init,             lb_ext_done,         lb_ext_test, 1,    0)
#endif
TEST(21,  "CPU Benchmarks",           TS_DPP,                 benchmark_init,          benchmark_done,   benchmark_test, 1,    0)
#if defined (BCM_PETRA_SUPPORT)
TEST(22,  "CPU S/G, Reload",          TRC | TSEL | TS_DPP,    lb_reload_init,          lb_reload_done,   lb_reload_test, 1,    0)
TEST(23,  "CPU S/G, Simple",          TRC | TSEL | TS_DPP,    lb_sg_dma_init,          lb_sg_dma_done,   lb_sg_dma_test, 1,    0)
TEST(24,  "CPU S/G, Random",          TRC | TSEL | TS_DPP,    lb_random_init,          lb_random_done,   lb_random_test, 1,    0)
#endif
TEST(30,  "Counter widths",           TRC | TSEL | TS_DPP,    0,                       0,                 ctr_test_width,         1,      0)
TEST(31,  "Counter read/write",       TRC | TSEL | TS_DPP,    0,                       0,                      ctr_test_rw,            1,      0)
TEST(39,  "New Snake Test",           TRC | TS_DPP,           lb2_snake_init,          lb2_done,               lb2_snake_test,         1,      0)
TEST(40,  "BCM Packet Send",          TS_DPP,                 tpacket_tx_init,         tpacket_tx_done,        tpacket_tx_test,        1,      0)
#if defined(BCM_FIELD_SUPPORT) || defined (BCM_PETRA_SUPPORT)
TEST(41,  "BCM Packet Receive",       TRC | TS_DPP,           rpacket_init,            rpacket_done,           rpacket_test,           1,      0)
#endif
#if defined (BCM_PETRA_SUPPORT)
TEST(48,  "MAC Loopback - Mark 2",    TRC | TS_DPP,           lb2_mac_init,            lb2_done,               lb2_port_test,          1,      0)
#endif
TEST(49,  "PHY Loopback - Mark 2",    TRC | TS_DPP,           lb2_phy_init,            lb2_done,               lb2_port_test,          1,      0)
#if defined (BCM_DFE_SUPPORT)|| defined (BCM_PETRA_SUPPORT)
TEST(50,  "Memory Fill/Verify",       TS_DPP | TS_DFE,        mem_test_init,           mem_test_done,          mem_test,               1,      0)
TEST(51,  "Memory Random Addr/Data",  TS_DPP | TS_DFE,        mem_rand_init,           mem_rand_done,          mem_rand,               1,      0)
TEST(52,  "Rand Mem Addr, write all", TS_DPP | TS_DFE,        addr_rand_init,          addr_rand_done,         addr_rand,              1,      0)
TEST(60,  "Linkscan MDIO",            TRC | TSEL | TS_DPP | TS_DFE, ls_test_init,      ls_test_done,           ls_test,                1,      0)
#endif
#if defined(BCM_DFE_SUPPORT) || defined (BCM_PETRA_SUPPORT)
TEST(71,  "Table DMA",                TSEL | TS_DPP | TS_DFE, td_test_init,            td_test_done,           td_test_test,           1,      0)
#endif
TEST(72,  "Traffic Test",             TRC | TS_DPP,           traffic_test_init,       traffic_test_done,      traffic_test,           1,      0)
#if defined (BCM_PETRA_SUPPORT)
TEST(73,  "SNMP MIB Object Test",     TS_DPP,                 snmp_test_init,          snmp_test_done,         snmp_test_test,         1,      0)
TEST(90,  "TX Reload Test",           TRC | TS_DPP,           pktspeed_test_init,      pktspeed_test_done,        pktspeed_test_tx,        1,      0)
TEST(91,  "RX Reload Test",           TRC | TS_DPP,           pktspeed_test_init,      pktspeed_test_done,        pktspeed_test_rx,        1,      0)
#endif
#ifdef BCM_88750_A0
#endif
#ifdef BCM_DFE_SUPPORT
TEST(131, "DFE fabric snake test",    TS_DFE,                 fabric_snake_test_init,  fabric_snake_test_done,          fabric_snake_test,      1, 0)
#endif
#if defined (BCM_DDR3_SUPPORT) || defined (BCM_PETRA_SUPPORT)
TEST(140, "DDR bist test",            TS_DPP,                 ddr_bist_test_init,      ddr_bist_test_done,              ddr_bist_test,           1, 0)
#endif
#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
TEST(141, "DEINT INIT (SOC BCM)",     TS_DPP | TS_DFE,        init_deinit_test_init,   init_deinit_test_done,       init_deinit_test, 1, 0)
#endif
#ifdef BCM_PETRA_SUPPORT
TEST(142, "ARAD field tests",         TS_DPP,                 arad_field_test_init,    arad_field_test_done,             arad_field_test,       1, 0)
#endif
#if defined (SER_TR_TEST_SUPPORT)
TEST(144, "Software Error Recovery",  TS_DPP,                 ser_test_init,           ser_test_done, ser_test, 1, 0)
#endif
#ifdef BCM_PETRA_SUPPORT
TEST(145, "Diag PP test",             TS_DPP,                 diag_pp_test_init,       diag_pp_test_done, diag_pp_test, 1, 0)
#endif
#if defined(BCM_CMICM_SUPPORT) || defined(BCM_IPROC_SUPPORT)
TEST(146, "ARM core",                 TS_DPP,                 arm_core_test_init,      arm_core_test_done, arm_core_test, 1, 0)
#endif
#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
TEST(152, "Cache Memory Test",        TS_DPP,                 do_cache_mem_test_init,  do_cache_mem_test_done,   do_cache_mem_test,      1, 0)
TEST(153, "Memory Ser Test",          TS_DPP | TS_DFE,        memory_ser_test_init,    memory_ser_test_done,      memory_ser_test_run,   1, 0)
#endif
#if defined(BCM_JERICHO_SUPPORT) && defined(INCLUDE_KBP)
TEST(154, "KAPS TCAM BIST",           TS_DPP,                 0,                       0,      kaps_bist_test,   1, 0)
#endif
#ifdef BCM_PETRA_SUPPORT
TEST(155, "AC-MP and IP tunnel inlif learn info encoding", TS_DPP, 0 ,0, lif_learn_info_bist_test, 1, 0)
TEST(157, "Exhaustive Configuration", TS_DPP,                 exhaustive_test_init,   exhaustive_test_done,       exhaustive_test, 1, 0)
#endif
#if defined(BCM_JERICHO_SUPPORT) && defined(INCLUDE_KBP)
TEST(158, "KBP VER",                  TS_DPP,                 0,                       0,      kbp_sdk_ver_test,   1, 0)
TEST(159, "KAPS SEARCH",              TS_DPP,                 0,                       0,      kaps_search_test,   1, 0)
#endif
#if defined (BCM_PETRA_SUPPORT) && !defined (__KERNEL__) && defined (_SHR_SW_STATE_EXM)
TEST(160,  "SW_STATE",                TS_DPP,                 sw_state_test_init,      sw_state_test_done,        sw_state_test,      1, 0)
#endif
#if defined (BCM_PETRA_SUPPORT)
TEST(161, "OAMP",                     TS_DPP,                 oamp_test_init,          oamp_test_done,        oamp_test,      1, 0)
#endif
#if defined (BCM_PETRA_SUPPORT) && !defined (__KERNEL__)
TEST(162, "SW_STATE_ISSU",            TS_DPP,                 sw_state_issu_test_init, sw_state_issu_test_done,   sw_state_issu_test, 1, 0)
#endif
#if defined(BCM_DNX_SUPPORT)
TEST(200, "DNX DATA",                 TS_DNX,               0,                        0,                          test_dnx_data_test, 1, 0)
TEST(201, "DBAL_UNIT_TEST",           TS_DNX,               0,                        0,                          test_dnx_dbal_unit_test, 1, 0)
TEST(202, "DBAL_LOGICAL_TEST",        TS_DNX,               0,                        0,                          test_dnx_dbal_logical_test, 1, 0)
#endif
};

int sand_test_cnt = COUNTOF(sand_test_list);      /* # entries. */
