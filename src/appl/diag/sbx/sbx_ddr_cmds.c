/*
 * $Id: sbx_ddr_cmds.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbx_ddr_cmds.c
 * Purpose:     Handles DDR commands for SBX devices
 * Requires:
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <soc/defs.h>
#include <soc/cm.h>
#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/cmdlist.h>
#include <appl/diag/sbx/sbx.h>

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)

char cmd_sbx_ddr_mem_write_usage[] = "\n"
" DDRMemWrite ci<n> range=0xstart-[0xend] data=0xdata\n"
" DDRMemWrite ci0,ci1 range=0x0\n"
" DDRMemWrite ci9 range=0x0-0x100"
"\n";


cmd_result_t
cmd_sbx_ddr_mem_write(int unit, args_t *a)
{
#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
      return cmd_sbx_sirius_ddr_mem_write(unit, a);
  }
#endif
#ifdef BCM_CALADAN3_SUPPORT
  if (SOC_IS_CALADAN3(unit)) {
      return cmd_ddr_mem_write(unit, a);
  }
#endif
  cli_out("\nCommand is not supported on this device");
  return CMD_OK;
} 


char cmd_sbx_ddr_mem_read_usage[] = "\n"
" DDRMemRead ci<n> range=0xstart-[0xend]\n"
" DDRMemRead ci0,ci1 range=0x0\n"
" DDRMemRead ci9 range=0x0-0x100"
"\n";

cmd_result_t
cmd_sbx_ddr_mem_read(int unit, args_t *a)
{
#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
      return cmd_sbx_sirius_ddr_mem_read(unit, a);
  }
#endif
#ifdef BCM_CALADAN3_SUPPORT
  if (SOC_IS_CALADAN3(unit)) {
      return cmd_ddr_mem_read(unit, a);
  }
#endif
  cli_out("\nCommand is not supported on this device");
  return CMD_OK;
} 


char cmd_sbx_ddr_phy_read_usage[] = "\n"
" DDRPhyRead ci (all)\n"
" DDRPhyRead ci0     \n"
" DDRPhyRead ci0,ci1 \n";


/* for debug */
cmd_result_t
cmd_sbx_ddr_phy_read(int unit, args_t *a)
{
#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
      return cmd_sbx_sirius_ddr_phy_read(unit, a);
  }
#endif
#ifdef BCM_CALADAN3_SUPPORT
  if (SOC_IS_CALADAN3(unit)) {
      return cmd_ddr_phy_read(unit, a);
  }
#endif
  cli_out("\nCommand is not supported on this device");
  return CMD_OK;
}

char cmd_sbx_ddr_phy_write_usage[] = "\n"
" DDRPhyWrite ci<n> block=<block> offset=<0xoffset> data=<0xdata>\n"
" Examples: Write to all ci's bytelane0\n"
" DDRPhyWrite ci block=1 offset=0x040 data=0x0\n"
" Write to ci0,1 bytelane0\n"
" DDRPhyWrite ci0,ci1 block=1 offset=0x040 data=0x0\n"
" <block> = 0,1,2 (addr_ctrl,bytelane0,bytelane1)\n";

/* for debug */
cmd_result_t
cmd_sbx_ddr_phy_write(int unit, args_t *a)
{
#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
      return cmd_sbx_sirius_ddr_phy_write(unit, a);
  }
#endif
#ifdef BCM_CALADAN3_SUPPORT
  if (SOC_IS_CALADAN3(unit)) {
      return cmd_ddr_phy_write(unit, a);
  }
#endif
  cli_out("\nCommand is not supported on this device");
  return CMD_OK;
}

char cmd_sbx_ddr_phy_tune_usage[] = "\n"
" Only on sirius device:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
"   DDRPhyTune ci (all) tuning_mode=<0-6> read_vdl=<0-63> read_en_vdl=<0-63> addr_vdl=<0-63> write_vdl=<0-63> verify_mode=<0-1>\n"
"   verify_mode = 1 (default) use indirect memory read/write to verify if setting works\n"
"   verify_mode = 0 use ddr functional tests to verify if setting works\n"
"   verify_mode = 0 is much slower than verify_mode = 1 and detect more wrong settings\n"
" Only on caladan3:\n"
"   DDRPhyTune ci0[,ci<2n>] PhyType=<p> CtlType=<c> [SaveCfg=1] [RestoreCfg=1]\n"
" Tune all CIs on caladan3:\n"
"   DDRPhyTune ci0,ci2,ci4,ci6,ci8,c10,c12,c14 PhyType=3 CtlType=3\n"
#endif
;

/* for debug */
cmd_result_t
cmd_sbx_ddr_phy_tune(int unit, args_t *a)
{
#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
      return cmd_sbx_sirius_ddr_phy_tune(unit, a);
  }
#endif
#ifdef BCM_CALADAN3_SUPPORT
  if (SOC_IS_CALADAN3(unit)) {
      return cmd_ddr_phy_tune(unit, a);
  }
#endif
  cli_out("\nCommand is not supported on this device");
  return CMD_OK;
}

char cmd_sbx_ddr_phy_tune_auto_usage[] = "\n"
" DDRPhyTuneAuto ci tread_en=<tread_en> verify_mode=<0-1>\n"
" DDRPhyTuneAuto ci tread_en=13\n"
" Will Tune for tread_en windows for <tread_en> and <tread_en+1> \n"
" optimal soc parameters are printed to screen for each ci interface\n"
" verify_mode = 1 (default) use indirect memory read/write to verify if setting works\n"
" verify_mode = 0 use ddr functional tests to verify if setting works\n"
" verify_mode = 0 is much slower than verify_mode = 1 and detect more wrong settings\n";

cmd_result_t
cmd_sbx_ddr_phy_tune_auto(int unit, args_t *a)
{
#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
      return cmd_sbx_sirius_ddr_phy_tune(unit, a);
  }
#endif
  cli_out("\nCommand is not supported on this device");
  return CMD_OK;
}


#endif /* BCM_SIRIUS_SUPPORT || BCM_CALADAN3_SUPPORT */
